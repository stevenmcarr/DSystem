/* $Id: groups.h,v 1.10 1997/03/11 14:29:31 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef	groups_h
#define	groups_h

/* some useful groupings of ASTnodes */

#ifndef	general_h
#include <libs/support/misc/general.h>
#endif

#ifndef	ast_h
#include <libs/frontEnd/ast/ast.h>
#endif

/* -------------- approved functions ------------ */

EXTERN(Boolean, is_f77_subprogram_stmt, (AST_INDEX n));      
EXTERN(Boolean, is_f77_initial_specification_stmt, (AST_INDEX n)); 
EXTERN(Boolean, is_f77_other_specification_stmt, (AST_INDEX n)); 
EXTERN(Boolean, is_f77_final_specification_stmt, (AST_INDEX n)); 
EXTERN(Boolean, is_f77_specification_stmt, (AST_INDEX n));   
EXTERN(Boolean, is_f77_executable_stmt, (AST_INDEX n));      
EXTERN(AST_INDEX, first_f77_executable_stmt, (AST_INDEX stmt_list));
EXTERN(Boolean, is_smp_extension_other_specification_stmt, (AST_INDEX n));
EXTERN(Boolean, is_smp_specification_stmt, (AST_INDEX n));   
EXTERN(Boolean, is_smp_extension_executable_stmt, (AST_INDEX n));
EXTERN(Boolean, is_smp_loop_specification_stmt, (AST_INDEX n));
EXTERN(Boolean, is_smp_loop_stmt, (AST_INDEX n));
EXTERN(Boolean, is_smp_executable_stmt, (AST_INDEX n));    
EXTERN(AST_INDEX, first_smp_executable_stmt, (AST_INDEX stmt_list));
EXTERN(AST_INDEX, first_smp_executable_stmt_in_loop, (AST_INDEX stmt_list));
EXTERN(Boolean, is_f77_statement, (AST_INDEX n));          
EXTERN(Boolean, is_f77_operator, (AST_INDEX n));   
EXTERN(AST_INDEX, first_executable_stmt, (Boolean (*pre_cond)(), Boolean
                                          (*spec_cond)(), Boolean
                                          (*post_cond)(), AST_INDEX stmt_list));            

#define is_smp_subprogram_stmt(n)	is_f77_subprogram_stmt(n)
#define is_smp_operator(n)		is_f77_operator(n)

#define is_operator(n) 			is_smp_operator(n)
#define is_loop_stmt(n) 		is_smp_loop_stmt(n)
#define is_subprogram_stmt(n) 		is_smp_subprogram_stmt(n)
#define is_executable_stmt(n) 		is_smp_executable_stmt(n)
#define is_specification_stmt(n) 	is_smp_specification_stmt(n)
#define is_common_stmt(n) 		is_smp_common_stmt(n)

/* -------------- deprecated interface ------------ */

#define loop_stmt(n) 		is_loop_stmt(n)
#define executable_stmt(n) 	is_executable_stmt(n)
/*
   bad form to have macros with lowercase names.
#define operator(n) 		is_operator(n)
*/
#define sub_expr_op(n) 		cprop_sub_expr_op(n)
#define inquiry_stmt(n) 	cprop_inquiry_stmt(n)
#define labelled_stmt(n) 	cprop_labelled_stmt(n)
#define new_instance(n) 	cprop_new_instance(n)
#define poss_gets_node(n) 	cprop_poss_gets_node(n)
#define file_pos_stmt(n) 	cprop_file_pos_stmt(n)
#define io_stmt(n) 		is_io_keyword(n)
#define stmt_containing_expr(n) cprop_stmt_containing_expr(n)
#define has_stmt_list(n) 	cprop_has_stmt_list(n)
#define has_close_lbl_def(n) 	cprop_has_close_lbl_def(n)
#define used_node(n) 		cprop_used_node(n)
#define control_flow(n) 	cprop_control_flow(n)
#define poss_alternate_edges(n) cprop_poss_alternate_edges(n)
		/* loops are different kind of alt edge - last statement
  		    multiply used adds edges */
#define usable_exp(t,n)        	cprop_usable_exp(t,n)

#define common_stmt(n)          cprop_common_stmt(n)
#define exit_stmt(n)		cprop_exit_stmt(n)
#define struct_guard(n)		cprop_struct_guard(n)
#define array_ident(t,n)	cprop_array_ident(t,n)
#define dummy_proc(t,n)		cprop_dummy_proc(t,n)

EXTERN(Boolean, is_loop, (AST_INDEX n));              
EXTERN(Boolean, is_executable, (AST_INDEX n));
EXTERN(Boolean, is_io_keyword, (AST_INDEX n));
EXTERN(Boolean, cprop_loop_stmt, (AST_INDEX n));
EXTERN(Boolean, cprop_executable_stmt, (AST_INDEX n));
EXTERN(Boolean, cprop_operator, (AST_INDEX n));
EXTERN(Boolean, cprop_sub_expr_op, (AST_INDEX n));
EXTERN(Boolean, cprop_inquiry_stmt, (AST_INDEX n));
EXTERN(Boolean, cprop_common_stmt, (AST_INDEX n));
EXTERN(Boolean, cprop_labelled_stmt, (AST_INDEX n));
EXTERN(Boolean, cprop_new_instance, (AST_INDEX n));
EXTERN(Boolean, cprop_poss_gets_node, (AST_INDEX n));
EXTERN(Boolean, cprop_file_pos_stmt, (AST_INDEX n));
EXTERN(Boolean, cprop_io_stmt, (AST_INDEX n));
EXTERN(Boolean, cprop_stmt_containing_expr, (AST_INDEX n));
EXTERN(Boolean, cprop_has_stmt_list, (AST_INDEX n));
EXTERN(Boolean, cprop_has_close_lbl_def, (AST_INDEX n));
EXTERN(Boolean, cprop_used_node, (AST_INDEX n));
EXTERN(Boolean, cprop_control_flow, (AST_INDEX n));
EXTERN(Boolean, cprop_poss_alternate_edges, (AST_INDEX n));
EXTERN(Boolean, cprop_usable_exp, (Generic t, AST_INDEX n));
EXTERN(Boolean, cprop_exit_stmt, (AST_INDEX n));
EXTERN(Boolean, cprop_struct_guard, (AST_INDEX n));
EXTERN(Boolean, cprop_array_ident, (Generic t, AST_INDEX n));
EXTERN(Boolean, cprop_dummy_proc, (Generic t, AST_INDEX n));

#endif
