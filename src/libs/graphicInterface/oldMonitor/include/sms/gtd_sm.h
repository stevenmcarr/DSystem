/* $Id: gtd_sm.h,v 1.4 1997/03/11 14:33:22 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 		       gtd_sm.h 			*/
		/*      Generalalized tree display screen module.	*/
		/* 							*/
		/********************************************************/

/* The Generalized tree display screen module allows the automatic display of trees.    */
/* Works through MANY user provided callbacks.						*/

#ifndef gtd_sm_h
#define gtd_sm_h

#define TREE_INDEX Generic
/* definitions needed for users of the tree display instance code */
/* we do information hiding as well as possible for C.... the user
   of the tdi maintains his own idea of the cursor, the area of interest,
   and the unparsing mode, and has only a pointer to the appropriate
   instance variable */

/* an array of predicate names, and the associated functions to call */
typedef FUNCTION_POINTER(int, sm_gtd_pred_func, (Pane *p, Generic owner, TREE_INDEX node));
struct pred_map {
        char *key;
        sm_gtd_pred_func  routine;
        };

EXTERN(short, sm_gtd_get_index, (void));
/* get the gtd_sm index number		*/
/* Takes no parameters.  Returns the index of the gtd screen module.			*/

EXTERN(Point, sm_gtd_pane_size, (Point size, short font));
/* get the minimum pane size for a block*/
/* Takes two parameters:  (Point size) the size of the character block and (short font)	*/
/* the font in which it will be displayed.  Returns the minimum pane size necessary to	*/
/* display the block.									*/

typedef FUNCTION_POINTER(int, sm_gtd_in_callback, (Pane *p, Generic owner, TREE_INDEX node));
typedef FUNCTION_POINTER(int, sm_gtd_in_to_end_callback, (Pane *p, Generic owner, TREE_INDEX node));
typedef FUNCTION_POINTER(int, sm_gtd_out_callback, (Pane *p, Generic owner, TREE_INDEX node));
typedef FUNCTION_POINTER(int, sm_gtd_next_callback, (Pane *p, Generic owner, TREE_INDEX node));
typedef FUNCTION_POINTER(int, sm_gtd_prev_callback, (Pane *p, Generic owner, TREE_INDEX node));
typedef FUNCTION_POINTER(int, sm_gtd_is_simple_callback, (Pane *p, Generic owner, TREE_INDEX node));
typedef FUNCTION_POINTER(int, sm_gtd_is_statement_callback, (Pane *p, Generic owner, TREE_INDEX node));
typedef FUNCTION_POINTER(int, sm_gtd_indent_delta_callback, (Pane *p, Generic owner, TREE_INDEX node));
typedef FUNCTION_POINTER(int, sm_gtd_mark_type_callback, (Pane *p, Generic owner, TREE_INDEX node));
typedef FUNCTION_POINTER(int, sm_gtd_mark_set_callback, (Pane *p, Generic owner, TREE_INDEX node, int val));
typedef FUNCTION_POINTER(int, sm_gtd_mark_get_callback, (Pane *p, Generic owner, TREE_INDEX node));
typedef FUNCTION_POINTER(int, sm_gtd_get_node_type_callback, (Pane *p, Generic owner, TREE_INDEX node));
typedef FUNCTION_POINTER(int, sm_gtd_get_son_callback, (Pane *p, Generic owner, TREE_INDEX node, int which));
typedef FUNCTION_POINTER(int, sm_gtd_get_first_callback, (Pane *p, Generic owner, TREE_INDEX node));
typedef FUNCTION_POINTER(char *, sm_gtd_get_text_callback, (Pane *p, Generic owner, TREE_INDEX node));
EXTERN(void, sm_gtd_initialize, (Pane *p, Generic owner_id, short font_id,
 char **open, char **close, char **simple, struct pred_map *pred_map,
 Generic fixed_indent, sm_gtd_in_callback In,
 sm_gtd_in_to_end_callback In_to_end, sm_gtd_out_callback Out,
 sm_gtd_next_callback Next, sm_gtd_prev_callback Prev,
 sm_gtd_is_simple_callback Is_simple,
 sm_gtd_is_statement_callback Is_statement,
 sm_gtd_indent_delta_callback Indent_delta,
 sm_gtd_mark_type_callback Mark_type, sm_gtd_mark_set_callback Mark_set,
 sm_gtd_mark_get_callback Mark_get,
 sm_gtd_get_node_type_callback Get_node_type,
 sm_gtd_get_son_callback Getson, sm_gtd_get_first_callback Getfirst,
 sm_gtd_get_text_callback Get_text));
/* set the status of the gtd pane	*/
/* This routine initializes the gtd screen module. It passes in pointers to callback    */
/* functions for manipulating a tree controlled entirely by the user.			*/
/* Takes the following incredible number of parameters:					*/
/*   p, owner_id, font_id,	 							*/
/*   open, close, simple, pred_map, fixed_indent,					*/
/*   In, In_to_end, Out, Next, Prev, Is_simple, Is_statement,				*/
/*   Indent_delta, Mark_type, Mark_set, Mark_get,					*/
/*   Get_node_type, Getson, Getfirst, Get_text) 					*/
/* This routine does not display a tree, it only initializes the needed information for */
/* later calls.										*/

EXTERN(void, sm_gtd_set_clipping, (Pane *p, Boolean clipping));
/* set clipping status for gtd pane	*/
/* Takes two parameters, the pane, and true indicating clipping and false indicating    */
/* wrapping. The default is wrapping.							*/


/* Mark_type should return one of the two values:					*/
#define MARK_UNDERLINE		1
#define MARK_HIGHLIGHTED	2


EXTERN(Point, sm_gtd_map_size, (Pane *p));
/* get the size of the display area	*/
/* Takes one parameter p.  Returns the size.						*/

EXTERN(void, sm_gtd_modified, (Pane *p, Generic baoi, Generic eaoi, Generic mode,
 Generic unp_node));
/* notify of change in the tree		*/
/* Takes 5 parameters p, baoi, eaoi, mode, unp_node.  					*/

EXTERN(void, sm_gtd_modified_extended, (Pane *p, Generic baoi, Generic eaoi,
 Generic mode, Generic unp_node, char StmtType));
/* notify of change in the tree	*/
/* Takes 6 parameters p, baoi, eaoi, mode, unp_node, StmtType 				*/

EXTERN(void, sm_gtd_shift, (Pane *p, Generic yshift));
/* shift the gtd display		*/
/* Takes 2 parameters p, yshift								*/

typedef FUNCTION_POINTER(int, sm_gtd_get_node_number_func, (char *nodename));
EXTERN(Generic, sm_gtd_init_unparse_table, (char *table_name, char **open,
 char **close, char **simple, sm_gtd_get_node_number_func Get_node_number));
/* Takes 5 parameters table_name, open, close, simple, Get_node_number, and
   returns 1 for success and 0 for failure						*/

EXTERN(void, sm_gtd_unmark_node, (Pane *p, Generic q));
/* takes 2 parameters - the pane id, and which node to remove the mark on */

EXTERN(void, sm_gtd_mark_node, (Pane *p, Generic q));
/* takes 2 parameters - the pane id, and which node to put the mark on    */

EXTERN(Boolean, sm_gtd_is_marked, (Pane *p, Generic q));
/* takes 2 parameters - the pane id, and the node in question--returns true if node has
   been marked */

EXTERN(Generic, sm_gtd_get_stmt_type, (Pane *p, Generic line));
/* takes 2 parameters - the pane id, and the line in the screen map in question--returns
 the stmt_type at that line */

EXTERN(void, sm_gtd_set_holophrasm, (Pane *p, Generic inlevel, Generic outlevel,
 Generic toplevel, Generic aroundlevel));
EXTERN(void, sm_gtd_query_holophrasm, (Pane *p, Generic *in, Generic *out,
 Generic *top, Generic *same));
EXTERN(void, sm_gtd_string_simple, (Pane *p, Generic node, char *s));
EXTERN(void, sm_gtd_string_open, (Pane *p, Generic node, char *s));
EXTERN(void, sm_gtd_string_close, (Pane *p, Generic node, char *s));
EXTERN(void, sm_gtd_line_string_simple, (Pane *p, Generic node, char *s,
 Generic indent));
EXTERN(void, sm_gtd_line_string_open, (Pane *p, Generic node, char *s,
 Generic indent));
EXTERN(void, sm_gtd_line_string_close, (Pane *p, Generic node, char *s,
 Generic indent));
EXTERN(Generic, sm_gtd_first, (Pane *p, Generic *node, Generic *type));
EXTERN(Generic, sm_gtd_last, (Pane *p, Generic *node, Generic *type));

EXTERN(int, screen_shuffle, (int first, int last));
EXTERN(void, dump_line, (int lines));

#define NIL 0

#define GTD_CENTER      (-2)
#define GTD_TOP         (-3)
#define GTD_BOTTOM      (-4)
#define GTD_OTHER       (-5)
#define GTD_CLEAR       (-6) /* just clear the pane */
#define GTD_CLEAR_OTHER (-7) /* invalidate the cached info, and unparse */


#define TABSIZE			2
#define TAB_COND_BREAK          ''
#define MAX_TEXT_LENGTH		3000
#define INFINITY		32000 

#define FBRC	''
#define OBRC	''
#define CBRC	''

/* and the definitions in this include file need to be accessed with gtd itself */
#include <libs/graphicInterface/oldMonitor/include/sms/gtd_ops.h>

#endif
