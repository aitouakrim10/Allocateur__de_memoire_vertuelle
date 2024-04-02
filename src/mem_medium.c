/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <stdint.h>
#include <assert.h>
#include "mem.h"
#include "mem_internals.h"

unsigned int puiss2(unsigned long size) {
    unsigned int p=0;
    size = size -1; // allocation start in 0
    while(size) {  // get the largest bit
	p++;
	size >>= 1;
    }
    if (size > (1 << p))
	p++;
    return p;
}

void split(void* ptr, uint32_t exp, uint32_t nbr_blocks){
    if(nbr_blocks == 0){
        *(void**) ptr = arena.TZL[exp];
        arena.TZL[exp] = ptr;
        return;
    }
    void* comp = (void*)((uintptr_t) ptr ^ (1 << (exp - 1)));
    *((void**) comp) = arena.TZL[exp-1];
    arena.TZL[exp-1] = comp;
    return split(ptr, exp - 1, nbr_blocks - 1);
}

void *
emalloc_medium(unsigned long size)
{
    assert(size < LARGEALLOC);
    assert(size > SMALLALLOC);
    //full size
    size += 4*sizeof(unsigned long);
    uint32_t index = puiss2(size);
    unsigned long size_tot = 1 << index;
    // si tzl[indec] est diponible : exploitation de 1 er block
    if( arena.TZL[index] != NULL){
        void* ptr = arena.TZL[index];
        arena.TZL[index] = *((void**) ptr);
        return mark_memarea_and_get_user_ptr(ptr, size_tot, MEDIUM_KIND);
    }
    // sinon on cherche indx tlq tzl[indx] est disponinle
    uint32_t dispo_index = index;
    while(arena.TZL[dispo_index] == NULL && (dispo_index < FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant)){
        dispo_index++;
    }
    // si on depasse la limite de tzl on ajoute un nouveau block
    if( dispo_index == FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant){
        mem_realloc_medium();
    }
    // on partitione le block disponible sur les blocks null
    void* ptr0 = arena.TZL[dispo_index];
    arena.TZL[dispo_index] = *((void**) ptr0);
    uint32_t nomb_blocks = dispo_index - index;
    // partion de block
    split(ptr0, dispo_index, nomb_blocks);
    // enlever le block demander et faire le marquage
    void* ptr = arena.TZL[index];
    arena.TZL[index] = *((void**) ptr);
    return mark_memarea_and_get_user_ptr(ptr, size_tot, MEDIUM_KIND);
}

void efree_medium(Alloc a) {
    uint32_t index = puiss2(a.size);
    void* aptr = a.ptr;
    while(index < FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant){
        void* buddy = (void*)((uintptr_t) aptr ^ (1 << index));
        // verifier si buddy appartient a tzl[index]
        void* ptr = arena.TZL[index];
        void* prev =arena.TZL[index];
        char deja_traite = 0;
        // si buddy est le 1 er block
        if( ptr == buddy){
            arena.TZL[index] = *(void**)ptr;
            aptr = ((uintptr_t) aptr < (uintptr_t) buddy)? aptr : buddy;
            ptr = *(void**)ptr;
            deja_traite = 1;
        }
        while(ptr != NULL && deja_traite == 0){
            //si buddy existe
            if(ptr == buddy){
                (*(void**) prev) = *((void**) ptr);
                aptr = ((uintptr_t) aptr < (uintptr_t) buddy)? aptr : buddy;
                break;
            }
            prev = ptr;
            ptr = *((void**) ptr);
        }
        if(ptr == NULL && deja_traite == 0){
            // si il n existe pas
            *((void**) aptr) = arena.TZL[index];
            arena.TZL[index] = aptr;
            break;
        }
        index++;
    }
}

