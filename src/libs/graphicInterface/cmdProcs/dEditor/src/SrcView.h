/* $Id: SrcView.h,v 1.2 1997/03/11 14:30:27 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/src/SrcView.h						*/
/*									*/
/*	SrcView -- View of DedDocument's Source				*/
/*	Last edited: October 14, 1993 at 3:43 pm			*/
/*									*/
/************************************************************************/




#ifndef SrcView_h
#define SrcView_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/FortView.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/*****************/
/* SrcView class */
/*****************/




struct SrcView_Repr_struct;
class  SrcEditor;


class SrcView: public FortView
{
public:

  SrcView_Repr_struct * SrcView_repr;

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* instance creation */
  static SrcView *		Create(Context context,
                                       DB_FP * session_fd,
                                       SrcEditor * editor,
                                       Point initScrollPos,
                                      int font);

/* initialization */
  META_DEF(SrcView)
				SrcView(Context context,
                                        DB_FP * session_fd,
                                        SrcEditor * editor,
                                        Point initScrollPos,
                                       int font);
  virtual void			Init(void);
  virtual void			Destroy(void);
  virtual			~SrcView(void);

/* change notification */
  virtual void			NoteChange(Object * ob, int kind, void * change);

/* window layout */
  virtual void			InitPanes(void);


protected:
  
/* dependence display */
  virtual void			calcDepDecorations(void);
  virtual void			autoScroll(void);

};




#endif /* __cplusplus */

#endif /* not SrcView_h */
