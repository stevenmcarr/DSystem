/* $Id: memory_menu.h,v 1.4 1994/06/13 16:13:32 carr Exp $ */

#ifndef memory_menu_h
#define memory_menu_h

#define NO_SELECT		-1
#define INTERCHANGE             0
#define	SCALAR_REP 		1
#define	UNROLL_AND_JAM  	2
#define UNROLL_SCALAR           3
#define LI_SCALAR               4
#define LI_UNROLL               5
#define MEM_ALL			5 
#define PREFETCH                7
#define ANNOTATE                8
#define LI_FUSION		9
#define CACHE_ANALYSIS          10
#define LI_STATS                11
#define UJ_STATS                12
#define SR_STATS                13
#define MEM_SIZE		14    /* MUST BE LAST! */

#define ARENAS 1

#endif
