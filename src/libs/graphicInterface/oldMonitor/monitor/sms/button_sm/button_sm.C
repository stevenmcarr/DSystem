/* $Id: button_sm.C,v 1.1 1997/06/25 14:56:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 		   	button_sm.c			*/
		/*	      Button manager screen module.		*/
		/* 							*/
		/********************************************************/

#include <libs/graphicInterface/oldMonitor/include/mon/sm_def.h>
#include <libs/graphicInterface/oldMonitor/include/sms/button_sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/check_sm.h>
#include <string.h>


	/* LOCAL PROCEDURES AND VARIABLES */

STATIC(void,		move_button_window,(Window *w));/* move a set of buttons within btn pane*/
static	short		check_sm;		/* the check screen module		*/
static	Bitmap		white_pattern;		/* the white pattern behind buttons	*/
static	Bitmap		fuzzy_pattern;		/* the fuzzy pattern when btns too big	*/
#define	FUZZ_WIDTH	1			/* the width of the fuzzy borders	*/


	/* PANE INFORMATION */

struct	button_pane_info {			/* PANE INFORMATION STRUCTURE		*/
	Window		*showing;		/* the showing window in the pane	*/
	Rectangle	top;			/* the area of fuzz showing hidden top	*/
	Rectangle	bot;			/* the area of fuzz showing hidden bot.	*/
	Rectangle	left;			/* the area of fuzz showing hidden left	*/
	Rectangle 	right;			/* the area of fuzz showing hidden right*/
			};
#define	SHOWING(p)	((struct button_pane_info *) p->pane_information)->showing
#define	TOP(p)		((struct button_pane_info *) p->pane_information)->top
#define	BOT(p)		((struct button_pane_info *) p->pane_information)->bot
#define	LEFT(p)		((struct button_pane_info *) p->pane_information)->left
#define	RIGHT(p)	((struct button_pane_info *) p->pane_information)->right


	/* WINDOW INFORMATION */

struct	startup_info	{			/* WINDOW STARTUP STRUCTURE		*/
	aLayoutDef	*layout;		/* the button layout definition		*/
	short		font;			/* the font that the buttons appear in	*/
	Boolean		fill;			/* true if all space should be used	*/
			};

struct	button		{			/* DEFINED BUTTON INFORMATION		*/
	Pane		*pane;			/* the pane of this button		*/
	short		num_faces;		/* the number of faces for this button	*/
	aFaceDef	*face_list;		/* the saved list of faces for this btn	*/
	short		current_face;		/* the current button face		*/
	Generic		id;			/* the id of the button			*/
	Boolean		selected;		/* true if the button is selected	*/
	Boolean		selectable;		/* true if the button is selectable	*/
			};

struct	button_window_info {			/* WINDOW INFORMATION STRUCTURE		*/
	Point		num_buttons;		/* number of buttons in either direction*/
	Point		layout_size;		/* the pixel size of the layout		*/
	struct	button	*button_info;		/* the saved button information		*/
	short		font;			/* the font of the layout		*/
	Boolean		fill;			/* true if window stretches always	*/
	Boolean		move_horiz;		/* true if window can move horizontally	*/
	Boolean		move_vert;		/* true if window can move vertically	*/
	Boolean		top_hidden;		/* the top of this window is hidden	*/
	Boolean		bot_hidden;		/* the bottom of this window is hidden	*/
	Boolean		left_hidden;		/* the left of this window is hidden	*/
	Boolean		right_hidden;		/* the right of this window is hidden	*/
			};
#define	NUM_BUTTONS(w)	((struct button_window_info *) w->window_information)->num_buttons
#define	LAYOUT_SIZE(w)	((struct button_window_info *) w->window_information)->layout_size
#define	BUTTON_INFO(w)	((struct button_window_info *) w->window_information)->button_info
#define	FONT(w)		((struct button_window_info *) w->window_information)->font
#define	FILL(w)		((struct button_window_info *) w->window_information)->fill
#define	MOVE_HORIZ(w)	((struct button_window_info *) w->window_information)->move_horiz
#define	MOVE_VERT(w)	((struct button_window_info *) w->window_information)->move_vert
#define	TOP_HIDDEN(w)	((struct button_window_info *) w->window_information)->top_hidden
#define	BOT_HIDDEN(w)	((struct button_window_info *) w->window_information)->bot_hidden
#define	LEFT_HIDDEN(w)	((struct button_window_info *) w->window_information)->left_hidden
#define	RIGHT_HIDDEN(w)	((struct button_window_info *) w->window_information)->right_hidden


	/* SCREEN MODULE DECLARATION */
STATIC(void,		button_start,(void));
STATIC(void,		button_finish,(void));
STATIC(void,		button_create,(Pane *p));
STATIC(void,		button_resize,(Pane *p));
STATIC(RectList,	button_propagate,(Pane *p, Window *w, RectList rl));
STATIC(void,		button_destroy,(Pane *p));
STATIC(void,		button_input,(Pane *p, Rectangle r));
STATIC(void,		button_window_tile,(Window *w, struct startup_info *info, 
                                            Boolean New));
STATIC(void,		button_window_destroy,(Window *w));

static	aScreenModule	scr_mod_button =
			{
			"button",
			button_start,
			button_finish,
			button_create,
			button_resize,
			button_propagate,
			button_destroy,
			button_input,
			(sm_window_tile_func)button_window_tile,
			button_window_destroy
			};


/* Start the button screen module.							*/
static
void
button_start()
{
static	BITMAPM_UNIT	fuzzy_data[] = {	/* a fuzzy bit pattern			*/
				0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555,
				0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555
			};

	white_pattern = makeBitmap(makePoint(16,16), "button_sm: white_pattern");
	BLTBitmap(NULL_BITMAP, white_pattern,
		makeRectFromSize(Origin, getBitmapSize(white_pattern)),
		Origin, BITMAP_CLR, false);	/* use the pane's background color	*/
	fuzzy_pattern = makeBitmapFromData(makePoint(16, 16), fuzzy_data, "button_sm.c: button_start()");
	check_sm = sm_check_get_index();
}


/* Finish the button screen module.							*/
static
void
button_finish()
{
	freeBitmap(white_pattern);

	freeBitmap(fuzzy_pattern);
}


/* Create an instance of the new kind of pane.						*/
static
void
button_create(Pane *p)
{
	p->pattern = white_pattern;
	p->pane_information = (Generic) get_mem(sizeof(struct button_pane_info), "Button pane local instance data.");
	SHOWING(p) = NULL_WINDOW;
}


/* Redraw a damaged area of the button pane.						*/
static
void
button_resize(Pane *p)
{
Window			*w;			/* a current window in p		*/

	for (w = p->child[FRST_NEXT]; w; w = w->sibling[FRST_NEXT])
	{/* reset the moving and boundary conditions for w */
		if (FILL(w))
		{/* retile this window to re-fill the pane */
			modifyWindow(w, (Generic) 0);
		}
		MOVE_HORIZ(w)   = BOOL(w->parent->size.x <= w->border.lr.x - w->border.ul.x - (w->border_width << 1));
		MOVE_VERT(w)    = BOOL(w->parent->size.y <= w->border.lr.y - w->border.ul.y - (w->border_width << 1));
		if ((w == SHOWING(p)) && !MOVE_HORIZ(w) && !MOVE_VERT(w))
		{/* the the showing window should be repositioned */
			w->border = relocRect(w->border, makePoint(-w->border_width, -w->border_width));
		}
		TOP_HIDDEN(w)   = BOOL(w->border.ul.y + w->border_width < 0);
		BOT_HIDDEN(w)   = BOOL(w->border.lr.y - w->border_width > p->size.y);
		LEFT_HIDDEN(w)  = BOOL(w->border.ul.x + w->border_width < 0);
		RIGHT_HIDDEN(w) = BOOL(w->border.lr.x - w->border_width > p->size.x);
	}

	/* figure the areas where the fuzz goes */
		TOP(p) = makeRectFromSize(
			makePoint(p->border_width, p->border_width),
			makePoint(p->size.x - (p->border_width << 1), FUZZ_WIDTH)
		);
		BOT(p) = makeRectFromSize(
			makePoint(p->border_width, p->size.y - p->border_width - FUZZ_WIDTH),
			makePoint(p->size.x - (p->border_width << 1), FUZZ_WIDTH)
		);
		LEFT(p) = makeRectFromSize(
			makePoint(p->border_width, p->border_width),
			makePoint(FUZZ_WIDTH, p->size.y - (p->border_width << 1))
		);
		RIGHT(p) = makeRectFromSize(
			makePoint(p->size.x - p->border_width - FUZZ_WIDTH, p->border_width),
			makePoint(FUZZ_WIDTH, p->size.y - (p->border_width << 1))
		);

	/* do the drawing for this pane */
		drawWindowsInPane(p);
		if (SHOWING(p))
		{/* there is a window to show */
			if (TOP_HIDDEN(SHOWING(p)))
			{/* fuzz the top */
				ColorPaneWithPattern(p, TOP(p), fuzzy_pattern, Origin,
						     true);
			}
			if (BOT_HIDDEN(SHOWING(p)))
			{/* fuzz the bottom */
				ColorPaneWithPattern(p, BOT(p), fuzzy_pattern, Origin,
						     true);
			}
			if (LEFT_HIDDEN(SHOWING(p)))
			{/* fuzz the left */
				ColorPaneWithPattern(p, LEFT(p), fuzzy_pattern, Origin,
						     true);
			}
			if (RIGHT_HIDDEN(SHOWING(p)))
			{/* fuzz the right */
				ColorPaneWithPattern(p, RIGHT(p), fuzzy_pattern, Origin,
						     true);
			}
		}
}


/* Propagate changes through a button pane from a subwindow.  Return what couldn't be	*/
/* figured out.										*/
/*ARGSUSED*/
static
RectList
button_propagate(Pane *p, Window *w, RectList rl)
{
struct	rect_node	*rn;			/* the current rectangle node		*/
RectList		touched;		/* the list we have touched		*/
Rectangle		aoi;			/* the intersection of the window & pane*/

	if (SHOWING(p) && NOT(emptyRectList(rl)))
	{/* check the showing window */
		initializeRectList(&touched);
		for (rn = rl; rn; rn = rn->next)
		{/* check each rectangle to propagate */
			aoi = interRect(rn->r, SHOWING(p)->border);
			if (positiveArea(aoi))
			{/* we have found some damage */
				BLTColorCopy(SHOWING(p)->image, p->parent->image, subRect(aoi, SHOWING(p)->border.ul), transPoint(aoi.ul, p->position), NULL_BITMAP, Origin, false);
				if (TOP_HIDDEN(SHOWING(p))   && intersect(TOP(p),   aoi))
				{/* fuzz the top */
					ColorPaneWithPattern(p, interRect(TOP(p),   aoi), fuzzy_pattern, Origin, true);
				}
				if (BOT_HIDDEN(SHOWING(p))   && intersect(BOT(p),   aoi))
				{/* fuzz the bottom */
					ColorPaneWithPattern(p, interRect(BOT(p),   aoi), fuzzy_pattern, Origin, true);
				}
				if (LEFT_HIDDEN(SHOWING(p))  && intersect(LEFT(p),  aoi))
				{/* fuzz the left */
					ColorPaneWithPattern(p, interRect(LEFT(p),   aoi), fuzzy_pattern, Origin, true);
				}
				if (RIGHT_HIDDEN(SHOWING(p)) && intersect(RIGHT(p), aoi))
				{/* fuzz the right */
					ColorPaneWithPattern(p, interRect(RIGHT(p),   aoi), fuzzy_pattern, Origin, true);
				}
				pushRectList(aoi, &touched);
			}
		}
		touchPaneRectList(p, touched);
		removeFromRectList(SHOWING(p)->border, &rl);
	}

	if (NOT(emptyRectList(rl)))
	{/* the remaining areas gets the cover */
		for (rn = rl; rn; rn = rn->next)
		{/* cover each rectangle in the list */
			ColorPaneWithPattern(p, rn->r, p->pattern, Origin, true);
			if (SHOWING(p))
			{/* there is a window potentially partially hidden */
				if (TOP_HIDDEN(SHOWING(p))   && intersect(TOP(p),   rn->r))
				{/* fuzz the top */
					ColorPaneWithPattern(p, interRect(TOP(p),   rn->r), fuzzy_pattern, Origin, true);
				}
				if (BOT_HIDDEN(SHOWING(p))   && intersect(BOT(p),   rn->r))
				{/* fuzz the bottom */
					ColorPaneWithPattern(p, interRect(BOT(p),   rn->r), fuzzy_pattern, Origin, true);
				}
				if (LEFT_HIDDEN(SHOWING(p))  && intersect(LEFT(p),  rn->r))
				{/* fuzz the left */
					ColorPaneWithPattern(p, interRect(LEFT(p),   rn->r), fuzzy_pattern, Origin, true);
				}
				if (RIGHT_HIDDEN(SHOWING(p)) && intersect(RIGHT(p), rn->r))
				{/* fuzz the right */
					ColorPaneWithPattern(p, interRect(RIGHT(p),   rn->r), fuzzy_pattern, Origin, true);
				}
			}
		}
		touchPaneRectList(p, rl);
		initializeRectList(&rl);
	}
	return (rl);
}


/* Destroy the information stored in the button pane p.					*/
static
void
button_destroy(Pane *p)
{
	/* the layouts will be freed automatically */
	free_mem((void*) p->pane_information);
}


/* Handle input to the button manager screen module.					*/
static
void
button_input(Pane *p, Rectangle r)
{
Point			i;			/* the current subpane index		*/
struct button		*current;		/* pointer to button information for i	*/
Point			position;		/* the window relative input event pos.	*/
Rectangle		aoi;			/* the area of subpanes			*/

	if (SHOWING(p))
	{/* there is a child window--run the mouse */
		i = Origin;
		current = BUTTON_INFO(SHOWING(p));
		aoi = clipRectWithBorder(SHOWING(p)->border, SHOWING(p)->border, SHOWING(p)->border_width);
		while ((mon_event.type < MOUSE_KEYBOARD) && pointInRect(mon_event.loc, r))
		{/* process an input event */
			if (pointInRect(mon_event.loc, aoi))
			{/* the event is in a subpane--find the subpane */
				position = subPoint(mon_event.loc, SHOWING(p)->border.ul);
				while (position.x < current->pane->position.x)
				{/* move left one pane */
					i.x--;	current--;
				}
				while (position.x >= current->pane->position.x + current->pane->size.x)
				{/* move right one pane */
					i.x++;	current++;
				}
				while (position.y < current->pane->position.y)
				{/* move up one pane */
					i.y--;	current -= NUM_BUTTONS(SHOWING(p)).x;
				}
				while (position.y >= current->pane->position.y + current->pane->size.y)
				{/* move down one pane */
					i.y++;	current += NUM_BUTTONS(SHOWING(p)).x;
				}
				handlePane(current->pane);
				switch (mon_event.type)
				{
					case EVENT_SELECT:
						if (current->selectable)
						{/* return this select event */
							mon_event.info = i;
							mon_event.msg  = current->id;
						}
						else
						{/* selections are not allowed */
							getEvent();
						}
						break;
					case EVENT_HELP:
						mon_event.info = i;
						mon_event.msg  = current->id;
						break;
					case EVENT_MOVE:
						move_button_window(SHOWING(p));
						aoi = clipRectWithBorder(SHOWING(p)->border, SHOWING(p)->border, SHOWING(p)->border_width);
						break;
				}
			}
			else if (pointInRect(mon_event.loc, SHOWING(p)->border))
			{/* handle an event in the window but not a subpane */
				if ((mon_event.type == MOUSE_DOWN) && (mon_event.info.x == BUTTON_MOVE) && (MOVE_HORIZ(SHOWING(p)) || MOVE_VERT(SHOWING(p))))
				{/* move the window */
					move_button_window(SHOWING(p));
					aoi = clipRectWithBorder(SHOWING(p)->border, SHOWING(p)->border, SHOWING(p)->border_width);
				}
				else
					getEvent();
			}
			else
			{/* the event is not in a window */
				if (mon_event.type == MOUSE_DOWN)
				{/* button click */
					if (mon_event.info.x == BUTTON_MOVE)
					{/* reset the window */
						moveWindowTo(SHOWING(p), makePoint(-SHOWING(p)->border_width, -SHOWING(p)->border_width));
					}
				}
				getEvent();
			}
		}
	}
	else
	{/* events have no effect */
		while ((mon_event.type < MOUSE_KEYBOARD) && pointInRect(mon_event.loc, r))
		{/* eat an input event */
			getEvent();
		}
	}
}


/* Window tiling entry point.  Handle the tiling of a new window.  Return success.	*/
/*ARGSUSED*/
static
void
button_window_tile(Window *w, struct startup_info *info, Boolean New)
{
Pane			*p;			/* the current pane of interest		*/
Point			position;		/* the position of p			*/
Point			button_size;		/* the size of the current buttton	*/
Point			minimum_size;		/* the minimum size of buttons		*/
Boolean			fill_horiz;		/* true if exactly filled the horizontal*/
Boolean			fill_vert;		/* true if exactly filled the vertical	*/
short			x, y;			/* the current pane index in each dir.	*/
short			count = 0;		/* the one dimensional pane index	*/
short			i;			/* the current text list entry		*/

	if (New)
	{/* a new window */
		w->border_width = 1;
		w->window_information = (Generic) get_mem(sizeof(struct button_window_info), "button_sm.c: button_window_tile window information structure");
		NUM_BUTTONS(w)  = info->layout->size;
		FILL(w)	        = info->fill;
		FONT(w)         = info->font;
		LAYOUT_SIZE(w)  = sm_button_layout_size(info->layout, FONT(w));
		BUTTON_INFO(w)  = (struct button *) get_mem(sizeof(struct button) * NUM_BUTTONS(w).x * NUM_BUTTONS(w).y, "button_sm.c button_window_tile button info list");
	}
	fill_horiz     = BOOL((w->parent->size.x > LAYOUT_SIZE(w).x) && FILL(w) || (w->parent->size.x == LAYOUT_SIZE(w).x));
	fill_vert      = BOOL((w->parent->size.y > LAYOUT_SIZE(w).y) && FILL(w) || (w->parent->size.y == LAYOUT_SIZE(w).y));
	minimum_size.x = LAYOUT_SIZE(w).x / NUM_BUTTONS(w).x;
	minimum_size.y = LAYOUT_SIZE(w).y / NUM_BUTTONS(w).y;
	position.y = w->border_width;
	for (y = 0; y < NUM_BUTTONS(w).y; y++)
	{/* create each row of buttons */
		position.x = w->border_width;
		button_size.y = (fill_vert) ? (w->parent->size.y - position.y + w->border_width) / (NUM_BUTTONS(w).y - y) : minimum_size.y;
		for (x = 0; x < NUM_BUTTONS(w).x; x++)
		{/* create and/or reposition each button within the row */
			button_size.x = (fill_horiz) ? (w->parent->size.x - position.x + w->border_width) / (NUM_BUTTONS(w).x - x) : minimum_size.x;
			if (New)
			{/* create and position a new button */
				p = newPane(w, check_sm, position, button_size, 1);
				if (info->layout->buttons[count].num_faces)
				{/* there is a face to define */
					sm_check_set_inversion(p, csmBackgroundNormal, csmTrackInvert);
					sm_check_set_text(p, info->layout->buttons[count].face_list[0].displayed_text, FONT(w), STYLE_BOLD, csmCentered, csmWithCheck, false);
					sm_check_set_help(p, info->layout->buttons[count].face_list[0].help_text);
				}
				else
				{/* make a dead face */
					sm_check_set_inversion(p, csmBackgroundNormal, csmTrackNormal);
				}
				BUTTON_INFO(w)[count].pane         = p;
				BUTTON_INFO(w)[count].num_faces    = info->layout->buttons[count].num_faces;
				BUTTON_INFO(w)[count].face_list    = (aFaceDef *) get_mem(sizeof(aFaceDef) * info->layout->buttons[count].num_faces, "button saved faces");
				BUTTON_INFO(w)[count].current_face = 0;
				BUTTON_INFO(w)[count].id           = info->layout->buttons[count].id;
				BUTTON_INFO(w)[count].selected     = false;
				BUTTON_INFO(w)[count].selectable   = BOOL(info->layout->buttons[count].num_faces);
				for (i = 0; i < info->layout->buttons[count].num_faces; i++)
				{/* install each of the faces */
					if (info->layout->buttons[count].face_list[i].displayed_text)
					{/* there is text to be copied */
						BUTTON_INFO(w)[count].face_list[i].displayed_text = (char *) get_mem(strlen(info->layout->buttons[count].face_list[i].displayed_text) + 1, "button copy face string 1");
						(void) strcpy(BUTTON_INFO(w)[count].face_list[i].displayed_text, info->layout->buttons[count].face_list[i].displayed_text);
					}
					else
					{/* save the 0 */
						BUTTON_INFO(w)[count].face_list[i].displayed_text = (char *) 0;
					}
					if (info->layout->buttons[count].face_list[i].help_text)
					{/* there is text to be copied */
						BUTTON_INFO(w)[count].face_list[i].help_text      = (char *) get_mem(strlen(info->layout->buttons[count].face_list[i].help_text     ) + 1, "button copy face string 2");
						(void) strcpy(BUTTON_INFO(w)[count].face_list[i].help_text,      info->layout->buttons[count].face_list[i].help_text     );
					}
					else
					{/* save the 0 */
						BUTTON_INFO(w)[count].face_list[i].help_text      = (char *) 0;
					}
				}
			}
			else
			{/* just reposition the button */
				p = BUTTON_INFO(w)[count].pane;
				p->position = position;
				p->size     = button_size;
			}
			count++;
			position.x += button_size.x;
		}
		position.y += button_size.y;
	}
	position.x += w->border_width;
	position.y += w->border_width;
	w->border = makeRectFromSize(makePoint(-w->border_width, -w->border_width), position);
	MOVE_HORIZ(w)   = BOOL(w->parent->size.x < position.x - (w->border_width << 1));
	MOVE_VERT(w)    = BOOL(w->parent->size.y < position.y - (w->border_width << 1));
	TOP_HIDDEN(w)   = BOOL(w->border.ul.y + w->border_width < 0);
	BOT_HIDDEN(w)   = BOOL(w->border.lr.y - w->border_width > w->parent->size.y);
	LEFT_HIDDEN(w)  = BOOL(w->border.ul.x + w->border_width < 0);
	RIGHT_HIDDEN(w) = BOOL(w->border.lr.x - w->border_width > w->parent->size.x);
}


/* Destroy local window information.							*/
static
void
button_window_destroy(Window *w)
{
short			button;			/* the current button			*/
short			face;			/* the current face index		*/

	for (button = NUM_BUTTONS(w).x * NUM_BUTTONS(w).y - 1; button >= 0; button--)
	{/* free the button'th button information entry */
		for (face = BUTTON_INFO(w)[button].num_faces - 1; face >= 0; face--)
		{/* free the face'th face entry */
			if (BUTTON_INFO(w)[button].face_list[face].displayed_text)
			{/* there was a saved text entry here */
				free_mem((void*) BUTTON_INFO(w)[button].face_list[face].displayed_text);
			}
			if (BUTTON_INFO(w)[button].face_list[face].help_text)
			{/* there was a saved text entry here */
				free_mem((void*) BUTTON_INFO(w)[button].face_list[face].help_text);
			}
		}
		free_mem((void*) BUTTON_INFO(w)[button].face_list);
	}
	free_mem((void*) BUTTON_INFO(w));
	free_mem((void*) w->window_information);
}


/* Get the index of this screen module.  Install it if necessary.			*/
short
sm_button_get_index()
{
	return (getScreenModuleIndex(&scr_mod_button));
}


static Boolean		skw(char *);			/* forward: is this skw weirdness?	*/
/* Figure the pane size necessary for a button layout.					*/
Point
sm_button_layout_size(aLayoutDef *layout, short font)
{
short			count;			/* the current button number		*/
short			i;			/* the current face index		*/
short			current;		/* the length of the current button	*/
short			max_btn = 0;		/* number of chars in the largest button*/
char			*long_label = (char *)0;/* the longest label so far		*/
Point			size;			/* the size of the button layout	*/

	for (count = layout->size.x * layout->size.y - 1; count >= 0; count--)
	{/* check each button's text */
		for (i = 0; i < layout->buttons[count].num_faces; i++)
		{/* check each face for the largest string */
			if (layout->buttons[count].face_list[i].displayed_text)
			{/* there is text here */
				current = strlen(layout->buttons[count].face_list[i].displayed_text);
				/** check for skw's weird strings **/
				if( skw(layout->buttons[count].face_list[i].displayed_text) )
				  current = current / 2;
				if (current > max_btn)
				{/* we have a new maximum */
					max_btn = current;
					long_label = layout->buttons[count].face_list[i].displayed_text;
				}
			}
		}
	}

	if (max_btn)
	{/* there is a longest button */
		size = sm_check_pane_size(long_label, font, csmWithCheck);
		size.x *= layout->size.x;
		size.y *= layout->size.y;
		return (size);
	}
	else
	{/* no maximum was found */
		return (Origin);
	}
}


/* Create (but do not display) a new button layout.					*/
Generic
sm_button_layout_create(Pane *p, aLayoutDef *layout, short font, Boolean fill)
{
struct	startup_info	info;			/* the information for starting 	*/

	info.layout = layout;
	info.font   = font;
	info.fill   = fill;
	return ((Generic) createWindow(p, (Generic) &info));
}


/* Destroy (and hide if necessary) a button layout.					*/
void
sm_button_layout_destroy(Pane *p, Window *layout)
{
	if (layout != NULL_WINDOW)
	{/* destroy a non-trivial layout */
		if (layout == SHOWING(p))
		{/* swap out the showing display with the null one */
			sm_button_layout_show(p, NULL_WINDOW);
		}
		destroyWindow(layout);
	}
}


/* Show a previously made layout.							*/
void
sm_button_layout_show(Pane *p, Window *layout)
{
	if (layout && (layout->parent != p))
	{/* this is a bogus request */
		die_with_message("sm_button_layout_show():  layout not owned by pane.");
	}
	if (SHOWING(p))
	{/* set up to hide the old window */
		SHOWING(p)->showing = false;
	}
	SHOWING(p) = layout;
	if (SHOWING(p))
	{/* set up to show the new window in its original location */
		SHOWING(p)->showing = true;
		SHOWING(p)->border = relocRect(SHOWING(p)->border, makePoint(-SHOWING(p)->border_width, -SHOWING(p)->border_width));
		TOP_HIDDEN(SHOWING(p))   = BOOL(SHOWING(p)->border.ul.y + SHOWING(p)->border_width < 0);
		BOT_HIDDEN(SHOWING(p))   = BOOL(SHOWING(p)->border.lr.y - SHOWING(p)->border_width > p->size.y);
		LEFT_HIDDEN(SHOWING(p))  = BOOL(SHOWING(p)->border.ul.x + SHOWING(p)->border_width < 0);
		RIGHT_HIDDEN(SHOWING(p)) = BOOL(SHOWING(p)->border.lr.x - SHOWING(p)->border_width > p->size.x);
	}
	resizePane(p, p->position, p->size);
	touchPane(p);
}


/* Say whether the current button layout is completely visible.				*/
Boolean
sm_button_visible(Pane *p)
{
	return (
		BOOL(
			SHOWING(p) &&
			NOT(
				TOP_HIDDEN(  SHOWING(p)) &&
				BOT_HIDDEN(  SHOWING(p)) &&
				LEFT_HIDDEN( SHOWING(p)) &&
				RIGHT_HIDDEN(SHOWING(p))
			)
		)
	);
}


/* Modify the selectablity/selectedness of all buttons in a layout.			*/
void
sm_button_modify_all(Pane *p, Window *layout, Boolean selected, Boolean selectable)
{
short			which;			/* which button in the button list	*/

	if (layout->parent != p)
	{/* this is a bogus request */
		die_with_message("sm_button_modify_all():  layout not owned by pane.");
	}
	else
	{/* modify the mentioned pane */
		for (which = NUM_BUTTONS(layout).x * NUM_BUTTONS(layout).y - 1; which >= 0; which--)
		{/* modify each button in turn */
			if (
				BUTTON_INFO(layout)[which].num_faces &&
				(
					(BUTTON_INFO(layout)[which].selected   != selected) ||
					(BUTTON_INFO(layout)[which].selectable != selectable)
				)
			)
			{/* the selected/selectability status of this button is not what it should be */
				sm_check_set_inversion(
					BUTTON_INFO(layout)[which].pane,
					csmBackgroundNormal,
					(selectable) ? csmTrackInvert : csmTrackNormal
				);	/* Note: this call does not redraw, but the next one does */
				sm_check_set_text(
					BUTTON_INFO(layout)[which].pane,
					(char *) 0,
					FONT(layout),
					(unsigned char) (STYLE_BOLD | (selectable ? 0 : ATTR_HALF)),
					csmCentered,
					csmWithCheck,
					selected
				);
				BUTTON_INFO(layout)[which].selected   = selected;
				BUTTON_INFO(layout)[which].selectable = selectable;
			}
		}
	}
}


/* Modify the selectablity/selectedness of a button in a button layout.			*/
void
sm_button_modify_button(Pane *p, Window *layout, Generic id, 
                        Boolean selected, Boolean selectable)
{
short			which;			/* which button in the button list	*/

	if (layout->parent != p)
	{/* this is a bogus request */
		die_with_message("sm_button_modify_button():  layout not owned by pane.");
	}
	else
	{/* modify the mentioned pane */
		for (which = NUM_BUTTONS(layout).x * NUM_BUTTONS(layout).y - 1; which >= 0; which--)
		{/* figure the button to modify */
			if (BUTTON_INFO(layout)[which].num_faces && (BUTTON_INFO(layout)[which].id == id))
			{/* we have found the button to modify */
				sm_check_set_inversion(
					BUTTON_INFO(layout)[which].pane,
					csmBackgroundNormal,
					(selectable) ? csmTrackInvert : csmTrackNormal
				);	/* Note: this call does not redraw, but the next one does */
				sm_check_set_text(
					BUTTON_INFO(layout)[which].pane,
					(char *) 0,
					FONT(layout),
					(unsigned char) (STYLE_BOLD | (selectable ? 0 : ATTR_HALF)),
					csmCentered,
					csmWithCheck,
					selected
				);
				BUTTON_INFO(layout)[which].selected   = selected;
				BUTTON_INFO(layout)[which].selectable = selectable;
				return;
			}
		}
		die_with_message("sm_button_modify_button():  button is not in layout.");
	}
}


/* Switch the current face of a button in a layout.					*/
void
sm_button_switch_face(Pane *p, Window *layout, Generic id, short num)
{
short			which;			/* which button in the button list	*/
char			*displayed;		/* the text to be displayed		*/

	if (layout->parent != p)
	{/* this is a bogus request */
		die_with_message("sm_button_switch_face():  layout not owned by pane.");
	}
	else
	{/* modify the mentioned pane */
		for (which = NUM_BUTTONS(layout).x * NUM_BUTTONS(layout).y - 1; which >= 0; which--)
		{/* figure the button to modify */
			if (BUTTON_INFO(layout)[which].num_faces && (BUTTON_INFO(layout)[which].id == id))
			{/* we have found the button to switch */
				if ((num < 0) || (num >= BUTTON_INFO(layout)[which].num_faces))
				{/* this is an illegal request */
					die_with_message("sm_button_switch_face():  invalid face index.");
				}
				displayed = BUTTON_INFO(layout)[which].face_list[num].displayed_text;
				if (displayed == (char *) 0)
				{/* make sure 0 is not sent */
					displayed = ""; 
				}
				sm_check_set_text(
					BUTTON_INFO(layout)[which].pane,
					displayed,
					FONT(layout),
					(unsigned char) (STYLE_BOLD | (BUTTON_INFO(layout)[which].selectable ? 0 : ATTR_HALF)),
					csmCentered,
					csmWithCheck,
					BUTTON_INFO(layout)[which].selected
				);
				sm_check_set_help(
					BUTTON_INFO(layout)[which].pane,
					BUTTON_INFO(layout)[which].face_list[num].help_text
				);
				BUTTON_INFO(layout)[which].current_face = num;
				return;
			}
		}
		die_with_message("sm_button_switch_face():  button is not in layout.");
	}
}


/* Toggle the current face of a button in a layout.					*/
void
sm_button_toggle_face(Pane *p, Window *layout, Generic id)
{
short			which;			/* which button in the button list	*/
short			num;			/* which face index to display		*/
char			*displayed;		/* the text to be displayed		*/

	if (layout->parent != p)
	{/* this is a bogus request */
		die_with_message("sm_button_toggle_face():  layout not owned by pane.");
	}
	else
	{/* modify the mentioned pane */
		for (which = NUM_BUTTONS(layout).x * NUM_BUTTONS(layout).y - 1; which >= 0; which--)
		{/* figure the button to toggle */
			if (BUTTON_INFO(layout)[which].num_faces && (BUTTON_INFO(layout)[which].id == id))
			{/* we have found the button to switch */
				if (BUTTON_INFO(layout)[which].num_faces == 0)
				{/* this is an illegal request */
					die_with_message("sm_button_toggle_face():  invalid face index.");
				}
				num = (BUTTON_INFO(layout)[which].current_face + 1) % BUTTON_INFO(layout)[which].num_faces;
				displayed = BUTTON_INFO(layout)[which].face_list[num].displayed_text;
				if (displayed == (char *) 0)
				{/* make sure 0 is not sent */
					displayed = ""; 
				}
				sm_check_set_text(
					BUTTON_INFO(layout)[which].pane,
					displayed,
					FONT(layout),
					(unsigned char) (STYLE_BOLD | (BUTTON_INFO(layout)[which].selectable ? 0 : ATTR_HALF)),
					csmCentered,
					csmWithCheck,
					BUTTON_INFO(layout)[which].selected
				);
				sm_check_set_help(
					BUTTON_INFO(layout)[which].pane,
					BUTTON_INFO(layout)[which].face_list[num].help_text
				);
				BUTTON_INFO(layout)[which].current_face = num;
				return;
			}
		}
		die_with_message("sm_button_toggle_face():  button is not in layout.");
	}
}


/* Move the window in the parent pane & redraw.						*/
static
void
move_button_window(Window *w)
{
MouseCursor		save_cursor;		/* the saved cursor			*/
Point			last_point;		/* the previous coordinate		*/
Point			delta;			/* the amount we moved this time	*/

	if (MOVE_VERT(w) || MOVE_HORIZ(w))
	{/* the buttons can be moved */
		save_cursor = CURSOR(moving_cursor);
		do
		{/* run the mouse and the box */
			last_point = mon_event.loc;
			getEvent();
			delta.x = (MOVE_HORIZ(w))
				? ( (mon_event.loc.x >= last_point.x)
					? min(mon_event.loc.x - last_point.x, -w->border.ul.x - w->border_width)
					: max(mon_event.loc.x - last_point.x, w->parent->size.x - w->border.lr.x + w->border_width)
				)
				: 0;
			delta.y = (MOVE_VERT (w))
				? ( (mon_event.loc.y >= last_point.y)
					? min(mon_event.loc.y - last_point.y, -w->border.ul.y - w->border_width)
					: max(mon_event.loc.y - last_point.y, w->parent->size.y - w->border.lr.y + w->border_width)
				)
				: 0;
			if (delta.x)
			{/* check the left and right borders */
				LEFT_HIDDEN(w)  = BOOL(w->border.ul.x + delta.x + w->border_width < 0);
				RIGHT_HIDDEN(w) = BOOL(w->border.lr.x + delta.x - w->border_width > w->parent->size.x);
			}
			if (delta.y)
			{/* check the top and bottom borders */
				TOP_HIDDEN(w)   = BOOL(w->border.ul.y + delta.y + w->border_width < 0);
				BOT_HIDDEN(w)   = BOOL(w->border.lr.y + delta.y - w->border_width > w->parent->size.y);
			}
			if (delta.x || delta.y)
			{/* move the box */
				moveWindowTo(w, transPoint(w->border.ul, delta));
			}
		} while (mon_event.type == MOUSE_DRAG);
		(void) CURSOR(save_cursor);
	}
	else
	{/* discard the event that got us here */
		getEvent();
	}
}


/******** THE FOLLOWING CALLS ARE OBSOLETE--DO NOT USE THEM FOR NEW DEVELOPMENT *********/
/**************** MIXING OLD AND NEW CALLS WILL GIVE UNEXPECTED RESULTS *****************/

/* Return the pane size necessary to display all of a list of buttons.			*/
Point
sm_button_pane_size(Point dim, char **labels, short font)
{
short			num;			/* the total number of buttons		*/
short			i;			/* the current button index		*/
Point			size;			/* the size of the button layout	*/
aFaceDef		*face_list;		/* the list of faces for each button	*/
aLayoutDef		layout;			/* the layout being created		*/

	num = dim.x * dim.y;
	layout.size    = dim;
	layout.buttons = (aButtonDef *) get_mem(num * sizeof(aButtonDef), "sm_button_pane_size(): button layout definition");
	face_list      = (aFaceDef *)   get_mem(num * sizeof(aFaceDef),   "sm_button_pane_size(): face list");
	for (i = 0; i < num; i++)
	{/* set up the ith button definition */
		layout.buttons[i].id        = i;
		layout.buttons[i].face_list = &face_list[i];
		layout.buttons[i].num_faces = 1;
		face_list[i].displayed_text = *labels++;
		face_list[i].help_text      = (char *) 0;
	}
	size = sm_button_layout_size(&layout, font);
	free_mem((void*) face_list);
	free_mem((void*) layout.buttons);
	return (size);
}


/* Create a two dimensional array of buttons in a button pane.				*/
void
sm_button_create_btns(Pane *p, Point dim, char **labels, short font, Boolean fill)
{
Window			*old;			/* the showing button layout		*/
aFaceDef		*face_list;		/* the list of faces for each button	*/
short			num;			/* the total number of buttons		*/
short			i;			/* the current button index		*/
aLayoutDef		layout;			/* the layout being created		*/

	old = SHOWING(p);
	num = dim.x * dim.y;
	layout.size    = dim;
	layout.buttons = (aButtonDef *) get_mem(num * sizeof(aButtonDef), "sm_button_create_btns(): button layout definition");
	face_list      = (aFaceDef *)   get_mem(num * sizeof(aFaceDef),   "sm_button_create_btns(): face list");
	for (i = 0; i < num; i++)
	{/* set up the ith button definition */
		layout.buttons[i].id        = i;
		layout.buttons[i].face_list = &face_list[i];
		layout.buttons[i].num_faces = 1;
		face_list[i].displayed_text = *labels++;
		face_list[i].help_text      = (char *) 0;
	}
	sm_button_layout_show(
		p,
		(Window *) sm_button_layout_create(p, &layout, font, fill)
	);
	sm_button_layout_destroy(p, old);
	free_mem((void*) face_list);
	free_mem((void*) layout.buttons);
}

static Boolean skw(char *s)
{
	if( strlen(s) == 0 ) return false;
	else return BOOL( (s[1] & 0xC0) == 0xC0 );
}

