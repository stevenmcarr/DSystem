/* $Id: text.C,v 1.1 1997/06/25 15:00:25 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 		          text.c			*/
		/*		   Text screen module.			*/
		/* 							*/
		/********************************************************/

#include <libs/graphicInterface/oldMonitor/monitor/sms/text_sm/text.i>


    /* declare the screen module */
STATIC(void,		text_start,(void));
STATIC(void,		text_finish,(void));
STATIC(void,		text_create,(Pane *p));
STATIC(void,		text_resize,(Pane *p));
STATIC(void,		text_destroy,(Pane *p));
STATIC(void,		text_input,(Pane *p, Rectangle r));

static	Bitmap		white_pattern;		/* background for text pane		*/
static	Color		text_foreground;	/* color of text			*/
static	Color		text_background;	/* color behind text			*/

static  aScreenModule	scr_mod_text = {
			"text",
			text_start,
			text_finish,
			text_create,
			text_resize,
			standardNoSubWindowPropagate,
			text_destroy,
			text_input,
			standardTileNoWindow,
			standardDestroyWindow
			};

STATIC(void,		handle_move,(Pane *p, Point start_coord));	
/* handle the text_sm moving convention	*/


/* Start the screen module--define the white_pattern background.			*/
static
void
text_start()
{
	text_foreground = getColorFromName("text.foreground");
	text_background = getColorFromName("text.background");

	white_pattern = makeBitmap(makePoint(16,16), "text_sm: white_pattern");
	BLTBitmap(NULL_BITMAP, white_pattern, makeRectFromSize(Origin, getBitmapSize(white_pattern)),
		Origin, BITMAP_CLR, false);	/* use the pane's background color	*/
}


/* Finish the screen module--free the white_pattern background.				*/
static
void
text_finish()
{
	freeBitmap(white_pattern);
}


/* Create a text pane information structure and set defaults.				*/
static
void
text_create(Pane *p)
{
	p->pattern	= white_pattern;
	p->pane_information = (Generic) get_mem(sizeof(struct text_pane_info), "text.c: text pane information structure");
	recolorPane(p, text_foreground, text_background, paneBorderColor(p), false, false, false);
	PANE_SIZE(p)   = Origin;
	sm_text_change_font(p, DEF_FONT_ID);
	MOVE_HORIZ(p)  = TSM_NO_MOVE_HORIZ;
	MOVE_VERT(p)   = TSM_NO_MOVE_VERT;
	BATCH_TOUCH(p) = 0;
	REGION_DEAD_RETURN(p) = makePoint(UNUSED, UNUSED);
	initializeRectList(&BATCH_RL(p));
}


/* Resize/reposition the pane.								*/
static
void
text_resize(Pane *p)
{
	if (!equalPoint(p->size, PANE_SIZE(p)))
	{/* the pane size has changed */
		sm_text_change_font(p, FONT(p));
	}
}


/* Destroy the pane and all structures below it.					*/
static
void
text_destroy(Pane *p)
{
	freeRectList(&BATCH_RL(p));
	free_mem((void*) p->pane_information);
}


/* Handle input to the text screen module.						*/
static
void
text_input(Pane *p, Rectangle r)
{
Point			coord;			/* the char coord of a down click	*/

	while ((mon_event.type < MOUSE_KEYBOARD) && pointInRect(mon_event.loc, r))
	{/* interpret the current event */
		if (mon_event.type == MOUSE_DOWN)
		{/* downclick--convert to a higher level event */
			coord = sm_text_point_pg(p, mon_event.loc, TSM_CLIP);
			switch(mon_event.info.x)
			{
				case BUTTON_SELECT:
					mon_event.type = EVENT_SELECT;
					mon_event.info = coord;
					break;
				case BUTTON_MOVE:
					handle_move(p, coord);
					break;
				case BUTTON_HELP:
					mon_event.type = EVENT_HELP;
					mon_event.info = coord;
					break;
			}
		}
		else
		{/* not an interesting event */
			getEvent();
		}
	}
}


/* Get the index of this screen module.							*/
short
sm_text_get_index()
{
	return (getScreenModuleIndex(&scr_mod_text));
}


#define	SLOP		4			/* the minimum number of border pixels	*/
/* Return the smallest pane necessary to show 'size' characters in 'font_id' font.	*/
Point
sm_text_pane_size(Point size, short font_id)
{
Point			glyph_size;		/* the size of one character		*/

	glyph_size = fontSize(font_id);
	size.x *= glyph_size.x;
	size.y *= glyph_size.y;
	size.x += SLOP + 1;
	size.y += SLOP + 1 + glyph_size.y - fontBaseline(font_id);
	return (size);
}


/* Change the font.									*/
void
sm_text_change_font(Pane *p, short font_id)
{
short			shift;			/* due to proper baseline placement	*/

	FONT(p)         = font_id;
	PANE_SIZE(p)    = p->size;
	GLYPH_SIZE(p)   = fontSize(font_id);
	shift           = GLYPH_SIZE(p).y - fontBaseline(font_id);
	MAP_SIZE(p).x   = (PANE_SIZE(p).x - p->border_width - SLOP        ) / GLYPH_SIZE(p).x;
	MAP_SIZE(p).y   = (PANE_SIZE(p).y - p->border_width - SLOP - shift) / GLYPH_SIZE(p).y;
	MAP_ORIGIN(p).x = (PANE_SIZE(p).x - MAP_SIZE(p).x * GLYPH_SIZE(p).x        ) / 2;
	MAP_ORIGIN(p).y = (PANE_SIZE(p).y - MAP_SIZE(p).y * GLYPH_SIZE(p).y + shift) / 2;
}


/* Change the text movement event capability.						*/
void
sm_text_set_move_status(Pane *p, Boolean horiz, Boolean vert)
/*Boolean  horiz;			/* true if horizontal move ability	*/
/*Boolean  vert;			/* true if vertical move ability	*/
{
	MOVE_HORIZ(p) = horiz;
	MOVE_VERT(p)  = vert;
}


/* Change the down click to a higher level move event.					*/
static
void
handle_move(Pane *p, Point start_coord)
{
MouseCursor		save_cursor;		/* the saved cursor			*/
Point			current_coord;		/* the current marked character position*/
Point			last_coord;		/* the last marked character position	*/
Boolean			quit = false;		/* true if we should quit		*/
Pane			*root_pane;		/* the root pane		*/

    /* find the root pane */
	for (root_pane = p; root_pane->parent->parent; root_pane = root_pane->parent->parent)
	        ;

	if (MOVE_VERT(p) || MOVE_HORIZ(p))
	{/* show feedback */
		current_coord = sm_text_point_pg(p, mon_event.loc, TSM_CLIP);
		save_cursor = CURSOR(invisible_cursor);
		ghostBoxInPane(p, sm_text_rect_gp(p, makeRect(start_coord, start_coord)), 2, true, true);
		if (!equalPoint(current_coord, start_coord))
		{/* turn on the current position */
			ghostBoxInPane(p, sm_text_rect_gp(p, makeRect(current_coord, current_coord)), 2, true, true);
		}
		while (NOT(quit))
		{
			getEvent();
			if (mon_event.type == MOUSE_DRAG)
			{/* handle the dragging */
				last_coord = current_coord;
				current_coord = sm_text_point_pg(p, mon_event.loc, TSM_NO_CLIP);
				if (!MOVE_VERT(p))
				{/* cannot move up and down */
					current_coord.y = last_coord.y;
				}
				if (!MOVE_HORIZ(p))
				{/* cannot move left and right */
					current_coord.x = last_coord.x;
				}
				if ((current_coord.x != last_coord.x) || (current_coord.y != last_coord.y))
				{/* the cursor has moved one character */
					if (!equalPoint(last_coord, start_coord))
					{/* turn off the last box */
						ghostBoxInPane(p, sm_text_rect_gp(p, makeRect(last_coord, last_coord)), 2, true, true);
					}
					if (!equalPoint(current_coord, start_coord))
					{/* turn on the new box */
						ghostBoxInPane(p, sm_text_rect_gp(p, makeRect(current_coord, current_coord)), 2, true, true);
					}
				}
			}
			else if (mon_event.type == MOUSE_KEYBOARD)
			{/* handle a keyboard event */
				switch (toKbChar(mon_event.info.x))
				{/* handle keyboard charcter */
					case toKbChar('t'):
					case toKbChar('T'):
					case KB_ArrowU:
						mon_event.type = EVENT_MOVE;
						mon_event.info = Origin;
						mon_event.msg = 1;
						ungetEvent();
						quit = true;
						break;
					case toKbChar('b'):
					case toKbChar('B'):
					case KB_ArrowD:
						mon_event.type = EVENT_MOVE;
						mon_event.info = Origin;
						mon_event.msg = 2;
						ungetEvent();
						quit = true;
						break;
					case toKbChar('l'):
					case toKbChar('L'):
					case KB_ArrowL:
						mon_event.type = EVENT_MOVE;
						mon_event.info = Origin;
						mon_event.msg = 3;
						ungetEvent();
						quit = true;
						break;
					case toKbChar('r'):
					case toKbChar('R'):
					case KB_ArrowR:
						mon_event.type = EVENT_MOVE;
						mon_event.info = Origin;
						mon_event.msg = 4;
						ungetEvent();
						quit = true;
						break;
					case toKbChar('c'):
					case toKbChar('C'):
					case toKbChar('m'):
					case toKbChar('M'):
						mon_event.type = EVENT_MOVE;
						mon_event.info = Origin;
						mon_event.msg = 5;
						ungetEvent();
						quit = true;
						break;
					default:
						flashPane(p);
						break;
				}
			}
			else
			{/* we don't know about this event */
				quit = true;
			}
		}
		if (!equalPoint(current_coord, start_coord))
		{/* turn off the current position */
			ghostBoxInPane(p, sm_text_rect_gp(p, makeRect(current_coord, current_coord)), 2, true, true);
		}
		ghostBoxInPane(p, sm_text_rect_gp(p, makeRect(start_coord, start_coord)), 2, true, true);
		(void) CURSOR(save_cursor);

		if (mon_event.type == MOUSE_UP)
		{/* make the move */
			mon_event.type   = EVENT_MOVE;
			mon_event.info.x = (MOVE_HORIZ(p)) ? start_coord.x - current_coord.x : start_coord.x;
			mon_event.info.y = (MOVE_VERT (p)) ? start_coord.y - current_coord.y : start_coord.y;
			mon_event.msg    = 0;
		}
	}
	else
	{/* just return the coordinate */
		mon_event.type = EVENT_MOVE;
		mon_event.info = start_coord;
		mon_event.msg  = 0;
	}
}
