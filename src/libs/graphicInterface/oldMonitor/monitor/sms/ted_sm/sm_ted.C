/* $Id: sm_ted.C,v 1.1 1997/06/25 14:59:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
	/********************************/
	/*				*/
	/*		ted.c		*/
	/*	Text editing module	*/
	/*				*/
	/********************************/

#include <stdarg.h>

#include <include/bstring.h>

#include <libs/graphicInterface/oldMonitor/monitor/sms/ted_sm/tedprivate.h>
extern Keymap *verbatim_map;			/* Used in bind.c */

/* a couple of statics used to optimize keyboard handling */
static Generic sm_ted_LAST_PANE_INDEX    = -1;
static int sm_ted_MESSAGE_PANE_CLEAR =  1;

typedef FUNCTION_POINTER(void,KeyMapFunc,(Pane *));

    /* declare the screen module */
STATIC(void,        sm_ted_start,(void));
STATIC(void,	    sm_ted_finish,(void));
STATIC(void,        sm_ted_create,(Pane *p));
STATIC(void,        sm_ted_resize_auto,(Pane *p));
STATIC(void,        sm_ted_destroy,(Pane *p));
STATIC(void,        sm_ted_input,(Pane *p, Rectangle r));

static aScreenModule scr_mod_ted = {
    "ted",
    sm_ted_start,
    sm_ted_finish,
    sm_ted_create,
    sm_ted_resize_auto,
    standardNoSubWindowPropagate,
    sm_ted_destroy,
    sm_ted_input,
    standardTileNoWindow,
    standardDestroyWindow
};

/* forward declarations */
void	sm_ted_register_mod_callback();

/*
 * Start the ted screen module
 */
static void
sm_ted_start()
{
	SM_TED_OPTIM_INDEX  = sm_optim_get_index();
	verbatim_map = keymap_create( (Generic)sm_ted_insert_char );
}

/*
 * Finish the ted screen module
 */
static void
sm_ted_finish()
{
	keymap_destroy( verbatim_map );
}

/*
 * Create a ted instance.
 */
static void
sm_ted_create(Pane *p)
{
    p->pane_information = (Generic) get_mem(sizeof(Ted),"sm_ted_create");
    bzero( (char *)INFO(p),sizeof(Ted));

    sm_ted_buf_create(p);
    sm_ted_win_create_internal(p);	/* Creates an optim_sm slave pane */
    sm_ted_register_mod_callback(p, 0); /* initialize it for REPLACE */
}

/*
 * Resize a ted pane.  For internal use.
 */
void
sm_ted_resize_internal(Pane *p)
{
    /* Redraw the saved map for the area of the text pane we know about */
    resizePane(OP(p), p->position, p->size);

    /* Perform violent size change only if the text pane has actually changed character size.*/
    if ( equalPoint( sm_optim_size(OP(p)), SIZE(p) ) )
	return;

    /* Get a new single screen line that is of the correct width. */
    sm_optim_free_line( OPTIM_LINE(p) );
    SIZE(p) = sm_optim_size(OP(p));
    OPTIM_LINE(p) = sm_optim_alloc_line(OP(p));

    sm_ted_damaged_win(p);
    sm_ted_damaged_dot_row(p);
    sm_ted_damaged_dot_col(p);

    if (VERT(p) != (ScrollBar)UNUSED)
	/* Set the slow,fast scroll amounts */
	sm_scroll_set_step(VERT(p),
		1,
		(int)(PAGE_FRAC(p)*SIZE(p).y));
}

/*
 * Resize a ted pane (for "automatic" calls by the monitor).
 */
static void
sm_ted_resize_auto(Pane *p)
{
    sm_optim_resizing(OP(p),true);
    sm_ted_resize_internal(p);
    sm_ted_repair(p);
    sm_optim_resizing(OP(p),false);

}

/* Destroy the pane and all structures below it.                    */
static void sm_ted_destroy(Pane *p)
{
    sm_ted_win_destroy(p);
    (void) sm_ted_buf_destroy(p);
    free_mem((void*)INFO(p));
}

short sm_ted_get_index()
{
	return (getScreenModuleIndex(&scr_mod_ted));
}

void sm_ted_handle_keyboard(Pane *p, KbChar ch)
{
	int i;

	keymap_enqueue_KbChar( KEY_MAP(p), ch );
	if ( keymap_mapping_complete( KEY_MAP(p) ) )
	{
		KeyMapFunc func = (KeyMapFunc)keymap_info( KEY_MAP(p) );
		i = MULTIPLIER(p);
		MULTIPLIER(p) = 1;

		if ((sm_ted_LAST_PANE_INDEX != (Generic) p) ||
		    (sm_ted_LAST_PANE_INDEX == (Generic) p  && 
		     sm_ted_MESSAGE_PANE_CLEAR == 0))
		{
		  sm_ted_inform (p, "");
		  sm_ted_MESSAGE_PANE_CLEAR = 1;
		  sm_ted_LAST_PANE_INDEX = (Generic) p;
		}

		if ((func == (KeyMapFunc)sm_ted_set_multiplier) && (i != 1))
		{ /* don't multiply the multiplier */
			sm_ted_bitch (p, "Can't multiply the multiplier");
			return;
		}
	
		for (; i > 0; i--)
			func(p);	
		sm_ted_repair(p);
	}
}

/* Handle input to the window manager screen module.                    */

static void
sm_ted_input(Pane *p, Rectangle r)
{
    KeyMapFunc	func;

    while ( pointInRect(mon_event.loc, r) )
    {
	handlePane(OP(p));
        switch(mon_event.type)
        {
	    case EVENT_SELECT:
	    case EVENT_MOVE:
	    case EVENT_HELP:
		if (NOT(ACTIVE(p)))
			return;
		keymap_enqueue_KbChar( EVENT_MAP(p), toKbChar(mon_event.type));
		if (!keymap_mapping_complete( EVENT_MAP(p) ))
			die_with_message("ted_sm: incomplete event mapping?");
	
		func = (KeyMapFunc)keymap_info( EVENT_MAP(p) );
		if (func == (KeyMapFunc)NULL)
			/* It's not our event? */
			return;

		CLICK_AT(p) = mon_event.info;
		func(p);
		sm_ted_repair(p);
		break;

            case MOUSE_MOVE:
            case MOUSE_DRAG:
            case MOUSE_UP:
            case MOUSE_KEYBOARD:
            case MOUSE_EXIT:
            default:
                return;

        }
        getEvent();
    }
}

/* For printing from within dbx */ 
TedBuf * sm_ted_buf(Pane *p)
{
	return BUF(p);
}

void sm_ted_message_handler_initialize (Pane *p, Generic id, 
					sm_ted_message_callback message_handler)
{
	MESG_HANDLER(p) = message_handler;	
	OWNER_ID(p)    = id;
}

Boolean sm_ted_modified (register Pane *p)
{
	if (MODS(p) != 0)
		return true;
	return false;
}

/* For printing from within dbx */ 
TedWin *sm_ted_win(Pane *p)
{
	return WIN(p);
}


#define MESS_SIZE 512

void sm_ted_inform (Pane *p, char* format, ...)
{
	va_list ap;
	char mess[MESS_SIZE];
	TedWin   *save_win;
	UtilNode *wnode;

	va_start(ap, format);
            vsprintf(mess, format, ap);
	va_end(ap);

	save_win = WIN(p);

	wnode = util_head(WIN_LIST(p));
	while (wnode != NULL)
	{
		WIN(p) = (TedWin *)util_node_atom(wnode);
		
		if (MESG_HANDLER(p) != NO_MESG_HANDLER)
			MESG_HANDLER(p)(p, OWNER_ID(p), mess, 0 /* unimportant */);
			
		wnode = util_next(wnode);
	}
	WIN(p) = save_win;

	sm_ted_LAST_PANE_INDEX    = (Generic) p;
	sm_ted_MESSAGE_PANE_CLEAR = 0;
}

void sm_ted_bitch (Pane *p, char *format, ...)
{
	va_list ap;
	char mess[MESS_SIZE];
	TedWin   *save_win;
	UtilNode *wnode;
	
	va_start(ap, format);
          vsprintf(mess, format, ap);
	va_end(ap);

	save_win = WIN(p);

	wnode = util_head(WIN_LIST(p));
	while (wnode != NULL)
	{
		WIN(p) = (TedWin *)util_node_atom(wnode);
		
		if (MESG_HANDLER(p) != NO_MESG_HANDLER)
			MESG_HANDLER(p)(p, OWNER_ID(p), mess, 1 /* important */);
			
		wnode = util_next(wnode);
	}
	WIN(p) = save_win;

	sm_ted_LAST_PANE_INDEX    = (Generic) p;
	sm_ted_MESSAGE_PANE_CLEAR = 0;
}

void sm_ted_register_mod_callback(Pane *p, sm_ted_modify_callback fn)
{
  BUF(p)->mod_function = fn;
}
