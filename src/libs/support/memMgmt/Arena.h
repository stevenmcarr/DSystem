/* $Id: Arena.h,v 1.2 1997/03/11 14:36:51 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * arena.h
 *
 * Arena-based allocation, based on the paper
 *
 *      Fast Allocation and Deallocation of Memory Based on Object Lifetimes
 *      David Hanson
 *      Software -- Practice and Experience, January 1990
 *
 * copied from Preston's C code
 *
 */

#ifndef Arena_h
#define Arena_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

  typedef struct arena_chunk {
    struct arena_chunk *next;
    char *limit;
    char *avail;
   } Arena, *Arenas; 

class arena_type {

  Arenas *arena;
  Arena  *first;
  Arenas free_arenas;
  int    num_arenas;

  void *allocate(int Size, int WhichArena);

public:
  
  arena_type(int NumArenas);
  ~arena_type();
  void arena_deallocate(int WhichArena);
  void *arena_alloc_mem(int Size, int WhichArena);
  void *arena_alloc_mem_clear(int Size, int WhichArena);
 };

EXTERN(void, arena_deallocate,(int WhichArena));
EXTERN(void *, arena_alloc_mem,(int Size,int WhichArena));
EXTERN(void *, arena_alloc_mem_clear,(int Size,int WhichArena));
#endif

