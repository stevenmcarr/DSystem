/* $Id: ftx_ast.h,v 1.8 1997/03/11 14:29:27 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
# ifndef ftx_ast_h
# define ftx_ast_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/fortTree/fortsym.h>
#include <libs/frontEnd/include/gi.h>

#ifndef cNameValueTable_h
#include <libs/support/tables/cNameValueTable.h>
#endif

#ifndef rn_string_h
#include <libs/support/strings/rn_string.h>
#endif

typedef	enum {
  before,
  after
} before_after;

EXTERN(int, ftx_findScopes, (AST_INDEX node,
				     int nesting_level, 
				     va_list arg_list));

EXTERN(int, ftx_findLabels, (AST_INDEX	  node,
				     int	  nesting_level,
				     va_list	  arg_list));

EXTERN(Boolean, ftx_is_loop, ( FortTreeNode node ));
EXTERN(int,  ftx_genLabel, ( cNameValueTable labels ));
EXTERN(int,  ftx_getDef, ( FortTreeNode node ));
EXTERN(int,  ftx_getRef, ( FortTreeNode node ));
EXTERN(void, ftx_putRef, ( FortTreeNode node, int ref ));
EXTERN(void, ftx_insertCONTINUEafter, ( FortTreeNode node, int def));
EXTERN(void, ftx_insertInstr, (FortTreeNode instr,
				       before_after where,
				       FortTreeNode *l_node,
				       FortTreeNode *decl_insert_point,
				       FortTreeNode *pdecl_insert_point,
				       Boolean move_label));
EXTERN(void, ftx_insertDeclAt,(FortTreeNode	 *insert_point,
				       FortTreeNode	  decl));
EXTERN(Boolean, ftx_is_scope, (FortTreeNode node));
EXTERN(Boolean, ftx_is_executable_stmt, (FortTreeNode n));
EXTERN(Boolean, ftx_is_declaration, (FortTreeNode node));
EXTERN(Boolean, ftx_is_local, (SymDescriptor	 d, fst_index_t	 ind));
EXTERN(Boolean, ftx_is_fn_return_variable, (SymDescriptor	 d,
						    fst_index_t	 ind));
EXTERN(int, ftx_TreeNotEqual, (FortTreeNode t1,
				       FortTreeNode	 t2));
EXTERN(int, ftx_TreeHash, (FortTreeNode	 t,
				   unsigned int size));
EXTERN(AST_INDEX, ftx_declInsertPoint, (FortTreeNode scope));

EXTERN(FortTreeNode, ftx_gen_node_from_type, (Generic         type_num));
EXTERN(FortTreeNode, ftx_gen_ID, (char *s));
EXTERN(FortTreeNode, ftx_gen_CONST, (char *s, Generic t));
EXTERN(char *, ftx_entry_name, (FortTreeNode node));
EXTERN(FortTreeNode, ftx_parenthesize, (FortTreeNode node));
EXTERN(unsigned int, ftx_StrHash, (char *s, unsigned int size));
EXTERN(int, ftx_IntNotEqual, (int i, int j ));
EXTERN(unsigned int, ftx_IntHash, (int i, unsigned int size));

EXTERN(void, ftx_add_type_declaration,
		(FortTreeNode *decl_insert_point,
		 FortTreeNode *pdecl_insert_point,
		 FortTreeNode var,
		 int type));
EXTERN(void, ftx_add_external,
		(FortTreeNode *decl_insert_point,
		 FortTreeNode var));

EXTERN(FortTreeNode, ftx_gen_temp, (SymDescriptor d,
				   FortTreeNode *decl_insert_point,
				   FortTreeNode *pdecl_insert_point,
				   char *name_prefix,
				   int *name_counter,
				   int type));
# endif
