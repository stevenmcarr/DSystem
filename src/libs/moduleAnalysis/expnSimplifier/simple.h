/* $Id: simple.h,v 1.9 1997/06/25 15:10:31 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef	simple_h
#define	simple_h

#include <libs/frontEnd/ast/ast.h>
#include <libs/frontEnd/fortTree/fortsym.h>

struct tableau_entry_type;
typedef struct tableau_entry_type tableau_entry;
typedef tableau_entry **tableau_type;      /* The tableau is an extending array.
                                             Since we always know the size of
                                             the tableau before building it,
                                             we can allocate for the tableau
                                             the exact amount of memory that
                                             we are going to need. */

struct SymTable_NestLev_Type;
typedef struct SymTable_NestLev_Type *SymTab_NestLev_Type;
typedef struct SymTable_NestLev_Type  SymTab_NestLev_Struct;

/********************************************************************/
/* Locally defined structures exported by the simplifier	    */
/********************************************************************/


/* This is a NULL terminated list of pointers to character strings which
 * typically identify induction variables in a loop nest.  This list is
 * stored sequentially in an array allocated by the caller.
 * NOTE: it is acceptable for the first entry of the array to be NULL.
 */

typedef char	**nest_level_type;		

/* Maximum number of entries checked in the nest_level_type structure. */

#define	MAX_NEST_LEVEL_TYPE	20	


typedef FUNCTION_POINTER(tableau_entry *,ProcessTermFunc,(AST_INDEX,int,nest_level_type,
							  SymDescriptor));

/********************************************************************/
/* Locally defined functions exported by the simplifier		    */
/********************************************************************/

EXTERN(void, sim_simplify, (AST_INDEX passed_node, 
                            nest_level_type nesting_level_array, 
                            SymDescriptor D));

EXTERN(int, walk_simplify, (AST_INDEX stmt, int dummy, SymDescriptor sym_t));

EXTERN(int, simplify, (AST_INDEX passed_node, nest_level_type nesting_level_array,
                       SymDescriptor D));

EXTERN(void, walk_simplify_divide, (AST_INDEX passed_node, 
                                    nest_level_type nesting_level_array, SymDescriptor D));

EXTERN(AST_INDEX, find_negative_constants, (AST_INDEX root));

EXTERN(Boolean, invert_constant, (AST_INDEX root));

EXTERN(tableau_type, allocate_tableau, (int tableau_size));

EXTERN(int, get_term_count, (AST_INDEX passed_node));

EXTERN(int, get_factor_count, (AST_INDEX passed_node));

EXTERN(void, make_tableau, (AST_INDEX root, tableau_type tableau, int tableau_size,
                            ProcessTermFunc process_term, int left_op, 
                            nest_level_type nesting_level_array, SymDescriptor D));

EXTERN(tableau_entry*, process_term, (AST_INDEX root, int op, 
                                      nest_level_type nesting_level_array, SymDescriptor D));

EXTERN(tableau_entry*, process_factor, (AST_INDEX root, int op, 
                                        nest_level_type nesting_level_array, 
                                        SymDescriptor D));

EXTERN(tableau_entry*, recreate_term, (tableau_type tableau, int tableau_size,
                                       int op));

EXTERN(void, combine_terms, (tableau_type tableau, int tableau_size));

EXTERN(int, compute_val, (int op, int val1, int val2));

EXTERN(void, insert_unary_minus, (AST_INDEX root));

EXTERN(AST_INDEX, recreate_tree, (tableau_type tableau, int tableau_size));

EXTERN(int, score_node, (AST_INDEX node, nest_level_type nesting_level_array,
                         SymDescriptor D));

EXTERN(int, score_ref, (AST_INDEX root, nest_level_type nesting_level_array,
                        SymDescriptor D));

EXTERN(int, simp_divide, (AST_INDEX passed_node, SymTab_NestLev_Type SymTab_NestLev));

EXTERN(void, generate_new_constant, (int left_val, int right_val, 
                                     AST_INDEX passed_node));

EXTERN(tableau_entry*, allocate_tableau_entry, (void));

EXTERN(void, sort, (tableau_type tableau, int tableau_size, 
                    nest_level_type nesting_level_array, SymDescriptor D));

EXTERN(int, compare_trees, (AST_INDEX r1, AST_INDEX r2, 
                            nest_level_type nesting_level_array, SymDescriptor D));

EXTERN(Boolean, ast_equal, (AST_INDEX r1, AST_INDEX r2));

EXTERN(AST_INDEX, normalize_term, (AST_INDEX passed_node, AST_INDEX new_root));

EXTERN(int, nesting_level, (AST_INDEX root, nest_level_type nesting_level_array));

EXTERN(AST_INDEX, get_left_child, (AST_INDEX passed_node));

EXTERN(AST_INDEX, get_right_child, (AST_INDEX passed_node));

#endif
