/* $Id: SplitView.C,v 1.9 1997/03/11 14:32:54 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*      framework/SplitView.C                                           */
/*                                                                      */
/*      SplitView -- View split into one or more subviews               */
/*	Last edited: November 10, 1993 at 6:20 pm			*/
/*                                                                      */
/************************************************************************/




#include <libs/graphicInterface/framework/SplitView.h>

#include <libs/graphicInterface/framework/CCP.h>
#include <libs/graphicInterface/framework/UserFilter.h>

#include <libs/graphicInterface/oldMonitor/include/mon/sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/vanilla_sm.h>
#include <libs/graphicInterface/oldMonitor/include/mon/cp.h>
#include <libs/support/arrays/FlexibleArray.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* SplitView object */

typedef struct SplitView_Repr_struct
  {
    /* creation parameters */
      Editor *		mainEditor;
      Boolean		showFilterNames;

    /* subviews */
      Flex *		views;			/* of ViewInfo */
      int		currentViewNum;
      View *		currentView;

    /* status */
      Boolean		initialized;
      Point		minSize;
      Point		defSize;

  } SplitView_Repr;


#define R(ob)		(ob->SplitView_repr)

#define INHERITED	View




typedef struct
  {
    View *		view;
    Editor *		editor;
    char *		caption;
    Pane * *		captionPane;

    Point		minSize;
    Point		defSize;
    Point		size;
    Point		capSize;
    float		fraction;
    
    Boolean		visible;

  } ViewInfo;






/*************************/
/*  Miscellaneous	 */
/*************************/




/* appearance of captions */

static int		caption_height;
static int		caption_font;
static unsigned char	caption_style;
static Color		caption_foreground;
static Color		caption_background;






/*************************/
/*  Forward declarations */
/*************************/




static void setCurrentView(SplitView * sv, int k);

static Boolean hasCaption(ViewInfo vi);

static void initViewInfo(ViewInfo &vi);

static void finiViewInfo(ViewInfo &vi);

static void setCaptionText(SplitView * sv, ViewInfo vi);

static void calcSizePrefs(SplitView * sv);




/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/






/*************************/
/*  Class initialization */
/*************************/




void SplitView::InitClass(void)
{
  int red, green, blue;

# define BRIGHTEN(x)    x = int ((x + (BRIGHT_FACTOR-1) * 65536L) / BRIGHT_FACTOR)
# define BRIGHT_FACTOR  4

  /* initialize needed submodules */
    ut_Init();    /* needed for black_color and white_color */
    REQUIRE_INIT(View);
    REQUIRE_INIT(Editor);

  /* caption appearance */
    caption_height = 22;
    caption_font   = fontOpen("screen.11.rnf");
    caption_style  = STYLE_BOLD;

    /* caption colors depend whether a color display is available */
      if( monochrome_screen )
        { caption_foreground = black_color;
          caption_background = white_color;
        }
      else
        { caption_foreground = white_color;

          /* background is a less-saturated version of title bar color */
            getRGBFromName("titlePane.highlighted", &red, &green, &blue);
            BRIGHTEN(red);
            BRIGHTEN(green);
            BRIGHTEN(blue);
            caption_background = getColorFromRGB(red, green, blue);
        }
}




void SplitView::FiniClass(void)
{
  fontClose(caption_font);
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(SplitView)




SplitView::SplitView(Context context,
                     DB_FP * session_fp,
                     Editor * editor,
                     Boolean showFilterNames)
         : View(context, session_fp, editor)
{
  /* allocate instance's private data */
    this->SplitView_repr =
        (SplitView_Repr *) get_mem(sizeof(SplitView_Repr), "SplitView instance");

  /* save creation parameters */
    R(this)->mainEditor = editor;
    R(this)->showFilterNames = showFilterNames;

  /* initialize subview list */
    R(this)->views = flex_create(sizeof(ViewInfo));
    R(this)->currentViewNum = UNUSED;
    R(this)->currentView = nil;

  /* not fully initialized until after 'InitPanes' */
    R(this)->initialized = false;
}




SplitView::~SplitView(void)
{
  int len = flex_length(R(this)->views);
  int k;
  ViewInfo vi;

  /* free view info storage */
    for( k = 0;  k < len;  k++ )
      { flex_get_buffer(R(this)->views, k, 1, (char *) &vi);
        finiViewInfo(vi);
        delete vi.view;
      }
    flex_destroy(R(this)->views);

  free_mem((void*) this->SplitView_repr);
}






/************/
/* Database */
/************/




void SplitView::isnew(Context context)
{
  this->INHERITED::isnew(context);
}




void SplitView::read(DB_FP * fp, DB_FP * session_fp)
{
  this->INHERITED::read(fp, session_fp);
}




void SplitView::write(DB_FP * fp, DB_FP * session_fp)
{
  /* TEMPORARY */
  this->INHERITED::write(fp, session_fp);
}






/************************/
/*  Change notification */
/************************/




void SplitView::NoteChange(Object * ob, int kind, void * change)
{
  NoteChangeTrace trace(this, ob, kind, change);
  int len = flex_length(R(this)->views);
  int k;
  ViewInfo vi;

  if( kind == CHANGE_FILTER )
    /* ASSERT: 'ob' is one of the subviews */
    this->Changed(kind, change);
  else
    for( k = 0;  k < len;  k++ )
      { flex_get_buffer(R(this)->views, k, 1, (char *) &vi);
        vi.view->NoteChange(ob, kind, change);
      }
}






/******************/
/*  Window layout */
/******************/




void SplitView::GetSizePrefs(Point &minSize, Point &defSize)
{
  calcSizePrefs(this);
  minSize = R(this)->minSize;
  defSize = R(this)->defSize;
}




Generic SplitView::GetTiling(Boolean init, Point totalSize)
{
  int len = flex_length(R(this)->views);
  float totalFraction;
  int k;
  ViewInfo vi;
  int height;
  Point subSize, capSize;
  Generic sub, caption, whole;

  /* ASSERT: there is at least one subpane */

  calcSizePrefs(this);

  /* first assign height to subpanes whose minimum would otherwise not be met */
    totalFraction = 1.0;
    for( k = 0;  k < len;  k++ )
      { flex_get_buffer(R(this)->views, k, 1, (char *) &vi);

        vi.size.x = totalSize.x;

        height = (int) (vi.fraction * totalSize.y);
        if( height < vi.minSize.y )
          { vi.size.y = vi.minSize.y;
            totalSize.y -= vi.size.y;
            totalFraction -= vi.fraction;

            if( hasCaption(vi) )
              { vi.capSize.x = totalSize.x;
                vi.capSize.y = caption_height;
                totalSize.y -= caption_height;
              }
          }
        else
          vi.size.y = UNUSED;

        flex_set_one(R(this)->views, k, (char *) &vi);
      }

  /* now assign height to remaining subpanes in proportion to their defaults */
    for( k = 0;  k < len;  k++ )
      { flex_get_buffer(R(this)->views, k, 1, (char *) &vi);

        if( vi.size.y == UNUSED )
          { height = (int) ( (vi.fraction / totalFraction) * totalSize.y);
            if( hasCaption(vi) )
              { vi.capSize.x = totalSize.x;
                vi.capSize.y = caption_height;
                height      -= caption_height;
              }
            vi.size.y = height;

            flex_set_one(R(this)->views, k, (char *) &vi);
          }
      }

  /* finally compute the tiling */
    for( k = 0;  k < len;  k++ )
      { flex_get_buffer(R(this)->views, k, 1, (char *) &vi);

        /* get tiling for this subview */
          sub = vi.view->GetTiling(init, vi.size);

          if( hasCaption(vi) )
            { if( init )
                *(vi.captionPane) = (Pane *) sm_vanilla_get_index();

              caption = cp_td_pane((Pane**)vi.captionPane, vi.capSize);
              sub = cp_td_join(TILE_UP, (aTilingDesc*)caption, (aTilingDesc*)sub);
            }

        /* assemble subviews into a whole tiling */
          if( k == 0 )
            whole = sub;
          else
            whole = cp_td_join(TILE_UP, (aTilingDesc*)whole, (aTilingDesc*)sub);
      }

  return whole;
}




void SplitView::InitPanes(void)
{
  int len = flex_length(R(this)->views);
  int k, num;
  ViewInfo vi;
  Pane * p;

  /* ASSERT: there is at least one subpane */

  for( k = 0;  k < len;  k++ )
    { flex_get_buffer(R(this)->views, k, 1, (char *) &vi);

      /* initialize subview */
        vi.view->InitPanes();

      /* initialize caption */
        if( hasCaption(vi) )
          { p = *(vi.captionPane);
            recolorPane(p, caption_foreground, caption_background,
                        paneBorderColor(p), false, false, false);
            p->border_width = 1;
            setCaptionText(this, vi);
          }
    }

  /* initialize the current-pane feedback */
    num = R(this)->currentViewNum;
    if( num == UNUSED )  num = 0;
    setCurrentView(this, num);

  R(this)->initialized = true;
}






/**********************/
/* Access to subviews */
/**********************/




int SplitView::NumViews(void)
{
  return flex_length(R(this)->views);
}




void SplitView::GetView(int k, View * &v, Editor * &e, char * &caption)
{
  ViewInfo vi;

  flex_get_buffer(R(this)->views, k, 1, (char *) &vi);
  v       = vi.view;
  e       = vi.editor;
  caption = vi.caption;
}




void SplitView::AddView(View * v, Editor * e, char * caption)
{
  ViewInfo vi;

  vi.view    = v;
  vi.editor  = e;
  vi.caption = caption;
  initViewInfo(vi);  
  flex_insert_one(R(this)->views, flex_length(R(this)->views), (char *) &vi);

  v->Notify(this, true);
}




void SplitView::InsertView(int k, View * v, Editor * e, char * caption)
{
  ViewInfo vi;

  vi.view    = v;
  vi.editor  = e;
  vi.caption = caption;
  initViewInfo(vi);
  flex_insert_one(R(this)->views, k, (char *) &vi);

  v->Notify(this, true);
}




void SplitView::DeleteView(int k)
{
  ViewInfo vi;

  flex_get_buffer(R(this)->views, k, 1, (char *) &vi);
  finiViewInfo(vi);
  vi.view->Notify(this, false);

  flex_delete(R(this)->views, k, 1);
}




void SplitView::DeleteAllViews(void)
{
  int len = flex_length(R(this)->views);
  int k;
  ViewInfo vi;

  for( k = 0;  k < len;  k++ )
    { flex_get_buffer(R(this)->views, k, 1, (char *) &vi);
      finiViewInfo(vi);
      vi.view->Notify(this, false);
      /* do not delete vi.view */
    }

  flex_delete(R(this)->views, 0, len);
}




void SplitView::SetViewVisible(int k, Boolean visible)
{
  ViewInfo vi;

  /* adjust specified view's status */
    flex_get_buffer(R(this)->views, k, 1, (char *) &vi);
    vi.visible = visible;
    flex_set_one(R(this)->views, k, (char *) &vi);
    
  if( R(this)->initialized )
    CP::CurrentCP->RetileWindow();
}






/*******************/
/* Current subview */
/*******************/




int SplitView::CurrentView(void)
{
  return R(this)->currentViewNum;
}




void SplitView::SetCurrentView(int k)
{
  setCurrentView(this, k);
}




void SplitView::GetCurrentView(View * &v, Editor * &e, char * &caption)
{
  this->GetView(R(this)->currentViewNum, v, e, caption);
}






/******************/
/* Input handling */
/******************/




Boolean SplitView::SelectionEvent(Generic generator, Point info)
{
  int len = flex_length(R(this)->views);
  int k;
  ViewInfo vi;
  Boolean found;

  found = false;
  k = 0;
  while( ! found  &&  k < len )
    { flex_get_buffer(R(this)->views, k, 1, (char *) &vi);
      if( generator == (Generic) *(vi.captionPane)
          || vi.view->SelectionEvent(generator, info) )
        { this->SetCurrentView(k);
          found = true;
        }
      else
        k += 1;
    }

  return found;
}








/************************************************************************/
/*	Private Operations						*/
/************************************************************************/




static
void setCurrentView(SplitView * sv, int k)
{
  int old = R(sv)->currentViewNum;
  ViewInfo old_vi, new_vi;

  if( R(sv)->currentViewNum == k )  return;

  /* remove feedback from old current view */
#ifdef NOTDEF
    if( old != UNUSED )
      { flex_get_buffer(R(sv)->views, old, 1, (char *) &old_vi);
        sm_vanilla_invert(*(old_vi.captionPane));
      }
#endif

  /* set new current view */
    R(sv)->currentViewNum = k;
    if( k != UNUSED )
      { flex_get_buffer(R(sv)->views, k, 1, (char *) &new_vi);
        R(sv)->currentView = new_vi.view;
      }
    else
      R(sv)->currentView = nil;

  /* add feedback to new current view */
#ifdef NOTDEF
    if( k != UNUSED )
      { sm_vanilla_invert(*(new_vi.captionPane));
      }
#endif
}




static
Boolean hasCaption(ViewInfo vi)
{
  return BOOL( strlen(vi.caption) > 0 );
}




static
void initViewInfo(ViewInfo &vi)
{
  vi.visible = true;

  if( hasCaption(vi) )
    vi.captionPane = (Pane * *) get_mem(sizeof(Pane *), "SplitView caption");
}




static
void finiViewInfo(ViewInfo &vi)
{
  if( hasCaption(vi) )
    free_mem((void *) vi.captionPane);
}




static
void setCaptionText(SplitView * sv, ViewInfo vi)
{
  Pane * p = *(vi.captionPane);
  UserFilter * filter;
  char * filterName;
  char completeCaption[200];

  strcpy(completeCaption, vi.caption);

  if( R(sv)->showFilterNames )
    { filter = (UserFilter *) (vi.view->GetFilter());
      if( filter != nil )
        { filterName = filter->GetName(true);
          if( strcmp(filterName, "normal") != 0 )
            { strcat(completeCaption, " (");
              strcat(completeCaption, filterName);
              strcat(completeCaption, ")");
            }
        }
    }

  sm_vanilla_set_text(p, completeCaption,
                      caption_font, caption_style, VSM_JUSTIFY_CENTER);
}




static
void calcSizePrefs(SplitView * sv)
{
  Point minSize, defSize;
  Flex * views = R(sv)->views;
  int len = flex_length(views);
  int k, total;
  ViewInfo vi;

  /* compute minimum and default sizes from subview prefs */
    minSize = Origin;
    defSize = Origin;
    total   = 0;

    for( k = 0;  k < len;  k++ )
      { flex_get_buffer(views, k, 1, (char *) &vi);
        if( vi.visible )
          vi.view->GetSizePrefs(vi.minSize, vi.defSize);
        else
          { vi.minSize = makePoint(0, 0);
            vi.defSize = makePoint(0, 0);
          }
        flex_set_one(views, k, (char *) &vi);

        minSize.x  = max(minSize.x, vi.minSize.x);
        minSize.y += vi.minSize.y;
        if( hasCaption(vi) )  minSize.y += caption_height;

        defSize.x  = max(defSize.x, vi.defSize.x);
        defSize.y += vi.defSize.y;
        if( hasCaption(vi) )  defSize.y += caption_height;

        total += vi.defSize.y;
      }

  /* compute each subview's fraction of total subviews height */
    for( k = 0;  k < len;  k++ )
      { flex_get_buffer(views, k, 1, (char *) &vi);
        vi.fraction = vi.defSize.y / ((float) total);
        flex_set_one(views, k, (char *) &vi);
      }


  R(sv)->minSize = minSize;
  R(sv)->defSize = defSize;
}




void SplitView::updateCaptions(void)
{
  int len = flex_length(R(this)->views);
  int k;
  ViewInfo vi;

  for( k = 0;  k < len;  k++ )
    { flex_get_buffer(R(this)->views, k, 1, (char *) &vi);
      if( hasCaption(vi) )  setCaptionText(this, vi);
    }
}






