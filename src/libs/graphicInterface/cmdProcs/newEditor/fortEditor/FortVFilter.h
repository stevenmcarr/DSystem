/* $Id: FortVFilter.h,v 1.6 1997/03/11 14:30:52 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/FortEditor/FortVFilter.h					*/
/*									*/
/*	FortVFilter -- determines how Fortran source is displayed	*/
/*	Last edited: October 17, 1991 at 3:03 pm    			*/
/*									*/
/************************************************************************/




#ifndef FortVFilter_h
#define FortVFilter_h


typedef Generic FortVFilterSpec;
typedef Generic FortVFilter;


/* elision styles -- must be same as in ViewFilter.h */

#define ff_NOTHING     0
#define ff_ELLIPSIS    1
#define ff_DIVLINE     2


typedef struct
  {
    int dummy;
  }  FF_LineData;




/************************/
/*  Initialization	*/
/************************/

EXTERN(void, ff_Init, (void));
EXTERN(void, ff_Fini, (void));

EXTERN(FortVFilterSpec, ffs_Open, (Context     context, DB_FP *fp,
 /* FortEditor */ Generic ed));
EXTERN(void, ffs_Close, (FortVFilterSpec ffs));
EXTERN(void, ffs_Save, (FortVFilterSpec ffs, Context     context,
 DB_FP *fp));

EXTERN(FortVFilter, ff_Open, (Context     context, DB_FP *fp,
 /* FortEditor */ Generic ed, FortVFilterSpec ffs));
EXTERN(void, ff_Close, (FortVFilter ff));




/************************/
/*  Filter specs	*/
/************************/

EXTERN(void, ffs_GetName, (FortVFilterSpec ffs, char **name));
EXTERN(void, ffs_SetName, (FortVFilterSpec ffs, char *name));
EXTERN(void, ffs_GetDefinition, (FortVFilterSpec ffs, char **def,
 Boolean *concealed, Boolean *errors));
EXTERN(Boolean, ffs_SetDefinition, (FortVFilterSpec ffs, char *def,
 Boolean concealed, Boolean errors, char **msg));

EXTERN(Boolean, ffs_InUse, (FortVFilterSpec ffs));
typedef FUNCTION_POINTER(void, ffs_CustomFunc,
 (Generic customOb, Boolean countOnly, /* FortEditor */ Generic ed,
 int line, int *subline, TextString *text, FF_LineData *data));
EXTERN(void, ffs_Customize,
 (FortVFilterSpec ffs, Generic customOb, ffs_CustomFunc customProc));




/************************/
/*  Filter settings	*/
/************************/

EXTERN(char *, ff_GetName, (FortVFilter ff, Boolean withError));

EXTERN(void, ff_SetShowErrors, (FortVFilter ff, Boolean show));
EXTERN(void, ff_GetShowErrors, (FortVFilter ff, Boolean *show));

EXTERN(void, ff_SetElision, (FortVFilter ff, int elision));
EXTERN(void, ff_GetElision, (FortVFilter ff, int *elision));



/************************/
/*  Change notification	*/
/************************/

EXTERN(void, ff_NoteChange, (FortVFilter ff, int kind,
 Boolean autoScroll, FortTreeNode node, int first, int last, int delta));




/************************/
/*  Filtering		*/
/************************/

EXTERN(/* ViewFilter */ Generic, ff_ViewFilter, (FortVFilter ff));




#endif
