/* $Id: View.h,v 1.6 1997/03/11 14:32:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*      framework/View.h                                                */
/*                                                                      */
/*      View -- Abstract class for all tilable views                    */
/*	Last edited: October 13, 1993 at 11:00 pm			*/
/*                                                                      */
/************************************************************************/




#ifndef View_h
#define View_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/DBObject.h>
#include <libs/graphicInterface/framework/Editor.h>



/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/**************/
/* View class */
/**************/




struct View_Repr_struct;
class  Editor;
class  CViewFilter;




class View: public DBObject
{
public:

  View_Repr_struct * View_repr;


public:

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(View)
				View(Context context,
                                     DB_FP * session_fp,
                                     Editor * editor);
  virtual			~View(void);
  virtual Editor *		GetEditor(void);

/* window layout */
  virtual void			GetSizePrefs(Point &minSize, Point &defSize);
  virtual Generic		GetTiling(Boolean init, Point size);
  virtual void			InitPanes(void);

/* filtering */
  virtual CViewFilter *		GetFilter(void);
  virtual void			SetFilter(CViewFilter * filter);

/* input handling */
  virtual Boolean		SelectionEvent(Generic generator, Point info);

};




#endif /* __cplusplus */

#endif /* not View_h */
