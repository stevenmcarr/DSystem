/* $Id: SplitView.h,v 1.4 1997/03/11 14:32:54 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/SplitView.h						*/
/*									*/
/*	SplitView -- View split into one or more subviews		*/
/*	Last edited: November 6, 1993 at 10:16 pm			*/
/*									*/
/************************************************************************/




#ifndef SplitView_h
#define SplitView_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/View.h>
#include <libs/graphicInterface/framework/Editor.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/*******************/
/* SplitView class */
/*******************/




struct SplitView_Repr_struct;
class Editor;




class SplitView: public View
{
public:

    SplitView_Repr_struct * SplitView_repr;

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(SplitView)
				SplitView(Context context,
                                          DB_FP * session_fp,
                                          Editor * editor,
                                          Boolean showFilterNames);
  virtual			~SplitView(void);

/* change notification */
  virtual void			NoteChange(Object * ob, int kind, void * change);

/* window layout */
  virtual void			GetSizePrefs(Point &minSize, Point &defSize);
  virtual Generic		GetTiling(Boolean init, Point size);
  virtual void			InitPanes(void);

/* access to subviews */
  virtual int			NumViews(void);
  virtual void			GetView(int k, View * &v, Editor * &e, char * &caption);
  virtual void			AddView(View * v, Editor * e, char * caption);
  virtual void			InsertView(int k, View * v, Editor * e, char * caption);
  virtual void			DeleteView(int k);
  virtual void			DeleteAllViews(void);
  virtual void			SetViewVisible(int k, Boolean visible);

/* current view */
  virtual int			CurrentView(void);
  virtual void			SetCurrentView(int k);
  virtual void			GetCurrentView(View * &v, Editor * &e, char * &caption);

/* input handling */
  virtual Boolean		SelectionEvent(Generic generator, Point info);

protected:

/* database */
  virtual void			isnew(Context context);
  virtual void			read(DB_FP * fp, DB_FP * session_fp);
  virtual void			write(DB_FP * fp, DB_FP * session_fp);

/* captions */
  virtual void			updateCaptions(void);

};




#endif /* __cplusplus */

#endif /* not SplitView_h */
