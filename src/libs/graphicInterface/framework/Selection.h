/* $Id: Selection.h,v 1.4 1997/03/11 14:32:52 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/Selection.h						*/
/*									*/
/*	Selection -- Abstract class for all selection specifiers	*/
/*	Last edited: October 13, 1993 at 10:40 pm			*/
/*									*/
/************************************************************************/




#ifndef Selection_h
#define Selection_h


#include <libs/graphicInterface/framework/framework.h>



/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/*******************/
/* Selection class */
/*******************/




struct Selection_Repr_struct;




class Selection
{
public:

    Selection_Repr_struct * Selection_repr;


public:

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(Selection)

/* access to selection */
  virtual Boolean		Empty(void);

};




#endif /* __cplusplus */

#endif /* not Selection_h */
