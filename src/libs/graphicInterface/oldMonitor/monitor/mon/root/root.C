/* $Id: root.C,v 1.1 1997/06/25 14:52:22 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*                                root.c                                */
/*                    screen module--monitor interface                  */
/*                                                                      */
/************************************************************************/

#include <stdio.h>

#include <libs/graphicInterface/oldMonitor/include/mon/sm_def.h>
#include <libs/graphicInterface/oldMonitor/include/mon/manager.h>
#include <libs/graphicInterface/oldMonitor/monitor/mon/scr_mod.h>
#include <libs/graphicInterface/oldMonitor/monitor/mon/root/desk_sm.h>
#include <libs/graphicInterface/oldMonitor/monitor/mon/root/notify_mgr.h>
#include <libs/graphicInterface/oldMonitor/monitor/mon/root/cp_mgr.h>
#include <libs/graphicInterface/oldMonitor/include/mon/cp.h>
#include <libs/graphicInterface/oldMonitor/include/mon/event.h>
#include <libs/graphicInterface/oldMonitor/monitor/mon/root/dialog_mgr.h>
#include <libs/graphicInterface/oldMonitor/monitor/mon/root/menu_mgr.h>
#include <libs/graphicInterface/oldMonitor/monitor/mon/root/system_mgr.h>
#include <libs/config/config.h>

extern  aProcessor  *processors[];      /* the command processor list   */
extern  short   num_processors;         /* number of defined cps        */
extern  short   D_startup_cp;          /* the startup command processor*/
extern  aScreenModule   *screenModules[];/* screen module array         */

extern	char    D_copyright[];		/* the copyright message	*/

static	Pane    *root_pane = NULL_PANE; /* the root pane                */
static  Point   root_size;              /* the root pane size           */
static  Generic system_mgr = 0;         /* the system manager handle    */
static	Generic menu_mgr = 0;           /* the menu manager handle      */
static  Boolean run_again = false;      /* run the root sm again        */

/* Change the size of the root window.  May be called asynchronously.   */
void
resizeRoot(Point size)
{
    root_size = size;
    if (root_pane)
    {/* resize the root pane */
        root_pane->parent->border = makeRectFromSize(Origin, root_size);
        resizePane(root_pane, Origin, root_size);
        touchPane(root_pane);

	if (menu_mgr)
	{/* notify the menu manager */
	    menu_mgr_resize_notify((aMenuMgr*)menu_mgr);
	}
    }
}


/* Redraw the screen for the system manager.                            */
static
void
redraw_screen(void)
{
    resizeRoot(root_size);
}


/* Set up to kill the environment.                                      */
void
quit_environment()
{
anEvent         saved_event;            /* the current event saved      */

    saved_event = mon_event;
    mon_event.type = EVENT_KILL;
    mon_event.from = 0;
    ungetEvent();
    mon_event = saved_event;
}


/* Redraw a damaged area of the root window.                            */
void
redrawRootRectList(RectList rl)
{
RectList        damage;                 /* damaged areas inside edges   */
Rectangle       r;                      /* the current rectangle        */

    if (root_pane)
    {/* redraw the areas of the root pane */
        r = makeRectFromSize(Origin, root_size);
        r = clipRectWithBorder(r, r, root_pane->border_width);
        damage = partitionRectList(r, &rl);
        while (NOT(emptyRectList(rl)))
        {/* fix the edge damage */
            r = popRectList(&rl);
	    ColorPaneWithPattern(root_pane, r, NULL_BITMAP, Origin, false);
        }
        touchPaneRectList(root_pane, rl);
        rl = (screenModules[root_pane->scr_mod_num]->propagate_change)(
                root_pane,
                NULL_WINDOW,
                damage
        );
    }
    freeRectList(&rl);
}


/* Run the root screen module again.                                    */
/*ARGSUSED*/
static
void
run_root_again(Generic who)
{
    run_again = true;
}


/* Send inputs to the root pane--quit when it can't handle an input.    */
int
runRoot(int argc, char **argv, OptsProcessFunc opts_process, 
	RootStartupFunc startup_func)
{
Window          win;                    /* the root window              */
Pane            *p;                     /* the unofficial root pane     */
Point           size;                   /* the unofficial size of p     */
Generic         notify_mgr;             /* the notify mgr handle        */
Generic		cp_mgr;			/* the cp manager handle	*/
Generic         dialog_mgr;             /* the dialog manager handle    */
int             ret;

    fprintf(stderr, "%s\n", D_copyright);
    
    /* make sure that we have a startup function to run... */
        if (startup_func == NULL)
	{
	  fprintf(stderr, "runRoot: invoked with NULL startup function\n");
	  return (0);
        }

    /* start the interactive world rolling */
        argc = filterStandardArgs(argc, argv);

        resolveConfiguration();

        if (opts_process != NULL)
          ret = opts_process(argc, argv);
        else
          ret = 0;

        /* bail out, if problem with args */
        if (ret < 0)
          return (-1);

	startKb();
	startRectLists();
        startEvents();
	fontsInitialize();
	screenModulesStart();

    /* create the root window */
        size = root_size;
        win.parent             = NULL_PANE;
        win.sibling[FRST_NEXT] = NULL_WINDOW;
        win.sibling[LAST_PREV] = NULL_WINDOW;
        win.child[FRST_NEXT]   = NULL_PANE;
        win.child[LAST_PREV]   = NULL_PANE;
        win.border             = makeRectFromSize(mon_event.offset, size);
        win.border_width       = 0;
	win.foreground	       = default_foreground_color;
	win.background	       = default_background_color;
	win.border_color       = default_border_color;
	win.e_fg	       = win.foreground;
	win.e_bg	       = win.background;
	win.e_bc	       = win.border_color;
        win.image              = screen_pixmap;
        win.exists             = true;
        win.showing            = true;
        win.window_information = 0;

    /* create & initialize the root pane */
    /* (while protecting against a resizeRoot call) */
        p = newPane(&win, sm_desk_get_index(), Origin, size, 1);
        resizePane(p, Origin, size);
        touchPane(p);
        sm_desk_initialize(
                p,
                (Generic) p,
                run_root_again,
                registerAsyncIO,
                unregisterAsyncIO,
                registerChildProcess,
                unregisterChildProcess
        );
        while (NOT(equalPoint(root_size, size)))
        {/* resize the mess until it stablilizes */
            size = root_size;
            win.border = makeRectFromSize(Origin, size);
            resizePane(p, Origin, size);
            touchPane(p);
        }
        (void) CURSOR(p->cursor);
        mon_event.loc = makePoint(size.x / 2, size.y / 2);
        mon_event.type = MOUSE_MOVE;
        /* ungetEvent();*/
        root_pane = p;
	p = NULL_PANE;

    /* install & initialize each of the managers (this may be slow) */
	beginComputeBound();
        menu_mgr     = sm_desk_use_manager(root_pane, &menu_manager);
        (void) menu_mgr_use((aMenuMgr*)menu_mgr);
        dialog_mgr   = sm_desk_use_manager(root_pane, &dialog_manager);
        (void) dialog_mgr_use((aDiaMgr*)dialog_mgr);

     /* show_copyright(); */

        notify_mgr   = sm_desk_use_manager(root_pane, &notify_manager);
	(void) notify_mgr_use(notify_mgr);
        cp_mgr       = sm_desk_use_manager(root_pane, &cp_manager);
        (void) cp_mgr_use(cp_mgr);
        cp_mgr_initialize((aCpMgr*)cp_mgr, num_processors, processors);
        system_mgr   = sm_desk_use_manager(root_pane, &sys_manager);
        sys_mgr_initialize((aSysMgr*)system_mgr, redraw_screen, quit_environment);

     /* hide_copyright(); */

	endComputeBound();
        ret = startup_func(argc, argv);

        if (ret == 0)
	{
	  /* run the root pane */
	  do
	  {/* run until death is requested */
	    if (run_again && !readyEvent())
	      mon_event.type = EVENT_NULL;
	    else
	      getEvent();
            run_again = false;
            (screenModules[root_pane->scr_mod_num]->input)(
							   root_pane,
							   makeRectFromSize(Origin, root_size)
							   );
	  } while (mon_event.type != EVENT_KILL);
	}

    /* destroy a root pane (while protecting against a resizeRoot call) */
	p = root_pane;
	root_pane = NULL_PANE;
	destroyPane(p);

    /* shutdown the window system */
	screenModulesFinish();
	fontsFinalize();
	stopEvents();
	finishRectLists();
	finishKb();

        return ret;
}


	/* DESKTOP FUNCTIONS */

/* load the ptrs with the redraw and shutdown functions			*/
void
root_load_funcs(PFV *redrawptr, PFV *shutdownptr)
{
    *redrawptr = redraw_screen;
    *shutdownptr = quit_environment;
}


/* Return true if window exists to circulate.				*/
Boolean
root_can_circulate()
{
    return(sm_desk_can_circulate(root_pane)) ;
}


/* Circulate the windows.						*/
void
root_circulate()
{
    sm_desk_circulate(root_pane);
}


/* simulate a root_menu event for cp_index given */
void
root_tickle_tool_cp(int id)
{
Generic         cp_desk_inst_id;	/* desk manager_id of cp_mgr	*/

    cp_desk_inst_id = sm_desk_inst_id(root_pane);
    sm_desk_tickle_cp(id, cp_desk_inst_id);
}


