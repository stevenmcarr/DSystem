/* $Id: scroll_sm.C,v 1.1 1997/06/25 14:59:20 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	scroll_sm.c							*/
/*									*/
/*	scroll -- screen module to control scrolling of another pane	*/
/*	Last edited: August 10, 1989 at 3:12 pm				*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/oldMonitor/include/mon/sm_def.h>
#include <libs/graphicInterface/oldMonitor/include/sms/scroll_sm.h>







/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/



typedef struct
  {
    /* control state */
      short direction;
      sm_scroll_scrollee_callback scrollProc;
      Generic scrollee;		/* a Pane * really */
      Boolean active;

    /* settings */
      int minVal;
      int maxVal;
      int curVal;
      int slowStep;
      int fastStep;

    /* auxiliary state */
      Boolean valueLimits;
      int valueRange;
      int length;
      int thumbRange;
      int thumbPos;
  } sb_Repr;


#define	PANE(sb)	((Pane *) sb)
#define	R(sb)		((sb_Repr *) PANE(sb)->pane_information)






/************************/
/* Screen module	*/
/************************/




/* these must be declared early for sm structure below */

STATIC(void, init,(void));
STATIC(void, fini,(void));
STATIC(void, create,(ScrollBar sb));
STATIC(void, destroy,(ScrollBar sb));
STATIC(void, resize,(ScrollBar sb));
STATIC(void, input,(ScrollBar sb, Rectangle r));


static aScreenModule sb_screenModuleOps =
{
  "scroll bar",
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






/************************/
/* Graphical appearance	*/
/************************/



#define SB_ARROW_LENGTH		20
#define SB_THUMB_LENGTH		 8


static Bitmap   white_pattern;

/* light ltGray pattern */

static Bitmap   ltGray_pattern;



/* arrow icon tables */

static Bitmap sb_arrow[2][2][2];
static Bitmap sb_noArrow[2][2][2];


/* colors */

static Color sb_fg;
static Color sb_bg;



/************************/
/* Mouse tracking	*/
/************************/



static Point sb_mousePt;





/************************/
/* Forward declarations	*/
/************************/



STATIC(void,		minmax,(ScrollBar sb, int *val));
STATIC(void,		recalc,(ScrollBar sb, Boolean repaint, Boolean touch));
STATIC(void,		paint,(ScrollBar sb, Boolean paintAll, Boolean touch));
STATIC(void,		blitIcon,(ScrollBar sb, Bitmap image, Rectangle rect,
                                  Point patternOrigin, Boolean invert));
STATIC(int,		val2pos,(ScrollBar sb, int val));
STATIC(int,		pos2val,());
STATIC(Rectangle,	partRect,());
STATIC(Rectangle,	bodyRect,(ScrollBar sb));
STATIC(Rectangle,	thumbRect,(ScrollBar sb, int val));
STATIC(Rectangle,	arrowRect,(ScrollBar sb, Boolean forward, Boolean fast));
STATIC(Rectangle,	insideArrowRect,());
STATIC(void,		shrink,(Rectangle *r, int left, int top, int right, 
                                int bottom));
STATIC(Boolean,		findArrow,(ScrollBar sb, Point pt, Boolean *forward, 
                                   Boolean *fast));
STATIC(void,		trackArrow,(ScrollBar sb, Boolean forward, Boolean fast));
STATIC(void,		trackThumb,(ScrollBar sb));
STATIC(Boolean,		setval,(ScrollBar sb, int newVal));
STATIC(int,		mainAxis,(ScrollBar sb, Point pt));
STATIC(Boolean,		stillDown,(Point *mousePt));





/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/




/************************/
/*  Screen module	*/
/************************/




short sm_scroll_get_index()
{
  return getScreenModuleIndex(&sb_screenModuleOps);
}






/************************/
/*  Scrolling control	*/
/************************/


void sm_scroll_activate(ScrollBar sb, Boolean activeNow)
//   ScrollBar sb;
//   Boolean activeNow;
{
    Boolean wasActive = R(sb)->active;

    R(sb)->active = activeNow;
    if( wasActive != activeNow ) paint(sb, true, true);
}




void sm_scroll_scrollee(ScrollBar sb,Generic scrollee,
			sm_scroll_scrollee_callback scrollProc)
//   ScrollBar sb;
//   Generic scrollee;	/* a 'Pane *' really */
//   sm_scroll_scrollee_callback scrollProc;
{
  R(sb)->scrollee   = scrollee;
  R(sb)->scrollProc = scrollProc;
}






/************************/
/*  Scroll position	*/
/************************/




void sm_scroll_get(ScrollBar sb, int *minVal, int *maxVal, int *curVal)
//   ScrollBar sb;
//   int *minVal;
//   int *maxVal;
//   int *curVal;

{
  *minVal = R(sb)->minVal;
  *maxVal = R(sb)->maxVal;
  *curVal = R(sb)->curVal;
}




void sm_scroll_set(ScrollBar sb, int minVal, int maxVal, int curVal, Boolean touch)
//   ScrollBar sb;
//   int minVal;
//   int maxVal;
//   int curVal;
//   Boolean touch;
{
  if( maxVal < minVal )  maxVal = minVal;
  R(sb)->minVal = minVal;
  R(sb)->maxVal = maxVal;

  minmax(sb, &curVal);
  R(sb)->curVal = curVal;

  recalc(sb,true,touch);
}




void sm_scroll_set_step(ScrollBar sb, int slowStep, int fastStep)
//   ScrollBar sb;
//   int slowStep;
//   int fastStep;
{
  R(sb)->slowStep = slowStep;
  R(sb)->fastStep = fastStep;
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




static
void init()
{
static BITMAPM_UNIT ltGray_data[] =
  { 0x8888, 0x2222, 0x8888, 0x2222, 0x8888, 0x2222, 0x8888, 0x2222,
    0x8888, 0x2222, 0x8888, 0x2222, 0x8888, 0x2222, 0x8888, 0x2222
  };
static BITMAPM_UNIT leftSlow_data[]  =
  { 0x0000,0x3000,   0x0000,0x3000,   0x0018,0x3000,   0x0078,0x3000,
    0x01f8,0x3000,   0x07f8,0x3000,   0x0ff8,0x3000,   0x07f8,0x3000,
    0x01f8,0x3000,   0x0078,0x3000,   0x0018,0x3000,   0x0000,0x3000,
    0x0000,0x3000
  };
static BITMAPM_UNIT leftFast_data[]  =
  { 0x0000,0x3000,   0x0000,0x3000,   0x0000,0x3000,   0x0183,0x3000,
    0x078f,0x3000,   0x1fbf,0x3000,   0x3fff,0x3000,   0x1fbf,0x3000,
    0x078f,0x3000,   0x0183,0x3000,   0x0000,0x3000,   0x0000,0x3000,
    0x0000,0x3000
  };
static BITMAPM_UNIT rightSlow_data[] =
  { 0xc000,0x0000,   0xc000,0x0000,   0xc180,0x0000,   0xc1e0,0x0000,
    0xc1f8,0x0000,   0xc1fe,0x0000,   0xc1ff,0x0000,   0xc1fe,0x0000,
    0xc1f8,0x0000,   0xc1e0,0x0000,   0xc180,0x0000,   0xc000,0x0000,
    0xc000,0x0000
  };
static BITMAPM_UNIT rightFast_data[] =
  { 0xc000,0x0000,   0xc000,0x0000,   0xc000,0x0000,   0xcc18,0x0000,
    0xcf1e,0x0000,   0xcfdf,0x8000,   0xcfff,0xc000,   0xcfdf,0x8000,
    0xcf1e,0x0000,   0xcc18,0x0000,   0xc000,0x0000,   0xc000,0x0000,
    0xc000,0x0000
  };
static BITMAPM_UNIT upSlow_data[]  =
  { 0x0000, 0x0000, 0x0000, 0x0000, 0x0200, 0x0700, 0x0700, 0x0f80, 0x0f80,
    0x1fc0, 0x1fc0, 0x3fe0, 0x3fe0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0xffff, 0xffff
  };
static BITMAPM_UNIT upFast_data[]  =
  { 0x0000, 0x0000, 0x0200, 0x0700, 0x0700, 0x0f80, 0x0f80, 0x1fc0, 0x1fc0,
    0x0200, 0x0700, 0x0700, 0x0f80, 0x0f80, 0x1fc0, 0x1fc0, 0x0000, 0x0000,
    0xffff, 0xffff
  };
static BITMAPM_UNIT downSlow_data[]  =
  { 0xffff, 0xffff,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x3fe0, 0x3fe0, 0x1fc0, 0x1fc0, 
    0x0f80, 0x0f80, 0x0700, 0x0700, 0x0200, 0x0000, 0x0000, 0x0000, 0x0000
  };
static BITMAPM_UNIT downFast_data[]  =
  { 0xffff, 0xffff,
    0x0000, 0x0000, 0x1fc0, 0x1fc0, 0x0f80, 0x0f80, 0x0700, 0x0700, 0x0200,
    0x1fc0, 0x1fc0, 0x0f80, 0x0f80, 0x0700, 0x0700, 0x0200, 0x0000, 0x0000
  };
static BITMAPM_UNIT noLeftSlow_data[]  =
  { 0x0000,0x3000,   0x0000,0x3000,   0x0000,0x3000,   0x0000,0x3000,
    0x0000,0x3000,   0x0000,0x3000,   0x0000,0x3000,   0x0000,0x3000,
    0x0000,0x3000,   0x0000,0x3000,   0x0000,0x3000,   0x0000,0x3000,
    0x0000,0x3000
  };
static BITMAPM_UNIT noLeftFast_data[]  =
  { 0x0000,0x3000,   0x0000,0x3000,   0x0000,0x3000,   0x0000,0x3000,
    0x0000,0x3000,   0x0000,0x3000,   0x0000,0x3000,   0x0000,0x3000,
    0x0000,0x3000,   0x0000,0x3000,   0x0000,0x3000,   0x0000,0x3000,
    0x0000,0x3000
  };
static BITMAPM_UNIT noRightSlow_data[] =
  { 0xc000,0x0000,   0xc000,0x0000,   0xc000,0x0000,   0xc000,0x0000,
    0xc000,0x0000,   0xc000,0x0000,   0xc000,0x0000,   0xc000,0x0000,
    0xc000,0x0000,   0xc000,0x0000,   0xc000,0x0000,   0xc000,0x0000,
    0xc000,0x0000
  };
static BITMAPM_UNIT noRightFast_data[] =
  { 0xc000,0x0000,   0xc000,0x0000,   0xc000,0x0000,   0xc000,0x0000,
    0xc000,0x0000,   0xc000,0x0000,   0xc000,0x0000,   0xc000,0x0000,
    0xc000,0x0000,   0xc000,0x0000,   0xc000,0x0000,   0xc000,0x0000,
    0xc000,0x0000
  };
static BITMAPM_UNIT noUpSlow_data[]  =
  { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0xffff, 0xffff
  };
static BITMAPM_UNIT noUpFast_data[]  =
  { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0xffff, 0xffff
  };
static BITMAPM_UNIT noDownSlow_data[]  =
  { 0xffff, 0xffff,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
  };
static BITMAPM_UNIT noDownFast_data[]  =
  { 0xffff, 0xffff,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
  };

  white_pattern = makeBitmap(makePoint(16,16), "scroll_sm: white_pattern");
  BLTBitmap(NULL_BITMAP, white_pattern, makeRectFromSize(Origin, getBitmapSize(white_pattern)),
	  Origin, BITMAP_CLR, false);	/* use the pane's background color	*/

  ltGray_pattern = makeBitmapFromData(makePoint(16, 16), ltGray_data, "scroll_sm.c");

        /* direction, forward, fast */
  sb_arrow  [SB_HORIZONTAL][0][0] = makeBitmapFromData(makePoint(20, 13), leftSlow_data,    "scroll_sm.c");
  sb_arrow  [SB_HORIZONTAL][0][1] = makeBitmapFromData(makePoint(20, 13), leftFast_data,    "scroll_sm.c");
  sb_arrow  [SB_HORIZONTAL][1][0] = makeBitmapFromData(makePoint(20, 13), rightSlow_data,   "scroll_sm.c");
  sb_arrow  [SB_HORIZONTAL][1][1] = makeBitmapFromData(makePoint(20, 13), rightFast_data,   "scroll_sm.c");
  sb_arrow  [SB_VERTICAL  ][0][0] = makeBitmapFromData(makePoint(13, 20), upSlow_data,      "scroll_sm.c");
  sb_arrow  [SB_VERTICAL  ][0][1] = makeBitmapFromData(makePoint(13, 20), upFast_data,      "scroll_sm.c");
  sb_arrow  [SB_VERTICAL  ][1][0] = makeBitmapFromData(makePoint(13, 20), downSlow_data,    "scroll_sm.c");
  sb_arrow  [SB_VERTICAL  ][1][1] = makeBitmapFromData(makePoint(13, 20), downFast_data,    "scroll_sm.c");

        /* direction, forward, fast */
  sb_noArrow[SB_HORIZONTAL][0][0] = makeBitmapFromData(makePoint(20, 13), noLeftSlow_data,  "scroll_sm.c");
  sb_noArrow[SB_HORIZONTAL][0][1] = makeBitmapFromData(makePoint(20, 13), noLeftFast_data,  "scroll_sm.c");
  sb_noArrow[SB_HORIZONTAL][1][0] = makeBitmapFromData(makePoint(20, 13), noRightSlow_data, "scroll_sm.c");
  sb_noArrow[SB_HORIZONTAL][1][1] = makeBitmapFromData(makePoint(20, 13), noRightFast_data, "scroll_sm.c");
  sb_noArrow[SB_VERTICAL  ][0][0] = makeBitmapFromData(makePoint(13, 20), noUpSlow_data,    "scroll_sm.c");
  sb_noArrow[SB_VERTICAL  ][0][1] = makeBitmapFromData(makePoint(13, 20), noUpFast_data,    "scroll_sm.c");
  sb_noArrow[SB_VERTICAL  ][1][0] = makeBitmapFromData(makePoint(13, 20), noDownSlow_data,  "scroll_sm.c");
  sb_noArrow[SB_VERTICAL  ][1][1] = makeBitmapFromData(makePoint(13, 20), noDownFast_data,  "scroll_sm.c");

	/* colors */
  sb_fg = getColorFromName("scrollBar.foreground");
  sb_bg = getColorFromName("scrollBar.background");

}




static
void fini()
{
  freeBitmap(white_pattern);
  freeBitmap(ltGray_pattern);
  freeBitmap(sb_arrow  [SB_HORIZONTAL][0][0]);
  freeBitmap(sb_arrow  [SB_HORIZONTAL][0][1]);
  freeBitmap(sb_arrow  [SB_HORIZONTAL][1][0]);
  freeBitmap(sb_arrow  [SB_HORIZONTAL][1][1]);
  freeBitmap(sb_arrow  [SB_VERTICAL  ][0][0]);
  freeBitmap(sb_arrow  [SB_VERTICAL  ][0][1]);
  freeBitmap(sb_arrow  [SB_VERTICAL  ][1][0]);
  freeBitmap(sb_arrow  [SB_VERTICAL  ][1][1]);
  freeBitmap(sb_noArrow[SB_HORIZONTAL][0][0]);
  freeBitmap(sb_noArrow[SB_HORIZONTAL][0][1]);
  freeBitmap(sb_noArrow[SB_HORIZONTAL][1][0]);
  freeBitmap(sb_noArrow[SB_HORIZONTAL][1][1]);
  freeBitmap(sb_noArrow[SB_VERTICAL  ][0][0]);
  freeBitmap(sb_noArrow[SB_VERTICAL  ][0][1]);
  freeBitmap(sb_noArrow[SB_VERTICAL  ][1][0]);
  freeBitmap(sb_noArrow[SB_VERTICAL  ][1][1]);
}




static
void create(ScrollBar sb)
  // ScrollBar sb;
{
  PANE(sb)->border_width = 1;
  PANE(sb)->pattern = white_pattern;
  PANE(sb)->pane_information = (Generic)get_mem(sizeof(sb_Repr), "scroll bar");
  recolorPane(PANE(sb), sb_fg, sb_bg, default_border_color, false, false, false);

  R(sb)->direction = UNUSED;
  R(sb)->scrollee = UNUSED;
  R(sb)->active = true;

  R(sb)->minVal   = 0;
  R(sb)->maxVal   = 0;
  R(sb)->curVal   = 0;
  R(sb)->slowStep = 0;
  R(sb)->fastStep = 0;

  recalc(sb,false,false);
}





static
void destroy(ScrollBar sb)
  // ScrollBar sb;
{
  free_mem((void*) R(sb));
}




static
void resize(ScrollBar sb)
  //ScrollBar sb;
{
  if( PANE(sb)->size.x == 0  ||  PANE(sb)->size.y == 0 )
    R(sb)->direction = UNUSED;
  else if( PANE(sb)->size.x > PANE(sb)->size.y )
    R(sb)->direction = SB_HORIZONTAL;
  else
    R(sb)->direction = SB_VERTICAL;

  recalc(sb,false,false);
  paint(sb,true,false);
}




static
void input(ScrollBar sb, Rectangle r)
//   ScrollBar sb;
//   Rectangle r;
{
  Boolean forward,fast;

  while( (mon_event.type < MOUSE_KEYBOARD) && pointInRect(mon_event.loc, r) )
    { 
      switch( mon_event.type )
        {
          case MOUSE_DOWN:
            if( R(sb)->valueRange > 0 )
              { if( findArrow(sb,mon_event.loc,&forward,&fast) )
                  trackArrow(sb,forward,fast);
                else if( pointInRect(mon_event.loc,thumbRect(sb,R(sb)->curVal)) )
                  trackThumb(sb);
                else
                  getEvent();
              }
            else getEvent();
            break;

          default:
            getEvent();
            break;
        }
    }
}




static
void minmax(ScrollBar sb, int *val)
  //  ScrollBar sb;
  // int *val;
{
  if( R(sb)->minVal != UNUSED )
    *val = MAX( R(sb)->minVal, MIN(R(sb)->maxVal,*val) );
}




static
void recalc(ScrollBar sb, Boolean repaint, Boolean touch)
//   ScrollBar sb;
//   Boolean repaint;
//   Boolean touch;
{
  int oldThumbPos = R(sb)->thumbPos;
  int used;
  Boolean arrows;

  /* range of permitted values */
    R(sb)->valueLimits = BOOL(R(sb)->minVal != UNUSED);
    R(sb)->valueRange  = R(sb)->maxVal - R(sb)->minVal;
 
  /* length of scrollbar in pixels */
    R(sb)->length = (R(sb)->direction == SB_HORIZONTAL  ?  PANE(sb)->size.x
                                                        :  PANE(sb)->size.y);

  /* range of permitted thumb positions */
    used = 4 * SB_ARROW_LENGTH + SB_THUMB_LENGTH + 2;
    R(sb)->thumbRange = R(sb)->length - used;

  /* current thumb position */
    if( R(sb)->valueRange > 0 )
      R(sb)->thumbPos = val2pos(sb,R(sb)->curVal);
    else
      R(sb)->thumbPos = UNUSED;

    if( repaint && (R(sb)->thumbPos != oldThumbPos) )
      { arrows = BOOL( (R(sb)->thumbPos == UNUSED) || (oldThumbPos == UNUSED) );
        paint(sb,arrows,touch);
      }
}




static
void paint(ScrollBar sb,Boolean paintAll,Boolean touch)
//   ScrollBar sb;
//   Boolean paintAll;
//   Boolean touch;
{
  int dir = R(sb)->direction;
  int thumbPos = R(sb)->thumbPos;
  Boolean disabled = BOOL( (R(sb)->active == false ) || 
			   (R(sb)->valueLimits && R(sb)->valueRange == 0) );

  /* do nothing if zero-size */
    if( dir == UNUSED )  return;

  /* paint the arrows if requested */
    if( paintAll )
      if( disabled )
        { blitIcon(sb, sb_noArrow[dir][0][0], arrowRect(sb,false,false), arrowRect(sb,false,false).ul, false);
          blitIcon(sb, sb_noArrow[dir][0][1], arrowRect(sb,false,true ), arrowRect(sb,false,true ).ul, false);
          blitIcon(sb, sb_noArrow[dir][1][0], arrowRect(sb,true, false), arrowRect(sb,true, false).ul, false);
          blitIcon(sb, sb_noArrow[dir][1][1], arrowRect(sb,true, true ), arrowRect(sb,true, true ).ul, false);
        }
      else
        { blitIcon(sb, sb_arrow[dir][0][0], arrowRect(sb,false,false), arrowRect(sb,false,false).ul, false);
          blitIcon(sb, sb_arrow[dir][0][1], arrowRect(sb,false,true ), arrowRect(sb,false,true ).ul, false);
          blitIcon(sb, sb_arrow[dir][1][0], arrowRect(sb,true, false), arrowRect(sb,true, false).ul, false);
          blitIcon(sb, sb_arrow[dir][1][1], arrowRect(sb,true, true ), arrowRect(sb,true, true ).ul, false);
        }

  /* paint the body */
    if( disabled )
      ColorPaneWithPattern(PANE(sb), bodyRect(sb), white_pattern, Origin, true);
    else
      {
	ColorPaneWithPattern(PANE(sb), bodyRect(sb), ltGray_pattern, Origin, true);
        if( thumbPos != UNUSED )
	  boxPaneColor(PANE(sb), thumbRect(sb,R(sb)->curVal),
		  sizeRect(thumbRect(sb,R(sb)->curVal)).x,
		  NULL_BITMAP, Origin, true, paneForeground(PANE(sb)), NULL_COLOR);
      }

  if( touch )
    touchPane(PANE(sb));
}




static
void
blitIcon(ScrollBar sb, Bitmap image, Rectangle rect, Point patternOrigin, 
	 Boolean invert)
//      ScrollBar sb;
//      Bitmap image;		/* pattern to place in rect, usually only one copy	*/
//      Rectangle rect;		/* location in sb's pane for the pattern		*/
//      Point patternOrigin;	/* pattern tiling's upper left corner			*/

//      Boolean invert;		/* true if the icon should be inverted 			*/
{
  /* When using the pattern to fill the rect, the pattern is conceputally tiled
   * over the entire scroll bar pane with the pattern's upper left corner at
   * patternOrigin.  Only the tiling within rect actually changes the pane.	*/

  /* ASSERT: rect is within PANE(sb). */
  ColorPaneWithPatternColor(PANE(sb),
		       rect,
		       image,
		       patternOrigin,
		       false,  /* ASSERT */
		       invert ? paneBackground(PANE(sb)) : NULL_COLOR,
		       invert ? paneForeground(PANE(sb)) : NULL_COLOR);
  touchPaneRect(PANE(sb), rect);
  return;
}




static
int val2pos(ScrollBar sb, int val)
  // ScrollBar sb;
  // int val;
{
  float pixel0 = 1 + 2 * SB_ARROW_LENGTH;
  float delta  = val - R(sb)->minVal;
  float pixels = R(sb)->thumbRange;
  float range  = R(sb)->valueRange;

  return (int) ( pixel0  +  ((delta * pixels) / range) + 0.5 );
}




static
int pos2val(ScrollBar sb, int pos)
  // ScrollBar sb;
  // int pos;
{
  int pixel0 = 1 + 2 * SB_ARROW_LENGTH;
  float delta  = pos - pixel0;
  float pixels = R(sb)->thumbRange;
  float range  = R(sb)->valueRange;

  return  (int)(R(sb)->minVal + ((delta * range) / pixels) + 0.5 );
}




static
Rectangle partRect(ScrollBar sb, int position, int length)
  // ScrollBar sb;
  // int position, length;
{
  Rectangle r;

  if( R(sb)->direction == SB_HORIZONTAL )
    r = makeRectFromSize(makePoint(position,1), makePoint(length,SB_WIDTH-2));
  else
    r = makeRectFromSize(makePoint(1,position), makePoint(SB_WIDTH-2,length));

  return r;
}




static
Rectangle bodyRect(ScrollBar sb)
  // ScrollBar sb;
{
  int arrowsLength = 1 + 2 * SB_ARROW_LENGTH;

  return partRect(sb, arrowsLength, R(sb)->length - 2 * arrowsLength);
}




static
Rectangle thumbRect(ScrollBar sb, int val)
  // ScrollBar sb;
  // int val;
{
  return partRect(sb, val2pos(sb,val), SB_THUMB_LENGTH);
}




static
Rectangle arrowRect(ScrollBar sb, Boolean forward, Boolean fast)
  // ScrollBar sb;
  // Boolean forward,fast;
{
  int end = R(sb)->length;
  int arrowPos;

  if( forward )
    arrowPos = (fast  ?  end - 1 - 2 * SB_ARROW_LENGTH
                      :  end - 1 - SB_ARROW_LENGTH);
  else
    arrowPos = (fast  ?  1 + SB_ARROW_LENGTH
                      :  1);

  return partRect(sb,arrowPos,SB_ARROW_LENGTH);
}




static
Rectangle insideArrowRect(ScrollBar sb, Boolean forward, Boolean fast)
  // ScrollBar sb;
  // Boolean forward,fast;
{
  Boolean horizontal = BOOL(R(sb)->direction == SB_HORIZONTAL);
  Rectangle rect;

  rect = arrowRect(sb,forward,fast);
  if( !horizontal && !forward )  shrink(&rect,0,0,0,2);
  if( !horizontal &&  forward )  shrink(&rect,0,2,0,0);
  if(  horizontal && !forward )  shrink(&rect,0,0,2,0);
  if(  horizontal &&  forward )  shrink(&rect,2,0,0,0);

  return rect;
}




static
void shrink(Rectangle *r, int left, int top, int right, int bottom)
  // Rectangle *r;
  // int left,top,right,bottom;
{
  r->ul.x += left;
  r->ul.y += top;
  r->lr.x -= right;
  r->lr.y -= bottom;
}




static
Boolean findArrow(ScrollBar sb, Point pt, Boolean *forward, Boolean *fast)
  // ScrollBar sb;
  // Point pt;
  // Boolean *forward, *fast;
{
  static Boolean forTable[4] = {false,true, false,true };
  static Boolean fasTable[4] = {false,false,true, true };
  int k;

  for( k = 0;  k <= 3;  k++ )
    { *forward = forTable[k];  *fast = fasTable[k];
      if( pointInRect(pt,insideArrowRect(sb,*forward,*fast)) )  return true;
    }

  return false;
}




static
void trackArrow(ScrollBar sb, Boolean forward, Boolean fast)
  // ScrollBar sb;
  // Boolean forward,fast;
{
  Rectangle rect;
  int step;
  Boolean hilited,inArrow;

  rect = insideArrowRect(sb,forward,fast);
  step = (forward ? 1 : -1) * (fast ? R(sb)->fastStep : R(sb)->slowStep);

  sb_mousePt = mon_event.loc;
  hilited = false;

  do
    { inArrow = pointInRect(sb_mousePt,rect);

      if( inArrow != hilited )
	{
	  blitIcon(sb, sb_arrow[R(sb)->direction == SB_VERTICAL][forward][fast],
		   rect, arrowRect(sb,forward,fast).ul, true);
          hilited = inArrow;
        }

      if( inArrow )
        { if( setval(sb, R(sb)->curVal + step) )
            (R(sb)->scrollProc)(R(sb)->scrollee, sb,
                                R(sb)->direction, R(sb)->curVal);
        }
    } while( stillDown(&sb_mousePt) );

  if (hilited)
    blitIcon(sb, sb_arrow[R(sb)->direction == SB_VERTICAL][forward][fast],
	     rect, arrowRect(sb,forward,fast).ul, false);
}




static
void trackThumb(ScrollBar sb)
  // ScrollBar sb;
{
  Rectangle barRect,feedbackRect;
  Boolean hilited,nearBar,changed;
  int slop,offset,lastVal,thisVal;

  /* calculate "slop" rect for thumb tracking */
    barRect = makeRectFromSize(Origin,PANE(sb)->size);
    slop = SB_WIDTH * 2;
    shrink(&barRect,-slop,-slop,-slop,-slop);

  /* note initial click */
    sb_mousePt = mon_event.loc;
    offset = R(sb)->thumbPos - mainAxis(sb,mon_event.loc);

  /* establish feedback*/
    feedbackRect = thumbRect(sb,R(sb)->curVal);
    blitIcon(sb, NULL_BITMAP, feedbackRect, Origin, true);
    hilited = true;
    lastVal = R(sb)->curVal;

  do
    { nearBar = pointInRect(sb_mousePt,barRect);
      thisVal = pos2val(sb, mainAxis(sb,sb_mousePt) + offset);
      minmax(sb,&thisVal);
      changed = BOOL( (nearBar != hilited) ||
                      (nearBar && (thisVal != lastVal)) );

      if( changed )
        { /* remove old feedback if any */
            if( hilited )
	      {
		blitIcon(sb, ltGray_pattern, feedbackRect, Origin, false);
                hilited = false;
              }

          /* add new feedback if any */
            if( nearBar )
              { feedbackRect = thumbRect(sb,thisVal);
		blitIcon(sb, NULL_BITMAP, feedbackRect, Origin, true);
                hilited = true;
                lastVal = thisVal;
              }
        }
    } while( stillDown(&sb_mousePt) );


  /* remove feedback*/
    if (hilited)
      blitIcon(sb, NULL_BITMAP, feedbackRect, Origin, false);

  /* generate the scroll request */
    if( nearBar )  
      { (void) setval(sb,thisVal);
        (R(sb)->scrollProc)(R(sb)->scrollee, sb,
                            R(sb)->direction, R(sb)->curVal);
      }
}




static
Boolean setval(ScrollBar sb, int newVal)
  // ScrollBar sb;
  // int newVal;
{
  int minVal,maxVal,oldVal;

  sm_scroll_get(sb,&minVal,&maxVal,&oldVal);
  sm_scroll_set(sb,minVal,maxVal,newVal,true);

  return BOOL( R(sb)->curVal != oldVal );
}



static
int mainAxis(ScrollBar sb, Point pt)
  // ScrollBar sb;
  // Point pt;
{
  return (R(sb)->direction == SB_HORIZONTAL  ?  pt.x  : pt.y);
}




static
Boolean stillDown(Point *mousePt)
  // Point *mousePt;
{
  if( readyEvent() )
    { getEvent();
      switch( mon_event.type )
        {
          case MOUSE_UP:
          case MOUSE_EXIT:
          case EVENT_KEYBOARD:
            return false;
          case MOUSE_DRAG:
            { *mousePt = mon_event.loc;
              return true;
            }
          default:
            die_with_message("Utility.stillDown: unexpected screen event.");
	    /*NOTREACHED*/
        }
    }
  else
    return true;
}
