/* $Id: params.C,v 1.1 1997/04/28 20:18:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <libs/support/misc/general.h>
#include <sys/file.h>

#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/support/lists/list.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/frontEnd/include/gi.h>
#include <libs/frontEnd/ast/asttree.h>
#include <libs/frontEnd/ast/aphelper.h>
#include <stdio.h>

#include <libs/f2i/ai.h>
#include <libs/f2i/sym.h>

#include <libs/f2i/mnemonics.h>

/* forward declarations */
static int FindParameters(AST_INDEX);
static void FixParameters(AST_INDEX);
static AST_INDEX FoldIntConstantExpr(AST_INDEX);



/* The routines in this file handle FORTRAN PARAMETER statements */


/* store value of the FORTRAN parameter in symbol table */
void MarkParameters(AST_INDEX Stmt)
  // AST_INDEX Stmt;
{
  AST_INDEX list;
  AST_INDEX name;
  AST_INDEX the_constant;
  int	    index;
  STR_TEXT  text;


  list = list_first(gen_PARAMETER_get_param_elt_LIST(Stmt));
  while(list != ast_null_node)
  {
    name         = gen_PARAM_ELT_get_name(list);
    the_constant = gen_PARAM_ELT_get_rvalue(list);

    text  = gen_get_text(name);
    index = fst_my_QueryIndex(ft_SymTable, text);
    if (index != SYM_INVALID_INDEX)    /* should be in symbol table */
    {
      SymInsertData(index, gen_get_real_type(name), 
		    OC_IS_DATA, 0, SC_NO_MEMORY, FALSE);
      
      /* stash away the constant node */
      fst_my_PutFieldByIndex(ft_SymTable, index, SYMTAB_EXPR, (Generic) the_constant); 
      fst_my_PutFieldByIndex(ft_SymTable, index, SYMTAB_fortran_parameter,  
			     FORTRAN_PARAMETER);
      
      /* evaluate it and replace the expression */
      if (gen_get_node_type(the_constant) != GEN_CONSTANT)
	{
	  FixParameters(the_constant);	/* update and replace it */
	  
	  the_constant = gen_PARAM_ELT_get_rvalue(list); /* update the copy */
	  fst_my_PutFieldByIndex(ft_SymTable, index, SYMTAB_EXPR, (Generic) the_constant); 
	  /* fix -cij 6/22/92 */

	  if (isParameterExpr(the_constant) == false)
	    {
	      (void) sprintf(error_buffer, 
			     "PARAMETER '%s' has non-constant value", text);
	      ERROR("MarkParameters", error_buffer, FATAL);
	    }
	  else if (ai_isConstantExpr(the_constant) == true &&
		   gen_get_real_type(the_constant) == TYPE_INTEGER)
	    {
	      fst_my_PutFieldByIndex(ft_SymTable, index, SYMTAB_EXPR,
				     (Generic) FoldIntConstantExpr(the_constant));
	    }
	}
    }
    else 
      {
	(void) sprintf(error_buffer, 
		       "Parameter '%s' in parameter statement not found in Symbol Table",
		       text);
	ERROR("MarkParameters", error_buffer, FATAL);
      }
    
    if (aiDebug > 0)
      (void) fprintf(stdout, "\tFound parameter '%s'.\n", text);
    
    list = list_next(list);
  }
} /* MarkParameters */




/* Process a node looking for PARAMETER statements.         */
/* Replace PARAMETERs in the AST with appropriate constant. */
void aiParameters( AST_INDEX node )
  // AST_INDEX node; /* a statement list */
{
  if (aiDebug > 0)
     (void) fprintf(stdout, "aiFindParameters().\n");  

  if (FindParameters(node) != 0)
  {
    FixParameters(node);
    if (aiDebug > 0)
    {
      (void) fprintf(stdout, "\n\nAfter parameter fix:\n");
      tree_print(node);
      (void) fprintf(stdout, "\n\n");
    }
  }

  if (aiDebug > 0)
     (void) fprintf(stdout, "\texiting aiFindParameters().\n");
} /* aiParameters */




/* detect all parameter statements and process */
static int FindParameters(AST_INDEX node)
  // AST_INDEX node; 	/* a statement list */
{
  AST_INDEX list;
  int type, done, found;

  list  = list_first(node);
  done  = 0;
  found = 0;

  while (list != ast_null_node && done == 0)
  {
     type = gen_get_node_type(list);
     if (type != GEN_COMMENT && ai_isExecutable(type) == 1)
	done = 1;
     else if (type == GEN_PARAMETER)
     {
	MarkParameters(list);
	found = 1;
     }

     list = list_next(list);
  }
  return found;
} /* FindParameters */




/* replace PARAMETER in AST with subtree representing the PARAMETER */
static void FixParameters(AST_INDEX node)
    // AST_INDEX node;
{
   register int i, n, type;
   AST_INDEX	model, nList, tempNode;

   if (node == AST_NIL) return;

   type = gen_get_node_type(node);

   if (is_list(node))
   {
     node = list_first(node);
     while(node != ast_null_node)
     {
       nList = list_next(node);
       FixParameters(node);
       node = nList;
     }
   }
   /* change 6/21/91
    * figure out the length of strings and update before 
    * replacing in the tree
    */
   else if (type == GEN_PARAM_ELT &&
		gen_get_real_type(gen_PARAM_ELT_get_name(node))
	    == TYPE_CHARACTER) 
     {
       tempNode = gen_PARAM_ELT_get_name(node);
       i = fst_my_QueryIndex(ft_SymTable, gen_get_text(tempNode));
       tempNode = gen_PARAM_ELT_get_rvalue(node);
       fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_CHAR_LENGTH,  
			      NewStringLength(tempNode));
     }
   else if (type == GEN_IDENTIFIER)
   {
	i = fst_my_QueryIndex(ft_SymTable, gen_get_text(node));
	if (i != SYM_INVALID_INDEX &&
	    fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_fortran_parameter) == 
	                                                     FORTRAN_PARAMETER)
	{
	  model = tree_copy_with_type(fst_my_GetFieldByIndex(ft_SymTable, i, 
							     SYMTAB_EXPR));
      	                                       /* handle new tree - cij 6/25/92 */
	  tree_replace(node, model);
	  tree_free(node);

	  if (aiDebug > 0)
	     (void) fprintf(stdout, "\tReplaced a copy of '%s'.\n", (char *) 
			    fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_NAME));
	  /* recurse downward, to catch definition of one parameter in
	   * terms of another 
	   */
	  FixParameters(model);
	}
      }
   else
     {
       n = ast_get_node_type_son_count(type);
       for (i=1; i<=n; i++)
	 FixParameters( ast_get_son_n(node, i) );
     }

 } /* FixParameters */




/* replace tree containing constant expression with a    */
/* tree containing the constant result of the expression */
static AST_INDEX FoldIntConstantExpr( AST_INDEX node )
  // AST_INDEX node;
{
  int		result;
  AST_INDEX	newNode;

  result  = evalExpr(node);
  newNode = gen_CONSTANT( );

  (void) sprintf(error_buffer, "%d", result);
  gen_put_text(newNode, error_buffer, STR_CONSTANT_INTEGER);
  gen_put_real_type(newNode, TYPE_INTEGER);
  gen_put_converted_type(newNode, TYPE_INTEGER);

  tree_replace(node, newNode);
  tree_free(node);

  return newNode;
} /* FoldIntConstantExpr */
