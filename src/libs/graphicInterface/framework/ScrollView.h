/* $Id: ScrollView.h,v 1.4 1997/03/11 14:32:52 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/ScrollView.h						*/
/*									*/
/*	ScrollView -- Scroll bars surrounding another view		*/
/*	Last edited: October 16, 1993 at 10:15 pm			*/
/*									*/
/************************************************************************/




#ifndef ScrollView_h
#define ScrollView_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/View.h>
#include <libs/graphicInterface/framework/CTextView.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/********************/
/* ScrollView class */
/********************/




struct ScrollView_Repr_struct;
class  Editor;




class ScrollView: public View
{
public:

    ScrollView_Repr_struct * ScrollView_repr;


public:

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(ScrollView)
				ScrollView(CTextView * contents,
				           Boolean hscrollWanted,
				           Boolean vscrollWanted);
  virtual			~ScrollView(void);

/* change notification */
  virtual void			NoteChange(Object * ob, int kind, void * change);

/* window layout */
  virtual void			GetSizePrefs(Point &minSize, Point &defSize);
  virtual Generic		GetTiling(Boolean init, Point size);
  virtual void			InitPanes(void);

/* scrolling */
  virtual Point			GetScroll(void);
  virtual void			SetScroll(Point scrollPos);
  virtual void			ScrollBy(Point delta);
  virtual void			EnsureVisible(Point pt);
  virtual void			EnsureSelVisible(void);

/* filtering */
  virtual CViewFilter *		GetFilter(void);
  virtual void			SetFilter(CViewFilter * filter);

/* input handling */
  virtual Boolean		SelectionEvent(Generic generator, Point info);

};




#endif /* __cplusplus */

#endif /* not ScrollView_h */
