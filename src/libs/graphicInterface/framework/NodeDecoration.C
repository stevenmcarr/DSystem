/* $Id: NodeDecoration.C,v 1.2 1997/03/11 14:32:49 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/NodeDecoration.C					*/
/*									*/
/*	NodeDecoration -- Coloring of certain nodes in a FortView	*/
/*	Last edited: November 12, 1993 at 4:34 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/framework/NodeDecoration.h>

#include <libs/graphicInterface/framework/CTextView.h>
#include <libs/graphicInterface/framework/FortView.h>
#include <libs/graphicInterface/framework/LineEditor.h>

#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/support/arrays/FlexibleArray.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* NodeDecoration object */

typedef struct NodeDecoration_Repr_struct
  {
    /* creation parameters */
      FortTextTree	ftt;
      Boolean		lineNumOnly;

    /* details of display context */
      int		marginWidth;
      int		lineNumFirst;
      int		lineNumLast;

    /* cache of line decorations */
      Flex *		decorationCache;

  } NodeDecoration_Repr;




#define R(ob)		(ob->NodeDecoration_repr)
#define INHERITED	Decoration






/*************************/
/*  Miscellaneous	 */
/*************************/




/* default colors for decoration */

static ColorPair nd_nullColors = {NULL_COLOR, NULL_COLOR};






/*************************/
/*  Forward declarations */
/*************************/




static Boolean overwrite(ColorPair existing, ColorPair decoration, ColorPair &result);







/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/






/*************************/
/*  Class initialization */
/*************************/




void NodeDecoration::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(Decoration);
    REQUIRE_INIT(FortView);
}




void NodeDecoration::FiniClass(void)
{
  /* nothing */
}






/**********************/
/*  Instance creation */
/**********************/




NodeDecoration * NodeDecoration::Create(FortTextTree ftt,
                                        Boolean lineNumOnly)
{
  NodeDecoration * d;
  
  d = new NodeDecoration;
  d->NodeDecoration::Init(ftt, lineNumOnly);
  d->NodeDecoration::PostInit();
  return d;
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(NodeDecoration)




NodeDecoration::NodeDecoration(void)
              : Decoration()
{
  /* allocate instance's private data */
    this->NodeDecoration_repr =
        (NodeDecoration_Repr *) get_mem(sizeof(NodeDecoration_Repr),
                                        "NodeDecoration instance");
}




void NodeDecoration::Init(FortTextTree ftt,
                          Boolean lineNumOnly)
{
  this->INHERITED::Init();  

  /* save creation parameters */
    R(this)->ftt         = ftt;
    R(this)->lineNumOnly = lineNumOnly;
    
  /* initialize cache */
    R(this)->decorationCache = flex_create(sizeof(TextData *));
}




void NodeDecoration::Destroy(void)
{
  /* retract change notification request made at AttachView time */
    this->GetView()->GetEditor()->Notify(this, false);

  this->purgeLineDecorations(0, INFINITY);
  flex_destroy(R(this)->decorationCache);
  
  this->INHERITED::Destroy();
}




NodeDecoration::~NodeDecoration()
{
  free_mem((void*) this->NodeDecoration_repr);
}






/********************/
/*  View attachment */
/********************/




void NodeDecoration::AttachView(CTextView * view)
{
  int start, width;

  /* request notification when viewed document changes */
    view->GetEditor()->Notify(this, true);

  /* get details of surrounding display context */
    ((FortView *) view)->GetLineNumPosition(start, width);
    R(this)->lineNumFirst = start;
    R(this)->lineNumLast  = start + width - 1;
}






/************/
/*  Drawing */
/************/




void NodeDecoration::ColorizeLine(int c_linenum,
                                  int marginWidth,
                                  TextString &text,
                                  TextData &data)
{
  TextData * decData;
  int num, k;
  ColorPair result;
  
  /* save margin width for recursive submethods */
    R(this)->marginWidth = marginWidth;

  decData = this->getLineDecorations(c_linenum);

  num = min(text.num_tc, MAX_TEXT_CHARS);
  for( k = 0;  k < num;  k++ )
    if( overwrite(data.chars[k], decData->chars[k], result) )
      { data.chars[k] = result;
        text.tc_ptr[k].style |= STYLE_BOLD;
      }
}




Rectangle NodeDecoration::BBox(void)
{
  return MaximumRect;
}






/***********************/
/* Change notification */
/***********************/




void NodeDecoration::NoteChange(Object * ob, int kind, void * change)
{
  NoteChangeTrace trace(this, ob, kind, change);
  Rectangle oldBBox, changedBBox;
  LineEditorChange lch;

  switch( kind )
    {
      case CHANGE_DOCUMENT:
        lch = * ((LineEditorChange *) change);
        this->purgeLineDecorations(0, INFINITY);
        this->INHERITED::NoteChange(ob, kind, change);
        break;
        
      default:
        this->INHERITED::NoteChange(ob, kind, change);
    }
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/






/*******************/
/* Node processing */
/*******************/




void NodeDecoration::beginDecorating(void)
{
  /* nothing */
}




void NodeDecoration::endDecorating(void)
{
  /* nothing */
}




Boolean NodeDecoration::isDecoratedNode(FortTreeNode node)
{
  SUBCLASS_RESPONSIBILITY("NodeDecoration::isDecoratedNode");
  return false;
}




void NodeDecoration::getNodeDecoration(FortTreeNode node, ColorPair &colors)
{
  SUBCLASS_RESPONSIBILITY("NodeDecoration::getNodeDecoration");
}






/**********************/
/* Decoration caching */
/**********************/




TextData * NodeDecoration::getLineDecorations(int c_linenum)
{
  Flex * cache = R(this)->decorationCache;
  int last = flex_length(cache) - 1;
  Boolean found;
  TextData * decData;
  int k, dummy;
  FortTreeNode node;
  TextData * nullData = nil;
  
  /* see whether the line's decorations are already cached */
    if( c_linenum > last )
      found = false;
    else
      { flex_get_buffer(cache, c_linenum, 1, (char *) &decData);
        found = BOOL( decData != nil );
      }
  
  /* compute the decorations if necessary */
    if( ! found )
      { /* prepare storage for new line decorations */
          decData = new TextData;
          decData->multiColored = true;
          for( k = 0;  k < MAX_TEXT_CHARS;  k++ )
            decData->chars[k] = nd_nullColors;
        
        /* traverse the line's subtree computing decorations */
          ftt_GetLineInfo(R(this)->ftt, c_linenum, &node, &dummy);
          this->beginDecorating();
          this->calcLineDecorations(c_linenum, node, true, decData);
          this->endDecorating();
        
        /* add the line's decorations to the cache */
          /* ensure that relevant flexarray position exists */
            for( k = last+1;  k <= c_linenum;  k++ )
              flex_insert_one(cache, k, (char *) &nullData);
          flex_set_one(cache, c_linenum, (char *) &decData);
      }
  
  return decData;
}




void NodeDecoration::calcLineDecorations(int c_linenum,
                                         FortTreeNode node,
                                         Boolean topLevel,
                                         TextData * decData)
{
  int line1, char1, line2, char2, k;
  ColorPair colors;
  FortTreeNode subnode;
  
  if( node == AST_NIL )  return;
  
  /* find text range for 'node' and return if not on original line */
    ftt_NodeToText(R(this)->ftt, node, &line1, &char1, &line2, &char2);
    if( !(topLevel || (line1 == c_linenum && line2 == c_linenum)) )
      return;

  if( this->isDecoratedNode(node) )
    { /* determine what colors to use */
        colors = nd_nullColors;
        this->getNodeDecoration(node, colors);
        
      /* determine what part of the line to color */
        if( topLevel )
          { if( R(this)->lineNumOnly )
              { char1 = R(this)->lineNumFirst;
                char2 = R(this)->lineNumLast;
              }
            else
              { char1 = R(this)->lineNumFirst;
                char2 = INFINITY;
              }
           }
        else
          { char1 = char1 + R(this)->marginWidth;
            char2 = char2 + R(this)->marginWidth;
          }
      /* do the appropriate coloring */
        char2 = min(char2, MAX_TEXT_CHARS-1);
        for( k = char1;  k <= char2;  k++ )
          decData->chars[k] = colors;
    }
  else
    if( is_list_node(node) )
      for( subnode = list_first(node);  subnode != nil;  subnode = list_next(subnode) )
        this->calcLineDecorations(c_linenum, subnode, false, decData);
    else
      for( k = 1;  k <= ast_get_son_count(node);  k++ )
        { subnode = ast_get_son_n(node, k);
          this->calcLineDecorations(c_linenum, subnode, false, decData);
        }
}




void NodeDecoration::purgeLineDecorations(int c_linenum1, int c_linenum2)
{
  Flex * cache = R(this)->decorationCache;
  int k, last;
  TextData * decData;
  
  last = min(c_linenum2, flex_length(cache)-1);
  for( k = c_linenum1; k <= last;  k++ )
    { flex_get_buffer(cache, k, 1, (char *) &decData);
      if( decData != nil )
        { delete decData;
          decData = nil;
          flex_set_one(cache, k, (char *) &decData);
        }
    }
}




static
Boolean overwrite(ColorPair existing, ColorPair decoration, ColorPair &result)
{
  result.foreground = (decoration.foreground != NULL_COLOR ? decoration.foreground
                                                           : existing.foreground);
  result.background = (decoration.background != NULL_COLOR ? decoration.background
                                                           : existing.background);

  return BOOL(decoration.foreground != NULL_COLOR || decoration.background != NULL_COLOR);
}
