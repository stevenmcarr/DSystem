/* $Id: FortArrow.h,v 1.2 1997/03/11 14:32:41 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/FortArrow.h						*/
/*									*/
/*	FortArrow -- Colored arrow between AST nodes in a FortView	*/
/*	Last edited: October 13, 1993 at 6:04 pm			*/
/*									*/
/************************************************************************/




#ifndef FortArrow_h
#define FortArrow_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/Arrow.h>
#include <libs/graphicInterface/framework/Text.h>

#include <libs/frontEnd/fortTree/FortTree.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus





/*******************/
/* FortArrow class */
/*******************/




struct FortArrow_Repr_struct;




class FortArrow: public Arrow
{
public:

  FortArrow_Repr_struct * FortArrow_repr;


public:

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* instance creation */
  static FortArrow *		Create(FortTreeNode src, FortTreeNode sink, ColorPair colors);

/* initialization */
  META_DEF(FortArrow)
				FortArrow(void);
  virtual void			Init(FortTreeNode src, FortTreeNode sink, ColorPair colors);
  virtual void			Destroy(void);
				~FortArrow(void);


protected:

/* appearance */
  virtual void			getSrcEnd(Rectangle &bbox, ColorPair &colors);
  virtual void			getSinkEnd(Rectangle &bbox, ColorPair &colors);
  virtual void			getEndFromNode(FortTreeNode node, Rectangle &bbox, ColorPair &colors);
    
};




#endif /* __cplusplus */

#endif /* not FortArrow_h */
