/* $Id: Utility.h,v 1.5 1997/03/11 14:30:36 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/Utility.h						*/
/*									*/
/*	Utility -- Commonly useful stuff for New Fortran Editor		*/
/*	Last edited: September 23, 1992 at 2:11 pm			*/
/*									*/
/************************************************************************/




#ifndef Utility_h
#define Utility_h

#include <libs/support/misc/general.h>
#include <libs/graphicInterface/support/graphics/point.h>
#include <libs/graphicInterface/support/graphics/rect.h>


/* no instances of this module */




/************************/
/*  Initialization	*/
/************************/

EXTERN(void, ut_Init, (void));
EXTERN(void, ut_Fini, (void));




/************************/
/*  Messages		*/
/************************/

EXTERN(void, notImplemented, (char * what));
EXTERN(void, CHECK, (int what));


/************************/
/*  Rectangles		*/
/************************/

EXTERN(void, setRect, (Rectangle *r, int left, int top, int right, 
	int bottom));
EXTERN(void, setRectSize, (Rectangle *r, int left, int top, int width, 
	int height));


/************************/
/*  Mouse		*/
/************************/

EXTERN(Boolean, stillDown, (Point *mousePt));




/************************/
/*  Color		*/
/************************/

extern Color black_color;
extern Color white_color;

EXTERN(Color, color, (char * name));


#endif
