/* $Id: FortArrow.C,v 1.2 1997/03/11 14:32:41 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/FortArrow.C						*/
/*									*/
/*	FortArrow -- Colored arrow between AST nodes in a FortView	*/
/*	Last edited: October 13, 1993 at 6:04 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/framework/FortArrow.h>

#include <libs/graphicInterface/framework/CFortEditor.h>

#include <libs/frontEnd/fortTextTree/FortTextTree.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* FortArrow object */

typedef struct FortArrow_Repr_struct
  {
    /* requested appearance */
      FortTreeNode	srcNode;
      FortTreeNode	sinkNode;
      ColorPair		colors;

  } FortArrow_Repr;


#define R(ob)		(ob->FortArrow_repr)

#define INHERITED	Arrow






/*************************/
/*  Forward declarations */
/*************************/




/* none */






/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/






/*************************/
/*  Class initialization */
/*************************/




void FortArrow::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(Arrow);
    REQUIRE_INIT(CFortEditor);
}




void FortArrow::FiniClass(void)
{
  /* nothing */
}






/**********************/
/*  Instance creation */
/**********************/




FortArrow * FortArrow::Create(FortTreeNode src, FortTreeNode sink, ColorPair colors)
{
  FortArrow * a;
  
  a = new FortArrow;
  a->FortArrow::Init(src, sink, colors);
  a->FortArrow::PostInit();
  return a;
}




/****************************/
/*  Instance initialization */
/****************************/




META_IMP(FortArrow)




FortArrow::FortArrow(void)
          : Arrow()
{
  /* allocate instance's private data */
    this->FortArrow_repr = (FortArrow_Repr *) get_mem(sizeof(FortArrow_Repr), "FortArrow instance");
}




void FortArrow::Init(FortTreeNode src, FortTreeNode sink, ColorPair colors)
{
  this->INHERITED::Init();

  /* save creation arguments */
    R(this)->srcNode  = src;
    R(this)->sinkNode = sink;
    R(this)->colors   = colors;
}




void FortArrow::Destroy(void)
{
  this->INHERITED::Destroy();
}




FortArrow::~FortArrow()
{
  free_mem((void*) this->FortArrow_repr);
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/***************/
/*  Appearance */
/***************/




void FortArrow::getSrcEnd(Rectangle &bbox, ColorPair &colors)
{
  this->getEndFromNode(R(this)->srcNode, bbox, colors);
}




void FortArrow::getSinkEnd(Rectangle &bbox, ColorPair &colors)
{
  this->getEndFromNode(R(this)->sinkNode, bbox, colors);
}




void FortArrow::getEndFromNode(FortTreeNode node, Rectangle &bbox, ColorPair &colors)
{
  CFortEditor * editor = (CFortEditor *) this->GetEditor();  /* ASSERT: editor is a FortEditor */
  FortTextTree ftt   = editor->GetFortTextTree();
  FortTree ft        = editor->GetFortTree();
  int line1, char1, line2, char2;
  Boolean isLineRange;

  /* set the end's bounding box */
    if( node == nil )
      bbox = EmptyRect;
    else
      { ft_AstSelect(ft);
        if( is_subscript(tree_out(node)) )  node = tree_out(node);

        isLineRange = ftt_NodeToText(ftt, node, &line1, &char1, &line2, &char2);
        if( isLineRange )
          { line2 = line1;
            char1 = ftt_GetLineIndent(ftt, line1);
            char2 = ftt_GetLineLength(ftt, line1) - 1;
          }
        setRect(&bbox, char1, line1, char2, line2);
      }
      
  /* set the end's colors */
    colors = R(this)->colors;
}
