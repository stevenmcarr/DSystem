/* $Id: list.C,v 1.1 1997/06/25 14:57:52 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 		 	list.c				*/
		/*	      List display screen module.		*/
		/* 							*/
		/********************************************************/

#include <string.h>

#include <libs/graphicInterface/oldMonitor/include/mon/sm_def.h>
#include <libs/graphicInterface/oldMonitor/include/sms/list_sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/text_sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/scroll_sm.h>
#include <libs/support/arrays/ExtensibleArray.h>

	/* declare the screen module routines */
STATIC(void,		list_start,(void));	/* start the screen module		*/
STATIC(void,		list_create,(Pane *p)); /* create a screen module instance	*/
STATIC(void,		list_resize,(Pane *p)); /* handle a screen module resize	*/
STATIC(void,		list_destroy,(Pane *p));/* destroy a screen module instance	*/
STATIC(void,		list_input,(Pane *p, Rectangle r));/* handle flow of input events		*/

static	aScreenModule	scr_mod_list = {	/* declare the screen module 		*/
				"list",
				list_start,
				standardFinish,
				list_create,
				list_resize,
				standardNoSubWindowPropagate,
				list_destroy,
				list_input,
				standardTileNoWindow,
				standardDestroyWindow
			};

typedef FUNCTION_POINTER(void,ShiftBackFunc,(Pane*,Generic,Point));

	/* local datatypes */
struct	list_pane_info	{			/* LOCAL LIST PANE INFORMATION		*/
	Pane		*tp;			/* the text slave pane			*/
	ScrollBar	hscroll;		/* the horizontal scrollbar (or 0)	*/
	ScrollBar	vscroll;		/* the vertical scrollbar (or 0)	*/
	lsm_generator_callback generator;	/* the line information generator	*/
	ShiftBackFunc		shiftback;		/* callback for shifting (or 0)		*/
	Generic		client;			/* the client handle for callbacks	*/
	Boolean		move_vert;		/* true if vertical shifts are possible	*/
	Boolean		move_horiz;		/* true if horizontal shifts are poss.	*/
	Point		data_shift;		/* the current data shift amount	*/
	Point		disp_size;		/* the display size in chars		*/
	short		data_height;		/* the total number of lines of data	*/
	short		high_water;		/* the largest known line		*/
	lsm_line_info	**line_array;		/* the array of lines			*/
	short		start_line;		/* the starting line of a region	*/
			};
#define	TP(p)		((struct list_pane_info *) p->pane_information)->tp
#define	HSCROLL(p)	((struct list_pane_info *) p->pane_information)->hscroll
#define	VSCROLL(p)	((struct list_pane_info *) p->pane_information)->vscroll
#define	SHIFTBACK(p)	((struct list_pane_info *) p->pane_information)->shiftback
#define	GENERATOR(p)	((struct list_pane_info *) p->pane_information)->generator
#define	CLIENT(p)	((struct list_pane_info *) p->pane_information)->client
#define	MOVE_VERT(p)	((struct list_pane_info *) p->pane_information)->move_vert
#define	MOVE_HORIZ(p)	((struct list_pane_info *) p->pane_information)->move_horiz
#define	DATA_SHIFT(p)	((struct list_pane_info *) p->pane_information)->data_shift
#define	DISP_SIZE(p)	((struct list_pane_info *) p->pane_information)->disp_size
#define DATA_HEIGHT(p)	((struct list_pane_info *) p->pane_information)->data_height
#define HIGH_WATER(p)	((struct list_pane_info *) p->pane_information)->high_water
#define	LINE_ARRAY(p)	((struct list_pane_info *) p->pane_information)->line_array
#define	START_LINE(p)	((struct list_pane_info *) p->pane_information)->start_line

	/* local routines and variables */
STATIC(void,		repaint_block,(Pane *p, Rectangle r));/* repaint a block of characters	*/
STATIC(void,		check_shift,(Pane *p)); /* force SHIFT to be in range		*/
STATIC(lsm_line_info,	*get_line,(Pane *p, short i));/* get a text line			*/
STATIC(void,		put_line,(Pane *p, short i, lsm_line_info *line));/* put a text line into the array	*/
STATIC(void,		forget_line,(Pane *p, short i));/* forget a text line			*/
STATIC(void,		forget_lines,(Pane *p));/* forget all text lines		*/
STATIC(short,		lookup_line,(Pane *p, Generic id));/* lookup the index for a line's id	*/
STATIC(void,		region_figure_callback,(Pane *tp, Pane *p, Point last, 
                                                Point current, RectList *New, 
                                                RectList *fix));/* interactively run a region selection*/
STATIC(Point,		move_list_callback,(Pane *tp, Pane *p, Point shift));/* callback to move a list		*/
STATIC(void,		scrollProc,(Pane *p, ScrollBar scrollbar, short dir, int val));/* the scroller callback		*/
STATIC(void,		check_scrollbars,(Pane *p, Boolean touch));	/* refigure & repaint the scrollbars	*/
static	short		text_sm;		/* the text screen module index		*/
static	short		scroll_sm;		/* the scroll screen module index	*/



	/* SCREEN MODULE DEFINITION ROUTINES */

/* Start the list screen module.							*/
static
void
list_start()
{
	text_sm   = sm_text_get_index();
	scroll_sm = sm_scroll_get_index();
}


/* Create a list pane.									*/
static
void
list_create(Pane *p)
{
	p->pane_information = (Generic) get_mem ( sizeof(struct list_pane_info), "list_sm/list.c: pane information structure");
	TP(p)             = newSlavePane(p, text_sm, p->position, p->size, p->border_width);
	HSCROLL(p)        = NULL_PANE;
	VSCROLL(p)        = NULL_PANE;
	GENERATOR(p)      = (lsm_generator_callback) 0;
	SHIFTBACK(p)      = (ShiftBackFunc) 0;
	CLIENT(p)         = 0;
	MOVE_VERT(p)      = false;
	MOVE_HORIZ(p)     = false;
	DATA_SHIFT(p)     = Origin;
	DISP_SIZE(p)      = sm_text_size(TP(p));
	DATA_HEIGHT(p)    = 0;
	HIGH_WATER(p)     = UNUSED;
	LINE_ARRAY(p)     = (lsm_line_info **) xalloc(10, sizeof(lsm_line_info *));
}


/* Resize/reposition a list pane.							*/
static
void
list_resize(Pane *p)
{
Point			textSize;		/* the size of the text pane		*/

	textSize = p->size;
	if (VSCROLL(p))
	{/* resize the vertical scrollbar */
		if (textSize.x > SB_WIDTH && textSize.y > SB_MIN_LENGTH)
		{/* a real sized scrollbar */
			textSize.x -= SB_WIDTH;
			resizePane(VSCROLL(p), transPoint(p->position, makePoint(textSize.x, 0)), makePoint(SB_WIDTH, textSize.y));
		}
		else
		{/* a "pinched out" scrollbar */
			resizePane(VSCROLL(p), p->position, Origin);
		}
	}
	if (HSCROLL(p))
	{/* resize the horizontal scrollbar */
		if (textSize.y > SB_WIDTH && textSize.x > SB_MIN_LENGTH)
		{/* a real sized scrollbar */
			textSize.y -= SB_WIDTH;
			resizePane(HSCROLL(p), transPoint(p->position, makePoint(0, textSize.y)), makePoint(textSize.x, SB_WIDTH));
		}
		else
		{/* a "pinched out" scrollbar */
			resizePane(HSCROLL(p), p->position, Origin);
		}
	}
	resizePane(TP(p), p->position, textSize);
	DISP_SIZE(p) = sm_text_size(TP(p));
	if (DATA_HEIGHT(p))
	{/* there is a potential list--redraw the screen */
		repaint_block(p, makeRectFromSize(Origin, DISP_SIZE(p)));
	}
	check_scrollbars(p, false);
}


/* Destroy the pane and all structures below it.					*/
static
void
list_destroy(Pane *p)
{
	destroyPane(TP(p));
	if (HSCROLL(p))
	{/* destroy the horizontal scrollbar */
		destroyPane(HSCROLL(p));
	}
	if (VSCROLL(p))
	{/* destroy the vertical scrollbar */
		destroyPane(VSCROLL(p));
	}
	forget_lines(p);
	xfree((int *) LINE_ARRAY(p));
	free_mem((void*) p->pane_information);
}


/* Handle input to the list screen module.						*/
static
void
list_input(Pane *p, Rectangle r)
{
Rectangle		textRect;		/* the extent of the text pane		*/
Rectangle		hscrollRect;		/* the extent of the h-scroll pane	*/
Rectangle		vscrollRect;		/* the extent of the v-scroll pane	*/
lsm_line_info		*line;			/* the line corresponding to a select	*/
Point			shift;			/* the amount to shift			*/

	/* Get bounding rectangles on subpanes */
		textRect = makeRectFromSize(subPoint(TP(p)->position, p->position), TP(p)->size);
		if (HSCROLL(p))
		{/* there is a horizontal scrollbar */
			hscrollRect = makeRectFromSize(subPoint(HSCROLL(p)->position, p->position), HSCROLL(p)->size);
		}
		else
		{/* there is no horizonal scrollbar */
			hscrollRect = makeRectFromSize(Origin, Origin);
		}
		if (VSCROLL(p))
		{/* there is a vertical scrollbar */
			vscrollRect = makeRectFromSize(subPoint(VSCROLL(p)->position, p->position), VSCROLL(p)->size);
		}
		else
		{/* there is no vertical scrollbar */
			vscrollRect = makeRectFromSize(Origin, Origin);
		}

	while ((mon_event.type < MOUSE_KEYBOARD) && pointInRect(mon_event.loc, r))
	{/* we can handle this event */
		if (pointInRect(mon_event.loc, textRect))
		{/* the event is in the text pane */
			handlePane(TP(p));
			switch (mon_event.type)
			{/* handle the returned event */
				case EVENT_SELECT:
					mon_event.info = transPoint(mon_event.info, DATA_SHIFT(p));
					line = get_line(p, mon_event.info.y);
					if (!line || NOT(line->selectable))
					{/* throw this event away */
						getEvent();
					}
					else
					{/* save the id in msg */
						mon_event.msg  = line->id;
					}
					break;

				case EVENT_MOVE:
					shift = Origin;
					switch (mon_event.msg)
					{/* handle a move event */
						case 0: 	/* standard movement */
							shift = mon_event.info;
							if (!MOVE_HORIZ(p))
							{/* horizontal moves are not allowed */
								shift.x = 0;
							}
							if (!MOVE_VERT(p))
							{/* vertical moves are not allowed */
								shift.y = 0;
							}
							break;

						case 1:		/* move to the top */
							if (MOVE_VERT(p))
							{/* can move vertically */
								shift = makePoint(0, -DATA_SHIFT(p).y);
							}
							break;

						case 2:		/* move to the bottom */
							if (MOVE_VERT(p))
							{/* can move vertically */
								shift = makePoint(0, DATA_HEIGHT(p) - DATA_SHIFT(p).y - 1);
							}
							break;

						case 3:		/* move to the left */
							if (MOVE_HORIZ(p))
							{/* can move horizontally */
								shift = makePoint(-DATA_SHIFT(p).x, 0);
							}
							break;

						case 4:		/* move to the right */
							if (MOVE_HORIZ(p))
							{/* can move horizontally */
								shift = makePoint(32767, 0);
							}
							break;

						default:	/* ignore  */
							break;
					}
					if (NOT(equalPoint(shift, Origin)))
					{/* there is a move to be made */
						if (SHIFTBACK(p))
						{/* set up the appropriate move callback */
							(SHIFTBACK(p))(p, CLIENT(p), shift);
						}
						else
						{/* do the move ourselves */
							(void) sm_list_shift(p, shift, LSM_SHIFT_REL);
						}
					}
					getEvent();
					break;

				case EVENT_HELP:
					mon_event.info = transPoint(mon_event.info, DATA_SHIFT(p));
					line = get_line(p, mon_event.info.y);
					if (!line)
					{/* throw this event away */
						getEvent();
					}
					else
					{/* save the id in msg */
						mon_event.msg  = line->id;
					}
			}
		}
		else if (pointInRect(mon_event.loc, hscrollRect))
		{/* the event is in the horizontal scrollbar */
			handlePane(HSCROLL(p));
		}
		else if (pointInRect(mon_event.loc, vscrollRect))
		{/* the event is in the vertical scrollbar */
			handlePane(VSCROLL(p));
		}
		else
		{/* from some untiled area */
			getEvent();
		}
	}
}



	/* CLIENT CALLBACKS */

/* Get the index of this screen module.							*/
short
sm_list_get_index()
{
	return (getScreenModuleIndex(&scr_mod_list));
}


/* Calculate the minimum sized pane to display a character block.			*/
Point
sm_list_pane_size(Point size, short font, Boolean hscroll, Boolean vscroll)
{
Point			pix;			/* the size of the pane			*/

	pix = sm_text_pane_size(size, font);
	if (hscroll)
	{/* adjust for a horizontal scrollbar */
		pix.y += SB_WIDTH;
	}
	if (vscroll)
	{/* adjust for a vertical scrollbar */
		pix.x += SB_WIDTH;
	}
	return pix;
}


/* Return the smallest suggested character block for this font.				*/ 
Point
sm_list_min_block(short font)
{
Point			size;			/* the size of the block		*/
Point			pix;			/* the pixel size of the text pane	*/

	size = Origin;
	do
	{/* try to "bump" up the size in both directions */
		pix = sm_text_pane_size(size, font);
		size.x += (pix.x < SB_MIN_LENGTH);
		size.y += (pix.y < SB_MIN_LENGTH);
	} while (pix.x < SB_MIN_LENGTH || pix.y < SB_MIN_LENGTH);
	return size;
}


/* Set the movment status for this pane.						*/ 
void
sm_list_initialize(Pane *p, Generic client, lsm_generator_callback generator, 
                   short font_id, ShiftBackFunc shiftback, Boolean hscroll, Boolean vscroll)
{
	CLIENT(p)      = client;
	GENERATOR(p)   = generator;
	sm_text_change_font(TP(p), font_id);
	SHIFTBACK(p)   = shiftback;

	if (hscroll && !HSCROLL(p))
	{/* create a horizontal scrollbar */
		HSCROLL(p) = newSlavePane(p, scroll_sm, p->position, Origin, 1);
		sm_scroll_scrollee(HSCROLL(p), (Generic) p, 
                                   (sm_scroll_scrollee_callback)scrollProc);
	}
	else if (!hscroll && HSCROLL(p))
	{/* destroy a horizontal scrollbar */
		destroyPane(HSCROLL(p));
		HSCROLL(p) = NULL_PANE;
	}
	if (vscroll && !VSCROLL(p))
	{/* create a vertical scrollbar */
		VSCROLL(p) = newSlavePane(p, scroll_sm, p->position, Origin, 1);
		sm_scroll_scrollee(VSCROLL(p), (Generic) p, 
                                   (sm_scroll_scrollee_callback)scrollProc);
	}
	else if (!vscroll && VSCROLL(p))
	{/* destroy a vertical scrollbar */
		destroyPane(VSCROLL(p));
		VSCROLL(p) = NULL_PANE;
	}
	resizePane(p, p->position, p->size);
	touchPane(p);
}


/* Return the map size of the list pane.						*/
Point
sm_list_map_size(Pane *p)
{
	return (DISP_SIZE(p));
}


/* Notify of a change in the list.							*/
Point
sm_list_modified(Pane *p, Point data_shift, short count)
{
short			i;			/* the line array entry number		*/

	/* set up for the new list */
		forget_lines(p);
		DATA_HEIGHT(p) = count;
		if (DATA_HEIGHT(p) == UNUSED)
		{/* count the lines ourselves */
			for (i = 0; get_line(p, i); i++)
			{/* continue counting */
				/* kludge:  depends on DATA_HEIGHT == UNUSED */
			}
			DATA_HEIGHT(p) = i;
		}

	/* set the new shift */
		if ((data_shift.x != UNUSED) && (data_shift.y != UNUSED))
		{/* install the new data shift */
			DATA_SHIFT(p) = data_shift;
		}
		check_shift(p);  /* needed even for old shift -- skw */

	/* repaint the screen */
		repaint_block(p, makeRectFromSize(Origin, DISP_SIZE(p)));
		sm_text_pane_touch(TP(p));

	check_scrollbars(p, true);
	return (DATA_SHIFT(p));
}


/* A line has changed.									*/
void
sm_list_line_modified(Pane *p, Generic id)
{
short			l;			/* the data line number of the change	*/
Rectangle		r;			/* the damaged (screen) rectangle	*/

	l = lookup_line(p, id);
	if (l != UNUSED)
	{/* redisplay the new information */
		forget_line(p, l);
		if ((l >= DATA_SHIFT(p).y) && (l < DISP_SIZE(p).y + DATA_SHIFT(p).y))
		{/* the line is on the screen--redraw it */
			r = makeRectFromSize(
				makePoint(0, l - DATA_SHIFT(p).y),
				makePoint(DISP_SIZE(p).x, 1)
			);
			repaint_block(p, r);
			sm_text_block_touch(TP(p), r);
		}
	}
	check_scrollbars(p, true);
}


/* Change the selectability/selectedness of a line.					*/
void
sm_list_line_change(Pane *p, Generic id, Boolean selected, Boolean selectable)
{
short			l;			/* the data line number of the change	*/
Rectangle		r;			/* the damaged (screen) rectangle	*/
lsm_line_info		*line;			/* the line being modified		*/

	l = lookup_line(p, id);
	if (l != UNUSED)
	{/* change and redisplay the line */
		line = get_line(p, l);
		line->selected   = selected;
		line->selectable = selectable;
		if ((l >= DATA_SHIFT(p).y) && (l < DISP_SIZE(p).y + DATA_SHIFT(p).y))
		{/* the line is on the screen--redraw it */
			r = makeRectFromSize(
				makePoint(0, l - DATA_SHIFT(p).y),
				makePoint(DISP_SIZE(p).x, 1)
			);
			repaint_block(p, r);
			sm_text_block_touch(TP(p), r);
		}
	}
}


/* Insert a line in the list.								*/
Point
sm_list_line_insert(Pane *p, lsm_line_info *line, short dir, Generic id)
{
short			ins_line = UNUSED;	/* the line to be inserted		*/
Rectangle		r;			/* the damaged (screen) rectangle	*/
short			i;			/* the line being transfered		*/

	/* figure where to insert the line */
		if (dir == LSM_INS_FIRST)
		{/* the line will be the first one */
			ins_line = 0;
		}
		else if (dir == LSM_INS_LAST)
		{/* the line will be the new last one */
			ins_line = DATA_HEIGHT(p);
		}
		else
		{/* our line is next to some (hopefully) existing line */
			ins_line = lookup_line(p, id);
			if (ins_line != UNUSED)
			{/* the line was found--adjust for type of insertion */
				ins_line += (dir == LSM_INS_AFTER) ? 1 : 0;
			}
		}

	if (ins_line == UNUSED)
	{/* we don't know where to put the line */
		if (DATA_HEIGHT(p) == HIGH_WATER(p) + 1)
		{/* we should have known as we have all of the lines */
			die_with_message("list_sm: sm_list_line_insert() illegal request.");
		}
		else
		{/* the line is out of our relm of knowledge -- ignore the request */
			if (line->should_free)
			{/* the text should be freed */
				free_mem((void*) line->text);
			}
			free_mem((void*) line);
		}
	}
	else
	{/* continue inserting the line */
		/* verify the line information */
			if (line->len == UNUSED)
			{/* lazy client--calculate the line length */
				line->len = strlen(line->text);
			}

		/* insert the line into the array */
			DATA_HEIGHT(p)++;
			for (i = HIGH_WATER(p); i >= ins_line; i--)
			{/* move each later entry up one */
				put_line(p, i + 1, get_line(p, i));
			}
			put_line(p, ins_line, line);

		/* redisplay the list */
			if (ins_line < DATA_SHIFT(p).y)
			{/* the line was inserted before the */
				DATA_SHIFT(p).y++;
			}
			else if (ins_line < DATA_SHIFT(p).y + DISP_SIZE(p).y)
			{/* the line is on the screen (shift doesn't change) */
				sm_text_line_insert(TP(p), ins_line - DATA_SHIFT(p).y),
				r = makeRectFromSize(
					makePoint(0, ins_line - DATA_SHIFT(p).y),
					makePoint(DISP_SIZE(p).x, 1)
				);
				repaint_block(p, r);
				r.lr.y = DISP_SIZE(p).y - 1;
				sm_text_block_touch(TP(p), r);
			}
			else
			{/* the line is below the bottom of the screen--do nothing */
			}
	}
	check_scrollbars(p, true);
	return (DATA_SHIFT(p));
}


/* Remove a line from the list.  Return the new shift of the list.			*/
Point
sm_list_line_delete(Pane *p, Generic id)
{
int			del_line;		/* the line which was deleted		*/
short			l;			/* the data line number to delete	*/
Rectangle		r;			/* the damaged (screen) rectangle	*/

	del_line = lookup_line(p, id);
	if (del_line != UNUSED)
	{/* remove the line from the array */
		forget_line(p, del_line);
		for (l = del_line + 1; l <= HIGH_WATER(p); l++)
		{/* condense the array */
			put_line(p, l - 1, get_line(p, l));
		}
		put_line(p, HIGH_WATER(p), (lsm_line_info *) 0);
		DATA_HEIGHT(p)--;

		if (del_line < DATA_SHIFT(p).y)
		{/* the line is above the top of the screen */
			DATA_SHIFT(p).y--;
		}
		else if ((del_line == DATA_SHIFT(p).y) && (del_line == DATA_HEIGHT(p)) && del_line)
		{/* the only line on the screen (but not last) was deleted */
			DATA_SHIFT(p).y--;
			r = makeRectFromSize(
				Origin,
				makePoint(DISP_SIZE(p).x, 1)
			);
			repaint_block(p, r);
			sm_text_block_touch(TP(p), r);
		}
		else if (del_line < DATA_SHIFT(p).y + DISP_SIZE(p).y)
		{/* the line is on the screen (shift doesn't change) */
			sm_text_line_delete(TP(p), del_line - DATA_SHIFT(p).y),
			r = makeRectFromSize(
				makePoint(0, DISP_SIZE(p).y - 1),
				makePoint(DISP_SIZE(p).x, 1)
			);
			repaint_block(p, r);
			r.ul.y = del_line - DATA_SHIFT(p).y;
			sm_text_block_touch(TP(p), r);
		}
		else
		{/* the line is below the bottom of the screen--do nothing */
		}
	}
	check_scrollbars(p, true);
	return (DATA_SHIFT(p));
}


/* Shift the display (by amount if rel is LSM_SHIFT_ABS or to amount if LSM_SHIFT_REL).	*/
Point
sm_list_shift(Pane *p, Point amount, Boolean rel)
{
Point			old_shift;		/* the last displayed shift value	*/
Point			diff;			/* the difference in shifts		*/
Rectangle		d;			/* the damaged rectangle		*/

	old_shift     = DATA_SHIFT(p);
	DATA_SHIFT(p) = (rel) ? transPoint(DATA_SHIFT(p), amount) : amount;
	check_shift(p);
	diff = subPoint(old_shift, DATA_SHIFT(p));
	if (diff.x || diff.y)
	{/* there is work to be done */
		/* move the image */
			sm_text_block_copy(
				TP(p),
				makeRectFromSize(Origin, DISP_SIZE(p)),
				diff,
				true
			);
		/* fix the left or right side */
			d.ul.y = diff.y;
			d.lr.y = DISP_SIZE(p).y + diff.y - 1;
			if (diff.x > 0)
			{/* figure for the left */
				d.ul.x = 0;
				d.lr.x = diff.x - 1;
			}
			else
			{/* figure for the right */
				d.ul.x = DISP_SIZE(p).x + diff.x;
				d.lr.x = DISP_SIZE(p).x - 1;
			}
			repaint_block(p, d);

		/* fix the top or bottom */
			d.ul.x = 0;
			d.lr.x = DISP_SIZE(p).x - 1;
			if (diff.y > 0)
			{/* figure for the top */
				d.ul.y = 0;
				d.lr.y = diff.y - 1;
			}
			else
			{/* figure for the bottom */
				d.ul.y = DISP_SIZE(p).y + diff.y;
				d.lr.y = DISP_SIZE(p).y - 1;
			}
			repaint_block(p, d);
		sm_text_pane_touch(TP(p));
	}
	check_scrollbars(p, true);
	return (DATA_SHIFT(p));
}


/* Return the laziest y-shift necessary to display a mentioned line. 			*/
short
sm_list_line_distance(Pane *p, Generic id)
{
lsm_line_info		*line;			/* the line information the current line*/
short			l;			/* the current line number		*/

	for (l = 0; line = get_line(p, l); l++)
	{/* walk down the list of lines */
		if (line->id == id)
		{/* we have found the line */
			if (l < DATA_SHIFT(p).y)
			{/* the line is above the top of the screen */
				return (l - DATA_SHIFT(p).y);
			}
			else if (l >= DATA_SHIFT(p).y + DISP_SIZE(p).y)
			{/* the line is below the bottom of the screen */
				return (l - (DATA_SHIFT(p).y + DISP_SIZE(p).y - 1));
			}
			else
			{/* the line is on the screen */
				return (0);
			}
		}
	}

	die_with_message("list_sm: sm_list_line_distance() illegal request.");
	return (0);	/* make lint happy */
}


/* Shift the list so that the line with id is showing.  Return the new shift.		*/
Point
sm_list_line_show(Pane *p, Generic id, Boolean New, short count)
{
lsm_line_info		*line;			/* the line information the current line*/
short			l;			/* the current line number		*/

	if (New)
	{/* forget the list */
		forget_lines(p);
		DATA_HEIGHT(p) = count;
		if (DATA_HEIGHT(p) == UNUSED)
		{/* count the lines ourselves */
			for (l = 0; get_line(p, l); l++)
			{/* continue counting */
				/* kludge:  depends on DATA_HEIGHT == UNUSED */
			}
			DATA_HEIGHT(p) = l;
		}
	}

	for (l = 0; line = get_line(p, l); l++)
	{/* check each entry in the list */
		if (line->id == id)
		{/* we have found our line--move it onto the screen */
			DATA_SHIFT(p).x = 0;
			if ((l < DATA_SHIFT(p).y) || (l >= DATA_SHIFT(p).y + DISP_SIZE(p).y))
			{/* the line is not on the screen--center it */
				DATA_SHIFT(p).y = l - DISP_SIZE(p).y / 2;
			}
			check_shift(p);
			repaint_block(p, makeRectFromSize(Origin, DISP_SIZE(p)));
			sm_text_pane_touch(TP(p));
			check_scrollbars(p, true);
			return (DATA_SHIFT(p));
		}
	}

	die_with_message("list_sm: sm_list_line_show() illegal request.");
	return (Origin);	/* make lint happy */
}


	/* LOCAL ROUTINES */

/* Repaint a block of data.  Clip against the display.					*/
static
void
repaint_block(Pane *p, Rectangle r)
{
Point			loc;			/* the current location being painted	*/
char			*cptr;			/* the pointer to the current character	*/
lsm_line_info		*line;			/* the current line information		*/
Boolean			hiding;			/* true if we are not printing		*/

	r = interRect(r, makeRectFromSize(Origin, DISP_SIZE(p)));
	if ((r.lr.x >= r.ul.x) && (r.lr.y >= r.ul.y))
	{/* there is work to be done */
		sm_text_block_clear(TP(p), r);
		for (loc.y = r.ul.y; loc.y <= r.lr.y; loc.y++)
		{/* walk down the y direction */
			line = get_line(p, loc.y + DATA_SHIFT(p).y);
			if (line)
			{/* print out a calculated line */
				for (
					hiding = true, loc.x = -DATA_SHIFT(p).x, cptr = line->text;
					*cptr && ((loc.x < r.ul.x) || (hiding = BOOL(hiding && (*cptr == ' '))));
					loc.x++, cptr++
				)
				{/* walk down the x direction until we find the first printed character */
				}

				if ((loc.x <= r.lr.x) && (*cptr))
				{/* there is something to print on this line */
					sm_text_string_put(
						TP(p),
						loc,
						makePartialTextString(
							cptr, 
							(unsigned char) (STYLE_NORMAL
								      | ((line->selected  ) ? ATTR_UNDERLINE : 0        )
								      | ((line->selectable) ? 0              : ATTR_HALF)),
							MIN(line->len - (cptr - line->text), r.lr.x - loc.x + 1),
							"list_sm"
						),
						TSM_NEVER
					);
				}
			}
			else
			{/* this line is dead--all following ones will be, too */
				break;
			}
		}
	}
}


/* Fix the value of DATA_SHIFT(p) to be inside the range.				*/
static
void
check_shift(Pane *p)
{
short			i;			/* the line number of the screen	*/
short			widest;			/* the widest screen line seen so far	*/
lsm_line_info		*line;			/* the current line of the screen	*/

	DATA_SHIFT(p).y = MAX(0, MIN(DATA_HEIGHT(p) - DISP_SIZE(p).y, DATA_SHIFT(p).y));
	/* figure the maximum size of a visible line */
		widest = 0;
		for (i = DATA_SHIFT(p).y; i < DATA_SHIFT(p).y + DISP_SIZE(p).y; i++)
		{/* check the width of each line on the screen */
			line = get_line(p, i);
			if (!line)
			{/* no more lines left */
				break;
			}
			widest = MAX(widest, line->len);
		}
	DATA_SHIFT(p).x = MAX(0, MIN(widest - DISP_SIZE(p).x, DATA_SHIFT(p).x));
}


/* Get a line of information for a pane.  May have to recurse.				*/
static
lsm_line_info
*get_line(Pane *p, short i)
{
lsm_line_info		*line = 0;		/* the current line of interest		*/

	if (DATA_HEIGHT(p) == UNUSED || i < DATA_HEIGHT(p))
	{/* the index is within our capabilities  */
		line = ((i <= HIGH_WATER(p)) ? LINE_ARRAY(p)[i] : 0);
		if (line == 0)
		{/* the line is not known--ask the client callback */
			if (i == 0)
			{/* get an unknown first line */
				line = GENERATOR(p)(p, CLIENT(p), LSM_REQ_FIRST, 0);
			}
			else
			{/* recurse getting an unknown non-first line  */
				line = get_line(p, i - 1);
				if (line)
				{/* the previous line exists--get the current one */
					line = GENERATOR(p)(p, CLIENT(p), LSM_REQ_NEXT,  line->id);
				}
				
			}

			if (line)
			{/* this line is good--store it away */
				if (line->len == UNUSED)
				{/* lazy client--calculate the line length */
					line->len = strlen(line->text);
				}
				put_line(p, i, line);
			}
		}
	}
	return (line);
}


/* Put line into the line array for pane p at position i.				*/
static
void
put_line(Pane *p, short i, lsm_line_info *line)
{
	HIGH_WATER(p) = MAX(HIGH_WATER(p), i);
	LINE_ARRAY(p) = (lsm_line_info **) xput((int *) LINE_ARRAY(p), i, (char *) &line);
}


/* Free and remove line l from the line array associated with p.			*/
static
void
forget_line(Pane *p, short i)
{
lsm_line_info		*line;			/* the current line information		*/

	line = get_line(p, i);
	if (line)
	{/* there is a line to be forgotten */
		if (line->should_free)
		{/* free the text itself */
			free_mem((void*) line->text);
		}
		free_mem((void*) line);
		put_line(p, i, (lsm_line_info *) 0);
	}
}


/* Free and remove all lines from the line array associated with p.			*/
static
void
forget_lines(Pane *p)
{
short			i;			/* the current line array entry		*/

	for (i = 0; i <= HIGH_WATER(p); i++)
	{/* forget this line */
		forget_line(p, i);
	}
	HIGH_WATER(p)  = UNUSED;
	DATA_HEIGHT(p) = 0;
}


/* Lookup the index of a line in the pane's line array which matches id.		*/
static
short
lookup_line(Pane *p, Generic id)
{
lsm_line_info		*line;			/* the current line information		*/
short			i;			/* the current line number		*/

	for (i = 0; i <= HIGH_WATER(p); i++)
	{/* search each line for the one we want */
		line = get_line(p, i);
		if (line && line->id == id)
		{/* we have found our line */
			return i;
		}
	}
	return UNUSED;
}


/* Satisfy the interactive region drawing requests from the text pane.			*/
/*ARGSUSED*/
static
void
region_figure_callback(Pane *tp, Pane *p, Point last, Point current, 
                       RectList *New, RectList *fix)
{
short			low;			/* the lowest line number to be updated	*/
short			high;			/* the highes line number to be updated	*/
short			y;			/* the current list line being updated	*/
lsm_line_info		*line;			/* the actual line being updated	*/

	low   = MIN(current.y, last.y) + DATA_SHIFT(p).y;
	low  += (low  >= START_LINE(p));
	high  = MAX(current.y, last.y) + DATA_SHIFT(p).y;
	high -= (high <= START_LINE(p));
	for (y = low; (y <= high) && (y <= HIGH_WATER(p)); y++)
	{/* walk y across lines that need to be changed */
		if (y != START_LINE(p))
		{/* add the line y to the proper list */
			line = get_line(p, y);
			pushRectList(
				makeRectFromSize(
					makePoint(-DATA_SHIFT(p).x, y - DATA_SHIFT(p).y),
					makePoint(line->len, 1)
				),
				((y > START_LINE(p)) == (current.y > last.y)) ? New : fix
			);
		}
	}
}


/* Interactive call to move the list by an amount.					*/
/*ARGSUSED*/
static
Point
move_list_callback(Pane *tp, Pane *p, Point shift)
{
Point			old;			/* the old shift value			*/
Point			New;			/* the new shift value			*/

	if (!MOVE_HORIZ(p))
	{/* squelch the possible horizional movement */
		shift.x = 0;
	}
	if (!MOVE_VERT(p))
	{/* squelch the possible vertical movement */
		shift.y = 0;
	}
	old = DATA_SHIFT(p);
	if (SHIFTBACK(p))
	{/* set up the appropriate move callback */
		(SHIFTBACK(p))(p, CLIENT(p), shift);
	}
	else
	{/* do the move ourselves */
		(void) sm_list_shift(p, shift, LSM_SHIFT_REL);
	}
	New = DATA_SHIFT(p);
	return (subPoint(New, old));
}


/* Handle a scroll event from a scrollbar.						*/
/*ARGSUSED*/
static
void
scrollProc(Pane *p, ScrollBar scrollbar, short dir, int val)
{
Point			shift;			/* the the shift to move the list	*/

	if (dir == SB_HORIZONTAL)
	{/* horizontal scroll */
		shift = makePoint(val - DATA_SHIFT(p).x, 0);
	}
	else
	{/* vertical scroll */
		shift = makePoint(0, val - DATA_SHIFT(p).y);
	}

	if (SHIFTBACK(p))
	{/* set up the appropriate move callback */
		(SHIFTBACK(p))(p, CLIENT(p), shift);
	}
	else
	{/* do the move ourselves */
		(void) sm_list_shift(p, shift, LSM_SHIFT_REL);
	}
}


/* Check the on/offness and positioning of the scrollbars.				*/
static
void
check_scrollbars(Pane *p, Boolean touch)
{
lsm_line_info		*line;			/* the actual line being updated	*/
short			i;			/* the index of the current line	*/
short			widest;			/* the running widest amount		*/

	/* check the horizontal scrollbar & horizontal movement */
		widest = 0;
		for (i = DATA_SHIFT(p).y; i < DATA_SHIFT(p).y + DISP_SIZE(p).y; i++)
		{/* check the width of each line on the screen */
			line = get_line(p, i);
			if (!line)
			{/* no more lines left */
				break;
			}
			widest = MAX(widest, line->len);
		}
		MOVE_HORIZ(p) = BOOL(HSCROLL(p) && widest > DISP_SIZE(p).x);
		if (HSCROLL(p))
		{/* set horizontal scrollbar */
			if (MOVE_HORIZ(p))
			{/* set the scrollbar's values */
				sm_scroll_set(HSCROLL(p), 0, widest - DISP_SIZE(p).x, DATA_SHIFT(p).x, touch);
				sm_scroll_set_step(HSCROLL(p), 1, DISP_SIZE(p).x - 1);
			}
			sm_scroll_activate(HSCROLL(p), MOVE_HORIZ(p));
		}

	/* check the vertical scrollbar & vertical movement */
		MOVE_VERT(p) = BOOL(VSCROLL(p) && DATA_HEIGHT(p) > DISP_SIZE(p).y);
		if (VSCROLL(p))
		{/* set the vertical scrollbar */
			if (MOVE_VERT(p))
			{/* set the scrollbar's values */
				sm_scroll_set(VSCROLL(p), 0, DATA_HEIGHT(p) - DISP_SIZE(p).y, DATA_SHIFT(p).y, touch);
				sm_scroll_set_step(VSCROLL(p), 1, DISP_SIZE(p).y - 1);
			}
			sm_scroll_activate(VSCROLL(p), MOVE_VERT(p));
		}

	sm_text_set_move_status(TP(p), MOVE_HORIZ(p), MOVE_VERT(p));
}
