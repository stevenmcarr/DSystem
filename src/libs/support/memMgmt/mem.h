/* $Id: mem.h,v 1.5 1997/03/11 14:36:54 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/*			   mem.h			*/
		/* 		  Memory use functions.			*/
		/*		        (util/mem.c)			*/
		/********************************************************/


#ifndef mem_h
#define mem_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

	/* Memory Allocation / Deallocation */

EXTERN(void*, get_mem, (int size, char* format, ...));
/* Takes two parameters (unsigned size) the size of the chuck to be allocated and	*/
/* (char* s) the description of the memory being allocated.  Returns a pointer to the	*/
/* chunk.  The new block may be zapped and/or a debugging message printed depending on	*/
/* the current debugging conditions.  See below.					*/

EXTERN(void*, reget_mem, (void* p, int size, char* format, ...));
/* Takes three parameters (void* p) the pointer to a previously get_mem()ed block,	*/
/* (unsigned size) the new chunk size, and (char* s) the description of the memory being*/
/* reallocated.  The old memory block will be copied up to the size of the smaller	*/
/* block.  The unused portion of the new block and the entire old block may be zapped	*/
/* and/or a memory debugging message printed depending on the current debugging		*/
/* conditions.  See below.								*/

EXTERN(void, free_mem, (void* old));
/* Takes one parameter (void* p) the pointer to the block being freed.  The block may	*/
/* be "zapped" and/or a memory debugging message printed depending on the current	*/
/* debugging conditions.  See below.							*/


	/* Memory Debugging */

EXTERN(void, turn_on_mem_debug, (void));
/* Takes no parameters.  Increments a memory debugging count used with get_mem() and	*/
/* reget_mem(), and free_mem().  The count value starts as zero.  If the  the count	*/
/* value is non-zero, a call to any of the above allocation/deallocation calls will	*/
/* write a descriptive message to standard output.  This output can be analyzed by	*/
/* the program "mem" in order to find memory leaks and other anomalies.			*/

EXTERN(void, turn_off_mem_debug, (void));
/* Takes no parameters.  Decrements the memory debugging count.  See the function	*/
/* turn_on_mem_debug().									*/

EXTERN(void, set_zap_mem, (void));
/* Takes no parameters.  Sets a flag which will zap all memory allocated by get_mem() or*/
/* reget_mem() and all memory freed by reget_mem() or free_mem().  The zapped memory	*/
/* will be written by bytes containing 0x0F.  This call must be made before any of the	*/
/* memory allocation/deallocation calls below.  Memory zapping is helpful for finding	*/
/* uses of uninitialized memory and uses of freed memory.				*/


	/* Memory Statistics */

extern	int total_rn_memory_in_use;	/* amount of memory in use		*/
/* This read-only variable indicates the running amount of memory currently allocated	*/
/* by get_mem(), reget_mem(), and free_mem().  The value is incremented or decremented	*/
/* based on the actual size of the block of memory rather than the size of the requests.*/

extern	int other_memory_in_use;	/* non-rn memory in use			*/
/* This read-only variable indicates the estimated amount of memory allocated outside	*/
/* of the rn memory usage.  (This estimate is made at the first rn allocation and	*/
/* does not change.)									*/

EXTERN(int, rn_size_of_brkable_region, (void));
/* Takes no parameters.  Returns an estimate of the amount of additional free memory	*/
/* that can be obtained by using sbrk().						*/

EXTERN(char*, rn_memory_use_string, (void));
/* Takes no parameters.  Returns a pointer to a static string which contains a short	*/
/* description of the current memory situation.  The string looks like: "10K in use,	*/
/* 5+100K left".  The first number is the Rn memory usage, the second is an estimate	*/
/* of the amount of memory available without moving the break point, and the third	*/
/* is the estimate ofthe additional free memory available beyond the current break	*/
/* point.										*/

#endif
