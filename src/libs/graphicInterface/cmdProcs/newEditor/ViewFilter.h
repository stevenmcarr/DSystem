/* $Id: ViewFilter.h,v 1.5 1997/03/11 14:30:37 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/ViewFilter.h						*/
/*									*/
/*	ViewFilter -- determines how text is displayed			*/
/*	Last edited: October 2, 1992 at 4:36 pm				*/
/*									*/
/************************************************************************/




#ifndef ViewFilter_h
#define ViewFilter_h


typedef Generic ViewFilter;

/* Line data */

typedef struct{int x[20];} VF_MaxData;


/* Filter func */

typedef FUNCTION_POINTER(void, vf_FilterFunc, (Generic filterOb,
 Boolean countOnly, Generic contents, int lineNum, int *sublineNum,
 TextString *text, VF_MaxData *data));


/* Elision styles */

#define vf_NOTHING	0
#define vf_ELLIPSIS	1
#define vf_DIVLINE	2




/************************/
/*  Initialization	*/
/************************/

EXTERN(void, vf_Init, (void));
EXTERN(void, vf_Fini, (void));

EXTERN(ViewFilter, vf_Open, (Context context, DB_FP *fp,
                                     Generic filterOb, vf_FilterFunc filterProc));
EXTERN(void, vf_Close, (ViewFilter vf));




/************************/
/*  Filter specs	*/
/************************/

EXTERN(void, vf_SetSpecs, (ViewFilter vf, Generic filterOb, vf_FilterFunc filterProc));
EXTERN(void, vf_SetElision, (ViewFilter vf, int elision));
EXTERN(void, vf_SetContents, (ViewFilter vf, Generic contents, TV_Methods *methods));




/************************/
/*  Change notification	*/
/************************/

typedef FUNCTION_POINTER(void, vf_NotifyFunc,
 (Generic ob, int kind, Boolean autoScroll, int first, int last, int delta));
EXTERN(void, vf_Notify,
 (ViewFilter vf, Generic ob, vf_NotifyFunc notifyProc));
EXTERN(void, vf_NoteChange, (ViewFilter vf, int kind, Boolean autoScroll, int c_first, int c_last, int c_delta));




/************************/
/*  Filtering		*/
/************************/

EXTERN(void, vf_GetDocSize, (ViewFilter vf, Point *size));
EXTERN(void, vf_GetLine, (ViewFilter vf, int v_lineNum, TextString *text, TV_Data *data));
EXTERN(void, vf_GetSelection, (ViewFilter vf, int *v_lineNum, int *v_sel1, int *v_sel2));
EXTERN(void, vf_SetSelection, (ViewFilter vf, int v_lineNum, int v_sel1, int v_sel2));

EXTERN(Boolean, vf_ContentsToView, (ViewFilter vf, int c_lineNum, int *v_lineNum));
EXTERN(Boolean, vf_ViewToContents, (ViewFilter vf, int v_lineNum, int *c_lineNum));


#endif
