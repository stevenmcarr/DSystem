/* $Id: memory_menu.h,v 1.14 1997/04/09 18:40:48 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef memory_menu_h
#define memory_menu_h

#define NO_SELECT		-1
#define INTERCHANGE             0
#define	SCALAR_REP 		1
#define	UNROLL_AND_JAM  	2
#define UNROLL_SCALAR           3
#define LI_SCALAR               4
#define LI_UNROLL               5
#define MEM_ALL			6 
#define PREFETCH                7
#define ANNOTATE                8
#define CACHE_ANALYSIS          9
#define LI_STATS                10
#define UJ_STATS                11
#define SR_STATS                12
#define STATS			13
#define LDST		        14
#define DEAD			15    
#define PARTITION_UNROLL	16    
#define MEM_SIZE		17    /* MUST BE LAST! */

#define ARENAS 1

#endif
