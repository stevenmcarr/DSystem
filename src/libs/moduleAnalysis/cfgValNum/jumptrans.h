/* $Id: jumptrans.h,v 1.7 1997/03/11 14:35:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/*
 * This file exists mainly to accomplish information hiding -- the point
 * is that routines which use this interface won't have to include a lot
 * of value-numbering, SSA, and CFG header files.
 */

#ifndef jumptrans_h
#define jumptrans_h

#include <libs/moduleAnalysis/valNum/val.h>
/*enum ValOpType;*/ 		/* minimal declaration -- complete one in val.h */

/* Hide the contents of this structure from the client. */
struct JumpTransInfo_struct_tag;
typedef struct JumpTransInfo_struct_tag *JumpTransInfo;

/* Abstract jump function */
typedef Generic JumpTransPtr;

/* Callback functions for building the jump function
*/
typedef FUNCTION_POINTER(JumpTransPtr,
			 jumptrans_createconst_func,
			 (int value));

typedef FUNCTION_POINTER(JumpTransPtr,
			 jumptrans_createvar_func,
			 (char *varname));

typedef FUNCTION_POINTER(JumpTransPtr,
			 jumptrans_createbottom_func,
			 (void));

typedef FUNCTION_POINTER(JumpTransPtr,
			 jumptrans_createop_func,
			 (ValOpType op, JumpTransPtr left, JumpTransPtr right));

typedef struct _jumptransinfo_callbacks {
  jumptrans_createconst_func create_constant;
  jumptrans_createvar_func create_var;
  jumptrans_createop_func create_op;
  jumptrans_createbottom_func create_bottom;
} jumptransinfo_callbacks;

/*
 * Initialize the interface for a given module (which may contain many
 * functions).
 */
EXTERN(JumpTransInfo, jump_trans_init,
		(FortTree ft, jumptransinfo_callbacks *callbacks));

/* Initialize the interface for a particular function in the module
*/
EXTERN(void, jump_trans_newfunction,
		(JumpTransInfo info, AST_INDEX rootnode));

/* Close down the interface 
*/
EXTERN(void, jump_trans_free, (JumpTransInfo info));

/*
 * Request a translation for the list of actuals at a call site.
 */
EXTERN(void, actual_list_to_jumpfunction_list,
		(char *fname, AST_INDEX actual_list,
		 int num_actuals, JumpTransPtr *ret_arr,
		 JumpTransInfo info));

/*
 * Request a translation of a loop bound expression (i.e. something
 * appearings in a loop upper/lower bound or step).
 */
EXTERN(JumpTransPtr, loopbound_expr_to_jumpfunction,
		(AST_INDEX expr, JumpTransInfo info));

/*
 * Request a translation of a conditional expression, i.e. something
 * appearing in a guard or if statement.
 */
EXTERN(JumpTransPtr, conditional_expr_to_jumpfunction,
		(AST_INDEX expr, JumpTransInfo info));


/* Request that debugging information be printed out.
*/
EXTERN(void, jump_trans_dump_debug_info, (FortTree ft));

#endif jumptrans_h

