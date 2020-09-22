/* $Id: CCP.C,v 1.3 1997/03/11 14:32:26 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/CCP.C							*/
/*									*/
/*	CP -- Command Processor Class					*/
/*	Last edited: November 10, 1993 at 10:17 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/framework/CCP.h>
#include <libs/graphicInterface/framework/View.h>


#include <libs/graphicInterface/oldMonitor/include/mon/cp.h>
#include <libs/graphicInterface/oldMonitor/include/mon/sm.h>
#include <libs/graphicInterface/framework/MenuBar.h>
#include <libs/graphicInterface/oldMonitor/include/sms/vanilla_sm.h>
#include <libs/graphicInterface/cmdProcs/help/help_cp.h>
#include <libs/support/arrays/FlexibleArray.h>





/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* CP object */

typedef struct CP_Repr_struct
  {
    /* creation parameters */
      anInstance *         cp_id;
      Context              mod_context;

    /* window appearance */
      Point                minSize;
      Point                defSize;
      Point                size;

    /* window parts */
      Window *             window;
      MenuBar *            menuBar;
      Generic              defaultContentsPane;  /* not used by subclasses */

    /* auxiliary windows */
      Flex *               auxWindowInfo;

  } CP_Repr;


#define R(ob)		(ob->CP_repr)




struct AuxWindowInfo
  {
    Window *		window;
    View *		view;
  };






/*************************/
/*  Miscellaneous	 */
/*************************/



/* window layout constants */

static Point cp_maxReasonableSize = {600, 750};




/* operation needed for CP::WinToTop -- apparently unsupported */

/*extern "C" void sm_desk_win_top(Generic);*/
EXTERN(void, sm_desk_win_top, (Generic));






/*************************/
/*  Forward declarations */
/*************************/




static Point windowSize(CP * cp, Point contentsSize);

static void tileWindow(CP * cp, Boolean init, Point size);

static void titleWindow(CP * cp);

static Boolean keyCommand(CP * cp, KbChar kb);

static Point reasonableSize(Point defSize, Point minSize);





/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/






/***********************************/
/*  Rn Command Processor interface */
/***********************************/




Boolean CP_HandleInput(Generic cp,
                       Generic generator,
                       short event_type,
                       Point info,
                       Generic msg)
{
  return ((CP *) cp)->CP_HandleInput(generator, event_type, info, msg);
}




/* ARGSUSED */

void CP_DestroyInstance(Generic cp, Boolean panicked)
{
  /* 'panicked' is ignored for now */

  CP * cp_ptr;

  cp_ptr = (CP *) cp;
  cp_ptr->Fini();

  delete cp_ptr;
}







/*************************/
/*  Class initialization */
/*************************/




CP * CP::CurrentCP;




void CP::InitClass(void)
{
  /* initialize all submodules */
    /* ... */

  CP::CurrentCP = nil;
}




void CP::FiniClass(void)
{
  /* nothing */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(CP)




CP::CP(Generic parent_id, Generic cp_id)
{
  /* allocate instance's private data */
    this->CP_repr = (CP_Repr *) get_mem(sizeof(CP_Repr), "CP instance");

  /* set creation parameters */
    R(this)->cp_id = (anInstance *) cp_id;

  /* perform initializations needed for subclass's Init/Open/OpenView etc */
    CP::CurrentCP = this;
    R(this)->auxWindowInfo = flex_create(sizeof(AuxWindowInfo));
}




CP::~CP(void)
{
  int num, k;
  AuxWindowInfo aux;
  /* close remaining auxiliary windows */
    num = flex_length(R(this)->auxWindowInfo);
    for( k = 0;  k < num;  k++ )
      { flex_get_buffer(R(this)->auxWindowInfo, k, 1, (char *) &aux);
        cp_window_destroy(aux.window, R(this)->cp_id);
      }
    flex_destroy(R(this)->auxWindowInfo);

  /* close any help windows */
    help_cp_terminate_help((Generic) R(this)->cp_id);

  cp_window_destroy(R(this)->window, R(this)->cp_id);

  /* subpanes are destroyed implicitly */

  free_mem((void*) this->CP_repr);
}




void CP::Init(void)
{
  MenuBar * mb;
  int k;
  Point minContentsSize, defContentsSize;

  /* set up the menus */
    R(this)->menuBar = mb = new MenuBar();
    this->InitMenuBar(mb);
    for( k = 0;  k < mb->NumMenus();  k++ )
      this->InitMenu(mb->GetMenuName(k), mb->GetMenu(k));

  /* tile the window */
    this->GetContentsSizePrefs(minContentsSize, defContentsSize);
    R(this)->minSize = windowSize(this, minContentsSize);
    R(this)->defSize = windowSize(this, defContentsSize);
    R(this)->defSize = reasonableSize(R(this)->defSize, R(this)->minSize);

    R(this)->window = (Window *) CP_WIN_RESIZABLE;
    tileWindow(this, true, R(this)->defSize);

  /* initialize panes of the tiled window */
    titleWindow(this);
    R(this)->menuBar->InitPanes();
    this->InitContentsPanes();
}




void CP::Fini(void)
{
  /* nothing */
}






/*********************************/
/*  Raw interface to Rn monitor  */
/*********************************/





Boolean CP::CP_HandleInput(Generic generator,
                           short event_type,
                           Point info,
                           Generic msg)
{
  Boolean kill = false;		/* unless changed below */
  int id;
  KbChar kb;
  View * dummy;

  switch( event_type )
    {
      case EVENT_MESSAGE:
        this->Message(msg);
        break;

      case EVENT_KILL:
        if( this->findAuxWindowInfo((Window *) generator, dummy) != UNUSED )
          this->CloseAuxWindow((Window *) generator);
        else
          kill = this->QueryKill();
        break;

      case EVENT_RESIZE:
        if( this->findAuxWindowInfo((Window *) generator, dummy) != UNUSED )
          this->ResizeAuxWindow((Window *) generator, info);
        else
          this->Resize(info);
        break;

      case EVENT_HELP:
        this->Help(info);
        break;

      case EVENT_SELECT:
//        if( generator == R(this)->menuBar->getButtonPane() )
//          { if( R(this)->menuBar->Select(info.x, id) )
 //             this->MenuChoice(id);
  //        }
  //      else
   //       this->SelectionEvent(generator, info);
        break;

      case EVENT_KEYBOARD:
        kb = toKbChar(info.x);
        if( ! keyCommand(this, kb) )  this->Keystroke(kb);
        break;

    }

  return kill;
}






/*******************/
/*  Window layout  */
/*******************/




char * CP::GetWindowTitle(void)
{
  return "Command Processor";
}




void CP::SetWindowTitle(char * title)
{
  cp_window_set_title(R(this)->window, "%s", title);
}




void CP::GetContentsSizePrefs(Point &minSize, Point &defSize)
{
  minSize = makePoint(100, 100);
  defSize = makePoint(600, 200);
}




Generic CP::GetContentsTiling(Boolean init, Point contentsSize)
{
  if( init )
    R(this)->defaultContentsPane = sm_vanilla_get_index();
  return cp_td_pane((pane**) &R(this)->defaultContentsPane, contentsSize);
}




void CP::InitContentsPanes(void)
{
  /* nothing */
}




void CP::RetileWindow(void)
{
  tileWindow(this, false, sizeRect(R(this)->window->border));
}







/***********/
/*  Menus  */
/***********/




void CP::InitMenuBar(MenuBar * mb)
{
  /* nothing */
}





void CP::InitMenu(char * name, CMenu * m)
{
  /* nothing */
}






/********************/
/*  Input handling  */
/********************/




void CP::Message(Generic msg)
{
  /* nothing */
}




void CP::Resize(Point newSize)
{
  tileWindow(this, false, newSize);
}




Boolean CP::QueryKill(void)
{
  return true;
}




void CP::Help(Point pt)
{
  char * helpfile = this->GetHelpFileName();

  if( helpfile != nil )
    help_cp_give_help((Generic) R(this)->cp_id, helpfile, 0, (short *) 0, 0);
}




char * CP::GetHelpFileName(void)
{
  return nil;
}




void CP::SelectionEvent(Generic generator, Point info)
{
  /* nothing */
}




Boolean CP::MenuChoice(Generic id)
{
  notImplemented("that menu choice");
  return true;
}




void CP::Keystroke(KbChar kb)
{
  /* nothing */
}







/*************************/
/*  Window manipulation  */
/*************************/




void CP::WindowToTop(void)
{
  sm_desk_win_top((Generic) R(this)->window);
}




void CP::CurrentWindowToTop(void)
{
  CP::CurrentCP->WindowToTop();
}








/*************************/
/*  Subordinate windows  */
/*************************/




Window * CP::OpenAuxWindow(View * view, char * title, Boolean visible)
{
  Point minSize, defSize;
  aTilingDesc * td;
  Window * window;
  
  /* compute desired window contents */
    view->GetSizePrefs(minSize, defSize);
    td = (aTilingDesc*) view->GetTiling(true, reasonableSize(defSize, minSize));
    
  /* create the window as specified */
    window = (Window *) CP_WIN_RESIZABLE;
    if( visible )
      cp_window_tile(&window, R(this)->cp_id, td);
    else
      cp_window_tile_hidden(&window, R(this)->cp_id, td);
    cp_window_set_title(window, title);
  
  /* finish initialization of contents */
    view->InitPanes();
  
  this->addAuxWindowInfo(window, view);

  return window;
}




void CP::CloseAuxWindow(Window * window)
{
  this->removeAuxWindowInfo(window);
  cp_window_destroy(window, R(this)->cp_id);
}




void CP::ResizeAuxWindow(Window * window, Point newSize)
{
  View * view;
  aTilingDesc * td;

  (void) this->findAuxWindowInfo(window, view);
  td = (aTilingDesc*) view->GetTiling(false, newSize);
  cp_window_tile(&window, R(this)->cp_id, td);
}




void CP::ShowAuxWindow(Window * window)
{
  cp_window_show(window);
}




void CP::HideAuxWindow(Window * window)
{
  cp_window_hide(window);
}







/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




static
Point windowSize(CP * cp, Point contentsSize)
{
  Point mbarSize, size, dummy;
  int BORDERS = 1 + 1;

  R(cp)->menuBar->GetSizePrefs(dummy, mbarSize);

  size.x = contentsSize.x + BORDERS;
  size.y = contentsSize.y + mbarSize.y + BORDERS;

  return size;
}




static
void tileWindow(CP * cp, Boolean init, Point size)
{
  Point mbarSize, contentsSize, dummy;
  Generic mbar, contents, whole;

  /* enforce the minimum size */
    R(cp)->size.x = max(size.x, R(cp)->minSize.x);
    R(cp)->size.y = max(size.y, R(cp)->minSize.y);

  /* calculate sizes of all the panes */
    /* the menu bar */
      R(cp)->menuBar->GetSizePrefs(dummy, mbarSize);
      mbarSize.x = R(cp)->size.x;

    /* the contents */
      contentsSize.x = R(cp)->size.x;
      contentsSize.y = R(cp)->size.y - mbarSize.y;

  /* calculate the window's tiling descriptor */
    mbar     = R(cp)->menuBar->GetTiling(init, mbarSize);
    contents = cp->GetContentsTiling(init, contentsSize);
    whole    = cp_td_join(TILE_UP, (aTilingDesc *) mbar, (aTilingDesc *) contents);

  /* tile the window */
    cp_window_tile(&R(cp)->window, R(cp)->cp_id, (aTilingDesc*) whole);
}




static
void titleWindow(CP * cp)
{
  cp->SetWindowTitle(cp->GetWindowTitle());
}




static
Boolean keyCommand(CP * cp, KbChar kb)
{
  Generic cmd;
  Boolean handled;

  /* see if 'kb' is a menu item's shortcut */

  if( R(cp)->menuBar->Keystroke(kb, cmd) )
    { cp->MenuChoice(cmd);
      handled = true;
    }
  else
    handled = false;

  return handled;
}




static
Point reasonableSize(Point defSize, Point minSize)
{
  Point size;
  
  size.x = max(minSize.x, min(defSize.x, cp_maxReasonableSize.x));
  size.y = max(minSize.y, min(defSize.y, cp_maxReasonableSize.y));
  
  return size;
}






/* Auxiliary windows */
/*********************/




void CP::addAuxWindowInfo(Window * window, View * view)
{
  AuxWindowInfo aux;

  aux.window = window;
  aux.view   = view;
  flex_insert_one(R(this)->auxWindowInfo,
                  flex_length(R(this)->auxWindowInfo),
                  (char *) &aux);
}




int CP::findAuxWindowInfo(Window * window, View * &view)
{
  int found, num, k;
  AuxWindowInfo aux;

  found = UNUSED;
  num = flex_length(R(this)->auxWindowInfo);
  for( k = 0; (found == UNUSED) && (k < num);  k++ )
    { flex_get_buffer(R(this)->auxWindowInfo, k, 1, (char *) &aux);
      if( aux.window == window )
        { found = k;
          view  = aux.view;
        }
    }

  return found;
}




void CP::removeAuxWindowInfo(Window * window)
{
  int k;
  View * dummy;

  k = this->findAuxWindowInfo(window, dummy);
  if( k != UNUSED )
    flex_delete(R(this)->auxWindowInfo, k, 1);
}
