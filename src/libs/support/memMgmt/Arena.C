/* $Id: Arena.C,v 1.3 1998/02/19 15:24:29 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * Arena.C
 *
 * Arena-based allocation, based on the paper
 *
 *      Fast Allocation and Deallocation of Memory Based on Object Lifetimes
 *      David Hanson
 *      Software -- Practice and Experience, January 1990
 *
 */

#include <libs/support/misc/general.h>
#include <stdlib.h>
#include <memory.h>
#include <libs/support/memMgmt/Arena.h>

#define MEMINCR 10


/****************************************************************************/
/*                                                                          */
/*   Function:       arena_type                                             */
/*                                                                          */
/*   Input:          s - number of arenas                                   */
/*                                                                          */
/*   Description:    Creates a number of arenas for memory allocation.      */
/*                                                                          */
/****************************************************************************/


arena_type::arena_type(int s)

  {
   int i;

     arena = (Arenas *)malloc(sizeof(Arenas)*s);
     first = (Arena *)malloc(sizeof(Arena)*s);
     num_arenas = s;
     for (i=0; i<num_arenas; i++)
       {
	char *p = (char *) &(first[i]) + sizeof(Arena);
	first[i].avail = p;
	first[i].limit = p;
	first[i].next = 0;
	arena[i] = &(first[i]);
       }
     free_arenas = 0;
  }


/****************************************************************************/
/*                                                                          */
/*   Function:     ~arena_type                                              */
/*                                                                          */
/*   Description:  Free up space taken by arenas                            */
/*                                                                          */
/****************************************************************************/


arena_type::~arena_type()

  {
   Arenas ap=free_arenas,dp;

     if (ap != (Arenas)0)
       while(ap->next)
	 {
	  dp = ap;
	  ap = ap->next;
	  free((char *)dp);
	 }
     free((char *)arena);
     free((char *)first);
  }


/****************************************************************************/
/*                                                                          */
/*   Function:      allocate                                                */
/*                                                                          */
/*   Input:         n - size of object to allocate                          */
/*                  k - arena in which object is allocated                  */
/*                                                                          */
/*   Description:   Create space within an arena for an object              */
/*                                                                          */
/****************************************************************************/


void *arena_type::allocate(int n,int k)
  
  {
   Arenas ap;
   char *v;

     for (ap=arena[k]; ap->avail+n > ap->limit; arena[k] = ap) 
       {
	Arenas tp = ap->next;
	if (tp) 
	 {
	  ap = tp;
	  ap->avail = (char *) ap + sizeof(Arena);
	 }
	else 
	  if (ap->next = free_arenas) 
	    {
	     free_arenas = free_arenas->next;
	     ap = ap->next;
	     ap->avail = (char *) ap + sizeof(Arena);
	     ap->next = 0;
	    }
	  else 
	    {
	     int m = n + MEMINCR * 1024;
	     char *tp = (char *) malloc(m);
	     if (tp == ap->limit)
	       ap->limit = tp + m;
	     else 
	       {
		ap->next = (Arenas) tp;
		ap = (Arenas) tp;
		ap->limit = tp + m;
		ap->avail = tp + sizeof(Arena);
		ap->next = 0;
	       }
	    }
       }
     v = ap->avail;
     ap->avail += n;
     return v;
  }


/****************************************************************************/
/*                                                                          */
/*   Function:     arena_deallocate                                         */
/*                                                                          */
/*   Input:        t - which arena to deallocate                            */
/*                                                                          */
/*   Description:  Adjust pointers within an arena so that no space is      */
/*                 allocated.                                               */
/*                                                                          */
/****************************************************************************/


void arena_type::arena_deallocate(int t)

  {
   Arenas ap = arena[t];

     while (ap->next)
       ap = ap->next;
     ap->next = free_arenas;
     free_arenas = first[t].next;
     first[t].next = 0;
     arena[t] = &(first[t]);
  }


/****************************************************************************/
/*                                                                          */
/*   Function:     arena_alloc_mem                                          */
/*                                                                          */
/*   Input:        n - which arena to allocate within                       */
/*                 k - size of space to allocate                            */
/*                                                                          */
/*   Description:  Allocate space within an arena for an object.  Create    */
/*                 more space if necessary.                                 */
/*                                                                          */
/****************************************************************************/


void *arena_type::arena_alloc_mem(int n,
				  int k)
  
  {
   Arenas a = arena[n];
   char *p = a->avail;
   char *q;

#ifdef LONG_POINTER
     k = (k + 7) & ~7;    /* ensuring alignment to 8 bytes */
#else
     k = (k + 3) & ~3;    /* ensuring alignment to 4 bytes */
#endif
     q = p + k;
     if (q > a->limit)
       return allocate(k,n);
     else
       {
	a->avail = q;
	return p;
       }
  }


/****************************************************************************/
/*                                                                          */
/*   Function:     arena_alloc_mem_clear                                    */
/*                                                                          */
/*   Input:        n - which arena to allocate within                       */
/*                 k - size of space to allocate                            */
/*                                                                          */
/*   Description:  Allocate space within an arena for an object.  Create    */
/*                 more space if necessary.  Finally, zero memory.          */
/*                                                                          */
/****************************************************************************/


void *arena_type::arena_alloc_mem_clear(int n,
					int k)

  {
   Arenas a = arena[n];
   char *p = a->avail;
   char *q;

#ifdef LONG_POINTER
     k = (k + 7) & ~7;    /* ensuring alignment to 8 bytes */
#else
     k = (k + 3) & ~3;    /* ensuring alignment to 4 bytes */
#endif
     q = p + k;
     if (q > a->limit)
       p = (char *)allocate(k,n);
     else
       a->avail = q;
     memset(p, 0, k);
     return p;
  }



