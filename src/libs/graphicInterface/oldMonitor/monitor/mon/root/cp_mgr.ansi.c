/* $Id: cp_mgr.ansi.c,v 1.8 1997/03/11 14:33:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*                                cp_mgr.c                              */
/*                        Command processor manager                     */
/*                                                                      */
/************************************************************************/

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <libs/graphicInterface/oldMonitor/include/mon/sm_def.h>
#include <libs/graphicInterface/oldMonitor/include/mon/manager.h>
#include <libs/graphicInterface/oldMonitor/monitor/mon/root/desk_sm.h>
#include <libs/graphicInterface/oldMonitor/monitor/mon/root/cp_mgr.h>
#include <libs/graphicInterface/oldMonitor/include/mon/cp.h>
#include <libs/support/strings/rn_string.h>

extern int global_argc;
extern char **global_argv;
extern PFV  global_startup_func;

    /*** MESSAGE QUEUE ***/

struct  mq_node {                       /* MESSAGE QUEUE NODE           */
    struct  mq_node     *older;         /* the next older message       */
    struct  mq_node     *newer;         /* the next newer message       */
    Generic             receiver;       /* the receiver of the message  */
    Generic             sender;         /* the sender of the message    */
    Point               info;           /* the message info parameter   */
    Generic             msg;            /* the message pointer parameter*/
                };
typedef
struct  mq_head {                       /* MESSAGE QUEUE HEADER         */
    struct  mq_node     *valid_oldest;  /* oldest valid message         */
    struct  mq_node     *valid_newest;  /* newest valid message         */
    struct  mq_node     *freed_oldest;  /* oldest freed message         */
    struct  mq_node     *freed_newest;  /* newest freed message         */
                } MessageQ;
#define         NULL_MQ (struct mq_node *)0/* end of a message queue    */


    /*** CHILD PROCESS REGISTRATION ***/

struct  cp_reg  {                       /* PROCESS REGISTRATION LIST    */
    int                 pid;            /* the child process id         */
    struct  inst        *owner;         /* cp instance which owns pid   */
    struct  cp_reg      *next;          /* next process registration    */
                };


    /*** COMMAND PROCESSOR INSTANCE ***/

struct  inst    {               /* COMMAND PROCESSOR INSTANCE   */
    struct  cp_info     *cp_mgr;        /* cp manager that manages it   */
    struct  inst        *parent;        /* the parent cp of this        */
    struct  inst        *sibling;       /* older cp with same parent    */
    struct  inst        *child;         /* oldest child cp              */
    short               cp_num;         /* the command processor index  */
    short               family_count;   /* # alive decendents + self    */
    short               message_count;  /* # pending non-self messages  */
    Generic             instance_info;  /* the handle to cp's info      */
                        } ;
#define         NULL_INSTANCE (anInstance *) 0/* null cp instance       */


    /*** WINDOW TILING ***/

struct  w_tile  {                       /* TILING DESCRITPTION          */
    Boolean             is_pane;        /* true if union is a pane      */
    Point               size;           /* size of the tile             */
    union   {                           /* pointer to an arbitrary son  */
            Pane            **area_pane_ptr;/* single pane tiling       */
            struct      {
                    aTilingDir          dir;    /* the orientation      */
                    struct  w_tile      *td1;   /* first tiling         */
                    struct  w_tile      *td2;   /* second tiling        */
                        }   join;           /* join tiling              */
            }           tile;           /* a pane or join tiling desc.  */
                };

#define         NULL_TILING (aTilingDesc *)0/* a null tiling descriptor */


    /*** WINDOW REGISTRATION ***/

struct  win_reg {                       /* WINDOW REGISTRATION STRUCTURE*/
    Window              *w;             /* the registered window        */
    anInstance          *inst;          /* the instance owning the win. */
    struct  win_reg     *next;          /* next registration            */
                };


    /*** MANAGER INFORMATION ***/

struct  cp_info {                       /* MANAGER INFORMATION STRUCT.  */
    MessageQ            *message_q;     /* the message queue            */
    anInstance          *adam_cp;       /* manager owned global parent  */
    Generic             manager_id;     /* the id of the manager        */
    short               num_processors; /* the number of processors     */
    aProcessor          **processors;   /* the processor array          */
    anInstance          **owner_inst;   /* fd owning instances          */
    struct  cp_reg      *child_list;    /* child pro. registration list */
    struct  win_reg     *win_list;      /* window registration list     */
                };
static  aCpMgr      *current_cpm;       /* the id of the cp manager     */

STATIC(void, set_tiling_sizes, (aTilingDesc* td, Point constraint));
STATIC(Point, set_panes_and_free, (Window* w, aTilingDesc* td, Point position));
STATIC(Boolean, pane_in_description, (Pane* p, aTilingDesc* td));
STATIC(void, print_tiling_descriptor, (aTilingDesc* td, short off));
STATIC(void, cp_mgr_start, (void));
STATIC(void, cp_mgr_finish, (void));
STATIC(Generic, cp_mgr_create, (Generic manager_id));
STATIC(anInstance*, find_victim, (anInstance* root));
STATIC(void, remove_family, (anInstance* root));
STATIC(void, give_cp_event, (anInstance* cp));
STATIC(void, cp_mgr_destroy, (aCpMgr* m));
STATIC(void, cp_mgr_event, (aCpMgr* m));
STATIC(Point, cp_mgr_window_tile, (aCpMgr* m, Window* w, aTilingDesc* td, 
                                   Point ulc, Boolean new));


aManager        cp_manager = {          /* MANAGER DECLARATION          */
                        "command processor manager",
                        0,
                        cp_mgr_start,
                        cp_mgr_create,
                        cp_mgr_event,
                        cp_mgr_window_tile,
                        cp_mgr_destroy,
                        cp_mgr_finish
                };



/********************** MESSAGE QUEUE MANIPULATION **********************/


/* Create a message queue.                                              */
static 
MessageQ*
createMessageQ(char* s)
/* s - the identification string    */
{
MessageQ        *mq;                    /* the message queue to return  */

    mq = (MessageQ *) get_mem(sizeof(MessageQ), "message_q for %s", s);
    mq->valid_oldest = NULL_MQ;
    mq->valid_newest = NULL_MQ;
    mq->freed_oldest = NULL_MQ;
    mq->freed_newest = NULL_MQ;
    return mq;
}


/* Return true if there is something in the message queue.              */
static
Boolean
readyMessageQ(MessageQ* mq)
/* mq - the message queue to check   */
{
    return (BOOL(mq->valid_oldest));
}


/* Get a message off the queue into mon_event.                          */
static
void
getFromMessageQ(MessageQ* mq)
/* mq - the message queue            */
{
struct  mq_node     *dead_node;         /* pointer to the dead node     */

    dead_node = mq->valid_oldest;
    if (dead_node == mq->valid_newest)
    {/* there was only one node on the valid queue */
        mq->valid_oldest = NULL_MQ;
        mq->valid_newest = NULL_MQ;
    }
    else
    {/* there are many nodes on the valid queue--detach the first */
        mq->valid_oldest = dead_node->newer;
        mq->valid_oldest->older = NULL_MQ;
    }
    mon_event.type = EVENT_MESSAGE;
    mon_event.to   = (Generic) dead_node->receiver;
    mon_event.from = (Generic) dead_node->sender;
    mon_event.info = dead_node->info;
    mon_event.msg  = dead_node->msg;
    dead_node->older = NULL_MQ;
    dead_node->newer = NULL_MQ;
    if (mq->freed_newest == NULL_MQ)
    {/* the freed queue is empty so make dead_node be the only node */
        mq->freed_oldest = dead_node;
    }
    else
    {/* the new node is just one of many */
        dead_node->older = mq->freed_newest;
        mq->freed_newest->newer = dead_node;
    }
    mq->freed_newest = dead_node;
}


/* Put a new message at the end of the message queue.                   */
static
void
addToMessageQ(MessageQ* mq, Generic receiver, Generic sender, Point info, Generic msg)
/* mq - the message queue            */
/* receiver - the receiver                 */
/* sender - the sender                   */
/* info - the information point field  */
/* msg - the information pointer field*/
{
struct  mq_node *new;                   /* the new queue entry          */

    /* get a new structure */
        if (mq->freed_oldest == NULL_MQ)
        {/* there is no structure on the freed queue--create another */
            new = (struct mq_node *) get_mem(
                        sizeof(struct mq_node),
                        "addToMessageQ(): new message queue entry created"
            );
        }
        else
        {/* remove the oldest entry from the freed list */
            new = mq->freed_oldest;
            if (new == mq->freed_newest)
            {/* we just took the last entry off of the freed list */
                mq->freed_oldest = NULL_MQ;
                mq->freed_newest = NULL_MQ;
            }
            else
            {/* fix up the freed queue */
                mq->freed_oldest = new->newer;
                mq->freed_oldest->older = NULL_MQ;
            }
        }

    /* initialize the new structure */
        new->receiver = receiver;
        new->sender   = sender;
        new->info     = info;
        new->msg      = msg;
        new->older    = NULL_MQ;
        new->newer    = NULL_MQ;

    /* put the structure on the new end of the valid list */
        if (mq->valid_newest == NULL_MQ)
        {/* the valid queue is empty so make new the only entry */
            mq->valid_oldest = new;
        }
        else
        {/* the new entry is just one of many */
            new->older = mq->valid_newest;
            mq->valid_newest->newer = new;
        }
        mq->valid_newest = new;
}


/* destroy a message queue.                                             */
static
void
destroyMessageQ(MessageQ* mq)
/* mq - the message queue            */
{
struct  mq_node *next;                  /* next entry in either queue   */

    while (mq->valid_oldest != NULL_MQ)
    {/* remove the oldest entry from the valid list */
        next = mq->valid_oldest->newer;
        free_mem((void*) mq->valid_oldest);
        mq->valid_oldest = next;
    }
    mq->valid_newest = NULL_MQ;

    while (mq->freed_oldest != NULL_MQ)
    {/* remove the oldest entry from the freed list */
        next = mq->freed_oldest->newer;
        free_mem((void*) mq->freed_oldest);
        mq->freed_oldest = next;
    }
    mq->freed_newest = NULL_MQ;
    free_mem((void*) mq);
}



/*************** COMMAND PROCESSOR INSTANCE MANIPULATION ****************/


/* Create a cp from the root with standard argument.                    */
void
cp_standard_root_starter(anInstance* creator, short cp_index)
/* creator - the creator of this instance     */
/* cp_index - the index of the new cp          */
{
    (void) cp_new(
            creator,
            cp_index,
            creator->cp_mgr->processors[cp_index]->root_startup_arg
    );
}


/* Return true if instances can be created (start if necessary).        */
Boolean
cp_assert_need(aCpMgr* m, short cp_index)
/* m - the cp_manager                   */
/* cp_index - index of cp to create            */
{
    if ((cp_index < 0) || (cp_index >= m->num_processors))
    {/* the index is out of range -- we can't start this */
        return (false);
    }
    else if (m->processors[cp_index]->status == CP_UNSTARTED)
    {/* it is unknown if the command processor will start */
        m->processors[cp_index]->status =
                ((m->processors[cp_index]->start_cp)((Generic) m))
                    ? CP_NOMINAL
                    : CP_CANNOT_START;
    }
    return (BOOL(m->processors[cp_index]->status == CP_NOMINAL));
}


/* Create a new instance of a command processor.                        */
Generic
cp_new(anInstance* creator, short cp_index, Generic arg)
/* creator - the instance creating the cp */
/* cp_index - index of cp to create        */
/* arg - the argument to send to it   */
{
anInstance      *new = NULL_INSTANCE;   /* the new cp instance          */
anInstance      *ancestor;              /* the current ancestor of new  */

    if (cp_assert_need(creator->cp_mgr, cp_index))
    {/* the command processor can be started */
        new = (anInstance *) get_mem(sizeof(anInstance), "cp instance");
        new->cp_mgr        = creator->cp_mgr;
        new->parent        = creator;
        new->sibling       = creator->child;
        new->child         = NULL_INSTANCE;
        new->cp_num        = cp_index;
        new->family_count  = 0;
        new->message_count = 0;
        creator->child     = new;
        for (ancestor = new; ancestor; ancestor = ancestor->parent)
        {/* increment the decendant count of the ancestors & new child */
            ancestor->family_count++;
        }

        new->instance_info = 
                (creator->cp_mgr->processors[cp_index]->create_instance)(
                    (Generic)creator,
                    (Generic)new,
                    arg);
        if (new->instance_info == UNUSED)
        {/* the command processor aborted the startup */
            while (new)
            {/* propagate death through all ancestors */
                new->family_count--;
                new = new->parent;
            }
            new = NULL_INSTANCE;
        }
    }
    return ((Generic) new);
}


/* Return the cp_id of the root command processor.                      */
Generic
cp_root_cp_id()
{
    return ((Generic) current_cpm->adam_cp);
}


/* Send a message to a command processor.                               */
Boolean
cp_message(anInstance* sender, anInstance* receiver, Point info, Generic msg)
/* sender - the sending commmand processor   */
/* receiver - the receiving commmand processor */
/* info - the point parameter              */
/* msg - the pointer parameter            */
{
    if (receiver->instance_info == UNUSED)
    {/* the message cannot be sent -- the receiver instance has died */
        return (false);
    }
    else
    {/* the message can be sent */
        if (receiver != sender)
        {/* the message is not to itself & is considered important */
            receiver->message_count++;
        }
        addToMessageQ(
                sender->cp_mgr->message_q,
                (Generic) receiver,
                (Generic) sender,
                info,
                msg
        );
        sm_desk_give_event((aMgrInst*)sender->cp_mgr->manager_id);
        return (true);
    }
}


/* Check to see if a command processor can successfully die.            */
Boolean
cp_assert_death(anInstance* dier)
/* dier - the cp instance of interest      */
{
    /* Note: dier->message_count represents the number of messages      */
    /* pending for the command processor instance that are not from the */
    /* instance itself.                                                 */
    return (NOT(dier->message_count));
}



/******************* CP EXPECTED EVENT REGISTRATION *********************/


/* Add a file descriptor from which to accept asynchronous input.       */
void
cp_register_async_fd(short fd, anInstance* inst, Boolean buffer)
/* fd - file to be registered        */
/* inst - the owner instance of fd     */
/* buffer - true if we should buffer     */
{
    if ((fd < 0) || (fd >= getdtablesize()))
    {/* illegal file number */
        die_with_message(
            "cp_register_async_fd():  File descriptor %d is out of range.",
            fd
        );
    }
    else if (inst->cp_mgr->owner_inst[fd] != NULL_INSTANCE)
    {/* this is not a valid request */
        die_with_message("cp_register_async_fd():  Duplicate registration.");
    }
    else
    {/* perform the request */
        inst->cp_mgr->owner_inst[fd] = inst;
        sm_desk_register_fd(fd, (aMgrInst*)inst->cp_mgr->manager_id, buffer, false);
    }
}


/* Remove a file descriptor from which to accept asynchronous input.    */
void
cp_unregister_async_fd(short fd, anInstance* inst)
/* fd - file to be unregistered      */
/* inst - the owner instance of fd     */
{
    if ((fd < 0) || (fd >= getdtablesize()))
    {/* illegal file number */
        die_with_message(
            "cp_unregister_async_fd():  File descriptor %d is out of range.",
            fd
        );
    }
    else if (inst->cp_mgr->owner_inst[fd] != inst)
    {/* there is not a registration */
        die_with_message(
            "cp_unregister_async_fd():  File descriptor not registered."
        );
    }
    else
    {/* perform the request */
        inst->cp_mgr->owner_inst[fd] = NULL_INSTANCE;
        sm_desk_unregister_fd(fd, (aMgrInst*)inst->cp_mgr->manager_id);
    }
}


/* Add a process from which to accept child signals.                    */
void
cp_register_process(int pid, anInstance* inst)
/* pid - process id to be registered  */
/* inst - the owner of the process     */
{
struct  cp_reg  *current;               /* current child registration   */
    
    for (current = inst->cp_mgr->child_list; current; current = current->next)
    {/* search for a previous registration */
        if (current->pid == pid)
        {/* cannot register a signal to two cp's */
            die_with_message(
                    "cp_register_process():  Duplicate registration."
            );
        }
    }
    current = (struct cp_reg *) get_mem(
                    sizeof(struct cp_reg),
                    "cp_register_process():  child process registration"
    );
    current->pid             = pid;
    current->owner           = inst;
    current->next            = inst->cp_mgr->child_list;
    inst->cp_mgr->child_list = current;
    sm_desk_register_process(pid, (aMgrInst*)inst->cp_mgr->manager_id, false);
}


/* Delete a preocess from which to accept child signals.                */
void
cp_unregister_process(int pid, anInstance* inst)
/* pid - process id to be unregistered*/
/* inst - the owner instance of pid    */
{
struct  cp_reg  *current;               /* current child registration   */
struct  cp_reg  **crpp;                 /* child registration ptr ptr   */

    for (crpp = &inst->cp_mgr->child_list; *crpp; crpp = &(*crpp)->next)
    {/* walk down the child registration stack */
        current = *crpp;
        if (current->pid == pid)
        {/* current is the registration node--delete it */
            *crpp = current->next;
            free_mem((void*) current);
            sm_desk_unregister_process(pid, (aMgrInst*)inst->cp_mgr->manager_id);
            return;
        }
    }
    die_with_message("cp_unregister_process():  No previous registration.");
}



/****************** TILING DESCRIPTION MANIPULATION *********************/


/* Set the tile sizes for each tile in a tiling description.            */
static
void
set_tiling_sizes(aTilingDesc* td, Point constraint)
/* td - tiling description for a window  */
/* constraint - constrained size in either dir.  */
{
    if (td->is_pane)
    {/* the description is for a pane */
        if (constraint.x != UNUSED)
        {/* there is a constraint in the x direction */
            td->size.x = constraint.x;
        }
        if (constraint.y != UNUSED)
        {/* there is a constraint in the y direction */
            td->size.y = constraint.y;
        }
    }
    else if (
            (td->tile.join.td1 == NULL_TILING) &&
            (td->tile.join.td2 == NULL_TILING)
    )
    {/* both children are null -- give tile zero size within constraints */
        td->size.x = (constraint.x != UNUSED) ? constraint.x : 0;
        td->size.y = (constraint.y != UNUSED) ? constraint.y : 0;
    }
    else if (td->tile.join.td1 == NULL_TILING)
    {/* the first branch is null -- figure based on the second branch */
        set_tiling_sizes(td->tile.join.td2, constraint);
        td->size = td->tile.join.td2->size;
    }
    else if (td->tile.join.td2 == NULL_TILING)
    {/* the second branch is null -- figure based on the first branch */
        set_tiling_sizes(td->tile.join.td1, constraint);
        td->size = td->tile.join.td1->size;
    }
    else if (td->tile.join.dir == TILE_UP || td->tile.join.dir == TILE_DOWN)
    {/* join vertically two non-null descriptors */
        set_tiling_sizes(td->tile.join.td1, makePoint(constraint.x, UNUSED));
        if (
            (constraint.y != UNUSED) &&
            (td->tile.join.td1->size.y > constraint.y)
        )
        {/* there is a height constraint which has been violated--redo */
            set_tiling_sizes(td->tile.join.td1, constraint);
        }
        constraint.x =  td->tile.join.td1->size.x;
        constraint.y -= (constraint.y == UNUSED)
                            ? 0 : td->tile.join.td1->size.y;
        set_tiling_sizes(td->tile.join.td2, constraint);
        td->size.x = td->tile.join.td1->size.x;
        td->size.y = td->tile.join.td1->size.y   + td->tile.join.td2->size.y;
    }
    else
    {/* join horizontally two non-null descriptors */
        set_tiling_sizes(td->tile.join.td1, makePoint(UNUSED, constraint.y));
        if (
            (constraint.x != UNUSED) &&
            (td->tile.join.td1->size.x > constraint.x)
        )
        {/* there is a width constraint which has been violated--redo */
            set_tiling_sizes(td->tile.join.td1, constraint);
        }
        constraint.x -= (constraint.x == UNUSED)
                            ? 0 : td->tile.join.td1->size.x;
        constraint.y =  td->tile.join.td1->size.y;
        set_tiling_sizes(td->tile.join.td2, constraint);
        td->size.x = td->tile.join.td1->size.x + td->tile.join.td2->size.x;
        td->size.y = td->tile.join.td1->size.y;
    }
}


/* Set the pane positions and sizes based on a tiling description.      */
/* Free the td.  Create panes as necessary.  Return td size.            */
static
Point
set_panes_and_free(Window* w, aTilingDesc* td, Point position)
/* w - the window being tiled           */
/* td - the current tiling description   */
/* position - starting position for this tiling*/
{
Point           size;		    /* some size amount			*/

    if (td->is_pane)
    {/* the description is a pane--set the position */
        if ((int) *td->tile.area_pane_ptr < MAX_SMs)
        {/* the pane has not been created yet */
            *td->tile.area_pane_ptr = newPane(
                    w,
                    (short)((int)*td->tile.area_pane_ptr),
                    position,
                    td->size,
                    1
            );
        }
        else
        {/* the pane has been created--change its size appropriately */
            (*td->tile.area_pane_ptr)->size     = td->size;
            (*td->tile.area_pane_ptr)->position = position;
        }
    }
    else if (
            (td->tile.join.td1 == NULL_TILING) &&
            (td->tile.join.td2 == NULL_TILING)
    )
    {/* both children are null--do nothing */
    }
    else if (td->tile.join.td1 == NULL_TILING)
    {/* the first branch is null -- figure based on the second branch */
        (void) set_panes_and_free(w, td->tile.join.td2, position);
    }
    else if (td->tile.join.td2 == NULL_TILING)
    {/* the second branch is null -- figure based on the first branch */
        (void) set_panes_and_free(w, td->tile.join.td1, position);
    }
    else
    {/* the description is a join of two tilings */
        switch (td->tile.join.dir)
        {/* handle each possible direction */
            case TILE_UP:
                size = set_panes_and_free(w, td->tile.join.td1, position);
                position.y += size.y;
                (void) set_panes_and_free(w, td->tile.join.td2, position);
                break;
            case TILE_DOWN:
                size = set_panes_and_free(w, td->tile.join.td2, position);
                position.y += size.y;
                (void) set_panes_and_free(w, td->tile.join.td1, position);
                break;
            case TILE_LEFT:
                size = set_panes_and_free(w, td->tile.join.td1, position);
                position.x += size.x;
                (void) set_panes_and_free(w, td->tile.join.td2, position);
                break;
            case TILE_RIGHT:
                size = set_panes_and_free(w, td->tile.join.td2, position);
                position.x += size.x;
                (void) set_panes_and_free(w, td->tile.join.td1, position);
                break;
        }
    }
    size = td->size;
    free_mem((void*) td);
    return size;
}


/* Create and return a tiling description for a pane.                   */
Generic
cp_td_pane(Pane** ppr, Point size)
/* ppr - pointer to the pane address  */
/* size - the size of the pane         */
{
aTilingDesc     *a;                     /* returned tiling description  */

    a = (aTilingDesc *) get_mem (sizeof(aTilingDesc), "cp_td_pane()");
    a->is_pane            = true;
    a->size               = size;
    a->tile.area_pane_ptr = ppr;
    return ((Generic) a);
}


/* Create and return a tiling description for a join of two tilings.    */
Generic
cp_td_join(aTilingDir dir, aTilingDesc* td1, aTilingDesc* td2)
/* dir - orientation of td1 & td2     */
/* td1 - the first tiling description */
/* td2 - the second tiling description*/
{
aTilingDesc     *b;                     /* returned tiling description  */

    b = (aTilingDesc *) get_mem (sizeof(aTilingDesc), "cp_td_join()");
    b->is_pane       = false;
    b->size          = Origin;
    b->tile.join.dir = dir;
    b->tile.join.td1 = td1;
    b->tile.join.td2 = td2;
    return ((Generic) b);
}


/* Return the size of a tiling description.                             */
Point
cp_td_size(aTilingDesc* td)
/* td - the tiling description       */
{
    if (td == NULL_TILING)
    {/* this is a null descriptor */
        return (Origin);
    }
    else
    {/* there is a valid (hopefully) descriptor there */
        set_tiling_sizes(td, makePoint(UNUSED, UNUSED));
        return (td->size);
    }
}


/* Recursively search a tiling description tree for a pane.             */
static
Boolean
pane_in_description(Pane* p, aTilingDesc* td)
/* p - pane which is sought         */
/* td - current tiling description   */
{
    if (td->is_pane)
    {/* the description is a pane--check directly */
        return (BOOL(p == *td->tile.area_pane_ptr));
    }
    else
    {/* the description is a join of two tilings */
        return BOOL(
            td->tile.join.td1 && pane_in_description(p, td->tile.join.td1) ||
            td->tile.join.td2 && pane_in_description(p, td->tile.join.td2)
        );
    }
}


/* A debugging call to print out a tiling description.                  */
/* Invoke with off = 0.                                                 */
static
void
print_tiling_descriptor(aTilingDesc* td, short off)
/* td - tiling descriptor to print   */
/* off - the offset to print it       */
{
short           i;                      /* space counter                */

    for (i = 0; i < off; i++)
        (void) printf(" ");
    if (td == NULL_TILING)
    {/* there is no tiling descriptor here */
        (void) printf("NULL TILING\n");
    }
    else if (td->is_pane)
    {/* the description is for a pane */
        if ((int) *td->tile.area_pane_ptr < MAX_SMs)
        {/* the pane has not been created yet */
            (void) printf(
                    "Uncreated pane: %d (%d x %d) (td = %#x)\n",
                    (int) *td->tile.area_pane_ptr,
                    td->size.x,
                    td->size.y,
                    td
            );
        }
        else
        {/* the pane has been created--change its size appropriately */
            (void) printf(
                    "Created pane: %d(%d x %d) (td = %#x)\n",
                    (*td->tile.area_pane_ptr)->scr_mod_num,
                    td->size.x,
                    td->size.y,
                    td
            );
        }
    }
    else
    {/* the description is a join of two tilings */
        (void) printf(
                "Join (%d x %d) (td = %#x) ",
                td->size.x,
                td->size.y,
                td
        );
        switch (td->tile.join.dir)
        {/* handle each possible direction of join */
            case TILE_UP:
                (void) printf("up:\n");
                break;
            case TILE_DOWN:
                (void) printf("Down:\n");
                break;
            case TILE_LEFT:
                (void) printf("Left:\n");
                break;
            case TILE_RIGHT:
                (void) printf("Right:\n");
                break;
        }
        print_tiling_descriptor(td->tile.join.td1, off + 5);
        for (i = 0; i < off; i++)
            (void) printf(" ");
        (void) printf("and\n");
        print_tiling_descriptor(td->tile.join.td2, off + 5);
    }
    if (off == 0)
        (void) fflush(stdout);
    
}



/*************** COMMAND PROCESSOR WINDOW MANIPULATION ******************/


/* Tile/retile a window.                                                */
void
cp_window_tile(Window** wptr, anInstance* owner, aTilingDesc* td)
/* wptr - the ptr to window of interest*/
/* owner - the cp who wants one         */
/* td - the new tiling description   */
{
struct  win_reg *reg;                   /* new window registration      */

    if (
        ((Generic) *wptr == CP_WIN_NOT_RESIZABLE) ||
        ((Generic) *wptr == CP_WIN_RESIZABLE)
    )
    {/* this is a new window */
        *wptr = sm_desk_win_create(
                (aMgrInst*)owner->cp_mgr->manager_id,
                (Generic) td,
                DEF_FONT_ID,
                ((Generic) *wptr == CP_WIN_RESIZABLE)
                    ? DSM_WIN_RESIZE
                    : DSM_WIN_NO_RESIZE
        );
        sm_desk_win_title(
                *wptr,
                owner->cp_mgr->processors[owner->cp_num]->name
        );
        sm_desk_win_show(*wptr);

        /* register the window with the manager */
            reg = (struct win_reg *) get_mem(
                    sizeof(struct win_reg), 
                    "cp_window_tile(): new window registration."
            );
            reg->w    = *wptr;
            reg->inst = owner;
            reg->next = owner->cp_mgr->win_list;
            owner->cp_mgr->win_list = reg;
    }
    else
    {/* the window already exists and this is the correct owner */
        sm_desk_win_modify(*wptr, (Generic) td);
    }
}


/* Tile/retile a window. - but keep it hidden                           */
void
cp_window_tile_hidden(Window** wptr, anInstance* owner, aTilingDesc* td)
/* wptr - the ptr to window of interest*/
/* owner - the cp who wants one         */
/* td - the new tiling description   */
{
struct  win_reg *reg;                   /* new window registration      */

    if (
        ((Generic) *wptr == CP_WIN_NOT_RESIZABLE) ||
        ((Generic) *wptr == CP_WIN_RESIZABLE)
    )
    {/* this is a new window */
        *wptr = sm_desk_win_create(
                (aMgrInst*)owner->cp_mgr->manager_id,
                (Generic) td,
                DEF_FONT_ID,
                ((Generic) *wptr == CP_WIN_RESIZABLE)
                    ? DSM_WIN_RESIZE
                    : DSM_WIN_NO_RESIZE);
        sm_desk_win_title(
                *wptr,
                owner->cp_mgr->processors[owner->cp_num]->name);
        sm_desk_win_hide(*wptr);

        /* register the window with the manager */
            reg = (struct win_reg *) get_mem(
                    sizeof(struct win_reg), 
                    "cp_window_tile(): new window registration."
            );
            reg->w    = *wptr;
            reg->inst = owner;
            reg->next = owner->cp_mgr->win_list;
            owner->cp_mgr->win_list = reg;
    }
    else
    {/* the window already exists and this is the correct owner */
        sm_desk_win_modify(*wptr, (Generic) td);
    }
}


/* Set a new title in the title pane of a cp window.                    */
void
cp_window_set_title(Window* w, char* format, ...)
{
  va_list  arg_list;               /* argument list as per varargs */
  char     buf[1024];              /* the output buffer            */

  va_start(arg_list, format);
    {
      vsprintf(buf, format, arg_list);
      sm_desk_win_title(w, buf);
    }
  va_end(arg_list);
}

/* Put a window on the top of the queue.                                */
void
cp_window_to_top(Window* w)
/* w - the window of interest       */
{
    sm_desk_win_top(w);
}

/* Hide a window                                                        */
void
cp_window_hide(Window* w)
/* w - the window of interest       */
{
    sm_desk_win_hide(w);
}


/* Hide a window                                                        */
void
cp_window_show(Window* w)
/* w - the window of interest       */
{
    sm_desk_win_show(w);
}

/* Destroy a window associated with a command processor.                */
void
cp_window_destroy(Window* w, anInstance* owner)
/* w - the window of interest       */
/* owner - the owner of the window      */
{
struct  win_reg *wr;                    /* current window registration  */
struct  win_reg **wrpp;                 /* window registration ptr ptr  */

    for (wrpp = &owner->cp_mgr->win_list; *wrpp; wrpp = &(*wrpp)->next)
    {/* walk down the window registration list */
        wr = *wrpp;
        if ((wr->inst == owner) && (wr->w == w))
        {/* wr is the unregistered window -- delete it */
            sm_desk_win_destroy(w);
            *wrpp = wr->next;
            free_mem((void*) wr);
            return;
        }
    }
    die_with_message("cp_window_destroy():  bogus window.");
}



/************************ MANAGER ENTRY POINTS **************************/


/* Start the command processor manager.                                 */
static
void
cp_mgr_start(void)
{
    /* if we weren't lazily starting cp's, we would do them all here */
}


/* Finish the command processor manager.                                */
static
void
cp_mgr_finish(void)
{
}


/* Create an instance command processor manager.                        */

extern Generic the_cp_manager_id;    /* skw: sorry, this is needed by CPed.C */
Generic the_cp_manager_id;

static
Generic
cp_mgr_create(Generic manager_id)
/* manager_id - manager id for callbacks     */
{
short           i;                      /* file descriptor index        */
aCpMgr          *m;                     /* manager instance structure   */
int             num_files;              /* the number of allowed files  */

    the_cp_manager_id = manager_id;  /* skw */

    /* create the manager instance structure */
        m = (aCpMgr *) get_mem(
                sizeof(aCpMgr),
                "cp_mgr_create information structure"
        );

    /* initialize the manager instance structure */
        m->message_q      = createMessageQ("cp_manager");
        m->manager_id     = manager_id;
        m->adam_cp        = (anInstance *) get_mem(
                                sizeof(anInstance),
                                "cp_mgr_create root CP instance"
        );
        m->child_list     = (struct cp_reg *) 0;
        m->win_list       = (struct win_reg *) 0;

    /* clear out the file descriptor registration table */
        num_files = getdtablesize();
        m->owner_inst = (anInstance **) get_mem(sizeof(anInstance *) * num_files, "cp_mgr_create(): file owners");
        for (i = 0; i < num_files; i++)
        {/* clear out each potential asynchronous regitration */
            m->owner_inst[i] = NULL_INSTANCE;
        }

    /* initialize the adam cp */
        m->adam_cp->cp_mgr        = m;
        m->adam_cp->parent        = NULL_INSTANCE;
        m->adam_cp->sibling       = NULL_INSTANCE;
        m->adam_cp->child         = NULL_INSTANCE;
        m->adam_cp->cp_num        = UNUSED;
        m->adam_cp->family_count  = 0;  /* Adam is already dead */
        m->adam_cp->message_count = 0;
        m->adam_cp->instance_info = UNUSED;

    return ((Generic) m);
}


/* Return the first node of the tree starting at root that has no alive */
/* ancestors and can be killed.  If no such tree exists, return         */
/* NULL_INSTANCE.                                                       */
static
anInstance *
find_victim(anInstance* root)
/* root - root of the tree to use      */
{
anInstance      *victim;                /* the current child of the tree*/

    if ((root->instance_info != UNUSED) && cp_assert_death(root))
    {/* this instance is the one */
        return (root);
    }
    else if ((root->instance_info == UNUSED) && root->child && (victim = find_victim(root->child  )))
    {/* the victim was found as one of the children */
        return (victim);
    }
    else if (root->sibling && (victim = find_victim(root->sibling)))
    {/* the victim was found as one of the siblings or siblings' children */
        return (victim);
    }
    else
    {/* a victim could not be found */
        return (NULL_INSTANCE);
    }
}


/* Recursively free a family tree.                                      */
static
void
remove_family(anInstance* root)
/* root - the root of the tree of interest */
{
    if (root == NULL_INSTANCE)
        return;
    if (root->sibling)
    {/* there is a sibling */
        remove_family(root->sibling);
        root->sibling = NULL_INSTANCE;
    }
    if (root->child)
    {/* there is a child */
        remove_family(root->child);
        root->child = NULL_INSTANCE;
    }
    free_mem((void*) root);
}


/* Handle an event to a command processor which may cause its death.    */
static
void
give_cp_event(anInstance* cp)
/* cp - the cp instance of the event */
{
struct  cp_reg  *cr;                    /* current child registration   */
struct  cp_reg  **crpp;                 /* child registration ptr ptr   */
struct  win_reg *wr;                    /* current window registration  */
struct  win_reg **wrpp;                 /* window registration ptr ptr  */
int             num_files;              /* the number of allowed files  */
short           i;                      /* fd registration index        */

    if (
        (cp->cp_mgr->processors[cp->cp_num]->handle_input)(
            cp->instance_info,
            mon_event.from,
            mon_event.type,
            mon_event.info,
            mon_event.msg
        )
    )
    {/* kill the cp */
        (cp->cp_mgr->processors[cp->cp_num]->destroy_instance)(
                cp->instance_info,
                false   /* unpanicked */
        );
        cp->instance_info = UNUSED;

        num_files = getdtablesize();
        for (i = 0; i < num_files; i++)
        {/* check to see if all async registrations have been removed */
            if (cp->cp_mgr->owner_inst[i] == cp)
            {/* there is a registration left for this cp */
                message(
                        "Programming error:\n'%s' command processor did not \
unregister file descriptor %d.\nManager has done so for it (file is still \
open?).",
                        cp->cp_mgr->processors[cp->cp_num]->name,
                        i
                );
                cp->cp_mgr->owner_inst[i] = NULL_INSTANCE;
                sm_desk_unregister_fd(i, (aMgrInst*)cp->cp_mgr->manager_id);
            }
        }

        crpp = &cp->cp_mgr->child_list;
        while (*crpp)
        {/* walk down the child registration stack */
            cr = *crpp;
            if (cr->owner == cp)
            {/* cr is the unregistered node--delete it */
                message(
                        "Programming error:\n'%s' command processor did not \
unregister child process %d.\nManager has done so for it (process is still \
running?).",
                        cp->cp_mgr->processors[cp->cp_num]->name,
                        cr->pid
                );
                sm_desk_unregister_process(cr->pid, (aMgrInst*)cp->cp_mgr->manager_id);
                *crpp = cr->next;
                free_mem((void*) cr);
                continue;   /* we have already advanced */
            }
            crpp = &(*crpp)->next;
        }

        wrpp = &cp->cp_mgr->win_list;
        while (*wrpp)
        {/* walk down the window registration list */
            wr = *wrpp;
            if (wr->inst == cp)
            {/* wr is the unregistered window -- delete it */
                message(
                        "Programming error:\n'%s' command processor did not \
destroy window.\nManager has done so for it.",
                        cp->cp_mgr->processors[cp->cp_num]->name
                );
                sm_desk_win_destroy(wr->w);
                *wrpp = wr->next;
                free_mem((void*) wr);
                continue;   /* we have already advanced */
            }
            wrpp = &(*wrpp)->next;
        }

        while (cp)
        {/* propagate death through all ancestors */
            cp->family_count--;
            cp = cp->parent;
        }
    }
}


/* Destroy an instance of the command processor manager.                */
static
void
cp_mgr_destroy(aCpMgr* m)
/* m - manager instance structure   */
{
anInstance      *cp;       /* the current cp of interest   */
short           cp_index;  /* the current processor index  */

    /* remove all command processor instances */
        while (m->adam_cp->family_count)
        {/* there are cp's to kill */
            /* mark the end of the queue */
                addToMessageQ(m->message_q, 0, 0, Origin, 0);

            while (true)
            {/* deliver all current messages and handle the caused deaths */
                getFromMessageQ(m->message_q);
                if ((cp = (anInstance *) mon_event.to) == NULL_INSTANCE)
                {/* this was the end of the queue */
                    break;
                }

                if (mon_event.to != mon_event.from)
                {/* the receiver has one less (non-self) message */
                    cp->message_count--;
                }

                if (cp->instance_info != UNUSED)
                {/* there is an appropriate cp instance */
                    give_cp_event(cp);
                }
                else
                {/* message event to a dead cp (don't deliver it) */
                }
            }

            if (cp = find_victim(m->adam_cp))
            {/* kill the next victim  */
                (cp->cp_mgr->processors[cp->cp_num]->destroy_instance)(
                    cp->instance_info,
                    true    /* panicked */
                );
                cp->instance_info = UNUSED;
                while (cp)
                {/* propagate death through all ancestors */
                    cp->family_count--;
                    cp = cp->parent;
                }
            }
        }
        remove_family(m->adam_cp);

    /* unregister the manager options */
        for (cp_index = 0; cp_index < m->num_processors; cp_index++)
        {/* call finish_cp on every cp that was started */
            if (m->processors[cp_index]->status == CP_NOMINAL)
            {/* this command processor was started and no instances remain */
                (m->processors[cp_index]->finish_cp)();
                m->processors[cp_index]->status = CP_UNSTARTED;
            }
            if (
                    m->processors[cp_index]->root_startup &&
                    (m->processors[cp_index]->status == CP_UNSTARTED)
            )
            {/* this command processor was listed as an option */
                sm_desk_unregister_option((Generic) cp_index, (aMgrInst*)m->manager_id);
            }
        }

    destroyMessageQ(m->message_q);
    free_mem((void*) m->owner_inst);
    free_mem((void*) m);
}


/* Handle an event to one of the instances.                             */
static
void
cp_mgr_event(aCpMgr* m)
/* m - the manager information      */
{
struct  cp_reg  *cr;                    /* current process registration */
struct  win_reg *wr;                    /* current window registration  */
anInstance      *cp;                    /* the instance of interest     */
Generic          cp_id;                 /* temporary storage            */

    if ((mon_event.type == EVENT_SELECT) && (mon_event.from == m->manager_id))
    {/* a selection from the option list */
        cp_id = mon_event.msg;
        (m->processors[cp_id]->root_starter)(
                (int)m->adam_cp,
                cp_id);
        if (m->processors[cp_id]->status == CP_CANNOT_START)
        {/* the cp couldn't be started--remove it from options */
            sm_desk_unregister_option(cp_id, (aMgrInst*)m->manager_id);
        }
    }
    else switch (mon_event.type)
    {/* do any necessary translation */
        case EVENT_NULL:    /* the event we asked for */
            break;

        case EVENT_IO:      /* file event */
            give_cp_event(m->owner_inst[mon_event.from]);
            break;

        case EVENT_SIGCHLD: /* child event */
            for (cr = m->child_list; cr; cr = cr->next)
            {/* search for the appropriate cp instance */
                if (cr->pid == (int) mon_event.from)
                {/* the registration has been found */
                    give_cp_event(cr->owner);
                    break;
                }
            }
            break;

        default:            /* window related events */
            for (wr = m->win_list; wr; wr = wr->next)
            {/* search for the approporiate cp instance */
                if (
                    (wr->w == (Window *) mon_event.from) ||
                    (wr->w == ((Pane *) mon_event.from)->parent)
                )
                {/* the window has been found */
                    if (
                        (mon_event.type == EVENT_SELECT) &&
                        (mon_event.from == (Generic) wr->w)
                    )
                    {/* a window selection */
                    }
                    else
                    {/* a normal event */
                        give_cp_event(wr->inst);
                    }
                    break;
                }
            }
            break;
    }

    if (readyMessageQ(m->message_q))
    {/* there is a message waiting--deliver it */
        getFromMessageQ(m->message_q);
        cp = (anInstance *) mon_event.to;
        if (mon_event.to != mon_event.from)
        {/* the receiver has one less (non-self) message */
            cp->message_count--;
        }
        if (cp->instance_info != UNUSED)
        {/* there is an appropriate cp instance */
            give_cp_event(cp);
        }
        else
        {/* message event to a dead cp (don't deliver it) */
        }
    }

    if (readyMessageQ(m->message_q))
    {/* ask to run again */
        sm_desk_give_event((aMgrInst*)m->manager_id);
    }
}


/* Handle the creation of a new window.  Return desired size.           */
static
Point
cp_mgr_window_tile(aCpMgr* m, Window* w, aTilingDesc* td, Point ulc, Boolean new)
/* m - the manager information      */
/* w - the new window               */
/* td - a tiling description         */
/* ulc - upper left corner of tiling  */
/* new - true if a new window         */
{
Pane            *p;                     /* the current pane of interest */
Pane            *next;                  /* the pane after p on w's list */

    /* delete existing but unmentioned panes */
        for (p = w->child[FRST_NEXT]; p; p = next)
        {/* check existing panes for inclusion in the description */
            next = p->sibling[FRST_NEXT];
            if (NOT(sm_desk_owns_pane(p)) && NOT(pane_in_description(p, td)))
            {/* the pane is not mentioned--delete it*/
                destroyPane(p);
            }
        }

    set_tiling_sizes(td, makePoint(UNUSED, UNUSED));
    return set_panes_and_free(w, td, ulc);
}



/************************** MANAGER CALLBACKS ***************************/


/* Set the manager to use for subsequent menu creation calls.           */
Generic
cp_mgr_use(Generic cpm)
/* cpm - the cp manager               */
{
aCpMgr          *saved;                 /* the saved handle             */

    saved = current_cpm;
    current_cpm = (aCpMgr *) cpm;
    return ((Generic) saved);
}


/* Initialize the command processor manager.                            */
void
cp_mgr_initialize(aCpMgr* m, short num_processors, aProcessor** processors)
/* m - the manager to initialize    */
/* num_processors - the number of processors     */
/* processors - the command processor list   */
{
short           cp_index;               /* the current processor index  */
char            help[100];              /* room for the help description*/

    /* store away the information */
        m->processors     = processors;
        m->num_processors = num_processors;

    /* initialize the option list for each command processor */
        for (cp_index = 0; cp_index < num_processors; cp_index++)
        {/* install the available command processors in the option list */
            if (
                    (processors[cp_index]->root_startup) &&
                    (processors[cp_index]->status == CP_UNSTARTED)
            )
            {/* this command processor should go on the option list */
                sprintf( help,
                        "Start the %s.",
                        processors[cp_index]->name);
                sm_desk_register_option(
                        (Generic) cp_index,
                        processors[cp_index]->name,
                        help,
                        (aMgrInst*)m->manager_id
                );
            }
        }
}


/* Start the first command processor if there is one.                   */
void
cp_mgr_run(aCpMgr* m, short startup)
/* m - the manager to initialize    */
/* startup - the startup command processor*/
{
    if ((0 <= startup) && (startup < m->num_processors))
    {/* start the initial command processor */
        (m->processors[startup]->root_starter)((int)m->adam_cp, startup);
    }
}
