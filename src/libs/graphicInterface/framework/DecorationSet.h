/* $Id: DecorationSet.h,v 1.2 1997/03/11 14:32:37 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/DecorationSet.h					*/
/*									*/
/*	DecorationSet -- Decoration composed of subdecorations		*/
/*	Last edited: October 13, 1993 at 5:44 pm			*/
/*									*/
/************************************************************************/




#ifndef DecorationSet_h
#define DecorationSet_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/Decoration.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/***********************/
/* DecorationSet class */
/***********************/




struct DecorationSet_Repr_struct;
class  CTextView;
class  LineEditor;




class DecorationSet: public Decoration
{
public:
  DecorationSet_Repr_struct * DecorationSet_repr;

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* instance creation */
  static DecorationSet *	Create(void);

/* initialization */
  META_DEF(DecorationSet)
				DecorationSet(void);
  virtual void			Init(void);
  virtual void			Destroy(void);
				~DecorationSet(void);

/* view attachment */
  virtual void			AttachView(CTextView * view);

/* drawing */
  virtual void			ColorizeLine(int c_linenum,
                                             int marginWidth,
                                             TextString &text,
                                             TextData &data);
  virtual void			Draw(void);
  virtual Rectangle		BBox(void);

/* access to subdecorations */
  virtual void			Add(Decoration * d);
  virtual void			SetEmpty(void);
  virtual Boolean		IsEmpty(void);
      
};




#endif /* __cplusplus */

#endif /* not DecorationSet_h */
