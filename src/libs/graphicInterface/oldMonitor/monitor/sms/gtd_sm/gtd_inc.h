/* $Id: gtd_inc.h,v 1.6 1997/03/11 14:33:59 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <stdio.h>
#include <setjmp.h>
#include <libs/support/strings/rn_string.h>
#include <string.h>

/*static void 	free_internals();
static TREE_INDEX   xy_to_node();
static char     *get_predicate();
static char 	*get_format();
static char 	*eval_str_func();
static PFI 	linear_lookup();

static void	free_line_data(),
		unparse(),
		unparse_internal(),
		display_half(),
		display_rest(),
		display_statement(),
		display_bracket(),
		check_cursor(),
		uncheck_cursor(),
		find_Nroot(),
		MarkNode(),
		UnmarkNode(),
		mark_out(),
		display_open(),
		display_close(),
		display_simple(),
		display_ellipsis(),
		display(),
		toggle_invert(),
		toggle_half(),
		toggle_underline(),
		start_highlight(),
		end_highlight(),
		clear_line_data(),
		clear_line_data_line(),
		do_string(),
		line_map_shift_left(),
		map_string(),
		draw_screen(),
		set_line(),
		set_node(),
		map_start(),
		map_shift_left(),
		map_shuffle(),
		line_swap(),
		map_fmt(),
		do_unparse(),
		new_shape();

static Generic  get_unparse_info(),
		structure_indent_level(),
		find_row_data(),
		find_col_data(),
		getnum_from_menu(),
		tree_depth();

static Boolean 	should_print(),
		under_mark(),
		is_highlighted(),
		WithinCursor(),
		part_highlighted(),
		is_near_cursor(),
		AroundCursor(),
		just_one_simple_statement(),
		OnPathToRoot(),
		NodeIsMarked(),
		eval_predicate();*/

	/* local datatype */
struct	gtd_info	{
	Pane	       *op;			/* the optim slave pane	*/
	Point		size;
	struct tdi     *tdi;
	Generic		owner_id;
	};

#define GTDINFO(p)	((struct gtd_info *) p->pane_information)
#define	OP(p)		(GTDINFO(p)->op)
#define	SIZE(p)		(GTDINFO(p)->size)
#define TDI(p)		(GTDINFO(p)->tdi)
#define OWNER(p)	(GTDINFO(p)->owner_id)

#define FIXEDINDENT 7

#define FOR_0      7
#define FOR_1     17
#define BACK_0    27
#define BACK_1    37

#define FORWARD   1    /* this must be non-zero */
#define BACKWARD  0    /* this must be zero     */
#define ALL       (forward ? 0 : INFINITY)

struct element {
	TextChar   tc;
        TREE_INDEX p;
        };

struct  line_data {
        struct element *elements;
        char type;          /* printed by open, close, or simple */
        short indent;       /* indenting level used when printed */
        short which;        /* which line of the node this is    */
        TREE_INDEX topnode; /* statement node that caused output */
        };

struct  unp_routines {
        PFI  In,In_to_end,Out,Next,Prev,Getson,Getfirst,
	     Is_simple,Is_statement,Indent_delta,
	     Mark_type,Mark_set,Mark_get,Get_node_type;
        PFS  Get_text;
        };

struct  tdi {
        Generic pane_length,pane_width;
        TREE_INDEX baoi, eaoi;	/* beginning and ending area of interest, for highlighting */
        Generic InLevel;       	/* # of levels in  from cursor to display   		   */
        Generic OutLevel;      	/* # of levels out from cursor to display   		   */
        Generic AroundLevel;   	/* # of stmts at level of cursor to display 		   */
        Generic TopLevel;      	/* # of levels from root of pgm to display  		   */
        TREE_INDEX save_baoi, 	/* first stmt containing highlighted text   		   */
                save_eaoi, 	/*  last stmt containing highlighted text   		   */
                Nroot;     	/* root of subtree OutLevel's out from baoi 		   */
        Generic cursor_depth;  	/* # of out's to get from baoi to root      		   */
        char CursorIsStmt; 	/* 1 iff baoi is not a sub-statement     		   */
        Generic 	    cursor_level;
        Generic 	    last_center;
	Generic 	    fixed_indent;
	Boolean             has_no_data;
	Boolean		    is_too_small;
        TREE_INDEX          unp_node;
        char		    stmttype;
	Generic    	    unparse_mode;

        struct pred_map     *pred_map;
        struct line_data    *line_data;
	Boolean		     clipping;	/* if true then clip, else wrap 		   */
        struct unp_routines routines;
        char **open,**close,**simple;
        struct line_data    *swap_array;
};

struct PrintInfo {
    TREE_INDEX ptr;
    Generic IndentLevel;
    Generic TreeDepth;
    Boolean Highlighted;	/* true if this entire statement is highlighted */
    Boolean NearCursor;
}; 

/*struct PrintInfo PI_In(), PI_In_To_End(), PI_Out(), PI_Next(), PI_Prev(), PI_Succ();*/

/* routines supplied by the user of the generalized tree display utlities */
#define _IN(node)               (gtdi->routines.In)(gtdip,OWNER(gtdip),node)
#define _IN_TO_END(node)        (gtdi->routines.In_to_end)(gtdip,OWNER(gtdip),node)
#define _OUT(node)              (gtdi->routines.Out)(gtdip,OWNER(gtdip),node)
#define _NEXT(node)             (gtdi->routines.Next)(gtdip,OWNER(gtdip),node)
#define _PREV(node)             (gtdi->routines.Prev)(gtdip,OWNER(gtdip),node)
#define _IS_SIMPLE(node)        (gtdi->routines.Is_simple)(gtdip,OWNER(gtdip),node)
#define _IS_STATEMENT(node)     (gtdi->routines.Is_statement)(gtdip,OWNER(gtdip),node)
#define _INDENT_DELTA(node)     (gtdi->routines.Indent_delta)(gtdip,OWNER(gtdip),node)
#define _MARK_TYPE(node)        (gtdi->routines.Mark_type)(gtdip,OWNER(gtdip),node)
#define _MARK_SET(node,amt)     (gtdi->routines.Mark_set)(gtdip,OWNER(gtdip),node,amt)
#define _MARK_GET(node)         (gtdi->routines.Mark_get)(gtdip,OWNER(gtdip),node)
#define _GET_NODE_TYPE(node)    (gtdi->routines.Get_node_type)(gtdip,OWNER(gtdip),node)
#define _GETSON(node,n)         (gtdi->routines.Getson)(gtdip,OWNER(gtdip),node,n)
#define _GETFIRST(node)         (gtdi->routines.Getfirst)(gtdip,OWNER(gtdip),node)
#define _GET_TEXT(p)            (gtdi->routines.Get_text)(gtdip,OWNER(gtdip),p)

#define get_simple_format(p)    get_format(gtdi->simple, p)
#define get_open_format(p)      get_format(gtdi->open, p)
#define get_close_format(p)     get_format(gtdi->close, p)

jmp_buf unparse_main;

#define ld gtdi->line_data

#define is_forward(dir)   (dir == 1)
#define is_backward(dir)  (dir == -1)
#define get_which(line)   ld[line].which
#define get_indent(line)  ld[line].indent
#define get_type(line)    ld[line].type
#define get_topnode(line) ld[line].topnode

static Boolean		td_line_used;
static Generic		td_start_col;
static Generic		td_case;
static Generic		td_line_indent;
static Generic		td_initial_indent;
static char		td_type;
static TREE_INDEX	td_topnode;
static Generic		td_n_lines;
static Generic		td_toskip;
static Generic		td_width;
static Generic		td_firstline;
static Generic		td_curline;
static Generic		td_oldcurline;
static Generic		td_curcol;
static Generic		td_endline;
static Generic		td_startline;
static Generic		td_dir;
static Generic		td_curstyle;
static Boolean		td_el_already;
static Boolean		td_newline;

#define gld gtdi->line_data
#define sld gtdi->swap_array

struct tdi     *gtdi;
Pane	       *gtdip;
extern char    *Rn_gtdtables;

static char pred_selector[256];


