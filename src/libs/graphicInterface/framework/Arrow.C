/* $Id: Arrow.C,v 1.2 1997/03/11 14:32:25 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/Arrow.C						*/
/*									*/
/*	Arrow -- Addition to a CTextView display			*/
/*	Last edited: November 11, 1993 at 1:20 am			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/framework/Arrow.h>

#include <libs/graphicInterface/framework/CTextView.h>
#include <libs/graphicInterface/framework/Decoration.h>

#include <math.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* Display of an arrow endpoint */

typedef struct EndDisplay_struct
  {
    /* requested appearance */
      Rectangle		bbox;		/* contents char coords */
      ColorPair		colors;

  } EndDisplay;




/* Display of an arrow */

typedef struct LineDisplay_struct
  {
    Point		e1;		/* view pixel coords */
    Point		e2;		/* view pixel coords */

  } LineDisplay;


typedef struct ArcDisplay_struct
  {
    Rectangle		bbox;		/* not used */
    double		angle;
    double		extent;

  } ArcDisplay;


typedef struct ShaftDisplay_struct
  {
    /* requested appearance */
      int		width;
      LineStyle *	style;
      Color		color;
    
    /* computed details */
      Rectangle		bbox;		/* view pixel coords */
      int		kind;
      LineDisplay	line1;
      ArcDisplay	arc;
      LineDisplay	line2;
      LineDisplay	head;		/* really just two points */

  } ShaftDisplay;




/* shaft kinds */

#define SHAFT_NONE	0
#define SHAFT_LINE	1
#define SHAFT_ARC	2
#define SHAFT_LOOP	3




/* Display of an arrow */

typedef struct ArrowDisplay_struct
  {
    Rectangle		bbox;		/* contents char coords */
    EndDisplay		src;
    EndDisplay		sink;
    ShaftDisplay	shaft;

  } ArrowDisplay;




/* Arrow object */

typedef struct Arrow_Repr_struct
  {
    /* appearance */
      ArrowDisplay	arrow;

  } Arrow_Repr;


#define R(ob)		(ob->Arrow_repr)

#define INHERITED	Decoration






/*************************/
/*  Miscellaneous	 */
/*************************/




/* display parameters */


#define SHAFT_HEAD_LENGTH	15
#define SHAFT_HEAD_ANGLE	20
#define MIN_SEPARATION		12
#define MAX_ARC_RADIUS		50
#define LOOP_RADIUS		15


typedef enum{ H, V }	Direction;
#define OPPOSITE(dir)	(dir == H ? V : H)


typedef int Side;

#define SIDE(dir,mag)	(2 * ((int)dir) + mag)

#define FIRST		0
#define LAST		1

#define LEFT		SIDE(H, FIRST)
#define RIGHT		SIDE(H, LAST )
#define TOP		SIDE(V, FIRST)
#define BOTTOM		SIDE(V, LAST )







/*************************/
/*  Forward declarations */
/*************************/




static Boolean separation(Rectangle box1, Rectangle box2,
                          Direction &dir, Side &side1, Side &side2);

static int side(Rectangle r, Side side);

static Point centerOfSide(Rectangle r, Side side);

static Point movePointTo(Point pt, Direction dir, int pos);

static Rectangle arcBox(Direction dir, Point p1, Point p2);

static float angle(Point p1, Point p2);

static Point v2s_point(Arrow * a, Point c_pt);

static Rectangle v2s_rect(Arrow * a, Rectangle c_rect);

static Rectangle expand(Rectangle r, int dx, int dy);






/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/






/*************************/
/*  Class initialization */
/*************************/




void Arrow::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(Decoration);
    REQUIRE_INIT(CTextView);
}




void Arrow::FiniClass(void)
{
  /* nothing */
}






/**********************/
/*  Instance creation */
/**********************/




Arrow * Arrow::Create(void)
{
  Arrow * a;
  
  a= new Arrow;
  a->Arrow::Init();
  a->Arrow::PostInit();
  return a;
}




/****************************/
/*  Instance initialization */
/****************************/




META_IMP(Arrow)




Arrow::Arrow(void)
          : Decoration()
{
  /* allocate instance's private data */
    this->Arrow_repr = (Arrow_Repr *) get_mem(sizeof(Arrow_Repr), "Arrow instance");
}




void Arrow::Init(void)
{  
  this->INHERITED::Init();
}




void Arrow::PostInit(void)
{
  /* nothing */
}




void Arrow::Destroy(void)
{
  this->INHERITED::Destroy();
}




Arrow::~Arrow()
{
  free_mem((void*) this->Arrow_repr);
}






/********************/
/*  View attachment */
/********************/




void Arrow::AttachView(CTextView * view)
{
  this->INHERITED::AttachView(view);
  this->calcAppearance();
}






/************/
/*  Drawing */
/************/




void Arrow::ColorizeLine(int c_linenum, TextString &text, TextData &data)
{
  this->colorizeEnd(c_linenum, true,  data);
  this->colorizeEnd(c_linenum, false, data);
}




void Arrow::Draw(void)
{
  EndDisplay &src     = R(this)->arrow.src;
  EndDisplay &sink    = R(this)->arrow.sink;
  ShaftDisplay &shaft = R(this)->arrow.shaft;
  CTextView * view = this->GetView();
  Point points[3];
  Rectangle src_bbox, sink_bbox;

# define Pt(p)           v2s_point(this, p)
# define Rt(r)           v2s_rect (this, r)

  if( shaft.kind == SHAFT_NONE )  return;
  
  view->SetDrawingStyle(shaft.width, shaft.style, shaft.color, NULL_COLOR);

  /* draw the first line */
    if( shaft.kind != SHAFT_LINE )
      view->DrawLine(Pt(shaft.line1.e1), Pt(shaft.line1.e2));

  /* draw the arc */
    if( shaft.kind != SHAFT_LINE )
      view->DrawArc(Rt(shaft.arc.bbox), shaft.arc.angle, shaft.arc.extent);

  /* draw the second line */
    view->DrawLine(Pt(shaft.line2.e1), Pt(shaft.line2.e2));

  /* draw the arrow head */
    points[0] = Pt(shaft.line2.e2);
    points[1] = Pt(shaft.head.e1);
    points[2] = Pt(shaft.head.e2);
    view->DrawPolygon(Origin, 3, points);

#if 1
  /* draw boxes around source and sink */
    src_bbox  = view->CharToPixelRect(view->ContentsToViewRect(src.bbox));
    sink_bbox = view->CharToPixelRect(view->ContentsToViewRect(sink.bbox));
    view->DrawRect(Rt(expand(src_bbox, 1, 1)));
    view->DrawRect(Rt(expand(sink_bbox, 1, 1)));
#endif
}




Rectangle Arrow::BBox(void)
{
  return R(this)->arrow.bbox;
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/***************/
/*  Appearance */
/***************/




void Arrow::calcAppearance(void)
{
  ArrowDisplay& arrow = R(this)->arrow;

  /* get details of desired appearance */
    this->getSrcEnd (arrow.src.bbox,  arrow.src.colors);
    this->getSinkEnd(arrow.sink.bbox, arrow.sink.colors);
    this->getShaft(arrow.shaft.width, arrow.shaft.style, arrow.shaft.color);
    
  /* calculate shaft layout */
    if( NOT( positiveArea(arrow.src.bbox) && positiveArea(arrow.sink.bbox) ) )
      arrow.shaft.kind = SHAFT_NONE;
    else
      this->calcShaftDisplay();

    /* compute approximate bounding box for autoscrolling -- shaft bbox ignored */
      arrow.bbox = unionRect(arrow.src.bbox, arrow.sink.bbox);
}




void Arrow::getSrcEnd(Rectangle &bbox, ColorPair &colors)
{
  SUBCLASS_RESPONSIBILITY("Arrow::getSrcEnd");
}




void Arrow::getSinkEnd(Rectangle &bbox, ColorPair &colors)
{
  SUBCLASS_RESPONSIBILITY("Arrow::getSinkEnd");
}




void Arrow::getShaft(int &width, LineStyle * &style, Color &color)
{
  width = 2;
  style = line_style_solid;
  color = R(this)->arrow.src.colors.background;
}




void Arrow::calcShaftDisplay(void)
{
  CTextView * view    = this->GetView();
  EndDisplay &src     = R(this)->arrow.src;
  EndDisplay &sink    = R(this)->arrow.sink;
  ShaftDisplay &shaft = R(this)->arrow.shaft;
  Rectangle src_bbox, sink_bbox;
  Direction dir, oppdir;
  int src_side, sink_side, arcBase;
  double headAngle, a, s, c;
  Boolean tiny = false /*** BOOL( sizeRect(src.bbox).x < 3 ) ***/;     /*** FIX THIS ***/
  int loopRadius = (tiny ? LOOP_RADIUS/2 : LOOP_RADIUS);

  /* determine whether entire arrow is elided */
    if( view->ContentsLineElided(src.bbox.ul.y) && view->ContentsLineElided(sink.bbox.ul.y) )
      { shaft.kind = SHAFT_NONE;
        return;
      }

  /* determine kind of shaft */
    src_bbox  = view->CharToPixelRect(view->ContentsToViewRect(src.bbox ));
    sink_bbox = view->CharToPixelRect(view->ContentsToViewRect(sink.bbox));

    if( rectEq(src.bbox, sink.bbox) )
      shaft.kind = SHAFT_LOOP;
    else
      if( separation(src_bbox, sink_bbox, dir, src_side, sink_side) )
        shaft.kind = SHAFT_LINE;
      else
        shaft.kind = SHAFT_ARC;

  switch( shaft.kind )
    {
      case SHAFT_LINE:
        shaft.line2.e1   = centerOfSide(src_bbox,  src_side);
        shaft.line2.e2   = centerOfSide(sink_bbox, sink_side);
        headAngle        = angle(shaft.line2.e2, shaft.line2.e1);
        break;

      case SHAFT_ARC:
        oppdir  = OPPOSITE(dir);
        arcBase = SHAFT_HEAD_LENGTH + max(side(src_bbox, SIDE(oppdir,LAST )),
                                          side(sink_bbox, SIDE(oppdir,LAST)));

        shaft.line1.e1   = centerOfSide(src_bbox, SIDE(oppdir,LAST));
        shaft.line1.e2   = movePointTo(shaft.line1.e1, oppdir, arcBase);
        shaft.line2.e2   = centerOfSide(sink_bbox, SIDE(oppdir,LAST));
        shaft.line2.e1   = movePointTo(shaft.line2.e2, oppdir, arcBase);
        shaft.arc.bbox   = arcBox(dir, shaft.line1.e2, shaft.line2.e1);
        shaft.arc.angle  = (oppdir == H ? 270.0 : 180.0);
        shaft.arc.extent = 180.0;
        headAngle        = (oppdir == H ? 0.0 :  270.0);
        break;

      case SHAFT_LOOP:
        shaft.line1.e1   = centerOfSide(src_bbox, RIGHT);
        shaft.line1.e2   = shaft.line1.e1;
        shaft.arc.bbox   = arcBox(V, shaft.line1.e2,
                                     makePoint(shaft.line1.e2.x,
                                               shaft.line1.e2.y + 2 * loopRadius));
        shaft.line2.e1   = makePoint(src_bbox.lr.x - loopRadius,
                                     shaft.line1.e2.y + loopRadius);
        shaft.line2.e2   = movePointTo(shaft.line2.e1, V, src_bbox.lr.y);
        shaft.arc.angle  = 180.0;
        shaft.arc.extent = 270.0;
        headAngle        =  90.0;  /* sic: cf. direction of 'y' axis */
        break;
    }

  /* calculate arrowhead line endpoints */
    a = M_PI * ((headAngle + SHAFT_HEAD_ANGLE) / 180.0);
    s = sin(a);  c = cos(a);
    shaft.head.e1 = transPoint(makePoint((int)(c * SHAFT_HEAD_LENGTH),
                                         (int)(s * SHAFT_HEAD_LENGTH)),
                               shaft.line2.e2);

    a = M_PI * ((headAngle - SHAFT_HEAD_ANGLE) / 180.0);
    s = sin(a);  c = cos(a);
    shaft.head.e2 = transPoint(makePoint((int)(c * SHAFT_HEAD_LENGTH),
                                         (int)(s * SHAFT_HEAD_LENGTH)),
                               shaft.line2.e2);
}




static
Boolean separation(Rectangle box1, Rectangle box2,
                   Direction &dir, Side &side1, Side &side2)
{
  int d, sep;
  int h_sep, h_side1, h_side2;
  int v_sep, v_side1, v_side2;

# define Distance(a, b)    (b - a)
# define Top(r)            r.ul.y
# define Bottom(r)         r.lr.y
# define Left(r)           r.ul.x
# define Right(r)          r.lr.x

  /* compute horizontal separation */
    if( (d = Distance(Right(box1), Left(box2))) > 0 )
      { h_sep = d; h_side1 = RIGHT; h_side2 = LEFT;}
    else if( (d = Distance(Right(box2), Left(box1))) > 0 )
      { h_sep = d; h_side1 = LEFT; h_side2 = RIGHT;}
    else
      h_sep = 0;

  /* compute vertical separation */
    if( (d = Distance(Bottom(box1), Top(box2))) > 0 )
      { v_sep = d; v_side1 = BOTTOM; v_side2 = TOP;}
    else if( (d = Distance(Bottom(box2), Top(box1))) > 0 )
      { v_sep = d; v_side1 = TOP; v_side2 = BOTTOM;}
    else
      v_sep = 0;

  /* choose separation direction */
    if( h_sep >= v_sep )
      { sep = h_sep; dir = H; side1 = h_side1; side2 = h_side2;}
    else
      { sep = v_sep; dir = V; side1 = v_side1; side2 = v_side2;}

  return BOOL( sep >= MIN_SEPARATION );
}




static
int side(Rectangle r, Side side)
{
  switch( side )
    {
      case TOP:    return r.ul.y;
      case BOTTOM: return r.lr.y;
      case LEFT:   return r.ul.x;
      case RIGHT:  return r.lr.x;
      default:     return 0;   /* can't happen but g++ complains */
    }
}




static
Point centerOfSide(Rectangle r, Side side)
{
  switch( side )
    {
      case TOP:    return makePoint((r.ul.x + r.lr.x)/2, r.ul.y);
      case BOTTOM: return makePoint((r.ul.x + r.lr.x)/2, r.lr.y);
      case LEFT:   return makePoint(r.ul.x, (r.ul.y + r.lr.y)/2);
      case RIGHT:  return makePoint(r.lr.x, (r.ul.y + r.lr.y)/2);
      default:     return Origin;  /* can't happen but g++ complains */
    }
}




static
Point movePointTo(Point pt, Direction dir, int pos)
{
  Point newPt;

  newPt = pt;
  if( dir == H )
    newPt.x = pos;
  else
    newPt.y = pos;

  return newPt;
}




static
Rectangle arcBox(Direction dir, Point p1, Point p2)
{
  int top, bottom, left, right;
  int radius;
  Rectangle box;

  if( dir == V )
    { /* ASSERT: p1.x == p2.x */
      top    = min(p1.y, p2.y);
      bottom = max(p1.y, p2.y);
      radius = min(MAX_ARC_RADIUS, (bottom - top)/2);
      box.ul = makePoint(p1.x - radius, top);
      box.lr = makePoint(p1.x + radius, bottom);
    }
  else
    { /* ASSERT: p1.y == p2.y */
      left   = min(p1.x, p2.x);
      right  = max(p1.x, p2.x);
      radius = min(MAX_ARC_RADIUS, (right - left)/2);
      box.ul = makePoint(left, p1.y - radius);
      box.lr = makePoint(right, p1.y + radius);
    }

  /* kludge */
    box = transRect(box, makePoint(-1, -1));

  return box;
}




static
float angle(Point p1, Point p2)
{
  float dx, dy, a;

  dx = p2.x - p1.x;
  dy = p2.y - p1.y;
  a  = (180.0 / M_PI) * atan2(dy, dx);

  return (a >= 0.0 ? a : a + 360.0);
}






/************/
/*  Drawing */
/************/




void Arrow::colorizeEnd(int c_linenum, Boolean src, TextData &td)
{
  EndDisplay& end = (src ? R(this)->arrow.src : R(this)->arrow.sink);
  Rectangle s_bbox;
  int first, last, k;

  if( end.bbox.ul.y <= c_linenum && c_linenum <= end.bbox.lr.y )
    { /* to account for margin, must convert to screen-char coords, */
      /* but doing contents-to-view mapping here may cause infinite */
      /* recursion when called by the 'ensure' routine of Ned's     */
      /* ViewFilter class.  So we do view-to-screen here even       */
      /* though 'end.bbox' is in contents coords.  The y-coords     */
      /* turn out wrong, but are not used.  It is a true but lucky  */
      /* fact that contents-to-screen and view-to-screen do the     */
      /* same thing to the x-coords.  Fixing this correctly would   */
      /* require modifying the ViewFilter class to probe for lines' */
      /* existence without making the caller manifest them.         */

        s_bbox = this->GetView()->ViewToScreenRect(end.bbox);

      if( ! td.multiColored )  Text_MultiColoredData(td);

      first = s_bbox.ul.x;
      last  = s_bbox.lr.x;
      for( k = first; k <= last;  k++ )
        { td.chars[k].foreground = end.colors.foreground;
          td.chars[k].background = end.colors.background;
        }
    }
}




static
Point v2s_point(Arrow * a, Point c_pt)
{
  CTextView * view = a->GetView();
  Point charSize, charOffset, pixelOffset;

  charSize      = view->CharacterSize();
  charOffset    = view->ScreenToViewPoint(Origin);
  pixelOffset.x = charOffset.x * charSize.x;
  pixelOffset.y = charOffset.y * charSize.y;

  return subPoint(c_pt, pixelOffset);
}




static
Rectangle v2s_rect(Arrow * a, Rectangle c_rect)
{
  Rectangle p_rect;
  
  p_rect.ul = v2s_point(a, c_rect.ul);
  p_rect.lr = v2s_point(a, c_rect.lr);
  return p_rect;
}




static
Rectangle expand(Rectangle r, int dx, int dy)
{
  r.ul.x -= dx;
  r.ul.y -= dy;
  r.lr.x += dx;
  r.lr.y += dy;

  return r;
}
