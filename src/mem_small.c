/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <assert.h>
#include "mem.h"
#include "mem_internals.h"

void *
emalloc_small(unsigned long size)
{   size += 4*sizeof(long);
    assert(size <= 96);
    //check if arena.chunkpool null si oui on chainer les ptrs
    if(arena.chunkpool == NULL){
        unsigned long size_block = mem_realloc_small();
        for(unsigned long i = 0; i <= size_block - CHUNKSIZE; i += CHUNKSIZE){
            *(void**) (arena.chunkpool + i) = (arena.chunkpool + i + CHUNKSIZE);
        }
        *(void**) (arena.chunkpool + size_block - CHUNKSIZE) = NULL;
    }
    // enlever la tete de chunkpool
    void* ptr = arena.chunkpool;
    arena.chunkpool = *((void**) ptr);
    return mark_memarea_and_get_user_ptr(ptr, CHUNKSIZE, SMALL_KIND);
}

void efree_small(Alloc a) {
    *((void**) a.ptr) = arena.chunkpool;
    arena.chunkpool = a.ptr;
}
