/* $Id: scr_mod.C,v 1.1 1997/06/25 14:53:48 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 			scr_mod.c			*/
		/* 							*/
		/* 	   Screen module interface/junction box.	*/
		/* 							*/
		/********************************************************/

#include <libs/graphicInterface/oldMonitor/include/mon/sm_def.h>
#include <libs/graphicInterface/oldMonitor/monitor/mon/scr_mod.h>

STATIC(Rectangle, visiblePaneRect,(Pane *p, Point pt));	
                                        /* figure the visible portion of a pane	*/
STATIC(void, damagedPaneRectList,(Pane *p, RectList rl));	
                                        /* reconstruct a damaged areas of a pane*/
STATIC(void, drawWindow,(Window *w));	/* draw a window in its pane		*/
STATIC(void, attach_slave_pane,(Pane *par, Pane *slave, short dir));
					/* add slave pane to parent's list	*/
STATIC(void, updateWindowColors,(Window *w));
					/* update effective colors fields of	*/
					/* window and its descendants		*/
STATIC(void, updatePaneColors,(/* Pane *p */));
					/* update effective colors fields of	*/
					/* pane and its descendants		*/
STATIC(void, updateSlavePaneColors,(Pane *p));
					/* update effective colors fields of	*/
					/* slave pane and its descendants	*/
STATIC(Rectangle, clipWithInsides,(Pane *p, Rectangle rect));	
                                        /* clip a rectangle to a pane's insides	*/


	/* SCREEN MODULE DEFINITION */

static	short		num_screen_modules = 0;	/* the total number of screen modules	*/
aScreenModule		*screenModules[MAX_SMs];/* SCREEN MODULE DEFINITION ARRAY	*/


/* Start the screen module abstraction.							*/
void
screenModulesStart()
{
	/* the screen modules are lazily started */
}


/* Finish the screen module abstraction.						*/
void
screenModulesFinish()
{
	while (num_screen_modules)
	{/* fire down the last screen module */
		(screenModules[--num_screen_modules]->finish)();
		screenModules[num_screen_modules] = (aScreenModule *) 0;
	}
}


/* Return the slot for a screen module.  Install it if necessary.			*/
short
getScreenModuleIndex(aScreenModule *sm)
{
register short		i;			/* the current screen module index	*/

	for (i = 0; i < num_screen_modules; i++)
	{/* search for the screen module index */
		if (screenModules[i] == sm)
		{/* this has been installed before */
			return (i);
		}
	}

	if (num_screen_modules == MAX_SMs)
	{/* too many have been seen */
		die_with_message("The %s screen module could not be installed.  Too many screen modules.", sm->name);
		return (UNUSED);	/* makes lint happy */
	}
	else
	{/* install a new screen module, start it, and return the index */
		screenModules[i = num_screen_modules++] = sm;
		(sm->start)();	/* the screen module may ask for itself (don't recurse) */
		return (i);
	}
}


/* Standard start of a screen module.							*/
void
standardStart()
{
}


/* Standard finish of a screen module.							*/
void
standardFinish()
{
}


/* Propagate changes in a subwindow in the case of there not being any subwindows.	*/
/*ARGSUSED*/
RectList
standardNoSubWindowPropagate(Pane *p, Window *w, RectList rl)
{
	/* Note: this procedure will be called if the pane p is a slave pane of a pane	*/
	/* which can contain windows.  Since there is no way to ask p to redraw only a 	*/
	/* portion of the area, it must redraw the whole thing.  Oh well...		*/
	if (NOT(emptyRectList(rl)))
	{/* we are required to touch something */
		freeRectList(&rl);
		resizePane(p, p->position, p->size);
		touchPane(p);
	}
	return (rl);
}


/* Propagate changes in a pane's subwindow to the parent window.  Return what couldn't	*/
/* be figured out.									*/
RectList
standardPropagate(Pane *p, Window *w, RectList rl)
{
register struct	rect_node *rn;			/* the current rectangle node		*/
register Window		*cw;			/* the child window being checked	*/
RectList		touched;		/* the list we have touched		*/

	for (cw = p->child[FRST_NEXT]; cw && NOT(emptyRectList(rl)); cw = cw->sibling[FRST_NEXT])
	{/* check the window cw */
		if (cw->exists && cw->showing)
		{/* work on this window */
			touched = partitionRectList(cw->border, &rl);
			if ((w == NULL_WINDOW) || (w == cw))
			{/* we should propagate the image based on what we can */
				for (rn = touched; rn; rn = rn->next)
				{/* check each rectangle to propagate */
					BLTColorCopy(cw->image, p->parent->image, subRect(rn->r, cw->border.ul), transPoint(rn->r.ul, p->position), NULL_BITMAP, Origin /* ignored */, false);
				}
				touchPaneRectList(p, touched);
			}
			else
			{/* don't propagate the image */
				freeRectList(&touched);
			}
		}
	}

	if (p->pattern != NULL_BITMAP && NOT(emptyRectList(rl)))
	{/* the remaining areas gets the pattern */
		for (rn = rl; rn; rn = rn->next)
		{/* cover each rectangle in the list */
			ColorPaneWithPattern(p, rn->r, p->pattern, Origin, false);
		}
		touchPaneRectList(p, rl);
		initializeRectList(&rl);
	}

	return (rl);
}


/* Destroy a pane with no local information.						*/
/*ARGSUSED*/
void
standardDestroyPane(Pane *p)
{
}


/* Tile a window for a screen module which does not want a window.			*/
/*ARGSUSED*/
void
standardTileNoWindow(Window *w, Generic info, Boolean New)
{
	die_with_message("Tiling a window is not appropriate.");
}


/* Destroy a window with no local information.						*/
/*ARGSUSED*/
void
standardDestroyWindow(Window *w)
{
}


/* Attach son to father as first (or last, depending on dir) child of father.		*/
static
void
attach_pane(Window *father, Pane *son, short dir)
{
	son->parent = father;
	son->sibling[OTHER_DIR(dir)] = NULL_PANE;
	if (father->child[dir])
	{/* attach to appropriate sibling */
		son->sibling[dir] = father->child[dir];
		father->child[dir]->sibling[OTHER_DIR(dir)] = son;
	}
	else
	{/* attach as the only child */
		son->sibling[dir] = NULL_PANE;
		father->child[OTHER_DIR(dir)] = son;
	}
	father->child[dir] = son;
}


/* Attach slave to owner as first (or last, depending on dir) slave of owner.		*/
static
void
attach_slave_pane(Pane *par, Pane *slave, short dir)
{
	slave->owner = par;
	slave->sibling[OTHER_DIR(dir)] = NULL_PANE;
	if (par->slave_child[dir])
	{/* attach to appropriate sibling */
		slave->sibling[dir] = par->slave_child[dir];
		par->slave_child[dir]->sibling[OTHER_DIR(dir)] = slave;
	}
	else
	{/* attach as the only child */
		slave->sibling[dir] = NULL_PANE;
		par->slave_child[OTHER_DIR(dir)] = slave;
	}
	par->slave_child[dir] = slave;
}


/* Attach son to father as first (or last, depending on dir) child of father.		*/
static
void
attach_window(Pane *father, Window *son, short dir)
{
	son->parent = father;
	son->sibling[OTHER_DIR(dir)] = NULL_WINDOW;
	if (father->child[dir])
	{/* attach to appropriate sibling */
		son->sibling[dir] = father->child[dir];
		father->child[dir]->sibling[OTHER_DIR(dir)] = son;
	}
	else
	{/* attach as the only child */
		son->sibling[dir] = NULL_WINDOW;
		father->child[OTHER_DIR(dir)] = son;
	}
	father->child[dir] = son;
}


/* Detach pane from siblings and (parent window or owner).				*/
static
void
detach_pane(Pane *chld)
{
short			dir;			/* arbitrary direction			*/

	for (dir = 0; dir < DIRS; dir++)
	{/* handle a side of the sibling detachment */
		if (chld->sibling[dir])
			chld->sibling[dir]->sibling[OTHER_DIR(dir)] = chld->sibling[OTHER_DIR(dir)];
		else if (chld->owner == NULL_PANE)  /* not slave pane */
			chld->parent->child[OTHER_DIR(dir)] = chld->sibling[OTHER_DIR(dir)];
		else  /* slave pane */
			chld->owner->slave_child[OTHER_DIR(dir)] = chld->sibling[OTHER_DIR(dir)];
	}
	chld->sibling[FRST_NEXT] = NULL_PANE;
	chld->sibling[LAST_PREV] = NULL_PANE;
	chld->parent = NULL_WINDOW;
	chld->owner = NULL_PANE;
}


/* Detach 'son' window from father pane and siblings.					*/
static
void
detach_window(Window *son)
{
short			dir;			/* arbitrary direction			*/

	for (dir = 0; dir < DIRS; dir++)
	{/* handle a side of the sibling detachment */
		if (son->sibling[dir])
			son->sibling[dir]->sibling[OTHER_DIR(dir)] = son->sibling[OTHER_DIR(dir)];
		else
			son->parent->child[OTHER_DIR(dir)] = son->sibling[OTHER_DIR(dir)];
	}
	son->sibling[FRST_NEXT] = NULL_WINDOW;
	son->sibling[LAST_PREV] = NULL_WINDOW;
	son->parent = NULL_PANE;
}


/* Create and install a new pane within window 'w' and with screen module 'scr_mod'.	*/
Pane *
newPane(Window *w, short scr_mod_num, Point position, Point size, short border)
{
	return newColorPane(w, scr_mod_num, position, size, border,
			    NULL_COLOR, NULL_COLOR, NULL_COLOR);
}


/* Create and install a new pane within window 'w' and with screen module 'scr_mod'.	*/
Pane *
newColorPane(Window *w, short scr_mod_num, Point position, Point size, 
             short border, Color fg, Color bg, Color bc)
{
Pane			*p;			/* the new pane				*/

	p = (Pane *) get_mem(sizeof(Pane), "PANE:  newColorPane in scr_mod.c");
	attach_pane(w, p, LAST_PREV);
	p->child[FRST_NEXT] = NULL_WINDOW;
	p->child[LAST_PREV] = NULL_WINDOW;
	p->slave_child[FRST_NEXT] = NULL_PANE;
	p->slave_child[LAST_PREV] = NULL_PANE;
	p->position         = position;
	p->size             = size;
	p->border_width     = border;
	p->foreground	    = fg;
	p->background	    = bg;
	p->border_color     = bc;
	p->e_fg		    = (fg == NULL_COLOR) ?
				(w ? w->e_fg : default_foreground_color) : fg;
	p->e_bg		    = (bg == NULL_COLOR) ?
				(w ? w->e_bg : default_background_color) : bg;
	p->e_bc		    = (bc == NULL_COLOR) ?
				(w ? w->e_bc : default_border_color) : bc;
	p->cursor           = (w->parent) ? w->parent->cursor : standard_cursor;
	p->pattern	    = NULL_BITMAP;
	p->scr_mod_num      = scr_mod_num;
	p->owner            = NULL_PANE;
	(screenModules[scr_mod_num]->create)(p);
	return p;
}


/* Create a new slave pane within window 'w' and with screen module 'scr_mod'.		*/
Pane *
newSlavePane(Pane *owner, short scr_mod_num, Point position, Point size, short border)
{
	return newColorSlavePane(owner, scr_mod_num, position, size, border,
				 NULL_COLOR, NULL_COLOR, NULL_COLOR);
}


/* Create a new colored slave pane within window 'w' and with screen module 'scr_mod'.	*/
Pane *
newColorSlavePane(Pane *owner, short scr_mod_num, Point position, Point size, 
                  short border, Color fg, Color bg, Color bc)
{
Pane			*p;			/* the new pane				*/

	p = (Pane *) get_mem(sizeof(Pane), "PANE:  newSlavePane in scr_mod.c");
	p->parent             = owner->parent;
	attach_slave_pane(owner, p, LAST_PREV);
	p->child[FRST_NEXT]   = NULL_WINDOW;
	p->child[LAST_PREV]   = NULL_WINDOW;
	p->slave_child[FRST_NEXT]   = NULL_PANE;
	p->slave_child[LAST_PREV]   = NULL_PANE;
	p->position           = position;
	p->size               = size;
	p->border_width       = border;
	p->foreground	      = fg;
	p->background	      = bg;
	p->border_color	      = bc;
	p->e_fg		      = (fg == NULL_COLOR) ? owner->e_fg : fg;
	p->e_bg		      = (bg == NULL_COLOR) ? owner->e_bg : bg;
	p->e_bc		      = (bc == NULL_COLOR) ? owner->e_bc : bc;
	p->cursor             = owner->cursor;
	p->pattern	      = NULL_BITMAP;
	p->scr_mod_num        = scr_mod_num;
	(screenModules[scr_mod_num]->create)(p);
	return p;
}


/* Resize and/or reposition a pane.							*/
void
resizePane(Pane *p, Point position, Point size)
{
	p->position = position;
	p->size = size;
	if (p->parent->exists)
	{/* Cover the area and redraw.  Avoid touching the border. */
		if (p->pattern != NULL_BITMAP)
			ColorPaneWithPattern(p, makeRectFromSize(Origin, p->size),
					     p->pattern, Origin, false);
		(screenModules[p->scr_mod_num]->resize)(p);
		boxColor(p->parent->image, makeRectFromSize(p->position, p->size),
			 p->border_width, NULL_BITMAP, p->position, UnusedRect, false,
			 default_border_color, NULL_COLOR);
	}
}


/* Change a pane's default colors.							*/
void
recolorPane(Pane *p, Color fg, Color bg, Color bc, Boolean invert, 
            Boolean useParent, Boolean display)
{
Window                  *parent = p->parent;    /* parent of current pane               */
Window			*cw;			/* one of pane's child windows		*/
Pane			*sp;			/* the current slave pane		*/

        if (useParent)
          {
                if (parent)  {
                        p->foreground = NULL_COLOR;
                        p->background = NULL_COLOR;
                        p->border_color = NULL_COLOR;
                }
                else  {
                        p->foreground = default_foreground_color;
                        p->background = default_background_color;
                        p->border_color = default_border_color;
                }
          }
        else if (fg != NULL_COLOR && bg != NULL_COLOR && bc != NULL_COLOR)
          {
                p->foreground = fg;
                p->background = bg;
                p->border_color = bc;
          }

        if (invert)
          {
                Color tmp = p->foreground;
                p->foreground = p->background;
                p->background = tmp;
                p->border_color = tmp;
          }

	p->e_fg = (p->foreground == NULL_COLOR) ? 
	            (parent ? parent->e_fg : default_foreground_color) :
		      p->foreground;
	p->e_bg = (p->background == NULL_COLOR) ? 
	            (parent ? parent->e_bg : default_background_color) :
		      p->background;
	p->e_bc = (p->border_color == NULL_COLOR) ? 
	            (parent ? parent->e_bc : default_border_color) :
		      p->border_color;

	if (display)
	  {
		resizePane(p, p->position, p->size);
		touchPane(p);
	  }

	/* Change descendants' inherited colors.					*/
	for (cw = p->child[FRST_NEXT]; cw; cw = cw ->sibling[FRST_NEXT])
		updateWindowColors(cw);
	for (sp = p->slave_child[FRST_NEXT]; sp; sp = sp->sibling[FRST_NEXT])
		updateSlavePaneColors(sp);

}


/* Update the effective color fields in a pane for inherited colors.  Update these	*/
/* fields for all descendants, including slave panes.					*/
static
void
updatePaneColors(Pane *p)
{
Window			*parent = p->parent;	/* parent of current pane               */
Window			*cw;			/* one of pane's child windows		*/
Pane			*sp;			/* the current slave pane		*/
Boolean			inheritedColor = false;	/* true if should change descendants	*/

	if (p->foreground == NULL_COLOR)
	{
		p->e_fg = parent ? parent->e_fg : default_foreground_color;
		inheritedColor = true;
	}
	if (p->background == NULL_COLOR)
	{
		p->e_bg = parent ? parent->e_bg : default_background_color;
		inheritedColor = true;
	}
	if (p->border_color == NULL_COLOR)
	{
		p->e_bc = parent ? parent->e_bc : default_border_color;
		inheritedColor = true;
	}

	if (inheritedColor == true)
	{	/* Change child windows' and slave panes' colors.			*/
		for (cw = p->child[FRST_NEXT]; cw; cw = cw->sibling[FRST_NEXT])
			updateWindowColors(cw);
		for (sp = p->slave_child[FRST_NEXT]; sp; sp = sp->sibling[FRST_NEXT])
			updateSlavePaneColors(sp);
	}

}


/* Update the effective color fields in a slave pane for inherited colors.  Update these*/
/* fields for all descendants, including slave panes.					*/
static
void
updateSlavePaneColors(Pane *p)
{
Pane			*owner = p->owner;	/* owner of current pane               */
Window			*cw;			/* one of pane's child windows		*/
Pane			*sp;			/* the current slave pane		*/
Boolean			inheritedColor = false;	/* true if should change descendants	*/

	if (p->foreground == NULL_COLOR)
	{
		p->e_fg = owner ? owner->e_fg : default_foreground_color;
		inheritedColor = true;
	}
	if (p->background == NULL_COLOR)
	{
		p->e_bg = owner ? owner->e_bg : default_background_color;
		inheritedColor = true;
	}
	if (p->border_color == NULL_COLOR)
	{
		p->e_bc = owner ? owner->e_bc : default_border_color;
		inheritedColor = true;
	}

	if (inheritedColor == true)
	{	/* Change child windows' and slave panes' colors.			*/
		for (cw = p->child[FRST_NEXT]; cw; cw = cw->sibling[FRST_NEXT])
			updateWindowColors(cw);
		for (sp = p->slave_child[FRST_NEXT]; sp; sp = sp->sibling[FRST_NEXT])
			updateSlavePaneColors(sp);
	}

}


/* Draw the windows in a pane from bottom to top into the parent window image.		*/
void
drawWindowsInPane(Pane *p)
{
Rectangle		area;			/* the area inside the border		*/
Window			*cw;			/* the child window being checked	*/
Rectangle		aoi;			/* the intersection of the window & pane*/

	area = makeRectFromSize(Origin, p->size);
	clipRectWithBorder(area, area, p->border_width);
	for (cw = p->child[LAST_PREV]; cw; cw = cw->sibling[LAST_PREV])
	{/* check the window cw */
		if (cw->exists && cw->showing)
		{/* this window should be considered */
			aoi = interRect(area, cw->border);
			if (positiveArea(aoi))
			{/* the window cw should be considered */
				BLTColorCopy(cw->image, p->parent->image, subRect(aoi, cw->border.ul), transPoint(aoi.ul, p->position), NULL_BITMAP, Origin, false);
			}
		}
	}
}


/* Propagate image through a slave pane from the master.				*/
RectList
propagateSlavePane(Pane *p, Window *w, RectList rl)
{
Point			adjust;			/* the amount to adjust each rectangle	*/
RectList		damage;			/* the list of damaged inside the border*/
Rectangle		r;			/* the current rectangle		*/

	adjust = subPoint(p->owner->position, p->position);

	/* adjust and purge the rectangle list to the slave's coordinates */
		transRectList(rl,adjust);
		r = makeRectFromSize(Origin, p->size);
		rl = interRectList(r, rl);
		damage = partitionRectList(clipRectWithBorder(r, r, p->border_width), &rl);
		while (NOT(emptyRectList(rl)))
		{/* fix the damaged border */
			r = popRectList(&rl);
			ColorPaneWithPatternColor(p, r, NULL_BITMAP, r.ul, false, paneBorderColor(p), NULL_COLOR);
		}

	/* make the slave propagate call if necessary */
		if (emptyRectList(damage))
		{/* quit the propagation here */
			return (damage);
		}
		damage = (screenModules[p->scr_mod_num]->propagate_change)(p, w, damage);

	/* adjust and purge the rectangle list back to the owner's coordinates */
		subRectList(damage, adjust);
		r = makeRectFromSize(Origin, p->owner->size);
		r = clipRectWithBorder(r, r, p->owner->border_width);
		damage = interRectList(r, damage);

	return (damage);
}


/* Destroy the pane p from its screen module.						*/
void
destroyPane(Pane *p)
{
Window			*w;			/* the current child window		*/

	(screenModules[p->scr_mod_num]->destroy)(p);
	while ((w = p->child[LAST_PREV]))
	{/* destroy the current end window */
		w->exists = false;	/* don't bother redrawing */
		destroyWindow(w);
	}
	if (p->owner == NULL_PANE)
	{/* this pane is not a slave--detach it */
		detach_pane(p);
	}
	else  /* Pane is a slave pane.  Detach it. */
		detach_pane(p);
	free_mem((void*) p);
}


/* Handle new inputs to an arbitrary pane.						*/
void
handlePane(Pane *p)
{
Point			adjust;			/* the amount to adjust the information	*/
MouseCursor		save_cursor;		/* the last cursor			*/
Pane			*current;		/* the current pane of adjustment	*/

	adjust = subPoint(Origin, mon_event.offset);
	for (current = p; current; current = current->parent->parent)
	{/* adjust through all parent panes and windows */
		adjust = transPoint(adjust, transPoint(current->position, current->parent->border.ul));
	}
	mon_event.loc    = subPoint  (mon_event.loc,    adjust);
	mon_event.offset = transPoint(mon_event.offset, adjust);
	save_cursor = CURSOR(p->cursor);
	(screenModules[p->scr_mod_num]->input)(p, visiblePaneRect(p, mon_event.loc));
	(void) CURSOR(save_cursor);
	mon_event.offset = subPoint  (mon_event.offset, adjust);
	mon_event.loc    = transPoint(mon_event.loc,    adjust);
	if ((mon_event.type != EVENT_SIGCHLD) && (mon_event.type != EVENT_IO))
	{/* probably a pane-associated event */
		mon_event.from   = (Generic) p;
	}
}


/* Return pane's foreground color.							*/
Color
paneForeground(Pane *p)
{
	return p->e_fg;
}


/* Return pane's background color.							*/
Color
paneBackground(Pane *p)
{
	return p->e_bg;
}


/* Return pane's border color.								*/
Color
paneBorderColor(Pane *p)
{
	return p->e_bc;
}


/* Create a new window with parent pane 'p' and border 'border'.			*/
Window *
createWindow(Pane *p, Generic info)
{
	return createColorWindow(p, info, NULL_COLOR, NULL_COLOR, NULL_COLOR);
}


/* Create a new window with parent pane 'p' and border 'border'.			*/
Window *
createColorWindow(Pane *p, Generic info, Color fg, Color bg, Color bc)
{
Window			*w;			/* the new window			*/
Pane			*sp;			/* the current subpane			*/

	w = (Window *) get_mem(sizeof(Window), "WINDOW:  createColorWindow in scr_mod.c");
	attach_window(p, w, FRST_NEXT);
	w->child[FRST_NEXT] = NULL_PANE;
	w->child[LAST_PREV] = NULL_PANE;
	w->border_width     = 0;
        w->foreground	    = fg;
        w->background	    = bg;
        w->border_color     = bc;
        w->e_fg		    = (fg == NULL_COLOR) ?
	  			(p ? p->e_fg : default_foreground_color) : fg;
        w->e_bg		    = (bg == NULL_COLOR) ?
	  			(p ? p->e_bg : default_background_color) : bg;
        w->e_bc		    = (bc == NULL_COLOR) ?
				(p ? p->e_bc : default_border_color) : bc;
	w->exists	    = false;
	w->showing          = false;
	(screenModules[p->scr_mod_num]->window_tile)(w, info, true);
	w->image  = makePixmap(sizeRect(w->border), "scr_mod.c:  NEW WINDOW PIXMAP");
	w->exists = true;
	boxColor(w->image, relocRect(w->border, Origin), w->border_width, NULL_BITMAP, Origin, UnusedRect, false, windowBorderColor(w), NULL_COLOR);
	for (sp = w->child[FRST_NEXT]; sp; sp = sp->sibling[FRST_NEXT])
	{/* redraw the panes in their correct position */
		resizePane(sp, sp->position, sp->size);
	}
	return (w);
}


/* Show a hidden window w.								*/
void
showWindow(Window *w)
{
	if (NOT(w->showing))
	{/* show the window */
		w->showing = true;
		drawWindow(w);
	}
}


/* Hide a showing window w.								*/
void
hideWindow(Window *w)
{
	if (w->showing)
	{/* show the window */
		w->showing = false;
		damagedPaneRectList(w->parent, singletonRectList(w->border));
	}
}


/* Modify (retile) a window that already exists.					*/
void
modifyWindow(Window *w, Generic info)
{
Pane			*sp;			/* the current subpane			*/
RectList		rl;			/* the damaged area list		*/
Rectangle		old;			/* the old window border		*/

	old = w->border;
	w->exists = false;
	freePixmap(w->image);
	(screenModules[w->parent->scr_mod_num]->window_tile)(w, info, false);
	w->image  = makePixmap(sizeRect(w->border), "scr_mod.c:  NEW WINDOW PIXMAP");
	w->exists = true;
	boxColor(w->image, relocRect(w->border, Origin), w->border_width, NULL_BITMAP, Origin, UnusedRect, false, windowBorderColor(w), NULL_COLOR);
	for (sp = w->child[FRST_NEXT]; sp; sp = sp->sibling[FRST_NEXT])
	{/* redraw each of the boxes and panes */
		resizePane(sp, sp->position, sp->size);
	}
	if (w->showing)
	{/* repair the screen */
		rl = singletonRectList(old);
		drawWindow(w);
		removeFromRectList(w->border, &rl);
		damagedPaneRectList(w->parent, rl);
	}
}


/* Change a window's default colors.							*/
void
recolorWindow(Window *w, Color fg, Color bg, Color bc, Boolean invert, Boolean useParent)
{
Pane                    *parent = w->parent;    /* parent of current window             */
Pane			*cp;			/* the current subpane			*/

        if (useParent)
          {
                if (parent)  {
                        w->foreground = NULL_COLOR;
                        w->background = NULL_COLOR;
                        w->border_color = NULL_COLOR;
                }
                else
                  die_with_message("changeWindowColor(): parentless window %#x", w);
          }
        else
          {
                w->foreground = fg;
                w->background = bg;
                w->border_color = bc;
          }

        if (invert)
          {
                Color tmp = w->foreground;
                w->foreground = w->background;
                w->background = tmp;
                w->border_color = tmp;
          }

	w->e_fg = (w->foreground == NULL_COLOR) ? 
	            (parent ? parent->e_fg : default_foreground_color) :
		      w->foreground;
	w->e_bg = (w->background == NULL_COLOR) ? 
	            (parent ? parent->e_bg : default_background_color) :
		      w->background;
	w->e_bc = (w->border_color == NULL_COLOR) ? 
	            (parent ? parent->e_bc : default_border_color) :
		      w->border_color;

	/* Change descendants' inherited colors.					*/
	for (cp = w->child[FRST_NEXT]; cp; cp = cp->sibling[FRST_NEXT])
		updatePaneColors(cp);

}


/* Update the effective color fields in a window for inherited colors.  Update these	*/
/* fields for all descendants, including slave panes.					*/
static
void
updateWindowColors(Window *w)
{
Pane			*parent = w->parent;	/* parent of current pane               */
Pane			*cp;			/* one of pane's child windows		*/
Boolean			inheritedColor = false;	/* true if should change descendants	*/

	if (w->foreground == NULL_COLOR)
	{
		w->e_fg = parent ? parent->e_fg : default_foreground_color;
		inheritedColor = true;
	}
	if (w->background == NULL_COLOR)
	{
		w->e_bg = parent ? parent->e_bg : default_background_color;
		inheritedColor = true;
	}
	if (w->border_color == NULL_COLOR)
	{
		w->e_bc = parent ? parent->e_bc : default_border_color;
		inheritedColor = true;
	}

	if (inheritedColor == true)
		/* Change child panes' colors.						*/
		for (cp = w->child[FRST_NEXT]; cp; cp = cp->sibling[FRST_NEXT])
			updatePaneColors(cp);

}


/* Destroy the window pointed to by w.  Destroy lower structures and redraw.		*/
void
destroyWindow(Window *w)
{
Pane			*parent;		/* the parent pane of the window	*/
Pane			*p;			/* the current child pane		*/

	(screenModules[w->parent->scr_mod_num]->window_destroy)(w);
 	while ((p = w->child[LAST_PREV]))
	{/* destroy the current end pane */
		destroyPane(p);
	}
	parent = w->parent;
	detach_window(w);

	if (w->exists && w->showing)
	{/* rebuild the screen in this area */
		damagedPaneRectList(parent, singletonRectList(w->border));
	}

	freePixmap(w->image);
	free_mem((void*) w);
}


/* Move window 'w' to the top or bottom of the window list (based on 'dir') & redisplay.*/
void
toEndWindow(Window *w, short dir)
{
Pane			*p = w->parent;		/* the parent pane of w			*/

	detach_window(w);
	attach_window(p, w, dir);
	if (w->showing)
	{/* repair the screen */
		drawWindow(w);
	}
}


/* Return window's foreground color.							*/
Color
windowForeground(Window *w)
{
	return w->e_fg;
}


/* Return window's background color.							*/
Color
windowBackground(Window *w)
{
	return w->e_bg;
}


/* Return window's border color.							*/
Color
windowBorderColor(Window *w)
{
	return w->e_bc;
}


	/* GRAPHICS ROUTINES */

/* Copy a region from source pane to destination modulo a bitmap.				*/
void
BLTColorCopyPane(Pane *src, Pane *dst, Rectangle srcArea, Point dstOrigin, 
                 Bitmap m, Point offset, Boolean clip)
{
Point			rel;
Rectangle		dstArea;

	if (dst->parent->exists)
	  {
		rel     = subPoint(dstOrigin, srcArea.ul);
		dstArea = transRect(srcArea, rel);
		dstArea = clipWithInsides(dst, dstArea);
		srcArea = subRect(dstArea, rel);

		BLTColorCopy(src->parent->image, dst->parent->image,
			     transRect(srcArea, dst->position),
			     transPoint(dstOrigin, dst->position),
			     m, offset, clip);
	  }
	else
		die_with_message("BLTColorCopyPane(): destination pane has no parent!");

}


/* Cover an area with a pattern, using pane's default colors.	*/
void
ColorPaneWithPattern(Pane *dst, Rectangle dstArea, Bitmap pattern, Point offset, 
                     Boolean clip)
{
	ColorPaneWithPatternColor(dst, dstArea, pattern, offset, clip,
				  NULL_COLOR, NULL_COLOR);
}


/* Cover an area with a pattern.	*/
void
ColorPaneWithPatternColor(Pane *dst, Rectangle dstArea, Bitmap pattern, 
                          Point offset, Boolean clip, Color foreground, Color background)
{
Rectangle		insides;

	if (dst->parent->exists)
	  {
		/* figure the inside of the pane (pane coords) */
			insides = clipRectWithBorder(
				makeRectFromSize(Origin, dst->size),
				makeRectFromSize(Origin, dst->size),
				dst->border_width
			);
	
		/* figure the area of damage in pane coordinates */
			dstArea = interRect(dstArea, insides);

		if (foreground == NULL_COLOR && background == NULL_COLOR)
			ColorWithPattern(dst->parent->image, transRect(dstArea, dst->position),
					 pattern, transPoint(offset, dst->position), clip,
					 paneForeground(dst), paneBackground(dst));
		else
			ColorWithPattern(dst->parent->image, transRect(dstArea, dst->position),
					 pattern, transPoint(offset, dst->position), clip,
					 foreground, background);
	  }

}


/* Draw a ghost box on the top window based on a rectangle in a pane.			*/
void
ghostBoxInPane(Pane *p, Rectangle r, short width, Boolean clip, Boolean redisplay)
{
Window			*w;			/* the current window			*/

	if (p->parent->exists)
		{
			invertBox(p->parent->image, transRect(r, p->position),
				  width, clip);
			if (redisplay && p->parent->image != screen_pixmap)
			  {
				drawWindow(p->parent);
			  }
				
	        }
}


/* Obtain a Bitmap (bit plane) from pane.							*/
Bitmap
flattenPane(Pane *p)
{
	return flattenPaneColor(p, paneForeground(p), paneBackground(p));
}


/* Obtain a Bitmap (bit plane) from pane.							*/
Bitmap
flattenPaneColor(Pane *p, Color foreground, Color background)
{
	if (p->parent->exists)
		if (foreground == NULL_COLOR && background == NULL_COLOR)
			return flattenPixmap(p->parent->image,
					     makeRectFromSize(Origin, p->size),
					     paneForeground(p), paneBackground(p));
		else
			return flattenPixmap(p->parent->image,
					     makeRectFromSize(Origin, p->size),
					     foreground, background);
	else
		die_with_message("flattenPane(): destination pane has no parent!");
}


/* Flash a pane momentarily.								*/
void
flashPane(Pane *p)
{
	invertPane(p, makeRectFromSize(Origin, p->size), false);
	touchPane(p);
	/* insert delay here if wanted */
	invertPane(p, makeRectFromSize(Origin, p->size), false);
	touchPane(p);
}


/* Draw a point in a pane, clip on pane border.  Return affected area.			*/
Rectangle
pointPane(Pane *p, Point p1, Rectangle clipper)
{
	return pointPaneColor(p, p1, clipper, NULL_COLOR);
}


/* Draw a point in a pane, clip on pane border.  Return affected area.			*/
Rectangle
pointPaneColor(Pane *p, Point p1, Rectangle clipper, Color c)
{
Rectangle		insides;		/* the usable pane area (pane coords)	*/
Rectangle		box;			/* the affected rectangle (pane coords)	*/

	/* figure the inside of the pane (pane coords) */
		insides = clipRectWithBorder(
			makeRectFromSize(Origin, p->size),
			makeRectFromSize(Origin, p->size),
			p->border_width
		);

	/* Figure the area of damage in pane coordinates */
		box = interRect(makeRect(p1, p1), insides);

	/* do the work */
		pointColor(
			p->parent->image,
			transPoint(p1, p->position),
			transRect(interRect(insides, clipper), p->position),
			true,
			c == NULL_COLOR ? paneForeground(p) : c
		);

	return interRect(box, clipper);
}


/* Draw a set of points in a pane, clip on pane border.  Return affected area.		*/
Rectangle
polyPointPane(Pane *p, Point origin, short n, Point *pp, Rectangle clipper)
{
	return polyPointPaneColor(p, origin, n, pp, clipper, NULL_COLOR);
}


/* Draw a set of points in a pane, clip on pane border.  Return affected area.		*/
Rectangle
polyPointPaneColor(Pane *p, Point origin, short n, Point *pp, Rectangle clipper, Color c)
{
Rectangle		insides;		/* the usable pane area (pane coords)	*/
Rectangle		box;			/* the affected rectangle (pane coords)	*/
short			i;			/* point list index			*/

	/* figure the inside of the pane (pane coords) */
		insides = clipRectWithBorder(
			makeRectFromSize(Origin, p->size),
			makeRectFromSize(Origin, p->size),
			p->border_width
		);

	/* Figure the area of damage in pane coordinates */
		if (n == 0)
		{/* no points--bogus box */
			box = makeRectFromSize(insides.ul, Origin);
		}
		else
		{/* there are points--add from the first one */
			box = makeRect(pp[0], pp[0]);
			for (i = 1; i < n; i++)
			{/* add this point's influence to the bounding box */
				box = unionRect(box, makeRect(pp[i], pp[i]));
			}
			box = interRect(transRect(box, origin), insides);
		}

	/* do the work */
		polyColorPoint(
			p->parent->image,
			transPoint(origin, p->position),
			n,
			pp,
			transRect(interRect(insides, clipper), p->position),
			true,
			c == NULL_COLOR ? paneForeground(p) : c
		);

	return interRect(box, clipper);
}


/* Draw a line within the pane.  Return the affected rectangle.				*/
Rectangle
linePane(Pane *p, Point p1, Point p2, short width, LineStyle *style)
{
	return linePaneColor(p, p1, p2, width, style, NULL_COLOR);
}


/* Draw a line within the pane.  Return the affected rectangle.				*/
Rectangle
linePaneColor(Pane *p, Point p1, Point p2, short width, LineStyle *style, Color c)
{
	return linePaneColorClipped(p, p1, p2, width, style, c, UnusedRect);
}


/* Draw a line within the pane.  Return the affected rectangle.				*/
Rectangle
linePaneColorClipped(Pane *p, Point p1, Point p2, short width, LineStyle *style, 
                     Color c, Rectangle clipper)
{
Rectangle		insides;		/* the usable pane area (pane coords)	*/
Rectangle		box;			/* the affected rectangle (pane coords)	*/
short			slop;			/* conservative border around min. box	*/

	/* figure the inside of the pane (pane coords) */
		insides = clipRectWithBorder(
			makeRectFromSize(Origin, p->size),
			makeRectFromSize(Origin, p->size),
			p->border_width
		);

	/* Figure the area of damage in pane coordinates */
		slop = width << 1;	/* wide lines go outside the minimal box */
		box = makeRect(makePoint(MIN(p1.x, p2.x) - slop, MIN(p1.y, p2.y) - slop),
			       makePoint(MAX(p1.x, p2.x) + slop, MAX(p1.y, p2.y) + slop));
		box = interRect(box, insides);
		if (NOT(rectEq(clipper, UnusedRect)))
			box = interRect(box, clipper);

	/* do the work */
		lineColor(
			p->parent->image,
			transPoint(p1, p->position),
			transPoint(p2, p->position),
			width,
			style,
			transRect(box, p->position),
			true,
			c == NULL_COLOR ? paneForeground(p) : c
		);

	return box;
}


/* Draw a box with possibly an empty inside.						*/
void
boxPane(Pane *dst, Rectangle r, short width, Bitmap pattern, Point offset, Boolean clip)
{
	boxPaneColor(dst, r, width, pattern, offset, clip, NULL_COLOR, NULL_COLOR);
}


/* Draw a box with possibly an empty inside.						*/
void
boxPaneColor(Pane *dst, Rectangle r, short width, Bitmap pattern, Point offset, 
             Boolean clip, Color foreground, Color background)
{
Rectangle	clipper;
Boolean		useDefaults;

	if (dst->parent->exists)
	  {
		clipper = clipWithInsides(dst, r);
		useDefaults = BOOL(foreground == NULL_COLOR && background == NULL_COLOR);
		boxColor(dst->parent->image,
			 transRect(r, dst->position),
			 width,
			 pattern,
			 transPoint(offset, dst->position),
			 transRect(clipper, dst->position),
			 true,
			 (useDefaults ? paneForeground(dst) : foreground),
			 (useDefaults ? paneBackground(dst) : background));
	  }
	else
		die_with_message("boxPane(): pane has no parent!");
}


/* Draw a set of lines in a pane, clip on pane border.  Return affected area.		*/
Rectangle
polyLinePane(Pane *p, Point origin, short n, Point *pp, short width, LineStyle *style)
{
	return polyLinePaneColor(p, origin, n, pp, width, style, NULL_COLOR);
}


/* Draw a set of lines in a pane, clip on pane border.  Return affected area.		*/
Rectangle
polyLinePaneColor(Pane *p, Point origin, short n, Point *pp, short width, 
                  LineStyle *style, Color c)
{
Rectangle		insides;		/* the usable pane area (pane coords)	*/
Rectangle		box;			/* the affected rectangle (pane coords)	*/
short			i;			/* point list index			*/
short			slop;			/* conservative border around min. box	*/

	/* figure the inside of the pane (pane coords) */
		insides = clipRectWithBorder(
			makeRectFromSize(Origin, p->size),
			makeRectFromSize(Origin, p->size),
			p->border_width
		);

	/* Figure the area of damage in pane coordinates */
		if (n == 0)
		{/* no points--bogus box */
			box = makeRectFromSize(insides.ul, Origin);
		}
		else
		{/* there are points--add from the first one */
			box = makeRect(pp[0], pp[0]);
			for (i = 1; i < n; i++)
			{/* add this point's influence to the bounding box */
				box = unionRect(box, makeRect(pp[i], pp[i]));
			}
			slop = width << 1;	/* wide lines go outside the minimal box */
			box.ul.x -= slop;
			box.ul.y -= slop;
			box.lr.x += slop;
			box.lr.y += slop;
			box = interRect(transRect(box, origin), insides);
		}

	/* do the work */
		polyColorLine(
			p->parent->image,
			transPoint(origin, p->position),
			n,
			pp,
			width,
			style,
			transRect(insides, p->position),
			true,
			c == NULL_COLOR ? paneForeground(p) : c
		);

	return box;
}


/* Cover a polygon in a pane, clip on pane border.  Return affected area.		*/
Rectangle
polygonPane(Pane *p, Point origin, short n, Point *pp, Bitmap pattern, 
            Point offset, Rectangle clipper)
{
	return polygonPaneColor(p, origin, n, pp, pattern, offset, clipper,
				NULL_COLOR, NULL_COLOR);
}


/* Cover a polygon in a pane, clip on pane border.  Return affected area.		*/
Rectangle
polygonPaneColor(Pane *p, Point origin, short n, Point *pp, Bitmap pattern, 
                 Point offset, Rectangle clipper, Color fg, Color bg)
{
Rectangle		insides;		/* the usable pane area (pane coords)	*/
Rectangle		box;			/* the affected rectangle (pane coords)	*/
short			i;			/* point list index			*/

	/* figure the inside of the pane (pane coords) */
		insides = clipRectWithBorder(
			makeRectFromSize(Origin, p->size),
			makeRectFromSize(Origin, p->size),
			p->border_width
		);

	/* Figure the area of damage in pane coordinates */
		if (n == 0)
		{/* no points--bogus box */
			box = makeRectFromSize(insides.ul, Origin);
		}
		else
		{/* there are points--add from the first one */
			box = makeRect(pp[0], pp[0]);
			for (i = 1; i < n; i++)
			{/* add this point's influence to the bounding box */
				box = unionRect(box, makeRect(pp[i], pp[i]));
			}
			box = interRect(transRect(box, origin), insides);
		}

	/* do the work */
		polygonColor(
			p->parent->image,
			transPoint(origin, p->position),
			n,
			pp,
			pattern,
			subPoint(offset, p->position),
			transRect(interRect(clipper, insides), p->position),
			true,
			(fg == NULL_COLOR && bg == NULL_COLOR) ? paneForeground(p) : fg,
			(fg == NULL_COLOR && bg == NULL_COLOR) ? paneBackground(p) : bg
		);

	return interRect(box, clipper);
}


/* Draw an arc in a pane, clip on pane border.  Return affected area.			*/
Rectangle
arcLinePane(Pane *p, Rectangle *r, double a1, double a2, short width, LineStyle *style)
{
	return arcLinePaneColor(p, r, a1, a2, width, style, NULL_COLOR);
}


/* Draw an arc in a pane, clip on pane border.  Return affected area.			*/
Rectangle
arcLinePaneColor(Pane *p, Rectangle *r, double a1, double a2, short width, 
                 LineStyle *style, Color c)
{
	return arcLinePaneColorClipped(p, r, a1, a2, width, style, c, UnusedRect);
}


/* Draw an arc in a pane, clip on pane border.  Return affected area.			*/
Rectangle
arcLinePaneColorClipped(Pane *p, Rectangle *r, double a1, double a2, short width, 
                        LineStyle *style, Color c, Rectangle clipper)
{
Rectangle		insides;		/* the usable pane area (pane coords)	*/
Rectangle		box;			/* the affected rectangle (pane coords)	*/
short			slop;			/* conservative border around min. box	*/
Rectangle               temp_rect;              /* temporary */

	/* figure the inside of the pane (pane coords) */
		insides = clipRectWithBorder(
			makeRectFromSize(Origin, p->size),
			makeRectFromSize(Origin, p->size),
			p->border_width
		);

	/* Figure the area of damage in pane coordinates */
		box = *r; 		/* not a smart estimate... */
		slop = width << 1;	/* wide lines go outside the minimal box */
		box.ul.x -= slop;
		box.ul.y -= slop;
		box.lr.x += slop;
		box.lr.y += slop;
		box = interRect(box, insides);
		if (NOT(rectEq(clipper, UnusedRect)))
			box = interRect(box, clipper);

	/* do the work */
                temp_rect = transRect(*r, p->position);

		arcLine( 
			p->parent->image,
			&temp_rect, 
			a1,
			a2,
			width,
			style,
			transRect(box, p->position),
			true,
			c == NULL_COLOR ? paneForeground(p) : c
		);

	return box;
}


/* Pattern an arc in a pane, clip on pane border.  Return affected area.		*/
Rectangle
arcCoverPane(Pane *p, Rectangle *r, double a1, double a2, Bitmap pattern, Point offset)
{
	return arcCoverPaneColor(p, r, a1, a2, pattern, offset, NULL_COLOR, NULL_COLOR);
}


/* Pattern an arc in a pane, clip on pane border.  Return affected area.		*/
Rectangle
arcCoverPaneColor(Pane *p, Rectangle *r, double a1, double a2, Bitmap pattern, 
                  Point offset, Color fg, Color bg)
{
Rectangle		insides;		/* the usable pane area (pane coords)	*/
Rectangle		box;			/* the affected rectangle (pane coords)	*/

	/* figure the inside of the pane (pane coords) */
		insides = clipRectWithBorder(
			makeRectFromSize(Origin, p->size),
			makeRectFromSize(Origin, p->size),
			p->border_width
		);

	/* Figure the area of damage in pane coordinates */
		box = interRect(*r, insides);	/* r is not a smart estimate... */

	/* do the work */
                {
                Rectangle dummy = transRect(*r, p->position);
		arcColor(
			p->parent->image,
			&dummy,
			a1,
			a2,
			pattern,
			subPoint(offset, p->position),
			transRect(insides, p->position),
			true,
			(fg == NULL_COLOR && bg == NULL_COLOR) ? paneForeground(p) : fg,
			(fg == NULL_COLOR && bg == NULL_COLOR) ? paneBackground(p) : bg
		);
	        }

	return box;
}


/* Figure a visible rectangle of a pane which includes the point.			*/
/* Note:  currently this function works if there is no possibility of the pane or any	*/
/* or its ancestors being in a window that is not the top window.  To properly handle	*/
/* this case, the region of interest must be broken up into smaller regions as defined	*/
/* by taking away subrectangles of the intercection.  Tackling the harder problem	*/
/* requires a point to help decide which regions are of interest.  Currently the more	*/
/* difficult problem is not solved but all of the hooks are here.			*/
static
Rectangle
visiblePaneRect(Pane *p, Point pt)
{
Point			offset;			/* the accumulated offset so far	*/
Point			adjust;			/* the current offset			*/
Rectangle		r;			/* the rectangle of interest 		*/

	offset = Origin;
	r = makeRectFromSize(Origin, p->size);
	while (p->parent->parent)
	{/* figure the clip of the enclosing window and pane */
		adjust = transPoint(p->position, p->parent->border.ul);
		offset = subPoint(offset, adjust);
		pt = transPoint(pt, adjust);
		r = transRect(r, adjust);
		r = clipRectWithBorder(r, p->parent->border, p->parent->border_width);
		r = clipRectWithBorder(r, makeRectFromSize(Origin, p->parent->parent->size), 0);
		p = p->parent->parent;
	}
	return (transRect(r, offset));
}


/* Propagate a change in an entire pane.						*/
void
touchPane(Pane *p)
{
	touchPaneRectList(p, singletonRectList(makeRectFromSize(Origin, p->size)));
}


/* Propagate changes in a region in a pane.						*/
void
touchPaneRect(Pane *p, Rectangle r)
{
	touchPaneRectList(p, singletonRectList(r));
}


/* Propagate a list of changes in a pane.						*/
void
touchPaneRectList(Pane *p, RectList rl)
{
Window			*w = p->parent;		/* the parent window			*/
Rectangle		r;			/* the current rectangle		*/

	if (w->exists && w->showing && w->parent && w->parent->parent->exists)
	{/* propagate the changes up one level */
		/* translate to the coordinates of w's parent */
			transRectList(rl, transPoint(p->position, w->border.ul));
			r = makeRectFromSize(Origin, w->parent->size);
			r = clipRectWithBorder(r, r, w->parent->border_width);
			rl = interRectList(r, rl);

		/* translate to the end-all owner pane */
			for (p = w->parent; p->owner; p = p->owner)
			{/* switch to owner coordinates and owner pane */
				transRectList(rl, subPoint(p->position, p->owner->position));
			}

		if (NOT(emptyRectList(rl)))
		{/* there is no more work to do */
			rl = (screenModules[p->scr_mod_num]->propagate_change)(p, w, rl);
		}
	}
	freeRectList(&rl);
}


/* Write a text string to a pixmap postion in a given font.				*/
void
fontWritePane(Pane *dst, Point p, Rectangle clip, TextString ts, short font)
{
	fontWritePaneColor(dst, p, clip, ts, font, NULL_COLOR, NULL_COLOR);
}


/* Write a text string to a pixmap postion in a given font.				*/
void
fontWritePaneColor(Pane *dst, Point p, Rectangle clip, TextString ts, short font, 
                   Color fg, Color bg)
{
	if (dst->parent->exists)
	  {
		clip = clipWithInsides(dst, clip);

		if (fg == NULL_COLOR && bg == NULL_COLOR)
			fontColorWrite(dst->parent->image,
				       transPoint(p, dst->position), transRect(clip, dst->position),
				       ts, font, paneForeground(dst), paneBackground(dst));
		else
			fontColorWrite(dst->parent->image,
				       transPoint(p, dst->position), transRect(clip, dst->position),
				       ts, font, fg, bg);
	  }
	else
		die_with_message("fontWritePane(): attempt to write to NULL Pane");
}


/* Move the window in the parent pane & redraw.						*/
void
moveWindow(Window *w)
{
Rectangle		mouse_area;		/* the region of mouse tracking		*/
Rectangle		ghost_border;		/* the border of the ghost rectangle	*/
MouseCursor		save_cursor;		/* the saved cursor			*/
Point			last_point;		/* the previous coordinate		*/
Pane		*root_pane;		/* the root pane		*/

    /* find the root pane */
	for (root_pane = w->parent; root_pane->parent->parent; root_pane = root_pane->parent->parent)
        	;

	mouse_area   = makeRectFromSize(Origin, w->parent->size);
	save_cursor  = CURSOR(moving_cursor);
	ghost_border = w->border;
	ghostBoxInPane(root_pane, ghost_border, 2, true, true);
	do
	{/* run the mouse and the box */
		last_point  = mon_event.loc;
		getEvent();
		ghostBoxInPane(root_pane, ghost_border, 2, true, false);
		ghost_border = transRect(ghost_border, subPoint(mon_event.loc, last_point));
		ghostBoxInPane(root_pane, ghost_border, 2, true, true);
	} while ((mon_event.type == MOUSE_DRAG) && pointInRect(mon_event.loc, mouse_area));
	ghostBoxInPane(root_pane, ghost_border, 2, true, true);
	(void) CURSOR(save_cursor);

	if (mon_event.type != MOUSE_DOWN)
	{/* make the move & save the event for the caller */
		moveWindowTo(w, ghost_border.ul);
	}
}


/* Make a window move to a new position and redraw it.					*/
void
moveWindowTo(Window *w, Point pt)
{
RectList		rl;			/* damaged rectangle list 		*/
Rectangle		old;			/* the old border			*/

	old = w->border;
	w->border  = relocRect(w->border, pt);
	if (w->showing)
	{/* fix the window damage */
		drawWindow(w);
		rl = singletonRectList(old);
		removeFromRectList(w->border, &rl);
		damagedPaneRectList(w->parent, rl);
	}
}


void
invertPane(Pane *p, Rectangle dstArea, Boolean clip)
{
	if (p->parent->exists)
	  {
		dstArea = clipWithInsides(p, dstArea);
		invertPixmap(p->parent->image, transRect(dstArea, p->position), clip);
	  }
}


/* Redraw (based on the current state) a damaged portion of a pane.			*/
static
void
damagedPaneRectList(Pane *p, RectList rl)
{
RectList		damage;			/* the damaged area inside the border	*/
Rectangle		r;			/* the current rectangle		*/

	if (p->parent->exists)
	{/* there is work to do */
		r = makeRectFromSize(Origin, p->size);
		rl = interRectList(r, rl);

		/* calculate & fix the edge damage */
			r = clipRectWithBorder(r, r, p->border_width);
			damage = partitionRectList(r, &rl);
			while(NOT(emptyRectList(rl)))
			{/* fix the edges */
				r = popRectList(&rl);
				ColorPaneWithPatternColor(p, r, NULL_BITMAP, Origin, false, paneBorderColor(p), NULL_COLOR);
			}
			touchPaneRectList(p, rl);

		/* fix the pane damage */
			while (p->owner)
			{/* switch to owner coordinates and owner pane (no further clipping is necessary) */
				transRectList(damage, subPoint(p->position, p->owner->position));
				p = p->owner;
			}
			damage = (screenModules[p->scr_mod_num]->propagate_change)(p, NULL_WINDOW, damage);
	}
	freeRectList(&damage);
}


/* Redisplay an entire visible window.							*/
static
void
drawWindow(Window *w)
{
Rectangle		r;			/* the showing area of the window	*/
RectList		rl;			/* the un delt-with rectangle list	*/

	r = clipRectWithBorder(w->border, makeRectFromSize(Origin, w->parent->size), w->parent->border_width);
	if (positiveArea(r) && w->parent->parent->exists)
	{/* we have a window to draw */
		rl = singletonRectList(r);
		rl = (screenModules[w->parent->scr_mod_num]->propagate_change)(w->parent, NULL_WINDOW, rl);
		freeRectList(&rl);
	}
}


/* Clip a rectangle to the inside of a pane */
static
Rectangle
clipWithInsides(Pane *p, Rectangle rect)
{
Rectangle		all, insides, clippedRect;

	all = makeRectFromSize(Origin, p->size);

	if (rectEq(rect, UnusedRect))
		clippedRect = all;
	else
	  {
		insides = clipRectWithBorder(all, all, p->border_width);
		clippedRect = interRect(rect, insides);
	  }

	return clippedRect;
}
