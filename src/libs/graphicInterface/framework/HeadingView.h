/* $Id: HeadingView.h,v 1.2 1997/03/11 14:32:43 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/HeadingView.h						*/
/*									*/
/*	HeadingView -- Column headings for ColumnViews			*/
/*	Last edited: October 13, 1993 at 6:15 pm			*/
/*									*/
/************************************************************************/




#ifndef HeadingView_h
#define HeadingView_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/CTextView.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/*********************/
/* HeadingView class */
/*********************/




struct HeadingView_Repr_struct;
class ColumnEditor;




class HeadingView: public CTextView
{
public:

  HeadingView_Repr_struct * HeadingView_repr;


public:

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(HeadingView)
				HeadingView(Context context,
                                            DB_FP * session_fd,
                                            ColumnEditor * editor,
                                            int numCols,
                                            int colWidths[],
                                            char * headings[],
                                            int contentsFont,
                                            int headingFont);
  virtual			~HeadingView(void);

/* window layout */
  virtual void			InitPanes(void);


protected:

/* access to contents */
  virtual int			numLines(void);
  virtual int			maxLineWidth(void);
  virtual void			getLine(int k, TextString &ts, TextData &td);

/* access to selection */
  virtual void			getSelection(int &line1, int &char1, int &line2, int &char2);
  virtual void			setSelection(int line1, int char1, int line2, int char2);

/* filtering */
  virtual CViewFilter *		makeDefaultFilter(void);

};




#endif /* __cplusplus */

#endif /* not HeadingView_h */
