/* $Id: ft2text.h,v 1.7 1997/03/11 14:30:01 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef ft2text_h
#define ft2text_h

#include <stdio.h>

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef FortTree_h
#include <libs/frontEnd/fortTree/FortTree.h>
#endif

#ifndef FortTextTree_h
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#endif

typedef enum { Sequent=0, IBM=1, Cray=2, None=3 } Company; 

typedef enum { disp, f77, none } Fort_exp;

EXTERN(int, ft_export,
       (Context                context,
	FortTree               ft,
	FortTextTree           ftt,
	FILE                  *outf,
	Company                dialect));

EXTERN(int, ft2text,
       (Context                context,
	FortTree               ft, 
	FortTextTree           ftt,  
	FILE                  *outf,
	Boolean                enddo2continue_p, 
	Boolean                dataRaceInstrumentation_p, 
	Company                dialect));

#endif /* ft2text_h */
