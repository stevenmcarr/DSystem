/* $Id: LoopIconDecoration.C,v 1.2 1997/03/11 14:32:47 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/LoopIconDecoration.C					*/
/*									*/
/*	LoopIconDecoration -- Loop icons in margin, headers colorized	*/
/*	Last edited: October 13, 1993 at 6:28 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/framework/LoopIconDecoration.h>

#include <libs/graphicInterface/framework/CTextView.h>
#include <libs/graphicInterface/framework/CFortEditor.h>
#include <libs/graphicInterface/framework/LineNumDecoration.h>

/* Ned stuff needed here */
#include <libs/graphicInterface/cmdProcs/newEditor/ned.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/textTree/TextTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/frontEnd/ast/groups.h>
#define tt_SEL_CHANGED    0






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* Details of current loop */

struct LoopInfo
  {
    Rectangle			bbox;		/* in contents "glyph" coords */
    FortTreeNode		node;

  };




/* LoopIconDecoration object */

typedef struct LoopIconDecoration_Repr_struct
  {
    /* creation arguments */
      LineNumDecoration *	linenumDec;

    /* appearance */
      Color			foreColor;
      Boolean			colorizeWholeLoop;

    /* display state */
      LoopInfo			curLoop;

  } LoopIconDecoration_Repr;


#define R(ob)			(ob->LoopIconDecoration_repr)


#define INHERITED		MarginDecoration






/*************************/
/*  Miscellaneous	 */
/*************************/




/* display parameters */

#define ICON_WIDTH	4	/* must be at least 4 */

#define CURLOOP_ICON    ">*"
#define LOOP_ICON       " *"
#define NO_ICON         "  "

static Color		ld_loopFgColor;






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




void LoopIconDecoration::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(MarginDecoration);
    REQUIRE_INIT(CTextView);
    REQUIRE_INIT(CFortEditor);
    REQUIRE_INIT(LineNumDecoration);

  /* initialize display parameters */
    ld_loopFgColor = color("ped2.currentLoop");    /* TEMPORARY */
}




void LoopIconDecoration::FiniClass(void)
{
  /* nothing */
}






/**********************/
/*  Instance creation */
/**********************/




LoopIconDecoration * LoopIconDecoration::Create(LineNumDecoration * linenumDec)
{
  LoopIconDecoration * dec;
  
  dec= new LoopIconDecoration;
  dec->LoopIconDecoration::Init(linenumDec);
  
  return dec;
}




/****************************/
/*  Instance initialization */
/****************************/




META_IMP(LoopIconDecoration)




LoopIconDecoration::LoopIconDecoration(void)
          : MarginDecoration()
{
  /* allocate instance's private data */
    this->LoopIconDecoration_repr =
        (LoopIconDecoration_Repr *) get_mem(sizeof(LoopIconDecoration_Repr),
                                            "LoopIconDecoration instance");
}




void LoopIconDecoration::Init(LineNumDecoration * linenumDec)
{  
  this->INHERITED::Init();

  /* save creation arguments */
    R(this)->linenumDec = linenumDec;

  /* initialize appearance */
    R(this)->foreColor         = ld_loopFgColor;
    R(this)->colorizeWholeLoop = false;
    
  /* initialize viewing state */
    this->calcCurLoop(AST_NIL);
}




LoopIconDecoration::~LoopIconDecoration()
{
  free_mem((void*) this->LoopIconDecoration_repr);
}






/***********/
/* Options */
/***********/



      
void LoopIconDecoration::SetOptions(Color foreColor, Boolean colorizeWholeLoop)
{
  R(this)->foreColor         = foreColor;
  R(this)->colorizeWholeLoop = colorizeWholeLoop;
}




void LoopIconDecoration::GetOptions(Color &foreColor, Boolean &colorizeWholeLoop)
{
  foreColor         = R(this)->foreColor;
  colorizeWholeLoop = R(this)->colorizeWholeLoop;
}






/************/
/*  Drawing */
/************/




void LoopIconDecoration::ColorizeLine(int c_linenum,
                                      int marginWidth,
                                      TextString &text,
                                      TextData &data)
{
  int first, last, k;
  Boolean wholeLine;

  if( R(this)->curLoop.node == AST_NIL )  return;

  if( R(this)->curLoop.bbox.ul.y <= c_linenum && c_linenum <= R(this)->curLoop.bbox.lr.y )
    { Text_MultiColoredData(data);

      wholeLine = BOOL( R(this)->colorizeWholeLoop              ||
                        c_linenum == R(this)->curLoop.bbox.ul.y ||
                        c_linenum == R(this)->curLoop.bbox.lr.y );

      if( wholeLine )
        { first = 0;
          last  = text.num_tc-1;
        }
      else
        if( R(this)->linenumDec != nil )
          { first = R(this)->linenumDec->Start();
            last  = first + R(this)->linenumDec->Width();
          }
        else
          { first =  0;
            last  = -1;
          }

      last = min(last, MAX_TEXT_CHARS);
      for( k = first; k <= last;  k++ )
        data.chars[k].foreground = R(this)->foreColor;
    }
}




Rectangle LoopIconDecoration::BBox(void)
{
  return R(this)->curLoop.bbox;
}






/********/
/* Text */
/********/



      
int LoopIconDecoration::Width(void)
{
  return ICON_WIDTH;
}




void LoopIconDecoration::GetMarginText(int c_linenum,
                                       TextChar * textstring_tc_ptr,
                                       ColorPair * textdata_chars)
{
  FortTreeNode node;
  int bracket;
  char * icon;
  int k;

  /* compute appropriate icon */
    if( this->isLoopHeader(c_linenum, node, bracket) )
      icon = R(this)->curLoop.node != AST_NIL  &&  R(this)->curLoop.bbox.ul.y == c_linenum
                  ? CURLOOP_ICON : LOOP_ICON;
    else
      icon = NO_ICON;

  /* insert the loop icon */
    textstring_tc_ptr[0] = makeTextChar(icon[0], STYLE_BOLD);
    textstring_tc_ptr[1] = makeTextChar(icon[1], STYLE_BOLD);

  /* insert the extra space */
    for( k = 2;  k < ICON_WIDTH;  k++ )
      textstring_tc_ptr[k] = makeTextChar(' ', STYLE_NORMAL);

  /* supply alternate feedback for current loop's range if necessary */
    if( monochrome_screen )
      if( R(this)->curLoop.bbox.ul.y <= c_linenum && c_linenum <= R(this)->curLoop.bbox.lr.y )
        textstring_tc_ptr[ICON_WIDTH-1 - 1] = makeTextChar('|', STYLE_NORMAL);
}






/***********************/
/* Change notification */
/***********************/




void LoopIconDecoration::NoteChange(Object * ob, int kind, void * change)
{
  NoteChangeTrace trace(this, ob, kind, change);
  Rectangle oldBBox, changedBBox;
  LineEditorChange lch;

  switch( kind )
    {
      case CHANGE_LOOP:
        oldBBox = R(this)->curLoop.bbox;
        this->calcCurLoop((FortTreeNode) change);
        changedBBox = unionRect(oldBBox, R(this)->curLoop.bbox);

        /* ought to have a View::Update method for this */
          lch.kind       = tt_SEL_CHANGED;
          lch.autoScroll = false;
          lch.first      = changedBBox.ul.y;
          lch.last       = changedBBox.lr.y;
          lch.delta      = 0;
          this->GetView()->NoteChange(ob, CHANGE_SELECTION, (void *) &lch);

        break;
        
      default:
        this->INHERITED::NoteChange(ob, kind, change);
    }
}






/*********/
/* Input */
/********/



      
void LoopIconDecoration::Clicked(int c_linenum, int char1)
{
  CFortEditor * editor = (CFortEditor *) this->GetEditor();
  FortTreeNode node;
  int dummy;
  
  if( this->isLoopHeader(c_linenum, node, dummy) )
    editor->SetCurrentLoop(node);
}







/****************/
/* Current loop */
/****************/




void LoopIconDecoration::GetCurrentLoop(FortTreeNode &node, Rectangle &bbox)
{
  node = R(this)->curLoop.node;
  bbox = R(this)->curLoop.bbox;
}







/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




void LoopIconDecoration::calcCurLoop(FortTreeNode loopNode)
{
  CFortEditor * editor = (CFortEditor *) this->GetEditor();
  CTextView * view = this->GetView();
  int line1, line2, dummy;
  LineEditorChange lch;

  R(this)->curLoop.node = loopNode;
  if( loopNode != AST_NIL )
    { editor->NodeToText(loopNode, line1, dummy, line2, dummy);
      R(this)->curLoop.bbox = makeRect(makePoint(0,        line1),
                                       makePoint(INFINITY, line2));
    }

  /* update the decorated view if any */
    if( view != nil )
      { lch.kind       = tt_SEL_CHANGED;
        lch.autoScroll = false;
        lch.first      = R(this)->curLoop.bbox.ul.y;
        lch.last       = R(this)->curLoop.bbox.lr.y;
        lch.delta      = 0;
        view->NoteChange(this, CHANGE_SELECTION, (void *) &lch);
      }
}




Boolean LoopIconDecoration::isLoopHeader(int c_linenum, FortTreeNode &node, int &bracket)
{
  CFortEditor * editor = (CFortEditor *) this->GetEditor();

  editor->GetLineInfo(c_linenum, node, bracket);
  return BOOL( is_loop_stmt(node) && bracket == ftt_OPEN );
}
