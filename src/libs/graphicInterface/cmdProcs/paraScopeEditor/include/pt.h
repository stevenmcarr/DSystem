/* $Id: pt.h,v 1.15 1997/06/25 13:52:01 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*----------------------------------------------------------------

	pt.h		Defines from pt routines

*/

#ifndef pt_h
#define pt_h

#ifndef pt_util_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>
#endif

#ifndef strong_h
#include <libs/moduleAnalysis/dependence/utilities/strong.h>
#endif

typedef FUNCTION_POINTER(void, ptDistribFunc,(AST_INDEX,Adj_List*,Generic));
typedef FUNCTION_POINTER(void, ptDivideFunc,(PedInfo,Adj_List*,int,AST_INDEX,Generic));

/********************************/
/*	function declarations		*/
/********************************/

/* EXTERN(Boolean, pt_isparallel, (Generic DG, AST_INDEX node, int level)); */
/*
 *   Generic	DG;
 *   AST_INDEX 	node;   DO node of the loop
 *   int  	level;  Level of nesting of the loop
 *   
 *   This routine determines  if the loop is parallel or not and returns 
 *   TRUE if it is , FALSE  otherwise
 */ 
 
EXTERN(void, pt_set_loop_type, (PedInfo ped, AST_INDEX node, int *loop_type, 
                                 int *par_status, int *tri_type)); 
 /*
  *  int        loop_type;    Shape of iteration space
  *  int        par_status;   Resulting parallelism
  *  int        tri_type;     Type of triangular/trapezoidal/pentagonal loop
  *
  *  To find out if two nested loops can be interchanged and the parallelism
  *  status of the two loops after interchange
  */  
  
EXTERN(void, pt_switch_loops, (PedInfo ped, AST_INDEX node, int loop_type, 
                                int tri_type));
 /*
  *  PedInfo	ped;
  *  int	level;
  *  int	loop_type;
  *  int	tri_type;
  *
  *  To interchange two nested loops 
  *  
  */    
  
EXTERN(Adj_List*,  pt_make_adj_list, (PedInfo ped, AST_INDEX node));
 /*
  * Creates the structure for adjacency list for
  * loop distribution   
  */

EXTERN(int, pt_split_loop, (Adj_List* adj_list, AST_INDEX node));
 /*
  *  Adj_List *adj_list;
  *  AST_INDEX node;
  *  
  *  Checks if the loop can be distributed in two parts starting
  *  from statement represernted by  node. Return code specifies
  *  if either or both the resulting loops are parallel
 */  

EXTERN(int, pt_find_distribution, (PedInfo ped, AST_INDEX loop, Generic PT,
                                   int type));
/*
 *  Finds the sections of the loop that will be grouped together
 *  after distribution.
 */

EXTERN(char*, pt_print_distribution, (Generic AL, Generic handle));
/*
 *  Generic AL;         
 *  Generic handle;
 * 
 *  Creating the message for the distribution dialog
 */

EXTERN(AST_INDEX, pt_do_distribution, (PedInfo ped, AST_INDEX do_node, int type));
/*
 *  Transforms the tree and the dependence graph to reflect the
 *  distribution.
 */

EXTERN(Boolean, pt_check_if_parallel, (Adj_List* adj_list, AST_INDEX node));
 /*
  *  Adj_List *adj_list;
  *  AST_INDEX node;
  *
  *  Checks if a statement , whose AST index is passed can become part
  *  of a parallel loop as a result of loop distribution.
  */

EXTERN(int, pt_can_reorder_stmts, (PedInfo ped, AST_INDEX index1, int level));
/*     
 *   index1 should be a simple statement
 */
 
EXTERN(AST_INDEX, pt_reorder_stmts, (AST_INDEX index1));
/*
 *    Should be called only if pt_can_reorder_stmts has returned 
 *    CAN_REORDER. Will change the tree to reflect reordering
 */

EXTERN(int, pt_handle_scexpnd_query1, (PedInfo ped, AST_INDEX node, int level, 
                                        char *var));
/*
 * Counts strongly connected regions
 */

EXTERN(int, pt_handle_scexpnd_query2, (PedInfo ped, AST_INDEX node, int level, 
                                        char *var));
/*
 * Counts parallel statements
 */

EXTERN(char*, pt_print_regions, (Generic AL, Generic Handle));


EXTERN(void, se_scalar_expand, (PedInfo ped, AST_INDEX loop, AST_INDEX selection));
EXTERN(int, se_test_scalar_expand, (PedInfo ped, AST_INDEX loop, AST_INDEX
                                    selection, char **msg));
EXTERN(Boolean, se_removable_edge, (PedInfo ped, EDGE_INDEX edge, char *var));


EXTERN(int, pt_test_delete_stmt, (PedInfo ped, AST_INDEX loop, AST_INDEX stmt,
                                   char **msg));

EXTERN(void, pt_delete_stmt, (PedInfo ped, AST_INDEX stmt));
/*
 *   Deletes stmt and returns true if its is correctly deleted. Only
 *   assignment statements within loops can be deleted. Loop must not
 *   contain any control flow.
 */

/* parallel->seq and seq->parallel routines -vas */

EXTERN(void, pt_make_parallel, (PedInfo ped, AST_INDEX node));
/*
 * Convert DO stmt (node) to a PARALLEL LOOP stmt with a PRIVATE stmt following
 * it. This is IBM PF syntax for a parallel loop.
 */

EXTERN(void, pt_make_sequential, (PedInfo ped, AST_INDEX node));
/*
 * Convert PARALLEL LOOP stmt (node) to a DO stmt. If a PRIVATE stmt follows
 * it is removed. 
 */

EXTERN(void, pt_peel_iteration, (PedInfo ped, AST_INDEX loop, Boolean
                                  iteration, int number_iterations));
/*
 * Peels off the first(1) or last(0) iteration of the loop.
 */

EXTERN(void, pt_strip_mine, (PedInfo ped, AST_INDEX loop, int step, 
                              char *stepstr));
/*  
 * ped - The dependence abstraction
 * loop- the loop
 * step - the step size of the loop
 *
 *  Adjusts the step size of the selected loop, loop, to be stepsize.
 *  Insert an inner loop to iterate over the portions of the iteration
 *  space. Insert a preloop to take care of uneven step divisions.
 *
 */

EXTERN(Boolean, pt_can_strip, (PedInfo ped));
/*
 * Returns true if the step size of the current loop is 1, false otherwise.
 */


EXTERN(int, find_distance_vector, (AST_INDEX src, AST_INDEX sink, STR_INDEX
                                   induction_var));

EXTERN(int, pt_is_protected, (PedInfo ped, EDGE_INDEX edge, AST_INDEX loop));
/*
 * Returns PROTECTED if the selected dependence is protected with synchronization.
 * Returns NOT_PROTECTED if is not.
 * Returns PERROR if there is not enough info to decide.
 */

EXTERN(void, pt_rename, (PedInfo ped, AST_INDEX loop, AST_INDEX selection));
/*
 *	Renames the selection to selection$.
 */

EXTERN(int, pt_test_renaming, (PedInfo ped, AST_INDEX loop, AST_INDEX selection,
                                char** msg));
/*
 * 	Returns CANNOT_CHANGE if the selection cannot be renamed.
 *	Returns CAN_CHANGE if it can.
 */


EXTERN(void, pt_build_do_edges, (PedInfo ped, Adj_List *adj_list, AST_INDEX node,
                                 int level, int depth));

EXTERN(void, pt_build_scexpnd_edges, (PedInfo ped, Adj_List *adj_list, AST_INDEX
                                      node, int level, int depth, char *var));

EXTERN(void, pt_build_stmt_edges2, (PedInfo ped, Adj_List *adj_list, AST_INDEX node,
                                   int level, int depth, char *var));

EXTERN(int, pt_count_par_stmts, (Adj_List *adj_list));

EXTERN(AST_INDEX, pt_do_user_distrib, (PedInfo ped, AST_INDEX do_node, 
				       ptDivideFunc pt_user_divide_loop, 
				       ptDistribFunc pt_user_post_distrib,
                                       Generic user_struct));

EXTERN(void, pt_divide_loop, (PedInfo ped, Adj_List *adj_list, int level, 
                             int type));

EXTERN(int, pt_look_back, (PedInfo ped, int last_node, Adj_List *adj_list,
                           AST_INDEX stmt, int level));

EXTERN(Boolean, pt_look_front, (PedInfo ped, int stmt_index, Adj_List *adj_list,
                                int level));

EXTERN(void, pt_make_loop_array, (Adj_List *adj_list));

EXTERN(AST_INDEX, pt_rebuild_tree, (PedInfo ped, Adj_List *adj_list, AST_INDEX
                                    old_do, int level));

EXTERN(void, pt_rebuild_graph, (PedInfo ped, Adj_List *adj_list, int level,
                                AST_INDEX first_do, AST_INDEX old_do));

EXTERN(AST_INDEX, find_new_loop, (int level, AST_INDEX stmt));




EXTERN(void, pt_get_arg_num, (AST_INDEX node, char *inductvar, int *arg_cnt,
                              int *arg_idx, int *coeff));

EXTERN(int, pt_process_switch, (AST_INDEX node, int nesting_level, Generic *parm));

EXTERN(int, pt_check_switch, (PedInfo ped, DG_Edge *edgeptr, AST_INDEX node,
                              int level));

EXTERN(int, pt_canswitch, (PedInfo ped, AST_INDEX node, int level));

EXTERN(int, pt_get_status, (int status1, int status2));


EXTERN(Boolean, pt_check_inner_control, (AST_INDEX inner_control, 
                                         char *outer_inductvar));


EXTERN(char, *se_can_expand, (PedInfo ped, AST_INDEX loop, AST_INDEX selection));



EXTERN(int, pt_skew_estimate, (PedInfo ped));

EXTERN(void, pt_skew, (char *skew_degree, AST_INDEX loop));

EXTERN(void, pt_split, (PedInfo ped, AST_INDEX loop, char *index_value));

EXTERN(void, pt_copy_dep, (PedInfo ped, DG_Edge *edgeptr, AST_INDEX loop,
                           int level, Pt_ref_params *old_refs, Pt_ref_params
			   *new_refs));

EXTERN(Boolean, pt_invariant_subs, (AST_INDEX node, AST_INDEX subs));

EXTERN(int, pt_rep_s_estimate, (PedInfo ped, char *msg));

EXTERN(void, pt_rep_s, (PedInfo ped));

EXTERN(void, walk_lhs, (PedInfo ped, AST_INDEX n));

EXTERN(void, walk_rhs, (PedInfo ped, AST_INDEX n));

EXTERN(void, pt_add_stmt, (PedInfo ped, char *stmt));

EXTERN(void, pt_copy_dep_deeper, (PedInfo ped, DG_Edge *edgeptr, Stack estack,
                                  AST_INDEX newloop, int level));

EXTERN(AST_INDEX, pt_create_func2, (char *name, AST_INDEX arg1, AST_INDEX arg2));

EXTERN(void, pt_get_arg_nodes, (AST_INDEX node, char *var, AST_INDEX *arg1,
                                AST_INDEX *arg2));

EXTERN(void, pt_switch_tri, (AST_INDEX node, int tri_type));

EXTERN(void, pt_switch_trap, (AST_INDEX node, int trap_type));

EXTERN(void, pt_switch_pent, (AST_INDEX node, int pent_type));

EXTERN(void, pt_switch_skew, (AST_INDEX node, int skew_type));

EXTERN(void, pt_switch_hex, (AST_INDEX node, int hex_type));

EXTERN(void, pt_adjust_deps, (PedInfo ped, DG_Edge *edgeptr, AST_INDEX node,
                              int level));

EXTERN(void, pt_adjust_stmt, (PedInfo ped, DG_Edge *edgeptr, AST_INDEX node,
                              int level));

EXTERN(void, pt_adjust_loop, (PedInfo ped, DG_Edge *edgeptr, AST_INDEX node,
                              int level));

EXTERN(void, pt_adjust_graph, (PedInfo ped, AST_INDEX node, int level));



/********************/
/*	definitions		*/
/********************/

/*--------------------------*/
/* return codes for routine canswitch. */

/* The code CANNOT_CHANGE (defined in dp.h) specifies that 
 *  loop interchange is not possible
 *  Other return codes signify that interchange is possible and determine
 *  and also reflect the parellelization status of th resulting inner and 
 *  outer loops.
 */ 

/*--------------------------*/
/* return code for split_loop routine for loop distribution  */ 

#define CANNOT_SPLIT    0

#define UPPER_ONLY      1
#define	OUTER_ONLY	1

#define LOWER_ONLY      2
#define INNER_ONLY	2

#define BOTH		3
#define NONE		4

#define COMPLEX_LOOP   -1
#define BAD_INNER_LOOP -2
#define BAD_OUTER_LOOP -3
#define BAD_DEP_LOOP   -4
#define OK_LOOP         0    /* used only for comparisons, not actually returned */
#define RECT_LOOP       1
#define SKEW_LOOP       2
#define TRI_LOOP        3
#define TRAP_LOOP       4
#define PENT_LOOP       5
#define HEX_LOOP        6

#define	PERROR		-1
#define	PROTECTED	1
#define	NOT_PROTECTED	2

/*--------------------------*/
/* return codes from statement reorder routine. */

#define CANNOT_REORDER 	0
#define CAN_REORDER	1
#define ILLEGAL_STMTS   -1

/*--------------------------*/
/* scalar expansion constants and routines */

#define PHASE1  0
#define PHASE2  0

/*--------------------------*/
/* defines for unroll & jam */

#define UNROLL_OK				 0
#define UNROLL_SYM_LOBOUND		-1
#define UNROLL_SYM_STEP			-2
#define UNROLL_IRREG_BOUND		-3
#define UNROLL_NEG_STEP			-4
#define UNROLL_DEPS				-5
#define UNROLL_ONELOOP			-6

/*---------------------------*/
/* defines for loop reversal */

#define REV_OKAY		0
#define REV_ILLEGAL		1
#define REV_UNABLE		2

/*---------------------------*/
/* defines for loop fusion   */

#define FUSE_LOOP		1	/* adjacent statements prevent fusion  */
#define FUSE_BOUND		2	/* loop bounds prevent fusion          */
#define FUSE_STEP		3	/* loop step prevent fusion            */
#define FUSE_DEP_ILL	4	/* fusion reverses dependences         */
#define FUSE_OK_CARRY	5	/* fusion ok, creates loop-carried dep */
#define FUSE_OK			6	/* fusion ok                           */

/*--------------------------------*/
/* defines for scalar replacement */

#define REP_S_BAD		0
#define REP_S_VAR		1
#define REP_S_INLOOP	2
#define REP_S_INNER		3
#define REP_S_NONE		4
#define REP_S_GOOD		5

/*-----------------------*/
/* defines for alignment */

#define ALIGN_OK		0
#define ALIGN_BAD		1
#define ALIGN_REP		2

/*------------------------------*/
/* defines for loop unswitching */

#define UNSWITCH_OKAY		0
#define UNSWITCH_NOIF		1
#define UNSWITCH_IFPLUS		2
#define UNSWITCH_VARIANT	3


/*----------------------------------------------*/
/* Granularity of partitions for distribution  	*/

#define    PG_PARALLEL  66 /* largest parallel and sequential			*/
#define    PG_VECTOR    67 /* finest parallel and largest sequential 		*/
#define    PG_DEEPNESTS 68 /* create perfect nests into the innermost loop	*/
#define    PG_NESTS     69 /* create loop nests at the current level, no deeper	*/
#define    PG_FINEST    70

#endif
