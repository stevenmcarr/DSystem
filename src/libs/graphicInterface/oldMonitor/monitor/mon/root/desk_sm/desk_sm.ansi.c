/* $Id: desk_sm.ansi.c,v 1.11 1997/03/11 14:33:51 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*                             desk_sm.c                                */
/*                        desk screen module                            */
/*                                                                      */
/************************************************************************/

#include <string.h>

#include <libs/graphicInterface/oldMonitor/include/mon/sm_def.h>
#include <libs/graphicInterface/oldMonitor/include/mon/manager.h>
#include <libs/graphicInterface/oldMonitor/monitor/mon/root/desk_sm.h>
#include <libs/graphicInterface/oldMonitor/monitor/mon/root/desk_sm/title_sm.h>


    /*** OPTION REGISTRATION ***/

typedef
struct  opt_reg {                       /* OPTION REGISTRATION STACK    */
    anOptionDef         opt;            /* the option structure         */
    struct  amgrinst    *owner;         /* manager instance which owns  */
    Generic             id;             /* the id of the event          */
    struct  opt_reg     *next;          /* next process registration    */
                } anOR;
#define         NULL_OPT_STACK ((anOR *) 0)/* end of the registration   */


    /*** CHILD PROCESS REGISTRATION ***/

typedef
struct  cp_reg  {                       /* PROCESS REGISTRATION STACK   */
    int                 pid;            /* child process id             */
    struct  amgrinst    *owner;         /* manager instance which owns  */
    Generic             id;             /* the id of the event          */
    struct  cp_reg      *next;          /* next process registration    */
                } aCR;
#define         NULL_REG_STACK ((aCR *) 0)/* end of the registration    */


    /*** MANAGER INSTANCE INFORMATION ***/

struct amgrinst{                               /* A MANAGER INSTANCE           */
    Pane                *desk_pane;     /* the owner desk pane          */
    aManager            *mgr;           /* the manager structure        */
    Generic             info;           /* the manager information      */
    Boolean             run_again;      /* true if we should run again  */
    struct  amgrinst    *next;          /* the next manager             */
                };
#define         NULL_MAN_INST ((aMgrInst *) 0)/* null manager instance  */


    /*** PANE INFORMATION ***/

struct  desk_pane_info {                /* PANE INFORMATION STRUCTURE   */
    Generic             reg_id;         /* registration id for this pane*/
    PFV                 run_again;      /* call to make to run us again */
    PFV                 async_reg;      /* asynchronous io registrar    */
    PFV                 async_unreg;    /* asynchronous io unregistrar  */
    aMgrInst            **fd_owners;	/* registered file owners	*/
    PFV                 child_reg;      /* child signal registrar       */
    PFV                 child_unreg;    /* child signal unregistrar     */
    struct  cp_reg      *reg_stack;     /* process registration stack   */
    struct  opt_reg     *opt_stack;     /* the option registration stack*/
    short               num_opts;       /* the number of options        */
    short               pending;        /* the number of pending mgrs   */
    aMenu               *root_menu;     /* the current root menu        */
    aMgrInst            *inst_list;     /* current instance list        */
    Window              *active_win;    /* the currently active window  */
    short               forced;         /* true if active window forced */
                };
#define         REG_ID(p)           \
            ((struct desk_pane_info *) p->pane_information)->reg_id
#define         RUN_AGAIN(p)        \
            ((struct desk_pane_info *) p->pane_information)->run_again
#define         ASYNC_REG(p)        \
            ((struct desk_pane_info *) p->pane_information)->async_reg
#define         ASYNC_UNREG(p)      \
            ((struct desk_pane_info *) p->pane_information)->async_unreg
#define         FD_OWNERS(p)        \
            ((struct desk_pane_info *) p->pane_information)->fd_owners
#define         CHILD_REG(p)        \
            ((struct desk_pane_info *) p->pane_information)->child_reg
#define         CHILD_UNREG(p)      \
            ((struct desk_pane_info *) p->pane_information)->child_unreg
#define         REG_STACK(p)        \
            ((struct desk_pane_info *) p->pane_information)->reg_stack
#define         OPT_STACK(p)        \
            ((struct desk_pane_info *) p->pane_information)->opt_stack
#define         NUM_OPTS(p)         \
            ((struct desk_pane_info *) p->pane_information)->num_opts
#define         PENDING(p)          \
            ((struct desk_pane_info *) p->pane_information)->pending
#define         ROOT_MENU(p)        \
            ((struct desk_pane_info *) p->pane_information)->root_menu
#define         INST_LIST(p)        \
            ((struct desk_pane_info *) p->pane_information)->inst_list
#define         ACTIVE_WIN(p)       \
            ((struct desk_pane_info *) p->pane_information)->active_win
#define         FORCED(p)           \
            ((struct desk_pane_info *) p->pane_information)->forced



    /* WINDOW INFORMATION */

struct  desk_window_info    {           /* WINDOW INFORMATION           */
    aMgrInst            *mgr_inst;      /* the manager instance owner   */
    Generic             info;           /* pane tiling information      */
    Pane                *title_pane;    /* the title pane of the window */
    short               title_font;     /* the font to display the title*/
    Boolean             resizable;      /* if the window can be resized */
                };
#define         MGR_INST(w)         \
            ((struct desk_window_info *) (w)->window_information)->mgr_inst
#define         INFO(w)             \
            ((struct desk_window_info *) (w)->window_information)->info
#define         TITLE_PANE(w)       \
            ((struct desk_window_info *) (w)->window_information)->title_pane
#define         TITLE_FONT(w)       \
            ((struct desk_window_info *) (w)->window_information)->title_font
#define         RESIZABLE(w)        \
            ((struct desk_window_info *) (w)->window_information)->resizable



    /**** LOCAL ROUTINES & VARIABLES ****/

static  short   title_sm;               /* index of the vanilla sm      */
static	Bitmap	background;		/* the background pattern to use*/
STATIC(void,    size_window,(Window *w));/* resize a window              */
STATIC(void,    give_event,(aMgrInst *mi));/* give event to manager inst.  */
STATIC(void,    active_adjust,(Pane *p));/* adjust the active window     */
STATIC(void,    handle_root_menu,(Pane *p, Boolean force));
                                        /* handle the root menu         */



    /**** SCREEN MODULE ENTRY POINTS ****/

STATIC(void,    desk_start,(void));     /* sm start routine             */
STATIC(void,    desk_finish,(void));    /* sm finish routine            */
STATIC(void,    desk_create,(Pane *p)); /* sm create routine            */
STATIC(void,    desk_destroy,(Pane *p));/* sm destroy routine           */
STATIC(void,    desk_input,(Pane *p, Rectangle r));     
                                         /* sm input handler routine     */
STATIC(void,    desk_window_tile,(Window *w, Generic info, Boolean new)); 
                                         /* sm window tile routine       */
STATIC(void,    desk_window_destroy,(Window *w));/* sm window destroy routine    */

static  aScreenModule scr_mod_desk = {  /* declare the screen module    */
                        "desk_sm",
                        desk_start,
                        desk_finish,
                        desk_create,
                        drawWindowsInPane,
                        standardPropagate,
                        desk_destroy,
                        desk_input,
                        desk_window_tile,
                        desk_window_destroy
                };


/* Start the desk screen module.                                        */
static
void desk_start()
{
static BITMAPM_UNIT mosaic_data[] = {   /* a mosaic pattern data        */
                        0xF80F, 0xF80F, 0xF80F, 0xF80F,
                        0x8410, 0x8410, 0x8410, 0x8410,
                        0x8220, 0x8220, 0x8220, 0x8220,
                        0x8140, 0x8140, 0x8140, 0x8140,
                        0x8080, 0x8080, 0x8080, 0x8080,
                        0x4081, 0x4081, 0x4081, 0x4081,
                        0x2082, 0x2082, 0x2082, 0x2082,
                        0x1084, 0x1084, 0x1084, 0x1084,
                        0x0FF8, 0x0FF8, 0x0FF8, 0x0FF8,
                        0x1084, 0x1084, 0x1084, 0x1084,
                        0x2082, 0x2082, 0x2082, 0x2082,
                        0x4081, 0x4081, 0x4081, 0x4081,
                        0x8080, 0x8080, 0x8080, 0x8080,
                        0x8140, 0x8140, 0x8140, 0x8140,
                        0x8220, 0x8220, 0x8220, 0x8220,
                        0x8410, 0x8410, 0x8410, 0x8410,
                        0xF80F, 0xF80F, 0xF80F, 0xF80F,
                        0x8410, 0x8410, 0x8410, 0x8410,
                        0x8220, 0x8220, 0x8220, 0x8220,
                        0x8140, 0x8140, 0x8140, 0x8140,
                        0x8080, 0x8080, 0x8080, 0x8080,
                        0x4081, 0x4081, 0x4081, 0x4081,
                        0x2082, 0x2082, 0x2082, 0x2082,
                        0x1084, 0x1084, 0x1084, 0x1084,
                        0x0FF8, 0x0FF8, 0x0FF8, 0x0FF8,
                        0x1084, 0x1084, 0x1084, 0x1084,
                        0x2082, 0x2082, 0x2082, 0x2082,
                        0x4081, 0x4081, 0x4081, 0x4081,
                        0x8080, 0x8080, 0x8080, 0x8080,
                        0x8140, 0x8140, 0x8140, 0x8140,
                        0x8220, 0x8220, 0x8220, 0x8220,
                        0x8410, 0x8410, 0x8410, 0x8410,
                        0xF80F, 0xF80F, 0xF80F, 0xF80F,
                        0x8410, 0x8410, 0x8410, 0x8410,
                        0x8220, 0x8220, 0x8220, 0x8220,
                        0x8140, 0x8140, 0x8140, 0x8140,
                        0x8080, 0x8080, 0x8080, 0x8080,
                        0x4081, 0x4081, 0x4081, 0x4081,
                        0x2082, 0x2082, 0x2082, 0x2082,
                        0x1084, 0x1084, 0x1084, 0x1084,
                        0x0FF8, 0x0FF8, 0x0FF8, 0x0FF8,
                        0x1084, 0x1084, 0x1084, 0x1084,
                        0x2082, 0x2082, 0x2082, 0x2082,
                        0x4081, 0x4081, 0x4081, 0x4081,
                        0x8080, 0x8080, 0x8080, 0x8080,
                        0x8140, 0x8140, 0x8140, 0x8140,
                        0x8220, 0x8220, 0x8220, 0x8220,
                        0x8410, 0x8410, 0x8410, 0x8410,
                        0xF80F, 0xF80F, 0xF80F, 0xF80F,
                        0x8410, 0x8410, 0x8410, 0x8410,
                        0x8220, 0x8220, 0x8220, 0x8220,
                        0x8140, 0x8140, 0x8140, 0x8140,
                        0x8080, 0x8080, 0x8080, 0x8080,
                        0x4081, 0x4081, 0x4081, 0x4081,
                        0x2082, 0x2082, 0x2082, 0x2082,
                        0x1084, 0x1084, 0x1084, 0x1084,
                        0x0FF8, 0x0FF8, 0x0FF8, 0x0FF8,
                        0x1084, 0x1084, 0x1084, 0x1084,
                        0x2082, 0x2082, 0x2082, 0x2082,
                        0x4081, 0x4081, 0x4081, 0x4081,
                        0x8080, 0x8080, 0x8080, 0x8080,
                        0x8140, 0x8140, 0x8140, 0x8140,
                        0x8220, 0x8220, 0x8220, 0x8220,
                        0x8410, 0x8410, 0x8410, 0x8410,
                };

    title_sm = sm_title_get_index();
    background = makeBitmapFromData(makePoint(64, 64), mosaic_data, "desk_sm.c: desk_start()");
}


/* Finish the desk screen module.                                       */
static
void desk_finish()
{
    freeBitmap(background);
}


/* Create an instance of the desk pane.                                 */
static
void desk_create(Pane* p)
{
int		num_files;		/* the number of files allowed	*/
short           i;                      /* file list index              */

    p->pattern	    = background;
    p->pane_information = (Generic) get_mem(
                    sizeof(struct desk_pane_info),
                    "desk_sm.c: desk_create pane information structure"
    );
    num_files = getdtablesize();
    REG_ID(p)      = 0;
    RUN_AGAIN(p)   = (PFV) 0;
    ASYNC_REG(p)   = (PFV) 0;
    ASYNC_UNREG(p) = (PFV) 0;
    CHILD_REG(p)   = (PFV) 0;
    CHILD_UNREG(p) = (PFV) 0;
    FD_OWNERS(p)   = (aMgrInst **) get_mem(sizeof(aMgrInst *) * num_files, "desk_sm: file list");
    for (i = 0; i < num_files; i++)
    {/* zero out the file descriptor owners */
        FD_OWNERS(p)[i] = NULL_MAN_INST;
    }
    REG_STACK(p)   = NULL_REG_STACK;
    OPT_STACK(p)   = NULL_OPT_STACK;
    NUM_OPTS(p)    = 0;
    PENDING(p)     = 0;
    ROOT_MENU(p)   = NULL_MENU;
    INST_LIST(p)   = NULL_MAN_INST;
    ACTIVE_WIN(p)  = NULL_WINDOW;
    FORCED(p)      = 0;
}


/* Destroy an instance of the desk pane.                                */
static
void desk_destroy(Pane* p)
{
aMgrInst        *inst;                  /* the manager instance         */

    while(INST_LIST(p))
    {/* there is an alive manager on the list */
        /* remove the first manager instance */
            inst = INST_LIST(p);
            INST_LIST(p) = inst->next;

        /* fire down the instance */    
            (inst->mgr->destroy)(inst->info);
            if (--inst->mgr->ref_count == 0)
            {/* finish off the manager */
                (inst->mgr->finish)();
            }

        /* destroy the instance */
            free_mem((void*) inst);
    }
    if (ROOT_MENU(p))
    {/* the root menu exists -- destroy it */
        destroy_menu(ROOT_MENU(p));
        ROOT_MENU(p) = NULL_MENU;
    }
    free_mem((void*) FD_OWNERS(p));
    free_mem((void*) p->pane_information);
}


/* Handle input to the window manager screen module.                    */
static
void desk_input(Pane* p, Rectangle r)
{
aMgrInst        *mi;                    /* the manager to send the event*/
Window          *w;                     /* the current window           */
anEvent         save;                   /* the saved event              */
Pane            *sp;                    /* sub pane in the top window   */
Boolean         processed;              /* true if has been processed   */
Rectangle       new_border;             /* the new border (for moving)  */
struct  cp_reg  *cr;                    /* current child registration   */
Boolean         cont = true;            /* true if we should continue   */
Point           min_size;               /* the minimum resize allowed   */

    while (cont)
    {/* process an input type */
        if (mon_event.type < MOUSE_KEYBOARD)
        {/* mouse event */
            processed = false;
            if (NOT(pointInRect(mon_event.loc, r)))
            {/* this is out of our rectangle--return */
                cont = false;
                processed = true;
            }
            for (w = p->child[FRST_NEXT]; w && NOT(processed); w = w->sibling[FRST_NEXT])
            {/* check each window in turn */
                if (w->showing && pointInRect(mon_event.loc, w->border))
                {/* we have found the window */
#if 1  /*** SKW for Ded ***/
                    if( w != ACTIVE_WIN(p) && mon_event.type == MOUSE_DOWN )
                      sm_desk_win_top(w);
#endif
                    if (w == ACTIVE_WIN(p))
                    {/* handle an input event to the top window */
                        for (sp = w->child[FRST_NEXT]; sp && NOT(processed); sp = sp->sibling[FRST_NEXT])
                        {/* loop over the panes */
                            if (pointInRect(mon_event.loc, makeRectFromSize(transPoint(w->border.ul, sp->position), sp->size)))
                            {/* the event is in pane sp */
                                handlePane(sp);
                                ungetEvent();
                                processed = true;
                            }
                        }
                        if (NOT(processed))
                        {/* the event is in the window but not in a pane  */
			    if ((mon_event.type == MOUSE_DOWN) && (mon_event.info.x == BUTTON_HELP)) {
			    	mon_event.type = EVENT_HELP ;
				mon_event.from = (Generic) w ;
				mi = MGR_INST(w) ;
				give_event(mi) ;
			    }
                            processed = true;
                        }
                    }
                    else
                    {/* the event is in a lower window */
                        if (mon_event.type == MOUSE_DOWN)
                        {/* down click event in a currently inactive window */
                            if (mon_event.info.x == BUTTON_SELECT)
                            {/* selection in a lower window */
                                if (FORCED(p))
                                {/* selection in space */
                                    mon_event.type = EVENT_SELECT;
                                    mon_event.from = (Generic) ACTIVE_WIN(p);
                                    ungetEvent();
                                }
                                else
                                {/* adjust the top window */
                                    sm_desk_win_top(w);
                                }
                            }
                            else if (mon_event.info.x == BUTTON_MOVE)
                            {/* move this window */
                                moveWindow(w);
                                ungetEvent();
                            }
                        }
                        processed = true;
                    }
                }
            }
            if (NOT(processed))
            {/* the event is not in a window */
                if (mon_event.type == MOUSE_DOWN)
                {/* down click event */
                    if (mon_event.info.x == BUTTON_SELECT)
                    {/* selection in space */
			if (ACTIVE_WIN(p))
			{/* there is a top window to select off of */
                            mon_event.type = EVENT_SELECT;
                            mon_event.from = (Generic) ACTIVE_WIN(p);
                            ungetEvent();
                        }
                    }
                    else if (mon_event.info.x == BUTTON_MOVE)
                    {/* center the top window in the pane */
                        if (ACTIVE_WIN(p))
                        {/* there is a top window to center */
                            new_border = centerRectInRect(
                                    ACTIVE_WIN(p)->border,
                                    makeRectFromSize(Origin, p->size)
                            );
                            moveWindowTo(ACTIVE_WIN(p), new_border.ul);
                        }
                    }
                    else if (mon_event.info.x == BUTTON_HELP && !FORCED(p))
                    {/* give the root menu */
                        handle_root_menu(p, false);
                    }
                }
                processed = true;
            }
        }
        else if ((mon_event.type == EVENT_KILL) && (mon_event.from == 0))
        {/* request our death */
            mon_event.type = EVENT_KILL;
            cont = false;
        }
        else
        {/* non-mouse event */
            switch (mon_event.type)
            {/* handle the non-mouse event */
                case MOUSE_EXIT:    /* mouse exits window */
	        case EVENT_NULL:
                    cont = false;
                    break;

                case EVENT_KEYBOARD:/* keyboard events */
		    if (ACTIVE_WIN(p))
                    {/* give to the active window */
                        mi = MGR_INST(ACTIVE_WIN(p));
                        mon_event.from = (Generic) ACTIVE_WIN(p);
                        give_event(mi);
                    }
                    break;

                case EVENT_IO:      /* asynchronous io event */
                    mi = FD_OWNERS(p)[mon_event.from];
                    if (mi)
                    {/* handle a file event to one of our managers */
                        give_event(mi);
                    }
                    else
                    {/* return the event to above (not ours) */
                        cont = false;
                    }
                    break;

                case EVENT_SIGCHLD: /* child process event */
                    mi = (aMgrInst *) 0;
                    for (cr = REG_STACK(p); cr; cr = cr->next)
                    {/* search for the registration for this signal */
                        if (cr->pid == (int) mon_event.from)
                        {/* the registration has been found */
                            mi = cr->owner;
                            break;
                        }
                    }
                    if (mi)
                    {/* we own a manager who wants this--give it */
                        give_event(mi);
                    }
                    else
                    {/* cannot handle this--return this event to above */
                        cont = false;
                    }
                    break;

                case EVENT_SELECT:  /* selection (somewhere) */
                    mi = MGR_INST(ACTIVE_WIN(p));
                    if (mon_event.from == (Generic) TITLE_PANE(ACTIVE_WIN(p)))
                    {/* title pane selection becomes window selection */
                        mon_event.from = (Generic) ACTIVE_WIN(p);
                    }
                    give_event(mi);
                    break;

                default:        /* other subpane events */
                    w = ((Pane *) mon_event.from)->parent;
                    mi = MGR_INST(w);
                    if (mon_event.from == (Generic) TITLE_PANE(w))
                    {/* title pane event */
                        switch (mon_event.type)
                        {/* figure the title pane event */
                            case EVENT_MOVE:    /* move the window */
                                moveWindow(w);
                                ungetEvent();
                                break;
                            case EVENT_HELP:    /* help event */
                            case EVENT_KILL:    /* quit event */
                                mon_event.from = (Generic) w;
                                give_event(mi);
                                break;
                            case EVENT_RESIZE:  /* resize the window */
                                size_window(w);
                                if (mon_event.type == EVENT_RESIZE)
                                {/* resize the window */
                                    /* take off the border */
                                        mon_event.info.y -= 2 * w->border_width;
                                        mon_event.info.x -= 2 * w->border_width;

                                    /* make sure it is big enough */
                                        min_size = sm_title_pane_size("", TITLE_FONT(w));
                                        mon_event.info.x = MAX(mon_event.info.x, min_size.x);
                                        mon_event.info.y = MAX(mon_event.info.y, min_size.y);

                                    /* adjust for the title pane */
                                        mon_event.info.y -= TITLE_PANE(w)->size.y;

                                    mon_event.from = (Generic) w;
                                    give_event(mi);
                                }
                                break;
                        }
                    }
                    else
                    {/* this is an event from a regular child screen module */
                        give_event(mi);
                    }
                    break;
            }
        }

        if (PENDING(p) && !FORCED(p))
        {/* there are pending manager instances */
            do
            {/* run managers again with a null event, if necessary */
                for (mi = INST_LIST(p); mi; mi = mi->next)
                {/* walk down the manager list */
                    if (mi->run_again)
                    {/* give this manager another chance to run */
                        save = mon_event;
                        mon_event.type = EVENT_NULL;
                        mon_event.to   = 0;
                        mon_event.from = 0;
                        mon_event.info = Origin;
                        mon_event.msg  = 0;
                        give_event(mi);
                        mon_event = save;
                    }
                }
            } while (PENDING(p) && cont && NOT(readyEvent()));
        }

        if (cont)
        {/* give the root menu */
#ifdef NO_ROOT_MENU
            if (NOT(ACTIVE_WIN(p)) && !FORCED(p))
            {/* there is no new active window */
                handle_root_menu(p, true);
            }
            getEvent();
#else
            if (NOT(ACTIVE_WIN(p)) && !FORCED(p))
            {/* there is no new active window */
		mon_event.type = EVENT_KILL;
		mon_event.from = 0;
            }
	    else
                getEvent();
#endif
        }
    }
    if (PENDING(p) && !FORCED(p))
    {/* someone wants to run again */
        RUN_AGAIN(p)(REG_ID(p));
    }
}


#define         BORDER      2           /* the border of the window     */
/* Window create entry point.  Handle the creation of a new window.     */
static
void desk_window_tile(Window* w, Generic info, Boolean new)
/* the new window               */
/* the information parameter    */
/* true if a new window         */
{
Point           size;                   /* size of the resultant window */
Point           (*tiler)();             /* the tiling routine           */

    if (new)
    {/* set up the window and title pane */
        w->window_information = info;
        TITLE_PANE(w) = newPane(
                w,
                title_sm,
                makePoint(BORDER, BORDER),
                sm_title_pane_size("", TITLE_FONT(w)),
                1
        );
        sm_title_initialize(TITLE_PANE(w), TITLE_FONT(w), RESIZABLE(w));
    }
    tiler = MGR_INST(w)->mgr->window_tile;
    size = tiler(
            MGR_INST(w)->info,
            w,
            INFO(w),
            makePoint(BORDER, BORDER + TITLE_PANE(w)->size.y),
            new
    );
    TITLE_PANE(w)->size.x = size.x;
    size.x += BORDER * 2;
    size.y += BORDER * 2 + TITLE_PANE(w)->size.y;
    w->border = (new)
        ? bounceRectInRect(
                makeRectFromSize(mon_event.loc, size),
                makeRectFromSize(Origin, w->parent->size)
        )
        : makeRectFromSize(w->border.ul, size);
    w->border_width = BORDER;
}


/* Free local information for a window.                                 */
static
void desk_window_destroy(Window* w)
{
    free_mem((void*) w->window_information);
}



/************************* AUXILIARY ROUTINES ***************************/


/* Handle the root menu--deliver any interesting event.                 */
static
void handle_root_menu(Pane* p, Boolean force)
/* the desk pane                */
/* force a selection            */
{
aMenuDef        md;                     /* the current menu definition  */
aChoiceDef      *choices;               /* the temporary choice list    */
short           this;                   /* the current choice index     */
struct  opt_reg *opt_reg;               /* the current option registr.  */
Generic         selection;              /* the selection off the menu   */
static
#define         CIRCULATE   0           /* circulate the windows        */
anOptionDef     standard[] =    {       /* standard menu options        */
                    { "circulate",  "Circulate the windows." },
                };
#define         NUM_STDS    (sizeof(standard)/sizeof(anOptionDef))

    if (NUM_OPTS(p) + NUM_STDS)
    {/* there is something to do */
        if (ROOT_MENU(p) == NULL_MENU)
        {/* there is currently no root menu -- create it */
            choices = (aChoiceDef *) get_mem(
                    sizeof(aChoiceDef) * (NUM_OPTS(p) + NUM_STDS),
                    "new menu choices"
            );
            this = NUM_OPTS(p) + NUM_STDS - 1;
            for (opt_reg = OPT_STACK(p); opt_reg; opt_reg = opt_reg->next)
            {/* load up the array backwards */
                choices[this].id          = (Generic) opt_reg;
		choices[this].kb_code	  = toKbChar(0);
                choices[this].num_options = 1;
                choices[this].option_list = &opt_reg->opt;
                this--;
            }
            for (this = 0; this < NUM_STDS; this++)
            {/* install the standard options */
                choices[this].id          = this;
		choices[this].kb_code	  = toKbChar(0);
                choices[this].num_options = 1;
                choices[this].option_list = standard + this;
            }
            md.title       = "Root menu";
            md.size        = makePoint(1, NUM_OPTS(p) + NUM_STDS);
            md.choice_list = choices;
            md.def         = UNUSED;
            ROOT_MENU(p) = create_menu(&md);
            free_mem((void*) choices);
        }

        modify_menu_choice(
                ROOT_MENU(p),
                CIRCULATE,
                false,
                BOOL(ACTIVE_WIN(p))
        );
        selection = select_from_menu(ROOT_MENU(p), force);

        switch (selection)
        {
            case UNUSED:        /* no selection was made */
                break;

            case CIRCULATE:     /* circulate the windows */
                toEndWindow(ACTIVE_WIN(p), LAST_PREV);
                active_adjust(p);
                break;

            default:        /* a manager selection */
                opt_reg = (struct opt_reg *) selection;
                mon_event.type = EVENT_SELECT;
                mon_event.from = (Generic) opt_reg->owner;
                mon_event.msg  = opt_reg->id;
                give_event(opt_reg->owner);
                break;
        }
    }
}


/* Resize a window from the resize box.                                 */
static
void size_window(Window* w)
{
Rectangle       ghost_border;           /* ghost outline (absolute)     */
MouseCursor     save_cursor;            /* the saved cursor             */
Point           last_point;             /* the previous coordinate      */
Point           delta;                  /* the amount we moved this time*/
Pane		*root_pane;		/* the root pane		*/

    /* find the root pane */
    for (root_pane = w->parent; root_pane->parent->parent; root_pane = root_pane->parent->parent)
        ;

    /* run the box until you stop dragging */
        ghost_border = w->border;
        save_cursor = CURSOR(invisible_cursor);
        ghostBoxInPane(root_pane, ghost_border, 2, true, true);
        last_point = mon_event.loc;
        do
        {/* run the mouse and the box */
            getEvent();
            delta = subPoint(mon_event.loc, last_point);
	    ghostBoxInPane(root_pane, ghost_border, 2, true, false);
            ghost_border.lr = transPoint(ghost_border.lr, delta);
	    ghostBoxInPane(root_pane, ghost_border, 2, true, true);
            last_point = mon_event.loc;
        } while (mon_event.type == MOUSE_DRAG);
	ghostBoxInPane(root_pane, ghost_border, 2, true, true);
        (void) CURSOR(save_cursor);

    /* make the resize if applicable */
        ungetEvent();
        if ((mon_event.type == MOUSE_UP) || (mon_event.type == MOUSE_EXIT))
        {/* make the move & save the event for the caller */
            mon_event.type = EVENT_RESIZE;
            mon_event.info = sizeRect(ghost_border);
        }
}


/* Give the current event to the mentioned manager instance.            */
static
void give_event(aMgrInst* mi)
{
    if (mi->run_again)
    {/* one less pending manager */
        mi->run_again = false;
        PENDING(mi->desk_pane)--;
    }

    (mi->mgr->event)(mi->info);
}


/* Make any necessary fix to the currently active window.               */
static
void active_adjust(Pane* p)
{
Window          *w;                     /* the new active window        */

    for (w = p->child[FRST_NEXT]; w; w = w->sibling[FRST_NEXT])
    {/* walk down the window list */
        if (w->showing && w->exists)
        {/* this window is the topmost visible one */
            break;
        }
    }

    if (w != ACTIVE_WIN(p))
    {/* change ACTIVE_WIN(p) to be w */
        if (ACTIVE_WIN(p))
        {/* shut off the old active window */
            sm_title_active(TITLE_PANE(ACTIVE_WIN(p)), false);
        }
        ACTIVE_WIN(p) = w;
        if (ACTIVE_WIN(p))
        {/* turn on the new active window */
            sm_title_active(TITLE_PANE(ACTIVE_WIN(p)), true);
        }
    }
}



/**************************** EVENT CALLBACKS ***************************/


/* Register a file descriptor event type.                               */
void sm_desk_register_fd(short fd, aMgrInst* mi, Boolean buffer, Boolean urgent)
/* file descriptor being regsit.*/
/* manager instance of the event*/
/* input should be buffered	*/
/* input should not be held	*/
{
Pane            *p = mi->desk_pane;     /* the owning pane              */

    FD_OWNERS(p)[fd] = mi;
    (ASYNC_REG(p))(fd, REG_ID(p), buffer, urgent);
}


/* Unregister a file descriptor event type.                             */
void sm_desk_unregister_fd(short fd, aMgrInst* mi)
/* file descriptor regsitered   */
/* manager instance of the event*/
{
Pane            *p = mi->desk_pane;     /* the owning pane              */

    FD_OWNERS(p)[fd] = NULL_MAN_INST;
    (ASYNC_UNREG(p))(fd, REG_ID(p));
}


/* Register a child process event type.                                 */
void sm_desk_register_process(int pid, aMgrInst* mi, Boolean urgent)
/* the process id               */
/* manager instance of the event*/
/* input should not be held	*/
{
struct  cp_reg  *current;               /* current child registration   */
Pane            *p = mi->desk_pane;     /* the owning pane              */
    
    current = (struct cp_reg *) get_mem(
            sizeof(struct cp_reg),
            "sm_desk_register_process(): new registration"
    );
    current->pid   = pid;
    current->owner = mi;
    current->next  = REG_STACK(p);
    REG_STACK(p)   = current;
    (CHILD_REG(p))(pid, REG_ID(p), urgent);
}


/* Unregister a child process event type.                               */
void sm_desk_unregister_process(int pid, aMgrInst* mi)
/* the process id               */
/* manager instance of the event*/
{
Pane            *p = mi->desk_pane;     /* the owning pane              */
struct  cp_reg  *current;               /* current child registration   */
struct  cp_reg  **crpp;                 /* child registration ptr ptr   */

    for (crpp = &REG_STACK(p); *crpp; crpp = &(*crpp)->next)
    {/* walk down the child registration stack */
        current = *crpp;
        if (current->pid == pid)
        {/* current is the registration node--delete it */
            *crpp = current->next;
            free_mem((void*) current);
            (CHILD_UNREG(p))(pid, REG_ID(p));
            return;
        }
    }
    die_with_message("sm_desk_unregister_process():  bogus request.");
}


/* Register an option.                                                  */
void sm_desk_register_option(Generic id, char* name, char* help, aMgrInst* mi)
/* id associated with the event */
/* the name of the option       */
/* the help of the option       */
/* manager instance of the event*/
{
struct  opt_reg *current;               /* current option registration  */
Pane            *p = mi->desk_pane;     /* the owning pane              */
    
    current = (struct opt_reg *) get_mem(
            sizeof(struct opt_reg),
            "sm_desk_register_process(): new registration"
    );
    current->opt.displayed_text = strcpy(
            (char *) get_mem(strlen(name) + 1, "name"),
            name
    );
    current->opt.help_text      = strcpy(
            (char *) get_mem(strlen(help) + 1, "help"),
            help
    );
    current->owner = mi;
    current->id    = id;
    current->next  = OPT_STACK(p);
    OPT_STACK(p)   = current;
    NUM_OPTS(p)++;
    if (ROOT_MENU(p))
    {/* we have invalidated the root menu -- destroy it */
        destroy_menu(ROOT_MENU(p));
        ROOT_MENU(p) = NULL_MENU;
    }
}


/* Unregister an option.                                                */
void sm_desk_unregister_option(Generic id, aMgrInst* mi)
/* id associated with the event */
/* manager instance of the event*/
{
Pane            *p = mi->desk_pane;     /* the owning pane              */
struct  opt_reg *current;               /* current option registration  */
struct  opt_reg **orpp;                 /* option registration ptr ptr  */

    for (orpp = &OPT_STACK(p); *orpp; orpp = &(*orpp)->next)
    {/* walk down the option registration stack */
        current = *orpp;
        if (current->id == id)
        {/* current is the registration node--delete it */
            *orpp = current->next;
            free_mem((void*) current->opt.displayed_text);
            free_mem((void*) current->opt.help_text);
            free_mem((void*) current);
            NUM_OPTS(p)--;
            if (ROOT_MENU(p))
            {/* we have invalidated the root menu -- destroy it */
                destroy_menu(ROOT_MENU(p));
                ROOT_MENU(p) = NULL_MENU;
            }
            return;
        }
    }
    die_with_message("sm_desk_unregister_option():  bogus request.");
}


/* (Eventually) send a null event to a manager instance.                */
void sm_desk_give_event(aMgrInst* mi)
{
    if (NOT(mi->run_again))
    {/* one more pending manager */
        mi->run_again = true;
        PENDING(mi->desk_pane)++;
	giveNullEvent();
    }
}



/**************************** WINDOW CALLBACKS **************************/


/* Create a new window from a manager.                                  */
Window* sm_desk_win_create(aMgrInst* mi, Generic info, short title_font, Boolean resizable)
/* the manager of the window    */
/* the information parameter    */
/* the font of the title bar    */
/* the window can be resized    */
{
Window          *w;                     /* the new window               */
struct  desk_window_info *w_info;       /* window information structure */

    w_info = (struct desk_window_info *) get_mem(
                    sizeof(struct desk_window_info), 
                    "desk_sm.c: sm_desk_win_create window information"
    );
    w_info->mgr_inst   = mi;
    w_info->info       = info;
    w_info->title_pane = NULL_PANE;
    w_info->title_font = title_font;
    w_info->resizable  = resizable;
    w = createWindow(mi->desk_pane, (Generic) w_info);
    return (w);
}


/* Modify the tiling of an existing window.                             */
void sm_desk_win_modify(Window* w, Generic info)
/* the window to title          */
/* the information to use       */
{
    INFO(w) = info;
    modifyWindow(w, w->window_information);
}


/* Get the pixel width necessary to display a title.                    */
short sm_desk_title_width(char* title, short font)
{
Point           size;                   /* the total pixel size         */

    size = sm_title_pane_size(title, font);
    return (size.x);
}


/* (Re)title a window.                                                  */
void sm_desk_win_title(Window* w, char* title)
{
    sm_title_display(TITLE_PANE(w), title);
}


/* Destroy a window.                                                    */
void sm_desk_win_destroy(Window* w)
{
    hideWindow(w);
    active_adjust(w->parent);
    destroyWindow(w);
}


/* Make a window visible.                                               */
void sm_desk_win_show(Window* w)
{
    showWindow(w);
    active_adjust(w->parent);
}


/* Remove a window from view.                                           */
void sm_desk_win_hide(Window* w)
{
    hideWindow(w);
    active_adjust(w->parent);
}


/* Bring a window to the top of the window list.                        */
void sm_desk_win_top(Window* w)
{
Pane            *p;                     /* the current ancestor pane    */
Point           ulc;                    /* upper left corner of w       */
Rectangle       r;                      /* the window border            */

    toEndWindow(w, FRST_NEXT);
    if (NOT(w->showing))
    {/* center the window under the cursor */
        ulc = transPoint(mon_event.loc, mon_event.offset);
        for (p = w->parent; p; p = p->parent->parent)
        {/* adjust through pane p and its parent */
            ulc = subPoint(ulc, transPoint(p->position, p->parent->border.ul));
        }
        r = bounceRectInRect(
                centerRectInRect(
                        w->border,
                        makeRectFromSize(ulc, Origin)
                ),
                makeRectFromSize(Origin, w->parent->size)
        );
        moveWindowTo(w, r.ul);
        showWindow(w);
    }
    active_adjust(w->parent);
}


/* Circulate the windows on the desk pane				*/
void sm_desk_circulate(Pane* p)
{
    toEndWindow(ACTIVE_WIN(p), LAST_PREV);
    active_adjust(p);
}

/* return true if circulate is possible */
Boolean sm_desk_can_circulate(Pane* p)
{
    return(BOOL(ACTIVE_WIN(p)));
}

/* return the current active manager for p */
Generic sm_desk_inst_id(Pane* p)
{
    return((Generic)INST_LIST(p));
}


/* tickle the tools cps as needed by root */
void sm_desk_tickle_cp(int id, Generic owner)
{
    mon_event.type = EVENT_SELECT;
    mon_event.from = owner;
    mon_event.msg  = id;
    give_event((aMgrInst *)owner);
}


/* Force a window to be delt with by the user.                          */
void sm_desk_win_force(Window* w, MouseCursor cur)
/* the window to be forced      */
/* the mouse cursor to use      */
{
MouseCursor     saved;                  /* the saved mouse cursor       */

    sm_desk_win_top(w);
    saved = w->parent->cursor;
    w->parent->cursor = cur;
    FORCED(w->parent)++;
    holdNonUrgentEvents(true);
    do
    {/* run until death */
        getEvent();
        handlePane(w->parent);
    } while (mon_event.type != EVENT_KILL);
    holdNonUrgentEvents(false);
    FORCED(w->parent)--;
    w->parent->cursor = saved;
    mon_event.type = MOUSE_MOVE;
}


/* Release a forced window.                                             */
/*ARGSUSED*/
void sm_desk_win_release(Window* w)
{
    mon_event.type = EVENT_KILL;
    mon_event.from = 0;
    ungetEvent();
}


/* Decide if a pane in a window is managed by the desk screen module.   */
Boolean sm_desk_owns_pane(Pane* p)
{
    return (BOOL(TITLE_PANE(p->parent) == p));
}



/************************ SCREEN MODULE CALLBACKS ***********************/


/* Get the index of this screen module.                                 */
short sm_desk_get_index()
{
    return (getScreenModuleIndex(&scr_mod_desk));
}


/* Initalize the desk pane.                                             */
void
sm_desk_initialize(Pane* p, Generic reg_id, PFV run_again, PFV fd_reg, 
                   PFV fd_unreg, PFV cp_reg, PFV cp_unreg)
/* the desk pane                */
/* use in following calls       */
/* call to make to run us again */
/* file descriptor regiterar    */
/* file descriptor unregisterar */
/* child process registrar      */
/* child process unregistrar    */
{
    REG_ID(p)      = reg_id;
    RUN_AGAIN(p)   = run_again;
    ASYNC_REG(p)   = fd_reg;
    ASYNC_UNREG(p) = fd_unreg;
    CHILD_REG(p)   = cp_reg;
    CHILD_UNREG(p) = cp_unreg;
}


/* Begin to use a new manager.                                          */
Generic sm_desk_use_manager(Pane* p, aManager* mgr)
/* the desk pane                */
/* the new manager to use       */
{
aMgrInst        *mi;                    /* the new manager instance     */

    if (mgr->ref_count++ == 0)
    {/* start the manager */
        (mgr->start)();
    }
    mi = (aMgrInst *) get_mem(
            sizeof(aMgrInst),
            "sm_desk_use_manager(%s)",
            mgr->name
    );
    mi->desk_pane = p;
    mi->mgr       = mgr;
    mi->info      = (mgr->create)((Generic) mi);
    mi->run_again = false;
    mi->next      = INST_LIST(p);
    INST_LIST(p)  = mi;
    return (mi->info);
}
