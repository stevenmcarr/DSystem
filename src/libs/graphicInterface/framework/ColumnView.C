/* $Id: ColumnView.C,v 1.2 1997/03/11 14:32:34 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/ColumnView.C						*/
/*									*/
/*	ColumnView -- View of text lines in labeled columns		*/
/*	Last edited: October 13, 1993 at 12:39 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/framework/ColumnView.h>

#include <libs/graphicInterface/oldMonitor/include/sms/text_sm.h>
#include <libs/graphicInterface/oldMonitor/include/mon/cp.h>
#include <libs/graphicInterface/framework/ColumnEditor.h>
#include <libs/graphicInterface/framework/HeadingView.h>


typedef Generic ViewFilter;






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* ColumnView object */

typedef struct ColumnView_Repr_struct
  {
    /* creation parameters */
      ColumnEditor *	editor;
      int		numCols;
      int		(*colWidths);
      char *		(*headings);
      int		numDivLines;
      int		(*divLineCols);
      short		contentsFont;
      short		headingFont;

      Point		headingCharSize;
      Point		contentsCharSize;

    /* subparts */
      HeadingView *	heading;

  } ColumnView_Repr;


#define R(ob)		(ob->ColumnView_repr)

#define INHERITED	LineView






/*************************/
/*  Miscellaneous	 */
/*************************/




/* headings */

static short cv_headingFont;


/* dividing lines */

static Color cv_divLineColor;






/*************************/
/*  Forward declarations */
/*************************/




static void paintDivLines(ColumnView * cv, Pane * textPane, ViewFilter nedFilter,
                          Rectangle * viewRect, int line1, int line2);

static int columnStart(ColumnView * cv, int colNum);

static Point c2p_point(ColumnView * cv, Rectangle * viewRect, Point pt);






/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/






/*************************/
/*  Class initialization */
/*************************/




void ColumnView::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(LineView);
    REQUIRE_INIT(HeadingView);
    REQUIRE_INIT(ColumnEditor);

  /* initialize display details */
    cv_divLineColor = black_color;
    cv_headingFont  = fontOpen("screen.7.rnf");
}




void ColumnView::FiniClass(void)
{
  /* finalize display details */
    fontClose(cv_headingFont);
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(ColumnView)




ColumnView::ColumnView(Context context,
                     DB_FP * session_fd,
                     ColumnEditor * editor,
                     Point initScrollPos,
                     int contentsFont,
                     int headingFont,
                     int numCols,
                     int colWidths[],
                     char * headings[],
                     int numDivLines,
                     int divLineCols[])
           : LineView (context, session_fd, editor, initScrollPos,
                       (contentsFont != UNUSED ? contentsFont : DEF_FONT_ID))
{
  int k;

  /* allocate instance's private data */
    this->ColumnView_repr = (ColumnView_Repr *) get_mem(sizeof(ColumnView_Repr), "ColumnView instance");

  /* save creation arguments */
    R(this)->editor       = editor;
    R(this)->contentsFont = (contentsFont != UNUSED ? contentsFont : DEF_FONT_ID);
    R(this)->headingFont  = (headingFont  != UNUSED ? headingFont  : cv_headingFont);
    R(this)->numCols      = numCols;

    R(this)->colWidths = (int *) get_mem(numCols * sizeof(int *), "ColumnView colWidths");
    for( k = 0;  k < numCols;  k++ )
      R(this)->colWidths[k] = colWidths[k];

    R(this)->headings = (char * *) get_mem(numCols * sizeof(char *), "ColumnView headings");
    for( k = 0;  k < numCols;  k++ )
      R(this)->headings[k] = ssave(headings[k]);

    R(this)->numDivLines = numDivLines;
    R(this)->divLineCols = (int *) get_mem(numDivLines * sizeof(int *), "ColumnView divLineCols");
    for( k = 0;  k < numDivLines;  k++ )
      R(this)->divLineCols[k] = divLineCols[k];

  /* create subparts */
    R(this)->heading = new HeadingView(context, session_fd, editor,
                                      numCols, colWidths, headings,
                                      R(this)->contentsFont,
                                      R(this)->headingFont);
    R(this)->heading->Notify(this, true);
}




ColumnView::~ColumnView(void)
{
  delete R(this)->heading;
  free_mem((void*) R(this)->colWidths);
  free_mem((void*) R(this)->headings);
  free_mem((void*) this->ColumnView_repr);
}






/******************/
/*  Window layout */
/******************/




void ColumnView::GetSizePrefs(Point &minSize, Point &defSize)
{
  minSize = makePoint(160, 24);
  defSize = makePoint(600, 160);
}




Generic ColumnView::GetTiling(Boolean init, Point size)
{
  Point headingSize, contentsSize;
  Generic contents_td, heading_td, this_td;

  /* calculate sizes of the subpanes */
    headingSize = sm_text_pane_size(makePoint(1, 1), R(this)->headingFont);
    headingSize.x = size.x;

    contentsSize.x = size.x;
    contentsSize.y = size.y - headingSize.y;

  /* compute the tiling descriptor */
    contents_td = this->INHERITED::GetTiling(init, contentsSize);
    heading_td  = R(this)->heading->GetTiling(init, headingSize);
    this_td = cp_td_join(
                  TILE_DOWN,
                  (aTilingDesc*)contents_td,
                  (aTilingDesc*)heading_td
                  );

  return this_td;
}




void ColumnView::InitPanes(void)
{
  this->INHERITED::InitPanes();

  R(this)->heading->InitPanes();
  R(this)->contentsCharSize = fontSize(R(this)->contentsFont);
  R(this)->headingCharSize  = fontSize(R(this)->headingFont);
  this->CustomizeRepainting((tv_CustomRepaintFunc) paintDivLines);
}






/***********************/
/* Change notification */
/***********************/




void ColumnView::NoteChange(Object * ob, int kind, void * change)
{
  NoteChangeTrace trace(this, ob, kind, change);

  if( kind == CHANGE_HEADING )
    R(this)->editor->SetSortColumn((int) change);
  else
    this->INHERITED::NoteChange(ob, kind, change);
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




static
void paintDivLines(ColumnView * cv, Pane * textPane, ViewFilter nedFilter,
                   Rectangle * viewRect, int line1, int line2)
{
  Rectangle temp;
  Point p1, p2;
  int k, pos;

# define Pt(p)           c2p_point(cv, viewRect, p)

# define Line(p1,p2)     linePaneColor(textPane, Pt(p1), Pt(p2),  \
                                1, line_style_solid, cv_divLineColor)

  /* draw the lines */
    p1.y = 0/2;  p2.y = INFINITY/2;
    for( k = 0;  k < R(cv)->numDivLines;  k++ )
      { pos = columnStart(cv, R(cv)->divLineCols[k]);
        setRect(&temp, pos, 0, 0, 0);
        temp = sm_text_rect_gp(textPane, temp);
        p1.x = p2.x = temp.ul.x - R(cv)->headingCharSize.x/2;
        Line(p1, p2);
      }
}




static
int columnStart(ColumnView * cv, int colNum)
{
  int k, total;

  total = 0;
  for( k = 0;  k < colNum;  k++ )
    total += R(cv)->colWidths[k];

  return total;
}





static
Point c2p_point(ColumnView * cv, Rectangle * viewRect, Point pt)
{
  Point offset;

  offset.x = viewRect->ul.x * R(cv)->contentsCharSize.x;
  offset.y = viewRect->ul.y * R(cv)->contentsCharSize.y;
  return subPoint(pt, offset);
}
