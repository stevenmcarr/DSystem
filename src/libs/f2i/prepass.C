/* $Id: prepass.C,v 1.5 2000/03/28 20:07:59 carr Exp $ */
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
#include <libs/f2i/char.h>
#include <libs/f2i/sym.h>
#include <include/frontEnd/astnode.h>
#include <libs/f2i/f2i_label.h>
#include <libs/f2i/mnemonics.h>

/* statics */
static char name_buffer[128];
static int  spaceForActuals;
static int  relopLevel;       /* number of nested relational operators */

extern int proc_type;

static void PrePass(AST_INDEX);
static void assigned_label(AST_INDEX);
static void find_modified_vars(AST_INDEX);
static void mark_assignment(AST_INDEX node);
static void RecordLabel(AST_INDEX, AST_INDEX);
static void check_stmt_actuals(AST_INDEX);
static void check_actuals(char *, AST_INDEX );
static void MakeRoomForExp(AST_INDEX);
static void MarkGlobals(SymDescriptor, fst_index_t,Generic);

/* 
 * The game being played with the size of the run-time stack is a
 * little complex.
 *
 *  spaceForActuals -
 *	should be SizeOfType(TYPE_INTEGER) * maximum number of
 *	formal parameters PLUS SizeOfType(TYPE_INTEGER) * number
 *      of character parameters -- this represents the hidden
 *      length parameter
 *
 *  aiExpressionStackSpace -
 *	the total amount of stack space required for expression
 *	evaluation and storage at call sites.  Computed in 
 *	CheckActuals().
 *
 *
 */




/* Initialize values computed in PrePass().  Call        */
/* PrePass() to recursively walk tree and compute values */
void aiPrePass(AST_INDEX node)
  // AST_INDEX node;
{
  int	 SavedStmtNo;

  if (aiDebug > 0)
     (void) fprintf(stdout, "Entering aiPrePass().\n");  

  SavedStmtNo = aiStmtCount;	/* Should be no errors in PrePass 	*/
  aiStmtCount = 0;		/* IF one happens, its stmt no. 0	*/

  relopLevel             = 0;
  spaceForActuals	 = 0;
  aiExpressionStackSpace = 0;

  PrePass( node );

  /* add 4 bytes for the parameter for "exit" if we have a program */
  if (proc_type == GEN_PROGRAM)
    aiExpressionStackSpace += GetDataSize(TYPE_INTEGER);

  aiStmtCount = SavedStmtNo;
  aiNextStack += max(spaceForActuals,getIOLActuals());
  aiMaxVariables = fst_MaxIndex (ft_SymTable);

  if (aiDebug > 0)
  {
    (void) fprintf(stdout, "Leaving aiPrePass().\n");
    (void) fprintf(stdout, "\tExpression Stack Space: %d (estimate)\n", 
		   aiExpressionStackSpace);  
    (void) fprintf(stdout, "\tSpace for actuals: %d\n", spaceForActuals);
    (void) fprintf(stdout, "\tNext available stack space: %d\n", aiNextStack);
  }
} /* aiPrePass */




/* records a variety of types of information used in the storage */
/* mapping routines and in later routines that generate iloc     */
static void PrePass(AST_INDEX node)
    // AST_INDEX node;
{
   register int i, n, type, itype;
   STR_TEXT	text;
   char		buffer[128];
   AST_INDEX	inv;
   int		ct;

  if (node == AST_NIL) return;

   type = gen_get_node_type(node);

   if (aiDebug > 1)
      (void) fprintf(stdout, "\tPrePass(%d) [%s].\n", node, 
	      ast_get_node_type_name(type) );

   if (is_list(node))
   {
     node = list_first(node);
     while(node != ast_null_node)
     {
       PrePass(node);
       node = list_next(node);
     }
   }
   else
   {
     switch(type)
     {
       /* When we find a relational operator, increment relopLevel,
	* parse the subtrees, and decrement relopLevel.  If a
	* concat is found inside a relation expression, stack space
	* for the comparison must be allocated.
	*/
	case GEN_BINARY_LT:
	case GEN_BINARY_LE:
	case GEN_BINARY_EQ:
	case GEN_BINARY_NE:
	case GEN_BINARY_GE:
	case GEN_BINARY_GT:
	  relopLevel++;
	  /* cheat -- don't need separate selector for each case */
	  PrePass(gen_BINARY_LT_get_rvalue1(node));
	  PrePass(gen_BINARY_LT_get_rvalue2(node));
	  relopLevel--;
	  break;
	  
	case GEN_BINARY_CONCAT:
	  {
	    int len = NewStringLength(node);
	    if (len == STAR_LEN)
	      ERROR("PrePass","Character expression must have fixed length",FATAL);
	    aiExpressionStackSpace += Align(len);
	  }
	  break;
       
	case GEN_ASSIGN:
	  RecordLabel(gen_ASSIGN_get_lbl_def(node),AST_NIL);
	  assigned_label(gen_ASSIGN_get_lbl_ref(node));
          fst_my_PutFieldByIndex(ft_SymTable, getIndex(gen_ASSIGN_get_name(node)), SYMTAB_scratch,  TRUE);
	  break;

        case GEN_ASSIGNMENT:
	  RecordLabel(gen_ASSIGNMENT_get_lbl_def(node),AST_NIL);
	  mark_assignment( node );
	  break;

	case GEN_BINARY_EXPONENT:
	  MakeRoomForExp(node);
	  PrePass(gen_BINARY_EXPONENT_get_rvalue1(node));
	  PrePass(gen_BINARY_EXPONENT_get_rvalue2(node));
	  break;

	case GEN_CALL:
	  RecordLabel(gen_CALL_get_lbl_def(node),AST_NIL);
          inv  = gen_CALL_get_invocation(node);
	  text = gen_get_text(gen_INVOCATION_get_name(inv));

	  itype = gen_get_real_type(gen_INVOCATION_get_name(inv));

	  if (itype == TYPE_NONE) itype = TYPE_UNKNOWN;

	  i = SymInsertSymbol(text, itype, OC_IS_EXECUTABLE, 
		0, SC_SUBROUTINE, NO_ALIAS);
	  check_actuals(text, gen_INVOCATION_get_actual_arg_LIST(inv));
	  break;

	case GEN_COMMENT: 
	  if (aiDirectiveIsInComment(node) && aiParseComments &&
	      (aiStatementIsPrefetch(node) || aiStatementIsFlush(node)))
	    PrePass(GET_DIRECTIVE_INFO(node)->Subscript);
	  break;

	case GEN_COMMON:
	  WalkCommon(node);
	  break;

	case GEN_COMPLEX_CONSTANT:

	  /*  This strategy was abandoned because the code that handles    */
	  /*  complex constants assumes that both parts are real:	   */
	  /*								   */
	  /*  can the real and imaginary part be represented as integers?  */
	  /*  if (ast_get_scratch(node)!=OC_IS_FORMAL_PAR)		   */
	  /*  {								   */
	  /*    convertToInt(gen_COMPLEX_CONSTANT_get_real_const(node));   */
	  /*    convertToInt(gen_COMPLEX_CONSTANT_get_imag_const(node));   */
	  /*  }								   */

	  /*  This code forces the parts of the complex constant to be	   */
	  /*  real.  This forces the assumption made in the original code  */
	  /*  to be correct.						   */

	  /*  Ultimately, the situation with regard to complex constants   */
	  /*  needs to be corrected.  We need to be able to load them as   */
	  /*  integers when possible.  In addition, we need to be able to  */
	  /*  handle double precision complex constants.                   */
	  
	  convertToReal(gen_COMPLEX_CONSTANT_get_real_const(node));
	  convertToReal(gen_COMPLEX_CONSTANT_get_imag_const(node));

	  (void) NameFromConstantNode( node, buffer );
	  ct   = gen_get_real_type(node);
	  text = ConstantName(buffer, ct, name_buffer);

 	  i = SymInsertSymbol(text, ct, OC_IS_DATA, 0, 
		SC_CONSTANT, NO_ALIAS);
	  break;

	case GEN_CONSTANT:
	  
	  /*  Can a floating point constant be represented as an integer?  */
	  if (ast_get_scratch(node) != OC_IS_FORMAL_PAR)
	    convertToInt(node);


	  ct = gen_get_real_type(node);
	  text = ConstantName(gen_get_text(node), ct, name_buffer);
	  i = SymInsertSymbol(text, ct, OC_IS_DATA, 0, SC_CONSTANT, 
			      NO_ALIAS);
	  break;

	case GEN_DATA:
	  HandleData(node);
	  break;

        case GEN_DO:
	  RecordLabel(gen_DO_get_lbl_def(node),AST_NIL);
	  find_modified_vars( node );
          break;

	case GEN_IDENTIFIER:
	  i = getIndex(node);
	  if (gen_get_real_type(node) == TYPE_UNKNOWN && iloc_intrinsic
	      ((char *) fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_NAME)) == A2I_INVALID_OPCODE
	      && !(fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_OBJECT_CLASS) & 
		(OC_IS_FORMAL_PAR | OC_IS_EXECUTABLE)))
	  {
	    (void) sprintf(error_buffer, "The name '%s' has no known type", 
		    (char *) fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_NAME));
	    ERROR("PrePass", error_buffer, SERIOUS);
	    ERROR("PrePass", 
	       "This indicates a type checking problem in the editor",SERIOUS);
	  }
	  break;

	case GEN_INVOCATION:
	  itype = gen_get_real_type(gen_INVOCATION_get_name(node));

	  if (itype == TYPE_UNKNOWN) /* s.b. unnecessary w/TC in ned */
 	     itype = gen_get_real_type(node);

	  if (itype == TYPE_UNKNOWN)
	  {
	    (void) sprintf(error_buffer, "procedure '%s' has no AST real type",
		    gen_get_text(gen_INVOCATION_get_name(node)) );
	    ERROR("PrePass", error_buffer, FATAL);
	  }
	  text = gen_get_text(gen_INVOCATION_get_name(node));
	  i = fst_my_QueryIndex(ft_SymTable, text);
	    
	  if ((i == SYM_INVALID_INDEX) || (!(fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS) & SC_STMT_FUNC))) {
	    i = SymInsertSymbol(text, itype, OC_IS_EXECUTABLE, 0, SC_FUNCTION, 
				NO_ALIAS);
	  }
	  
	  /* change 6/20/91
	   * intrinsic "char" has default length of one
	   */
	  if (itype == TYPE_CHARACTER && strcmp(text,"char") == 0 &&
	      (!(fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS) & SC_STMT_FUNC)))
	    {
	      fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_CHAR_LENGTH,  1);
	    }
	  
	  /* change 6/14/91
	   * allocate return space for the function
	   */
	  if (itype == TYPE_CHARACTER) {
	    if (fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_CHAR_LENGTH) == STAR_LEN) {
	      (void) sprintf(error_buffer,"Character function %s has unknown length",
		      gen_get_text(gen_INVOCATION_get_name(node)));
	      ERROR("PrePass",error_buffer,FATAL);
	    }
	    aiExpressionStackSpace += Align(fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_CHAR_LENGTH));
	  }
	  /* change 6/26/91
	   * lets check the space for statement functions differently
	   */
	  if (fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS) & SC_STMT_FUNC)
	    check_stmt_actuals(node);
	  else
	    check_actuals(text, gen_INVOCATION_get_actual_arg_LIST(node));
	  break;

	case GEN_PAUSE:
	  BuildIOL (node, AST_NIL, gen_PAUSE_get_constant(node), /*mod*/0, /*ref*/0);
	  break;

	case GEN_STOP:
	  BuildIOL (node, AST_NIL, gen_STOP_get_constant(node), /*mod*/0, /*ref*/0);
	  break;

	case GEN_FORMAT:
	  RecordLabel(gen_FORMAT_get_lbl_def(node),node);
	  break;

	case GEN_LABEL_DEF:
	  RecordLabel(node,AST_NIL);
	  break;

	case GEN_PRINT:
	  BuildIOL(node, gen_PRINT_get_format_identifier(node),
		   gen_PRINT_get_data_vars_LIST(node), /*mod*/0, /*ref*/1);
	  break;

	case GEN_READ_SHORT:
	  BuildIOL(node, gen_READ_SHORT_get_format_identifier(node),
		   gen_READ_SHORT_get_data_vars_LIST(node),
		   /*mod*/1, /*ref*/0);
	  break;

	case GEN_READ_LONG:
	  BuildIOL(node, gen_READ_LONG_get_kwd_LIST(node),
		   gen_READ_LONG_get_io_LIST(node), /*mod*/1, /*ref*/0);
	  break;

	case GEN_WRITE:
	  BuildIOL(node, gen_WRITE_get_kwd_LIST(node), 
		   gen_WRITE_get_data_vars_LIST(node), /*mod*/0, /*ref*/1);
	  break;

	case GEN_OPEN:
	  BuildIOL(node, gen_OPEN_get_kwd_LIST(node), AST_NIL, 
		   /*mod*/0,/*ref*/1);
	  break;

	case GEN_CLOSE:
	  BuildIOL(node, gen_CLOSE_get_kwd_LIST(node), AST_NIL, 
		   /*mod*/0,/*ref*/1);
	  break;

	case GEN_BACKSPACE_SHORT:
	  BuildIOL(node, gen_BACKSPACE_SHORT_get_unit_identifier(node), 
		   AST_NIL, /*mod*/0,/*ref*/1);
	  break;

	case GEN_BACKSPACE_LONG:
	  BuildIOL(node, gen_BACKSPACE_LONG_get_kwd_LIST(node), 
		   AST_NIL, /*mod*/0,/*ref*/1);
	  break;

	case GEN_ENDFILE_SHORT:
	  BuildIOL(node, gen_ENDFILE_SHORT_get_unit_identifier(node), 
		   AST_NIL, /*mod*/0,/*ref*/1);
	  break;

	case GEN_ENDFILE_LONG:
	  BuildIOL(node, gen_ENDFILE_LONG_get_kwd_LIST(node), AST_NIL, 
		   /*mod*/0,/*ref*/1);
	  break;

	case GEN_INQUIRE:
	  BuildIOL(node, gen_INQUIRE_get_kwd_LIST(node), AST_NIL, 
		   /*mod*/0,/*ref*/1);
	  break;

	case GEN_REWIND_SHORT:
	  BuildIOL(node, gen_REWIND_SHORT_get_unit_identifier(node), 
		   AST_NIL, /*mod*/0,/*ref*/1);
	  break;

	case GEN_REWIND_LONG:
	  BuildIOL(node, gen_REWIND_LONG_get_kwd_LIST(node), AST_NIL, 
		   /*mod*/0,/*ref*/1);
	  break;

	default:
	  n = ast_get_node_type_son_count(type);
	  for (i=1; i<=n; i++)
	      PrePass( ast_get_son_n(node, i) );
	  break;
     }
   }
} /* PrePass */




/* Record all labels appearing in an ASSIGN statement */
static void assigned_label( AST_INDEX node )
//   AST_INDEX	node;
{
  STR_TEXT	AST_label;
  int		iloc_label;

  if (ast_get_node_type(node) == GEN_LABEL_REF)
  {
    AST_label = string_table_get_text(ast_get_symbol(node));
    LabelInAssign(AST_label, &iloc_label);
  }
  else
     ERROR("AssignedLabel", "Missing target statement label", FATAL);
} /* assigned_label */




/* Mark variables modified in a DO loop.  In particular, annotate */
/* AST to indicate whether or not loop indices are modified       */
static void find_modified_vars( AST_INDEX node )
//   AST_INDEX	node;
{
  AST_INDEX son;
  int i, n, Index;

  if (aiDebug > 0)
     (void) fprintf(stdout, "FindModVars: looking.\n");

  son = gen_DO_get_control(node);
  if (gen_get_node_type(son) == GEN_INDUCTIVE)
  {
    if (aiDebug > 0)
       (void) fprintf(stdout, "FindModVars: inside loop!\n");

    son = gen_INDUCTIVE_get_name(son); /* induction variable */

    Index = getIndex(son);
    if (Index != -1)
       fst_my_PutFieldByIndex(ft_SymTable, Index, SYMTAB_scratch,  FALSE);

    n = ast_get_node_type_son_count(ast_get_node_type(node));
    for (i=1; i<=n; i++)
    {
      if (i != DO_lbl_def)
	 PrePass( ast_get_son_n(node, i) );
    }

    if (fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_scratch) == FALSE)
    {
      ast_put_scratch(node, INDEX_NOT_MODIFIED);
      if (aiDebug > 0)
	 (void) fprintf(stdout, "FindModVars: index not modified.\n");
    }
    else
    {
      ast_put_scratch(node, INDEX_MODIFIED);
      if (aiDebug > 0)
	 (void) fprintf(stdout, "FindModVars: index modified.\n");
    }

    /* and mark it as modified by the loop in case of nested loops */
    fst_my_PutFieldByIndex(ft_SymTable, Index, SYMTAB_scratch,  TRUE);

  }
  else
  {
    n = ast_get_node_type_son_count(gen_get_node_type(node));
    for (i=1; i<=n; i++)
	PrePass( ast_get_son_n(node, i) );
  }
} /* find_modified_vars */




/* mark modified variables in assignment statements */
static void mark_assignment( AST_INDEX node )
  // AST_INDEX node;
{
  AST_INDEX son;

  if (aiDebug > 0)
     (void) fprintf(stdout, "MarkAssignment: looking.\n");

  son = gen_ASSIGNMENT_get_lvalue(node);

  if (gen_get_node_type(son) == GEN_SUBSCRIPT)
     son = gen_SUBSCRIPT_get_name(son);

  if (gen_get_node_type(son) == GEN_IDENTIFIER) /* mark it by changing it */
  {
    fst_my_PutFieldByIndex(ft_SymTable, getIndex(son), SYMTAB_scratch,  TRUE);
    if (aiDebug > 0)
       (void) fprintf(stdout, "MarkAssignment: variable %s modified!\n",
	      (char *) fst_my_GetFieldByIndex(ft_SymTable, getIndex(son), SYMTAB_NAME));
  }

  PrePass( gen_ASSIGNMENT_get_lvalue(node));
  PrePass( gen_ASSIGNMENT_get_rvalue(node));
} /* mark_assignment */




/* Record detected AST labels in the label table */
static void RecordLabel( AST_INDEX node, AST_INDEX stmt )
  // AST_INDEX	node;
  // AST_INDEX	stmt;
{
  int Index, iloc_label;

  if (node == ast_null_node) 
     return;

  Index = LabelDefine(gen_get_text(node), &iloc_label, stmt);  

  if (aiDebug > 0)
     (void) fprintf(stdout, "RecordLabel('%s'(%d)) returns flag %d, iloc label %d.\n",
	     gen_get_text(node), node, Index, iloc_label);

} /* RecordLabel */




/* check the space requirements for statment functions */
static void check_stmt_actuals( AST_INDEX invocation )
  // AST_INDEX invocation;
{
  AST_INDEX   parm_list   = gen_INVOCATION_get_actual_arg_LIST(invocation);
  AST_INDEX   list        = list_first(parm_list);
  int         actualSpace = 0;
  int         size;
  int         ct;
  char       *text;

  while (list != ast_null_node) {
    switch (gen_get_node_type(list)) {
    case GEN_CONSTANT:
      ct    = gen_get_converted_type(list);
      text  = ConstantName(gen_get_text(list), ct, name_buffer);
      (void) SymInsertSymbol(text, ct, OC_IS_DATA, 0, SC_CONSTANT, 
			     NO_ALIAS);
      break;
    default:
      if (gen_get_real_type(list) == TYPE_CHARACTER) {
	if ((size = NewStringLength(list)) == STAR_LEN) {
	  if (gen_get_node_type(list) == GEN_BINARY_CONCAT)
	    ERROR("check_stmt_actuals","Binary concat has operand of * length",FATAL);
	}
	else 
	  actualSpace += Align(size);
      }
      break;
    }
    list = list_next(list);
  }
  aiExpressionStackSpace += actualSpace;
} /* check_stmt_actuals */




/* check the space requirements for functions other than statement functions */
static void check_actuals( char *name, AST_INDEX node )
  // char *name;
  // AST_INDEX node;
{
  AST_INDEX	list;
  int		Index, actualSpace, size, type, ct, intrinsic;
  char 		*text;

  actualSpace = 0;

  if (iloc_intrinsic(name) != A2I_INVALID_OPCODE)
     intrinsic = TRUE;		/* call to intrinsic */
  else
     intrinsic = FALSE;		/* call to user code */

  list = list_first(node);

  while(list != ast_null_node)
  {
    switch(gen_get_node_type(list))
    {
      case GEN_IDENTIFIER:
	Index = getIndex(list);

/*	if (intrinsic == FALSE && 
 *	    aiNameIsMod(invocation, getIndex(list)) == TRUE)
 *	   fst_my_PutFieldByIndex(ft_SymTable, Index, SYMTAB_scratch,  TRUE);
 */
	/* EQUALITY in test rules out parameters! */
        if (fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_OBJECT_CLASS) & OC_IS_DATA)
        {
	  /* if it's local, scalar, and has no storage, it will need a	*/
	  /* stack location for storage during the call ...		*/
	  if (fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_STORAGE_CLASS) & SC_NO_MEMORY	&&
	      fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_NUM_DIMS) == 0			&&
	      fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_addressReg) == NO_REGISTER)
	  { /* mark the fact that we have got it's stack space already	*/
	    (void) getAddressRegister(Index);	/* convenient trick!	*/
	    aiExpressionStackSpace += SizeOfType(fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_TYPE));
	    /* doubles might need to be word aligned */
	    if (aiAlignDoubles && fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_SIZE) == 8)
	      aiExpressionStackSpace += 4;
	  }
	}
	actualSpace += max(SizeOfType(TYPE_INTEGER),
			   SizeOfType(fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_TYPE)));

	if (fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_TYPE) == TYPE_CHARACTER)
	  actualSpace += SizeOfType(TYPE_INTEGER); /* hidden length parm */

	if (gen_get_real_type(list) == TYPE_UNKNOWN &&
	    ! (fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_OBJECT_CLASS) & 
		(OC_IS_FORMAL_PAR | OC_IS_EXECUTABLE)) )
	{
	  (void) sprintf(error_buffer, "The name '%s' has no known type", 
		  (char *) fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_NAME));
	  ERROR("CheckActuals", error_buffer, SERIOUS);
	  ERROR("CheckActuals", 
	       "This indicates a type checking problem in the editor", FATAL);
	}
	break;

      case GEN_CONSTANT:
	ct    = gen_get_converted_type(list);
	text  = ConstantName(gen_get_text(list), ct, name_buffer);
	Index = SymInsertSymbol(text, ct, OC_IS_DATA, 0, SC_CONSTANT, 
			      NO_ALIAS);

	if (fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_TYPE) == TYPE_CHARACTER)
	{
	  size  = Align(StringLength(Index));
	  size += SizeOfType(TYPE_INTEGER); /* hidden length parm */
	}
	else
	   size = SizeOfType(fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_TYPE));

	/*  this is used later in the prepass to prevent parameters from  */
        /*  being converted to integers.				  */
	if ((ct == TYPE_REAL) || (ct == TYPE_DOUBLE_PRECISION))
	  ast_put_scratch(list, OC_IS_FORMAL_PAR);

	actualSpace += size;
	aiExpressionStackSpace += size;
	break;

      case GEN_SUBSCRIPT:
	Index = getIndex(gen_SUBSCRIPT_get_name(list));
/*	if (intrinsic == FALSE && 
 *	    aiNameIsMod(invocation, Index) == TRUE)
 *	   fst_my_PutFieldByIndex(ft_SymTable, Index, SYMTAB_scratch,  TRUE);
 */
	actualSpace += SizeOfType(TYPE_INTEGER);
	break;

      case GEN_SUBSTRING:
	if (gen_get_node_type(gen_SUBSTRING_get_substring_name(list)) 
	    == GEN_SUBSCRIPT)
	  Index = getIndex(gen_SUBSCRIPT_get_name(gen_SUBSTRING_get_substring_name(list)));
	else
	  Index = getIndex(gen_SUBSTRING_get_substring_name(list));
/*	if (intrinsic == FALSE && 
 *	    aiNameIsMod(invocation, Index) == TRUE)
 *	   fst_my_PutFieldByIndex(ft_SymTable, Index, SYMTAB_scratch,  TRUE);
 */
	actualSpace += SizeOfType(TYPE_INTEGER); /* ptr to string 	*/
	actualSpace += SizeOfType(TYPE_INTEGER); /* hidden length parm	*/
	break;

	/* change 6/20/91
	 * new case -- binary concat -- must allocate stack space
	 */
      case GEN_BINARY_CONCAT:
	size = NewStringLength(list);
	if (size == STAR_LEN) {
	  (void) sprintf(error_buffer,"Actual argument has illegal (*) length.\n");
	  ERROR("CheckActuals",error_buffer,FATAL);
	}
	aiExpressionStackSpace += Align(size);
	actualSpace += SizeOfType(TYPE_INTEGER) + SizeOfType(TYPE_INTEGER);
	break;

      case GEN_COMPLEX_CONSTANT:
	ast_put_scratch(list, OC_IS_FORMAL_PAR);
	/*  fall through to default handling for general expressions  */

      default:			/* general expression */
	type = gen_get_converted_type(list);
	if (type == TYPE_UNKNOWN)
	{
	  (void) sprintf(error_buffer, 
		"subtree of type '%s' (%d) has no converted type.\n",
		ast_get_node_type_name(gen_get_node_type(list)), list);
	  ERROR("CheckActuals", error_buffer, WARNING);
	}
	size = max(SizeOfType(TYPE_INTEGER), SizeOfType(type));
	aiExpressionStackSpace += size;
	actualSpace += size;
	if (type == TYPE_CHARACTER)
	   actualSpace += SizeOfType(TYPE_INTEGER);
	break;
    }

    /* and check out the actual parameter */
    PrePass(list);

    list = list_next(list);
  }
  if (spaceForActuals < actualSpace)
     spaceForActuals = actualSpace;

  if (intrinsic == FALSE)
     fst_ForAll (ft_SymTable, MarkGlobals, 0);
} /* check_actuals */




/* This routine returns the maximum amount of      */
/* space that could be required for exponentiation */
/*ARGSUSED*/
static void MakeRoomForExp( AST_INDEX op )
  // AST_INDEX	op;
{
  aiExpressionStackSpace += 48;
}




/* Perform the prepass on the elements of the common list */
void WalkCommon(AST_INDEX node)
  // AST_INDEX node;
{
  AST_INDEX 	name_list;
  AST_INDEX	common_elt_list;

  /* get the first common element */
  common_elt_list = list_first(gen_COMMON_get_common_elt_LIST(node));

  while(common_elt_list != ast_null_node)
  {
    /* ignore the common name field */

    /* walk the list of names and mark them in the table	*/
    name_list = list_first(
		  gen_COMMON_ELT_get_common_vars_LIST(common_elt_list));

    while(name_list != ast_null_node) 
    {
	PrePass( name_list );
	name_list = list_next(name_list);
    }

    common_elt_list = list_next(common_elt_list);
  }
} /* WalkCommon */




/*ARGSUSED*/
/* Given no interprocedural information, globals are marked as modified */
static void MarkGlobals(SymDescriptor SymTab, 
			fst_index_t i, 
			Generic dummy)
//   SymDescriptor SymTab;
//   fst_index_t i;
//   Generic dummy;
{
  
  if (fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS) & SC_GLOBAL &&
      fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_OBJECT_CLASS)  & OC_IS_DATA)
    
    /* need an interface to IDFA information */
    fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_scratch,  TRUE);

} /* MarkGlobals */




/* If a floating point number can be represented */
/* as an integer, convert it to an integer       */
void convertToInt(AST_INDEX node)
  //   AST_INDEX	node;
{
  char		*s;
  char		*text;
  int		type;
  int		integerValue;
  double	doubleValue;

  type   = gen_get_real_type(node);
  text = gen_get_text(node);

  /*  Can a floating point constant be represented as an integer?  */
  if ((type == TYPE_REAL) || (type == TYPE_DOUBLE_PRECISION))
  {
    /*  convert string to floating point number  */
    if (type == TYPE_DOUBLE_PRECISION)
    {
      /*  replace "d"s with "e"s  */
      s = text;
      while ((*s!='d')&&(*s!='D')&&(*s!='\0')) s++;
      if (*s!='\0') *s = 'e';
      (void) sscanf(text, "%lg", &doubleValue);
      *s = 'd';     
    }
    else
      (void) sscanf(text, "%lg", &doubleValue);

    /*  change to integer if no precision is lost  */
    /*  when converted to integer value		   */
    integerValue = (int) doubleValue;
    if (doubleValue == (double) integerValue)
    {
      if (aiDebug > 0) 
	(void) fprintf(stdout, 
	  "Changing float constant to integer:  %s == %d\n", 
	  text, integerValue);

      /*  change symbol table information  */
      type = TYPE_INTEGER;
      (void) sprintf (error_buffer, "%d", integerValue);

      /*  change tree information  */
      gen_put_real_type(node, TYPE_INTEGER);
      gen_put_text(node, error_buffer, STR_CONSTANT_INTEGER);	
    }
    else
      if (aiDebug > 0)
      (void) fprintf(stdout, 
	"Cannot change float constant to integer:  %s != %d\n", 
	text, integerValue);
  }
} /* convertToInt */




/* The real and imaginary parts of a complex constant must be represented */
/* as real numbers.  This routine performs the necessary conversion.      */
void convertToReal(AST_INDEX node)
  // AST_INDEX	node;
{
  char		*s;
  
  /*  convert string to floating point number  */
  switch(gen_get_real_type(node))
  {
    case TYPE_DOUBLE_PRECISION:
      /*  replace "d"s with "e"s  */
      s = gen_get_text(node);
      while ((*s!='d')&&(*s!='D')&&(*s!='\0')) s++;
      if (*s!='\0') *s = 'e';
      gen_put_real_type(node, TYPE_REAL);
      break;

    case TYPE_INTEGER:
      /*  add a period  */
      s = gen_get_text(node);
      while (*s!='\0') s++;
      *s++ = '.';
      *s = '\0';
      gen_put_real_type(node, TYPE_REAL);
      break;

    default:
      break;
  }
} /* convertToReal */
