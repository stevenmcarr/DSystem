/* $Id: hybrid_disp.C,v 1.1 1997/06/24 17:56:30 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/****************************************************************/
		/* 		      hybrid_display.c				*/
		/*	   This file implements the hybrid display.		*/
		/* 								*/
		/****************************************************************/

#include <string.h>
#include <libs/graphicInterface/cmdProcs/help/help.h>
#include <libs/graphicInterface/oldMonitor/include/sms/list_sm.h>

STATIC(void,		hybrid_start,(void));   /* start the display */
STATIC(void,		hybrid_finish,(void));	/* finish the display */
STATIC(void,		hybrid_initialize,(STATE *instance)); /* instance initializer */
STATIC(void,		hybrid_finalize,(STATE *instance)); /* instance finalizer */
STATIC(void,		hybrid_new_tree,(STATE *instance)); /* handle a new tree */
STATIC(Generic,		hybrid_tile,(STATE *instance, Point size)); /* display tiler */
STATIC(void,		hybrid_select,(STATE *instance)); /* selector */
STATIC(void,		hybrid_event,(STATE *instance, Generic generator, short type,
                                      Point coord, Generic msg)); /* event handler */
STATIC(lsm_line_info,	*hybrid_handler,(Generic pane, STATE *instance, Boolean first,
                                         Generic id)); 
                         /* the list screen module callback */

helpDisplay		helpcp_hybrid_display={	/* THE HYBRID DISPLAY */
			"hybrid display",
			(helpDisplayStart)hybrid_start,
			(helpDisplayFinish)hybrid_finish,
			(helpDisplayInitialize)hybrid_initialize,
			(helpDisplayFinalize)hybrid_finalize,
			(helpDisplayNewTree)hybrid_new_tree,
			(helpDisplayTile)hybrid_tile,
			(helpDisplaySelect)hybrid_select,
			(helpDisplayEvent)hybrid_event
			};

#define	TAB_SIZE	2			/* the size of the tab (outline) */
static	short		list_sm;		/* the list screen module index	*/

struct	hybrid_info	{			/* HYBRID DISPLAY STATE STRUCTURE */
	Generic		outline_pane;		/* pane where outline is displayed */
	Generic		text_pane;		/* pane where text is dispalayed */
	Generic		selected_line;		/* the currently selected line */
	Boolean		list_inited;		/* true if the list panes are inited */
			};
#define OUTLINE_PANE	((struct hybrid_info *) instance->display_info)->outline_pane
#define TEXT_PANE	((struct hybrid_info *) instance->display_info)->text_pane
#define SELECTED_LINE	((struct hybrid_info *) instance->display_info)->selected_line
#define LIST_INITED	((struct hybrid_info *) instance->display_info)->list_inited


/* Set up for the hybrid display. */
static
void
hybrid_start(void)
{
	list_sm = sm_list_get_index();
}


/* Fire down the hybrid display. */
static
void
hybrid_finish(void)
{
}


/* Initialize the hybrid display. */
static
void
hybrid_initialize(STATE *instance)
{
	instance->display_info = (Generic) get_mem(sizeof(struct hybrid_info), "helpcp, hybrid display instance variable");
	OUTLINE_PANE  = list_sm;
	TEXT_PANE     = list_sm;
	LIST_INITED   = false;
	SELECTED_LINE = 0;	/* no line is selected */
}


/* Free display instance storage. */
static
void
hybrid_finalize(STATE *instance)
{
	free_mem((void*) instance->display_info);
	instance->display_info = 0;
}


/* Handle a new tree. */
static
void
hybrid_new_tree(STATE *instance)
{
	if (!LIST_INITED)
	{/* initialize the list panes (just once) */
		sm_list_initialize(
			(Pane*)OUTLINE_PANE,
			(Generic) instance,
			(lsm_generator_callback)hybrid_handler,
			helpcp_font,
			LSM_SHIFT_AUTO,
			LSM_H_SCROLLBAR,
			LSM_V_SCROLLBAR
		);
		sm_list_initialize(
			(Pane*)TEXT_PANE,
			(Generic) instance,
			(lsm_generator_callback)hybrid_handler,
			helpcp_font,
			LSM_SHIFT_AUTO,
			LSM_NO_H_SCROLLBAR,
			LSM_V_SCROLLBAR
		);
		LIST_INITED = true;
	}

	/* Clear out the old information */
		sm_list_modified((Pane*)OUTLINE_PANE, Origin, UNUSED);
		sm_list_modified((Pane*)TEXT_PANE,    Origin, 0);

	SELECTED_LINE = 0;
}


/* Create a tiling descriptor for the display. */
static
Generic
hybrid_tile(STATE *instance, Point size)
{
Point			outline_size;		/* the size of the outline pane	*/
Point			text_size;		/* the size of the text pane */

	outline_size   = sm_list_pane_size(makePoint(30, 10), helpcp_font, LSM_H_SCROLLBAR,    LSM_V_SCROLLBAR);
	text_size      = sm_list_pane_size(makePoint(40, 10), helpcp_font, LSM_NO_H_SCROLLBAR, LSM_V_SCROLLBAR);
	outline_size.x = MAX(size.x / 2, outline_size.x);
	text_size.x    = MAX(size.x / 2, text_size.x);
	text_size.y    = MAX(text_size.y, size.y);
	return (
		cp_td_join(
			TILE_RIGHT,
			(aTilingDesc*)cp_td_pane((Pane**)&TEXT_PANE, text_size),
			(aTilingDesc*)cp_td_pane((Pane**)&OUTLINE_PANE, outline_size)
		)
	);
}


/* make a new category choice */
static
void
hybrid_select(STATE *instance)
{
Point			size;			/* the size of the list pane */

	if (SELECTED_LINE)
	{/* deselect the old line */
		sm_list_line_change((Pane*)OUTLINE_PANE, SELECTED_LINE, false, true);
		SELECTED_LINE = (Generic) instance->current;
		sm_list_line_show((Pane*)OUTLINE_PANE, SELECTED_LINE, false, UNUSED);
		sm_list_line_change((Pane*)OUTLINE_PANE, SELECTED_LINE, true, true);
	}
	else
	{/* we have a new list */
		SELECTED_LINE = (Generic) instance->current;
		sm_list_line_show((Pane*)OUTLINE_PANE, SELECTED_LINE, true, UNUSED);
	}
	size = sm_list_map_size((Pane*)TEXT_PANE);
	helpcp_format_text(instance->current, size.x);
	(void) sm_list_modified((Pane*)TEXT_PANE, Origin, MAX(instance->current->text_formated_length, 1));
}


/* Handle an event to the display. */
/*ARGSUSED*/
static
void
hybrid_event(STATE *instance, Generic generator, short type, Point coord, Generic msg)
{
	if (generator == OUTLINE_PANE)
	{/* an outline event */
		if (type == EVENT_SELECT)
		{/* move up in the outline */
			helpcp_select(instance, (NODE *) msg);
		}
		else if (type == EVENT_HELP)
		{/* give help */
			message("Select on the heading of interest.\nOutline may be scrolled.");
		}
	}
	else if (generator == TEXT_PANE)
	{/* text event */
		if (type == EVENT_HELP)
		{/* give help */
			message("Text may be scrolled.");
		}
	}
}


/* List screen module callback handler.	*/
static
lsm_line_info *
hybrid_handler(Generic pane, STATE *instance, Boolean first, Generic id)

{
lsm_line_info		*line = 0;		/* the returned line information */
struct	text_entry	*te;			/* the current requested text entry */
NODE			*node;			/* the currrent requested node */
char			*text;			/* the current text being moved	*/
short			indent;			/* the number of characters to indent */

	if (pane == TEXT_PANE)
	{/* the request is for the text pane */
		te = (first)
			? instance->current->first_text_entry
			: ((id) ? ((struct text_entry *) id)->next_text_entry : 0);
		if (te)
		{/* there is a line for this request */
			line = (lsm_line_info *) get_mem(sizeof(lsm_line_info), "helpcp: hybred_disp: line request");
			line->text        = te->formated_text;
			line->should_free = false;
			line->len         = UNUSED;
			line->id          = (Generic) te;
			line->selected    = false;
			line->selectable  = true;
		}
		else if (first)
		{/* there is no text and this is the first line */
			line = (lsm_line_info *) get_mem(sizeof(lsm_line_info), "helpcp: hybred_disp: line request");
			line->text        = "---NO TEXT---";
			line->should_free = false;
			line->len         = UNUSED;
			line->id          = 0;
			line->selected    = false;
			line->selectable  = false;
		}
		
	}
	else if (pane == OUTLINE_PANE)
	{/* the request is for the outline pane */
		if (first)
		{/* give the first line of the list (root) */
			node = instance->root;
		}
		else 
		{/* give the next entry in the list */
			node = (NODE *) id;
			if (node->first_subheading)
			{/* the first child is next */
				node = node->first_subheading;
			}
			else
			{/* move up until you can move over--that brother is next */
				while ((!node->next_heading) && (node->parent_heading))
					node = node->parent_heading;
				node = node->next_heading;
			}
		}
		if (node)
		{/* make up a lsm_line_info for this line */
			line = (lsm_line_info *) get_mem(sizeof(lsm_line_info), "helpcp: hybred_disp: line request");
			line->len         = strlen(node->heading_text) + node->heading_level * TAB_SIZE;
			line->text        = (char *) get_mem(line->len + 1, "helpcp: hybred_disp text line");
			line->should_free = true;
			line->id          = (Generic) node;
			line->selected    = BOOL((Generic) node == SELECTED_LINE);
			line->selectable  = true;
			/* indent the heading */
				text = line->text;
				for (indent = node->heading_level * TAB_SIZE; indent; indent--)
					*text++ = ' ';
			(void) strcpy(text, node->heading_text);
		}
	}
	return (line);
}



