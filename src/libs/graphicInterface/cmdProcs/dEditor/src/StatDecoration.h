/* $Id: StatDecoration.h,v 1.2 1997/03/11 14:30:28 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/src/StatDecoration.h					*/
/*									*/
/*	StatDecoration -- Statement-cost coloring for Ded src pane	*/
/*	Last edited: October 14, 1993 at 3:43 pm			*/
/*									*/
/************************************************************************/




#ifndef StatDecoration_h
#define StatDecoration_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/NodeDecoration.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/*************************/
/* StatDecoration class */
/*************************/




struct StatDecoration_Repr_struct;
class  DedDocument;




class StatDecoration: public NodeDecoration
{
public:

  StatDecoration_Repr_struct * StatDecoration_repr;

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* instance creation */
  static StatDecoration *	Create(DedDocument * doc);

/* initialization */
  META_DEF(StatDecoration)
				StatDecoration(void);
  virtual void			Init(DedDocument * doc);
  virtual void			Destroy(void);
				~StatDecoration(void);


protected:

/* node processing */
  virtual Boolean		isDecoratedNode(FortTreeNode node);
  virtual void			getNodeDecoration(FortTreeNode node, ColorPair &colors);
      
};




#endif /* __cplusplus */

#endif /* not StatDecoration_h */
