/* $Id: mem.C,v 1.3 2001/09/17 01:29:21 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 			   mem.c			*/
		/* 		   Memory entry points.			*/
		/* 							*/
		/********************************************************/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include <include/bstring.h>

#include <libs/support/memMgmt/mem.h>

extern int end;                                    /* the end of uninitialized data   */

#define	SBRK_INCR   0x00010000                  /* amount to extend the break      */
#define	ZAP_CHAR    (0x0F)			/* the character to zap with       */
#define SIZE        1000			/* the size of the output buffer   */

static	short       mem_debug = 0;		/* memory debugging count          */
static	Boolean     should_zap = false;	        /* true if we should zap memory	   */
static	Generic     mem_count = 0;		/* the number of times been called */
static	char        buf[SIZE];		        /* the output buffer               */

Generic             total_rn_memory_in_use = 0; /* amt malloc'ed by Rn code        */
Generic             other_memory_in_use = 0;    /* estimate of non-Rn mallocs      */

STATIC(void, zap_block, (char*, int));

	/* memory allocator (machine) specific macros! */
#ifdef sparc
#define	BLK_SIZE(a)	(*(((int*)a) - 2))	/* size of malloc'd block   */
#define	RGN_SIZE(a)	(BLK_SIZE(a) - 8)	/* size of the usable part  */
#endif /* sparc */

#ifdef _AIX
#define	BLK_SIZE(a)	(8<<(*(((int*)a) - 1))) /* size of malloc'd block   */
#define	RGN_SIZE(a)	(BLK_SIZE(a) - 8)	/* size of the usable part  */
#endif /* _AIX */

#ifndef BLK_SIZE	/* sun3 */
#define	BLK_SIZE(a)	(*(((int*)a) - 1))	/* size of malloc'd block   */
#define	RGN_SIZE(a)	(BLK_SIZE(a) - 4)	/* size of the usable part  */
#endif


/* Allocate a chunk of memory with a purpose. */
void* get_mem(int size, char* format, ...)
{
  va_list arg_list;		/* the argument list as per varargs	*/
  void*   where;	       	/* where the memory is allocated	*/
  int     internalSize;

  if (mem_count++ == 0)
    {     /* determine the the first estimate for o.m.i.u. */
       other_memory_in_use = (Generic)sbrk(0) - (Generic)&end;
    }

  va_start(arg_list, format);
    {
       if (size < 0)
         {
            fprintf(stderr, "get_mem(): passed size %d\n", size);
            internalSize = 1;
         }
       else if (size == 0)
         {
            internalSize = 1;
         }
       else 
         {
            internalSize = size;
         }

       where = (void*)malloc((unsigned)internalSize);

       if (!where)
         {     /* malloc failed! */
            die_with_message("get_mem():  malloc returned 0! (Out of memory).");
         }

       if (should_zap)
         {     /* zap this block of memory */
            zap_block((char*)where, RGN_SIZE(where));
         }

       total_rn_memory_in_use += BLK_SIZE(where);

       if (mem_debug)
         {     /* print out a message with regard to this request */
            fprintf(stderr, "get_mem of %d bytes by ", internalSize);
            vfprintf(stderr, format, arg_list);
            fprintf(stderr, " at %d\n", (Generic)where);
            fflush(stderr);
         }
    }
  va_end(arg_list);

  return (where);
}


/* Reallocate a chunk of memory with a purpose.	*/
void* reget_mem(void* old, int size, char* format, ...)
{
  va_list  arg_list;		/* the argument list as per varargs	*/
  void*    chunk;		/* the new chunk of memory		*/
  int      internalSize;

  mem_count++;
  va_start(arg_list, format);
    {
       if (!old)
         {
            fprintf(stderr, "reget_mem(): passed NIL pointer\n");
            return (void*)0;
         }

       if (size <= 0)
         {
            fprintf(stderr, "reget_mem(): passed size %d\n", size);
            internalSize = 1;
         }
       else 
         {
            internalSize = size;
         }

       total_rn_memory_in_use -= BLK_SIZE(old);

       if (should_zap)
         {     /* zap incoming and outgoing blocks */
            chunk = (void*)malloc((unsigned)internalSize);
            if (!chunk)
              {     /* realloc failed! */
                 die_with_message("reget_mem():  realloc returned 0! (Out of memory).");
              }
            bcopy((const char *)old, (char *)chunk, MIN(RGN_SIZE(old), RGN_SIZE(chunk)));
            zap_block((char*)chunk + RGN_SIZE(old), MAX(0, RGN_SIZE(chunk) - RGN_SIZE(old)));
            zap_block((char*)old, RGN_SIZE(old));
            free(old);
         }
       else
         {     /* normal realloc */
            chunk = (void*)realloc(old, (unsigned)internalSize);
            if (!chunk)
              {     /* realloc failed! */
                 die_with_message("reget_mem():  realloc() returned 0!\nOut of memory.\n");
              }
         }

       total_rn_memory_in_use += BLK_SIZE(chunk);

       if (mem_debug)
         {     /* print out a message with regard to this request */
            fprintf(stderr, "freeing (reget_mem) %d\n", (Generic)old);
            fprintf(stderr, "get_mem (reget_mem) of %d bytes by ", internalSize);
            vfprintf(stderr, format, arg_list);
            fprintf(stderr, " at %d\n", (Generic)chunk);
            fflush(stderr);
         }
    }
  va_end(arg_list);

  return (chunk);
}


/* Free a chunk of memory from above.							*/
void free_mem(void* old)
{
  if (!old)
    {
       fprintf(stderr, "free_mem(): passed NIL pointer\n");
       return;
    }

  mem_count++;
  if (should_zap)
    {     /* zap the freed memory */
       zap_block((char*)old, RGN_SIZE(old));
    }

  total_rn_memory_in_use -= BLK_SIZE(old);
  free(old);

  if (mem_debug)
    {     /* print out a message with regard to this request */
       fprintf(stderr, "freeing %d\n", (Generic)old);
       fflush(stderr);
    }

  return;
}


/* Zap a block of memory of a specified size. */
static void zap_block(char* ptr, int bytes)
{
  if (bytes > 0)
    {     /* there is work to do */
       while (bytes--)
         {     /* zap a byte */
            *ptr++ = ZAP_CHAR;
         }
    }

  return;
}


/* Increment memory debugging count. */
void turn_on_mem_debug(void)
{
  mem_debug++;

  return;
}


/* Decrement memory debugging count. */
void turn_off_mem_debug(void)
{
  mem_debug--;

  return;
}


/* Turn on zap memory flag.  May only be set at beginning. */
void set_zap_mem(void)
{
  if (!should_zap)
    {     /* the flag is not set */
       if (mem_count++ == 0)
         {     /* determine the the first estimate for o.m.i.u. */
            other_memory_in_use = (Generic)sbrk(0) - (Generic)&end;
         }
       else
         {     /* not called at the beginning */
            die_with_message("set_zap_mem(): Can't set flag after memory was allocated.");
         }
       should_zap = true;
    }

  return;
}


/* Return the approximate amount of sbrk()able memory left. */
Generic rn_size_of_brkable_region(void)
{
  void* current;   /* the current break value     */
  void* pushed;    /* the highest breakable value */
  void* temp;      /* a temporary test for pushed */

  current = (void*)sbrk(0);

  for (pushed = current; (temp = (void*)sbrk(SBRK_INCR)) != (void*)(-1); pushed = temp)
    {     /* we successfully pushed another break */
    }

     /* reset the break */
  brk((char *)current);

  return (Generic)((Generic)pushed - (Generic)current);
}


/* Return a (static) string describing the current memory usage. */
char* rn_memory_use_string(void)
{
  static char buffer[100];		/* the output buffer		*/

  sprintf(buffer, "%dK in use, %d+%dK left", (Generic)(total_rn_memory_in_use >> 10),
       (Generic)(((Generic)sbrk(0) - (Generic)&end - total_rn_memory_in_use - other_memory_in_use) >> 10),
       (rn_size_of_brkable_region() >> 10));

  return buffer;
}
