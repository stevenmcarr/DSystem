/* $Id: Decoration.h,v 1.2 1997/03/11 14:32:36 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/Decoration.h						*/
/*									*/
/*	Decoration -- Addition to a CTextView display			*/
/*	Last edited: October 13, 1993 at 5:43 pm			*/
/*									*/
/************************************************************************/




#ifndef Decoration_h
#define Decoration_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/CObject.h>
#include <libs/graphicInterface/framework/Text.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/********************/
/* Decoration class */
/********************/




struct Decoration_Repr_struct;
class  CTextView;
class  LineEditor;



class Decoration: public Object
{
public:
  Decoration_Repr_struct * Decoration_repr;


public:

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(Decoration)
				Decoration(void);
  virtual void			Init(void);
  virtual void			Destroy(void);
				~Decoration(void);

/* view attachment */
  virtual void			AttachView(CTextView * view);
  virtual CTextView *		GetView(void);
  virtual LineEditor *		GetEditor(void);

/* drawing */
  virtual void			ColorizeLine(int c_linenum,
                                             int marginWidth,
                                             TextString &text,
                                             TextData &data);
  virtual void			Draw(void);
  virtual Rectangle		BBox(void);
      
};




#endif /* __cplusplus */

#endif /* not Decoration_h */
