/* $Id: FortView.h,v 1.2 1997/03/11 14:32:43 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/FortView.h						*/
/*									*/
/*	FortView -- View of Fortran source code				*/
/*	Last edited: October 15, 1993 at 1:02 pm			*/
/*									*/
/************************************************************************/




#ifndef FortView_h
#define FortView_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/CTextView.h>

#include <libs/frontEnd/fortTree/FortTree.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/************************/
/* Appearance constants */
/************************/


#define FortView_Font    DEF_FONT_ID




/******************/
/* FortView class */
/******************/




struct FortView_Repr_struct;
class  CFortEditor;




class FortView: public CTextView
{
public:

  FortView_Repr_struct * FortView_repr;

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* instance creation */
  static FortView *		Create(Context context,
                                       DB_FP * session_fd,
                                       CFortEditor * editor,
                                       Point initScrollPos,
                                       int font);

/* initialization */
  META_DEF(FortView)
				FortView(Context context,
                                         DB_FP * session_fd,
                                         CFortEditor * editor,
                                         Point initScrollPos,
                                         int font);
  virtual void			Init(void);
  virtual void			Destroy(void);
  virtual			~FortView(void);

/* change notification */
  virtual void			NoteChange(Object * ob, int kind, void * change);

/* window layout */
  virtual void			GetSizePrefs(Point &minSize, Point &defSize);
  virtual void			InitPanes(void);
  virtual void			GetLineNumPosition(int &start, int &width);

/* current loop */
  virtual void			GetCurrentLoop(FortTreeNode &node, Rectangle &bbox);

};




#endif /* __cplusplus */

#endif /* not FortView_h */
