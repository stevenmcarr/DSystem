/* $Id: table_sm.C,v 1.1 1997/06/25 14:44:32 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 		      table_sm.c			*/
		/*	      Table display screen module.		*/
		/* 							*/
		/********************************************************/

#include <string.h>

#include <libs/support/strings/rn_string.h>
#include <libs/graphicInterface/oldMonitor/include/mon/sm_def.h>
#include <libs/graphicInterface/oldMonitor/include/sms/optim_sm.h>

#include <libs/graphicInterface/oldMonitor/dialogs/filer/table_sm.h>

	/* declare the screen module */
STATIC(void, table_start, (void));
STATIC(void, table_create, (Pane *p));
STATIC(void, table_resize, (Pane *p));
STATIC(void, table_destroy, (Pane *p));
STATIC(void, table_input, (Pane *p, Rectangle r));
STATIC(void, table_adjust, (Pane *p));
STATIC(void, table_paint, (Pane *p));

static	aScreenModule	scr_mod_table =
			{
			"table",
			table_start,
			standardFinish,
			table_create,
			table_resize,
			standardNoSubWindowPropagate,
			table_destroy,
			table_input,
			standardTileNoWindow,
			standardDestroyWindow
			};

	/* local datatypes */
struct	table_pane_info	{			/* LOCAL TABLE PANE INFORMATION		*/
	Pane		*op;			/* the optim slave pane			*/
	Point		disp_size;		/* the display size in chars		*/
	short		nstrings;
	char		**strings;
	short		selected;
	short		font_id;

	short		shift;			
	short		max_shift;
        short 	  	real_width;
	short		colwidth;
			};

#define	OP(p)	      	((struct table_pane_info *) p->pane_information)->op
#define	SHIFT(p)	((struct table_pane_info *) p->pane_information)->shift
#define	MAX_SHIFT(p)	((struct table_pane_info *) p->pane_information)->max_shift
#define	SELECTED(p)	((struct table_pane_info *) p->pane_information)->selected
#define COLWIDTH(p)	((struct table_pane_info *) p->pane_information)->colwidth
#define	DISP_SIZE(p)	((struct table_pane_info *) p->pane_information)->disp_size
#define	REAL_WIDTH(p)	((struct table_pane_info *) p->pane_information)->real_width
#define	STRINGS(p)	((struct table_pane_info *) p->pane_information)->strings
#define	NSTRINGS(p)	((struct table_pane_info *) p->pane_information)->nstrings
#define	FONT_ID(p)	((struct table_pane_info *) p->pane_information)->font_id

static short    optim_sm;


/* SCREEN MODULE DEFINITION ROUTINES */

/* Start the list screen module.							*/
static void table_start ()
{
    optim_sm = sm_optim_get_index ();
}

/* Create a table pane.									*/
static void table_create (Pane *p)
{
    p->pane_information = (Generic) get_mem (sizeof (struct table_pane_info), "table_sm/table.c: pane information structure");
    OP (p) = newSlavePane (p, optim_sm, p->position, p->size, p->border_width);
    SHIFT (p) = 0;
    DISP_SIZE (p) = Origin;
    STRINGS (p) = 0;
    NSTRINGS (p) = 0;
    SELECTED (p) = -1;
    sm_optim_set_move_status (OP (p), true, false);
    p->border_width = 0;
}


/* Resize/reposition a table pane. */
static void table_resize (Pane *p)
{
    resizePane (OP (p), p->position, p->size);
    DISP_SIZE (p) = sm_optim_size (OP (p));
    SHIFT (p) = 0;

    sm_optim_resizing (OP (p), true);
    table_adjust (p);
    table_paint (p);
    sm_optim_resizing (OP (p), false);
}

/* Destroy the pane and all structures below it.	*/
static void table_destroy (Pane *p)
{
    destroyPane (OP (p));
    free_mem ((void*) p->pane_information);
}

/* Handle input to the table screen module.						*/
static void table_input (Pane *p, Rectangle r)
{
    while ((mon_event.type < MOUSE_KEYBOARD) && pointInRect (mon_event.loc, r))
    {				/* we can handle this event */
	handlePane (OP (p));
	switch (mon_event.type)
	{			/* handle the returned event */
	    case EVENT_SELECT:
		/*
		 * return the event in info.x 
		 *
		 */
		mon_event.info.x = table_map (p, mon_event.info);
		SELECTED (p) = mon_event.info.x;
		table_paint (p);
		break;
	    case EVENT_HELP:
		break;
	}
    }
}

void sm_table_initialize (Pane *p, short font_id)
{
    FONT_ID (p) = font_id;
}

void sm_table_new_table (Pane *p, short nstrings, char **strings, short selected)
{
    int             i;

    /* free the old strings */
    if (STRINGS (p) != 0)
    {
	for (i = 0; i < NSTRINGS (p); i++)
	    sfree (STRINGS (p)[i]);
	free_mem ((void*) STRINGS (p));
    }

    /* stash away the new ones */
    STRINGS (p) = (char **) get_mem (nstrings * sizeof (char *), "table entries");
    for (i = 0; i < nstrings; i++)
	STRINGS (p)[i] = ssave (strings[i]);

    NSTRINGS (p) = nstrings;
    SELECTED (p) = selected;
    SHIFT (p)    = 0;

    table_adjust (p);
    table_paint (p);
}

/* don't change the shift */
void sm_table_update_table (Pane *p, short nstrings, char **strings, short selected)
{
    int             i;

    /* free the old strings */
    if (STRINGS (p) != 0)
    {
	for (i = 0; i < NSTRINGS (p); i++)
	    sfree (STRINGS (p)[i]);
	free_mem ((void*) STRINGS (p));
    }

    /* stash away the new ones */
    STRINGS (p) = (char **) get_mem (nstrings * sizeof (char *), "table entries");
    for (i = 0; i < nstrings; i++)
	STRINGS (p)[i] = ssave (strings[i]);

    NSTRINGS (p) = nstrings;
    SELECTED (p) = selected;

    table_adjust (p);
    table_paint (p);
}


/* USER CALLBACKS */

/* Get the index of this screen module.	*/
short sm_table_get_index ()
{
    return (getScreenModuleIndex (&scr_mod_table));
}


/* Calculate the minimum sized pane to display a character block. */
Point sm_table_pane_size (Point size, short font)
{
    return (sm_optim_pane_size (size, font));
}


/* INTERNAL ROUTINES */
void oldtable_adjust (Pane *p)	/* maintained to show how the old one was */
{
    int             i;
    int             numcols;
    int             sum;
    short	    cmax;

    if (NSTRINGS (p) == 0 || DISP_SIZE (p).y == 0)
	return;

    numcols = 1 + (NSTRINGS (p) - 1) / DISP_SIZE (p).y;

    sum = 0;
    for (i = 0; i < NSTRINGS (p); i++)
	sum += strlen (STRINGS (p)[i]);

    /* use a heuristic to determine the width of a column, strings will
       be truncated at COLWIDTH - 1 chars if necessary so that we can
       insert a space after the string */
    COLWIDTH (p) = /* space */ 1 + (2 * sum) / NSTRINGS (p);

    cmax = 0;
    for (i = DISP_SIZE(p).y * (numcols - 1); i < NSTRINGS (p); i++)
         cmax = MAX(cmax,(int)strlen(STRINGS(p)[i]));
    cmax = MIN(cmax,COLWIDTH(p) - 1);
    
    REAL_WIDTH(p) = (numcols - 1) * COLWIDTH(p) + cmax;

    if (REAL_WIDTH(p) > DISP_SIZE(p).x)
        MAX_SHIFT(p) = REAL_WIDTH(p) - DISP_SIZE(p).x;
    else
        MAX_SHIFT(p) = 0;
}

void table_adjust (Pane *p)
{
    int             i;
    int             numcols;
    short	    cmax ;

    if (NSTRINGS (p) == 0 || DISP_SIZE (p).y == 0)
	return;

	/* number of columns needed to show all strings */
    numcols = 1 + (NSTRINGS (p) - 1) / DISP_SIZE (p).y;

	/* figure stats needed for adjustment */
    cmax = 0;
    for (i = 0 ; i < NSTRINGS (p); i++) {
         cmax = MAX(cmax,(int)strlen(STRINGS(p)[i]));
    }

	/* cmax is now the maximum width of any particular string */

    COLWIDTH(p) = cmax+2 ;

    REAL_WIDTH(p) = (numcols - 1) * COLWIDTH(p) + cmax;

    if (REAL_WIDTH(p) > DISP_SIZE(p).x)
        MAX_SHIFT(p) = REAL_WIDTH(p) - DISP_SIZE(p).x;
    else
        MAX_SHIFT(p) = 0;
}

void table_paint (Pane *p)
{
    Point           ploc;
    Point           mloc;
    TextChar        tc;
    int             nstrings, dispsizex, dispsizey, selected, 
		    colwidth, i, j;
    char           *name;
    int             len;

    sm_optim_clear (OP (p));

    if (((nstrings=NSTRINGS (p)) == 0) || ((dispsizey=DISP_SIZE (p).y) == 0))
    {				/* no work to do here */
	sm_optim_touch (OP (p));
	return;
    }
    selected = SELECTED (p) ;
    colwidth = COLWIDTH (p) ;
    dispsizex = DISP_SIZE (p).x ;

    mloc.x = 0;			/* location in list of files map */
    mloc.y = 0;
    for (i = 0; i < nstrings; i++)
    {
	if (i != selected)
	    tc.style = STYLE_NORMAL;
	else
	    tc.style = STYLE_BOLD;

	ploc.x = mloc.x * COLWIDTH (p) - SHIFT (p);
	ploc.y = mloc.y;

	name = STRINGS (p)[i];
	len = MIN ((int)strlen(name), colwidth - 1);

	if (ploc.x >= dispsizex)
	{			/* start of name past right edge of screen */
	    break;
	}
	else if (ploc.x + len >= 0)
	{			/* some portion of this name is on the screen, since the name begins before the right 
				 *
				 * edge of the screen, and ends after the left, and have some positive length */
	    if (ploc.x < 0)
	    {			/* starts before left edge of screen */
		j = -ploc.x;
		ploc.x = 0;
	    }
	    else
		j = 0;

	    /* how long a segment do we want? */
	    if (ploc.x + len >= dispsizex) {
		len = dispsizex - ploc.x;
	    }
	    for (; j < len; j++)
	    {
		tc.ch = name[j];
		sm_optim_putchar (OP (p), ploc, tc);
		ploc.x++;
	    }
	}

	/* advance through the grid */
	if (mloc.y == dispsizey - 1)
	{
	    mloc.y = 0;
	    mloc.x++;
	}
	else
	    mloc.y++;
    }

    sm_optim_touch (OP (p));
}

int table_map (Pane *p, Point info)
{
    short           item;
    short           col,
                    row;

    /* no files to select */
    if (NSTRINGS (p) == 0)
	return -1;

    col = (info.x + SHIFT (p)) / COLWIDTH (p);
    row = info.y;

    item = col * DISP_SIZE (p).y + row;
    if (item < 0)
	return 0;
    else if (item >= NSTRINGS (p))
	return NSTRINGS(p) - 1;

    return (int)item;
}

short sm_table_get_selection(Pane *p)
{
    return SELECTED(p);
}

void sm_table_configuration(Pane *p, short *fullwidth, short *viewwidth, short *offset)
{
    *viewwidth = DISP_SIZE(p).x;
    *fullwidth = REAL_WIDTH(p);
    *offset    = SHIFT(p);
}

void sm_table_shift_absolute(Pane *p, short curval)
{
    if (curval < 0) 
	curval = 0;

    if (curval > MAX_SHIFT(p)) 
	curval = MAX_SHIFT(p);

    SHIFT (p) = curval;
    table_paint (p);
}


void sm_table_shift_relative(Pane *p, short shift)
{
    short	    curval;

    curval = SHIFT(p) + shift;

    if (curval < 0)
	curval = 0;

    if (curval > MAX_SHIFT(p)) 
	curval = MAX_SHIFT(p);

    SHIFT(p) = curval;
    table_paint (p);
}
