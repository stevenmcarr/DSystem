/* $Id: HeadingView.C,v 1.2 1997/03/11 14:32:43 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/HeadingView.C						*/
/*									*/
/*	HeadingView -- Column headings for ColumnViews			*/
/*	Last edited: October 13, 1993 at 6:15 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/framework/HeadingView.h>

#include <libs/graphicInterface/framework/CTextView.h>
#include <libs/graphicInterface/framework/CViewFilter.h>
#include <libs/graphicInterface/framework/ColumnEditor.h>


/* Ned stuff, sigh */
#define tt_NOTIFY_SEL_CHANGED  0






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* HeadingView object */

typedef struct HeadingView_Repr_struct
  {
    /* creation parameters */
      ColumnEditor *	editor;
      TextString	headings;

  } HeadingView_Repr;


#define R(ob)		(ob->HeadingView_repr)

#define INHERITED	CTextView






/*************************/
/*  Forward declarations */
/*************************/




static TextString makeHeadings(int numCols,
                               int colWidths[],
                               char * headings[],
                               int contentsFont,
                               int headingFont);

static void columnBounds(HeadingView * hv, int col, int &first, int &width);

static int findColumn(HeadingView * hv, int pos);






/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/






/*************************/
/*  Class initialization */
/*************************/




void HeadingView::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(CTextView);
    REQUIRE_INIT(CViewFilter);
    REQUIRE_INIT(ColumnEditor);
}




void HeadingView::FiniClass(void)
{
  /* nothing */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(HeadingView)




HeadingView::HeadingView(Context context,
                         DB_FP * session_fd,
                         ColumnEditor * editor,
                         int numCols,
                         int colWidths[],
                         char * headings[],
                         int contentsFont,
                         int headingFont)
            : CTextView(context, session_fd, editor, makePoint(0, 0), headingFont)
{
  /* allocate instance's private data */
    this->HeadingView_repr = (HeadingView_Repr *) get_mem(sizeof(HeadingView_Repr),
                                                          "HeadingView instance");

  /* save creation parameters */
    R(this)->editor   = editor;
    R(this)->headings = makeHeadings(numCols, colWidths, headings,
                                     contentsFont, headingFont);
}




HeadingView::~HeadingView(void)
{
  destroyTextString(R(this)->headings);
  free_mem((void*) this->HeadingView_repr);
}






/******************/
/*  Window layout */
/******************/




void HeadingView::InitPanes(void)
{
  this->INHERITED::InitPanes();
  this->SetDefaultColors(color("titlePane.highlighted"), UNUSED_COLOR, UNUSED_COLOR,
                         false, false, false);
}




/**********************/
/* Access to contents */
/**********************/




int HeadingView::numLines(void)
{
  return 1;
}




int HeadingView::maxLineWidth(void)
{
  return R(this)->headings.num_tc;
}




void HeadingView::getLine(int lineNum, TextString &ts, TextData &td)
{
  int colNum, first, width, k;

  /* ASSERT: 'lineNum' == 0 */

  ts = copyTextString(R(this)->headings);
  ts.ephemeral = false;

  R(this)->editor->GetSortColumn(colNum);
  columnBounds(this, colNum, first, width);
  for( k = first;  k < first + width;  k++ )
    ts.tc_ptr[k].style |= ATTR_UNDERLINE;

  Text_DefaultData(td);
}







/***********************/
/* Access to selection */
/***********************/




void HeadingView::getSelection(int &line1, int &char1, int &line2, int &char2)
{
  line1 = line2 = INFINITY;
  char1 = char2 = 0;
}




void HeadingView::setSelection(int line1, int char1, int line2, int char2)
{
  int colNum;
  LineEditorChange change;

  colNum = findColumn(this, char1);

  /* send appropriate notifications */
    this->Changed(CHANGE_HEADING, (void *) colNum);

#if 0
    change.kind       = tt_NOTIFY_SEL_CHANGED;
    change.autoScroll = false;
    change.data       = (Generic) nil;
    change.first      = 0;
    change.last       = 0;
    change.delta      = 0;
    this->NoteChange(this, CHANGE_SELECTION, (void *) &change);
#endif
}






/*************/
/* Filtering */
/*************/




CViewFilter * HeadingView::makeDefaultFilter(void)
{
  /* identity filter */
  return new CViewFilter;
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




static
TextString makeHeadings(int numCols,
                        int colWidths[],
                        char * headings[],
                        int contentsFont,
                        int headingFont)
{
  int contentsCharWidth = fontSize(contentsFont).x;
  int headingCharWidth  = fontSize(headingFont).x;
  char buffer[300];
  int k, next, totalContentsChars, pixelStart, hcharsStart;
  TextString ts;

  /* compose a string with headings positioned properly by space chars */
    totalContentsChars = 0;
    next = 0;
    for( k = 0;  k < numCols;  k++ )
      { /* compute heading-char position at which the k-th heading should start */
          pixelStart  = totalContentsChars * contentsCharWidth;
          hcharsStart = (pixelStart + headingCharWidth-1) / headingCharWidth;

        /* insert appropriate padding */
          while( next < hcharsStart )
            { buffer[next] = ' ';
              next += 1;
            }

        /* copy the heading text into the buffer */
          strcpy(&buffer[next], headings[k]);
          next += strlen(headings[k]);

        totalContentsChars += colWidths[k];
      }

  /* make a corresponding TextString */
    ts = makeTextString(buffer, STYLE_BOLD, "HeadingView headings");
    ts.ephemeral = false;

  return ts;
}




static
void columnBounds(HeadingView * hv, int col, int &first, int &width)
{
  TextString headings = R(hv)->headings;
  int len = headings.num_tc;
  int k, c;

  if( col == UNUSED )
    { first = 0;
      width = 0;
    }
  else
    { k = 0;  c = -1;
      do
        { /* advance to start of next column */
            while( headings.tc_ptr[k].ch == ' '  &&   k < len )
              k += 1;
            first = k;

          /* advance past the column */
            while( headings.tc_ptr[k].ch != ' '  &&   k < len )
              k += 1;
            width = k - first;

          c += 1;

        } while( c < col );
    }
}




static
int findColumn(HeadingView * hv, int pos)
{
  TextString headings = R(hv)->headings;
  int len = headings.num_tc;
  int k, c, first, width;

  /* ASSERT: 'pos' is the index of a character in 'headings' */

  if( headings.tc_ptr[pos].ch == ' ' )  return UNUSED;

  k = 0;  c = -1;
  do
    { /* advance to start of next column */
        while( headings.tc_ptr[k].ch == ' '  &&   k < len )
          k += 1;
        first = k;

      /* advance past the column */
        while( headings.tc_ptr[k].ch != ' '  &&   k < len )
          k += 1;
        width = k - first;

      c += 1;

    } while( ! (first <= pos  &&  pos < first + width) );

  return c;
}
