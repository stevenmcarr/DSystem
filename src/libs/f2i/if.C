/* $Id: if.C,v 1.1 1997/04/28 20:18:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/support/lists/list.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/frontEnd/include/gi.h>
#include <stdio.h>

#include <libs/f2i/ai.h>
#include <libs/f2i/sym.h>
#include <libs/f2i/f2i_label.h>
#include <include/frontEnd/astnode.h>

#include <libs/f2i/mnemonics.h>

/* forward declarations */




/*  handles if and if, then, else constructs  */
void HandleIf (AST_INDEX node)

  // AST_INDEX	node;	/* if node */

  {
    /*  extract the first guard from the list of guards  */
	node = gen_IF_get_guard_LIST (node);
	node = list_first (node);

    /*  if the list is not empty, process the list  */
	if (node != ast_null_node)
	  {
	    /*  generate the if  */
		ExtendIf (node, LABEL_NEW);
	  }
  } /* HandleIf */



/*  recursively processes the list of guards in if, then, else constructs  */
void ExtendIf (AST_INDEX node, int end_if)

  // AST_INDEX	node;		/*  if node  */
  // int		end_if;		/*  label    */

  {
    register int	condition;	/*  register		*/
    register int	if_false;	/*  label		*/
    int			IlocLabel;	/*  statment label	*/
    AST_INDEX		StmtLabel;	/*  statement label node*/
    AST_INDEX		expression;	/*  expression node     */

    /*  process the statement as in aiStmtList  */

    /*  process the statement label  */
	StmtLabel = gen_GUARD_get_lbl_def(node); 

	if (StmtLabel != ast_null_node)
	  {
	    (void) LabelGet(gen_get_text(StmtLabel), &IlocLabel);
	    generate(IlocLabel, NOP, 0, 0, 0, "Label from GUARD statement");
	  }

    /*  generate the condition on the if  */
	expression = gen_GUARD_get_rvalue(node);

    /*  if we do not have an "else", generate code  */
	if (expression != ast_null_node)
	  {
	/*  check to see if we have a logical expression  */
	    if (gen_get_real_type(expression) != TYPE_LOGICAL)
		ERROR("ExtendIf", 
			"invalid expression type (not logical)", FATAL);

	/*  generate the expression  */
	    condition = getExprInReg (expression);

        /*  if condition is false, branch around the body  */
	    if (list_next(node) == ast_null_node)
	        if_false = end_if;
	    else
		    if_false = LABEL_NEW;
	    IlocLabel = LABEL_NEW;
	    generate(0, BR, IlocLabel, if_false, condition, "if (.false.) branch");
	    generate(IlocLabel, NOP, 0, 0, 0, NOCOMMENT);
	  }

    /*  generate the body of the if  */
	aiStmtList (gen_GUARD_get_stmt_LIST(node));

    /*  retrieve the next conditional node in the list  */
	node = list_next (node);

    /*  if list is empty, end the if statement  */
	if (node == ast_null_node)
	    generate (end_if, NOP, 0, 0, 0, "end of if");

	  /*  otherwise, generate an else or else-if block  */
	      else 
		{
		  /*  if previous condition was true, branch to end of if  */
		      generate (0, JMPl, end_if, 0, 0,
				"branch to end of if");

		  /*  generate the label definition for if_false label  */
		      generate (if_false, NOP, 0, 0, 0, "begin the else");

		  /*  generate an else-if  */
		      ExtendIf (node, end_if);
		}
  } /* ExtendIf */




/* generate intermediate code for a Fortran logical if statement */
void HandleLogicalIf (AST_INDEX node)

  // AST_INDEX	node;		/*  if node  */

  {
    register int	condition;	/*  register		*/
    register int	if_false;	/*  label		*/
    register int        next_stmt;      /*  label               */
    register AST_INDEX	expr;		/*  expression node     */

    /*  check to see if we have a logical expression  */
	expr = gen_LOGICAL_IF_get_rvalue(node);
	if (gen_get_real_type(expr) != TYPE_LOGICAL)
	  ERROR("HandleLogicalIf", "invalid expression type (not logical)", FATAL);

    /*  generate the condition on the if; if the   */
    /*  condition is false, branch around the body */
	condition = getExprInReg (expr);
	next_stmt = LABEL_NEW;
        if_false = LABEL_NEW;
	generate(0, BR, next_stmt, if_false, condition, "logical if condition");

    /*  generate the body of the if  */
	generate(next_stmt, NOP, 0, 0, 0, "Body of logical if");
	aiStmtList (gen_LOGICAL_IF_get_stmt_LIST(node));

    /*  generate the end of the if  */
	generate (if_false, NOP, 0, 0, 0, "end of logical if");
  } /* HandleLogicalIf */




/*  generate ILOC for an arithmetic if  */
void HandleArithmeticIf (AST_INDEX node)

  // AST_INDEX	node;	/* if node */

  {
    int			value;		/*  register      */
    int			zero_reg;	/*  register      */
    register int 	type;		/*  for condition */
    int			label;		/*  label	  */
    int			lt_label;	/*  label	  */
    STR_TEXT		AST_label;	/*  AST label	  */
    AST_INDEX		son;		/*  for condition */


    /*  compute the value of the condition  */
	son  		= gen_ARITHMETIC_IF_get_rvalue(node); 
	type 		= gen_get_real_type(son);

    /*  if the type is complex or double complex, the arithmetic if is invalid  */
	if((type == TYPE_COMPLEX) || (type == TYPE_DOUBLE_COMPLEX))
	    ERROR("HandleArithmeticIf", "Invalid expression type (complex or double complex)", FATAL);

    /*  compare expression value with zero */
	value = getExprInReg (son);
	zero_reg = (int) getConstantInRegFromString("0", TYPE_INTEGER, type);

    /*  generate the greater than branch  */
	AST_label = gen_get_text (
		    gen_ARITHMETIC_IF_get_lbl_ref3(node));
	LabelGet (AST_label, &label);
	generate_branch(0, GT, value, zero_reg, type,
		label, NO_TARGET, NOCOMMENT);

    /*  generate the equal to branch and the less than branch  */
	AST_label = gen_get_text (
		    gen_ARITHMETIC_IF_get_lbl_ref2(node));
	LabelGet (AST_label, &label);
	AST_label = gen_get_text (
		    gen_ARITHMETIC_IF_get_lbl_ref1(node));
	LabelGet (AST_label, &lt_label);
	generate_branch(0, EQ, value, zero_reg, type,
		label, lt_label, NOCOMMENT);

  } /* HandleArithmeticIf */
