/* $Id: stand_alone.ansi.c,v 1.11 1999/03/31 22:05:36 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*                              stand_alone.c                           */
/*                      stand-alone database calls                      */
/*                                                                      */
/************************************************************************/

#include <stdio.h>
#include <unistd.h>

#include <libs/graphicInterface/oldMonitor/include/mon/sm_def.h>
#include <libs/graphicInterface/oldMonitor/include/mon/event.h>
#include <libs/graphicInterface/oldMonitor/include/mon/manager.h>
#include <libs/graphicInterface/oldMonitor/monitor/mon/root/notify_mgr.h>
#include <libs/graphicInterface/oldMonitor/include/mon/standalone.h>
#include <libs/config/config.h>


extern	char    D_copyright[];		/* the copyright message	*/

    /*** CHILD PROCESS REGISTRATION ***/

struct amgrinst;
typedef
struct  cp_reg  {                       /* PROCESS REGISTRATION STACK   */
    int                 pid;            /* child process id             */
    struct  amgrinst    *owner;         /* manager instance which owns  */
    Generic             id;             /* the id of the event          */
    struct  cp_reg      *next;          /* next process registration    */
                } aCR;
#define         NULL_REG_STACK ((aCR *) 0)/* end of the registration    */


    /*** MANAGER INSTANCE INFORMATION ***/

typedef
struct  amgrinst{                       /* A MANAGER INSTANCE           */
    aManager            *mgr;           /* the manager structure        */
    Generic             info;           /* the manager information      */
    Boolean             run_again;      /* true if we should run again  */
    struct  amgrinst    *next;          /* the next manager             */
                } aMgrInst;
#define         NULL_MAN_INST ((aMgrInst *) 0)/* null manager instance  */


    /*** LOCAL INFORMATION ***/

static
aMgrInst        **fd_owners;            /* registered file owners       */
static
struct  cp_reg  *reg_stack;             /* process registration stack   */
static
aMgrInst        *inst_list;             /* current instance list        */



/**************************** local routines ****************************/


/* Begin to use a new manager.                                          */
static
Generic
use_manager(aManager *mgr)
{
aMgrInst        *mi;                    /* the new manager instance     */

    if (mgr->ref_count++ == 0)
    {/* start the manager */
        (mgr->start)();
    }
    mi = (aMgrInst *) get_mem(
            sizeof(aMgrInst),
            "use_manager(%s)",
            mgr->name
    );
    mi->mgr   = mgr;
    mi->info  = (mgr->create)((Generic) mi);
    mi->next  = inst_list;
    inst_list = mi;
    return (mi->info);
}


/* Handle an event to one of the managers.                              */
static
void
handle_event()
{
aMgrInst        *mi = NULL_MAN_INST;    /* the manager to send the event*/
struct  cp_reg  *cr;                    /* current child registration   */

    switch (mon_event.type)
    {/* handle the event */
        case EVENT_IO:      /* asynchronous io event */
            mi = fd_owners[mon_event.from];
            (mi->mgr->event)(mi->info);
            break;

        case EVENT_SIGCHLD: /* child process event */
            for (cr = reg_stack; cr; cr = cr->next)
            {/* search for the registration for this child */
                if (cr->pid == (int) mon_event.from)
                {/* the registration has been found */
                    mi = cr->owner;
                    break;
                }
            }
            (mi->mgr->event)(mi->info);
            break;

        default:
            die_with_message("handle_event(): bogus event.");
            break;
    }
}



/********************** desk_sm and root callbacks **********************/


/* Register a file descriptor event type.                               */
void
sm_desk_register_fd(short fd, Generic mi, Boolean buffer, Boolean urgent)
{
    fd_owners[fd] = (aMgrInst *) mi;
    registerAsyncIO(fd, 0, buffer, urgent);
}


/* Unregister a file descriptor event type.                             */
/*ARGSUSED*/
void
sm_desk_unregister_fd(short fd, Generic mi)
{
    fd_owners[fd] = NULL_MAN_INST;
    unregisterAsyncIO(fd, 0);
}


/* Register a child process event type.                                 */
void
sm_desk_register_process(int pid, Generic mi, Boolean urgent)
{
struct  cp_reg  *current;               /* current child registration   */
    
    current = (struct cp_reg *) get_mem(
            sizeof(struct cp_reg),
            "sm_desk_register_process(): new registration"
    );
    current->pid   = pid;
    current->owner = (aMgrInst *) mi;
    current->next  = reg_stack;
    reg_stack      = current;
    registerChildProcess(pid, 0, urgent);
}


/* Unregister a child process event type.                               */
/*ARGSUSED*/
void
sm_desk_unregister_process(int pid, Generic mi)
{
struct  cp_reg  *current;               /* current child registration   */
struct  cp_reg  **crpp;                 /* child registration ptr ptr   */

    for (crpp = &reg_stack; *crpp; crpp = &(*crpp)->next)
    {/* walk down the child registration stack */
        current = *crpp;
        if (current->pid == pid)
        {/* current is the registration node--delete it */
            *crpp = current->next;
            free_mem((void*) current);
            unregisterChildProcess(pid, 0);
            return;
        }
    }
    die_with_message("sm_desk_unregister_process():  bogus request.");
}


/* Redraw a damaged area of the root window.                            */
void
redrawRootRectList(RectList rl)
{
    freeRectList(&rl);
}


/* Change the size of the root window.  May be called asynchronously.   */
/*ARGSUSED*/
void
resizeRoot(Point size)
{
}


/********************** database client calls ***************************/


/* Startup the stand-alone database.                                    */
void
monInit()
{
int		num_files;		/* the number of files allowed	*/
short           i;                      /* file list index              */

    /* start up the environment */
/* startEvents();  I don't need this for Memoria and f2i, plus it's seg faulting*/

    /* initialize the management info */
	num_files = getdtablesize();
	fd_owners = (aMgrInst **) get_mem(sizeof(aMgrInst *) * num_files, "stand_alone.c: file owners");
        for (i = 0; i < num_files; i++)
        {/* zero out the file descriptor owners */
            fd_owners[i] = NULL_MAN_INST;
        }
        reg_stack = NULL_REG_STACK;
        inst_list = NULL_MAN_INST;

    /* initialize the managers */
        (void) notify_mgr_use(use_manager(&notify_manager));
}

/* Firedown the stand-alone database.                                   */
void
monFini()
{
aMgrInst        *inst;                  /* the manager instance         */

    /* finalize the management info */
        while(inst_list)
        {/* there is an alive manager on the list */
            /* remove the first manager instance */
                inst = inst_list;
                inst_list = inst->next;

            /* fire down the instance */
                (inst->mgr->destroy)(inst->info);
                if (--inst->mgr->ref_count == 0)
                {/* finish off the manager */
                    (inst->mgr->finish)();
                }

            /* destroy the instance */
                free_mem((void*) inst);
        }

    /* fire-down the environment */
        /* stopEvents(); not needed for Memoria and f2i */
    free_mem((void*) fd_owners);
}

/* Check on database notification.                                      */
void
databaseCheckNotification()
{
    while (readyEvent())
    {/* get and deliver an event */
        getEvent();
        if (mon_event.type != MOUSE_EXIT)
        {/* we will get exit events from getEvent */
             handle_event();
        }
    }
}


/* Wait for a database notification.                                    */
void
databaseWaitNotification()
{
    do
    {/* get a real event */
        getEvent();
    } while (mon_event.type == MOUSE_EXIT);
    handle_event();
}

/* 
 * run a procedure in the context of the standalone monitor
 */

int
runRoot(int argc, char **argv, OptsProcessFunc opts_process, RootStartupFunc startup_func)
{
  int  ret;

  fprintf(stderr, "%s\n", D_copyright);  

  if (startup_func == NULL)
  {
    fprintf(stderr, "runRoot: invoked with NULL startup function\n");
    return -1;
  }  

  /* process the standard startup arguments */
  argc = filterStandardArgs(argc, argv);

  resolveConfiguration();

  monInit();

  if (opts_process != NULL)
     ret = opts_process(argc, argv);
  else
     ret = 0;

  if (ret == 0)
    ret = startup_func(argc, argv);
 
  monFini();
  return ret;
}

