/* $Id: goto.C,v 1.2 1997/06/25 15:21:51 carr Exp $ */
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
#include <libs/f2i/f2i_label.h>
#include <libs/f2i/sym.h>
#include <include/frontEnd/astnode.h>

#include <libs/f2i/mnemonics.h>

/* variable used to generate label tables */
int	label_table_number = 1;

/* forward declarations */




/* generate ILOC for a FORTRAN GOTO statement from an AST node */
void HandleGoto( AST_INDEX node )
  // AST_INDEX	node;	/*  GEN_GOTO AST node  */
{
  register AST_INDEX 	label;		/* AST node for target label */
  register STR_TEXT  	AST_label;	/* text version of label     */
  int			iloc_label;	/* ILOC number for label     */

  label = gen_GOTO_get_lbl_ref(node);

  /* if the target label is valid, generate the goto */
  if (gen_get_node_type(label) == GEN_LABEL_REF)
  {
    AST_label = gen_get_text(label);
    LabelGet( AST_label, &iloc_label );
    generate(0, JMPl, iloc_label, 0, 0, NOCOMMENT);
  }
  else 
     ERROR("HandleGoto", "target is not a label", FATAL);
}  /* HandleGoto */




/* generate ILOC for a FORTRAN computed GOTO statement from an AST node */
void HandleComputedGoto( AST_INDEX node )
  // AST_INDEX	node;		/* GEN_COMPUTED_GOTO AST node */
{
  register int	index,		/* register with label table index */
		expr,   	/* register containing expression  */
		begin_table,	/* label beginning goto table      */
		end_goto,	/* label at end of computed goto   */
		count,		/* number of labels in the table   */
		i;		/* loop index			   */

  AST_INDEX	list,		/* list of label references        */
		list_element;   /* element of the label list	   */

  STR_TEXT	AST_label;	/* label reference in the AST      */
  int		label;		/* iloc value assigned to AST_label*/

  int		four,
		base,
		offset,
		addr,
		num,
		contents;

  char		buffer[128],
  		label_list[700];
  			
  char		*label_list_ptr;

  /*  if we have a valid computed goto list, generate the computed goto  */
      list = gen_COMPUTED_GOTO_get_lbl_ref_LIST(node);
      if (is_list(list))
      {
	/*  compute the number of targets in the computed goto  */
	count = 0;
	list_element = list_first (list);
	while (list_element != ast_null_node)
	{
	  count++;
	  list_element = list_next(list_element);
	}

	/*  generate the expression used to select the label  */
	expr = getExprInReg(gen_COMPUTED_GOTO_get_rvalue(node));

	/* force the expression to be an integer */
	if (fst_my_GetFieldByIndex(ft_SymTable, expr, SYMTAB_TYPE) !=
		TYPE_INTEGER)
	  expr = getConversion(expr, TYPE_INTEGER);
	  
	/*  if the number of targets is small, generate naive code  */
	if (count < 4)
	{
	  list_element = list_first(list);
	  for (i = 1; i <= count; i++)
	  {
	    AST_label = gen_get_text(list_element);
	    LabelGet(AST_label, &label);

	    num   = getConstantInRegFromInt(i);

	    (void) sprintf(buffer, "Branch to choice number %d", i);
	    generate_branch(0, EQ, num, expr, TYPE_INTEGER, 
		label, NO_TARGET, buffer);

	    list_element = list_next(list_element);
	  }

	  /* no branch taken implies fall through to next stmt */
	  generate(0, NOP, 0, 0, 0, "end of computed goto");
	}
	/*  generate efficient code that uses TBLs  */
	else
	{
	    /*  generate a label to denote the end of the goto  */
	  	end_goto = LABEL_NEW;

	    /*  determine if expression computed is below the range  */
		generate_branch(0, LE, expr,
			getIntConstantInRegister("0"),
			TYPE_INTEGER, end_goto, NO_TARGET, "skip if <= 0");

	    /*  determine if expression computed is above the range  */
		generate_branch(0, GT, expr, (int) getConstantInRegFromInt(count),
			TYPE_INTEGER, end_goto, NO_TARGET, "skip if > u.b.");

	    /*  generate the address of the label in the goto table  */
		/* get the base address */
		begin_table = LABEL_NEW;
		index = StrTempReg("(-L)+", begin_table, TYPE_INTEGER);
		generate(0, iLDI, begin_table, index, GEN_LABEL, 
			  "read label from table");
		if (aiLongIntegers)
		    four = getIntConstantInRegister("8");
		else
		    four = getIntConstantInRegister("4");
		base = TempReg(index, four, iSUB, TYPE_INTEGER);
		generate(0, iSUB, index, four, base, NOCOMMENT);

		/* compute an offset */
		offset = TempReg(expr, four, iMUL, TYPE_INTEGER);
		generate(0, iMUL, expr, four, offset, "Element length is Integer");

		/* compute name for label table */
		(void) sprintf(buffer, "@L_T_%d", label_table_number++);

		/* get the contents of MEM(base+offset) */
		addr = TempReg(base, offset, iADD, TYPE_INTEGER);
		generate(0, iADD, base, offset, addr, NOCOMMENT);
		contents = StrTempReg("!", addr, TYPE_INTEGER);
		generate_long(0, iCONor, (Generic) buffer, 4, 0, addr, contents, 0, 0, NOCOMMENT);

		/* generate list of potential labels */
		list_element = list_first (list);
		label_list_ptr = label_list;
		while (list_element != ast_null_node)
		  {
		    AST_label = gen_get_text(list_element);
		    LabelGet (AST_label, &label);
		    (void) sprintf(label_list_ptr, "LL%-4.4d ", label);
		    label_list_ptr = label_list_ptr + 7;
		    list_element = list_next(list_element);
		  }

	    /*  generate branch to label in the computed goto table  */
		generate(0, JMPr, contents, (Generic) label_list, 0, NOCOMMENT);

	    /*  generate DATA statements for computed goto table  */
		generate(0, NOP, 0, 0, 0, "Computed GOTO table");
		list_element = list_first(list);
	        AST_label = gen_get_text(list_element);
		LabelGet(AST_label, &label);
		generate(begin_table, iDATA, label, 1, DATA_LABEL, 
			  NOCOMMENT);
		list_element = list_next(list_element);

		while (list_element != ast_null_node)
		{
	    	  AST_label = gen_get_text(list_element);
		  LabelGet(AST_label, &label);
		  generate(0, iDATA, label, 1, DATA_LABEL, NOCOMMENT);
		  list_element = list_next(list_element);
		}

	    /*  generate the label at the end of the computed goto  */
		generate (end_goto, NOP, 0, 0, 0, "End Computed GOTO");
	  }
        }
} /* HandleComputedGoto */



/* generate ILOC for a FORTRAN assigned GOTO statement from an AST node */
void HandleAssignedGoto( AST_INDEX node )
  // AST_INDEX 	node;	/*  GEN_ASSIGNED_GOTO AST node  */
{
  AST_INDEX 	label;	/* name associated with the target label */
  AST_INDEX 	list;	/* possible values for the target label  */

  STR_TEXT	AST_label;	/* AST node for potential label    */
  int		iloc_label,	/* ILOC number for potential label */
		state,	/* flag determining list position  */
		target;	/* register holding target label   */

  char			label_list[700];
  char			*label_list_ptr;

  label = gen_ASSIGNED_GOTO_get_name(node);
  list  = gen_ASSIGNED_GOTO_get_lbl_ref_LIST(node);

  /* first, we find the label variable */
  if (gen_get_node_type(label) == GEN_IDENTIFIER)
  {
    target = getExprInReg(label);
    if (fst_my_GetFieldByIndex(ft_SymTable, target, SYMTAB_TYPE) != TYPE_INTEGER)
	ERROR("HandleAssignedGoto", "invalid target variable", FATAL);
  }
  else
     ERROR("HandleAssignedGoto", "invalid target variable", FATAL);

  /* generate list of possible labels for the branch */
  label_list_ptr = label_list;
  if ((list != ast_null_node)&&(gen_get_node_type(list) != GEN_PLACE_HOLDER))
  {
    if (is_list(list))
    {
      list = list_first(list);
      while(list != ast_null_node)
      {
	if (gen_get_node_type(list) == GEN_LABEL_REF)
 	{
	  AST_label = gen_get_text(list);
	  if (LabelAssigned(AST_label) == FALSE)
	  {
	    (void) sprintf(error_buffer, 
	    "'%s' is used in an ASSIGNED GOTO but is not used in an ASSIGN.  '%s' is ignored.",
		AST_label, AST_label);
	    ERROR("HandleAssignedGoto", error_buffer, WARNING);
	  }
	  else
	  {
	    LabelGet( AST_label, &iloc_label );
	    (void) sprintf(label_list_ptr, "LL%-4.4d ", iloc_label);
	    label_list_ptr = label_list_ptr + 7;
	  }
	}
	else 
	   ERROR("HandleAssignedGoto", "invalid target in list", FATAL);

        list = list_next(list);
      }
    }
  }
  else /* no list of labels, so we add every label */
  {    /* used in an ASSIGN statement in the procedure	*/

    /* first, chew them out! */
    ERROR("HandleAssignedGoto", "Assigned goto has no label list", WARNING);
    ERROR("HandleAssignedGoto", "This severely impedes optimization", WARNING);

    state = START_OF_LIST;
    iloc_label = LabelNextAssigned( &state );

    if (aiDebug > 0)
	(void) fprintf(stdout, "HandleAssignedGoto: no label list!\n");

    while(iloc_label != aiEND_OF_LIST)
    {
      (void) sprintf(label_list_ptr, "LL%-4.4d ", iloc_label);
      label_list_ptr = label_list_ptr + 7;
      iloc_label = LabelNextAssigned( &state );
    }
  }
  
  /* at this point, target contains the address to which we should branch */
  generate(0, JMPr, target, (Generic) label_list, 0, "assigned goto");

} /* HandleAssignedGoto */
