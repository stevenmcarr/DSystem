/* $Id: vanilla.C,v 1.1 1997/06/25 15:01:17 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 		       vanilla_sm.c			*/
		/*	           Vanilla screen module		*/
		/* 							*/
		/********************************************************/

#include <libs/graphicInterface/oldMonitor/include/mon/sm_def.h>
#include <libs/support/strings/rn_string.h>
#include <string.h>
#include <libs/graphicInterface/oldMonitor/include/sms/vanilla_sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/check_sm.h>

EXTERN(void, sm_check_set_text,(Pane *p, char *text, short font, 
                                unsigned char style, Boolean justify,
                                ToggleType flavor, Boolean checked));	

extern char *CHECK_MARKED;			/* special characters in Rn fonts	*/
extern char *CHECK_EMPTY;
extern char *BOX_MARKED;
extern char *BOX_EMPTY;

STATIC(Point,		printMarker,(Pane *p, Point inset, Rectangle clipper));
/* forward: print box or check		*/
STATIC(Point,		sizeMarker,(short fint_id, ToggleType flavor));
/* Forward: size of box or check	*/
STATIC(Point,		findTextSize,(char *text));
/* Forward: bounding box of text, in chars */
STATIC(Boolean,		skw,(char *s));       		
/* forward: is this skw weirdness?	*/

	/* LOCAL INFORMATION */

#define	SLOP		4			/* added horizontal and vertical space	*/
static	MouseCursor	crosshair_cursor;	/* our own crosshair cursor		*/
static	Bitmap		white_pattern;		/* our own background			*/
static	Color		foreground;		/* text color				*/
static	Color		background;		/* color behind text			*/

struct	van_pane_info	{			/* PANE INFORMATIenvX-sun4-O.colorON STRUCTURE		*/
	char		*text;			/* saved text				*/
	short		font_id;		/* the font index the text		*/
	unsigned char	style;			/* the style to use			*/
	Point		block_size;		/* the size of the block for text	*/
	ToggleType	flavor;			/* type of toggle, to use		*/
	Boolean		toggle;			/* setting of toggle			*/
   	Boolean		justify;		/* true if text is left justified 	*/
	Boolean		invert_normal;		/* true if the pane is inverted		*/
	Boolean		invert_track;		/* "invert when pane has mouse"		*/
	char		*help_text;		/* the help message to give		*/
			};

/* Invert and color invariant:  The vanilla pane's foreground and background colors	*/
/*	will always reflect the current status of invert_normal and whether the cursor	*/
/*	is in the pane.  That is, when invert_normal changes value, the foreground and	*/
/*	 background colors are inverted.  The same is true for when the cursor moves	*/
/*	into xor out of the pane.  The border and background colors are always the same.*/

#define	INVERT_BKGND(p) ((struct van_pane_info *) p->pane_information)->invert_normal
#define	INVERT_TRACK(p)	((struct van_pane_info *) p->pane_information)->invert_track
#define	TEXT(p)		((struct van_pane_info *) p->pane_information)->text
#define BLOCK_SIZE(p)   ((struct van_pane_info *) p->pane_information)->block_size
#define	FONT_ID(p)	((struct van_pane_info *) p->pane_information)->font_id
#define JUSTIFY(p)	((struct van_pane_info *) p->pane_information)->justify
#define STYLE(p)	((struct van_pane_info *) p->pane_information)->style
#define HELP_TEXT(p)	((struct van_pane_info *) p->pane_information)->help_text
#define FLAVOR(p)	((struct van_pane_info *) p->pane_information)->flavor
#define TOGGLE(p)	((struct van_pane_info *) p->pane_information)->toggle


	/* SCREEN MODULE DECLARATION */

STATIC(void,		check_start,(void));
STATIC(void,		check_finish,(void));
STATIC(void,		check_create,(Pane *p));
STATIC(void,		check_resize,(Pane *p));
STATIC(void,		check_destroy,(Pane *p));
STATIC(void,		check_input,(Pane *p, Rectangle r));

static aScreenModule	scr_mod_check =
			{
			"check",
			check_start,
			check_finish,
			check_create,
			check_resize,
			standardNoSubWindowPropagate,
			check_destroy,
			check_input,
			standardTileNoWindow,
			standardDestroyWindow
			};


/* Start the screen module--define the cursor and white_pattern background.		*/
static
void
check_start()
{
static	BITMAPM_UNIT	crosshair_data[] = {	/* a crosshair image data		*/
				0x0000, 0x0000, 0x0200, 0x0200, 0x0200, 0x0200, 0x3FE0, 0x0200,
				0x0200, 0x0200, 0x0200, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
			};
static	BITMAPM_UNIT	crosshair_outline_data[] = {/* a crosshair outline image data	*/
				0x0000,	0x0700,	0x0700,	0x0700,	0x0700,	0x7FF0,	0x7FF0,	0x7FF0,
				0x0700,	0x0700,	0x0700,	0x0700,	0x0000,	0x0000,	0x0000,	0x0000
			};

	crosshair_cursor = makeColorMouseCursorFromData(
				makePoint(16, 16),
				crosshair_data,
				BITMAP_XOR,
				crosshair_data,
				crosshair_outline_data,
				makePoint(6, 6),
				getColorFromName("vanillaCursor.foreground"),
				getColorFromName("vanillaCursor.background"),
				"vanilla.c: check_start()"
	);

	white_pattern = makeBitmap(makePoint(16,16), "check_sm: white_pattern");
	BLTBitmap(NULL_BITMAP, white_pattern, makeRectFromSize(Origin, getBitmapSize(white_pattern)),
		Origin, BITMAP_CLR, false);	/* use the pane's background color	*/

	foreground = getColorFromName("vanillaText.foreground");
	background = getColorFromName("vanillaText.background");
}


/* Finish the screen module--free the cursor and white_pattern background.		*/
static
void
check_finish()
{
	freeMouseCursor(crosshair_cursor);
	freeBitmap(white_pattern);
}


/* Set the default status for this pane: non-inverted and no buttons returned.		*/
static
void
check_create(Pane *p)
{
	p->pane_information = (Generic) get_mem ( sizeof(struct van_pane_info), "vanilla_sm.c: van_create pane information structure");
	recolorPane(p, foreground, background, paneBorderColor(p), false, false, false);
	INVERT_TRACK(p) = VSM_NORMAL_TRACK;
	INVERT_BKGND(p) = VSM_NORMAL_BKGND;
	TEXT(p)         = (char *) get_mem (1, "vanilla_sm.c: van_create original text string");
	TEXT(p)[0]      = '\0';
	BLOCK_SIZE(p)   = Origin;
	FONT_ID(p)      = DEF_FONT_ID;
	JUSTIFY(p)      = csmLeft;
	FLAVOR(p)	= csmPlain;
	TOGGLE(p)	= false;
	STYLE(p)        = STYLE_NORMAL;
	HELP_TEXT(p)    = (char *) 0;
	p->pattern	= white_pattern;
	p->cursor	= crosshair_cursor;
}


/* Resize/reposition the check pane.							*/
static
void
check_resize(Pane *p)
{
Point			font_size;		/* the current font size	 	*/
char			*line;			/* the start of the current line	*/
char			*end_ptr;		/* the end (\0 or \n) of the current ln.*/
Point			position;		/* the pane relative position of text 	*/
Point			inset;			/* allow for slop, flavor, vcentering	*/
Point			markerUL;		/* location to draw marker		*/
Rectangle		clipper;		/* clip all fontWrites with this	*/
TextString		skw_text;		/* to print skw's weird strings		*/

	font_size = fontSize(FONT_ID(p));
	clipper = makeRectFromSize(Origin,p->size);
	markerUL.y = (p->size.y - font_size.y * BLOCK_SIZE(p).y + font_size.y - fontBaseline(FONT_ID(p))) / 2;
	markerUL.x = SLOP/2;


	inset = transPoint(markerUL,sizeMarker(FONT_ID(p), FLAVOR(p)));
	position = inset;

	line = TEXT(p);

	/* prepare to print skw's weird strings */
	if( skw(line) )
	  { skw_text.num_tc = strlen(line)/(size_t)2;
	    skw_text.tc_ptr = (TextChar *) line;
	    skw_text.ephemeral = false;
	  }

	do
	{/* write the current line onto the temporary pane */
		if (!(end_ptr = strchr(line, '\n')))
		{/* we have reached the end of the string */
			end_ptr = line + strlen(line);
		}

		if (JUSTIFY(p) == csmCentered  &&  ! skw(line))
			/* Ignore inset.  Center in pane instead. */
			position.x = (p->size.x - font_size.x * (end_ptr - line))/2;

		if (skw(line))
		{/* skw text is only one line long */
			fontWritePane(
				p,
				position,
				clipper,
				skw_text,
				FONT_ID(p)
			);
		}
		else
		{/* print this line of text */
			fontWritePane(
				p,
				position,
				clipper,
				makePartialTextString(line, STYLE(p), end_ptr - line, "vanilla_sm"),
				FONT_ID(p)
			);
		}
		line = end_ptr + 1;
		position.y += font_size.y;
	} while (*end_ptr);

	(void) printMarker(p, markerUL, clipper);
}


/* Destroy the pane and all structures below it.					*/
static
void
check_destroy(Pane *p)
{
	if (HELP_TEXT(p))
	{/* free the registered help text */
		free_mem((void*) HELP_TEXT(p));
	}
	sfree(TEXT(p));
	free_mem((void*) p->pane_information);
}


/* Handle input to the vanilla/check screen module.						*/
static
void
check_input(Pane *p, Rectangle r)
{
	if (INVERT_TRACK(p))
	{/* invert the pane */
		sm_vanilla_invert(p);
	}

	while ((mon_event.type < MOUSE_KEYBOARD) && pointInRect(mon_event.loc, r))
	{/* eat inputs until an interesting event comes along */
		if (mon_event.type == MOUSE_DOWN)
		{/* a button click sets up a return event */
			if (mon_event.info.x == BUTTON_SELECT)
			{/* this is a select event */
				mon_event.info = Origin;
				mon_event.type = EVENT_SELECT;
			}
			else if (mon_event.info.x == BUTTON_MOVE)
			{/* this is a move event */
				mon_event.type = EVENT_MOVE;
			}
			else if (mon_event.info.x == BUTTON_HELP)
			{/* this is a help event */
				mon_event.info = Origin;
				if (HELP_TEXT(p))
				{/* give the help message */
					message(HELP_TEXT(p));
					continue;
				}
				else
				{/* return a help event */
					mon_event.type = EVENT_HELP;
				}
			}
			mon_event.info = Origin;
		}
		else
			getEvent();
	}

	if (INVERT_TRACK(p))
	{/* invert the pane */
		sm_vanilla_invert(p);
	}
}


/****************************** VANILLA USER CALLBACKS ******************************/

/* Get the index of this screen module.							*/
short
sm_vanilla_get_index()
{
	return (getScreenModuleIndex(&scr_mod_check));
}

/* Return the pane size necessary to display all of a piece of text.			*/
Point
sm_vanilla_pane_size(char *s, short font_id)
{
	return sm_check_pane_size( s, font_id, csmPlain );
}


/* Change the normal inversion status for this pane and invert the pane.		*/
void
sm_vanilla_invert(Pane *p)
{
	INVERT_BKGND(p) = NOT(INVERT_BKGND(p));
	recolorPane(p, NULL_COLOR, NULL_COLOR, NULL_COLOR, true, false, true);
}


/* Change the button normal/tracking inversion status for this pane.			*/
void
sm_vanilla_set_inversion(Pane *p, Boolean normal, Boolean track)
{
	if (normal != INVERT_BKGND(p))
		recolorPane(p, NULL_COLOR, NULL_COLOR, NULL_COLOR, true, false, true);
	INVERT_BKGND(p) = normal;
	INVERT_TRACK(p) = track;
	return;
}


/* Return the button normal/tracking inversion status for this pane.			*/
void
sm_vanilla_get_inversion(Pane *p, Boolean *normal, Boolean *track)
{
	*normal = INVERT_BKGND(p) ;
	*track  = INVERT_TRACK(p) ;
}


/* Set the text to be displayed.							*/
void
sm_vanilla_set_text(Pane *p, char *text, short font_id, unsigned char style, 
                    Boolean justify)
{

	sm_check_set_text( p, text, font_id, style, justify, csmPlain, false );
}


/* Set help text or help return status.							*/
void
sm_vanilla_set_help(Pane *p, char *help)
{
	if (HELP_TEXT(p))
	{/* free the old help text */
		sfree(HELP_TEXT(p));
		HELP_TEXT(p) = 0;
	}
	if (help)
	{/* install the new help */
		HELP_TEXT(p) = ssave(help);
	}
}

/****************************** CHECK USER CALLBACKS ******************************/



short sm_check_get_index()
{
	return getScreenModuleIndex(&scr_mod_check);
}


/*  The following three functions were originally pointers assigned to the  */
/*  vanilla_sm functions.  To make matters clearer, the functions now call  */
/*  those vanilla functions they were originally pointing to.               */

void
sm_check_invert(Pane *p)
{
	sm_vanilla_invert(p);
}


void
sm_check_set_inversion(Pane *p, Boolean normal, Boolean track)
{
	sm_vanilla_set_inversion(p, normal, track);
}


void
sm_check_set_help(Pane *p, char *help)
{
	sm_vanilla_set_help(p, help);
}


Point
sm_check_pane_size(char *s, short font_id, ToggleType flavor)
{
Point			size;			/* the size being calculated		*/
Point			font_size;		/* the size of a character of the font	*/
Point			block_size;		/* the size of the character block	*/
Point			mark_size;		/* the size of the box or check		*/

	mark_size	= sizeMarker( font_id, flavor );
	font_size	= fontSize(font_id);
	block_size	= findTextSize(s);
	size.x = font_size.x * block_size.x + SLOP + 2 + mark_size.x * 2;
	size.y = font_size.y * block_size.y + SLOP + 2 + font_size.y - fontBaseline(font_id);
	return (size);
}

void
sm_check_set_text(Pane *p, char *text, short font, unsigned char style, 
                  Boolean justify, ToggleType flavor, Boolean checked)
{
	if (text)
	{/* install the new text */
		sfree(TEXT(p));
		TEXT(p) = ssave(text);
		BLOCK_SIZE(p) = findTextSize(text);
	}

	FONT_ID(p)	= font;
	STYLE(p)	= style;
	JUSTIFY(p)	= justify;
	if (flavor != csmNoChange)
	{
		FLAVOR(p) = flavor;
		TOGGLE(p) = checked;
	}

	resizePane(p, p->position, p->size );
	touchPane(p);
}


Boolean sm_check_change_check(Pane *p, Boolean checked)
{
	if (checked == TOGGLE(p))
		return checked;

	if (FLAVOR(p) == csmPlain)
		/* ??? */
		return false;

	sm_check_set_text( p, (char *) 0, FONT_ID(p), STYLE(p), JUSTIFY(p), FLAVOR(p), checked );

	return NOT(checked);
}

void sm_check_change_text(Pane *p, char *text, unsigned char style)
{
	sm_check_set_text( p, text, FONT_ID(p), style, JUSTIFY(p), csmNoChange, false );
}

/****************************** utilities ******************************/

static Point findTextSize(char *text)
{
	Point size;
	int len;

    /* first check for skw's weird strings */
	if( skw(text) )  return makePoint(strlen(text)/(size_t)2, 1);

	size = Origin;
	do
	{/* walk through each conceptual line */
		for (len = 0; text[len] != '\0' && text[len] != '\n'; len++)
		{/* walk to the end of this conceptual line */
		}
		size.x = max(size.x, len);
		size.y++;
		text += len + (text[len] == '\n');
	} while (*text);

	return size;
}


static
Point
printMarker(Pane *p, Point inset, Rectangle clipper)
{
	char *mark;
	Point font_size;

	font_size = fontSize(FONT_ID(p));

	switch(FLAVOR(p))
	{
	    case csmWithBox:
		mark = TOGGLE(p) ? BOX_MARKED : BOX_EMPTY ;
		break;
	    case csmWithCheck:
		mark = TOGGLE(p) ? CHECK_MARKED : CHECK_EMPTY ;
		break;
	    default:
		return inset;
	}

	fontWritePane(p,
		      inset,
		      clipper,
		      makeTextString( mark, STYLE(p), "vanilla_sm" ),
		      FONT_ID(p)
		      );
	inset.x += font_size.x * strlen(mark);

	return inset;
}

static
Point
sizeMarker(short font_id, ToggleType flavor)
{
	char *mark;
	Point fs;

	fs = fontSize(font_id);

	switch(flavor)
	{
	    case csmWithBox:
		mark = BOX_MARKED ;
		break;
	    case csmWithCheck:
		mark = CHECK_MARKED ;
		break;
	    default:
		mark = "";
	}

	return makePoint( fs.x*strlen(mark), 0 );
}

static Boolean skw(char *s)
{
	if( strlen(s) == 0 ) return false;
	else return BOOL( (s[1] & 0xC0) == 0xC0 );
}

