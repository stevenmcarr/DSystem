/* $Id: MarginDecoration.h,v 1.2 1997/03/11 14:32:48 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/MarginDecoration.h					*/
/*									*/
/*	MarginDecoration -- Addition to a CTextView's left margin	*/
/*	Last edited: August 20, 1993 at 5:43 pm				*/
/*									*/
/************************************************************************/




#ifndef MarginDecoration_h
#define MarginDecoration_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/Decoration.h>

#include <libs/graphicInterface/framework/Text.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/**************************/
/* MarginDecoration class */
/**************************/




struct MarginDecoration_Repr_struct;
class  CTextView;




class MarginDecoration: public Decoration
{
public:

  MarginDecoration_Repr_struct * MarginDecoration_repr;


public:

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(MarginDecoration)
				MarginDecoration(void);
				~MarginDecoration(void);

/* margin text */
  virtual int			Width(void);
  virtual void			SetStart(int start);
  virtual int			Start(void);
  virtual void			GetMarginText(int c_linenum,
                                              TextChar * textstring_tc_ptr,
                                              ColorPair * textdata_chars);

/* input */
  virtual void Clicked(int c_linenum, int char1);

};




#endif /* __cplusplus */

#endif /* not MarginDecoration_h */
