/* $Id: tree_util.h,v 3.2 1997/03/11 14:35:36 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *	treeutil.h	Utilities for accessing the AST
 *			mostly created for Ped, Cfg, and Cprop requirements.
 */

#ifndef _tree_util_h_
#define _tree_util_h_

EXTERN(Boolean, is_executable, (AST_INDEX stmt));
EXTERN(Boolean, is_loop, (AST_INDEX stmt));
EXTERN(int, loop_level, (AST_INDEX stmt));
EXTERN(AST_INDEX, gen_get_stmt_list, (AST_INDEX stmt));
EXTERN(AST_INDEX, gen_get_control, (AST_INDEX stmt));
EXTERN(AST_INDEX, gen_get_lbl_ref, (AST_INDEX stmt));

#endif
