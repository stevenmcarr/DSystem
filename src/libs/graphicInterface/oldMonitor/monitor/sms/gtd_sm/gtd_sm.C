/* $Id: gtd_sm.C,v 1.2 2001/09/17 00:44:29 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * gtd_sm.c:			Generalized tree display screen module.	 
 */

#include <stdlib.h>

#include <libs/graphicInterface/oldMonitor/include/mon/sm_def.h>
#include <libs/graphicInterface/oldMonitor/include/sms/gtd_sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/optim_sm.h>
#include <string.h>

#ifdef SOLARIS
EXTERN(char*,index,(const char *,char));
#endif


#include <libs/graphicInterface/oldMonitor/monitor/sms/gtd_sm/gtd_inc.h>

typedef FUNCTION_POINTER(int,GetNodeNumberFunc,(char *));

/* declare the screen module */
STATIC(void,	gtd_start, (void));
STATIC(void,    gtd_create, (Pane *p));
STATIC(void,    gtd_resize, (Pane *p));
STATIC(void,    gtd_destroy, (Pane *p));
STATIC(void,    gtd_input, (Pane *p, Rectangle r));

STATIC(void, new_shape,(void));
STATIC(void, unparse,(TREE_INDEX baoi, TREE_INDEX eaoi, Generic mode, 
                      TREE_INDEX unp_mode, char StmtType, Boolean how));
STATIC(Generic, get_unparse_info,(TREE_INDEX p, char *type, Generic *line, Boolean how));
STATIC(void, unparse_internal,(TREE_INDEX p, Generic line, char StmtType, Generic k));
STATIC(void, display_half,(struct PrintInfo P, char forward));
STATIC(void, display_rest,(struct PrintInfo P, char forward));
STATIC(void, display_statement,(struct PrintInfo P, char forward, Generic start_line));
STATIC(void, display_bracket,(struct PrintInfo P, char forward, Generic start_line));
STATIC(Boolean, should_print,(struct PrintInfo P));
STATIC(void, check_cursor,(void));
STATIC(void, uncheck_cursor,(void));
STATIC(struct PrintInfo, PI_In,(struct PrintInfo P));
STATIC(struct PrintInfo, PI_In_To_End,(struct PrintInfo P));
STATIC(struct PrintInfo, PI_Out,(struct PrintInfo P));
STATIC(struct PrintInfo, PI_Next,(struct PrintInfo P));
STATIC(struct PrintInfo, PI_Prev,(struct PrintInfo P));
STATIC(struct PrintInfo, PI_Succ,(struct PrintInfo P, char forward));
STATIC(Generic, tree_depth,(TREE_INDEX p));
STATIC(Boolean, is_highlighted,(TREE_INDEX p, int pDepth));
STATIC(Boolean, WithinCursor,(TREE_INDEX p));
STATIC(Boolean, part_highlighted,(TREE_INDEX p));
STATIC(Boolean, is_near_cursor,(TREE_INDEX p, int pLevel));
STATIC(Boolean, AroundCursor,(TREE_INDEX p, int n));
STATIC(Boolean, OnPathToRoot,(TREE_INDEX p));
STATIC(Boolean, NodeIsMarked,(TREE_INDEX p));
STATIC(void, mark_out,(TREE_INDEX p, Generic incr));
STATIC(void, display_open,(TREE_INDEX p, Generic indent, char highlight, 
                           Generic toskip));
STATIC(void, display_close,(TREE_INDEX p, Generic indent, char highlight, 
                            Generic toskip));
STATIC(void, display_simple,(TREE_INDEX p, Generic indent, Boolean highlight, 
                           Generic toskip));
STATIC(void, display_ellipsis,(TREE_INDEX p, Generic indent, Boolean highlight));
STATIC(void, display,(TREE_INDEX p, Generic indent, Boolean highlight, char type,
                      char **table, Generic skip));
STATIC(void, toggle_underline,(void));
STATIC(void, start_highlight,(void));
STATIC(void, end_highlight,(void));
STATIC(void, map_start,(TREE_INDEX p, Generic indent, Generic toskip, int type));
STATIC(void, map_fmt,(TREE_INDEX p, char c));
STATIC(void, map_shuffle,(void));
STATIC(void, line_swap,(Generic i, Generic j));
STATIC(void, do_unparse,(TREE_INDEX p, char *s));
STATIC(char*, get_format,(char **table, TREE_INDEX p));
STATIC(char*, get_predicate,(char *s, char *pred));
STATIC(Boolean, eval_predicate,(char *name, TREE_INDEX p));
STATIC(char*, eval_str_func,(char *name, TREE_INDEX p));
STATIC(TREE_INDEX, xy_to_node,(Point loc));
STATIC(Generic, find_row_data,(int line));
STATIC(Generic, find_col_data,(Generic line, Generic col));
STATIC(void, clear_line_data,(void));
STATIC(void, clear_line_data_line,(Generic lines));
STATIC(void, do_string,(TREE_INDEX p, char *s));
STATIC(void, map_string,(char c));
STATIC(Generic, getnum_from_menu,(char *str));
STATIC(void, free_internals,(void));
STATIC(void, draw_screen,(void));
STATIC(void, set_node,(int Y, int X, int C, int P, int STYLE));

static aScreenModule scr_mod_gtd =
{
 "gtd",
 gtd_start,
 standardFinish,
 gtd_create,
 gtd_resize,
 standardNoSubWindowPropagate,
 gtd_destroy,
 gtd_input,
 standardTileNoWindow,
 standardDestroyWindow
};

/* global storage for this screen module */
static short	optim_sm;	/* the optim screen module index	 */
char            string_result[MAX_TEXT_LENGTH];
Generic         string_result_length;
extern char    *Rn_gtd_dirs[];

# define INLEV    0
# define OUTLEV   1
# define TOPLEV   2
# define SAMELEV  3
# define NUMHOLO  4

# define IMPLICIT true
# define EXPLICIT false

char           *menu[] = {"set In level", "set Out level", "set Top level", "set Same level"};

/*
 * Start the gtd screen module.
 */
static
void
gtd_start ()
{
    optim_sm = sm_optim_get_index();
}


/*
 * Set the default status for this pane: non-inverted and no buttons returned.		 
 */
static
void
gtd_create (Pane *p)
{
    p->pane_information = (Generic) get_mem (sizeof (struct gtd_info), "gtd_sm/gtd_sm.c: pane information structure");
    OP (p) = newSlavePane (p, optim_sm, p->position, p->size, p->border_width);
    SIZE (p) = makePoint (UNUSED, UNUSED);
    TDI (p) = 0;
    sm_optim_set_move_status (OP (p), false /* MOVE_HORIZ */ , true /* MOVE_VERT */ );
    p->border_width = 0;
}


/* Resize a gtd pane.									 */
static
void
gtd_resize (Pane *p)
{
    gtdi = TDI (p);
    gtdip = p;

    resizePane (OP (p), p->position, p->size);

    if (!equalPoint (SIZE (p), sm_optim_size (OP (p))))
    {				/* the lower pane has changed size */
	/* I need to unparse again for this size.... */
	SIZE (p) = sm_optim_size (OP (p));
	/* only if we have been initialized */
	if (gtdi)
	    new_shape ();
    }
}

/* Destroy the pane and all structures below it.					 */
static
void
gtd_destroy (Pane *p)
{
    gtdi = TDI (p);
    gtdip = p;

    destroyPane (OP (p));
    /* free the instance variable and local storage for the gtd */

    free_internals ();
    free_mem ((void*) gtdi);
    free_mem ((void*) p->pane_information);
}


/* Handle input to the gtd screen module.						 */
static
void
gtd_input (Pane *p, Rectangle r)
{
    TREE_INDEX      root,oroot;
    char	    StmtType;

    gtdi = TDI (p);
    gtdip = p;

    while ((mon_event.type < MOUSE_KEYBOARD) && pointInRect (mon_event.loc, r))
    {				/* we can handle this event */
	handlePane (OP (p));
	if (mon_event.type == EVENT_SELECT)
	{			/* a potential select event--convert from text to gtd */

	    /*
	     * mon_event.info is the location of the selected char, map this to a node in the tree 
	     */
	    mon_event.msg = (Generic) xy_to_node (mon_event.info);
	    break;
	}
	else
	if (mon_event.type == EVENT_MOVE)
	{			/* a potential move event */
	    switch (mon_event.msg)
	    {
		    case 0:	/* normal */
		        sm_gtd_shift (p, mon_event.info.y);
		        return;
		    case 1:	/* top */
		        oroot = _OUT(gtdi->baoi);   /* walk out to the root */
			root  = gtdi->baoi;
			while (oroot != NIL)
			{
			    root = oroot;
		            oroot = _OUT(root);
			}

			oroot = root;		   /* walk prev is possible */
			while (oroot != NIL)
			{
			    root = oroot;
		            oroot = _PREV(root);
			}			

	    		StmtType = _IS_SIMPLE (root) ? SIMPLE : OPEN;
			sm_gtd_modified_extended(p, gtdi->baoi, gtdi->eaoi, GTD_TOP,    root,       StmtType);
			return;
		    case 2:	/* bottom */
		        oroot = _OUT(gtdi->baoi);	/* walk out to the root */
			root  = gtdi->baoi;
			while (oroot != NIL)
			{
			    root = oroot;
		            oroot = _OUT(root);
			}

			oroot = root;		   /* walk next is possible */
			while (oroot != NIL)
			{
			    root = oroot;
		            oroot = _NEXT(root);
			}

			/* move to the bottom of the pane */
	    		StmtType = _IS_SIMPLE (root) ? SIMPLE : CLOSE;
			sm_gtd_modified_extended(p, gtdi->baoi, gtdi->eaoi, GTD_BOTTOM, root,       StmtType);
			return;
		    case 5:	/* center */
			/* move the cursor to the middle of the pane */
	    		StmtType = _IS_SIMPLE (gtdi->baoi) ? SIMPLE : OPEN;
			sm_gtd_modified_extended(p, gtdi->baoi, gtdi->eaoi, GTD_CENTER, gtdi->baoi, StmtType);
			return;
	     }
	     break;
	    
	}
	else
	if (mon_event.type == EVENT_HELP)
	{
	    mon_event.msg = (Generic) xy_to_node (mon_event.info);
	    break;
	}
    }
}

void
sm_gtd_initialize (Pane *p, Generic owner_id, short font_id,
		   char **open, char **close, char **simple, 
                   struct pred_map *pred_map, Generic fixed_indent,
		   UnpIntFunc In, UnpIntFunc In_to_end, UnpIntFunc Out, UnpIntFunc Next, UnpIntFunc Prev, 
                   UnpIntFunc Is_simple, UnpIntFunc Is_statement,
		   UnpIntFunc Indent_delta, UnpIntFunc Mark_type, UnpIntFunc4 Mark_set, UnpIntFunc Mark_get,
		   UnpIntFunc Get_node_type, UnpIntFunc4 Getson, UnpIntFunc Getfirst, UnpStringFunc Get_text)
{
    struct line_data *thisld;
    Generic         lines,
                    cols,
                    i;
    Point           size;

    OWNER (p) = owner_id;
    sm_optim_change_font (OP (p), font_id);
    TDI (p) = (struct tdi *) get_mem (sizeof (struct tdi), "tdi structure");

    gtdi = TDI (p);

    size = sm_optim_size (OP (p));
    gtdi->pane_length = lines = size.y;
    gtdi->pane_width = cols = size.x;

    gtdi->has_no_data  = false;
    gtdi->is_too_small = BOOL(size.y == 0);

    gtdip = p;

    gtdi->clipping = false;
    gtdi->Nroot = NIL;
    gtdi->open = open;
    gtdi->close = close;
    gtdi->simple = simple;
    gtdi->cursor_depth = 0;
    gtdi->save_baoi = NIL;
    gtdi->save_eaoi = NIL;
    gtdi->cursor_level = 0;
    gtdi->fixed_indent = fixed_indent;
    gtdi->baoi = NIL;
    gtdi->eaoi = NIL;
    gtdi->unp_node = NIL;
    gtdi->stmttype = OPEN;

    gtdi->routines.In = In;
    gtdi->routines.In_to_end = In_to_end;
    gtdi->routines.Out = Out;
    gtdi->routines.Next = Next;
    gtdi->routines.Prev = Prev;
    gtdi->routines.Is_simple = Is_simple;
    gtdi->routines.Is_statement = Is_statement;
    gtdi->routines.Indent_delta = Indent_delta;
    gtdi->routines.Mark_type = Mark_type;
    gtdi->routines.Mark_set = Mark_set;
    gtdi->routines.Mark_get = Mark_get;
    gtdi->routines.Get_node_type = Get_node_type;
    gtdi->routines.Getson = Getson;
    gtdi->routines.Getfirst = Getfirst;
    gtdi->routines.Get_text = Get_text;
    gtdi->pred_map = pred_map;

    gtdi->line_data = thisld = (struct line_data *) get_mem (lines * sizeof (struct line_data), "line_data");

    for (i = 0; i < lines; i++)
    {
	thisld[i].elements = (struct element *) get_mem (cols * sizeof (struct element), "elements");
    }
    gtdi->swap_array = (struct line_data *) get_mem (lines * sizeof (struct line_data), "swap_array");

    /* make sure there is no trash in the screen map */
    clear_line_data ();
}

void
sm_gtd_set_clipping (Pane *p, Boolean clipping)
{
    gtdi = TDI (p);
    gtdip = p;

    gtdi->clipping = clipping;
}

void
sm_gtd_modified (Pane *p, TREE_INDEX baoi, TREE_INDEX eaoi, Generic mode, 
                 TREE_INDEX unp_node)
{
    char	    StmtType;
    gtdi = TDI (p);
    gtdip = p;

    if (unp_node != NIL)
    {
        if (!_IS_STATEMENT (unp_node))
	   unp_node = _OUT (unp_node);

        StmtType = _IS_SIMPLE (unp_node) ? SIMPLE : OPEN;
    }

    unparse (baoi, eaoi, mode, unp_node,StmtType, IMPLICIT);
}


void
sm_gtd_modified_extended (Pane *p, TREE_INDEX baoi, TREE_INDEX eaoi, Generic mode, 
                          TREE_INDEX unp_node, char StmtType)
{
    gtdi = TDI (p);
    gtdip = p;

    if (unp_node != NIL && !_IS_STATEMENT (unp_node))
	unp_node = _OUT (unp_node);

    unparse (baoi, eaoi, mode, unp_node, StmtType, EXPLICIT);
}

void
sm_gtd_shift (Pane *p, Generic yshift)
{
    TREE_INDEX      newpos;
    char            StmtType;
    Generic         k,
                    y,
                    line;

    gtdi = TDI (p);
    gtdip = p;

    /* nothing to shift */
    if (gtdi->has_no_data || gtdi->is_too_small)
	return;

    if (yshift == 0)
	return;

    if (yshift > 0)
    {				/* move up */
	y = find_row_data (gtdi->pane_length - 1);	/* was right button code */
	newpos = xy_to_node (makePoint (0, y));	/* the node at the last line of scr */

	line = y - yshift;
	if (line < 0)
	    line = 0;

	gtdi->unp_node = newpos;
	k = get_which (y) - 1;
	StmtType = get_type (y);
    }
    else
    {
	yshift = -yshift;	/* move down */
	y = find_row_data (0);

	line = y + yshift;	/* was left button code */
	if (line >= gtdi->pane_length)
	    line = gtdi->pane_length - 1;

	newpos = xy_to_node (makePoint (0, y));	/* the node at the first line of scr */
	gtdi->unp_node = newpos;
	k = get_which (y) - 1;
	StmtType = get_type (y);
    }

    sm_optim_clear (OP (gtdip));

    if (!_IS_STATEMENT (newpos))
	newpos = _OUT (newpos);

    clear_line_data ();
    unparse_internal (newpos, line, StmtType, k);
    draw_screen ();

    sm_optim_touch (OP (gtdip));
}

void
sm_gtd_set_holophrasm (Pane *p, Generic inlevel, Generic outlevel, 
                       Generic toplevel, Generic aroundlevel)
{
    gtdi = TDI (p);
    gtdip = p;

    gtdi->TopLevel = toplevel;
    gtdi->InLevel = inlevel;
    gtdi->OutLevel = outlevel;
    gtdi->AroundLevel = aroundlevel;
}

/*
 * open, close, simple should be pointers to array of character strings, the arrays should be large enough to hold the
 * table we are reading in 
 */
Generic
sm_gtd_init_unparse_table (char *table_name, char **open, char **close, 
                           char **simple, GetNodeNumberFunc Get_node_number)
{
    char            s[128];
    char          **table;
    char          **trial_dir;	/* the remaining directory list		 */
    FILE           *input = NULL;	/* the current file descriptor		 */

    for (trial_dir = Rn_gtd_dirs; *trial_dir && !input; trial_dir++)
    {				/* try opening the ith directory */
	(void) strcpy (s, *trial_dir);
	(void) strcat (s, table_name);
	input = fopen (s, "r");
    }

    if (input == NULL)
    {				/* could not open file -- quit */
	(void) sprintf (s, "Gtd table '%s' could not be opened.", table_name);
	message (s);
	return (0);
    }

    while (fgets (s, sizeof (s), input) != NULL)
    {
	s[strlen (s) - 1] = 0;	/* kill the newline */
	if (strcmp (s, "OPEN") == 0)
	    table = open;
	else
	if (strcmp (s, "CLOSE") == 0)
	    table = close;
	else
	if (strcmp (s, "SIMPLE") == 0)
	    table = simple;
	else
	if (strcmp (s, "EXPR") == 0);
	else
	{
	    if (s[0] && s[0] != '#')
	    {
		char           *nodename,
		               *format,
			       *cut;
		Generic         nodenum;

		nodename = s;
		format = (char*)index (s, '"') + 1;
		cut = (char*)index(s, ' ' /* space */);
		if (cut) *cut = 0;
		cut = (char*)index(s, '	' /* tab */);
		if (cut) *cut = 0;
		format[strlen (format) - 1] = 0;	/* kill closing " */
		nodenum = Get_node_number (nodename);
		table[nodenum] = (char *) get_mem (strlen (format) + 1, "unparse string");
		(void) strcpy (table[nodenum], format);
	    }
	}
    }

    (void) fclose (input);
    return 1;
}

void
sm_gtd_query_holophrasm (Pane *p, Generic *in, Generic *out, 
                         Generic *top, Generic *same)
{
    short           sel;
    Generic         temp;

    gtdi = TDI (p);
    gtdip = p;

    sel = menu_select ("Modify parameter", NUMHOLO, menu);

    if (sel == -1)
	return;

    switch (sel)
    {
	case INLEV:
	    temp = getnum_from_menu ("levels to right of cursor");
	    if (temp < 0)
		return;
	    *in = temp;
	    break;
	case OUTLEV:
	    temp = getnum_from_menu ("levels to left of cursor");
	    if (temp < 0)
		return;
	    *out = temp;
	    break;
	case TOPLEV:
	    temp = getnum_from_menu ("levels in from left margin");
	    if (temp < 0)
		return;
	    *top = temp;
	    break;
	case SAMELEV:
	    temp = getnum_from_menu ("stmts in same list as cursor");
	    if (temp < 0)
		return;
	    *same = temp;
	    break;
    }
}

/* Get the index of this screen module.  Install it if necessary.			 */
short
sm_gtd_get_index()
{
    return (getScreenModuleIndex (&scr_mod_gtd));
}

/* Calculate the minimum sized pane to display a character block.			 */
Point
sm_gtd_pane_size (Point size, short font)
{
    return (sm_optim_pane_size (size, font));
}

Generic
sm_gtd_get_stmt_type (Pane *p, Generic line)
{
    gtdi = TDI (p);
    gtdip = p;

    if (line < gtdi->pane_length && line >= 0)
	return get_type (line);
    else return SIMPLE;
}

/* Return the actual size of the map.			 */
Point
sm_gtd_map_size (Pane *p)
{
    return (SIZE (p));
}

void
sm_gtd_string_simple (Pane *p, TREE_INDEX node, char *s)
{
    gtdi = TDI (p);
    gtdip = p;

    string_result_length = 0;
    do_string (node, get_format (gtdi->simple, node));
    (void) strncpy (s, string_result, string_result_length);
    s[string_result_length] = '\0';
}

void
sm_gtd_string_open (Pane *p, TREE_INDEX node, char *s)
{
    gtdi = TDI (p);
    gtdip = p;

    string_result_length = 0;
    do_string (node, get_format (gtdi->open, node));
    (void) strncpy (s, string_result, string_result_length);
    s[string_result_length] = '\0';
}

void
sm_gtd_string_close (Pane *p, TREE_INDEX node, char *s)
{
    gtdi = TDI (p);
    gtdip = p;

    string_result_length = 0;
    do_string (node, get_format (gtdi->close, node));
    (void) strncpy (s, string_result, string_result_length);
    s[string_result_length] = '\0';
}

void
sm_gtd_line_string_simple (Pane *p, TREE_INDEX node, char *s, Generic indent)
{
    gtdi = TDI (p);
    gtdip = p;

    td_line_indent = indent;
    (void) sprintf (string_result, "%*s", indent, "");
    string_result_length = indent;
    do_string (node, get_format (gtdi->simple, node));
    (void) strncpy (s, string_result, string_result_length);
    s[string_result_length] = '\0';
}

void
sm_gtd_line_string_open (Pane *p, TREE_INDEX node, char *s, Generic indent)
{
    gtdi = TDI (p);
    gtdip = p;

    td_line_indent = indent;
    (void) sprintf (string_result, "%*s", indent, "");
    string_result_length = indent;
    do_string (node, get_format (gtdi->open, node));
    (void) strncpy (s, string_result, string_result_length);
    s[string_result_length] = '\0';
}

void
sm_gtd_line_string_close (Pane *p, TREE_INDEX node, char *s, Generic indent)
{
    gtdi = TDI (p);
    gtdip = p;

    td_line_indent = indent;
    (void) sprintf (string_result, "%*s", indent, "");
    string_result_length = indent;
    do_string (node, get_format (gtdi->close, node));
    (void) strncpy (s, string_result, string_result_length);
    s[string_result_length] = '\0';
}

TREE_INDEX
sm_gtd_first (Pane *p, TREE_INDEX *node, Generic *type)
{
    Generic         i;
    struct line_data *thisld;

    gtdi = TDI (p);
    gtdip = p;

    for (i = 0; i < gtdi->pane_length; i++)
    {
	thisld = &(gtdi->line_data[i]);
	if (thisld->topnode != (TREE_INDEX) - 1)
	{
	    *node = thisld->topnode;
	    *type = thisld->type;
	    return (TREE_INDEX)1;
	}
    }
    *node = NIL;
    *type = 0;

	return (TREE_INDEX)0;
}

TREE_INDEX
sm_gtd_last (Pane *p, TREE_INDEX *node, Generic *type)
{
    Generic         i;
    struct line_data *thisld;

    gtdi = TDI (p);
    gtdip = p;

    for (i = gtdi->pane_length - 1; i >= 0; i--)
    {
	thisld = &(gtdi->line_data[i]);
	if (thisld->topnode != (TREE_INDEX) - 1)
	{
	    *node = thisld->topnode;
	    *type = thisld->type;
	    return (TREE_INDEX)1;
	}
    }
    *node = NIL;
    *type = 0;

	return (TREE_INDEX)0;
}

/*
 * Mark node p as one that should be printed in spite of any holophrasm 
 */
void
sm_gtd_mark_node (Pane *p, TREE_INDEX q)
{
    gtdi = TDI (p);
    gtdip = p;

    /*
     * if node is not marked, mark it and increment ref counts out to root 
     */
    if (!NodeIsMarked (q))
    {
	_MARK_SET (q, _MARK_GET (q) + 1);
	mark_out (_OUT (q), 1);
    }
}

/*
 * have node p revert to normal print status 
 */
void
sm_gtd_unmark_node (Pane *p, TREE_INDEX q)
{
    gtdi = TDI (p);
    gtdip = p;

    /*
     * if node is marked, unmark it and decrement ref counts out to root 
     */
    if (NodeIsMarked (q))
    {
	_MARK_SET (q, _MARK_GET (q) - 1);
	mark_out (_OUT (q), -1);
    }
}

/*
 * return marked status of node q 
 */
Boolean
sm_gtd_is_marked (Pane *p, TREE_INDEX q)
{
    gtdi = TDI (p);
    gtdip = p;

    return NodeIsMarked (q);
}

static void
free_line_data ()
{
    Generic         i;
    struct line_data *thisld;

    thisld = gtdi->line_data;

    for (i = 0; i < gtdi->pane_length; i++)
    {
	free_mem ((void*) thisld[i].elements);
    }

    free_mem ((void*) thisld);
}

static void
new_shape ()
{
    struct line_data *thisld;
    Generic           i,
                      lines,
                      cols;

    free_internals ();

    gtdi->pane_length  = lines = SIZE (gtdip).y;
    gtdi->pane_width   = cols = SIZE (gtdip).x;
    gtdi->is_too_small = BOOL(lines == 0);

    gtdi->line_data = thisld = (struct line_data *) get_mem (lines * sizeof (struct line_data), "line_data");

    for (i = 0; i < lines; i++)
	thisld[i].elements = (struct element *) get_mem (cols * sizeof (struct element), "elements");

    gtdi->swap_array = (struct line_data *) get_mem (lines * sizeof (struct line_data), "swap_array");

    clear_line_data ();		/* clean out all cached info */

    /* nothing to find here */
    if (gtdi->has_no_data)
	return;

    /* nothing to find here */
    if (gtdi->is_too_small)
	return;

    sm_optim_resizing (OP (gtdip), true);

    unparse (gtdi->baoi, gtdi->eaoi, GTD_OTHER, gtdi->unp_node, gtdi->stmttype, EXPLICIT);
    sm_optim_resizing (OP (gtdip), false);
}

/*
 * unparse around node p into the pane indicated by *tdi -- ScreenPos encodes where to start printing get some
 * information saved on the previous unparse...find out which line node p is really on.... this k value indicates which
 * line of a multiple statement construction was at line l of the screen. (this is 1 based, so a single line statement
 * always has k = 1). 
 *
 * this l value indicates which line of the screen the cached information was found on... 
 */

static void
unparse (TREE_INDEX baoi, TREE_INDEX eaoi, Generic mode, TREE_INDEX unp_node, 
         char StmtType, Boolean how)
{
    Generic         l,
                    k;

    gtdi->has_no_data = false;

    sm_optim_clear (OP (gtdip));

    /* no tree here to display - just clear the screen */
    if (mode == GTD_CLEAR || baoi == NIL || eaoi == NIL)
    {
        gtdi->has_no_data = true;
	gtdi->baoi        = NIL;
	gtdi->eaoi        = NIL;
	gtdi->save_baoi   = NIL;
	gtdi->save_eaoi   = NIL;
	clear_line_data ();
	draw_screen ();

	sm_optim_touch (OP (gtdip));
	return;
    }

    k = 0;

    if (mode == GTD_TOP)
	l = 0;
    else
    if (mode == GTD_BOTTOM)
	l = gtdi->pane_length - 1;
    else
    if (mode == GTD_CENTER)
	l = gtdi->pane_length / 2;
    else
    if (mode >= 0 && mode < gtdi->pane_length)
	l = mode;

    /*
     * I believe that GTD_CLEAR_OTHER is now obsolete.
     */
    else
    if (mode == GTD_OTHER || mode == GTD_CLEAR_OTHER)

	/*
	 * unparse from a new node without disturbing the screen too much 
	 */
	l = get_unparse_info (unp_node, &StmtType, &k, how);
    else
	die_with_message ("mode value invalid in unparse().");

    gtdi->stmttype = StmtType;
    gtdi->unp_node = unp_node;
    gtdi->baoi     = baoi;
    gtdi->eaoi     = eaoi;

    clear_line_data ();
    unparse_internal (unp_node, l, StmtType, k);
    draw_screen ();
    sm_optim_touch (OP (gtdip));
}

/*
 * look in cached information for the location of node p on the previously unparsed screen. If we cannot find p on the
 * screen, then look for _NEXT(p) and _PREV(p). We want at least one line of p to be visible on the screen, but for now
 * we will not enforce this feature. 
 */
static Generic
get_unparse_info (TREE_INDEX p, char *type, Generic *line, Boolean how)
{
    TREE_INDEX      prevp,
                    nextp;
    Generic         l;
    char	    pStmtType,nStmtType;

    if (how == EXPLICIT) /* look for the correct node, and the correct statement type */
       for (l = 0; l < gtdi->pane_length; l++)
       {
	  if (ld[l].topnode == p && *type == get_type(l))
	  {
	    *line = get_which (l) - 1;
	    return l;
	  }
       }
    else /* just find a line for a statement with the same index */
       for (l = 0; l < gtdi->pane_length; l++)
       {
	  if (ld[l].topnode == p)
	  {
	    *type = get_type(l); 
	    *line = get_which (l) - 1;
	    return l;
	  }
       }

    /* maybe the node before this one is on the screen - find the last line on the screen for the previous node */
    prevp = _PREV (p);
    pStmtType = _IS_SIMPLE (p) ? SIMPLE : CLOSE;
    if (ld[gtdi->pane_length - 1].topnode != prevp)
    {
	for (l = gtdi->pane_length - 2; l >= 0; l--)
	{
	    if (ld[l].topnode == prevp && pStmtType == get_type(l))
	    {
		l = l + 1;
		*line = get_which (l) - 1;
		return l;
	    }
	}
    }

    /* maybe the node after this one is on the screen */
    nextp = _NEXT (p);
    nStmtType = _IS_SIMPLE (p) ? SIMPLE : OPEN;
    if (ld[0].topnode != nextp)
    {
	for (l = 1; l < gtdi->pane_length; l++)
	{
	    if (ld[l].topnode == nextp && nStmtType == get_type(l))
	    {
		l = l - 1;
		*line = get_which (l) - 1;
		return l;
	    }
	}
    }

    /* no luck, just punt this time */
    *line = 0;
    return gtdi->pane_length / 2;
}

/*
 * Determine the display indenting level of a node, by going out to the root. 
 */
static Generic
structure_indent_level (int p)
{
    register Generic level = 0;

    if (!_IS_STATEMENT (p))
	p = _OUT (p);

    while (p = _OUT (p), p != NIL)
    {
	level += _INDENT_DELTA (p);
    }
    return level;
}


static void
unparse_internal (TREE_INDEX p, Generic line, char StmtType, Generic k)
{
    struct PrintInfo P;

    /* do we have a non-zero sized pane to fill with characters ? */
    if (gtdi->pane_length == 0 || gtdi->pane_width == 0)
	return;

    /* check the cursor in */
    check_cursor ();

    /* establish P's position in tree */
    P.ptr = p;
    P.IndentLevel = structure_indent_level (p);
    P.TreeDepth = tree_depth (p);
    P.Highlighted = is_highlighted (p, P.TreeDepth);
    P.NearCursor = is_near_cursor (p, P.IndentLevel);

    /*
     * first unparse bottom of screen in forward direction, when low-level print routines do a longjmp (signifying no
     * more room) unparse top of screen backwards from P 
     */

    if (line > k)
    {
	line = line - k;
	k = 0;
    }
    else
    {
	line = 0;
	k = k - line;
    }

    td_startline = td_curline = line;

    td_curstyle = STYLE_NORMAL;
    if (setjmp (unparse_main) == 0)
    {
	if (StmtType == SIMPLE)
	{
	    td_dir = -1;
            td_curline = td_startline - 1;
	    td_oldcurline = td_curline;
	    if (k > 1)
		display_statement (P, BACKWARD, k - 1);
	    display_half (P, BACKWARD);
	}
	else if (StmtType == OPEN)
	{
	    td_dir = -1;
            td_curline = td_startline - 1;
	    td_oldcurline = td_curline;
	    if (k > 1)
		display_bracket (P, OPEN, k - 1);
	    display_half (P, BACKWARD);
	}
	else
	{			/* StmtType == CLOSE */
	    td_oldcurline = td_curline;
	    td_dir = -1;
	    display_statement (P, BACKWARD, k);
	    display_half (P, BACKWARD);
	}
    }

    td_startline = screen_shuffle(td_curline + 1, td_oldcurline);
    td_curline   = td_startline;

    td_curstyle = STYLE_NORMAL;
    if (setjmp (unparse_main) == 0)
    {
	if (StmtType == SIMPLE)
	{
	    td_dir = 1;
	    display_statement (P, FORWARD, k);
	    display_half (P, FORWARD);
	}
	else
	if (StmtType == OPEN)
	{
	    td_dir = 1;
	    display_statement (P, FORWARD, k);
	    display_half (P, FORWARD);
	}
	else
	{			/* StmtType == CLOSE */
	    td_dir = 1;
	    display_bracket (P, CLOSE, k + 1);
	    display_half (P, FORWARD);
	}
    }

    /* check the cursor back in */
    uncheck_cursor ();
}

/*
 * unparse in the "forward" direction from node P (already printed) 
 */
static void
display_half (struct PrintInfo P, char forward)
{
    struct PrintInfo Q;

    Q = PI_Succ (P, forward);

    while (1)
    {
	display_rest (Q, forward);
	P = PI_Out (P);
	if (P.ptr == NIL)
	    break;

	display_bracket (P, (forward ? CLOSE : OPEN), 0);
	Q = PI_Succ (P, forward);
    }
}

/*
 * display P and the rest of the list in the "forward" direction 
 */
static void
display_rest (struct PrintInfo P, char forward)
{
    struct PrintInfo Q;

    while (P.ptr != NIL)
    {
	display_statement (P, forward, 0);
	P = PI_Succ (P, forward);

	/*
	 * make sure the statement we are looking at really does exist... 
	 */
	if (P.ptr != NIL && !forward && !should_print (P))
	{
	    /* set P to be the top stmt of elided list */
	    Q = PI_Prev (P);
	    while (Q.ptr != NIL && !should_print (Q))
	    {
		P = Q;
		Q = PI_Prev (Q);
	    }
	}
    }
}

/*
 * unparse the single (perhaps compound) statement P 
 */
static void
display_statement (struct PrintInfo P, char forward, Generic start_line)
{
    struct PrintInfo Q;

    if (!should_print (P))
    {
	display_ellipsis (P.ptr, P.IndentLevel, P.Highlighted);
    }
    else
    if (_IS_SIMPLE (P.ptr))
    {
	display_simple (P.ptr, P.IndentLevel,
			BOOL(P.Highlighted && 
			     NOT (part_highlighted (P.ptr))), start_line);
    }
    else
    {
	display_bracket (P, (forward ? OPEN : CLOSE), start_line);

	/*
	 * check to see if there really is an IN....we are general now aren't we? 
	 */
	Q = PI_In (P);
	if (Q.ptr != NIL)
	{
	    if (should_print (PI_In (P)) || OnPathToRoot (P.ptr))
	    {
		display_rest ((forward ? PI_In (P) : PI_In_To_End (P)), forward);
	    }
	    else
	    {
		Q = PI_In (P);
		display_ellipsis (Q.ptr, Q.IndentLevel, Q.Highlighted);
	    }
	}
	display_bracket (P, (forward ? CLOSE : OPEN), 0);
    }
}

/*
 * display opening or closing bracket of a compound stmt 
 */
static void
display_bracket (struct PrintInfo P, char type, Generic start_line)
{
    if (type == OPEN)
	display_open (P.ptr, P.IndentLevel, P.Highlighted && NOT (part_highlighted (P.ptr)), start_line);
    else
	display_close (P.ptr, P.IndentLevel, P.Highlighted && NOT (part_highlighted (P.ptr)), start_line);
}

/*
 * return 1 if statement p should be printed, 0 otherwise 
 */
static Boolean
should_print (struct PrintInfo P)
{
    if (P.IndentLevel <= gtdi->TopLevel)
	return true;
    if (OnPathToRoot (P.ptr))
	return true;
    if (P.IndentLevel > gtdi->cursor_level + gtdi->InLevel)
	return false;
    if (P.Highlighted)
	return true;
    if (!P.NearCursor)
	return false;
    if (P.TreeDepth != gtdi->cursor_depth)
	return true;
    if (_OUT (P.ptr) != _OUT (gtdi->save_baoi))
	return true;
    if (AroundCursor (P.ptr, gtdi->AroundLevel))
	return true;
    if (P.ptr == gtdi->save_baoi)
	return true;
    return false;
}

/*
 * update PATH field in tree in event of cursor change also, find new root of the "near" region of the tree 
 */
static void
check_cursor ()
{
    TREE_INDEX      p,q,		/* father of current cursor */
                    new_baoi,
                    new_eaoi;
    Generic         qLevel,
                    cursorLevel;


    gtdi->CursorIsStmt = _IS_STATEMENT (gtdi->baoi);
    new_baoi 	       = (gtdi->CursorIsStmt) ? gtdi->baoi : _OUT (gtdi->baoi);
    new_eaoi 	       = (gtdi->CursorIsStmt) ? gtdi->eaoi : _OUT (gtdi->eaoi);

    gtdi->save_baoi    = new_baoi;
    gtdi->save_eaoi    = new_eaoi;

    p 		       = _OUT (gtdi->save_baoi);

    /* mark path to root */
    mark_out (p, 1);

    /* find new root of the N region */
    q = gtdi->save_baoi;
    cursorLevel = gtdi->cursor_level;
    qLevel = cursorLevel;
    while (_OUT (q) != NIL && qLevel - _INDENT_DELTA (_OUT (q)) >= cursorLevel - gtdi->OutLevel)
    {
	q = _OUT (q);
	qLevel -= _INDENT_DELTA (q);
    }
    gtdi->Nroot = q;

    /* find depth of statement containing new cursor */
    gtdi->cursor_level = structure_indent_level (gtdi->save_baoi);
    gtdi->cursor_depth = tree_depth (gtdi->save_baoi);
}

static void
uncheck_cursor()
{
   TREE_INDEX        q;

   /* un-mark path to root */
   q = _OUT(gtdi->save_baoi);
   mark_out (q, -1);
}

static struct PrintInfo
PI_In (struct PrintInfo P)
{
    TREE_INDEX      p,
                    oldp;
    struct PrintInfo R;

    oldp = (P.ptr);
    p = _IN (oldp);

    R.ptr = p;
    if (p == NIL)
	return R;

    R.IndentLevel = P.IndentLevel + _INDENT_DELTA (oldp);
    R.TreeDepth = P.TreeDepth + 1;
    R.Highlighted = BOOL (P.Highlighted || BOOL (p == gtdi->baoi));
    R.NearCursor = BOOL (P.NearCursor || BOOL (p == gtdi->Nroot));
    return R;
}

static struct PrintInfo
PI_In_To_End (struct PrintInfo P)
{
    struct PrintInfo R;
    TREE_INDEX      p,
                    oldp;

    oldp = (P.ptr);
    p = _IN_TO_END (oldp);

    R.ptr = p;
    if (p == NIL)
	return R;

    R.IndentLevel = P.IndentLevel + _INDENT_DELTA (oldp);
    R.TreeDepth = P.TreeDepth + 1;
    R.Highlighted = BOOL (P.Highlighted || BOOL (p == gtdi->eaoi));
    R.NearCursor = BOOL (P.NearCursor || BOOL (p == gtdi->Nroot));
    return R;
}

static struct PrintInfo
PI_Out (struct PrintInfo P)
{
    struct PrintInfo R;
    TREE_INDEX      p,
                    oldp;

    oldp = P.ptr;
    p = _OUT (oldp);

    R.ptr = p;
    if (p == NIL)
	return R;
    R.NearCursor = BOOL (P.NearCursor && BOOL (oldp != gtdi->Nroot));
    R.Highlighted = BOOL (P.Highlighted &&
			  BOOL (_OUT (oldp) != _OUT (gtdi->baoi)));
    R.IndentLevel = P.IndentLevel - _INDENT_DELTA (p);
    R.TreeDepth = P.TreeDepth - 1;
    return R;
}

static struct PrintInfo
PI_Next (struct PrintInfo P)
{
    struct PrintInfo R;
    TREE_INDEX      p,
                    oldp;

    oldp = P.ptr;
    p = _NEXT (oldp);

    R.ptr = p;
    if (p == NIL)
	return R;

    R.IndentLevel = P.IndentLevel;
    R.TreeDepth = P.TreeDepth;

    R.Highlighted = BOOL (P.Highlighted && BOOL (oldp != gtdi->eaoi));
    R.Highlighted = BOOL (R.Highlighted || BOOL (p == gtdi->baoi));
    R.NearCursor = BOOL (P.NearCursor && BOOL (oldp != gtdi->Nroot));
    R.NearCursor = BOOL (R.NearCursor || BOOL (p == gtdi->Nroot));
    return R;
}

static struct PrintInfo
PI_Prev (struct PrintInfo P)
{
    struct PrintInfo R;
    TREE_INDEX      p,
                    oldp;

    oldp = P.ptr;
    p = _PREV (oldp);

    R.ptr = p;
    if (p == NIL)
	return R;

    R.IndentLevel = P.IndentLevel;
    R.TreeDepth = P.TreeDepth;

    R.Highlighted = BOOL (P.Highlighted && BOOL (oldp != gtdi->baoi));
    R.Highlighted = BOOL (R.Highlighted || BOOL (p == gtdi->eaoi));
    R.NearCursor = BOOL (P.NearCursor && BOOL (oldp != gtdi->Nroot));
    R.NearCursor = BOOL (R.NearCursor || BOOL (p == gtdi->Nroot));
    return R;
}

/*
 * if forward is true/false return next/prev(P) 
 */
static
struct PrintInfo
PI_Succ (struct PrintInfo P, char forward)
{
    if (forward)
	return PI_Next (P);
    else
	return PI_Prev (P);
}

/*
 * return the depth in the tree of node p 
 */
static Generic
tree_depth (TREE_INDEX p)
{
    Generic         depth;

    for (depth = 0; p != NIL; depth++)
	p = _OUT (p);
    return (depth);
}


/*
 * true iff node p is or is subordinate to a marked node 
 */
static Boolean
under_mark (TREE_INDEX p)
{
    while (p != NIL)
    {
	if (NodeIsMarked (p))
	    return true;
	p = _OUT (p);
    }
    return false;
}

/*
 * true iff node p should be highlighted 
 */
static Boolean
is_highlighted (TREE_INDEX p, int pDepth)
{
    while (pDepth > gtdi->cursor_depth)
    {
	p = _OUT (p);
	pDepth--;
    }

    return (BOOL (_OUT (p) == _OUT (gtdi->baoi) && WithinCursor (p)));
}

/*
 * true iff node p is in "area of interest" 
 */
static Boolean
WithinCursor (TREE_INDEX p)
{
    TREE_INDEX      q,
                    r;

    q = gtdi->baoi;
    r = gtdi->eaoi;

    while (q != r)
    {
	if (p == q)
	    return true;
	q = _NEXT (q);
    }
    return (BOOL (p == q));
}

/*
 * true iff a sub-list of node p should be highlighted 
 */
static Boolean
part_highlighted (TREE_INDEX p)
{
    return (BOOL (!gtdi->CursorIsStmt && p == gtdi->save_baoi));
}

/*
 * true iff node p is in the subtree N (whose root is roughly the OutLevel'th ancestor of the cursor) 
 */
static Boolean
is_near_cursor (TREE_INDEX p, int pLevel)
{
    while (_OUT (p) != NIL &&
	   pLevel - _INDENT_DELTA (_OUT (p)) >=
	   gtdi->cursor_level - gtdi->OutLevel)
    {
	p = _OUT (p);
	pLevel -= _INDENT_DELTA (p);
    }
    return (BOOL (p == gtdi->Nroot));
}

/*
 * true iff node p is not in "area of interest" but within n of it 
 */
static Boolean
AroundCursor (TREE_INDEX p, int n)
{
    Generic         i;
    TREE_INDEX      q,
                    r;

    q = gtdi->baoi;
    r = gtdi->eaoi;

    if (!_IS_STATEMENT (q))
	q = _OUT (q);
    if (!_IS_STATEMENT (r))
	r = _OUT (r);

    for (i = 0; i <= n; i++)
    {
	if (p == q || p == r)
	    return true;
	if (q == NIL && r == NIL)
	    return false;

	if (q != NIL)
	    q = _PREV (q);
	if (r != NIL)
	    r = _NEXT (r);
    }

    return false;
}


/*
 * true iff node p is marked as being on the path from cursor back to root 
 */
static Boolean
OnPathToRoot (TREE_INDEX p)
{
    return (BOOL (_MARK_GET (p) ? 1 : 0));
}

/*
 * return 1 if node p has been "marked" 
 */
static Boolean
NodeIsMarked (TREE_INDEX p)
{
    return (BOOL (_MARK_GET (p) & 0x0001));
}

/*
 * change the ref count by incr for all ancestors of node p (incl p itself) 
 */
static void
mark_out (TREE_INDEX p, Generic incr)
{
    TREE_INDEX      q;

    q = p;

    while (q != NIL)
    {
	_MARK_SET (q, _MARK_GET (q) + incr + incr);
	q = _OUT (q);
    }
}


static void
display_open (TREE_INDEX p, Generic indent, char highlight, Generic toskip)
{
    display (p, indent, (Boolean)highlight, OPEN, gtdi->open, toskip);
}

static void
display_close (TREE_INDEX p, Generic indent, char highlight, Generic toskip)
{
    display (p, indent, (Boolean)highlight, CLOSE, gtdi->close, toskip);
}

static void
display_simple (TREE_INDEX p, Generic indent, Boolean highlight, Generic toskip)
{
    display (p, indent, (Boolean)highlight, SIMPLE, gtdi->simple, toskip);
}

/*
 * We won't display an ellipsis if there one was just output, or if the statement it points to is a placeholder. 
 */
static void
display_ellipsis (TREE_INDEX p, Generic indent, Boolean highlight)
{
    Boolean         do_highlighting = false;
    Boolean         do_marking = false;

    if (NOT (td_el_already))
    {
	map_start (p, indent, 0, _IS_SIMPLE (p) ? SIMPLE : OPEN);
	if (highlight)
	    start_highlight ();
	if (under_mark (p))
	{
	    do_marking = true;
	    if (_MARK_TYPE (p) == MARK_HIGHLIGHTED)
	    {
		do_highlighting = true;	/* mark is highlight */
		start_highlight ();
	    }
	    else
		toggle_underline ();
	}
	else
	    do_marking = false;

	map_fmt (p, '.');
	map_fmt (p, '.');
	map_fmt (p, '.');
	map_fmt (p, '\n');
	td_el_already = true;

	if (do_marking)
	{
	    if (do_highlighting)
		end_highlight ();
	    else
		toggle_underline ();
	}

	if (highlight)
	    end_highlight ();
	map_shuffle ();
    }
}

static void
display (TREE_INDEX p, Generic indent, Boolean highlight, char type, 
         char **table, Generic skip)
{
    Boolean         do_highlighting = false;
    Boolean         do_marking = false;

    map_start (p, indent, skip, type);
    td_el_already = false;
    if (highlight)
	start_highlight ();

    if (under_mark (p))
    {
	do_marking = true;
	if (_MARK_TYPE (p) == MARK_HIGHLIGHTED)
	{
	    do_highlighting = true;	/* mark is highlight */
	    start_highlight ();
	}
	else
	    toggle_underline ();
    }
    else
	do_marking = false;

    do_unparse (p, get_format (table, p));
    if (do_marking)
    {
	if (do_highlighting)
	    end_highlight ();
	else
	    toggle_underline ();
    }

    if (highlight)
	end_highlight ();
    map_shuffle ();
}

static void
toggle_invert ()
{
    td_curstyle = td_curstyle ^ ATTR_INVERSE;
}

static void
toggle_half ()
{
    td_curstyle = td_curstyle ^ ATTR_HALF;
}

static void
toggle_underline ()
{
    td_curstyle = td_curstyle ^ ATTR_UNDERLINE;
}

static void
start_highlight ()
{
    td_curstyle = td_curstyle | STYLE_BOLD;
}

static void
end_highlight ()
{
    if (td_curstyle & STYLE_BOLD)
    {				/* if bold is on toggle it off */
	td_curstyle = td_curstyle ^ STYLE_BOLD;
    }
}

static void
map_start (TREE_INDEX p, Generic indent, Generic toskip, int type)
{
    td_newline = true;
    /* width of screen available */
    td_width = gtdi->pane_width - indent * TABSIZE;
    /* total number of lines for this construction... */
    td_n_lines = 0;

    /*
     * which line of the current construct we wish to put on the screen at curline....this value should be nonzero only
     * if we are printing forward....and we are at the beginning of the screen 
     */

    td_type = type;
    td_topnode = p;
    td_toskip = toskip;

    /*
     * td_toskip has the value 0 - in case of no specific information, display the entire node. otherwise td_toskip > 0
     * and indicates which line of the current construct is to be displayed at td_curline. 
     */

    /*
     * if we are going backwards..then the requested line td_toskip will be the last line in the blob (td_curline) if
     * we are going forwards...then the requested line td_toskip is the first line of the blob. (td_curline) 
     */

    /*
     * define clipping regions ... 
     */

    if (is_backward (td_dir))
    {
	if (td_toskip == 0)
	{			/* display it all.... wrapping around */
	    td_firstline = 0;
	    td_endline = td_curline;
	    td_curline = td_firstline;
	    td_case = BACK_0;
	}
	else
	{
	    td_firstline = td_curline - td_toskip + 1;
	    td_endline = td_curline;
	    td_curline = td_firstline;
	    td_case = BACK_1;
	}
    }
    else
    {
	if (td_toskip == 0)
	{
	    td_firstline = td_curline;
	    td_endline = gtdi->pane_length - 1;
	    td_curline = td_firstline;
	    td_case = FOR_0;
	}
	else
	{
	    td_firstline = td_curline;
	    td_endline = gtdi->pane_length - 1;
	    td_curline = td_firstline - td_toskip;
	    td_case = FOR_1;
	}
    }

    td_initial_indent = indent;
    td_curcol = gtdi->fixed_indent + td_initial_indent * TABSIZE;

    td_start_col = td_curcol;

    if (is_forward (td_dir) && td_firstline >= gtdi->pane_length) 
    {
	longjmp (unparse_main, 1);
    }

    if (is_backward (td_dir) && td_endline < 0)
    {
	longjmp (unparse_main, 1);
    }
}

/*
 * just start writing out from wherever we were before... we are at 
 * td_curline, td_curcol.... when we hit td_endline and the maximum 
 * row and are printing forward, then we are done and should do a 
 * longjmp back out of here.... otherwise just write characters into 
 * the map starting at the firstline, and looping around...
 */
static void
map_fmt (TREE_INDEX p, char c)
{
    switch (c)
    {
	case '\n':
	    if (!td_line_used)
	    {
		if (td_start_col > gtdi->pane_width - 1)
		  set_node (td_curline, gtdi->pane_width - 1, '+', p, td_curstyle);
		else 
		  set_node (td_curline, td_curcol, ' ', p, td_curstyle);
	    }
	    if (td_curline == td_endline)
	    {
		if (is_forward (td_dir))
		{
		    longjmp (unparse_main, 1);
		}
		else
		if (td_case == BACK_0)
		    td_curline = td_firstline;
	    }
	    else
		td_curline++;

	    td_newline   = true;
	    td_line_used = false;
    	    td_curcol = td_start_col + TABSIZE;
	    break;
	case TAB_COND_BREAK:
	    if (!gtdi->clipping && td_curcol + 5 > gtdi->pane_width)
	    {
		if (td_line_used)
		    set_node (td_curline, gtdi->pane_width - 1, '+', p, td_curstyle);
		else 
		    set_node (td_curline, td_curcol, ' ', p, td_curstyle);

		if (td_curline == td_endline)
		{
		    if (is_forward (td_dir))
		    {
			longjmp (unparse_main, 1);
		    }
		    else
		    if (td_case == BACK_0)
			td_curline = td_firstline;
		}
		else
		    td_curline++;

		td_newline = true;
	        td_line_used = false;		
		td_curcol = td_start_col + TABSIZE;
	    }
	    break;
	default:
	    if (td_curcol < gtdi->pane_width - 1)
	    {
	        td_line_used = true;
		set_node (td_curline, td_curcol, c, p, td_curstyle);
		td_curcol++;
	    }
	    else if (gtdi->clipping)
	    {
		/* drop a continuation character in the last column */
		/* this actually writes a + sign for each character past the end of line....oh well! */
	        td_line_used = true;
		set_node (td_curline, gtdi->pane_width - 1, '+', p, td_curstyle); 
	    }
	    else
	    {	/* drop a continuation character in the last column,
		   and go to next line... */
	        td_line_used = true;
		set_node (td_curline, gtdi->pane_width - 1, '+', p, td_curstyle);

		if (td_curline == td_endline)
		{
		    if (is_forward (td_dir))
		    {
			longjmp (unparse_main, 1);
		    }
		    else
		    if (td_case == BACK_0)
			td_curline = td_firstline;
		}
		else
		    td_curline++;

		td_newline = true;
		td_curcol = td_start_col + TABSIZE;
		set_node (td_curline, td_curcol, c, p, td_curstyle);
		td_curcol++;
	    }
    }
}

/* we have just unparsed going towards the top of the screen? did we get there? if not,
   then we should swap lines and reset td_startline */
int screen_shuffle(int first, int last)
{
   int i,j;

   /* if we have lines displayed */
   if (first <= last)
   {
     /* shift them if they are not already at the top of the pane */
     if (first > 0)
     {
        j = 0;
        for (i = first;  i <= last;  i++)
        {
	  line_swap(i,j);
          j++;
        }
     }

     return last - first + 1;
   }

   /* no lines to shift */
   else return 0;
}

void dump_line(int lines)
{
        struct line_data *thisld;
	int    cols;
        Point  loc;

 	(void) printf("line %d : ",lines);

	thisld = &(gtdi->line_data[lines]);
	if (thisld->topnode == (TREE_INDEX) - 1)
	    return;

	for (cols = 0; cols < gtdi->pane_width; cols++)
	{
	    if (thisld->elements[cols].p == (TREE_INDEX) - 1)
		continue;
	    loc = makePoint (cols, lines);
	    (void) printf("%c", thisld->elements[cols].tc.ch);
	}
        (void) printf("\n");
}

static void
map_shuffle ()
{
    Generic         total;
    Generic         i,
                    last,
                    cur;

    switch (td_case)
    {
	case FOR_0:
	case FOR_1:
	    return;
	case BACK_0:
	    total = td_endline - td_firstline + 1;
	    if (td_n_lines == total)
	    {	/* filled exactly - including first and last */
		td_curline = -1;
		longjmp (unparse_main, 1);
	    }

	    if (td_n_lines > total)
	    {   /* more lines than necessary - unwrap */
		last = td_endline;

		/* we may not have printed anything here.... */
		if (td_line_used)
		{
		    cur = td_curline - 1;
		    if (cur < 0)
			cur = td_endline;
		}
		else
		    cur = td_curline;
		for (i = 0; i < total; i++)
		{
		    sld[last--] = gld[cur];
		    cur--;
		    if (cur < 0)
			cur = td_endline;
		}

		for (i = 0; i <= td_endline; i++)
		    gld[i] = sld[i];

		td_curline = -1;
		longjmp (unparse_main, 1);
	    }


	    last = td_endline;
	    for (i = td_n_lines - 1; i >= 0; i--)
	    {
		line_swap (last--, i);
	    }

	    td_curline = td_endline - td_n_lines;
	    break;
	case BACK_1:
	    td_curline = td_firstline - 1;
	    break;
    }
    return;
}

static void
line_swap (Generic i, Generic j)
{
    struct line_data k;

    k = gld[i];
    gld[i] = gld[j];
    gld[j] = k;
}

/* below the statement level.... */
static void
do_unparse (TREE_INDEX p, char *s)
{
    char            c,
                    son_id[2],
                    pred[64];
    TREE_INDEX      son;
    char            str[MAX_TEXT_LENGTH];
    int             len,
                    k;
    char           *s1;

    if (s == 0)	/* changed by ACW 10-20-87 */
	return;

    if (p == NIL)
	return;

    while (c = *s++)
    {
	switch (c)
	{
	    case '%':
		/* snarf the son identifier and recursively invoke. */
		son_id[0] = *s++;
		son_id[1] = 0;
		if (son_id[0] == '*')
		{
		    son = _GETFIRST (p);
		    while (son != NIL)
		    {
			if (son == gtdi->baoi)
			{
			    start_highlight ();
			}
			do_unparse (son, get_simple_format (son));
			if (son == gtdi->eaoi)
			    end_highlight ();
			do_unparse (son, s);
			son = _NEXT (son);
		    }
		    return;
		}
		else
		if (son_id[0] == 's')
		{
		    (void) strcpy (str, _GET_TEXT (p));
		    len = strlen (str);
		    for (k = 0; k < len; k++)
			map_fmt (p, str[k]);
		}
		else
		{
		    son = _GETSON (p, atoi (son_id));
		    if (son == gtdi->baoi)
		    {
			start_highlight ();
		    }
		    do_unparse (son, get_simple_format (son));
		    if (son == gtdi->eaoi)
			end_highlight ();
		}
		break;
	    case '\\':
		/* snarf the next character and interpret. */
		c = *s++;
		switch (c)
		{
		    case 'n':
			map_fmt (p, '\n');
			break;
		    case 'c':
			map_fmt (p, TAB_COND_BREAK);
			break;
		    case 'h':
			toggle_half ();
			break;
		    case 'u':
			toggle_underline ();
			break;
		    case 'i':
			toggle_invert ();
			break;
		    case '<':  /* slam back to the left margin - this should be the first thing on the line */
			td_curcol    = 0;
			break;
		    case '>':	/* reindent - are the chars in between blank ? */
			td_curcol = td_start_col;
			break;
		    default:
			map_fmt (p, c);
		}
		break;
	    case FBRC:
		s = get_predicate (s, pred);
		s1 = eval_str_func (pred, p);
		/* print out chars in returned string */
		while (*s1)
		    map_fmt (p, *s1++);

		/* skip to closing bracket. */
		while (*s++ != CBRC);
		break;
	    case OBRC:
		s = get_predicate (s, pred);
		if (!eval_predicate (pred, p))
		{
		    /* skip to closing bracket. */
		    while (*s++ != CBRC);
		}
		break;
	    case CBRC:
		break;
	    default:
		map_fmt (p, c);
		break;
	}
    }
}

static char    *
get_format (char **table, TREE_INDEX p)
{
    char *temp;
    /* check to see if the node's nodetype has a table entry. */
    temp = table[_GET_NODE_TYPE (p)];
    return temp;
}

/*
 * Break a string of the form predicate(nodesel) into the composite parts predicate and nodesel, returning the advanced
 * string pointer just beyond the close paren.  Nodesel is put in pred_selector. 
 */
static
char           *
get_predicate (char *s, char *pred)
{
    char            c,
                   *sel;

    sel = pred_selector;
    while ((c = *s++) != '(')
	*pred++ = c;
    *pred = 0;
    while ((c = *s++) != ')')
	*sel++ = c;
    return s;
}

/*
 * returns the address of the function that evaluates this predicate... 
 */

static
UnpIntFunc
linear_lookup (char *name)
{
    Generic         i;

    for (i = 0; gtdi->pred_map[i].key; i++)
	if (strcmp (gtdi->pred_map[i].key, name) == 0)
	    return gtdi->pred_map[i].routine;

    return (UnpIntFunc) - 1;
}

static Boolean
eval_predicate (char *name, TREE_INDEX p)
{
    UnpIntFunc             func;
    Boolean         result;
    Boolean         negate = false;

    if (name[0] == '!')
    {
	negate = true;
	name++;
    }

    if (pred_selector[1] != '0')
	p = _GETSON (p, pred_selector[1] - '0');

    func = linear_lookup (name);
    if ((Generic) func < 0)
	return false;

    result = BOOL (func (gtdip, OWNER (gtdip), p));
    if (negate)
	return NOT (result);
    return result;
}

static char    *
eval_str_func (char *name, TREE_INDEX p)
{
    UnpIntFunc             func;

    if (pred_selector[1] != '0')
	p = _GETSON (p, pred_selector[1] - '0');

    func = linear_lookup (name);
    if ((Generic) func < 0)
	return "oops";

    return ((char *) func (gtdip, OWNER (gtdip), p));
}

static          TREE_INDEX
xy_to_node (Point loc)
{
    TREE_INDEX      node;
    Generic         line,
                    col;
    Generic         nline,
                    ncol;
    struct line_data *thisld;

    /* nothing to find here */
    if (gtdi->has_no_data || gtdi->is_too_small)
	return NIL;

    line = loc.y;
    col = loc.x;

    nline = find_row_data (line);
    ncol  = find_col_data (nline, col);

    /* nothing on the screen since there was no room for it */
    if (nline == -1 && ncol == -1)
	return NIL;

    thisld = &(gtdi->line_data[nline]);
    if (nline != line || ncol != col)
	node = thisld->topnode;
    else
	node = thisld->elements[ncol].p;
    return node;
}

static Generic
find_row_data (int line)
{
    Generic         i;
    struct line_data *thisld;

    /* nothing to find here */
    if (gtdi->has_no_data || gtdi->is_too_small)
	return -1;

    for (i = line; i >= 0; i--)
    {
	thisld = &(gtdi->line_data[i]);
	if (thisld->topnode != (TREE_INDEX) - 1)
	    return i;
    }
    for (i = line + 1; i < gtdi->pane_length; i++)
    {
	thisld = &(gtdi->line_data[i]);
	if (thisld->topnode != (TREE_INDEX) - 1)
	    return i;
    }

    return -1;
}

static Generic
find_col_data (Generic line, Generic col)
{
    Generic         i;
    struct element *elem;
    struct line_data *thisld;

    /* nothing to find here */
    if (gtdi->has_no_data || gtdi->is_too_small)
	return -1;

    if (line < 0)
        return -1;

    thisld = &(gtdi->line_data[line]);
    elem = thisld->elements;
    for (i = col; i >= 0; i--)
    {
	if (elem[i].p != (TREE_INDEX) - 1)
	    return i;
    }
    for (i = col + 1; i < gtdi->pane_width; i++)
    {
	if (elem[i].p != (TREE_INDEX) - 1)
	    return i;
    }

    return -1;
}

static void
clear_line_data ()
{
    register Generic lines,
                    len;

    len = gtdi->pane_length;

    for (lines = 0; lines < len; lines++)
	clear_line_data_line (lines);
}

static void
clear_line_data_line (Generic lines)
{
    struct element *elem;
    struct line_data *thisld;
    Generic         cols,
                    max_cols;

    thisld = &(gtdi->line_data[lines]);
    max_cols = gtdi->pane_width;
    elem = thisld->elements;
    for (cols = 0; cols < max_cols; cols++)
    {
	elem[cols].tc = makeTextChar (0, 0);
	elem[cols].p = (TREE_INDEX) - 1;
    }
    thisld->type = (char) -1;
    thisld->indent = (short) -1;
    thisld->which = (short) -1;
    thisld->topnode = (TREE_INDEX) - 1;
}

static void
do_string (TREE_INDEX p, char *s)
{
    char            c,
                    son_id[2],
                    pred[64];
    TREE_INDEX      son;
    char            str[MAX_TEXT_LENGTH];
    int             len,
                    k;
    char           *s1;

    if (s == 0 && p != NIL)
    {
	return;
    }
    while (c = *s++)
    {
	switch (c)
	{
	    case '%':
		/* snarf the son identifier and recursively invoke. */
		son_id[0] = *s++;
		son_id[1] = 0;
		if (son_id[0] == '*')
		{
		    son = _GETFIRST (p);
		    while (son != NIL)
		    {
			do_string (son, get_simple_format (son));
			do_string (son, s);
			son = _NEXT (son);
		    }
		    return;
		}
		else
		if (son_id[0] == 's')
		{
		    (void) strcpy (str, _GET_TEXT (p));
		    len = strlen (str);

		    for (k = 0; k < len; k++)
			map_string (str[k]);
		}
		else
		{
		    son = _GETSON (p, atoi (son_id));
		    do_string (son, get_simple_format (son));
		}
		break;
	    case '\\':
		/* snarf the next character and interpret. */
		c = *s++;

		switch (c)
		{
		    case 'n':
		    case 'c':
		    case 'h':
		    case 'u':
		    case 'i':
			break;	/* ignore these.... */
		    case '<':
			string_result_length = 0;
		        break;
		    case '>':
			string_result_length = td_line_indent;
			break;
		    default:
			map_string (c);
			break;
		}
		break;
	    case FBRC:
		s = get_predicate (s, pred);
		s1 = eval_str_func (pred, p);

		/* print out chars in returned string */
		while (*s1)
		    map_string (*s1++);

		/* skip to closing bracket. */
		while (*s++ != CBRC);
		break;
	    case OBRC:
		s = get_predicate (s, pred);
		if (!eval_predicate (pred, p))
		{
		    /* skip to closing bracket. */
		    while (*s++ != CBRC);
		}
		break;
	    case CBRC:
		break;
	    default:
		map_string (c);
		break;
	}
    }
}

static void
map_string (char c)
{
    /* need to do some range checking here */
    if (string_result_length < MAX_TEXT_LENGTH - 1)
        string_result[string_result_length++] = c;
}


# define RANGE 7		/* the numbers available are 0, 1, ..., RANGE - 3, and "off" */
static Generic
getnum_from_menu (char *str)
{
    char           *nummenu[RANGE];
    short           sel;

    nummenu[0] = str;
    nummenu[1] = "0";
    nummenu[2] = "1";
    nummenu[3] = "2";
    nummenu[4] = "3";
    nummenu[5] = "4";

    nummenu[RANGE - 1] = "INFINITY";

    sel = menu_select ("Select a value: ", RANGE, nummenu);
    if (sel == -1 || sel == 0)
	return -1;

    return (sel == RANGE - 1 ? INFINITY : sel - 1);
}

static void
free_internals ()
{
    if (gtdi == NULL)
    {
	die_with_message ("trying to free a NULL gtdi in free_internals \n");
	return;
    }

    free_line_data ();
    free_mem ((void*) gtdi->swap_array);
}


static void
draw_screen ()
{
    Point           loc;
    Generic         cols,
                    lines;
    struct line_data *thisld;

    for (lines = 0; lines < gtdi->pane_length; lines++)
    {
	thisld = &(gtdi->line_data[lines]);
	if (thisld->topnode == (TREE_INDEX) - 1)
	    continue;
	for (cols = 0; cols < gtdi->pane_width; cols++)
	{
	    if (thisld->elements[cols].p == (TREE_INDEX) - 1)
		continue;
	    loc = makePoint (cols, lines);
	    sm_optim_putchar (OP (gtdip), loc, thisld->elements[cols].tc);
	}
    }
}

static void
set_line (int Y, int Type, int Indent, int Nlines)
{
    struct line_data *thisld;

    if (!(Y < td_firstline || Y > td_endline))
    {
	thisld          = &(gtdi->line_data[Y]);
	thisld->type    = Type;
	thisld->indent  = Indent;
	thisld->which   = Nlines;
	thisld->topnode = td_topnode;
    }
}

static void
set_node (int Y, int X, int C, int P, int STYLE)
{
    struct element   *elem;
    struct line_data *thisld;

    if (!(Y < td_firstline || Y > td_endline))
    {
	if (td_newline)
	{
	    clear_line_data_line (Y);
	    td_n_lines++;
	    set_line (td_curline, td_type, td_initial_indent, td_n_lines);
	    td_newline = false;
	}

	thisld = &(gtdi->line_data[Y]);
	elem   = thisld->elements;
	elem[X].tc = makeTextChar ((unsigned char) C, (unsigned char) STYLE);
	elem[X].p = P;
    }
    else
    {
	if (td_newline)
	{
	    td_n_lines++;
	    td_newline = false;
	}
    }
}

