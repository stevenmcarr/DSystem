/* $Id: gtd_inc.h,v 1.7 1997/06/25 15:00:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <stdio.h>
#include <setjmp.h>
#include <libs/support/strings/rn_string.h>
#include <string.h>

struct tdi;
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

typedef FUNCTION_POINTER(int, UnpIntFunc,(Pane*,Generic,TREE_INDEX));
typedef FUNCTION_POINTER(int, UnpIntFunc4,(Pane*,Generic,TREE_INDEX,Generic));
typedef FUNCTION_POINTER(char*, UnpStringFunc,(Pane*,Generic,TREE_INDEX));

struct  unp_routines {
  UnpIntFunc  In,In_to_end,Out,Next,Prev,Getfirst,
    Is_simple,Is_statement,Indent_delta,
    Mark_type,Mark_get,Get_node_type;
  UnpIntFunc4 Mark_set,Getson;
  UnpStringFunc  Get_text;
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


