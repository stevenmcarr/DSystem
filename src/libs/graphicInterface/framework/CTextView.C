/* $Id: CTextView.C,v 1.8 1997/03/11 14:32:31 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*      framework/CTextView.C                                           */
/*                                                                      */
/*      CTextView -- Wrapper for Ned's TextView.c                       */
/*	Last edited: November 12, 1993 at 3:10 pm			*/
/*                                                                      */
/************************************************************************/




#include <libs/graphicInterface/framework/CTextView.h>


#include <libs/graphicInterface/oldMonitor/include/mon/sm.h>
#include <libs/graphicInterface/oldMonitor/include/mon/cp.h>
#include <libs/graphicInterface/oldMonitor/include/sms/text_sm.h>
#include <libs/support/arrays/FlexibleArray.h>

#include <libs/graphicInterface/framework/Text.h>
#include <libs/graphicInterface/framework/LineEditor.h>
#include <libs/graphicInterface/framework/CViewFilter.h>
#include <libs/graphicInterface/framework/Decoration.h>
#include <libs/graphicInterface/framework/MarginDecoration.h>

#include <libs/graphicInterface/framework/UserFilterDef.h>
#include <libs/graphicInterface/framework/UserFilter.h>


/* Ned stuff needed here */
#include <libs/graphicInterface/cmdProcs/newEditor/TextView.h>
#include <libs/graphicInterface/cmdProcs/newEditor/ViewFilter.h>
#define tt_SEL_CHANGED    0






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* CTextView object */

#define MAX_CLIP_DEPTH    4

typedef struct CTextView_Repr_struct
  {
    /* creation parameters */
      LineEditor *	editor;
      Point		initScrollPos;
      int		font;

    /* decorations */
      Flex *		decorations;
      Flex *		marginDecorations;
      int		totalMarginWidth;

    /* viewing state */
      CViewFilter *	filter;

    /* drawing state */
      Rectangle		clipRect;
      Rectangle		clipStack[MAX_CLIP_DEPTH];
      int		clipDepth;
      int		lineWidth;
      LineStyle *	lineStyle;
      Color		foreColor;
      Color		backColor;

    /* selecting state */
      Boolean		oneLine;
      Boolean		noElidedLine;

    /* Ned components */
      TextView		tv;
      Pane *		textPane;
      ViewFilter	nedFilter;

  } CTextView_Repr;


#define R(ob)		(ob->CTextView_repr)

#define INHERITED	View






/*****************************************/
/*  Customizing methods for Ned TextView */
/*****************************************/




static void getDocSize(CTextView * ctv, Point *size);

static Boolean getLine(CTextView * ctv,
                       int linenum,
                       TextString *text,
                       void *data);

static void getSelection(CTextView * ctv, int *linenum, int *sel1, int *sel2);

static void setSelection(CTextView * ctv, int linenum, int sel1, int sel2);

static void customRepaint(CTextView * ctv, Pane * textPane,
                          ViewFilter nedFilter,
                          Rectangle * viewRect,
                          int pane_line1, int pane_line2);




static TV_Methods CTextView_methods =
  {
    (tv_GetDocSizeFunc)getDocSize,
    (tv_GetLineFunc)getLine,
    (tv_GetSelectionFunc)getSelection,
    (tv_SetSelectionFunc)setSelection
  };






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




void CTextView::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(View);
    REQUIRE_INIT(Text);
    REQUIRE_INIT(LineEditor);
    REQUIRE_INIT(CViewFilter);
    REQUIRE_INIT(Decoration);
    REQUIRE_INIT(MarginDecoration);
    REQUIRE_INIT(UserFilterDef);
    REQUIRE_INIT(UserFilter);
}




void CTextView::FiniClass(void)
{
  /* nothing */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(CTextView)




CTextView::CTextView(Context context,
                     DB_FP * session_fp,
                     Editor * editor,
                     Point initScrollPos,
                     int font)
           : View(context, session_fp, editor)
{
  /* allocate instance's private data */
    this->CTextView_repr = (CTextView_Repr *) get_mem(sizeof(CTextView_Repr),
                                                      "CTextView instance");

  /* save creation parameters */
    R(this)->editor        = (LineEditor *) editor;   /* ASSERT: 'editor' is a 'LineEditor' */
    R(this)->initScrollPos = initScrollPos;
    R(this)->font          = font;
    
  /* create subparts */
    R(this)->decorations       = flex_create(sizeof(Decoration *));
    R(this)->marginDecorations = flex_create(sizeof(Decoration *));
    R(this)->totalMarginWidth = 0;
    
  /* initialize viewing state */
    R(this)->filter    = nil;

  /* initialize drawing state */
    R(this)->clipRect  = MaximumRect;
    R(this)->clipDepth = 0;

  /* initialize selecting state */
    R(this)->oneLine      = false;
    R(this)->noElidedLine = false;
}




CTextView::~CTextView(void)
{
  /* destroy the subparts */
    flex_destroy(R(this)->decorations);
    flex_destroy(R(this)->marginDecorations);
    /* 'tv' is destroyed implicitly */

  free_mem((void*) this->CTextView_repr);
}




void CTextView::CustomizeRepainting(tv_CustomRepaintFunc repaintProc)
{
  tv_CustomizeRepainting(R(this)->tv, (Generic) this, repaintProc);
}






/************/
/* Database */
/************/




void CTextView::Save(Context context, DB_FP * session_fp)
{
  /* ... */
}






/************************/
/*  Change notification */
/************************/




void CTextView::NoteChange(Object * ob, int kind, void * change)
{
  NoteChangeTrace trace(this, ob, kind, change);
  LineEditorChange lch;

  /* notify the Ned textview */
    if( kind == CHANGE_SELECTION || kind == CHANGE_DOCUMENT )
      { lch = * ((LineEditorChange *) change);
        vf_NoteChange(R(this)->nedFilter,
                      lch.kind, lch.autoScroll, lch.first, lch.last, lch.delta);
      }
    else if( kind == CHANGE_NAVIGATE_TO )
      this->EnsureVisible(* ((Point *) change));
}






/******************/
/*  Window layout */
/******************/




Generic CTextView::GetTiling(Boolean init, Point size)
{
  if( init )  R(this)->tv = (TextView) tv_ScreenModuleIndex();
  return cp_td_pane((Pane**) &R(this)->tv, size);
}




void CTextView::InitPanes(void)
{
  CViewFilter * filter;

  filter = this->makeDefaultFilter();

  /* initialize the TextView with corresponding Ned filter */
    R(this)->nedFilter = filter->getNedViewFilter();
    tv_PaneInit(R(this)->tv, (Generic) this, & CTextView_methods,
                R(this)->nedFilter, R(this)->initScrollPos, R(this)->font);
    R(this)->textPane = (Pane *) tv_getTextPane(R(this)->tv);

  /* install the filter officially */
    this->SetFilter(filter);

  this->CustomizeRepainting((tv_CustomRepaintFunc) customRepaint);
}




Point CTextView::TextSize(void)
{
  return sm_text_size(R(this)->textPane);
}




void CTextView::SetScrollBars(Generic hscroll, Generic vscroll)
{
  tv_ScrollBars(R(this)->tv, (Pane *) hscroll, (Pane *) vscroll);
}







/**************/
/*  Scrolling */
/**************/




Point CTextView::GetScroll(void)
{
  return tv_GetScroll(R(this)->tv);
}




void CTextView::SetScroll(Point scrollPos)
{
  tv_SetScroll(R(this)->tv, scrollPos);
}




void CTextView::ScrollBy(Point delta)
{
  tv_ScrollBy(R(this)->tv, delta);
}




void CTextView::EnsureVisible(Point pt)
{
  tv_EnsureContentsVisible(R(this)->tv, pt, false);
}




void CTextView::EnsureSelVisible(void)
{
  NOT_IMPLEMENTED("CTextView::EnsureSelVisible");
}






/*************/
/* Filtering */
/*************/




void CTextView::SetFilter(CViewFilter * filter)
{
  Rectangle infinity;
  LineEditorChange lch;

  R(this)->nedFilter = filter->getNedViewFilter();
  setRect(&infinity, 0, 0, 999999, 999999);
  tv_SetFilter(R(this)->tv, R(this)->nedFilter, false, infinity);

  R(this)->filter = filter;
  this->INHERITED::SetFilter(filter);
}




CViewFilter * CTextView::makeDefaultFilter(void)
{
  UserFilterDef * def;

  this->GetEditor()->GetFilterDefByName("normal", def);
  return def->MakeFilter( (void *) this->GetEditor()->getContents() );
}






/********************/
/* Mapping linenums */
/********************/




Boolean CTextView::ContentsLineElided(int c_lineNum)
{
  if( R(this)->filter != nil )
    return R(this)->filter->ContentsLineElided(c_lineNum);
  else
    return true;
}




int CTextView::ContentsToViewLinenum(int c_lineNum)
{
  if( R(this)->filter != nil )
    return R(this)->filter->ContentsToViewLinenum(c_lineNum);
  else
    return c_lineNum;
}




int CTextView::ContentsToScreenLinenum(int c_lineNum)
{
  return this->ViewToScreenLinenum(this->ContentsToViewLinenum(c_lineNum));
}




int CTextView::ViewToContentsLinenum(int v_lineNum)
{
  if( R(this)->filter != nil )
    return R(this)->filter->ViewToContentsLinenum(v_lineNum);
  else
    return v_lineNum;
}




int CTextView::ViewToScreenLinenum(int v_lineNum)
{
  Point scrollPos = this->GetScroll();
  
  return v_lineNum - scrollPos.y;
}




int CTextView::ScreenToContentsLinenum(int s_lineNum)
{
  return this->ViewToContentsLinenum(this->ScreenToViewLinenum(s_lineNum));
}




int CTextView::ScreenToViewLinenum(int s_lineNum)
{
  Point scrollPos = this->GetScroll();
  
  return s_lineNum + scrollPos.y;
}






/******************/
/* Mapping points */
/******************/




Point CTextView::ContentsToViewPoint(Point c_pt)
{
  Point v_pt;
  
  v_pt.x = c_pt.x;
  v_pt.y = this->ContentsToViewLinenum(c_pt.y);
  return v_pt;
}




Point CTextView::ContentsToScreenPoint(Point c_pt)
{
  return this->ViewToScreenPoint(this->ContentsToViewPoint(c_pt));
}




Point CTextView::ViewToContentsPoint(Point v_pt)
{
  Point c_pt;
  
  c_pt.x = v_pt.x;
  c_pt.y = this->ViewToContentsLinenum(v_pt.y);
  return c_pt;
}




Point CTextView::ViewToScreenPoint(Point v_pt)
{
  Point scrollPos = this->GetScroll();
  Point s_pt;
  
  s_pt.x = (v_pt.x - scrollPos.x) + R(this)->totalMarginWidth;
  s_pt.y = this->ViewToScreenLinenum(v_pt.y);
  return s_pt;
}




Point CTextView::ScreenToContentsPoint(Point s_pt)
{
  Point c_pt;
  
  c_pt.x = s_pt.x - R(this)->totalMarginWidth;
  c_pt.y = this->ScreenToContentsLinenum(s_pt.y);
  return c_pt;
}




Point CTextView::ScreenToViewPoint(Point s_pt)
{
  Point scrollPos = this->GetScroll();
  Point v_pt;
  
  v_pt.x = (s_pt.x + scrollPos.x) - R(this)->totalMarginWidth;
  v_pt.y = this->ScreenToViewLinenum(s_pt.y);
  return v_pt;
}






/**********************/
/* Mapping rectangles */
/**********************/




Rectangle CTextView::ContentsToViewRect(Rectangle c_rect)
{
  Rectangle v_rect;
  
  v_rect.ul = this->ContentsToViewPoint(c_rect.ul);
  v_rect.lr = this->ContentsToViewPoint(c_rect.lr);
  return v_rect;
}




Rectangle CTextView::ContentsToScreenRect(Rectangle c_rect)
{
  Rectangle s_rect;
  
  s_rect.ul = this->ContentsToScreenPoint(c_rect.ul);
  s_rect.lr = this->ContentsToScreenPoint(c_rect.lr);
  return s_rect;
}




Rectangle CTextView::ViewToContentsRect(Rectangle v_rect)
{
  Rectangle c_rect;
  
  c_rect.ul = this->ViewToContentsPoint(v_rect.ul);
  c_rect.lr = this->ViewToContentsPoint(v_rect.lr);
  return c_rect;
}




Rectangle CTextView::ViewToScreenRect(Rectangle v_rect)
{
  Rectangle s_rect;
  
  s_rect.ul = this->ViewToScreenPoint(v_rect.ul);
  s_rect.lr = this->ViewToScreenPoint(v_rect.lr);
  return s_rect;
}




Rectangle CTextView::ScreenToContentsRect(Rectangle s_rect)
{
  Rectangle c_rect;
  
  c_rect.ul = this->ScreenToContentsPoint(s_rect.ul);
  c_rect.lr = this->ScreenToContentsPoint(s_rect.lr);
  return c_rect;
}




Rectangle CTextView::ScreenToViewRect(Rectangle s_rect)
{
  Rectangle v_rect;
  
  v_rect.ul = this->ScreenToViewPoint(s_rect.ul);
  v_rect.lr = this->ScreenToViewPoint(s_rect.lr);
  return v_rect;
}






/************************************/
/* Mapping between chars and pixels */
/************************************/




Rectangle CTextView::CharToPixelRect(Rectangle ch_rect)
{
  return sm_text_rect_gp(R(this)->textPane, ch_rect);
}




Rectangle CTextView::PixelToCharRect(Rectangle px_rect)
{
  Rectangle ch_rect;
  
  ch_rect.ul = sm_text_point_pg(R(this)->textPane, px_rect.ul, false);
  ch_rect.lr = sm_text_point_pg(R(this)->textPane, px_rect.lr, false);
  return ch_rect;
}





/**********************/
/* Access to contents */
/**********************/




int CTextView::numLines(void)
{
  return R(this)->editor->NumLines();
}




int CTextView::maxLineWidth(void)
{
  return R(this)->editor->MaxLineWidth();
}




void CTextView::getLine(int k, TextString &ts, TextData &td)
{
  R(this)->editor->GetLine(k, ts, td);
}







/***********************/
/* Access to selection */
/***********************/




void CTextView::getSelection(int &line1, int &char1, int &line2, int &char2)
{
  R(this)->editor->GetSelection(line1, char1, line2, char2);
}




void CTextView::setSelection(int line1, int char1, int line2, int char2)
{
 Boolean makeSelection = true;

  if( R(this)->oneLine )
    { /* avoid multi-line selections which might be discontiguous due to filtering */
        line1 = max(0, min(R(this)->editor->NumLines()-1, line1));
        line2 = line1;

      if( R(this)->noElidedLine )
        { /* avoid selection of a line which is invisible due to filtering */
            if( this->ContentsLineElided(line1) )
              makeSelection = false;
        }
    }

  if( makeSelection )
    R(this)->editor->SetSelection(line1, char1, line2, char2);
  else
    R(this)->editor->SetSelectionNone();
}






/******************/
/* Input handling */
/******************/




Boolean CTextView::SelectionEvent(Generic generator, Point info)
{
  return BOOL( generator == (Generic) R(this)->tv );
}




void CTextView::SetSelectionOptions(Boolean oneLine, Boolean noElidedLine)
{
  R(this)->oneLine      = oneLine;
  R(this)->noElidedLine = noElidedLine;
}






/***********/
/* Drawing */
/***********/




Point CTextView::CharacterSize(void)
{
  return fontSize(R(this)->font);
}




void CTextView::BeginClip(Rectangle clip)
{
  ASSERT( R(this)->clipDepth < MAX_CLIP_DEPTH );
  R(this)->clipStack[ R(this)->clipDepth ] = R(this)->clipRect;
  R(this)->clipDepth += 1;

  R(this)->clipRect = interRect(clip, R(this)->clipRect);
}




void CTextView::EndClip(void)
{
  ASSERT( R(this)->clipDepth > 0 );
  R(this)->clipDepth -= 1;

  R(this)->clipRect = R(this)->clipStack[ R(this)->clipDepth ];
}




Rectangle CTextView::GetClip(void)
{
  return R(this)->clipRect;
}




void CTextView::SetDefaultColors(Color foreColor,
                                      Color backColor,
                                      Color borderColor,
                                      Boolean useParentColors,
                                      Boolean invertColors,
                                      Boolean updateScreen)
{
  TextView tv = R(this)->tv;
  
  if( foreColor   == UNUSED_COLOR )  foreColor   = paneForeground(tv);
  if( backColor   == UNUSED_COLOR )  backColor   = paneBackground(tv);
  if( borderColor == UNUSED_COLOR )  borderColor = paneBorderColor(tv);
  
  recolorPane(tv, foreColor, backColor, borderColor,
              useParentColors, invertColors, updateScreen);
}




void CTextView::SetDrawingStyle(int lineWidth, LineStyle * lineStyle, Color fg, Color bg)
{
  R(this)->lineWidth = lineWidth;
  R(this)->lineStyle = lineStyle;
  R(this)->foreColor = fg;
  R(this)->backColor = bg;
}




void CTextView::GetDrawingStyle(int &lineWidth, LineStyle * &lineStyle, Color &fg, Color &bg)
{
  lineWidth = R(this)->lineWidth;
  lineStyle = R(this)->lineStyle;
  fg        = R(this)->foreColor;
  bg        = R(this)->backColor;
}




void CTextView::DrawLine(Point p1, Point p2)
{
  linePaneColorClipped(R(this)->tv,
                       p1, p2,
                       (short) R(this)->lineWidth,
                       R(this)->lineStyle,
                       R(this)->foreColor,
                       R(this)->clipRect);
}




void CTextView::DrawRect(Rectangle r)
{
  Point ur, ll;

  /* there is no 'boxPaneColorClipped' */

  ur.x = r.lr.x;
  ur.y = r.ul.y;

  ll.x = r.ul.x;
  ll.y = r.lr.y;

  this->DrawLine(r.ul, ur  );
  this->DrawLine(ur,   r.lr);
  this->DrawLine(r.lr, ll  );
  this->DrawLine(ll,   r.ul);
}




void CTextView::DrawArc(Rectangle box, double angle, double extent)
{
  arcLinePaneColorClipped(R(this)->tv,
                          &box, angle, extent,
                          (short) R(this)->lineWidth,
                          R(this)->lineStyle,
                          R(this)->foreColor,
                          R(this)->clipRect);
}




void CTextView::DrawPolygon(Point start, int numPts, Point * points)
{
  polygonPaneColor(R(this)->tv,
                     start, numPts, points,
                     NULL_BITMAP, Origin,
                     R(this)->clipRect,
                     R(this)->foreColor,
                     R(this)->backColor);
}






/**************/
/* Decorating */
/**************/




void CTextView::AddDecoration(Decoration * d)
{
  this->addDecoration(R(this)->decorations, d, false);
  this->update();
}




void CTextView::RemoveDecoration(Decoration * d)
{
  this->removeDecoration(R(this)->decorations, d);
  this->update();
}




void CTextView::RemoveAllDecorations(void)
{
  flex_delete(R(this)->decorations, 0, flex_length(R(this)->decorations));
  this->update();
}




void CTextView::AddMarginDecoration(MarginDecoration * md)
{
  this->addDecoration(R(this)->marginDecorations, md, true);
  this->layoutMarginDecorations();
  this->update();
}




void CTextView::RemoveMarginDecoration(MarginDecoration * md)
{
  this->removeDecoration(R(this)->marginDecorations, md);
  this->layoutMarginDecorations();
  this->update();
}




void CTextView::RemoveAllMarginDecorations(void)
{
  flex_delete(R(this)->marginDecorations, 0, flex_length(R(this)->marginDecorations));
  R(this)->totalMarginWidth = 0;
  this->update();
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/





/******************************/
/* Ned TextView customization */
/******************************/




static
void getDocSize(CTextView * ctv, Point *size)
{
  (*size).x = ctv->maxLineWidth() + R(ctv)->totalMarginWidth;
  (*size).y = ctv->numLines();
}




static
Boolean getLine(CTextView * ctv, int c_linenum, TextString *text, void *data)
{
  Boolean exists;

  if( c_linenum < ctv->numLines() )
    { exists = true;
      ctv->getLine(c_linenum, *text, *((TextData *) data));
      ctv->decorateMargins(c_linenum, *text, *(TextData *) data);
      ctv->colorizeLine(c_linenum, *text, *(TextData *) data);
    }
  else
    exists = false;

  return exists;
}




static
void getSelection(CTextView * ctv, int *linenum, int *sel1, int *sel2)
{
  int line1, char1, line2, char2;

  ctv->getSelection(line1, char1, line2, char2);
  
  if( line1 == line2  &&  NOT(char1 == 0 && char2 >= INFINITY) )
    { *linenum = line1;
      *sel1    = char1 + R(ctv)->totalMarginWidth;
      *sel2    = char2 + R(ctv)->totalMarginWidth;
    }
  else
    { *linenum = UNUSED;
      *sel1    = line1;
      *sel2    = line2;
    }
}




static
void setSelection(CTextView * ctv, int linenum, int sel1, int sel2)
{
  int line1, char1, line2, char2;

  if( linenum != UNUSED &&
      sel1 < R(ctv)->totalMarginWidth && sel2 == sel1-1 )
    ctv->marginDecorationClicked(linenum, sel1);

  else
    { if( linenum != UNUSED )
        { line1 = linenum;
          char1 = max(sel1 - R(ctv)->totalMarginWidth, 0);
          line2 = linenum;
          char2 = max(sel2 - R(ctv)->totalMarginWidth, char1-1);
        }

      else
        { line1 = sel1;
          char1 = 0;
          line2 = sel2;
          char2 = 9999;
        }

      ctv->setSelection(line1, char1, line2, char2);
    }
}




static
void customRepaint(CTextView * ctv, Pane * textPane,
                   ViewFilter nedFilter,
                   Rectangle * viewRect,
                   int pane_line1, int pane_line2)
{
  ctv->drawDecorations(pane_line1, pane_line2);
}






/**************/
/* Decorating */
/**************/




void CTextView::addDecoration(Flex * decs, Decoration * d, Boolean first)
{
  int where = (first ? 0 : flex_length(decs));

  flex_insert_one(decs, where, (char *) &d);
  d->AttachView(this);
}




void CTextView::removeDecoration(Flex * decs, Decoration * d)
{
  int index;
  
  index = flex_index(decs, 0, (char *) &d);
  if( index != UNUSED)
    flex_delete(decs, index, 1);
}




void CTextView::layoutMarginDecorations(void)
{
  Flex * decs = R(this)->marginDecorations;
  int numDecs, k;
  MarginDecoration * md;

  numDecs = flex_length(R(this)->marginDecorations);
  R(this)->totalMarginWidth = 0;
  for( k = 0;  k < numDecs;  k++ )
    { flex_get_buffer(decs, k, 1, (char *) &md);
      md->SetStart(R(this)->totalMarginWidth);
      R(this)->totalMarginWidth += md->Width();
    }
}




void CTextView::update(void)
{
  /* should really have a View method for this */
    vf_NoteChange(R(this)->nedFilter, tt_SEL_CHANGED, false, 0, INFINITY, 0);
}




void CTextView::decorateMargins(int c_linenum, TextString &text, TextData &data)
{
  Flex * decs = R(this)->marginDecorations;
  int numDecs = flex_length(R(this)->marginDecorations);
  int totalWidth = R(this)->totalMarginWidth;
  int len, k, next;
  TextString newText;
  MarginDecoration * md;

  if( totalWidth == 0 )  return;

  /* make room for left-margin decoration texts */
    /* make a new longer textstring */
      len = text.num_tc;
      newText = createTextString(len + totalWidth, "CTextView::decorateMargins");

    /* copy the original textstring */
      for( k = 0;  k < len;  k++ )
        newText.tc_ptr[k + totalWidth] = text.tc_ptr[k];
    
    /* adjust the color data */
      Text_MultiColoredData(data);
      for( k = MAX_TEXT_CHARS-totalWidth-1;  k >= 0;  k-- )
       data.chars[k + totalWidth] = data.chars[k];
      for( k = 0;  k < totalWidth;  k++ )
        data.chars[k] = Text_DefaultColorPair;

    /* replace original by new textstring */
      destroyTextString(text);
      text = newText;
   
  /* add each margin decoration */
    next = 0;
    for( k = 0;  k < numDecs;  k++ )
      { flex_get_buffer(decs, k, 1, (char *) &md);
        md->GetMarginText(c_linenum, newText.tc_ptr + next, data.chars + next);
        next += md->Width();
      }
}




void CTextView::colorizeLine(int c_linenum, TextString &text, TextData &data)
{
  Flex * decs       = R(this)->decorations;
  int numDecs       = flex_length(R(this)->decorations);
  Flex * marginDecs = R(this)->marginDecorations;
  int numMarginDecs = flex_length(R(this)->marginDecorations);
  int k;
  MarginDecoration * md;
  Decoration * d;

  if( numDecs == 0 && numMarginDecs == 0 )
    return;
  else
    Text_MultiColoredData(data);

  /* colorize by each margin decoration */
    for( k = 0;  k < numMarginDecs;  k++ )
      { flex_get_buffer(marginDecs, k, 1, (char *) &md);
        md->ColorizeLine(c_linenum, R(this)->totalMarginWidth, text, data);
      }

  /* colorize by each plain decoration */
    for( k = 0;  k < numDecs;  k++ )
      { flex_get_buffer(decs, k, 1, (char *) &d);
        d->ColorizeLine(c_linenum, R(this)->totalMarginWidth, text, data);
      }
}




void CTextView::drawDecorations(int pane_line1, int pane_line2)
{
  Rectangle clip;
  int num, k;
  Decoration * d;

  /* clip to just the lines being repainted */
    clip = sm_text_rect_gp(R(this)->textPane,
                           makeRect(makePoint(0, pane_line1),
                                    makePoint(INFINITY, pane_line2)));
    this->BeginClip(clip);

  /* draw the margin decorations */
    num = flex_length(R(this)->marginDecorations);
    for( k = 0;  k < num;  k++ )
      { flex_get_buffer(R(this)->marginDecorations, k, 1, (char *) &d);
        d->Draw();
      }

  /* draw the plain decorations */
    num = flex_length(R(this)->decorations);
    for( k = 0;  k < num;  k++ )
      { flex_get_buffer(R(this)->decorations, k, 1, (char *) &d);
        d->Draw();
      }
    
  this->EndClip();
}




void CTextView::marginDecorationClicked(int c_linenum, int char1)
{
  Flex * decs = R(this)->marginDecorations;
  int num  = flex_length(decs);
  int k;
  Boolean found;
  MarginDecoration * md;

  found = false;
  for( k = 0;  ! found && k < num;  k++ )
    { flex_get_buffer(decs, k, 1, (char *) &md);
      if( md->Start() <= char1  &&  char1 < md->Start() + md->Width() )
        { md->Clicked(c_linenum, char1 - md->Start());
          found = true;
        }
    }
}
