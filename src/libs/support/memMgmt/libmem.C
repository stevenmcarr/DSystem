/* $Id: libmem.C,v 1.7 1997/03/27 20:50:26 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * Portable SAFE memory usage, currently BUGGY
 *
 * Author: Cliff Click, Copyright (C) 1991, 1992
 * This material is released for public use, but is not placed in the public
 * domain.  You are free to use/mangle/copy this code, but since I retain
 * ownership you cannot copyright or patent this code.	Please leave this
 * copyright notice in copies of this code, so that future programmers cannot
 * be denied using it.
 */

#ifdef __GNUC__

#include <stdio.h>
#include <stdlib.h>

#elif  __TURBOC__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloc.h>
#define malloc	farmalloc
#define calloc	farcalloc
#define realloc farrealloc
#define free	farfree

#else  All other machines

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#endif

#include <libs/support/misc/port.h>
/* #include "memory.h"   Do not include; macros shadow system calls */
#define _MEMORY_
void *sys_malloc( uint32 size ) { return malloc( size ); }
void *sys_calloc( uint n, uint32 size ) { return calloc( n, size ); }
void *sys_realloc( void *ptr, uint32 size )
  { return realloc( (char *)ptr, size ); }
void sys_free( void *ptr ) { free( (char *)ptr ); }
#include <libs/support/misc/dict.h>
#define min(a,b) (((a)<(b))?(a):(b))


/*------------------------------data-----------------------------------------*/
static long used;
static Dict _Heap(NULL,hashptr,1024);  /* Dictionary of valid memory heaps */
static const char *fence = "0123456789ABCDE";
static uint32 highwater = 0;	/* Maximum memory ever used */
static uint32 totalmem = 0;	/* Current memory ever used */

/* For every heap block allocated a TAGS structure is allocated BEFORE the   */
/* heap block, followed by a "picket" area of 16 bytes.  Thus the user's     */
/* area extends from the address of the TAGS structure + sizeof(Tag)+16 to   */
/* 16 bytes before the end.  The checksum is used to catch if the user is    */
class Tag;
static void printnode( Tag *tag );
class Tag {		/* Memory tags */
 public:
  const char *fname;		/* Filename of caller */
  uint32 size;			/* Actual size of area */
  int32 checksum;		/* Sanity check */
  uint16 line;			/* Line number of caller */

/* Sum the node's values, except for the checksum itself. */
  int32 makechecksum( void ) { return (int32)(fname)+line+size; }
/* Heap corrupted, final print & exit */
  void deathnode( const char *msg )
    {fputs(msg,stderr); printnode(this); abort();}
  void validnode( void );

};

/*------------------------------prototypes-----------------------------------*/
typedef void (*NodeFunc)(Tag *node); /* Functions on nodes */

static void doall( NodeFunc f );
static Tag *dofind( char huge *ptr );
static void doins( Tag *node );
static void dodel( Tag *tag );

/*------------------------------my_malloc------------------------------------*/
/* Allocate storage of given size.  If fails, report error & abort.  Save    */
/* block in array of allocated blocks, with 16 byte pad on either size.      */
void *my_malloc(const char *fname, uint line, uint32 size)
{
  register Tag *tag;
  char huge *s;
  uint32 bs;

  if( size > 2000000000L ) {		/* Sanity check */
    fprintf(stderr,"Bad malloc size: %ld bytes, file %s line %d.\n",
	    size,fname,line);
    abort();
  }
  bs = size+sizeof(Tag)+32;	/* Size of block, after debug stuff added */
  tag = (Tag *)malloc((size_t)bs);	/* Get space */
  if( !tag ) {			/* Bonk! */
    fprintf(stderr,"Out of memory in file %s line %d allocating %ld bytes.\n",
	   fname,line,size);
    abort();
  }
  tag->fname = fname;		/* Save info about who got space */
  tag->line = line;
  tag->size = bs;
  s = ((char *)tag) + sizeof(Tag);
  if( dofind( s+16 ) ) {	/* Allocations disjoint? */
    fprintf(stderr,"Heap corrupted, new allocation overwrites old.  New, then old:\n");
    printnode( tag );
    printnode( dofind( (char huge *)tag ) );
    abort();
  }
  strcpy( s, fence );		/* Insert under & over-run fences */
  strcpy( s+16+size, fence );
  doins( tag ); 		/* Insert block in dictionary of blocks */
  totalmem += size;		/* Bump total memory every allocated */
  if( totalmem > highwater ) highwater = totalmem;
  return s+16;			/* The user's area starts here */
}

/*------------------------------my_free--------------------------------------*/
/* Free previously allocated data.  Check for valid free ptr & good block.   */
void  my_free(const char *fname, unsigned line, void *ptr)
{
  Tag *node;
  register int32 k;
  register int32 *np;
  uint32 i, j;

  node = dofind( (char huge *)ptr ); /* Find node going */
  if( !node ) { 		/* For starters, freeing something weird */
#ifdef __TURBOC__
    fprintf(stderr,"Freeing unallocated pointer %p in file %s line %d.\n",
#else  __TURBOC__
    fprintf(stderr,"Freeing unallocated pointer %d in file %s line %d.\n",
#endif __TURBOC__
	    ptr, fname, line );
/*    doall( printnode );	 Walk the tree, printing nodes */
    abort();
  }
  j = node->size >> 2;
  k = ((((('F'<<8)+'R')<<8)+'E')<<8)+'E';
  totalmem -= (node->size - sizeof(Tag) - 32);
  np = (int32 *)node;
  for( i=0; i<j; i++ ) *np++ = k; /* Fill freed memory with `FREE' pattern */
  dodel( node );		  /* Remove from tree */
}

/*------------------------------my_calloc------------------------------------*/
void *my_calloc(const char *fname, unsigned line, unsigned nitems, uint32 size)
{
  void *ptr;

  if( (size < 0L) || (nitems < 0) || (nitems*size < 0L) ) {
    fprintf(stderr,"Bad calloc parameters: %d x %ld = %ld bytes, file %s line %d.\n",
	    nitems,size,nitems*size,fname,line);
    abort();
  }
  ptr = my_malloc(fname,line,nitems*size); /* Get space */

/* Exactly HERE TurboC breaks with objects > 64K!!!  */
/* Turbo lacks copy & fill for objects > 64K	     */
  bzero(ptr,(uint)(nitems*size));    /* Clear it out */
  return ptr;
}

/*------------------------------my_realloc-----------------------------------*/
void *my_realloc(const char *fname, unsigned line, void *ptr, uint32 size)
{
  Tag *node;			/* Node being altered */
  char *s;
  uint32 len;

  node = dofind( (char huge *)ptr ); /* Find node being altered */
  if( !node ) { 		/* Realloc`ing something weird */
    fprintf(stderr,"Realloc'ing unallocated pointer %d in file %s line %d.\n",
	   ptr, fname, line );
    abort();
  }
  s = (char *)my_malloc( fname, line, size );
  len = node->size - sizeof(Tag) - 32;
/* HERE TurboC breaks with objects > 64K */
  bcopy(s,ptr,(uint)(min(len,size)));
  totalmem -= len;	       /* Already added for malloc'd area, now lower */
  dodel( node );	       /* Nuke old node */
  return s;		       /* Return adjusted user's area */
}

/*------------------------------close_heap-----------------------------------*/
/* For every un-freed Tag structure, the user has lost some memory.  Zero is */
/* returned if heap is clean, -1 if dangling heap blocks.		     */
signed char close_heap( void )
{
  if( !_Heap.Size() ) { 	/* No heap allocations? */
    highwater = totalmem = 0;	/* Clear out stats */
    return 0;			/* All OK, return good */
  }
  fprintf(stderr,"Blocks not freed before closing heap:\n");
  doall( printnode );		/* Walk the tree, printing nodes */
  doall( dodel );
  return -1;
}

/*------------------------------Functions over nodes-------------------------*/
/* Validate a node */
void Tag::validnode()
{
  if( makechecksum() != checksum )
    deathnode("Corrupted block header!  Checksum invalid, block is:");
  if( strncmp( ((char huge *)this)+sizeof(Tag), fence, 16 ) )
    deathnode("Block underrun:");
  if( strncmp( ((char huge *)this)+size-16, fence, 16 ) )
    deathnode("Block overrun:");
}

/*------------------------------Abstract data type routines------------------*/
/* Insert a node, no check for duplicates */
static void doins( Tag *node )
{
  node->checksum = node->makechecksum();
  _Heap.Insert(node,node);
}

/* Find a node based on key.  Key must land on node proper.  Returns NULL    */
/* or a Tag *ptr.							     */
static Tag *dofind( char huge *node )
{
  node -= sizeof(Tag)+16;
  return (Tag *)_Heap[node];
}

/* Walk the dictionary, performing given routine on each node.	If any node  */
/* is corrupted print out what you can on the node & give up there.	     */
static void doall( NodeFunc f )
{
  forDict(&_Heap) {
    ((Tag *)v)->validnode();
    f((Tag *)v);
  } endDict;
}

/* Delete a node, no check for not there.  NOT a destructor!!! */
static void dodel( Tag *tag )
{
  _Heap.Delete(tag);
  free((char *)tag);
}

/* Print a node */
static void printnode( Tag *tag )
{
#ifdef __TURBOC__
    fprintf(stderr,"Block size %5ld created by %15s on line %4d at %p\n",
#else  __TURBOC__
    fprintf(stderr,"Block size %5ld created by %15s on line %4d at %lX\n",
#endif __TURBOC__
      tag->size-sizeof(Tag)-32L, tag->fname, tag->line,
      ((int32)(tag))+sizeof(Tag)+16 );
}

/* Add size of this node to total */
static void addit(Tag *tag)
{
  used += tag->size - sizeof(Tag) - 32;
}

/* Print some nice statistics about the tree */
uint32 heapstats( void )
{
  int32 cnt,max;

  used = 0L;
  doall( addit );
  cnt = _Heap.Size();
  max = _Heap._get_size();
  if( !max ) max = 65536L;
  fprintf(stderr,"Used=%9ld  Blocks=%5ld  Buckets:%5ld  Fullness:%3d%%\n",
	  used,cnt,max,(100L*cnt)/max);
  return highwater;
}
