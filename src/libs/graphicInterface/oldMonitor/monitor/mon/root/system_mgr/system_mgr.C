/* $Id: system_mgr.C,v 1.1 1997/06/25 14:51:46 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*                              system_mgr.c                            */
/*                             system manager                           */
/*                                                                      */
/************************************************************************/

#include <unistd.h>

#include <libs/graphicInterface/oldMonitor/include/mon/sm_def.h>
#include <libs/graphicInterface/oldMonitor/include/mon/manager.h>
#include <libs/graphicInterface/oldMonitor/monitor/mon/root/desk_sm.h>
#include <libs/graphicInterface/oldMonitor/monitor/mon/root/system_mgr.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/filer.h>
#include <signal.h>

    /*** MANAGER INFORMATION ***/

struct  sys_info {                      /* MANAGER INFORMATION STRUCT.  */
        Generic         manager_id;     /* the id of the manager        */
        RedrawFunc redrawer;       /* the screen redrawer          */
        QuitFunc             quitter;        /* the environment quitter      */
                };

STATIC(void,    sys_mgr_start,(void));              /* manager start routine        */
STATIC(Generic, sys_mgr_create,(Generic manager_id));       
                                                    /* manager create routine       */
STATIC(void,    sys_mgr_event,(aSysMgr *m));        /* manager event handler        */
STATIC(Point,   sys_mgr_window_tile,(aSysMgr *m, Window *w, Generic info,
                                     Point ulc, Boolean New));  
                                                    /* manager window tiler         */
STATIC(void,    sys_mgr_destroy,(aSysMgr *m));      /* manager destroy routine      */
STATIC(void,    sys_mgr_finish,(void));             /* manager finish routine       */

aManager        sys_manager = {         /* MANAGER DECLARATION          */
                        "system manager",
                        0,
                        sys_mgr_start,
                        sys_mgr_create,
                        (ManagerEventFunc)sys_mgr_event,
                        (ManagerWindowTileFunc)sys_mgr_window_tile,
                        (ManagerDestroyFunc)sys_mgr_destroy,
                        sys_mgr_finish
                };


/************************ MANAGER ENTRY POINTS **************************/

#define         PAUSE       1           /* pause the command processor  */
#define         REDRAW      2           /* redraw from the root up      */
#define         SHUTDOWN    3           /* quit the environment         */


/* Start the system manager.                                            */
static
void
sys_mgr_start()
{
}


/* Finish the system manager.                                           */
static
void
sys_mgr_finish()
{
}


/* Create an instance system manager.                                   */
static
Generic
sys_mgr_create(Generic manager_id)
// Generic         manager_id;             /* manager id for callbacks     */
{
aSysMgr         *m;                     /* manager instance structure   */

    /* create the manager instance structure */
        m = (aSysMgr *) get_mem(
                sizeof(aSysMgr),
                "sys_mgr_create information structure"
        );

    /* initialize the manager instance structure */
        m->manager_id = manager_id;

        m->redrawer   = (RedrawFunc) 0;
        m->quitter    = (QuitFunc) 0;

    return ((Generic) m);
}


/* Destroy an instance of the system manager.                           */
static
void
sys_mgr_destroy(aSysMgr *m)
//aSysMgr         *m;                     /* manager instance structure   */
{
    /* unregister the system options */
        sm_desk_unregister_option(PAUSE,        (aMgrInst*)m->manager_id);
        sm_desk_unregister_option(REDRAW,       (aMgrInst*)m->manager_id);
        sm_desk_unregister_option(SHUTDOWN,     (aMgrInst*)m->manager_id);

    free_mem((void*) m);
}


/* Handle an event.                                                     */
static
void
sys_mgr_event(aSysMgr *m)
//aSysMgr         *m;                     /* the manager information      */
{
    if ((mon_event.type == EVENT_SELECT) && (mon_event.from == m->manager_id))
    {/* a selection from the option list */
            switch (mon_event.msg)
            {/* do the right action based on the option */
                case PAUSE:         /* pause the environment */
                    (void) kill(getpid(), SIGTSTP);
                    break;
                case REDRAW:        /* redraw */
                    if (m->redrawer != (RedrawFunc) 0)
                    {/* we can redraw */
                        (m->redrawer)();
                    }
                    break;
#ifdef DOES_UPSHOT
		case UPSHOT:
		    call_upshot();
		    break;
#endif
                case SHUTDOWN:      /* quit */
                    if (m->quitter != (QuitFunc) 0)
                    {/* we can redraw */
                        (m->quitter)();
                    }
                    break;
            }
    }
}


/* Handle the creation of a new window.  Return desired size.           */
/*ARGSUSED*/
static
Point
sys_mgr_window_tile(aSysMgr *m, Window *w, Generic info, Point ulc, Boolean New)
// aSysMgr         *m;                     /* the manager information      */
// Window          *w;                     /* the new window               */
// Generic         info;                   /* window tiling information    */
// Point           ulc;                    /* upper left corner of tiling  */
// Boolean         New;                    /* true if a new window         */
{
    /* no windows will be created */
        return (Origin);
}



/************************** MANAGER CALLBACKS ***************************/


/* Set up for redraw callback.                                          */
void
sys_mgr_initialize(aSysMgr *m, RedrawFunc redrawer,QuitFunc quitter)
// aSysMgr         *m;                     /* the system manager           */
// PFV             redrawer;               /* the redrawer                 */
// PFV             quitter;                /* the quitter                  */
{
    m->redrawer = redrawer;
    m->quitter  = quitter;

    /* add options for system items */
        sm_desk_register_option(
                PAUSE,
                "pause",
                "Pause the environment.",
                (aMgrInst*)m->manager_id
        );
        sm_desk_register_option(
                REDRAW,
                "redraw",
                "Redraw the screen.",
                (aMgrInst*)m->manager_id
        );
#ifdef DOES_UPSHOT
        sm_desk_register_option(
                UPSHOT,
                "upshot",
                "Invoke upshot performance visualization tool.",
                m->manager_id
        );
#endif
        sm_desk_register_option(
                SHUTDOWN,
                "shutdown",
                "Quit the environment.",
                (aMgrInst*)m->manager_id
        );
}



