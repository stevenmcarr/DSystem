/* $Id: NavView.h,v 1.2 1997/03/11 14:30:22 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/nav/NavView.h						*/
/*									*/
/*	NavView -- Overview of DedDocument's source code		*/
/*	Last edited: October 14, 1993 at 3:43 pm			*/
/*									*/
/************************************************************************/




#ifndef NavView_h
#define NavView_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/FortView.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/*****************/
/* NavView class */
/*****************/




struct NavView_Repr_struct;
class  NavEditor;




class NavView: public FortView
{
public:

  NavView_Repr_struct * NavView_repr;

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* instance creation */
  static NavView *		Create(Context context,
                                       DB_FP * session_fd,
                                       NavEditor * editor,
                                       Point initScrollPos);
/* initialization */
  META_DEF(NavView)

				NavView(Context context,
                                        DB_FP * session_fd,
                                        NavEditor * editor,
                                        Point initScrollPos);
  virtual void			Init(void);
  virtual void			Destroy(void);
  virtual			~NavView(void);

/* change notification */
  virtual void			NoteChange(Object * ob, int kind, void * change);

/* window layout */
  virtual void			GetSizePrefs(Point &minSize, Point &defSize);
  virtual void			InitPanes(void);

};




#endif /* __cplusplus */

#endif /* not NavView_h */
