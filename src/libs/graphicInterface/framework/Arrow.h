/* $Id: Arrow.h,v 1.2 1997/03/11 14:32:26 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/Arrow.h						*/
/*									*/
/*	Arrow -- Colored arrow in a CTextView display			*/
/*	Last edited: October 13, 1993 at 12:03 am			*/
/*									*/
/************************************************************************/




#ifndef Arrow_h
#define Arrow_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/Decoration.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/***************/
/* Arrow class */
/***************/




struct Arrow_Repr_struct;




class Arrow: public Decoration
{
public:

  Arrow_Repr_struct * Arrow_repr;


public:

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* instance creation */
  static Arrow *		Create(void);

/* initialization */
  META_DEF(Arrow)
				Arrow(void);
  virtual void			Init(void);
  virtual void			PostInit(void);
  virtual void			Destroy(void);
				~Arrow(void);

/* view attachment */
  virtual void			AttachView(CTextView * view);

/* drawing */
  virtual void			ColorizeLine(int c_linenum, TextString &text, TextData &data);
  virtual void			Draw(void);
  virtual Rectangle		BBox(void);


protected:
  
/* appearance */
  virtual void			calcAppearance(void);
  virtual void			getSrcEnd(Rectangle &bbox, ColorPair &colors);
  virtual void			getSinkEnd(Rectangle &bbox, ColorPair &colors);
  virtual void			getShaft(int &width, LineStyle * &style, Color &color);
  virtual void			calcShaftDisplay(void);

/* drawing */
  virtual void			colorizeEnd(int c_linenum, Boolean src, TextData &td);
    
};




#endif /* __cplusplus */

#endif /* not Arrow_h */
