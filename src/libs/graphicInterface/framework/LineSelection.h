/* $Id: LineSelection.h,v 1.4 1997/03/11 14:32:46 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/LineSelection.h					*/
/*									*/
/*	LineSelection -- Abstract class for all text-lines selections	*/
/*	Last edited: October 13, 1990 at 6:23 pm			*/
/*									*/
/************************************************************************/




#ifndef LineSelection_h
#define LineSelection_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/Selection.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/***********************/
/* LineSelection class */
/***********************/




struct LineSelection_Repr_struct;




class LineSelection: public Selection
{
public:

  LineSelection_Repr_struct * LineSelection_repr;


public:

/* instance variables */
  int line1;
  int char1;
  int line2;
  int char2;

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(LineSelection)
				LineSelection(void);
				LineSelection(int line1, int char1, int line2, int char2);

/* access to selection */
  virtual Boolean		Empty(void);

};






#endif /* __cplusplus */

#endif /* not LineSelection_h */
