/* $Id: TextView.C,v 1.1 1997/06/25 13:51:02 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/TextView.c						*/
/*									*/
/*	TextView -- screen module showing a view on a text		*/
/*	Last edited:  November 15, 1992 at 11:44 am			*/
/*									*/
/************************************************************************/



#include <libs/graphicInterface/cmdProcs/newEditor/ned.h>

#include <libs/graphicInterface/oldMonitor/include/mon/sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/text_sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/scroll_sm.h>

#include <libs/graphicInterface/cmdProcs/newEditor/TextView.h>
#include <libs/graphicInterface/cmdProcs/newEditor/ViewFilter.h>




/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




typedef struct
  {
    Generic			contents;
    TV_Methods *		methods;
    ViewFilter			filter;
    Rectangle			viewRect;

    Pane *			textPane;
    int				charWidth;

    ScrollBar			hscroll;
    ScrollBar			vscroll;

    Boolean			selCached;
    int				selLine;
    int				sel1;
    int				sel2;

    int				selBehavior;
    Generic			repaintOb;
    tv_CustomRepaintFunc	repaintProc;

  } tv_Repr;


#define	SM(ob)		((Pane *) ob)
#define	R(ob)		((tv_Repr *) SM(ob)->pane_information)






/************************/
/* Screen module	*/
/************************/




/* these must be declared early for sm structure below */

STATIC(void, init,(void));
STATIC(void, fini,(void));
STATIC(void, create,(TextView tv));
STATIC(void, destroy,(TextView tv));
STATIC(void, resize,(TextView tv));
STATIC(void, input,(TextView tv, Rectangle r));


static aScreenModule tv_screenModuleOps =
{
  "TextView",
  init,
  fini,
  create,
  resize,
  standardNoSubWindowPropagate,
  destroy,
  input,
  standardTileNoWindow,
  standardDestroyWindow
};






/*****************/
/* Miscellaneous */
/*****************/




static TV_ColorPair tv_defaultColorPair;






/************************/
/* Forward declarations	*/
/************************/


STATIC(void,            scrollMax,(TextView tv, int *x_max, int *x_step, int *y_max,
                                   int *y_step));
STATIC(void,            setScrollBars,(TextView tv, Boolean touch));
STATIC(void,            scrollTo,(TextView tv, int x, int y));
STATIC(void,            scrollProc,(TextView tv, ScrollBar sb, int dir, int value));
STATIC(void,            paint,(TextView tv, Rectangle rect, Boolean touch));
STATIC(void,            guessVertScroll,(TextView tv, int *y_request));
STATIC(void,            paintLine,(TextView tv, int c_lineNum, int c_left, int c_right,
                                   TextString text, TV_Data *data));
STATIC(void,            putMultiColorString,(Pane *textPane, Point pos, TextString ts,
                                             TV_Data *data, int dataOffset));
STATIC(void,            setInverse,(TextString ts, TV_Data *td, int left, int right,
                                    Boolean partialLine));
STATIC(TextString,      clipText,(TextString text, int left, int right));
STATIC(void,            shift,(TextView tv, Rectangle rect, int dx, int dy,
                               Boolean touch));
STATIC(void,            reverseIP,(TextView tv, int line, int char1));
STATIC(void,            trackMouse,(TextView tv));
STATIC(Point,           textPoint,(TextView tv, Point pt));

STATIC(int,             c2p_x,(TextView tv, int colNum));
STATIC(int,             c2p_y,(TextView tv, int lineNum));
STATIC(Point,           c2p_pt,(TextView tv, Point pt));
STATIC(Rectangle,       c2p_rect,(TextView tv, Rectangle c_rect));
STATIC(int,             p2c_x,(TextView tv, int colNum));
STATIC(int,             p2c_y,(TextView tv, int lineNum));
STATIC(Point,           p2c_pt,(TextView tv, Point pt));
STATIC(Rectangle,       p2c_rect,(TextView tv, Rectangle p_rect));

STATIC(void,            getDocSize,(TextView tv, Point *size));
STATIC(void,            getLine,(TextView tv, int lineNum, TextString *text,
                                 TV_Data *data));
STATIC(void,            getSelection,(TextView tv, int *line, int *sel1, int *sel2));
STATIC(void,            setSelection,(TextView tv, int line, int sel1, int sel2));

STATIC(void,            customize,(TextView tv, int c_line1, int c_line2));









/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/




/************************/
/*  Screen module	*/
/************************/




short tv_ScreenModuleIndex(void)
{
  return getScreenModuleIndex(&tv_screenModuleOps);
}




Point tv_ViewSize(Point charSize, short font)
{
  return sm_text_pane_size(charSize,font);
}






/************************/
/*  Instance init	*/
/************************/




void tv_DefaultData(TV_Data* data)
{
  data->multiColored   = false;
  data->all.foreground = black_color;
  data->all.background = white_color;
}




void tv_MultiColoredData(TV_Data* data)
{
  int k;

  data->multiColored = true;

  for( k = 0;  k < tv_MAX_CHARS;  k++ )
    { data->chars[k].foreground = data->all.foreground;
      data->chars[k].background = data->all.background;
    }
}




void tv_PaneInit(TextView tv, Generic contents, TV_Methods* methods, 
                 ViewFilter filter, Point scrollPos, int font)
{
  sm_text_change_font(R(tv)->textPane, font);    /* must precede 'sm_text_size' */

  R(tv)->methods     = methods;
  R(tv)->contents    = contents;
  R(tv)->filter      = filter;
  R(tv)->viewRect    = makeRectFromSize(scrollPos,sm_text_size(R(tv)->textPane));
  R(tv)->charWidth   = xPoint(fontSize(font));
  R(tv)->selCached   = false;
  R(tv)->selBehavior = tv_SB_NORMAL;
  R(tv)->repaintProc = (tv_CustomRepaintFunc) nil;

  vf_SetContents(filter,R(tv)->contents,R(tv)->methods);
  vf_Notify(filter, (Generic)tv, (vf_NotifyFunc)tv_NoteChange);

  paint(tv,R(tv)->viewRect,true);
  setScrollBars(tv,true);
}




void tv_ScrollBars(TextView tv, ScrollBar hscroll, ScrollBar vscroll)
{
  R(tv)->hscroll = hscroll;
  R(tv)->vscroll = vscroll;

  if( hscroll != nil )
    sm_scroll_scrollee(hscroll, (Generic)tv, (sm_scroll_scrollee_callback)scrollProc);
  if( vscroll != nil )
    sm_scroll_scrollee(vscroll, (Generic)tv, (sm_scroll_scrollee_callback)scrollProc);

  setScrollBars(tv,true);
}




Generic tv_getTextPane(TextView tv)
{
  return (Generic) R(tv)->textPane;
}






/************************/
/*  Change Notification	*/
/************************/




void tv_NoteChange(TextView tv, int kind, Boolean autoScroll, int first, int last, int delta)
{
  Rectangle shiftRect;		/* in contents coords */
  Rectangle paintRect;		/* in contents coords */

  R(tv)->selCached = false;

  if( kind == NOTIFY_DOC_WILL_CHANGE )  return;        
  
  /* shift text below the change -- if sel change, delta is zero */
    if( delta != 0 )
      { shiftRect = R(tv)->viewRect;
        shiftRect.ul.y = last + 1;
        shift(tv,shiftRect,0,delta,true);

        /* repaint area exposed by shift */
          paintRect = R(tv)->viewRect;
          paintRect.ul.y = (shiftRect.lr.y + 1) + delta;
          paint(tv,paintRect,true);
      }

  /* repaint changed area */
    paintRect = R(tv)->viewRect;
    paintRect.ul.y = first;
    paintRect.lr.y = max(last,last+delta);
    paint(tv,paintRect,true);

  /* notice any change in size of document */
    if( kind == NOTIFY_DOC_CHANGED )  setScrollBars(tv,true);
}




void tv_CustomizeRepainting(TextView tv, Generic repaintOb, tv_CustomRepaintFunc repaintProc)
{
  R(tv)->repaintOb   = repaintOb;
  R(tv)->repaintProc = repaintProc;

  paint(tv,R(tv)->viewRect,true);
}






/************************/
/*  Access to View	*/
/************************/




Point tv_GetViewSize(TextView tv)
{
  return sizeRect(R(tv)->viewRect);
}




Point tv_GetScroll(TextView tv)
{
  return R(tv)->viewRect.ul;
}




void tv_SetScroll(TextView tv, Point scrollPos)
{
  int x_max, y_max, dummy;
  
  scrollMax(tv,&x_max,&dummy,&y_max,&dummy);
  scrollTo(tv, max(0, min(x_max,scrollPos.x)), max(0, min(y_max,scrollPos.y)));
  setScrollBars(tv,true);
}




void tv_ScrollBy(TextView tv, Point delta)
{
  int x_max,y_max,x,y,dummy;

  scrollMax(tv,&x_max,&dummy,&y_max,&dummy);
  x = max(0, min(x_max, R(tv)->viewRect.ul.x + delta.x));
  y = max(0, min(y_max, R(tv)->viewRect.ul.y + delta.y));

  scrollTo(tv,x,y);
  setScrollBars(tv,true);
}




void tv_EnsureContentsVisible(TextView tv, Point pt, Boolean bounce)
/* pt - in underlying-contents coords */
{
  int temp1,temp2;

  /* convert desired point to view coords */
    temp1 = pt.y;
    vf_ContentsToView(R(tv)->filter,temp1,&temp2);
    pt.y = temp2;

  tv_EnsureVisible(tv,pt,bounce);
}




void tv_EnsureVisible(TextView tv, Point pt, Boolean bounce)
/* pt - in view-contents coords */
{
  Point size,old_ul,new_ul;
  int extra;

  size    = sizeRect(R(tv)->viewRect);
  old_ul  = R(tv)->viewRect.ul;

  /* calculate the desired x scrollpos */
    extra = (bounce  ?  size.x / 10  :  0);
    if( pt.x < old_ul.x )
      new_ul.x = pt.x - extra;
    else if( pt.x >= old_ul.x + size.x )
      new_ul.x = (pt.x - size.x + 1) + extra;
    else
      new_ul.x = old_ul.x;

  /* calculate the desired y scrollpos */
    extra = (bounce  ?  size.y / 2  :  0);
    if( pt.y < old_ul.y )
      new_ul.y = pt.y - extra;
    else if( pt.y >= old_ul.y + size.y )
      new_ul.y = (pt.y - size.y + 1) + extra;
    else
      new_ul.y = old_ul.y;

  if( NOT( equalPoint(old_ul,new_ul) ) )
    tv_SetScroll(tv,new_ul);
}




ViewFilter tv_GetFilter(TextView tv)
{
  return R(tv)->filter;
}




void tv_SetFilter(TextView tv, ViewFilter filter, Boolean coords, Rectangle changed)
/* coords - true if new filter preserves view coords */
/* changed - if 'coords', only 'changed' rect looks different */
{
  int v_pos_x,v_pos_y,c_pos_x,c_pos_y;
  int x_max,y_max,dummy;

  /* selection is cached in perhaps-now-invalid view coords */
    R(tv)->selCached = coords;

  /* remember the scroll position in contents coords */
    if( ! coords )
      { v_pos_x = R(tv)->viewRect.ul.x;
        v_pos_y = R(tv)->viewRect.ul.y;
        c_pos_x = v_pos_x;
        vf_ViewToContents(R(tv)->filter,v_pos_y,&c_pos_y);
      }

  /* install the new filter */
    R(tv)->filter = filter;
    vf_SetContents(filter,R(tv)->contents,R(tv)->methods);
    vf_Notify(filter, (Generic)tv, (vf_NotifyFunc)tv_NoteChange);

  /* try to establish a new scroll position corresponding to the old one */
    if( ! coords )
      { v_pos_x = c_pos_x;
        vf_ContentsToView(R(tv)->filter,c_pos_y,&v_pos_y);

        scrollMax(tv,&x_max,&dummy,&y_max,&dummy);
        R(tv)->viewRect = relocRect(R(tv)->viewRect, makePoint(
                                    max(0, min(x_max,v_pos_x)),
                                    max(0, min(y_max,v_pos_y))));
      }

  /* update the screen */
    if( ! coords )
      { paint(tv,R(tv)->viewRect,true);
        setScrollBars(tv,true);
      }
    else
      paint(tv,changed,true);
}




void tv_GetSelectionBehavior(TextView tv, int* beh)
{
  *beh = R(tv)->selBehavior;
}




void tv_SetSelectionBehavior(TextView tv, int beh)
{
  R(tv)->selBehavior = beh;
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




static
void init(void)
{
  /* initialize structured constants */
    tv_defaultColorPair.foreground = black_color;
    tv_defaultColorPair.background = white_color;
}




static
void fini(void)
{
  /* nothing */
}




static
void create(TextView tv)
{
  short text_sm = sm_text_get_index();

  SM(tv)->border_width = 1;
  SM(tv)->pane_information = (Generic) get_mem(sizeof(tv_Repr), "TextView");

  R(tv)->textPane = newSlavePane(SM(tv), text_sm, SM(tv)->position, SM(tv)->size,
                                 SM(tv)->border_width);

  R(tv)->contents = nil;
  R(tv)->hscroll  = nil;
  R(tv)->vscroll  = nil;
}





static
void destroy(TextView tv)
{
  destroyPane(R(tv)->textPane);
  free_mem((void*)SM(tv)->pane_information);
}




static
void resize(TextView tv)
{
  resizePane(R(tv)->textPane, SM(tv)->position, SM(tv)->size);
  R(tv)->viewRect = makeRectFromSize( R(tv)->viewRect.ul,
                                      sm_text_size(R(tv)->textPane) );

  if( R(tv)->contents != nil )
    { paint(tv, R(tv)->viewRect, false);
      setScrollBars(tv,false);
    }
}




static
void input(TextView tv, Rectangle r)
{
  anEvent oldEvent;
  int x_cur,y_cur,x_max,y_max,x,y,dummy;

  while ((mon_event.type < MOUSE_KEYBOARD) && pointInRect(mon_event.loc, r))
    { handlePane(R(tv)->textPane);
      switch( mon_event.type )
        {
          case EVENT_SELECT:
            oldEvent = mon_event;
            trackMouse(tv);
            mon_event = oldEvent;
            break;

          case EVENT_MOVE:
              x_cur = R(tv)->viewRect.ul.x;
              y_cur = R(tv)->viewRect.ul.y;
              scrollMax(tv, &x_max, &dummy, &y_max, &dummy);
              switch( mon_event.msg )
                { 
                  case 0:  /* normal movement */
                    x = max(0, min(x_max, x_cur + mon_event.info.x));
                    y = max(0, min(y_max, y_cur + mon_event.info.y));
                    scrollTo(tv,x,y);
                    break;

                  case 1:  /* move to the top */
                    scrollTo(tv, x_cur, 0);
                    break;

                  case 2:  /* move to the bottom */
                    scrollTo(tv, x_cur, y_max);
                    break;

                  case 3:  /* move to the left */
                    scrollTo(tv, 0, y_cur);
                    break;

                  case 4:  /* move to the right */
                    scrollTo(tv, x_max, y_cur);
                    break;
                }
              setScrollBars(tv,true);
              getEvent();
              break;

          case EVENT_HELP:
            mon_event.info = transPoint(mon_event.info, R(tv)->viewRect.ul);
            break;
        }
    }
}




static
void scrollMax(TextView tv, int* x_max, int* x_step, int* y_max, int* y_step)
{
  Point docSize,viewSize;    /* all in pane coords */

  getDocSize(tv, &docSize);
  viewSize  = sizeRect(R(tv)->viewRect);

  *x_max  = max(0, docSize.x - viewSize.x);
  *x_step = viewSize.x - 1;

  *y_max  = max(0, docSize.y - viewSize.y);
  *y_step = viewSize.y - 1;
}




static
void setScrollBars(TextView tv, Boolean touch)
{
  ScrollBar hscroll = R(tv)->hscroll;
  ScrollBar vscroll = R(tv)->vscroll;
  int x_cur = R(tv)->viewRect.ul.x;
  int y_cur = R(tv)->viewRect.ul.y;
  int x_max,x_step,y_max,y_step;

  scrollMax(tv,&x_max,&x_step,&y_max,&y_step);
  if( x_cur > x_max  ||  y_cur > y_max )
    scrollTo(tv, min(x_cur,x_max), min(y_cur,y_max));
  
  sm_text_set_move_status( R(tv)->textPane,
                           (x_max > 0 ? TSM_MOVE_HORIZ : TSM_NO_MOVE_HORIZ),
                           (y_max > 0 ? TSM_MOVE_VERT  : TSM_NO_MOVE_VERT)
                         );

  if( hscroll != nil )
    { sm_scroll_set(hscroll,0,x_max,R(tv)->viewRect.ul.x,touch);
      sm_scroll_set_step(hscroll,1,x_step);
    }

  if( vscroll != nil )
    { sm_scroll_set(vscroll,0,y_max,R(tv)->viewRect.ul.y,touch);
      sm_scroll_set_step(vscroll,1,y_step);
    }
}




static
void scrollTo(TextView tv, int x, int y)
{
  Point old_ul;			/* in contents coords */
  int dx,dy;
  Rectangle paintRect;		/* in contents coords */

  /* calculate change info for update */
    old_ul = R(tv)->viewRect.ul;
    dx = old_ul.x - x;
    dy = old_ul.y - y;

  /* adjust pane coordinate system */
    R(tv)->viewRect = relocRect(R(tv)->viewRect, makePoint(x,y));

  /* update the screen */
    shift(tv,R(tv)->viewRect,dx,dy,false);

    if( dx != 0 )
      { paintRect = R(tv)->viewRect;
        if( dx > 0 )
          paintRect.lr.x = paintRect.ul.x + dx - 1;
        else
          paintRect.ul.x = paintRect.lr.x + dx - 1;
        paint(tv,paintRect,false);
      }

    if( dy != 0 )
      { paintRect = R(tv)->viewRect;
        if( dy > 0 )
          paintRect.lr.y = paintRect.ul.y + dy - 1;
        else
          paintRect.ul.y = paintRect.lr.y + dy - 1;
        paint(tv,paintRect,false);
      }

    touchPane(tv);
}




/*ARGSUSED*/

static
void scrollProc(TextView tv, ScrollBar sb, int dir, int value)
{
  if( dir == SB_HORIZONTAL )
    scrollTo(tv,value,R(tv)->viewRect.ul.y);
  else
    { guessVertScroll(tv,&value);
      scrollTo(tv,R(tv)->viewRect.ul.x,value);
    }

  setScrollBars(tv,true);
}




static
void guessVertScroll(TextView tv, int* y_request)
{
  int delta,y,old_y,y_step,y_max,old_y_max,dummy;
  float frac;

  y = *y_request;

  delta = y - R(tv)->viewRect.ul.y;
  scrollMax(tv,&dummy,&dummy,&y_max,&y_step);
  if( delta > y_step )    /* decreasing 'y' needs no guess */
    { frac = ((float) y) / y_max;
      vf_ViewToContents(R(tv)->filter,99999,&dummy);
      scrollMax(tv,&dummy,&dummy,&y_max,&dummy);
      y = frac * y_max;
/************************************************
      do
        { old_y = y;
          vf_ViewToContents(R(tv)->filter,old_y,&y);
          old_y_max = y_max;
          scrollMax(tv,&dummy,&dummy,&y_max,&dummy);
          y = frac * y_max;
        }  while( (((float) y_max) / old_y_max) > 1.2 );
*********************************************************/
    }

  *y_request = y;
}




static
void paint(TextView tv, Rectangle rect, Boolean touch)
{
  TextString text;
  TV_Data data;
  int left, top, right, bottom, k;

  /* inflate vertically so text_sm top & bottom slop are painted  */
  /*   NB -- won't work horizontally, so side slop is not painted */
    left   = max(rect.ul.x,   R(tv)->viewRect.ul.x);
    top    = max(rect.ul.y-1, R(tv)->viewRect.ul.y-1);
    right  = min(rect.lr.x,   R(tv)->viewRect.lr.x);
    bottom = min(rect.lr.y+1, R(tv)->viewRect.lr.y+1);

  for( k = top;  k <= bottom;  k++ )
    { getLine(tv,k,&text,&data);
      paintLine(tv,k,left,right,text,&data);
      destroyTextString(text);
    }

  customize(tv, top, bottom);

  if( touch )
    sm_text_lines_touch(R(tv)->textPane, c2p_y(tv,top), c2p_y(tv,bottom));
}




static
void paintLine(TextView tv, int c_lineNum, int c_left, int c_right, 
               TextString text, TV_Data* data)
/* c_lineNum -  in contents coords */
/* c_left,c_right -  in contents coords */
{
  TextString clipped;
  int lineNum,left,right,clippedRight;	/* in pane coords */
  Point pos;				/* in pane coords */
  Rectangle clearRect;			/* in pane coords */
  int selLine, sel1, sel2;
  Boolean inverse;

  /* must clear lines outside of viewRect because of text_sm slop */
    if( c_lineNum < R(tv)->viewRect.ul.y  ||
        c_lineNum > R(tv)->viewRect.lr.y )
      { lineNum = c2p_y(tv, c_lineNum);
        setRect(&clearRect, 0, lineNum, 9999, lineNum);
        sm_text_block_clear(R(tv)->textPane, clearRect);
        return;
      }

  /* convert to pane coords */
    lineNum = c2p_y(tv,c_lineNum);
    left    = c2p_x(tv,c_left);
    right   = c2p_x(tv,c_right);

  /* must clear left and right dregs of line explicitly */
    /* NB -- clearing would not always be the right thing to do, */
    /*       except that partial lines are only painted in       */
    /*       response to scrolling (i.e. not exposure).          */
    /* Also, this relies on the fact that it is OK to "clear"    */
    /*       outside the text_sm's char grid.                    */

    setRect(&clearRect, -1, lineNum, 0, lineNum);
    sm_text_block_clear(R(tv)->textPane, clearRect);
    setRect(&clearRect, sm_text_size(R(tv)->textPane).x, lineNum, 9999, lineNum);
    sm_text_block_clear(R(tv)->textPane, clearRect);

  /* clip 'text' because text_sm forbids writing outside its "map" */
    clipped = clipText(text,c_left,c_right);
    clippedRight = left + clipped.num_tc - 1;
  
  /* display visible part of selection in inverse video */
    getSelection(tv, &selLine, &sel1, &sel2);
    if( selLine != UNUSED )
      { if( selLine == c_lineNum )
          if( sel2 >= sel1 )
            setInverse(clipped, data, sel1, sel2, true);
      }
    else
      { if( sel1 <= c_lineNum  &&  c_lineNum <= sel2 )
          setInverse(clipped, data, 0, c_right, false);
      }

  /* paint the clipped text and clear trailing whitespace */
    pos = makePoint(left,lineNum);
    if( data->multiColored )
      putMultiColorString(R(tv)->textPane, pos, clipped, data, c_left);
    else
      sm_text_string_put_color(R(tv)->textPane, pos, clipped, TSM_NEVER,
                               data->all.foreground, data->all.background);
    if( clippedRight < right )
      { setRect(&clearRect, clippedRight+1, lineNum, right, lineNum);
        inverse = BOOL( selLine == UNUSED && sel1 <= c_lineNum && c_lineNum <= sel2 );
        sm_text_block_clear_color(R(tv)->textPane, clearRect,
                                  data->all.foreground, data->all.background);
      }

  /* display an insertion point if necessary */
    if( selLine == c_lineNum )
      if( sel2 == sel1-1 )
        if( c2p_x(tv,sel1) <= right )
          reverseIP( tv, lineNum, c2p_x(tv,sel1) );
}




static
void putMultiColorString(Pane* textPane, Point pos, TextString ts, 
                         TV_Data* data, int dataOffset)
{
  int k, start, len, total_len;
  TV_ColorPair color, color_k;

# define COLOR(k)    (k + dataOffset < tv_MAX_CHARS ? data->chars[k + dataOffset] : tv_defaultColorPair)

  total_len = ts.num_tc;
  k = 0;
  while( k < total_len )
    { /* find the next run of same colors */
        start = k;
        color = COLOR(start);
        while( k < total_len &&
               (color_k = COLOR(k),
                color_k.foreground == color.foreground &&
                color_k.background == color.background) )
          k += 1;
        len = k - start;

      /* output the run */
        ts.num_tc = len;
        sm_text_string_put_color(textPane, pos, ts, TSM_NEVER,
                                 color.foreground, color.background);

      /* advance past the run */
        ts.tc_ptr += len;
        pos.x     += len;
    }
}




static
void setInverse(TextString ts, TV_Data* td, int left, int right, Boolean partialLine)
{
  int first, last, k;
  Color temp;
  TextData *tData;

# define SPECIAL(c)		(c != black_color && c != white_color)
# define ALL			td->all
# define CHAR_K			td->chars[k]
# define USE_FG_ON_BLACK(x)	(x.background = black_color)
# define USE_BG_ON_BLACK(x)	(x.foreground = x.background,			\
				 x.background = black_color)
# define USE_INVERSE(x)		(temp = x.foreground,				\
				 x.foreground = x.background,			\
				 x.background = temp)

  if( partialLine && ! td->multiColored )
    {
     tData = (TextData*)td;
     Text_MultiColoredData(*tData);
    }

  if( td->multiColored )
    { /* calculate char-by-char inverse colors */
        first = left;
        last  = min(right, tv_MAX_CHARS-1);
        for( k = first;  k <= last;  k++ )
          if( SPECIAL(CHAR_K.foreground) && ! SPECIAL(CHAR_K.background) )
            USE_FG_ON_BLACK(CHAR_K);
          else if( SPECIAL(CHAR_K.background) && ! SPECIAL(CHAR_K.foreground) )
            USE_BG_ON_BLACK(CHAR_K);
          else
            USE_INVERSE(CHAR_K);
    }

  if( ! partialLine )
    { /* calculate overall inverse colors for trailing whitespace */
        if( SPECIAL(ALL.foreground) && ! SPECIAL(ALL.background) )
          USE_FG_ON_BLACK(ALL);
        else if( SPECIAL(ALL.background) && ! SPECIAL(ALL.foreground) )
          USE_BG_ON_BLACK(ALL);
        else
          USE_INVERSE(ALL);
    }
}




static
TextString clipText(TextString text, int left, int right)
{
  TextString clipped;
  int width = right - left + 1;

  clipped.num_tc = max(0, min(width, text.num_tc - left));
  clipped.tc_ptr = text.tc_ptr + left;
  clipped.ephemeral = text.ephemeral;

  return clipped;
}




static
void shift(TextView tv, Rectangle rect, int dx, int dy, Boolean touch)
/* rect - in contents coords */
{
  Rectangle srcRect;
  Point dstPt;

  /* inflate srcRect so text_sm slop is updated */
    srcRect = c2p_rect(tv,rect);
    srcRect.ul.x -= 1;
    srcRect.ul.y -= 1;
    srcRect.lr.x += 1;
    srcRect.lr.y += 1;

  dstPt = transPoint(srcRect.ul, makePoint(dx,dy));

  sm_text_block_copy(R(tv)->textPane,srcRect,dstPt,true);
  if( touch )
    sm_text_block_touch(R(tv)->textPane,relocRect(srcRect,dstPt));
}




static
void reverseIP(TextView tv, int line, int char1)
{
  int char2 = char1 - 1;
  Rectangle textRect,pixelRect;

  setRect(&textRect,char1,line,char2,line);
  pixelRect = sm_text_rect_gp(R(tv)->textPane,textRect);
  pixelRect.lr.x = pixelRect.ul.x;    /* empty --> 1 pixel wide */

  boxPane(tv, pixelRect, 1, NULL_BITMAP, Origin, true);
}




static
void trackMouse(TextView tv)
{
  Point mousePt;		/* in pixel coords */
  Point textPt1,textPt2;	/* in contents coords */

  /* make initial selection */
    mousePt = mon_event.loc;
    textPt1 = textPoint(tv,mousePt);
    switch( R(tv)->selBehavior )
      { case tv_SB_NORMAL:
          setSelection(tv, textPt1.y, textPt1.x, textPt1.x-1);
          break;
        case tv_SB_LINES_ONLY:
          setSelection(tv,UNUSED,textPt1.y,textPt1.y);
          break;
      }

  /* follow the mouse extending the selection */
    
    while( stillDown(&mousePt) )
      { textPt2 = textPoint(tv,mousePt);

	switch( R(tv)->selBehavior )
          { case tv_SB_NORMAL:
              if( textPt1.y == textPt2.y )
                setSelection( tv, textPt1.y,
                                  min(textPt1.x,textPt2.x),
                                  max(textPt1.x,textPt2.x)-1 );
              else
                if( textPt1.y < textPt2.y )
                  setSelection(tv,UNUSED,textPt1.y,textPt2.y-1);
                else
                  setSelection(tv,UNUSED,textPt2.y+1,textPt1.y);
              break;
            case tv_SB_LINES_ONLY:
              setSelection(tv,UNUSED,textPt2.y,textPt2.y);
              break;
          }

        tv_EnsureVisible(tv,textPt2,false);
      }
}




static
Point textPoint(TextView tv, Point pt)
/* pt - in pixel coords */
{
  Point adjustedPt;

  adjustedPt = makePoint(pt.x + R(tv)->charWidth/2, pt.y);
  return p2c_pt( tv, sm_text_point_pg(R(tv)->textPane,adjustedPt,TSM_NO_CLIP) );
}




static
int c2p_x(TextView tv, int colNum)
{
  return colNum - R(tv)->viewRect.ul.x;
}




static
int c2p_y(TextView tv, int lineNum)
{
  return lineNum - R(tv)->viewRect.ul.y;
}




static
Point c2p_pt(TextView tv, Point pt)
{
  return subPoint(pt, R(tv)->viewRect.ul);
}




static
Rectangle c2p_rect(TextView tv, Rectangle c_rect)
{
  return subRect(c_rect, R(tv)->viewRect.ul);
}




static
int p2c_x(TextView tv, int colNum)
{
  return colNum + R(tv)->viewRect.ul.x;
}




static
int p2c_y(TextView tv, int lineNum)
{
  return lineNum + R(tv)->viewRect.ul.y;
}




static
Point p2c_pt(TextView tv, Point pt)
{
  return transPoint(pt, R(tv)->viewRect.ul);
}




static
Rectangle p2c_rect(TextView tv, Rectangle p_rect)
{
  return transRect(p_rect, R(tv)->viewRect.ul);
}




static
void getDocSize(TextView tv, Point* size)
{
  ViewFilter filter = R(tv)->filter;

  if( filter == UNUSED )
    (R(tv)->methods->getDocSize) (R(tv)->contents,size);
  else
    vf_GetDocSize(filter,size);
}




static
void getLine(TextView tv, int lineNum, TextString* text, TV_Data* data)
{
  ViewFilter filter = R(tv)->filter;

  if( filter == UNUSED )
    (R(tv)->methods->getLine) (R(tv)->contents, lineNum, text, (Generic)data);
  else
    vf_GetLine(filter,lineNum,text,data);
}




static
void getSelection(TextView tv, int* line, int* sel1, int* sel2)
{
  ViewFilter filter = R(tv)->filter;

  if( R(tv)->selCached )
    { *line = R(tv)->selLine;
      *sel1 = R(tv)->sel1;
      *sel2 = R(tv)->sel2;
    }
  else
    { if( filter == UNUSED )
        (R(tv)->methods->getSelection) (R(tv)->contents,line,sel1,sel2);
      else
        vf_GetSelection(filter,line,sel1,sel2);

      R(tv)->selCached = true;
      R(tv)->selLine   = *line;
      R(tv)->sel1      = *sel1;
      R(tv)->sel2      = *sel2;
    }
}




static
void setSelection(TextView tv, int line, int sel1, int sel2)
{
  ViewFilter filter = R(tv)->filter;

  if( filter == UNUSED )
    (R(tv)->methods->setSelection) (R(tv)->contents,line,sel1,sel2);
  else
    vf_SetSelection(filter,line,sel1,sel2);
}




static
void customize(TextView tv, int c_line1, int c_line2)
{
  int p_line1 = c2p_y(tv, c_line1);
  int p_line2 = c2p_y(tv, c_line2);

  if( R(tv)->repaintProc != nil )
    (R(tv)->repaintProc) (R(tv)->repaintOb, R(tv)->textPane, R(tv)->filter,
                          &R(tv)->viewRect, p_line1, p_line2);
}
