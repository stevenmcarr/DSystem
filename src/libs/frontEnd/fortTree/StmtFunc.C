/* $Id: StmtFunc.C,v 1.3 1997/03/11 14:29:52 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************
 *                                                                      *
 *  StmtFunc -- stmt function expansion utility                         *
 *                                                                      *
 ************************************************************************/

#include <libs/frontEnd/fortTree/ft.h>

#include <libs/frontEnd/fortTree/fortsym.h>
#include <libs/frontEnd/include/stmt_func.h>

#include <libs/support/tables/cNameValueTable.h>


STATIC(void, replace_stmt_func_expression,
       (cNameValueTable ht, AST_INDEX *args, AST_INDEX expr_node));


AST_INDEX
expand_stmt_funcs(SymDescriptor d, AST_INDEX node)
{
  if (is_invocation(node))
  {
    AST_INDEX name, sf_expr;
    fst_index_t index;
    int sc;

    name = gen_INVOCATION_get_name(node);
    index = fst_QueryIndex(d, gen_get_text(name));
    sc = fst_GetFieldByIndex(d, index, SYMTAB_STORAGE_CLASS);
    if (sc & SC_STMT_FUNC)
    {
      sf_expr = stmt_func_invocation_to_expression(d, node);
      tree_replace(node, sf_expr);
      node = expand_stmt_funcs(d, sf_expr);
    }    
  }
  else if (is_list(node))
  {
    AST_INDEX elt, elt_next;
    for (elt = list_first(node); elt != AST_NIL; elt = elt_next)
    {
      elt_next = list_next(elt);
      (void) expand_stmt_funcs(d, elt);
    }
  }
  else
  {
    int i, n = gen_how_many_sons(gen_get_node_type(node));
    for (i = 1; i <= n; i++)
      (void) expand_stmt_funcs(d, ast_get_son_n(node,i));
  }
  return node;
}

AST_INDEX
stmt_func_invocation_to_expression(SymDescriptor d, AST_INDEX invocation_node)
{
	fst_index_t findex = fst_QueryIndex(d, 
		gen_get_text(gen_INVOCATION_get_name(invocation_node)));

	AST_INDEX sf_expr = tree_copy((AST_INDEX) 
		fst_GetFieldByIndex(d, findex, SYMTAB_SF_EXPR));

	int nargs = 
		fst_GetFieldByIndex(d, findex, SYMTAB_NARGS);

	if (nargs > 0) {
		int count = 0;
		AST_INDEX *arguments = new AST_INDEX[nargs];
		AST_INDEX current = list_first(
			gen_INVOCATION_get_actual_arg_LIST(invocation_node));

		while (current != AST_NIL) {
			arguments[count++] = current;
			current = list_next(current);
		}

		assert(count == nargs);

		cNameValueTable ht = 
                     (cNameValueTable)fst_GetFieldByIndex(d, findex, SYMTAB_FORMALS_HT);

		replace_stmt_func_expression(ht, arguments, sf_expr);
	}
	return sf_expr;
}

static void
replace_stmt_func_expression(cNameValueTable ht, AST_INDEX *args, AST_INDEX expr_node)
{
	if (expr_node == AST_NIL) return;

	if (is_list(expr_node)) {
		AST_INDEX expr_node_next;
		for (expr_node = list_first(expr_node);
		     expr_node != ast_null_node;
		     expr_node = expr_node_next)
		{
			expr_node_next = list_next(expr_node);
			replace_stmt_func_expression(ht, args, expr_node);
		}
	} else {
		int type = gen_get_node_type(expr_node);
		if (type == GEN_IDENTIFIER) {
			int value;
			if (NameValueTableQueryPair(ht,
				(Generic) gen_get_text(expr_node), (Generic*)&value)) {
				tree_replace(expr_node, tree_copy(args[value - 1]));
				tree_free(expr_node);
			}
		} else {
			int i;
			int n = ast_get_node_type_son_count(type);
			for (i=1; i<=n; i++)
			replace_stmt_func_expression(ht, args, 
				ast_get_son_n(expr_node, i));
		}
	}
}
