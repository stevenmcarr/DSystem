/* $Id: pt_util.h,v 1.13 1997/03/11 14:31:21 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*---------------------------------*/
/* global functions from pt_util.c */
/*---------------------------------*/

#ifndef pt_util_h
#define pt_util_h

#include <libs/frontEnd/ast/ast.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>

struct pt_ref_params;
typedef struct pt_ref_params Pt_ref_params;

EXTERN(char *,    ast2str,(PedInfo ped, AST_INDEX ast));
EXTERN(char *,    pt_get_stmt_text, (PedInfo ped, AST_INDEX num));

EXTERN(void,      pt_get_loop_bounds, (AST_INDEX stmt, AST_INDEX *ivar, 
				       AST_INDEX *lbound,
				       AST_INDEX *ubound,
				       AST_INDEX *stepsize));
EXTERN(Boolean,   pt_is_loop_with_inductive, (AST_INDEX loop_node));
EXTERN(Boolean,   pt_loop_is_normal, (AST_INDEX loop_node));
EXTERN(Boolean,   pt_loop_get_count, (AST_INDEX loop_node,
				      int *iter_cnt,
				      AST_INDEX *iter_cnt_node,
				      Boolean *loop_normal_flag));
EXTERN(void,      pt_flip_nodes, (AST_INDEX node1,AST_INDEX node2));
EXTERN(void,      pt_zero_var, (AST_INDEX expr,char *var));
EXTERN(void,      pt_check_min_max, (AST_INDEX node,char *var,
				    AST_INDEX expr));
EXTERN(int,       pt_convert_to_int, (char *str));
EXTERN(void,      pt_itoa, (int n,char *a));
EXTERN(AST_INDEX, pt_gen_comment, (char *str));
EXTERN(AST_INDEX, pt_gen_const, (char *str));
EXTERN(AST_INDEX, pt_gen_ident, (char *str));
EXTERN(AST_INDEX, pt_gen_label_ref, (char *str));
EXTERN(AST_INDEX, pt_gen_label_def, (char *str));
EXTERN(AST_INDEX, pt_gen_int, (int num));
EXTERN(AST_INDEX, pt_gen_func1, (char *fn,AST_INDEX node1));
EXTERN(AST_INDEX, pt_gen_add, (AST_INDEX node1,AST_INDEX node2));
EXTERN(AST_INDEX, pt_gen_sub, (AST_INDEX node1,AST_INDEX node2));
EXTERN(AST_INDEX, pt_gen_mul, (AST_INDEX node1,AST_INDEX node2));
EXTERN(AST_INDEX, pt_gen_div, (AST_INDEX node1,AST_INDEX node2));
EXTERN(AST_INDEX, pt_gen_mod, (AST_INDEX node1,AST_INDEX node2));
EXTERN(AST_INDEX, pt_gen_call,(char * name,AST_INDEX args));
EXTERN(AST_INDEX, pt_gen_invoke,(char * name,AST_INDEX args));
EXTERN(Boolean,   pt_comp_inv_name, (AST_INDEX node,char *name));
EXTERN(void,      pt_tree_replace, (AST_INDEX old_node,
				    AST_INDEX new_node));
EXTERN(AST_INDEX, pt_add_const, (AST_INDEX expr,int constant));
EXTERN(void,      pt_fold_term, (AST_INDEX exprin, AST_INDEX *exprout,
				 int *constout));
EXTERN(AST_INDEX, pt_del_unary_minus, (AST_INDEX expr));
EXTERN(AST_INDEX, pt_simplify_expr, (AST_INDEX expr));
EXTERN(AST_INDEX, pt_find_var_node, (AST_INDEX expr, char *var));
EXTERN(Boolean,   pt_find_var, (AST_INDEX expr,char *var));
EXTERN(void,      pt_sep_linear, (AST_INDEX expr,char *var,Boolean *lin,
				  AST_INDEX *factor,
				  AST_INDEX *constant));
EXTERN(void,      pt_separate_linear, (AST_INDEX expr,char *var,
				       Boolean *lin, AST_INDEX *factor,
				       AST_INDEX *constant));
EXTERN(void,      pt_get_coeff, (AST_INDEX expr,char *var,Boolean *lin,
				 int *coeff));
EXTERN(Boolean,   pt_expr_equal, (AST_INDEX expr1, AST_INDEX expr2));
EXTERN(int,       pt_perf_nest, (AST_INDEX node));
EXTERN(void,      pt_clear_info, (PedInfo ped,AST_INDEX tree));
EXTERN(void,      pt_var_replace, (AST_INDEX tree,char *var,
				  AST_INDEX new_expr));
EXTERN(void,      pt_var_add, (AST_INDEX tree,char*var,int value));
EXTERN(void,      pt_var_mul, (AST_INDEX tree,char*var,int value));
EXTERN(Boolean,   pt_eval, (AST_INDEX node,int *iptr));
EXTERN(AST_INDEX, pt_simplify_node, (AST_INDEX node));
EXTERN(Pt_ref_params*, pt_refs, (AST_INDEX tree, PedInfo ped));

EXTERN(AST_INDEX, pt_insert_before, (AST_INDEX target, AST_INDEX inject));
EXTERN(AST_INDEX, pt_insert_after, (AST_INDEX target, AST_INDEX inject));
EXTERN(int,       pt_unroll_estimate, (PedInfo ped, Boolean jam));
EXTERN(void,      pt_unroll_jam, (PedInfo ped, char *degree, Boolean jam));

#endif
