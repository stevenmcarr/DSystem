/* $Id: NodeDecoration.h,v 1.2 1997/03/11 14:32:50 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/NodeDecoration.h					*/
/*									*/
/*	NodeDecoration -- Coloring of certain nodes in a FortView	*/
/*	Last edited: October 16, 1993 at 8:40 pm			*/
/*									*/
/************************************************************************/




#ifndef NodeDecoration_h
#define NodeDecoration_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/Decoration.h>

#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/frontEnd/fortTree/FortTree.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/************************/
/* NodeDecoration class */
/************************/




struct NodeDecoration_Repr_struct;




class NodeDecoration: public Decoration
{
public:

  NodeDecoration_Repr_struct * NodeDecoration_repr;

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* instance creation */
  static NodeDecoration *	Create(FortTextTree ftt, Boolean lineNumOnly);

/* initialization */
  META_DEF(NodeDecoration)
				NodeDecoration(void);
  virtual void			Init(FortTextTree ftt, Boolean lineNumOnly);
  virtual void			Destroy(void);
				~NodeDecoration(void);

/* view attachment */
  virtual void			AttachView(CTextView * view);

/* drawing */
  virtual void			ColorizeLine(int c_linenum,
                                             int marginWidth,
                                             TextString &text,
                                             TextData &data);
  virtual Rectangle		BBox(void);

/* change notification */
  void				NoteChange(Object * ob, int kind, void * change);


protected:

/* node processing */
  virtual void			beginDecorating(void);
  virtual void			endDecorating(void);
  virtual Boolean		isDecoratedNode(FortTreeNode node);
  virtual void			getNodeDecoration(FortTreeNode node, ColorPair &colors);
                                                  
/* decoration caching */
  virtual TextData *		getLineDecorations(int c_linenum);
  virtual void			calcLineDecorations(int c_linenum,
                                                    FortTreeNode node,
                                                    Boolean topLevel,
                                                    TextData * decData);
  virtual void			purgeLineDecorations(int c_linenum1, int c_linenum2);
  
};




#endif /* __cplusplus */

#endif /* not NodeDecoration_h */
