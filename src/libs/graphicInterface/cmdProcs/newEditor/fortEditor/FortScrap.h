/* $Id: FortScrap.h,v 1.4 1997/03/11 14:30:52 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/FortEditor/FortScrap.h					*/
/*									*/
/*	FortScrap -- fragments of Fortran for the public scrap		*/
/*	Last edited: October 21, 1987 at 7:02 pm			*/
/*									*/
/************************************************************************/




#ifndef FortScrap_h
#define FortScrap_h


/* typedef Generic FortScrap; defined in FortEditor.h */




/************************/
/*  Initialization	*/
/************************/

EXTERN(void,		fsc_Init,(void));
EXTERN(void,		fsc_Fini,(void));

EXTERN(FortScrap,	fsc_CreateSmall,(FortTree ft, TextString text, 
                                         int start, int count));
EXTERN(FortScrap,	fsc_CreateLarge,(FortTree ft));
EXTERN(void,		fsc_AddLarge,(FortScrap fsc, TextString text, 
                                      int conceal));
EXTERN(void,		fsc_Destroy,(FortScrap fsc));



/************************/
/*  Scrap access	*/
/************************/



extern SC_Type		fsc_Type;

EXTERN(Boolean,		fsc_IsEmpty,(FortScrap fsc));
EXTERN(Boolean,		fsc_IsSmall,(FortScrap fsc));
EXTERN(TextString,	fsc_GetSmall,(FortScrap fsc));
EXTERN(int,		fsc_GetLargeLength,(FortScrap fsc));
EXTERN(void,		fsc_GetLarge,(FortScrap fsc, int k, TextString *text, 
                                      int *conceal));
EXTERN(FortScrap,	fsc_Copy,(FortScrap fsc));




/************************/
/*  Scrap protocol	*/
/************************/

/* ... */




#endif
