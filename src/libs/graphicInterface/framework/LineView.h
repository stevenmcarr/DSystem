/* $Id: LineView.h,v 1.5 1997/03/11 14:32:46 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/LineView.h						*/
/*									*/
/*	LineView -- View of a LineEditor				*/
/*	Last edited: October 13, 1993, at 6:26 pm			*/
/*									*/
/************************************************************************/




#ifndef LineView_h
#define LineView_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/CTextView.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/******************/
/* LineView class */
/******************/




struct LineView_Repr_struct;
class  LineEditor;




class LineView: public CTextView
{
public:

  LineView_Repr_struct * LineView_repr;


public:

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(LineView)
				LineView(Context context,
                                         DB_FP * session_fp,
                                         LineEditor * editor,
                                         Point initScrollPos,
                                         int font);
  virtual			~LineView(void);


public:    /* should be 'protected', cf. g++ bug */

/* access to contents */
  virtual int			numLines(void);
  virtual int			maxLineWidth(void);
  virtual void			getLine(int k, TextString &ts, TextData &td);

/* access to selection */
  virtual void			getSelection(int &line1, int &char1, int &line2, int &char2);
  virtual void			setSelection(int line1, int char1, int line2, int char2);

};




#endif /* __cplusplus */

#endif /* not LineView_h */
