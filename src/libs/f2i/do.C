/* $Id: do.C,v 1.1 1997/04/28 20:18:07 carr Exp $ */
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



/* return the register containing the loop increment */
int get_increment(AST_INDEX induction, int variable_type, int *increment_is_one, 
		  int *integer_constant)

//   AST_INDEX induction;       /* AST node for control of the inductive loop */
//   int	variable_type,       /* type of the inductive loop variable        */
//         *increment_is_one,   /* return flag indicating if increment is one */
//         *integer_constant;   /* return flag indicating if increment is:    */
//                              /*   a positive constant --- returns  1       */
//                              /*   a negative constant --- returns -1       */
//                              /*   not a constant      --- returns  0       */

{
  AST_INDEX son;	/* AST node containing the increment */
  
  int	increment,     /* register containing the increment          */
        temp,          /* temporary for holding register number      */
        a_constant;   /* flag indicating that increment is constant */

  /* initialize  */
  *integer_constant = 0;

  /* compute increment */
  son = gen_INDUCTIVE_get_rvalue3(induction);

  if (son == ast_null_node || 
      gen_get_node_type(son) == GEN_PLACE_HOLDER)
  { /* no specified increment => one of appropriate type */
    *increment_is_one = TRUE;
    a_constant = TRUE;
    if (variable_type == TYPE_INTEGER)
      {
	increment = getIntConstantInRegister("1");
	*integer_constant = 1;
      }
    else
      increment = getConstantInRegFromString("1", TYPE_INTEGER, variable_type);
  }
  else /* a specified increment  */
  {
     *increment_is_one	= FALSE;

     /* get a register for the increment */
     increment = getExprInReg(son);

     /* is the increment 1? */
     if (fst_my_GetFieldByIndex(ft_SymTable, increment, SYMTAB_STORAGE_CLASS) 
		& SC_CONSTANT)
       {
	 a_constant = TRUE;
	 if (strcmp("1", gen_get_text(son)) == 0)
	   *increment_is_one = TRUE;
       }

     /* if the increment and loop variable are not the same type, convert */
     if (fst_my_GetFieldByIndex(ft_SymTable, increment, SYMTAB_TYPE) !=
     	variable_type)
     	increment = getConversion(increment, variable_type);
  }

  /* get a temporary register for the increment */
  if (*increment_is_one && !a_constant)
    {
      temp = increment;
      increment = StrTempReg("m3", (int) induction, variable_type);
      generate_move(increment, temp, variable_type);
    }

  /* is the increment an integer constant?  */
  if (variable_type == TYPE_INTEGER)
    if (gen_get_node_type(son) == GEN_CONSTANT)
      *integer_constant = 1;
    else if ((gen_get_node_type(son) == GEN_UNARY_MINUS) &&
	(gen_get_node_type(gen_UNARY_MINUS_get_rvalue(son)) == GEN_CONSTANT))
      *integer_constant = -1;

  /* return the increment */
  return increment;
} /* get_increment */



/*  generates code for an inductive FORTRAN do loop                 */
/*                                                                  */
/*  Note that shadow variables are generated if the index variable  */
/*  is modified in the loop (because this is not standard f77, an   */
/*  error message is also generated) or if the index variable is    */
/*  not integer (i.e. real or double precision).                    */
void HandleInductiveDo( AST_INDEX node )		
  // AST_INDEX node;        /* AST node for the inductive FORTRAN do loop */
{
  AST_INDEX son;         /* temporary for holding AST nodes              */
  AST_INDEX induction;   /* AST node for the control portion of the loop */
  AST_INDEX index_node;  /* AST node for the loop variable               */
  
  int	loop_variable,   /* register containing the loop variable             */
        shadow,          /* register containing the shadow variable of loop   */
        increment,       /* register containing the increment for the loop    */
        initial_value,   /* register containing the initial value of the loop */
        high_bound,      /* register containing the upper bound of the loop   */
        zero_reg,        /* register containing the integer 0                 */
        one_reg;         /* register containing the integer 1                 */
        
  int	increment_is_one, /* returned flag indicating that the increment is 1 */
        integer_constant; /* returned flag idicating that the  increment is   */
                          /* an integer constant                              */
                          
  int	variable_type;    /* type of the loop variable */
 
  int	need_shadow;      /* flag indicating that shadow variable is needed */
 
  int	condition,        /* temporary for holding register number */
        convert,          /* temporary for holding register number */
        temp,             /* temporary for holding register number */
        TReg,             /* temporary for holding register number */
        Temp;             /* temporary for holding register number */

  int	loop_start,	  /* label at top of loop    */
        loop_end;	  /* label at bottom of loop */

  /* see what the prepass found out about shadow variables */
  if ((int)ast_get_scratch(node) == INDEX_NOT_MODIFIED) /* cast needed `cuz */
     need_shadow = FALSE;				/* it's unsigned    */
  else 
     need_shadow = TRUE;

  if (aiDebug > 0)
     (void) fprintf(stdout, "DoLoop: need_shadow = %d.\n", need_shadow);

  /* maneuver through the tree to find loop control information */
  induction  = gen_DO_get_control(node);
  index_node = gen_INDUCTIVE_get_name(induction);

  /* get a register for the loop variable */
  loop_variable = getExprInReg( index_node );

  if (need_shadow == TRUE)
  {
    (void) sprintf(error_buffer, 
	   "DO-loop index variable '%s' may be modified in the loop", 
	   (char *) fst_my_GetFieldByIndex
		(ft_SymTable, loop_variable, SYMTAB_NAME));
    ERROR("HandleInductiveDo", error_buffer, WARNING);
    ERROR("HandleInductiveDo", "This impedes optimization", WARNING);
  }

  /* type check the variable */
  variable_type = fst_my_GetFieldByIndex
		(ft_SymTable, loop_variable, SYMTAB_TYPE);
  if ((variable_type == TYPE_REAL)||(variable_type == TYPE_DOUBLE_PRECISION))
     need_shadow = TRUE;
  /* ParaScope type checker does not catch this case */
  else if (variable_type != TYPE_INTEGER)
     ERROR("HandleInductive", 
	"loop variable must be integer, real, or double precision", FATAL);

  /* get a register for "0" */
  zero_reg = (int) getIntConstantInRegister("0");

  /* compute initial value of loop variable */
  son = gen_INDUCTIVE_get_rvalue1(induction);
  initial_value = getExprInReg(son);

  /* if the initial value and loop variable are not the same type, convert */
  if (fst_my_GetFieldByIndex(ft_SymTable, initial_value, SYMTAB_TYPE) !=
	variable_type)
    initial_value = getConversion(initial_value, variable_type);
	
  /* compute upper bound of the loop variable */
  son = gen_INDUCTIVE_get_rvalue2(induction);

  /* get a register for the upper bound */
  high_bound = getExprInReg(son);

  /* if the upper bound and loop variable are not the same type, convert */
  if (fst_my_GetFieldByIndex(ft_SymTable, high_bound, SYMTAB_TYPE) !=
	variable_type)
    high_bound = getConversion(high_bound, variable_type);
    
  /* intialize top and bottom labels of the loop and a branch target */
  loop_start = LABEL_NEW;		/* top of loop */
  loop_end   = LABEL_NEW;		/* statement after loop */

  /* generate code for the case where a shadow variable is needed */
  if (need_shadow == TRUE)
  {
    /* compute the number of iterations needed */
    shadow = StrTempReg("s", (int) induction, TYPE_INTEGER);

    Temp = TempReg(high_bound, initial_value, iSUB, variable_type);

    /* compute the increment */
    increment = get_increment(induction, variable_type, 
		&increment_is_one, &integer_constant);

    /* compute the iteration count, ala the standard */
    generate(0, typed_arithmetic(variable_type, iSUB), high_bound, 
	     initial_value, Temp, NOCOMMENT);
    TReg = TempReg(increment, Temp, iADD, variable_type);
    generate(0, typed_arithmetic(variable_type, iADD), increment, 
	     Temp, TReg, NOCOMMENT);

    if (!increment_is_one)
    {
      convert = TReg;
      TReg    = TempReg(convert, increment, iDIV, variable_type);
      generate(0, typed_arithmetic(variable_type, iDIV), convert, 
	        increment, TReg, NOCOMMENT);
    }
  
    /* convert the shadow variable if necessary */
    if (variable_type != TYPE_INTEGER)
       Temp = getConversion(TReg, TYPE_INTEGER);
    else
       Temp = TReg;

    generate_move(shadow, Temp, TYPE_INTEGER);

    /* branch around the loop, if the shadow variable is less than 1 */
    generate_branch(0, GE,  zero_reg, shadow ,TYPE_INTEGER,
		loop_end, NO_TARGET, "exit loop when shadow variable < 1");

  } /* end of shadow variable section */

  /* generate code for case when we do not need a shadow variable */
  else
  {
    /* get the increment */
    increment = get_increment(induction, variable_type, 
		&increment_is_one, &integer_constant);

    /* branch around the loop, if no iterations are justified */

    if (!increment_is_one)
      {
	/* get a temporary register for the loop variable */
	condition = TempReg(high_bound, initial_value, iSUB, TYPE_INTEGER);
	generate(0, iSUB, high_bound, initial_value, condition, NOCOMMENT);

        temp = TempReg(condition, increment, iADD, TYPE_INTEGER);
        generate(0, iADD, condition, increment, temp, NOCOMMENT);
        condition = TempReg(temp, increment, iDIV, TYPE_INTEGER);
        generate(0, iDIV, temp, increment, condition, NOCOMMENT);
        generate_branch(0, GE, zero_reg, condition, TYPE_INTEGER,
        	loop_end, NO_TARGET, "when condition <= 0, branch to loop exit");
      }
    else
      {
        generate_branch(0, GT, initial_value, high_bound, TYPE_INTEGER,
        	loop_end, NO_TARGET, "when condition < 0, branch to loop exit");
      }

    /* get a temporary register for the upper bound */
    temp = high_bound;
    high_bound = StrTempReg("m2", (int) induction, TYPE_INTEGER);
    generate_move(high_bound, temp, TYPE_INTEGER);
  } /* end case for no shadow variable */



  /* initialize the loop variable */
  generate_move(loop_variable, initial_value, variable_type);

  if (!(fst_my_GetFieldByIndex(ft_SymTable, loop_variable,
	SYMTAB_STORAGE_CLASS) & SC_NO_MEMORY))
  {
    temp = AddressFromNode(index_node);
    generate_store(temp, loop_variable, fst_my_GetFieldByIndex(ft_SymTable,
		loop_variable, SYMTAB_TYPE), loop_variable,"&unknown");
  }

  generate(loop_start, NOP, 0, 0, 0, "Start of loop body");

  /* walk the loop body */
  aiStmtList( gen_DO_get_stmt_LIST(node) );

  /* increment loop variable - make optimizations obvious */
  temp = StrTempReg("i", (int) induction, variable_type);
  if (increment == IntConstant("1"))
    generate(0, iADD, getIntConstantInRegister("1"), loop_variable, temp,
		"increment loop variable");
  else
  {
    if (fst_my_GetFieldByIndex(ft_SymTable, increment, SYMTAB_OBJECT_CLASS) & 
		OC_IS_DATA &&
	fst_my_GetFieldByIndex(ft_SymTable, increment, SYMTAB_STORAGE_CLASS) 
		& SC_CONSTANT)
      (void) getConstantInRegFromIndex(increment, variable_type);

    generate(0, typed_arithmetic(variable_type, iADD), increment,
	      loop_variable, temp, "increment loop variable");
  }
  generate_move(loop_variable, temp, variable_type);

  if (!(fst_my_GetFieldByIndex(ft_SymTable, loop_variable, SYMTAB_STORAGE_CLASS) 
	& SC_NO_MEMORY))
  {
    temp = AddressFromNode(index_node);
	generate_store(temp, loop_variable, fst_my_GetFieldByIndex(ft_SymTable,
		loop_variable, SYMTAB_TYPE), loop_variable,"&unknown");
  }

  if (need_shadow == TRUE)
  {
  	one_reg = getIntConstantInRegister("1");
    TReg = TempReg(shadow, one_reg, iSUB, TYPE_INTEGER);
    generate(0, iSUB, shadow, one_reg, TReg, "decrement iteration count");
    generate_move(shadow, TReg, TYPE_INTEGER);
    generate_branch(0, LT, zero_reg, shadow, TYPE_INTEGER,
    	loop_start, loop_end, "if shadow variable > 0, branch to start");
  }
  else
  {
    /*  check to see if the increment is a positive integer constant  */
    if (integer_constant == 1)
      generate_branch(0, LE, loop_variable, high_bound, TYPE_INTEGER,
		loop_start, loop_end, "if condition >= 0, branch to start");

    /*  check to see if the increment is a negative integer  */
    else if (integer_constant == -1)
      generate_branch(0, GE, loop_variable, high_bound, TYPE_INTEGER,
		loop_start, loop_end, "if condition <= 0, branch to start");

    /*  the increment is not an integer constant  */
    else 
    {
      /* get a temporary register for the loop variable */
      condition = TempReg(high_bound, loop_variable, iSUB, TYPE_INTEGER);
      generate(0, iSUB, high_bound, loop_variable, condition, NOCOMMENT);

      temp = TempReg(condition, increment, lXOR, TYPE_LOGICAL);
      generate(0, lXOR, condition, increment, temp, NOCOMMENT);

      generate_branch(0, GE, temp, zero_reg, TYPE_INTEGER,
		loop_start, NO_TARGET, "common case");
      generate_branch(0, EQ, condition, zero_reg, TYPE_INTEGER,
		loop_start, loop_end, "uncommon case");
    }
  }

  generate(loop_end, NOP, 0, 0, 0, "loop exit label");
} /* HandleInductiveDo */



/* generate code for repetitive FORTRAN do loop */
void HandleRepetitiveDo( AST_INDEX node )

  // AST_INDEX node;	/* AST node for repetitive FORTRAN do loop */
{
  int	counter;		/* register */
  int	initial_value;		/* register */
  int	zero_reg;		/* register */
  int	loop_start;		/* label    */
  int	loop_end;		/* label    */

  /* get new labels */
  loop_start	= LABEL_NEW;
  loop_end   	= LABEL_NEW;

  /* start loop initialization */
  generate(0, NOP, 0, 0, 0, "loop initialization");

  /* put the integer zero in a register */
  zero_reg = (int) getIntConstantInRegister("0");

  /* compute the number of repetitions; force initial value to be integer */
  initial_value = getExprInReg(gen_REPETITIVE_get_rvalue
		( gen_DO_get_control(node)));

  /* force the initial value to be an integer */
  if (fst_my_GetFieldByIndex(ft_SymTable, initial_value, SYMTAB_TYPE) !=
  	TYPE_INTEGER)
  initial_value = getConversion(initial_value, TYPE_INTEGER);
  
  /* generate the conditional branch */
  generate_branch(0, LE, initial_value, zero_reg, TYPE_INTEGER,
  	loop_end, NO_TARGET, "if initial value <= 0, branch to exit");

  /* create temporary register for the counter and initialize it */
  counter = StrTempReg("i", (int) node, TYPE_INTEGER);
  generate_move(counter, initial_value, TYPE_INTEGER);

  /* mark the top of the loop */
  generate(loop_start, NOP, 0, 0, 0, "start of loop");

  /* generate the loop body */
  aiStmtList(gen_DO_get_stmt_LIST(node));

  generate(0, iSUB, counter, getIntConstantInRegister("1"), counter,
	"decrement counter");
  generate_branch(0, GT, counter, zero_reg, TYPE_INTEGER,
  	 loop_start, loop_end, "if counter > 0, branch to start");
  generate(loop_end, NOP, 0, 0, 0, "loop exit label");
} /* HandleRepetitiveLoop */




/* generate code for conditional FORTRAN do loop */
void HandleConditionalDo( AST_INDEX node )
  // AST_INDEX node;	/* AST node for conditional FORTRAN do loop */
{
  int	boolean;	/*  register  */
  int	loop_start;	/*  label     */
  int	next_stmt;	/*  label     */
  int	loop_end;	/*  label     */

  /* get new labels */
  loop_start	= LABEL_NEW;
  next_stmt	= LABEL_NEW;
  loop_end	= LABEL_NEW;

  /* evaluate the condition */
  generate(0, NOP, 0, 0, 0, "initial evaluation of condition");
  boolean = getExprInReg(gen_CONDITIONAL_get_rvalue
		(gen_DO_get_control(node)));

  /* generate the conditional branch */
  generate(loop_start, NOP, 0, 0, 0, "top of loop body");
  generate(0, BR, next_stmt, loop_end, boolean, "branch to loop exit");
  generate(next_stmt, NOP, 0, 0, 0, "target of true branch");

  /* generate the loop body */
  aiStmtList( gen_DO_get_stmt_LIST(node) );

  /* generate the branch to the top of the loop */
  generate(0, JMPl, loop_start, 0, 0, "back to the top");
  generate(loop_end, NOP, 0, 0, 0, "loop exit label");
} /* HandleConditionalDo */


/* determines the opcode value for an operation a particular type */
/*                                                                */
/* Note that the value of the opcode can be determined as the     */
/* value of the opcode for the integer operation plus the         */
/* the distance between an add of the correct type and an         */
/* integer add.  If opcodes are ever rearranged so that this      */
/* is no longer true, this routine will return incorrect results. */
int typed_arithmetic(int type, int op)
//   int type;     /* type of the operation                 */
//   int op;       /* opcode value of the integer operation */
{
  switch(type)
  {
    case TYPE_INTEGER:
	break;

    case TYPE_REAL:
	op = op + fADD - iADD;
	break;

    case TYPE_DOUBLE_PRECISION:
	op = op + dADD - iADD;
	break;

    case TYPE_COMPLEX:
	op = op + cADD - iADD;
	break;

    case TYPE_DOUBLE_COMPLEX:
	op = op + qADD - iADD;
	break;

    default:
	op = ERR;
	break;
  }
  return op;
} /* typed_arithmetic */

