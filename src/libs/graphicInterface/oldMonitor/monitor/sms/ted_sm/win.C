/* $Id: win.C,v 1.1 1997/06/25 14:59:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <include/bstring.h>
#include <libs/graphicInterface/oldMonitor/monitor/sms/ted_sm/tedprivate.h>
#include <libs/graphicInterface/oldMonitor/monitor/sms/ted_sm/damage.h>

#define PAGE_AMOUNT .75

void sm_ted_win_create_internal(Pane *p)
{
	TedWin *win = (TedWin *)get_mem( sizeof(TedWin), "ted_win_create" );

	bzero((char *)win,sizeof(TedWin));
	WIN(p) = win;
	BMS(p) = bm_create( (BMS_private *)0 );

	/* win is the only existing window into this buffer */
	win -> topskip		= 0;
	win -> botskip		= 0;
	win -> page_fraction	= PAGE_AMOUNT;
	win -> prefer_col	= 0;
	win -> tsize		= 8;
	win -> ctrlstyle	= OCTAL;
	win -> style		= STYLE_NORMAL;
	win -> font		= DEF_FONT_ID;
	win -> bot_scrline	= 0;
	win -> dot_loc		= Origin;
	win -> old_dot_loc	= Origin;
	win -> dot_offset	= 0;
	win -> dot_line		= 0;
	win -> old_dot_offset	= 0;
	win -> multiplier	= 1;
	win -> active		= true;
	RPATTERN(p)		= ssave("");
	RLENGTH(p)		= 0;
	FRD(p)			= NULL;

	MESG_HANDLER(p) = NO_MESG_HANDLER;
	OWNER_ID(p)     = UNUSED;
	
	MY_PANE(p)	= p;
	OP(p)		= newSlavePane(p, SM_TED_OPTIM_INDEX, p->position, p->size, p->border_width);
	sm_optim_set_move_status(OP(p), false, true);
	SIZE(p)		= Origin;
	OPTIM_LINE(p)	= sm_optim_alloc_line( OP(p) );

	VERT(p)		= (ScrollBar) UNUSED;
	HORZ(p)		= (ScrollBar) UNUSED;

	WIN_OWNER(p)	= util_node_alloc((Generic)win,"sm_ted_win_create");
	DAMAGE(p)	= util_list_alloc((Generic)win, "sm_ted_win_create");
	sm_ted_default_bindings( p );

	EVENT_MAP(p) = keymap_create( 0 );
	keymap_bind_KbChar( EVENT_MAP(p), (Generic)sm_ted_dot_to_click,	toKbChar(EVENT_SELECT) );
	keymap_bind_KbChar( EVENT_MAP(p), (Generic)sm_ted_scroll,	toKbChar(EVENT_MOVE)   );
/*	keymap_bind_KbChar( EVENT_MAP(p), (Generic)sm_ted_print_info,	toKbChar(EVENT_HELP)   );
*/
	util_push( win->owner, WIN_LIST(p) );
}

void sm_ted_win_create(Pane *p)
{
	TedWin *owin = WIN(p);

	sm_ted_win_create_internal(p);

	if ( owin != NULL )
	{
		/* there is already a window provided for initialization, just use its info */

		TOPSKIP(p)	= owin -> topskip;
		BOTSKIP(p)	= owin -> botskip;
		PREFER(p)	= owin -> prefer_col;
		TAB_SIZE(p)	= owin -> tsize;
		CTRL_STYLE(p)	= owin -> ctrlstyle;
		STYLE(p)	= owin -> style;
		FONT(p)		= owin -> font;
		LASTLINE(p)	= owin -> bot_scrline;
		DOT_LOC(p)	= owin -> dot_loc;
		OLD_DOT_LOC(p)	= owin -> dot_loc;
		DOT(p)		= owin -> dot_offset;
		DOTLINE(p)	= owin -> dot_line;
		OLD_DOT(p)	= owin -> old_dot_offset;
		MULTIPLIER(p)	= owin -> multiplier;
		ACTIVE(p)	= owin -> active;
		sfree(RPATTERN(p));
		RPATTERN(p)	= ssave (owin->rpattern);
		RLENGTH(p)	= strlen (RPATTERN(p));
		FRD(p)		= owin -> frd;
		bm_newpattern (BMS(p), bm_pattern (owin->bms), strlen (bm_pattern(owin->bms)) );
		if (!bm_is_forward (owin->bms))
			(void) bm_toggle_dir (BMS(p));
		if (bm_is_casefolded (owin->bms))
			(void) bm_toggle_case_fold (BMS(p));
	}
	sm_optim_change_font(OP(p),FONT(p));
	sm_ted_resize_internal(p);
}

void sm_ted_win_destroy(Pane *p)
{
	sm_optim_free_line( OPTIM_LINE(p) );
	destroyPane(OP(p));		/* Destroy the optim slave pane */

	sfree (RPATTERN(p));
	if (FRD(p) != 0)
		find_dialog_destroy(FRD(p));
	util_pluck(WIN_OWNER(p));
	util_free_node(WIN_OWNER(p));
	util_free_nodes_and_atoms(DAMAGE(p));
	util_list_free(DAMAGE(p));
	
	bm_destroy( BMS(p) );

	keymap_destroy( KEY_MAP(p) );
	keymap_destroy( EVENT_MAP(p) );

	free_mem( (void*)WIN(p) ) ;
}


Generic sm_ted_get_win(Pane *p)
{
	return (Generic) WIN(p);
}

void sm_ted_set_win(Pane *p, Generic win)
{
	WIN(p) = (TedWin *)win;
}


/*
 * Perform a font change.  Pass the font to optim,
 * with actual touching of the pane inhibited.
 * Then refigure the contents of the ted pane, adding
 * appropriate damage.
 */
void sm_ted_win_change_font(Pane *p, short font)
{
	FONT(p) = font;
	sm_optim_resizing(OP(p),true);
	sm_optim_change_font(OP(p),font);
	sm_optim_resizing(OP(p),false);
	sm_ted_resize_internal(p);
}

/*
 * Perform a style change.
 */
void sm_ted_win_set_style(Pane *p, char style)
{
	STYLE(p) = style;
	sm_ted_damaged_win(p);
}

unsigned char sm_ted_win_get_style(Pane *p)
{
	return STYLE(p);
}

/*
 * Attempt to place the dot at screen location loc.
 */
void sm_ted_win_set_xy(Pane *p, Point loc)
{
	short height = sm_optim_height(OP(p)) - 1;

	/* We need to have DOT() and DOT_LOC() consistent. Sorry */
	sm_ted_repair(p);

	loc.y = (loc.y >  height - BOTSKIP(p)) ?
			height - BOTSKIP(p) :
			( (loc.y < TOPSKIP(p)) ? TOPSKIP(p) : loc.y ) ;
	DOT_LOC(p).y += sm_ted_goto_line_rel (p, loc.y - DOT_LOC(p).y);

	PREFER(p) = loc.x;
	sm_ted_damaged_dot_col (p);
}

/*
 * Return the location of the dot in the pane
 */
Point sm_ted_win_get_xy(Pane *p)
{
	return DOT_LOC(p);
}

void sm_ted_win_active(Pane *p, Boolean status)
{
	ACTIVE(p) = status;
	if (VERT(p) != (ScrollBar) UNUSED)
		sm_scroll_activate(VERT(p),status);
	if (HORZ(p) != (ScrollBar) UNUSED)
		sm_scroll_activate(HORZ(p),status);
	sm_ted_damaged_win(p);
}

Generic sm_ted_win_get_owner(Pane *p)
{
	UtilList *list = util_list( WIN_OWNER(p) );

	if (list == NULLLIST)
		return NULL;
	else
		return util_list_atom(list);
}


/*
 * sm_ted_win_set_owner - sets buf to be the owner of win.
 */
void sm_ted_win_set_owner(Pane *p)
{
	if (util_list(WIN_OWNER(p)) == NULLLIST)
		util_pluck(WIN_OWNER(p));

	util_push(WIN_OWNER(p),WIN_LIST(p));
	sm_ted_damaged_win (p);
}


char sm_ted_win_get_char(register Pane *p)
{
	return flex1_get_one (STORE(p), DOT(p));
}

/*
 * adjust_offset - Adjust an offset based on an insertion/deletion
 *	Affected region that we are adjusting for is:
 *		(amt >=0 => [at .. at+amt]) ( amt < 0 => [at+amt .. at])
 */
static int adjust_offset(int old, int at, int amt)
{
	int New;	/* The new offset. */

	if (amt >= 0)
	{/* amt is non-negative => we are adjusting for an insertion of amt bytes */
		if (at < old)
		{/* The change made requires that we adjust "old" */
			New = old + amt;
		}
		else
		{/* old is unaffected, since the insertion was at a greater offset */
			New = old;
		}
	}
	else
	{/* amt is negative => we are adjusting for a deletion of amt bytes */
		if (old < at + amt)
		{/* old is unaffected, since all of the deletion was at a greater offset */
			New = old;
		}
		else if (at < old)
		{/* old was beyond the deletion area, so adjustment is simple */
			New = old + amt;
		}
		else
		{/* old was within the deleted region, so it collapses to the low end of the region */
			New = at + amt;
		}
	}

	return New;
}

/*
 * sm_ted_win_adjust - adjust all the offsets that are associated with the TedWin of "p",
 *	to correct for an insertion of "elts" elements at location "at", and an insertion
 *	of "lines" new lines at location "at".
 *	Note that a deletion is represented by a negative value of "elts", and a possibly
 *	negative value of "lines".
 */
void sm_ted_win_adjust(Pane *p, int at, int elts)
{
	TedWin *save_win;
	WinChange *damage;
	UtilNode *wnode;
	UtilNode *dnode;
	TedMark *marks;
	int i;
	
	save_win = WIN(p);

	wnode = util_head(WIN_LIST(p));
	while (wnode != NULL)
	{
		WIN(p) = (TedWin *)util_node_atom(wnode);

		sm_ted_set_dot( p, adjust_offset (DOT(p), at, elts) );
		OLD_DOT(p) = adjust_offset (OLD_DOT(p), at, elts);
		
		marks = (TedMark *) (Generic) flex_get_buffer (MARKLIST(p), 0, NUM_MARKS(p), (char *) 0);
		for (i = 0; i < NUM_MARKS(p); i++)
		{
			marks[i].location = adjust_offset (marks[i].location, at, elts);
			flex_set_one (MARKLIST(p), i, (char *) &marks[i]);
		}
		free_mem((void*) marks);
		MARK(p) = adjust_offset (MARK(p), at, elts);	
		
		for (dnode = util_head(DAMAGE(p));dnode != NULLNODE; dnode = util_next(dnode))
		{
			damage = Damage_of(dnode);
			damage->loc = adjust_offset(damage->loc,at,elts);
		}
		wnode = util_next(wnode);
	}

	WIN(p) = save_win;
}


/*
 * TedWin_of - given a node on a list of windows (eg. WIN_LIST(p) ),
 *	return the TedWin associated with it.
 */
TedWin *TedWin_of(UtilNode *node)
{
	return (TedWin *)util_node_atom(node);
}

Point sm_ted_size(Pane *p)
{
	return sm_optim_size(OP(p));
}

Point
sm_ted_pane_size(Point size, short font_id)
{
	return sm_optim_pane_size(size,font_id);
}

void sm_ted_find_dialog_create (Pane *p)
{
	FRD(p) = find_dialog_create("Find","Replace", bm_pattern (BMS(p)), RPATTERN(p), 
				    bm_is_forward(BMS(p)), bm_is_casefolded (BMS(p)), 
				    (find_dialog_finder_func)sm_ted_finder, 
				    (find_dialog_replacer_func) sm_ted_replacer,
				    (find_dialog_global_replacer_func)sm_ted_global_replacer, (Generic) p);
}


