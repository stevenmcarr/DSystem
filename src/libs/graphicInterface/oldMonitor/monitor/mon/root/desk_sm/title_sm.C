/* $Id: title_sm.C,v 1.1 1997/06/25 14:49:52 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*                              title_sm.c                              */
/*                          Title screen module                         */
/*                                                                      */
/************************************************************************/

#include <libs/graphicInterface/oldMonitor/include/mon/sm_def.h>
#include <libs/graphicInterface/oldMonitor/monitor/mon/root/desk_sm/title_sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/vanilla_sm.h>


    /*** SCREEN MODULE DECLARATION ***/

STATIC(void,    title_start,(void));          /* start the screen module      */
STATIC(void,    title_finish,(void));         /* finish the screen module     */
STATIC(void,    title_create,(Pane *p));      /* create a sm instance         */
STATIC(void,    title_resize,(Pane *p));      /* handle a sm resize           */
STATIC(void,    title_destroy,(Pane *p));     /* destroy a sm instance        */
STATIC(void,    title_input,(Pane *p, Rectangle r));/* handle flow of input events  */

static  aScreenModule scr_mod_title = { /* declare the screen module    */
                        "title_sm",
                        title_start,
                        title_finish,
                        title_create,
                        title_resize,
                        standardNoSubWindowPropagate,
                        title_destroy,
                        title_input,
                        standardTileNoWindow,
                        standardDestroyWindow
                };


    /*** LOCAL DATATYPES ***/

struct  title_pane_info {               /* TITLE PANE INFORMATION       */
    Pane                *vanilla_pane;  /* the slave vanilla pane       */
    short               font;           /* the font to use              */
    Rectangle           quit_box;       /* position of the quit box     */
    Rectangle           resize_box;     /* position of the resize box   */
    Boolean             resizable;      /* the window can be resized    */
    Boolean             active;         /* the pane is active           */
					/** no longer necessary		*/
                };
#define         VANILLA_PANE(p)     \
            ((struct title_pane_info *) p->pane_information)->vanilla_pane
#define         FONT(p)             \
            ((struct title_pane_info *) p->pane_information)->font
#define         QUIT_BOX(p)         \
            ((struct title_pane_info *) p->pane_information)->quit_box
#define         RESIZE_BOX(p)       \
            ((struct title_pane_info *) p->pane_information)->resize_box
#define         RESIZABLE(p)        \
            ((struct title_pane_info *) p->pane_information)->resizable
#define         ACTIVE(p)           \
            ((struct title_pane_info *) p->pane_information)->active


    /*** LOCAL VARIABLES ***/

#define         BORDER      1           /* assume a one pixel border    */
#define         SLOP        2           /* two pixels around pixmaps    */
static  short   vanilla_sm;             /* vanilla screen module index  */
static  Bitmap	resize_bitmap;            /* resize bitmap                */
static	Point	resize_bitmap_size = { 16, 16 };
static  Bitmap	quit_bitmap;              /* the quit bitmap              */
static	Point	quit_bitmap_size = { 16, 16 };
static	Color	highlighted;		/* color of highlighted bar	*/
static	Color	dim;			/* color of non-highlighted bar	*/


/*********************** SCREEN MODULE ROUTINES *************************/


/* Start the title screen module.                                       */
static
void
title_start()
{
static
BITMAPM_UNIT    quit_data[] = {         /* the quit icon pattern        */
                        0xFFFF, 0xFFFF, 0xC003, 0xC003,
                        0xC003, 0xC423, 0xC243, 0xC183,
                        0xC183, 0xC243, 0xC423, 0xC003,
                        0xC003, 0xC003, 0xFFFF, 0xFFFF
                };
static
BITMAPM_UNIT    resize_data[] = {       /* the resize icon pattern      */
                        0xFFFF, 0xFFFF, 0xC063, 0xC063,
                        0xC063, 0xC063, 0xC063, 0xC063,
                        0xC063, 0xFFE3, 0xFFE3, 0xC003,
                        0xC003, 0xC003, 0xFFFF, 0xFFFF
                };

    resize_bitmap = makeBitmapFromData(resize_bitmap_size, resize_data, "title_sm.c:  title_start() resize_bitmap");
    quit_bitmap   = makeBitmapFromData(quit_bitmap_size,   quit_data,   "title_sm.c:  title_start() quit_bitmap");
    vanilla_sm = sm_vanilla_get_index();
    highlighted = getColorFromName("titlePane.highlighted");
    dim = getColorFromName("titlePane.dim");
}


/* Finish the title screen module.                                      */
static
void
title_finish()
{
    freeBitmap(resize_bitmap);
    freeBitmap(quit_bitmap);
}


/* Create a title pane.                                                 */
static
void
title_create(Pane *p)
{
    p->pane_information = (Generic) get_mem (
            sizeof(struct title_pane_info),
            "title_create(): pane information structure"
    );
    recolorPane(p, dim, highlighted, highlighted, false, false, false);
    VANILLA_PANE(p) = newSlavePane(
            p,
            vanilla_sm,
            p->position,
            p->size,
            0);
    recolorPane(VANILLA_PANE(p), dim, highlighted, highlighted, true, false, false);
    FONT(p)       = DEF_FONT_ID;
    QUIT_BOX(p)   = makeRectFromSize(Origin, Origin);
    RESIZE_BOX(p) = makeRectFromSize(Origin, Origin);
    RESIZABLE(p)  = false;
    ACTIVE(p)     = false;
}


/* Resize/reposition a title pane.                                      */
static
void
title_resize(Pane *p)
{

	ColorPaneWithPattern(p, makeRectFromSize(Origin, p->size), NULL_BITMAP,
			     Origin, false);

    /* resize the vanilla pane */
        resizePane(
                VANILLA_PANE(p),
                transPoint(
                        makePoint(
                                quit_bitmap_size.x + BORDER + SLOP * 2,
                                BORDER
                        ),
                        p->position
                ),
                makePoint(
                        p->size.x - BORDER * 2 - SLOP * 4 - quit_bitmap_size.x
                                            - resize_bitmap_size.x,
                        p->size.y - BORDER * 2
                )
        );

    /* show the quit bitmap */
        QUIT_BOX(p) = makeRectFromSize(
                makePoint(
                        BORDER + SLOP,
                        (p->size.y - quit_bitmap_size.y) / 2
                ),
                quit_bitmap_size
        );
	ColorPaneWithPatternColor(p, QUIT_BOX(p), quit_bitmap,
			     QUIT_BOX(p).ul, true, paneBackground(p), paneForeground(p));

        RESIZE_BOX(p) = makeRectFromSize(
                makePoint(
                        p->size.x - resize_bitmap_size.x - BORDER - SLOP - 1,
                        (p->size.y - resize_bitmap_size.y) / 2
                ),
                resize_bitmap_size
        );
    if (RESIZABLE(p))
	ColorPaneWithPatternColor(p, RESIZE_BOX(p), resize_bitmap, RESIZE_BOX(p).ul,
				  true, paneBackground(p), paneForeground(p));
    else  {
	ColorPaneWithPattern(p, RESIZE_BOX(p), NULL_BITMAP, RESIZE_BOX(p).ul, true);
	/* color as rest of title bar */
        RESIZE_BOX(p) = makeRectFromSize(Origin, Origin);
    }
}


/* Destroy the pane and all structures below it.                        */
static
void
title_destroy(Pane *p)
{
    destroyPane(VANILLA_PANE(p));
    free_mem((void*) p->pane_information);
}


/* Handle input to the title screen module.                             */
static
void
title_input(Pane *p, Rectangle r)
{
    while ((mon_event.type < MOUSE_KEYBOARD) && pointInRect(mon_event.loc, r))
    {/* we can handle this event */
        if (mon_event.type == MOUSE_DOWN)
        {/* this is a downclick (potentially interesting) */
            switch (mon_event.info.x)
            {/* which button */
                case BUTTON_SELECT:     /* selected something */
                    if (pointInRect(mon_event.loc, QUIT_BOX(p)))
                    {/* selected the quit option */
                        mon_event.type = EVENT_KILL;
                    }
                    else if (pointInRect(mon_event.loc, RESIZE_BOX(p)))
                    {/* selected the resize option */
                        mon_event.type = EVENT_RESIZE;
                    }
                    else
                    {/* somewhere else in the pane */
                        mon_event.type = EVENT_SELECT;
                    }
                    break;

                case BUTTON_MOVE:       /* moving */
                    mon_event.type = EVENT_MOVE;
                    break;

                case BUTTON_HELP:       /* help */
                    mon_event.type = EVENT_HELP;
                    break;
            }
            return;
        }
        getEvent();
    }
}



/*********************** SCREEN MODULE CALLBACKS ************************/


/* Get the index of the title screen module.                            */
short
sm_title_get_index()
{
    return (getScreenModuleIndex(&scr_mod_title));
}


/* Get the minimum size of a title in a given pane.                     */
Point
sm_title_pane_size(char *title, short font)
{
Point           size;                   /* the size we are working with */

    size = sm_vanilla_pane_size(title, font);

    /* adjust height for both bitmaps */
        size.y = MAX(size.y, quit_bitmap_size.y   + SLOP * 2 + BORDER * 2);
        size.y = MAX(size.y, resize_bitmap_size.y + SLOP * 2 + BORDER * 2);

    /* adjust width for both bitmaps */
        size.x += quit_bitmap_size.x + resize_bitmap_size.x;
        size.x += SLOP * 4 + BORDER * 2;

    return size;
}


/* Initalize the title pane.                                            */
void
sm_title_initialize(Pane *p, short font, Boolean resizable)
{
    RESIZABLE(p) = resizable;
    FONT(p)      = font;
}


/* Set a new title in the title pane.                                   */
void
sm_title_display(Pane *p, char *title)
{
Point           want;                   /* the size the title wants     */

    want = sm_vanilla_pane_size(title, FONT(p));
    sm_vanilla_set_text(
            VANILLA_PANE(p),
            title,
            FONT(p),
            STYLE_BOLD,
            (want.x > VANILLA_PANE(p)->size.x)
                ? VSM_JUSTIFY_LEFT
                : VSM_JUSTIFY_CENTER
    );
}


/* Change the activity status of the title pane.                        */
void
sm_title_active(Pane *p, Boolean active)
{
    if (ACTIVE(p) != active)
    {/* toggle the activeness of the pane */
	recolorPane(p, NULL_COLOR, NULL_COLOR, NULL_COLOR, true, false, true);
	sm_vanilla_set_inversion(VANILLA_PANE(p),
				 (active) ? VSM_INVERT_BKGND : VSM_NORMAL_BKGND,
				 VSM_NORMAL_TRACK);
        ACTIVE(p) = active;
    }
}
