/* $Id: ColumnView.h,v 1.2 1997/03/11 14:32:34 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/ColumnView.h						*/
/*									*/
/*	ColumnView -- View of text lines in labeled columns		*/
/*	Last edited: October 13, 1993 at 12:39 pm			*/
/*									*/
/************************************************************************/




#ifndef ColumnView_h
#define ColumnView_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/LineView.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/********************/
/* ColumnView class */
/********************/




struct ColumnView_Repr_struct;
class  ColumnEditor;




class ColumnView: public LineView
{
public:

  ColumnView_Repr_struct * ColumnView_repr;


public:

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(ColumnView)
				ColumnView(Context context,
                                           DB_FP * session_fd,
                                           ColumnEditor * editor,
                                           Point initScrollPos,
                                           int contentsFont,
                                           int headingFont,
                                           int numCols,
                                           int colWidths[],
                                           char * headings[],
                                           int numDivLines,
                                           int divLineCols[]);
  virtual			~ColumnView(void);

/* window layout */
  virtual void			GetSizePrefs(Point &minSize, Point &defSize);
  virtual Generic		GetTiling(Boolean init, Point size);
  virtual void			InitPanes(void);

/* change notification */
  virtual void			NoteChange(Object * ob, int kind, void * change);

};






#endif /* __cplusplus */

#endif /* not ColumnView_h */
