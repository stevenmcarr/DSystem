/* $Id: RefDecoration.h,v 1.2 1997/03/11 14:30:25 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/src/RefDecoration.h					*/
/*									*/
/*	RefDecoration -- Reference-cost coloring for Ded src pane	*/
/*	Last edited: October 14, 1993 at 3:43 pm			*/
/*									*/
/************************************************************************/




#ifndef RefDecoration_h
#define RefDecoration_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/NodeDecoration.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/************************/
/* RefDecoration class */
/************************/




struct RefDecoration_Repr_struct;
class  DedDocument;




class RefDecoration: public NodeDecoration
{
public:

  RefDecoration_Repr_struct * RefDecoration_repr;

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* instance creation */
  static RefDecoration *	Create(DedDocument * doc);

/* initialization */
  META_DEF(RefDecoration)
				RefDecoration(void);
  virtual void			Init(DedDocument * doc);
  virtual void			Destroy(void);
				~RefDecoration(void);


protected:

/* node processing */
  virtual Boolean		isDecoratedNode(FortTreeNode node);
  virtual void			getNodeDecoration(FortTreeNode node, ColorPair &colors);
      
};




#endif /* __cplusplus */

#endif /* not RefDecoration_h */
