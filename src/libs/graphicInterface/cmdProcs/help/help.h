/* $Id: help.h,v 1.6 1997/06/24 17:56:30 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/****************************************************************/
		/* 			help.h					*/
		/* Global declarations for the HELP command processor.		*/
		/* 								*/
		/****************************************************************/

#include <libs/graphicInterface/oldMonitor/include/mon/cp_def.h>


	/* GENERAL LIST */

typedef struct	list	{			/* GENERAL SINGULARLY-LINKED LIST	*/
	Generic		item;			/* the item on the list			*/
struct	list		*next;			/* the next item on the list		*/
			} LIST;


struct  node;

	/* TEXT LIST entry */

typedef	struct	text_entry {			/* TEXT LIST ENTRY NODE (2-LIST)	*/
struct	node		*enclosing_heading;	/* the parent heading of the list entry	*/
struct	text_entry	*next_text_entry;	/* the next text entry			*/
struct	text_entry	*prev_text_entry;	/* the previous text entry		*/
	char		*formated_text;		/* the text of the formated line	*/
			} TEXT_ENTRY;


	/* NODE of the outline */

typedef	struct	node	{			/* NODE REPRESENTS A HEADING OF OUTLINE	*/
	short		heading_level;		/* number of ancestors			*/
struct	node		*parent_heading;	/* the heading enclosing this one	*/
	char		heading_text[100];	/* title for this node			*/
struct	node		*next_heading;		/* the next heading at this level	*/
struct	node		*prev_heading;		/* the previous heading at this level	*/
	short		number_of_subheadings;	/* the number of subheadings		*/
struct	node		*first_subheading;	/* the first subheading of this one	*/
struct	node		*last_subheading;	/* the last subheading of this one	*/

struct	list		*identification_list;	/* the list of id numbers for this node	*/
struct	list		*transfer_list;		/* the list of transfers from this node	*/

	char		*text_block;		/* text for this node (full block)	*/
	short		text_block_length;	/* the length of the text block (chars)	*/
	short		text_formated_width;	/* the formated width of the text	*/
	short		text_formated_length;	/* the number of formated lines		*/
struct	text_entry	*first_text_entry;	/* the first formated text entry	*/
struct	text_entry	*last_text_entry;	/* the last formated text entry		*/
		} NODE;


typedef struct  state   {                       /* HELP SESSION STATE STRUCTURE
        */
        short           display;                /* the user interface option
        */
        Generic         display_info;           /* display specific information
        */
        Generic         owner;                  /* the owner of the help cp
        */
        Generic         cp_id;                  /* the command processor id
        */
        Generic         window;                 /* the window number
        */
        Generic         button_pane;            /* the top button pane
        */
        Generic         button_layout;          /* the top button layout
        */
struct  node            *root;                  /* root node of the outline
        */
struct  node            *last;                  /* last node of the outline
        */
struct  node            *current;               /* current leaf of the outline
        */
struct  list            *return_list;           /* list of nodes stacked for ret
urns    */
struct  state           *next_instance;         /* the next instance in the list
        */
struct  state           *prev_instance;         /* the previous instance in the
list    */
        char            *file_name;             /* the name of the input file
        */
        short           num_args;               /* the number of arguments
        */
        short           *arg_list;              /* the list of arguments
        */
        short           position_number;        /* the position indicator
        */
                        } STATE;



EXTERN(char,		*helpcp_read_tree,(STATE *instance));	/* read a helptext into a tree in memory*/
/* Takes two parameters (STATE *instance) the instance.  Returns 0 if the tree is good	*/
/* and an error message otherwize.							*/

EXTERN(void,		helpcp_free_tree,(STATE *instance));	/* free the tree and auxilary structs.	*/
/* Takes one parameter (STATE *instance) the owining instance of the tree.		*/

EXTERN(void,		helpcp_walk_tree,(STATE *instance, short val));	
/* make a movement within the tree	*/
/* Takes two parameters (STATE *instance) the instance owning the tree and (short val)	*/
/* the tree walking code below or the son number (>=0).					*/
#define	NODE_PARENT		-1		/* signal to go up one in the outline	*/
#define	NODE_ROOT		-2		/* signal to go to the top heading	*/
#define	NODE_NEXT		-3		/* signal to go to next heading		*/
#define	NODE_PREVIOUS		-4		/* signal to go to the previous heading	*/
#define NODE_TRANSFER		-5		/* signal to go to a transfer heading	*/
#define NODE_RETURN		-6		/* signal to go to a return heading	*/

EXTERN(void,		helpcp_format_text,(NODE *node, short width));	
/* format a text node to a width	*/
/* Takes two parameters (NODE *tn) the tree node and (short width) the with to format	*/
/* the text to.										*/

EXTERN(void,		helpcp_select,(STATE *instance, NODE *node));/* select a new node			*/
/* Takes two parameters (STATE *instance) the helpcp instance variable and (NODE *node)	*/
/* the new node to display.								*/


	/* INSTANCE information */


	/* DISPLAY TYPE	*/

typedef FUNCTION_POINTER (void, helpDisplayStart, (void));
typedef FUNCTION_POINTER (void, helpDisplayFinish, (void));
typedef FUNCTION_POINTER (void, helpDisplayInitialize, (STATE*));
typedef FUNCTION_POINTER (void, helpDisplayFinalize, (STATE*));
typedef FUNCTION_POINTER (void, helpDisplayNewTree, (STATE*));
typedef FUNCTION_POINTER (Generic, helpDisplayTile, (STATE*, Point));
typedef FUNCTION_POINTER (void, helpDisplaySelect, (STATE*));
typedef FUNCTION_POINTER (void, helpDisplayEvent, (STATE*, Generic, short, 
                                                   Point, Generic));

typedef	struct		{			/* A DISPLAY ROUTINE STRUCTURE		*/
char			*name;			/* the name of the display		*/

helpDisplayStart       start;		/* start the display			*/
/* Takes no parameters.  This routine is called only once to set up the display.	*/

helpDisplayFinish      finish;		/* finish the display			*/
/* Takes no parameters.  This routine is called only once to remove the display.	*/

helpDisplayInitialize  initialize;	/* initialize a new display instance	*/
/* Takes one parameter (STATE *instance) the help cp instance structure.  The routine	*/
/* should set up for a new display instance and tile the window.			*/

helpDisplayFinalize    finalize;		/* finalize a display instance		*/
/* Takes one parameter (STATE *instance) the help cp instance structure.  The routine	*/
/* should fire down a display instance.							*/

helpDisplayNewTree     new_tree;		/* set up for a new tree		*/
/* Takes one parameter (STATE *instance) the help cp instance structure.  The routine	*/
/* sets up for a new tree.								*/

helpDisplayTile        tile;		/* tile a display based on a size	*/
/* Takes two parameters (STATE *instance) the help cp instance structure and		*/
/* (Point size) the suggested size for the tile.  The tiling descriptor for the display	*/
/* should be returned.									*/

helpDisplaySelect      select;		/* select a new node of interest	*/
/* Takes one parameter (STATE *instance) the help cp instance structure.  The display	*/
/* should be redrawn for the current heading.						*/

helpDisplayEvent       event;		/* handle an input event to the display	*/
/* Takes five parameters (STATE *instance) the help cp instance structure,		*/
/* (Generic generator) the genenerator of the event, (short type) the event type,	*/
/* (Point coord) the event coordinate, and (Generic msg) the event message.		*/
			} helpDisplay;

extern	short		helpcp_font;		/* the font to use for everything	*/
