/* $Id: postmap.C,v 1.1 1997/04/28 20:18:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <libs/support/misc/general.h>
#include <libs/support/strings/rn_string.h>
#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/support/lists/list.h>
#include <libs/frontEnd/include/gi.h>
#include <libs/frontEnd/ast/gen.h>
#include <stdio.h>

#include <libs/f2i/ai.h>
#include <libs/f2i/sym.h>
#include <include/frontEnd/astnode.h>
#include <libs/f2i/f2i_label.h>
#include <libs/f2i/mnemonics.h>

/* forward declarations */

static void findModifiedVars(AST_INDEX);
static void checkActuals(AST_INDEX);
static void mark(AST_INDEX);



/* Walk the AST to detect all modified variables.  Mark the scratch */
/* field in the symbol table when a modified variable is found.     */
void aiPostMap(AST_INDEX node)
  // AST_INDEX node;
{
  register int i, n, recurse;

  if (node == AST_NIL)		/* quick, unstructured cutoff */
     return;

  /* Recurse through the list nodes */
  if (is_list(node))
  {
    node = list_first(node);
    while (node != AST_NIL)
    {
      aiPostMap(node);
      node = list_next(node);
    }
  }

  /* The following nodes may contain modifications */
  else 
  {
    switch(gen_get_node_type(node))
    { /* handle do, assign, assignment, invocation */
      case GEN_ASSIGNMENT:
	mark(gen_ASSIGNMENT_get_lvalue(node));
	recurse = TRUE;
	break;

      case GEN_ASSIGN:
	mark(gen_ASSIGN_get_name(node));
	recurse = TRUE;
	break;

      case GEN_DO:
	findModifiedVars(node);
	recurse = FALSE;
	break;

      case GEN_INVOCATION:
	checkActuals(node);
	recurse = FALSE;
	break;

      default:
	recurse = TRUE;
	break;
    }
    
    /* and recurse down when necessary */
    if (recurse == TRUE)
    {
      n = ast_get_node_type_son_count(gen_get_node_type(node));
      for (i=1; i<=n; i++)
	  aiPostMap(ast_get_son_n(node, i));
    }
  }
} /* aiPostMap */




/* Mark modified variables in the DO structure, handling    */
/* recursion as necessary.  Also mark scratch field in the  */
/* AST to indicate whether or not a loop index is modified. */
static void findModifiedVars(AST_INDEX node)
  //   AST_INDEX node;
{
  AST_INDEX son;
  register int i, n, index;

  son = gen_DO_get_control(node);
  if (gen_get_node_type(son) == GEN_INDUCTIVE)
  {
    son = gen_INDUCTIVE_get_name(son);
    index = getIndex(son);
    if (index == -1)
    {
      (void) sprintf(error_buffer, "Induction variable '%s' not in Symbol Table",
		     gen_get_text(son));
      ERROR("findModifiedVars", error_buffer, FATAL);
    }
    fst_my_PutFieldByIndex(ft_SymTable, index, SYMTAB_scratch,  FALSE);

    n = ast_get_node_type_son_count(gen_get_node_type(node));
    for (i=1; i<=n; i++)
    {
      if (i != DO_lbl_def)
	 aiPostMap(ast_get_son_n(node, i));
    }
    if (fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_scratch) == FALSE)
       ast_put_scratch(node, INDEX_NOT_MODIFIED);
    else
       ast_put_scratch(node, INDEX_MODIFIED);

    fst_my_PutFieldByIndex(ft_SymTable, index, SYMTAB_scratch,  TRUE);
  }
  else
  {
    n = ast_get_node_type_son_count(gen_get_node_type(node));
    for (i=1; i<=n; i++)
	aiPostMap(ast_get_son_n(node, i));
  }
} /* findModifiedVars */




/* Mark the modified variables by using  */
/* the scratch field in the symbol table */
static void mark(AST_INDEX node)
  // AST_INDEX node;
{
  AST_INDEX index;

  switch(gen_get_node_type(node))
  {
    case GEN_IDENTIFIER:
	fst_my_PutFieldByIndex(ft_SymTable, getIndex(node), SYMTAB_scratch,  TRUE);
	break;

    case GEN_SUBSCRIPT:
	fst_my_PutFieldByIndex(ft_SymTable, getIndex(gen_SUBSCRIPT_get_name(node)), SYMTAB_scratch,  TRUE);
	aiPostMap(gen_SUBSCRIPT_get_rvalue_LIST(node));
	break;

    case GEN_SUBSTRING:
	/* if substring has subscript, get the name of */
	/* the subscript rather than the substring */
	/* index is used as a temp variable */
	if (gen_get_node_type(index = gen_SUBSTRING_get_substring_name(node)) 
	    == GEN_SUBSCRIPT) {
	  index = getIndex(gen_SUBSCRIPT_get_name(index)); 
	}
	else {
	  index = getIndex(index);
	}
	fst_my_PutFieldByIndex(ft_SymTable, (int) index, SYMTAB_scratch,  TRUE);
	aiPostMap(gen_SUBSTRING_get_rvalue1(node));
	aiPostMap(gen_SUBSTRING_get_rvalue2(node));
	break;

    default:
	(void) sprintf(error_buffer, 
		"Unexpected node type (%d) in lhs of assignment", 
		gen_get_node_type(node));
	ERROR("mark", error_buffer, FATAL);
	break;
  }
} /* mark */




/* mark actual parameters */
static void checkActuals(AST_INDEX node)
  // AST_INDEX node;
{
  register int intrinsic, i;
  AST_INDEX list;


  if (iloc_intrinsic(gen_get_text(gen_INVOCATION_get_name(node))) != A2I_INVALID_OPCODE)
     intrinsic = TRUE;
  else 
     intrinsic = FALSE;

  if (aiDebug > 0)
    (void) fprintf(stdout, "checkActuals:  function name = %s, intrinsic? = %d\n",
		   (char *) gen_get_text(gen_INVOCATION_get_name(node)), intrinsic);

  list = list_first(gen_INVOCATION_get_actual_arg_LIST(node));
  while(list != ast_null_node)
  {
    switch(gen_get_node_type(list))
    {
      case GEN_IDENTIFIER:
	i = getIndex(list);
	break;

      case GEN_SUBSCRIPT:
	i = getIndex(gen_SUBSCRIPT_get_name(list));
	aiPostMap(gen_SUBSCRIPT_get_rvalue_LIST(list));
	break;

      case GEN_SUBSTRING:
	if (gen_get_node_type(gen_SUBSTRING_get_substring_name(list))
	    == GEN_SUBSCRIPT)
	  i = getIndex(gen_SUBSCRIPT_get_name(gen_SUBSTRING_get_substring_name(list)));
	else
	  i = getIndex(gen_SUBSTRING_get_substring_name(list));
	break;

      default:
	i = -1;
	break;
    }
    if (i != -1 && intrinsic == FALSE && aiNameIsMod(ft_SymTable,node, i) == TRUE)
	fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_scratch,  TRUE);
    list = list_next(list);
  }
} /* checkActuals */
