/* $Id: notify_mgr.ansi.c,v 1.9 1999/06/11 21:24:51 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*                              notify_mgr.c                            */
/*                          Notification manager                        */
/*                                                                      */
/************************************************************************/

#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <libs/graphicInterface/oldMonitor/include/mon/sm_def.h>
#include <libs/graphicInterface/oldMonitor/include/mon/dialog.h>
#include <libs/graphicInterface/oldMonitor/include/mon/manager.h>
#include <libs/graphicInterface/oldMonitor/monitor/mon/root/desk_sm.h>
#include <libs/graphicInterface/oldMonitor/include/mon/notify.h>
#include <libs/graphicInterface/oldMonitor/monitor/mon/root/notify_mgr.h>
#include <libs/support/strings/rn_string.h>

typedef FUNCTION_POINTER(void,HandlerFunc,(Generic,Generic,char *, int));
#ifdef LINUX
typedef FUNCTION_POINTER(void,CRHandlerFunc,(Generic,int,int *));
#else
typedef FUNCTION_POINTER(void,CRHandlerFunc,(Generic,int,union wait *));
#endif

    /*** CHILD PROCESS REGISTRATION ***/

struct  cp_reg  {                       /* PROCESS REGISTRATION LIST    */
    int                 pid;            /* the child process id         */
    Generic	        owner;          /* instance which owns pid      */
    CRHandlerFunc	handler;	/* the handler function		*/
    struct  cp_reg      *next;          /* next process registration    */
                };

struct  fd_reg  {                       /* FD REGISTRATION LIST         */
    Generic	        owner;          /* instance which owns fd       */
    HandlerFunc handler;	/* the handler function		*/
                };


    /*** MANAGER INFORMATION ***/

typedef
struct  notify_info {                   /* MANAGER INFORMATION STRUCT.  */
    Generic             manager_id;     /* the id of the manager        */
    struct  fd_reg      *fd_list;	/* fd registration list		*/
    struct  cp_reg      *child_list;    /* child pro. registration list */
                } aNfyMgr;
static  aNfyMgr *current_notify_mgr;    /* the current notify manager   */

STATIC(void,    notify_mgr_start,(void)); /* manager start routine        */
STATIC(Generic, notify_mgr_create,(Generic manager_id)); 
                                          /* manager create routine       */
STATIC(void,    notify_mgr_event,(aNfyMgr *m)); /* manager event handler        */
STATIC(Point,   notify_mgr_window_tile,(aNfyMgr *m, Window *w, Generic info,
                                        Point ulc, Boolean New));
                                           /* manager window tiler        */
STATIC(void,    notify_mgr_destroy,(aNfyMgr *m)); /* manager destroy routine      */
STATIC(void,    notify_mgr_finish,(void)); /* manager finish routine       */

aManager        notify_manager = {      /* MANAGER DECLARATION          */
                        "notify manager",
                        0,
                        notify_mgr_start,
                        notify_mgr_create,
                        (ManagerEventFunc)notify_mgr_event,
                        (ManagerWindowTileFunc)notify_mgr_window_tile,
                        (ManagerDestroyFunc)notify_mgr_destroy,
                        notify_mgr_finish
                };


/*********************** NOTIFICATION REGISTRATION **********************/


/* Add a file descriptor to the notification list.			*/
void
notify_register_async_fd(short fd, Generic owner, HandlerFunc handler, 
                         Boolean read, Boolean urgent)
{
    if ((fd < 0) || (fd >= getdtablesize()))
    {/* illegal file number */
        die_with_message(
            "notify_register_async_fd():  File descriptor %d is out of range.",
            fd
        );
    }
    else if (current_notify_mgr->fd_list[fd].handler != (HandlerFunc) 0)
    {/* this is not a valid request */
        die_with_message("notify_register_async_fd():  Duplicate registration.");
    }
    else
    {/* perform the request */
        current_notify_mgr->fd_list[fd].owner   = owner;
        current_notify_mgr->fd_list[fd].handler = handler;
        sm_desk_register_fd(fd, (aMgrInst*)current_notify_mgr->manager_id, read, urgent);
    }
}


/* Remove a file descriptor asynchronous notificaton.			*/
void
notify_unregister_async_fd(short fd)
{
    if ((fd < 0) || (fd >= getdtablesize()))
    {/* illegal file number */
        die_with_message(
            "notify_unregister_async_fd():  File descriptor %d is out of range.",
            fd
        );
    }
    else if (current_notify_mgr->fd_list[fd].handler == (HandlerFunc) 0)
    {/* there is not a registration */
        die_with_message(
            "notify_unregister_async_fd():  File descriptor not registered."
        );
    }
    else
    {/* perform the request */
        current_notify_mgr->fd_list[fd].owner   = 0;
        current_notify_mgr->fd_list[fd].handler = (HandlerFunc) 0;
        sm_desk_unregister_fd(fd, (aMgrInst*)current_notify_mgr->manager_id);
    }
}


/* Add a process from which to accept child signals.                    */
void
notify_register_process(int pid, Generic owner, CRHandlerFunc handler, Boolean urgent)
{
struct  cp_reg  *current;               /* current child registration   */
    
    for (current = current_notify_mgr->child_list; current; current = current->next)
    {/* search for a previous registration */
        if (current->pid == pid)
        {/* cannot register a signal to two owners's */
            die_with_message(
                    "notify_register_process():  Duplicate registration."
            );
        }
    }
    current = (struct cp_reg *) get_mem(
                    sizeof(struct cp_reg),
                    "notify_register_process():  child process registration"
    );
    current->pid                   = pid;
    current->owner                 = owner;
    current->handler               = handler;
    current->next                  = current_notify_mgr->child_list;
    current_notify_mgr->child_list = current;
    sm_desk_register_process(pid, (aMgrInst*)current_notify_mgr->manager_id, urgent);
}


/* Delete a preocess from which to accept child signals.                */
void
notify_unregister_process(int pid)
{
struct  cp_reg  *current;               /* current child registration   */
struct  cp_reg  **crpp;                 /* child registration ptr ptr   */

    for (crpp = &current_notify_mgr->child_list; *crpp; crpp = &(*crpp)->next)
    {/* walk down the child registration stack */
        current = *crpp;
        if (current->pid == pid)
        {/* current is the registration node--delete it */
            *crpp = current->next;
            free_mem((void*) current);
            sm_desk_unregister_process(pid, (aMgrInst*)current_notify_mgr->manager_id);
            return;
        }
    }
    die_with_message("notify_unregister_process():  No previous registration.");
}



/************************ MANAGER ENTRY POINTS **************************/


/* Start the notify manager.						*/
static
void
notify_mgr_start()
{
}


/* Finish the notify manager.                                           */
static
void
notify_mgr_finish()
{
}


/* Create an instance notify manager.                                   */
static
Generic
notify_mgr_create(Generic manager_id)
{
int		num_files;		/* the number of files allowed	*/
short           i;                      /* file descriptor index        */
aNfyMgr         *m;                     /* manager instance structure   */

    /* create the manager instance structure */
        m = (aNfyMgr *) get_mem(
                sizeof(aNfyMgr),
                "notify_mgr_create information structure"
        );

    /* initialize the manager instance structure */
        m->manager_id     = manager_id;
        m->child_list     = (struct cp_reg *) 0;

    /* clear out the file descriptor registration table */
	num_files = getdtablesize();
	m->fd_list = (struct fd_reg *) get_mem(sizeof(struct fd_reg) * num_files, "notify_mgr: file table");
        for (i = 0; i < num_files; i++)
        {/* clear out each potential asynchronous regitration */
            m->fd_list[i].owner   = 0;
            m->fd_list[i].handler = (HandlerFunc) 0;
        }

    return ((Generic) m);
}


/* Destroy an instance of the notify manager.                           */
static
void
notify_mgr_destroy(aNfyMgr *m)
{
int		num_files;		/* the number of files allowed	*/
short           i;                      /* fd registration index        */
struct  cp_reg  *cr;                    /* current child registration   */
struct  cp_reg  **crpp;                 /* child registration ptr ptr   */

    /* check the file registration table for left overs */
	num_files = getdtablesize();
        for (i = 0; i < num_files; i++)
        {/* check to see if all async registrations have been removed */
            if (m->fd_list[i].handler != (HandlerFunc) 0)
            {/* there is a registration left */
                message(
                        "Programming error:\nNotify client did not unregister\
 file descriptor %d.\nManager has done so for it (file is still open?).",
                        i
                );
                m->fd_list[i].owner   = 0;
                m->fd_list[i].handler = (HandlerFunc) 0;
                sm_desk_unregister_fd(i, (aMgrInst*)m->manager_id);
            }
        }
	free_mem((void*) m->fd_list);

    /* check the child registration table for left overs */
        crpp = &m->child_list;
        while (*crpp)
        {/* walk down the child registration stack */
            cr = *crpp;
            if (cr->handler != (CRHandlerFunc) 0)
            {/* cr is the unregistered node--delete it */
                message(
                        "Programming error:\nNotify client did not unregister\
 child process %d.\nManager has done so for it (process is still running?).",
                        cr->pid
                );
                sm_desk_unregister_process(cr->pid, (aMgrInst*)m->manager_id);
                *crpp = cr->next;
                free_mem((void*) cr);
                continue;   /* we have already advanced */
            }
            crpp = &(*crpp)->next;
        }

    free_mem((void*) m);
}


/* Handle an event to one of the clients.                               */
static
void
notify_mgr_event(aNfyMgr *m)
{
struct  fd_reg  *fr;                    /* current fd registration      */
struct  cp_reg  *cr;                    /* current child registration   */

    switch (mon_event.type)
    {/* do any necessary translation */
        case EVENT_IO:      /* file event */
	    fr = &m->fd_list[mon_event.from];
            (fr->handler)(fr->owner, mon_event.from, (char *) mon_event.msg, mon_event.info.x);
            break;

        case EVENT_SIGCHLD: /* child event */
            for (cr = m->child_list; cr; cr = cr->next)
            {/* search for the appropriate entry */
                if (cr->pid == (int) mon_event.from)
                {/* the registration has been found */
                    break;
                }
            }
#ifdef LINUX
	    (cr->handler)(cr->owner, cr->pid, (int *) mon_event.msg);
#else
	    (cr->handler)(cr->owner, cr->pid, (union wait *) mon_event.msg);
#endif
            break;
	default:
	    die_with_message("Notify_mgr.c:  Unknown event.");
	    break;
    }
}


/* Handle the creation of a new window.  Return desired size.           */
/*ARGSUSED*/
static
Point
notify_mgr_window_tile(aNfyMgr *m, Window *w, Generic info, 
                       Point ulc, Boolean New)
{
    return Origin;
}



/************************** MANAGER CALLBACKS ***************************/


/* Set the manager to use for subsequent calls.                         */
Generic
notify_mgr_use(Generic nm)
{
aNfyMgr         *saved;                 /* the saved handle             */

    saved = current_notify_mgr;
    current_notify_mgr = (aNfyMgr *) nm;
    return ((Generic) saved);
}
