/* $Id: LineNumDecoration.h,v 1.2 1997/03/11 14:32:45 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/LineNumDecoration.h					*/
/*									*/
/*	LineNumDecoration -- Ordinal line numbers in left margin	*/
/*	Last edited: October 13, 1993 at 6:20 pm			*/
/*									*/
/************************************************************************/




#ifndef LineNumDecoration_h
#define LineNumDecoration_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/MarginDecoration.h>

#include <libs/graphicInterface/framework/Text.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/**************************/
/* LineNumDecoration class */
/**************************/




struct LineNumDecoration_Repr_struct;
class  CTextView;




class LineNumDecoration: public MarginDecoration
{
public:

  LineNumDecoration_Repr_struct * LineNumDecoration_repr;


public:

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* instance creation */
  static LineNumDecoration *	Create(void);

/* initialization */
  META_DEF(LineNumDecoration)
				LineNumDecoration(void);
  void				Init(void);
				~LineNumDecoration(void);

/* options */
  virtual void			SetOptions(int digits, int extra, unsigned char style);
  virtual void			GetOptions(int &digits, int &extra, unsigned char &style);

/* margin text */
  virtual int			Width(void);
  virtual void			GetMarginText(int c_linenum,
                                              TextChar * textstring_tc_ptr,
                                              ColorPair * textdata_chars);

/* input */
  virtual void			Clicked(int c_linenum, int char1);

};




#endif /* __cplusplus */

#endif /* not LineNumDecoration_h */
