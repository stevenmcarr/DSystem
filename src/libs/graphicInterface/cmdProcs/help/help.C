/* $Id: help.C,v 1.1 1997/06/24 17:56:30 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/****************************************************************/
		/* 			   help.c				*/
		/* This file is the command processor interface of the Help	*/
		/* command processor to the Rn programming environment.		*/
		/* The help command prcessor was written by Donald Baker.	*/
		/* 								*/
		/****************************************************************/

#include <libs/graphicInterface/cmdProcs/help/help.h>
#include <libs/graphicInterface/cmdProcs/help/help_cp.h>
#include <libs/graphicInterface/oldMonitor/include/sms/button_sm.h>
#include <libs/support/strings/rn_string.h>
#include <include/bstring.h>
#include <string.h>


	/* instance list / startup information */

extern	short		help_cp_index;		/* index of the help command processor	*/
extern	char		*D_helpfile;		/* the general Rn helpfile		*/
static 	STATE		*first_instance;	/* the first instance in the list	*/
static 	STATE		*last_instance;		/* the last instance in the list	*/
#define	REFRESH		1			/* refresh the session			*/
#define TERMINATE	2			/* terminate the session		*/


	/* global information */

short			helpcp_font;		/* the font to run everything in	*/


	/* startup structure */

struct	startup		{			/* STARTUP STRUCTURE 			*/
	Generic		owner;			/* owner handle				*/
	char		*file_name;		/* the file name to read		*/
	short		num_args;		/* number of startup arguments		*/
	short		*arg_list;		/* the argument list			*/
	short		position_number;	/* the position number			*/
			};


	/* display information */

extern	helpDisplay	helpcp_hybrid_display;	/* the hybrid display			*/
static	helpDisplay	*display[] = {		/* the display array			*/
			&helpcp_hybrid_display,
			};
static	short		num_displays = 		/* the number of defined displays	*/
			sizeof(display) / sizeof(helpDisplay *);


	/* user button information */

static	short		button_sm;		/* the button screen module index	*/
#define	TRANSFER_BUTTON	0			/* the transfer button			*/
#define	RETURN_BUTTON	1			/* the return button			*/
#define	PREVIOUS_BUTTON	2			/* the previous button			*/
#define	NEXT_BUTTON	3			/* the next button			*/
#define	DISPLAY_BUTTON	4			/* the display button			*/
static	aFaceDef	button_faces[] = {	/* All of the faces of the window	*/
				{  "transfer",	"Transfer to a mentioned heading."             },
				{  "return",	"Return to where the last transfer was made."  },
				{  "previous",	"Move to previous heading."                    },
				{  "next",	"Move to next heading."			       },
				{  "display",	"Change the type of the display."	       },
			};
static	aButtonDef	buttons[] = {		/* All of the buttons of the window	*/
				{  TRANSFER_BUTTON, &button_faces[0], 1 },
				{  RETURN_BUTTON,   &button_faces[1], 1 },
				{  PREVIOUS_BUTTON, &button_faces[2], 1 },
				{  NEXT_BUTTON,     &button_faces[3], 1 },
				{  DISPLAY_BUTTON,  &button_faces[4], 1 },
			};
static	aLayoutDef	button_layout_def = {	/* the button layout for the window	*/
				{ 4, 1 },	/* omit the display button since we only have one display */
				buttons
			};

	/* aProcessor definition */

STATIC(void,		helpcp_root_starter,(Generic creator, short id));
STATIC(void,		helpcp_tile,(STATE *instance, Point size));
STATIC(Boolean,		helpcp_start,(Generic mgr));
STATIC(Generic,		helpcp_create_instance,(Generic parent, Generic cp_id, struct
                                                startup *startup));
STATIC(Boolean,		helpcp_handle_input,(STATE *instance, Generic generator, 
                                             short type, Point coord, Generic msg));
STATIC(void,		helpcp_destroy_instance,(STATE *instance, Boolean panicked));
STATIC(void,		helpcp_finish,(void));
aProcessor		help_processor = {	/* THE HELP_CP PROCESSOR STRUCTURE	*/
			"help",			/* the name of the cp			*/
			true,			/* can be started from the root menu	*/
			0,			/* not used				*/
			helpcp_root_starter,	/* root menu procedure			*/
			helpcp_start,		/* start the cp				*/
			(cp_create_instance_func)
                        helpcp_create_instance,	/* create an instance of the cp		*/
			(cp_handle_input_func)
                        helpcp_handle_input,	/* handle input to cp			*/
			(cp_destroy_instance_func)
                        helpcp_destroy_instance,/* destroy cp instance			*/
			helpcp_finish,		/* finish with cp			*/
			CP_UNSTARTED		/* can be but hasn't been started flag	*/
			};


/* Handle creation from the root menu.							*/
static
void
helpcp_root_starter(Generic creator, short id)
{
	if (id != help_cp_index)
	{/* we cannot fire this cp up. */
		die_with_message("help_cp:  root starter error.");
	}
	help_cp_give_help(creator, D_helpfile, 0, (short *) 0, 0);
}


/* Begin the command processor.								*/
/*ARGSUSED*/
static
Boolean
helpcp_start(Generic mgr)
{
short			i;			/* the current processor number		*/

	first_instance = (STATE *) 0;
	last_instance  = (STATE *) 0;
	button_sm   = sm_button_get_index();
	helpcp_font = DEF_FONT_ID;
	for (i = 0; i < num_displays; i++)
	{/* start each of the displays */
		(display[i]->start)();
	}
	return (true);
}


/* Finish the command processor.							*/
static
void
helpcp_finish()
{
short			i;			/* the current processor number		*/

	for (i = 0; i < num_displays; i++)
	{/* start each of the displays */
		(display[i]->finish)();
	}
}


/* Start up a new instance. 								*/
/*ARGSUSED*/
static
Generic
helpcp_create_instance(Generic parent, Generic cp_id, struct startup *startup)
{
STATE			*instance;		/* the instance structure pointer	*/
char			*error;			/* the tree read error made		*/

	instance = (STATE *) get_mem (sizeof (STATE), "Helpcp: (help.c) state information");
	instance->file_name	  = ssave(startup->file_name);
	instance->num_args        = startup->num_args;
	instance->arg_list        = (short *) get_mem(sizeof(short) * (startup->num_args + 1), "helpcp: saved argument list");
	instance->position_number = startup->position_number;
	instance->root		  = 0;
	instance->current	  = 0;
	instance->last		  = 0;
	instance->display	  = 0;
	instance->cp_id		  = cp_id;
	instance->owner           = startup->owner;
	instance->return_list	  = 0;
	instance->window	  = CP_WIN_RESIZABLE;
	instance->button_pane	  = button_sm;
	instance->button_layout   = NULL_LAYOUT;
	instance->prev_instance   = 0;
	bcopy((char *) startup->arg_list, (char *) instance->arg_list, sizeof(short) * startup->num_args);
	error = helpcp_read_tree(instance);
	if (error)
	{/* there was an error--die */
		sfree(instance->file_name);
		free_mem((void*) instance->arg_list);
		free_mem((void*) instance);
		instance = (STATE *) UNUSED;
		message(error);
	}
	else
	{/* finish the creation */
		if (first_instance)
		{/* there is another instance */
			instance->next_instance = first_instance;
			first_instance->prev_instance = instance;
		}
		else
		{/* this is the only instance */
			instance->next_instance = 0;
			last_instance = instance;
		}
		first_instance = instance;
		(display[instance->display]->initialize)(instance);
		helpcp_tile(instance, Origin);
		(display[instance->display]->new_tree)(instance);
		cp_window_set_title((Window*)instance->window, "Help on the %s", instance->root->heading_text);
		instance->button_layout = sm_button_layout_create((Pane*)instance->button_pane, &button_layout_def, helpcp_font, false);
		sm_button_layout_show((Pane*)instance->button_pane, (Window*)instance->button_layout);
		sm_button_modify_button((Pane*)instance->button_pane, (Window*)instance->button_layout, TRANSFER_BUTTON, false, BOOL(instance->current->transfer_list));
		sm_button_modify_button((Pane*)instance->button_pane, (Window*)instance->button_layout, RETURN_BUTTON,   false, BOOL(instance->return_list));
		sm_button_modify_button((Pane*)instance->button_pane, (Window*)instance->button_layout, PREVIOUS_BUTTON, false, BOOL(instance->current != instance->root));
		sm_button_modify_button((Pane*)instance->button_pane, (Window*)instance->button_layout, NEXT_BUTTON,     false, BOOL(instance->current != instance->last));
		helpcp_select(instance, instance->current);
	}
	return ((Generic) instance);
}


/* Destroy this instance.								*/
/*ARGSUSED*/
static
void
helpcp_destroy_instance(STATE *instance, Boolean panicked)
{
	help_cp_terminate_help(instance->cp_id);
	(display[instance->display]->finalize)(instance);
	helpcp_free_tree(instance);
	cp_window_destroy((Window*)instance->window, (anInstance*)instance->cp_id);
	if (instance->next_instance)
		instance->next_instance->prev_instance = instance->prev_instance;
	else
		last_instance  = instance->prev_instance;
	if (instance->prev_instance)
		instance->prev_instance->next_instance = instance->next_instance;
	else
		first_instance = instance->next_instance;
	sfree(instance->file_name);
	free_mem((void*) instance->arg_list);
	free_mem((void*) instance);
}


/* Handle input to the command processor.						*/
static
Boolean
helpcp_handle_input(STATE *instance, Generic generator, short type, 
                    Point coord, Generic msg)
{
char			*error;			/* the tree read error made		*/
short			i;			/* the display index			*/
char			**labels;		/* the display name list for changing	*/

	if (type == EVENT_KEYBOARD)
	{/* key clicks are ignored */
	}
	else if (type == EVENT_MESSAGE)
	{/* two types of messages: refresh or die */
		if (msg == TERMINATE)
		{/* this is a die message */
			return (true);
		}
		else if (msg == REFRESH)
		{/* this is a refresh message */
			helpcp_free_tree(instance);
			error = helpcp_read_tree(instance);
			if (error)
			{/* there was an error in the tree kill this instance */
				message(error);
				return (true);
			}
			else
			{/* the tree is OK */
				cp_window_set_title((Window*)instance->window, "Help on the %s", instance->root->heading_text);
				(display[instance->display]->new_tree)(instance);
				helpcp_select(instance, instance->current);
				cp_window_to_top((Window*)instance->window);
			}
		}
	}
	else if (generator == instance->window)
	{/* this is a window event */
		if (type == EVENT_RESIZE)
		{/* resize the window */
			helpcp_tile(instance, coord);
			helpcp_select(instance, instance->current);
		}
		else if (type == EVENT_KILL)
		{/* hide the window for later */
			cp_window_hide((Window*)instance->window);
		}
		else if (type == EVENT_HELP)
		{/* offer help on our window */
			help_cp_give_help(instance->cp_id, "help.H", 0, (short *) 0, 0);
		}
	}
	else if (generator == instance->button_pane && type == EVENT_SELECT)
	{/* a button selection */
		switch (msg)
		{/* which button */
			case TRANSFER_BUTTON:
				helpcp_walk_tree(instance, NODE_TRANSFER);
				break;
			case RETURN_BUTTON:
				helpcp_walk_tree(instance, NODE_RETURN);
				break;
			case PREVIOUS_BUTTON:
				helpcp_walk_tree(instance, NODE_PREVIOUS);
				break;
			case NEXT_BUTTON:
				helpcp_walk_tree(instance, NODE_NEXT);
				break;
			case DISPLAY_BUTTON:
				labels = (char **) get_mem(num_displays * sizeof(char *), "helpcp: change display label list");
				for (i = 0; i < num_displays; i++)
				{/* create a menu list */
					labels[i] = display[i]->name;
				}
				i = menu_select("Choose new display type:", num_displays, labels);
				if ((i != UNUSED) && (i != instance->display))
				{/* change displays */
					(display[instance->display]->finalize)(instance);
					instance->display = i;
					(display[instance->display]->initialize)(instance);
					helpcp_tile(instance, Origin);
					(display[instance->display]->new_tree)(instance);
					helpcp_select(instance, instance->current);
				}
				free_mem((void*) labels);
				break;
		}
	}
	else
	{/* send the event to the display */
		(display[instance->display]->event)(instance, generator, type, coord, msg);
	}
	sm_button_modify_button((Pane*)instance->button_pane, (Window*)instance->button_layout, TRANSFER_BUTTON, false, BOOL(instance->current->transfer_list));
	sm_button_modify_button((Pane*)instance->button_pane, (Window*)instance->button_layout, RETURN_BUTTON,   false, BOOL(instance->return_list));
	sm_button_modify_button((Pane*)instance->button_pane, (Window*)instance->button_layout, PREVIOUS_BUTTON, false, BOOL(instance->current != instance->root));
	sm_button_modify_button((Pane*)instance->button_pane, (Window*)instance->button_layout, NEXT_BUTTON,     false, BOOL(instance->current != instance->last));
	return false;	/* do not kill the command processor */
}


/* Tile the window based on the size.							*/
static
void
helpcp_tile(STATE *instance, Point size)
{
Point			button_size;		/* the size of the button pane		*/

	button_size = sm_button_layout_size(&button_layout_def, helpcp_font);
	size.y = max(0, size.y - button_size.y);
	cp_window_tile(
		(Window**)&instance->window,
		(anInstance*)instance->cp_id,
		(aTilingDesc*)cp_td_join(
			TILE_DOWN,
			(aTilingDesc*)(display[instance->display]->tile)(instance, size),
			(aTilingDesc*)cp_td_pane((Pane**)&instance->button_pane, button_size)
		)
	);
}


/* Select a new node.									*/
void
helpcp_select(STATE *instance, NODE *node)
{
	if (node)
	{/* there is a node to go to */
		instance->current = node;
		(display[instance->display]->select)(instance);
	}
}


/* Create/refresh an instance of the helpcp.						*/
void
help_cp_give_help(Generic owner, char *file_name, short num_valid, 
                  short *arg_list, short position_number)
{
STATE			*inst;			/* the current instance in the list	*/
struct	startup		startup;		/* the startup structure		*/

	for (inst = first_instance; inst; inst = inst->next_instance)
	{/* check each current instance for possible refresh */
		if (inst->owner == owner)
		{/* we have found the owner -- refresh it */
			sfree(inst->file_name);
			free_mem((void*) inst->arg_list);
			inst->file_name	      = ssave(file_name);
			inst->num_args        = num_valid;
			inst->arg_list        = (short *) get_mem(sizeof(short) * (num_valid + 1), "helpcp: saved argument list (refresh)");
			inst->position_number = position_number;
			bcopy((char *) arg_list, (char *) inst->arg_list, sizeof(short) * num_valid);
			(void) cp_message((anInstance*)inst->cp_id, (anInstance*)inst->cp_id, Origin, REFRESH);
			return;
		}
	}
	/* the instance was not found -- create a new one */
		startup.owner           = owner;
		startup.file_name       = file_name;
		startup.num_args        = num_valid;
		startup.arg_list        = arg_list;
		startup.position_number = position_number;
		(void) cp_new((anInstance*)cp_root_cp_id(), help_cp_index, (Generic) &startup);
}


/* Destroy the help associated with an instance.					*/
void
help_cp_terminate_help(Generic owner)
{
STATE			*inst;			/* the current instance in the list	*/

	for (inst = first_instance; inst; inst = inst->next_instance)
	{/* check each current instance for possible kill */
		if (inst->owner == owner)
		{/* we have found the owner -- kill it */
			(void) cp_message((anInstance*)inst->cp_id, (anInstance*)inst->cp_id, Origin, TERMINATE);
			break;
		}
	}
}
