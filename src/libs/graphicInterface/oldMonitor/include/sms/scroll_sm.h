/* $Id: scroll_sm.h,v 1.4 1997/03/11 14:33:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	scroll_sm.h							*/
/*									*/
/*	scroll -- screen module to control scrolling of another pane	*/
/*	Last edited: July 3, 1990 at 12:35 pm				*/
/*									*/
/************************************************************************/


#ifndef scroll_sm_h
#define scroll_sm_h

#ifdef sm_h
typedef Pane * ScrollBar;
#else
typedef Generic ScrollBar;
#endif



/************************/
/*  Constants		*/
/************************/

#define		SB_WIDTH		 15
#define		SB_MIN_LENGTH		 95

#define		SB_HORIZONTAL		  0
#define		SB_VERTICAL		  1




/************************/
/*  Screen module	*/
/************************/

EXTERN(short, sm_scroll_get_index, (void));




/************************/
/*  Scrolling control	*/
/************************/

EXTERN(void, sm_scroll_activate, (ScrollBar sb, Boolean activeNow));
typedef FUNCTION_POINTER(void, sm_scroll_scrollee_callback,
 (Generic scrollee, ScrollBar sb, int direction, int curVal));
EXTERN(void, sm_scroll_scrollee, (ScrollBar sb, Generic scrollee,
 sm_scroll_scrollee_callback scrollProc));




/************************/
/*  Scroll position	*/
/************************/


EXTERN(void, sm_scroll_get, (ScrollBar sb, int *minVal, int *maxVal,
 int *curVal));
EXTERN(void, sm_scroll_set, (ScrollBar sb, int minVal, int maxVal, int curVal,
 Boolean touch));
EXTERN(void, sm_scroll_set_step, (ScrollBar sb, int slowStep, int fastStep));

#endif
