/* $Id: stmts.C,v 1.1 1997/04/28 20:18:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/*
 *  Statement lists
 *
 */

/*
 * The f77 standard lists the following statements as executable:
 *
 *  Assignment statement 	HandleAssignment()	assignment.c
 *  ASSIGN statement		HandleAssign()		assign.c
 *  
 *  Unconditional GO TO		HandleGoto()		goto.c
 *  Computed GO TO		HandleComputedGoto()	goto.c
 *  Assigned GO TO		HandleAssignedGoto()	goto.c
 *
 *  Arithmetic IF		HandleArithmeticIf()	if.c
 *  
 *  Logical IF			HandleLogicalIf()	if.c
 *  
 *  Block IF			HandleIf()		if.c
 *  ELSE IF			  ditto
 *  ELSE			  ditto
 *  END IF			  ditto
 *  
 *  DO				HandleInductiveDo()	do.c
 *				HandleConditionalDo()	do.c
 *				HandleRepetitiveDo()	do.c
 *
 *  CONTINUE			aiStmtList()		stmts.c
 *  
 *  STOP (like RETURN)		aiStmtList() 		stmts.c
 *  PAUSE			
 *
 *  CALL			HandleCall()		call.c
 *  RETURN			aiStmtList()		stmts.c
 *
 *  READ			
 *  WRITE			
 *  PRINT			
 *  OPEN			
 *  CLOSE			
 *  INQUIRE			
 *  BACKSPACE			
 *  ENDFILE			
 *  REWIND			
 *  
 *  FORMAT 			
 * 
 * 
 */

#include <libs/support/misc/general.h>
#include <sys/file.h>

#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/support/lists/list.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/frontEnd/include/gi.h>
#include <libs/frontEnd/ast/asttree.h>
#include <libs/frontEnd/ast/aphelper.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <stdio.h>

#include <libs/f2i/ai.h>
#include <libs/f2i/sym.h>
#include <libs/f2i/f2i_label.h>
#include <libs/f2i/char.h>

#include <libs/f2i/mnemonics.h>

#include <libs/Memoria/annotate/DirectivesInclude.h>

/* global names - shared with procedures.c */

int 	 	proc_type;
AST_INDEX	formal_list;

/* ... shared with get.c */
int	NotMapped = 1;

/* shared with io.c, label.c, stmt.c, and ai.c */
extern FortTree     ft;
extern FortTextTree ftt;

/* forward declarations */

static void MarkNameList(AST_INDEX,int,int);
static void findType(AST_INDEX);
static void TypeLenFault(int,int);

/* and some statics (required for passing around TYPE and LENGTH information */
static int FoundType;
static int FoundLength;




/* Walk the list of statements invoking the appropriate   */
/* routine listed above to process each type of statement */
void aiStmtList(AST_INDEX StmtList)
  // AST_INDEX	StmtList;
{
  AST_INDEX	Stmt;
  AST_INDEX	StmtLabel;
  AST_INDEX	son;
  int		StmtType;
  int		IlocLabel;
  int		l1, c1, l2, c2;
  char		*line;
  char          parms[32];
  Boolean       IsDirective = false;

  if (aiDebug > 0)
     (void) fprintf(stdout, "aiStmtList(%d).\n", StmtList);

  if (StmtList != ast_null_node)
     Stmt = list_first(StmtList);

  while (Stmt != ast_null_node)
  {
   /*  Note that guard statements do not pass through this code.  */
   /*  Add any necessary statement code to ExtendIf (if.c)        */

   aiStmtCount++;
   StmtType = gen_get_node_type(Stmt);

   /* Label handling - 								*/
   /*										*/
   /*  This assumes that all LABEL definitions are stored in the 1st son	*/
   /*  field of the node for the statement.  Unfortunately, this assumption	*/
   /*  might someday get invalidated.						*/
   /*										*/
   /*  This is also the right place to spit out the comment field containing	*/
   /*  AST indexes for ExMon.							*/
   
   StmtLabel = gen_ASSIGNMENT_get_lbl_def(Stmt); 

   (void) sprintf(error_buffer, "%s - index %5d", 
	    ast_get_node_type_name(StmtType), Stmt);

   if (aiDebug > 1)
      (void) fprintf(stdout, "\tStmt %d (%d) has type %s (%d).\n", aiStmtCount,
	       Stmt, ast_get_node_type_name(StmtType), StmtType);

   if (StmtType == GEN_COMMENT && aiParseComments)
     IsDirective = aiDirectiveIsInComment(Stmt);

   if (StmtType != GEN_COMMENT || IsDirective)
    if (ai_isExecutable(StmtType) || IsDirective)
    {
      if (NotMapped)
      {
	NotMapped = FALSE;
	MapStorage(StmtList);

	/* PrePass() computed these */
	aiStackSize = aiExpressionStackSpace + aiNextStack; 

        /* compute parameter list */
	(void) sprintf(parms, "r%d r%d", 
		       fst_my_GetFieldByIndex(ft_SymTable, aiStackBase(), SYMTAB_REG), 
		       fst_my_GetFieldByIndex(ft_SymTable, aiStaticLabel(), SYMTAB_REG));

	if (proc_type != GEN_PROGRAM)
	   aiProcedurePrologue(formal_list);
	else
	{
	  if (aiSparc) 
	    generate_string("_MAIN_", FRAME, aiStackSize, (int) parms, 0,
		     "Start of the program");
	  else if (aiRt)
	    generate_string("_.MAIN_", FRAME, aiStackSize, (int) parms, 0, 
		     "Start of the program");
	  else
	    generate_string("_main", FRAME, aiStackSize, (int) parms, 0,
		     "Start of the program");

	   aiLoadUpStuff();
	}
      }


      ftt_NodeToText(ftt, Stmt, &l1, &c1, &l2, &c2);
      line = (char *) ftt_GetTextLine(ftt,l1);

      if (StmtLabel != ast_null_node)
      {
        (void) LabelGet(gen_get_text(StmtLabel), &IlocLabel);
	generate(IlocLabel, NOP, 0, 0, 0, line);
      }
      else
	/* generate declaratory statements only if labelled */
	generate(0, COMMENT, 0, 0, 0, line);

      switch(StmtType)
      {
	case GEN_ARITHMETIC_IF:
		HandleArithmeticIf(Stmt);
		break;

	case GEN_ASSIGN:
		HandleAssign(Stmt);
		break;

	case GEN_ASSIGNED_GOTO:
		HandleAssignedGoto(Stmt);
		break;

	case GEN_ASSIGNMENT:
		HandleAssignment(Stmt);
		break;

	case GEN_CALL:
		HandleCall(gen_CALL_get_invocation(Stmt));
		break;

	case GEN_COMMENT:
		HandleDirective(Stmt);
		break;

	case GEN_COMPUTED_GOTO:
		HandleComputedGoto(Stmt);
		break;

	case GEN_DO:
		son = gen_DO_get_control(Stmt);
		switch(gen_get_node_type(son))
		{
		  case GEN_INDUCTIVE:
			HandleInductiveDo(Stmt);
			break;
		  case GEN_CONDITIONAL:
			HandleConditionalDo(Stmt);
			break;
		  case GEN_REPETITIVE:
			HandleRepetitiveDo(Stmt);
			break;
		  default:
			ERROR("aiStmtList", "unknown loop type", FATAL);
			break;
		}
		break;

	case GEN_GOTO:
		HandleGoto(Stmt);
		break;

	case GEN_IF:
		HandleIf(Stmt);
		break;

	case GEN_LOGICAL_IF:
		HandleLogicalIf(Stmt);
		break;

	case GEN_PAUSE:
		HandlePause(Stmt);
		break;

	case GEN_RETURN:
		generate(0, JMPl, aiEpilogue, 0, 0, "branch to epilogue");
		break;

	case GEN_STOP:
		HandleStop(Stmt);
		break;

	case GEN_CONTINUE:
	case GEN_PLACE_HOLDER:
		break;

	case GEN_FORMAT:
		break;

	case GEN_READ_SHORT:
	case GEN_READ_LONG:
	case GEN_WRITE:
	case GEN_PRINT:
	case GEN_OPEN:
	case GEN_CLOSE:
	case GEN_INQUIRE:
	case GEN_BACKSPACE_SHORT:
	case GEN_BACKSPACE_LONG:
	case GEN_ENDFILE_SHORT:
	case GEN_ENDFILE_LONG:
	case GEN_REWIND_SHORT:
	case GEN_REWIND_LONG:
		DoIO(StmtType, Stmt);
		break;

	default:
		UNIMPL(StmtType);
		break;
      }
    }
    else 
    {
      if (NotMapped == FALSE)
	 ERROR("Stmts", "Declaration found after first executable statement",
	       FATAL);

      if (StmtLabel != ast_null_node)
      {
        (void) LabelGet(gen_get_text(StmtLabel), &IlocLabel);
	generate(IlocLabel, NOP, 0, 0, 0, error_buffer);
      }

      switch(StmtType)
      {
	case GEN_COMMON:
		break;

	case GEN_DATA:
		break;

	case GEN_DIMENSION:
		ArrayDeclLenList(gen_DIMENSION_get_array_decl_len_LIST(Stmt), DK, DK);
		break;

	case GEN_EQUIVALENCE:
		SymHaveSeenAnEquivalence++;
		break;

	case GEN_EXTERNAL:
		MarkNameList(gen_EXTERNAL_get_name_LIST(Stmt), 
			     OC_IS_EXECUTABLE, SC_EXTERNAL);
		break;

	case GEN_IMPLICIT:
		/* ignore it, since the tree should include types
		 * (both REAL and CONVERTED) for each IDENTIFIER
		 */
		break;

	case GEN_INTRINSIC:
		MarkNameList(gen_INTRINSIC_get_name_LIST(Stmt), 
			     OC_IS_EXECUTABLE, SC_INTRINSIC);
		break;

	case GEN_PARAMETER:
		break;

	case GEN_SAVE:
		HandleSave(Stmt);		
		break;

	case GEN_STMT_FUNCTION:
		HandleStatementFunction(Stmt);
		break;

	case GEN_TYPE_STATEMENT:
		findType(gen_TYPE_STATEMENT_get_type_len(Stmt));
		ArrayDeclLenList(gen_TYPE_STATEMENT_get_array_decl_len_LIST(Stmt),
			FoundType, FoundLength);
		break;

	default:
		UNIMPL(StmtType);
		break;
      }
    }
    Stmt = list_next(Stmt);
  }

  /* if the list contains no executable statements, we still
   * need to do the storage mapping stuff to ensure that any
   * data initializations happen correctly (like in a BLOCK
   * DATA subprogram)
   *
   */
  if (NotMapped)
  {
    MapStorage(StmtList);	/* calls aiPrePass()	*/
    NotMapped = FALSE;
    aiStackSize = aiExpressionStackSpace + aiNextStack;
   }
} /* aiStmtList */




/* process the array length information from the array declaration */
int ArrayDeclLen(AST_INDEX node, int Class, int type, int length)
//   AST_INDEX node;
//   int Class, type, length;
{
  AST_INDEX Var;
  AST_INDEX Dimensions;
  AST_INDEX lbound;
  AST_INDEX hbound;

  register int i, index, ndims;
  int value, param_reg;

  ArrayBound *bounds;

  if (aiDebug > 1)
     (void) fprintf(stdout, "ArrayDeclLen(%d, %d, %d, %d).\n", node, Class,
	     type, length);

  Var		= gen_ARRAY_DECL_LEN_get_name(node);
  Dimensions	= gen_ARRAY_DECL_LEN_get_dim_LIST(node);

  if (type == DONT_KNOW)
     type = gen_get_real_type(Var);

  index = fst_QueryIndex(ft_SymTable, gen_get_text(Var));

  if (Class != DONT_KNOW)
    fst_PutFieldByIndex(ft_SymTable, index, SYMTAB_STORAGE_CLASS, Class);
/*  else
    fst_PutFieldByIndex(ft_SymTable, index, SYMTAB_STORAGE_CLASS, SC_NO_MEMORY);
*/

  if (type != fst_GetFieldByIndex(ft_SymTable, index, SYMTAB_TYPE))
    {
     fst_PutFieldByIndex (ft_SymTable, index, SYMTAB_TYPE, type);
     if (aiDebug > 1)
       (void) fprintf (stdout, "inserting type %d into symtable\n",type);
   }

  if (aiDebug > 1)
     (void) fprintf(stdout, "\tFor variable '%s'(%d).\n", 
	     (char *) fst_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME), index);

  if (length != DONT_KNOW) {
	fst_PutFieldByIndex(ft_SymTable, index, SYMTAB_CHAR_LENGTH,  length);
	fst_PutFieldByIndex(ft_SymTable, index, SYMTAB_SIZE,  length);
      }

  if (Dimensions != ast_null_node)
  {

    Dimensions = list_first( Dimensions );
    (void) SymInsertSymbol("DIM(*)", TYPE_INTEGER, OC_IS_DATA, 0,
			    SC_CONSTANT, 0);
    ndims = fst_GetFieldByIndex(ft_SymTable, index, SYMTAB_NUM_DIMS);
    bounds = (ArrayBound *) fst_GetFieldByIndex(ft_SymTable, 
						   index, SYMTAB_DIM_BOUNDS);

    for (i=0; i < ndims; i++)
    {
      lbound = gen_DIM_get_lower(Dimensions);
      hbound = gen_DIM_get_upper(Dimensions);

      if (fst_GetFieldByIndex(ft_SymTable, index, SYMTAB_OBJECT_CLASS) & 
	                                                        OC_IS_FORMAL_PAR)
      {
	if (bounds[i].lb.type == dim_error)
	  {
	    (void) sprintf(error_buffer, 
			   "Invalid dimension expression for '%s'", (char *) 
			   fst_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME));
	    ERROR("ArrayDeclLen", error_buffer, SERIOUS);
	    ERROR("ArrayDeclLen", "Code for array references will be incorrect",
		  FATAL);
	  }
	else if (bounds[i].lb.type == symbolic_expn_ast_index)
	  {
	  if (ai_isConstantExpr(bounds[i].lb.value.ast))
	    {
	      value = evalExpr(bounds[i].lb.value.ast);
	      bounds[i].lb.type = constant;
	      bounds[i].lb.value.const_val = value;
	    }
	  else if (gen_get_node_type(bounds[i].lb.value.ast) != GEN_IDENTIFIER)
	    {  /* then it has a parameter in the expr */
	      param_reg = StrTempReg((char *) fst_GetFieldByIndex(ft_SymTable, 
			     index, SYMTAB_NAME), 2*ndims, TYPE_INTEGER);

	      fst_PutFieldByIndex(ft_SymTable, param_reg, SYMTAB_OBJECT_CLASS,
				     OC_IS_DATA);
	      
	      fst_PutFieldByIndex(ft_SymTable, param_reg, SYMTAB_STORAGE_CLASS,
				     SC_NO_MEMORY);
	      RecordInitialExp(param_reg, bounds[i].lb.value.ast);
	    }
	  else if (!isParameterExpr(lbound))
	    {
	      (void) sprintf(error_buffer, 
			     "Invalid dimension expression for '%s'", (char *) 
			     fst_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME));
	      ERROR("ArrayDeclLen", error_buffer, SERIOUS);
	      ERROR("ArrayDeclLen", "Code for array references will be incorrect",
		    FATAL);
	    }
	}
	if (bounds[i].ub.type == dim_error)
	  {
	    (void) sprintf(error_buffer, 
			   "Invalid dimension expression for '%s'", (char *) 
			   fst_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME));
	    ERROR("ArrayDeclLen", error_buffer, SERIOUS);
	    ERROR("ArrayDeclLen", "Code for array references will be incorrect",
		  FATAL);
	  }
	else if (bounds[i].ub.type == symbolic_expn_ast_index)
	  {
	  if (ai_isConstantExpr(bounds[i].ub.value.ast))
	    {
	      value = evalExpr(bounds[i].ub.value.ast);
	      bounds[i].ub.type = constant;
	      bounds[i].ub.value.const_val = value;
	    }
	  else if (gen_get_node_type(bounds[i].ub.value.ast) != GEN_IDENTIFIER)
	    {  /* then it has a parameter in the expr */
	      param_reg = StrTempReg((char *) fst_GetFieldByIndex(ft_SymTable, 
			     index, SYMTAB_NAME), 2*ndims, TYPE_INTEGER);

	      fst_PutFieldByIndex(ft_SymTable, param_reg, SYMTAB_OBJECT_CLASS,
				     OC_IS_DATA);
	      
	      fst_PutFieldByIndex(ft_SymTable, param_reg, SYMTAB_STORAGE_CLASS, 
				     SC_NO_MEMORY);
	      RecordInitialExp(param_reg, bounds[i].ub.value.ast);
	    }
	  else if (!isParameterExpr(hbound))
	    {
	    (void) sprintf(error_buffer, 
			   "Invalid dimension expression for '%s'", (char *) 
			   fst_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME));
	    ERROR("ArrayDeclLen", error_buffer, SERIOUS);
	    ERROR("ArrayDeclLen", "Code for array references will be incorrect",
		  FATAL);
	    }
	}
      }

      if (aiDebug > 1)
	 (void) fprintf(stdout, "\tdim %d runs from '%s' to '%s'.\n", i,
		(char *) fst_GetFieldByIndex(ft_SymTable, 
                   getIndexForlb(bounds,i,index), SYMTAB_NAME), 
	        (char *) fst_GetFieldByIndex(ft_SymTable,
                   getIndexForub(bounds,i,index), SYMTAB_NAME));

      Dimensions = list_next(Dimensions);       /* and bump the pointer */

    }
  }
  return index;
} /* ArrayDeclLen */




/* walk list processing array lengths using ArrayDeclLen */
void ArrayDeclLenList(AST_INDEX list, int type, int length)
//   AST_INDEX	list;
//   int		type;
//   int		length;
{
  char *s;
  int len;

  list = list_first(list);

  /* character length descriptor following variable	*/
  /* declaration takes precedence over generic length	*/
  /* so look for length(s) following declaration	*/
  if (type == TYPE_CHARACTER)
    while (list != ast_null_node) {
      if (gen_ARRAY_DECL_LEN_get_len(list) != ast_null_node) {
	if (aiDebug > 2)
	  (void) fprintf(stdout,"Local character length replacing generic length.\n");
	if (is_star(gen_ARRAY_DECL_LEN_get_len(list)))
	  len = STAR_LEN;
	else {
	  s = gen_get_text(gen_ARRAY_DECL_LEN_get_len(list));
	  (void) get_num( s, &len );
	}
	(void) ArrayDeclLen( list, DONT_KNOW, type, len );
	list = list_next(list);
      }
      else {
	(void) ArrayDeclLen( list, DONT_KNOW, type, length );
	list = list_next(list);
      }
    }
  else
    /* end change */
    while (list != ast_null_node)
      {
	(void) ArrayDeclLen( list, DONT_KNOW, type, length );
	list = list_next(list);
      }
} /* ArrayDeclLenList */




/* insert elements from the name list of either an intrinsic or */
/* external routine or a SAVE statement into the symbol table   */
static void MarkNameList( AST_INDEX node, int OClass, int SClass )
//   AST_INDEX	node;
//   int		OClass, SClass;
{
  register int type;
  if (node != ast_null_node)
     node = list_first(node);

  while (node != ast_null_node)
  {
    type = gen_get_real_type(node);
    if (type == TYPE_UNKNOWN)
	type = DONT_KNOW;
    (void) SymInsertSymbol(gen_get_text(node),type,OClass,DK,SClass,DK);
    node = list_next(node);
  }
} /* MarkNameList */




/* determine whether or not a statement is executable */
int ai_isExecutable(int s) 
  // int s;
{
  register int result;
  if (s == GEN_TYPE_STATEMENT	|| s == GEN_COMMON	|| s == GEN_COMMENT ||
      s == GEN_DATA             || s == GEN_DIMENSION	||
      s == GEN_EQUIVALENCE	|| s == GEN_EXTERNAL	|| 
      s == GEN_INTRINSIC	|| s == GEN_PARAMETER   ||
      s == GEN_SAVE		|| s == GEN_STMT_FUNCTION ||
      s == GEN_IMPLICIT)
     result = FALSE;
  else
     result = TRUE;

  return result;
} /* ai_isExecutable */




/* check for type/length mismatch */
static void findType(AST_INDEX TypeLen)
  // AST_INDEX TypeLen;
{
  int type, len /* can't be a register */;
  char *s;

  if (gen_get_node_type(TypeLen) != GEN_TYPE_LEN)
     ERROR("FindType", "Invalid argument - expected GEN_TYPE_LEN", FATAL);

  type = gen_get_node_type(gen_TYPE_LEN_get_type(TypeLen));
    
  if (gen_TYPE_LEN_get_length(TypeLen) != ast_null_node)
  {
    /* add if..then to test for *(*) length */
    if (is_star(gen_TYPE_LEN_get_length(TypeLen)))
      len = STAR_LEN;
    else {
      s = gen_get_text(gen_TYPE_LEN_get_length(TypeLen));
      s = get_num( s, &len );    
    }
  }
  else 
    len = 0;

  switch(type)
  {
    case GEN_CHARACTER:
	/* if no given length, assume len = 1 */
        if (len == 0)
	  len = 1;
	type = TYPE_CHARACTER;
	break;
    case GEN_LOGICAL:
	if (!(len == 0 || len == 1))
	   TypeLenFault(TYPE_LOGICAL, len);
	type = TYPE_LOGICAL;
	break;
    case GEN_INTEGER:
    case GEN_LABEL:
	if (!(len == 0 || len == 4))
	   TypeLenFault(TYPE_INTEGER, len);
	type = TYPE_INTEGER;
	break;
    case GEN_REAL:
	if (!(len == 0 || len == 4 || len == 8))
	   TypeLenFault(TYPE_REAL, len);
	else if (len == 8)
	   type = TYPE_DOUBLE_PRECISION;
        else
	   type = TYPE_REAL;
	break;
    case GEN_COMPLEX:
	if (len != 0)
	   TypeLenFault(TYPE_COMPLEX, len);
	type = TYPE_COMPLEX;
	break;
    case GEN_DOUBLE_PRECISION:
	if (!(len == 0 || len == 8))
	   TypeLenFault(TYPE_DOUBLE_PRECISION, len);
	type = TYPE_DOUBLE_PRECISION;
	break;
    default:
	(void) sprintf(error_buffer, "Unknown TYPE_STATEMENT node type, %d", type);
	ERROR("findType", error_buffer, FATAL);
	type = TYPE_INTEGER;
	break;
  }

  FoundType   = type;
  FoundLength = len;

  if (aiDebug > 1)
     (void) fprintf(stdout, "\tfindType: type statement results in '%s' (%d).\n",
	     TypeName(FoundType), FoundLength);

} /* findType */




/* print error message when invalid type/length combination is detected */
static void TypeLenFault( int type, int length )
  // int type, length;
{
  (void) sprintf(error_buffer, "Invalid TYPE/LENGTH combination (%s*%d)",
          TypeName(type), length);
  ERROR("TypeLenFault", error_buffer, FATAL);
} /* TypeLenFault */




/* generate code for a SAVE statement */
void HandleSave(AST_INDEX Stmt)
  // AST_INDEX Stmt;
{
  AST_INDEX list;

  list = gen_SAVE_get_name_LIST(Stmt);

  if (list == ast_null_node)
  {
    /* This assignment takes place only if we see a SAVE with no arg.	*/
    /* list.  That means (section 8.9, 1977 Standard) that all names	*/
    /* are SAVEd, within the bounds of legality.  No parameters, no	*/
    /* procedure names, no variables in common.				*/
    /* 									*/
    /* This flag, initialized in SymCreateTable(), causes MapStorage()	*/
    /* to mark such variables as SAVEd.  See that code for the list of	*/
    /* things that get marked.						*/
    /* 									*/
    /* See also aiGenerateStaticArea() for initializations of SAVEs. 	*/
    SymHaveSeenASave = 1;
    ERROR("HandleSave", 
	  "Use of a SAVE for all variables will impede optimization",WARNING);
    ERROR("HandleSave",
	  "Specifying a smaller set of names will help", WARNING);
  }
  else
     MarkNameList(list, OC_IS_DATA, SC_STATIC);
} /* HandleSave */




/* and a service routine */
/* if the integer constant is not already in the symbol table, insert it */
int IntConstant(char *name)
  // char *name;
{
  int	 i;
  char	*p;
  char  buffer[32];

  p = ConstantName(name, TYPE_INTEGER, buffer);
  i = fst_QueryIndex(ft_SymTable, p);
  if (i == SYM_INVALID_INDEX)
  {
    i = SymInsertSymbol(p, TYPE_INTEGER, OC_IS_DATA, 0, 
			SC_CONSTANT, NO_ALIAS);
  }
  return i;
} /* IntConstant */




/* determine whether or not an expression is composed of constants */
Boolean ai_isConstantExpr(AST_INDEX node)
  // AST_INDEX node;
{
  Boolean result;

  switch(gen_get_node_type(node))
  {
    case GEN_CONSTANT:
	result = true;
	break;
    case GEN_COMPLEX_CONSTANT:
	if (ai_isConstantExpr(gen_COMPLEX_CONSTANT_get_real_const(node)) &&
	    ai_isConstantExpr(gen_COMPLEX_CONSTANT_get_imag_const(node)) )
	   result = true;
	else
	   result = false;
	break;
    case GEN_BINARY_TIMES:
    case GEN_BINARY_DIVIDE:
    case GEN_BINARY_PLUS:
    case GEN_BINARY_MINUS:
    case GEN_BINARY_EXPONENT:
	if (ai_isConstantExpr(gen_BINARY_TIMES_get_rvalue1(node)) &&
	    ai_isConstantExpr(gen_BINARY_TIMES_get_rvalue2(node)) )
	   result = true;
	else
	   result = false;
	break;
    case GEN_UNARY_MINUS:
	result = ai_isConstantExpr(gen_UNARY_MINUS_get_rvalue(node));
	break;
    default:
	result = false;
	break;
  }
  return result;
} /* ai_isConstantExpr */




/* determine if an expression is composed of parameters and constants */
Boolean isParameterExpr(AST_INDEX node)
  // AST_INDEX node;
{
  Boolean result;

  switch(gen_get_node_type(node))
  {
    case GEN_CONSTANT:
	result = true;
	break;
    case GEN_COMPLEX_CONSTANT:
	if (isParameterExpr(gen_COMPLEX_CONSTANT_get_real_const(node)) &&
	    isParameterExpr(gen_COMPLEX_CONSTANT_get_imag_const(node)) )
	   result = true;
	else
	   result = false;
	break;
    case GEN_BINARY_TIMES:
    case GEN_BINARY_DIVIDE:
    case GEN_BINARY_PLUS:
    case GEN_BINARY_MINUS:
    case GEN_BINARY_EXPONENT:
	if (isParameterExpr(gen_BINARY_TIMES_get_rvalue1(node)) &&
	    isParameterExpr(gen_BINARY_TIMES_get_rvalue2(node)) )
	   result = true;
	else
	   result = false;
	break;
    case GEN_UNARY_MINUS:
	result = isParameterExpr(gen_UNARY_MINUS_get_rvalue(node));
	break;
    case GEN_IDENTIFIER:
	if (fst_GetFieldByIndex(ft_SymTable, getIndex(node), SYMTAB_OBJECT_CLASS) &
	                                                               OC_IS_FORMAL_PAR)
	   result = true;
	else
	   result = false;
	break;
    default:
	result = false;
	break;
  }
  return result;
} /* isParameterExpr */

Boolean aiDirectiveIsInComment(AST_INDEX Stmt)
  
  //   AST_INDEX Stmt;

  {
    return (BOOL(GET_DIRECTIVE_INFO(Stmt) != NULL));
  }
