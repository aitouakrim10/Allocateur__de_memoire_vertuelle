cmake_minimum_required(VERSION 3.20)
####
# The version number in previous line may be reduced if needed.
####

project(ensimalloc)

#########
# Gestion des équipes et variante du sujet / Team management and addition
#########
if (EXISTS ${CMAKE_SOURCE_DIR}/AUTHORS)
  message(STATUS "Les auteurs du TP incrits dans AUTHORS sont / AUTHORS are:")
  execute_process (COMMAND cat AUTHORS WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
else()
  message(FATAL_ERROR "** ERREUR / ERROR **: Vous devez mettre vos prénoms, noms et login dans un fichier nommé 'AUTHORS' à la racine du projet / Add an 'AUTHORS' file with all authors' name, firstname and login at the project root; Fichier manquant/missing file: "  ${CMAKE_SOURCE_DIR}/AUTHORS)
endif()

###
# Configuration de cmake et de C
###
enable_testing()
# Use the 2017-2018 C standard with GNU extensions ("99" instead
# should work too with this skeleton)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Werror -fanalyzer -fsanitize=undefined -fsanitize-address-use-after-scope")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Werror -fsanitize-address-use-after-scope")

# Nom des auteurs comme un identifiant C (monoligne alpha)
execute_process(COMMAND cat AUTHORS WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} OUTPUT_VARIABLE AUTHOR_LIST)
string(STRIP "${AUTHOR_LIST}" AUTHOR_LIST)
string(MAKE_C_IDENTIFIER "${AUTHOR_LIST}" AUTHOR_LIST)

#######
# Compilation standard avec googletest
#######

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

##
# Si vous utilisé plusieurs fichiers, en plus de mem.c et les autres,
# pour votre allocateur il faut les ajouter ici
##
add_library(ensimalloc SHARED src/mem.c src/mem_internals.c src/mem_small.c src/mem_medium.c src/mem_large.c)

##
# Construction du programme de tests unitaires
##
add_executable(alloctest tests/alloctest.cc tests/test_mark.cc tests/test_generic.cc tests/test_buddy.cc tests/test_run_cpp.cc)
target_link_libraries(alloctest gtest gtest_main ensimalloc)
add_test(AllTestsAllocator alloctest)

##
# Ajout d'une cible pour lancer les tests de manière verbeuse
##
add_custom_target(check alloctest)

##
# Construction du shell
##
add_executable(memshell src/memshell.c)
target_link_libraries(memshell ensimalloc)

##
# Construction de l'archive
# inclu le .git si il est là
##
set(CPACK_PACKAGE_VERSION_MAJOR "3")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0-${AUTHOR_LIST}")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_IGNORE_FILES
  "~$"
  "\\\\.o$"
  "^${PROJECT_SOURCE_DIR}/build/"
)
include(CPack)
