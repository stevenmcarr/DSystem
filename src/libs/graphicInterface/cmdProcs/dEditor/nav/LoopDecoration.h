/* $Id: LoopDecoration.h,v 1.2 1997/03/11 14:30:19 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/nav/LoopDecoration.h					*/
/*									*/
/*	LoopDecoration -- Loop-cost coloring for Ded nav pane		*/
/*	Last edited: October 14, 1993 at 1:20 pm			*/
/*									*/
/************************************************************************/




#ifndef LoopDecoration_h
#define LoopDecoration_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/NodeDecoration.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/************************/
/* LoopDecoration class */
/************************/




struct LoopDecoration_Repr_struct;
class  DedDocument;




class LoopDecoration: public NodeDecoration
{
public:

  LoopDecoration_Repr_struct * LoopDecoration_repr;

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* instance creation */
  static LoopDecoration *	Create(DedDocument * doc);

/* initialization */
  META_DEF(LoopDecoration)
				LoopDecoration(void);
  virtual void			Init(DedDocument * doc);
  virtual void			Destroy(void);
				~LoopDecoration(void);


protected:

/* node processing */
  virtual Boolean		isDecoratedNode(FortTreeNode node);
  virtual void			getNodeDecoration(FortTreeNode node, ColorPair &colors);
      
};




#endif /* __cplusplus */

#endif /* not LoopDecoration_h */
