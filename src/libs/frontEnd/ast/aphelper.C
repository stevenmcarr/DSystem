/* $Id: aphelper.C,v 1.1 1997/06/24 17:41:50 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <string.h>

#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astlist.h>
#include <libs/frontEnd/ast/asttree.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/frontEnd/include/gi.h>
#include <libs/frontEnd/ast/aphelper.h>

/*
 * If node is not an error node, then go to the first node in 
 * node's stmt list. If it is an error node, then go in to its
 * subtree.
 */
AST_INDEX
in (AST_INDEX node)
{
    Generic         son;

    if (is_compound (node))
    {
	son = in_son (node);
	return list_first (ast_get_son_n (node, son));
    }
    return AST_NIL;
}

AST_INDEX
in_to_end (AST_INDEX node)
{
    Generic         son;

    if (is_compound (node))
    {
	son = in_son (node);
	return list_last (ast_get_son_n (node, son));
    }
    return AST_NIL;
}

/*
 * If node is not a stmt node, then chase fathers until at a stmt.
 * If node is a stmt node, then chase fathers until at a compound node.
 */
AST_INDEX
out (AST_INDEX node)
{
    if (is_statement (node))
    {
	AST_INDEX       t = tree_out (node);

	while (t != AST_NIL && !is_compound (t))
	{
	    t = tree_out (t);
	}
	return t;
    }
    else
    {
	AST_INDEX       t = tree_out (node);

	while (t != AST_NIL && !is_statement (t))
	{
	    t = tree_out (t);
	}
	return t;
    }
}

/*
 * Go to the next node in a list.
 */
AST_INDEX
next (AST_INDEX node_in_list)
{
    return list_next (node_in_list);
}

/*
 * Go to the previous node in a list.
 */
AST_INDEX
prev (AST_INDEX node_in_list)
{
    return list_prev (node_in_list);
}

/*
 * Placeholder and metatype routines.
 */
AST_INDEX
place_holder_for (AST_INDEX node)
{
    AST_INDEX       n = gen_PLACE_HOLDER ();

    gen_put_meta_type (n, get_proper_meta_type (node));
    gen_put_status (n, get_proper_status (node));
    return n;
}

AST_INDEX
ph_from_mtype (Generic mtype)
{
    AST_INDEX       n = gen_PLACE_HOLDER ();

    gen_put_meta_type (n, THE_MTYPE(mtype));
    return n;
}

/* annotate a node with placeholder information */
AST_INDEX
ph_ify (AST_INDEX node, TYPE ph_type, Generic ph_status)
{
    gen_put_meta_type (node, ph_type);
    gen_put_status (node, ph_status);
    return node;
}

/* make a required placeholder */
AST_INDEX
gen_rph (META_TYPE phtype)
{
    AST_INDEX       ph;

    ph = gen_PLACE_HOLDER ();
    gen_put_meta_type (ph, phtype);
    gen_put_status (ph, PLACE_REQUIRED);
    return ph;
}

/* make an optional placeholder */
AST_INDEX
gen_oph (META_TYPE phtype)
{
    AST_INDEX       ph;

    ph = gen_PLACE_HOLDER ();
    gen_put_meta_type (ph, phtype);
    gen_put_status (ph, PLACE_OPTIONAL);
    return ph;
}

AST_INDEX
gen_any_list (AST_INDEX list)
{
    /* Make sure that we have a list */
    if (list == AST_NIL)
    {
	list = list_create (gen_any_node (AST_NIL));
    }
    else if (list_empty (list))
    {
	(void) list_insert_first (list, gen_any_node (AST_NIL));
    }
    return list;
}

AST_INDEX
gen_any_node (AST_INDEX node)
{
    if (node == AST_NIL)
	node = ast_alloc (GEN_PLACE_HOLDER);
    return node;
}

AST_INDEX
gen_no_meta_list (AST_INDEX list, Generic status, META_TYPE type)
{
    AST_INDEX       curr;

    if (list == AST_NIL)
	return list;

 /* commented out by dcs 5/20/92 
    gen_put_meta_type (list, type);
    gen_put_status    (list, status); */

    for (curr = list_first(list);
	 curr != AST_NIL;
	 curr = list_next(curr))
    {
	if (ast_get_node_type(curr) != GEN_PLACE_HOLDER)
	{/* do not reset the metatype of placeholders */
	    gen_put_meta_type (curr, type);
	    gen_put_status (curr, status);
        }
    }

   return list;
}

AST_INDEX
gen_meta_list (AST_INDEX list, Generic status, META_TYPE type)
{
    AST_INDEX       curr,
                    end;

    list = gen_any_list (list);
 
    /*
      lists no longer have metatypes or status
      gen_put_meta_type (list, type);
      gen_put_status (list, status);
     */

    curr = list_first (list);
    end = list_last (list);

    while (curr != end)
    {
	gen_put_meta_type (curr, type);
	gen_put_status (curr, status);
	curr = list_next (curr);
    }

    gen_put_meta_type (curr, type);
    gen_put_status (curr, status);
    return list;
}

AST_INDEX
gen_no_meta_node (AST_INDEX node, Generic status, META_TYPE type)
{
    if (node != AST_NIL && ast_get_node_type(node) != GEN_PLACE_HOLDER)
    {
    	gen_put_meta_type (node, type);
    	gen_put_status (node, status);
    }
    return node;
}

AST_INDEX
gen_meta_node (AST_INDEX node, Generic status, META_TYPE type)
{
    node = gen_any_node (node);
    gen_put_meta_type (node, type);
    gen_put_status (node, status);
    return node;
}

Boolean
is_first_in_list (AST_INDEX node_in_list)
{
    return BOOL (
		 in_list (node_in_list) &&
		 BOOL (list_prev (node_in_list) == AST_NIL));
}

Boolean
is_last_in_list (AST_INDEX node_in_list)
{
    return BOOL (in_list (node_in_list) &&
		 BOOL (list_next (node_in_list) == AST_NIL));
}

Boolean
is_only_in_list (AST_INDEX node_in_list)
{
    return BOOL (in_list (node_in_list) &&
		 BOOL (list_next (node_in_list) == AST_NIL) &&
		 BOOL (list_prev (node_in_list) == AST_NIL));
}

Boolean
is_scope (AST_INDEX node)
{
    switch (ast_get_node_type(node))
    {
	case GEN_GLOBAL:
	case GEN_PROGRAM:
	case GEN_SUBROUTINE:
	case GEN_FUNCTION:
	case GEN_BLOCK_DATA:
	    return true;
	default:
	    return false;
    }
}


Boolean is_subpgm(AST_INDEX node)
{
  switch (ast_get_node_type(node)) {
  case GEN_PROGRAM:
  case GEN_SUBROUTINE:
  case GEN_FUNCTION:
    return true;
  default:
    return false;
  }
}

AST_INDEX 
find_scope (AST_INDEX node)
{
    /* move out to a scope */
    while (node != AST_NIL && !is_scope(node)) 
	node = tree_out(node);
    return node;
}

AST_INDEX 
get_name_in_entry(AST_INDEX node)
{
    switch (ast_get_node_type(node))
    {
	case GEN_PROGRAM:
	     return gen_PROGRAM_get_name(node);
	case GEN_FUNCTION:
	     return gen_FUNCTION_get_name(node);
	case GEN_SUBROUTINE:
	     return gen_SUBROUTINE_get_name(node);
	case GEN_BLOCK_DATA:
	     return gen_SUBROUTINE_get_name(node);
	case GEN_ENTRY:
	     return gen_ENTRY_get_name(node);
	default:
	     return AST_NIL;
    }
}

AST_INDEX 
get_stmts_in_scope(AST_INDEX node)
{
    switch (ast_get_node_type(node))
    {
	case GEN_GLOBAL:
	     return gen_GLOBAL_get_subprogram_scope_LIST(node);
	case GEN_PROGRAM:
	     return gen_PROGRAM_get_stmt_LIST(node);
	case GEN_FUNCTION:
	     return gen_FUNCTION_get_stmt_LIST(node);
	case GEN_SUBROUTINE:
	     return gen_SUBROUTINE_get_stmt_LIST(node);
	default:
	     return AST_NIL;
    }
}

AST_INDEX 
get_formals_in_entry(AST_INDEX node)
{
    switch (ast_get_node_type(node))
    {
	case GEN_FUNCTION:
	     return gen_FUNCTION_get_formal_arg_LIST(node);
	case GEN_SUBROUTINE:
	     return gen_SUBROUTINE_get_formal_arg_LIST(node);
	case GEN_ENTRY:
	     return gen_ENTRY_get_formal_arg_LIST(node);
	default:
	     return AST_NIL;
    }
}

char           *
gen_status_get_text (Generic value)
{
    switch (value)
    {
	case PLACE_OPTIONAL:
	    return "OPTIONAL";
	case PLACE_REQUIRED:
	    return "REQUIRED";
	default:
	    return "UNKNOWN";
    }
}

char           *
gen_type_get_text (Generic type)
{
    switch (type)
    {
	case TYPE_UNKNOWN:
	    return "UNKNOWN";
	case TYPE_INTEGER:
	    return "INTEGER";
	case TYPE_REAL:
	    return "REAL";
	case TYPE_DOUBLE_PRECISION:
	    return "DOUBLE_PRECISION";
	case TYPE_EXACT:
	    return "EXACT_PRECISION";
	case TYPE_COMPLEX:
	    return "COMPLEX";
	case TYPE_LOGICAL:
	    return "LOGICAL";
	case TYPE_LABEL:
	    return "LABEL";
	case TYPE_CHARACTER:
	    return "CHARACTER";
	case TYPE_ERROR:
	    return "ERROR";
	default:
	    return "ILLEGAL";
    }
}

int_type (char *type_str)
{
    if (strcmp (type_str, "LABEL") == 0)
	return TYPE_LABEL;
    if (strcmp (type_str, "INTEGER") == 0)
	return TYPE_INTEGER;
    if (strcmp (type_str, "REAL") == 0)
	return TYPE_REAL;
    if (strcmp (type_str, "DOUBLE_PRECISION") == 0)
	return TYPE_DOUBLE_PRECISION;
    if (strcmp (type_str, "EXACT_PRECISION") == 0)
	return TYPE_EXACT;
    if (strcmp (type_str, "COMPLEX") == 0)
	return TYPE_COMPLEX;
    if (strcmp (type_str, "LOGICAL") == 0)
	return TYPE_LOGICAL;
    if (strcmp (type_str, "CHARACTER") == 0)
	return TYPE_CHARACTER;
    return TYPE_UNKNOWN;
}

char           *
gen_type_get_text_short (Generic type)
{
    switch (type)
    {
	case TYPE_UNKNOWN:
	    return "UNKN";
	case TYPE_INTEGER:
	    return "INT";
	case TYPE_REAL:
	    return "REAL";
	case TYPE_DOUBLE_PRECISION:
	    return "DBLP";
	case TYPE_EXACT:
	    return "EXAC";
	case TYPE_COMPLEX:
	    return "CPLX";
	case TYPE_LOGICAL:
	    return "LGCL";
	case TYPE_LABEL:
	    return "LABL";
	case TYPE_CHARACTER:
	    return "CHAR";
	case TYPE_ERROR:
	    return "ERR";
	default:
	    return "ILL";
    }
}

/* figure out the proper meta type here, by looking at the node
definition. Find out which son we have, then look in the node
definition array and find the proper metatype. This node must
be attached.... */
Generic
get_proper_meta_type (AST_INDEX node)
{
    AST_INDEX       father;
    Generic         which;
    META_TYPE       type;

    if (in_list (node))
	node = list_head (node);
    father = tree_out (node);

    if (father == AST_NIL)
    {
	return gen_get_meta_type (node);
    }

    which = ast_which_son (father, node);
    type = nodedef[ast_get_node_type (father)].sons[which - 1];
    return THE_TYPE(type);
}


Generic
get_proper_status (AST_INDEX node)
{
    AST_INDEX       father;
    Generic         which;
    META_TYPE       type;

    if (in_list (node))
	node = list_head (node);
    father = tree_out (node);
    which = ast_which_son (father, node);
    type = nodedef[ast_get_node_type (father)].sons[which - 1];
    if (is_OPTIONAL (type))
	return PLACE_OPTIONAL;
    else
	return PLACE_REQUIRED;
}


