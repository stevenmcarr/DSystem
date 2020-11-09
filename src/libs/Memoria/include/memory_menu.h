/* $Id: memory_menu.h,v 1.16 1998/07/07 19:39:52 carr Exp $ */
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
#define F2I_ANALYSIS            9
#define LI_STATS                10
#define UJ_STATS                11
#define SR_STATS                12
#define STATS			13
#define LDST		        14
#define DEAD			15    
#define PARTITION_UNROLL	16    
#define DEP_STATS               17
#define MEM_SIZE		18    
#define AST_DUMP                19 
#define DG_DUMP                 20 
#define VECTORIZE               21 /* MUST BE LAST! */

#define ARENAS 1

#endif
