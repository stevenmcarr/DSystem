/* $Id: message.C,v 1.1 1997/06/25 14:58:15 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 		        message.c			*/
		/*	           Message screen module		*/
		/* 							*/
		/********************************************************/

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <libs/graphicInterface/oldMonitor/include/mon/sm_def.h>
#include <libs/graphicInterface/oldMonitor/include/sms/message_sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/list_sm.h>
#include <libs/support/strings/rn_string.h>


	/* LOCAL INFORMATION */

static	short		list_sm;		/* the list screen module index number	*/

struct	mes_pane_info	{			/* MESSAGE PANE INFORMATION STRUCTURE	*/
	Pane 		*list_pane;		/* the list pane index			*/
	char		*current;		/* the current message text		*/
			};
#define	LIST_PANE(p)	((struct mes_pane_info *) p->pane_information)->list_pane
#define	CURRENT(p)	((struct mes_pane_info *) p->pane_information)->current


	/* SCREEN MODULE DECLARATION */

STATIC(void,		mes_start,(void));
STATIC(void,		mes_create,(Pane *p));
STATIC(void,		mes_resize,(Pane *p));
STATIC(void,		mes_destroy,(Pane *p));
STATIC(void,		mes_input,(Pane *p, Rectangle r));
STATIC(lsm_line_info*,  message_query_handler,(Pane *list_pane, Pane *p, Boolean first,
                                               Generic line_id));

static	aScreenModule	scr_mod_message =
			{
			"message",
			mes_start,
			standardFinish,
			mes_create,
			mes_resize,
			standardNoSubWindowPropagate,
			mes_destroy,
			mes_input,
			standardTileNoWindow,
			standardDestroyWindow
			};


/* Start this screen module.								*/
static
void
mes_start()
{
	list_sm = sm_list_get_index();
}


/* Set the default status for this pane.						*/
static
void
mes_create(Pane *p)
{
	p->pane_information = (Generic) get_mem ( sizeof(struct mes_pane_info), "message_sm: create pane information structure");
	LIST_PANE(p)	= newSlavePane(p, list_sm, p->position, p->size, p->border_width);
	CURRENT(p)      = (char *) get_mem(1, "message_sm: initial message string");
	*CURRENT(p)     = '\0';
	sm_list_initialize(
		LIST_PANE(p),
		(Generic) p,
		(lsm_generator_callback)message_query_handler, 
		DEF_FONT_ID,
		LSM_SHIFT_AUTO,
		LSM_NO_H_SCROLLBAR,
		LSM_NO_V_SCROLLBAR
	);
	p->border_width = 0;
}


/* Resize/reposition the message pane.							*/
static
void
mes_resize(Pane *p)
{
	resizePane(LIST_PANE(p), p->position, p->size);
}


/* Destroy the pane and all structures below it.					*/
static
void
mes_destroy(Pane *p)
{
	free_mem((void*) CURRENT(p));
	destroyPane(LIST_PANE(p));
	free_mem((void*) p->pane_information);
}


/* Handle input to the message screen module.						*/
/*ARGSUSED*/
static
void
mes_input(Pane *p, Rectangle r)
{
	handlePane(LIST_PANE(p));
}


/* Handle a message pane query about what lines to display.				*/
/*ARGSUSED*/
static
lsm_line_info *
message_query_handler(Pane *list_pane, Pane *p, Boolean first, Generic line_id)
{
lsm_line_info		*line = (lsm_line_info *) 0;/* the return line information	*/

	if (first)
	{/* only the first line is defined */
		line = (lsm_line_info *) get_mem(sizeof(lsm_line_info), "message_sm: line request");
		line->text        = CURRENT(p);
		line->should_free = false;
		line->len         = UNUSED;
		line->id          = 0;
		line->selected    = false;
		line->selectable  = true;
	}
	return (line);
}


	/* USER CALLBACKS */

/* Get the index of this screen module.  Install it if necessary.			*/
short
sm_message_get_index()
{
	return (getScreenModuleIndex(&scr_mod_message));
}


/* Return the pane size necessary to display all of a piece of text.			*/
Point
sm_message_pane_size(short width, short font_id)
{
	return (sm_list_pane_size(makePoint(width, 1), font_id, LSM_NO_H_SCROLLBAR, LSM_NO_V_SCROLLBAR));
}


/* Set the text to be displayed.							*/
void
sm_message_change_font(Pane *p, short font_id)
{
	sm_list_initialize(
		LIST_PANE(p),
		(Generic) p,
		(lsm_generator_callback)message_query_handler, 
		font_id,
		LSM_SHIFT_AUTO,
		LSM_NO_H_SCROLLBAR,
		LSM_NO_V_SCROLLBAR
	);
	sm_list_modified(LIST_PANE(p), Origin, 1);
}



/* Set a new message string.								*/
/*VARARGS*/
void
sm_message_new(Pane *p, char *format, ...)
{
va_list			arg_list;		/* the argument list as per varargs	*/
static	char		msg_buffer[1024];       /* scratch buffer for mesg. const.	*/

	va_start(arg_list, format);
        vsprintf (msg_buffer, format, arg_list);
	va_end(arg_list);

	if ( ( msg_buffer[0] == '\0' ) && ((CURRENT(p)[0]) == '\0') ) {
	    return ; /* improves performance by skipping redundant clears */
	}
	free_mem((void*) CURRENT(p));
	CURRENT(p) = (char *) get_mem(strlen(msg_buffer) + 1, "sm_message_new(): new message string");
	(void) strcpy(CURRENT(p), msg_buffer);
	sm_list_modified(LIST_PANE(p), Origin, 1);
}







