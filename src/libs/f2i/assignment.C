/* $Id: assignment.C,v 1.1 1997/04/28 20:18:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/*
 *  Individual statements
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
#include <stdio.h>

#include <libs/f2i/ai.h>
#include <libs/f2i/sym.h>
#include <libs/f2i/f2i_label.h>
/* change */
/* added include statements */
#include <libs/f2i/char.h>
#include <include/frontEnd/astsel.h>
/* end change */
#include <libs/f2i/mnemonics.h>

/* functions in this file */
/* HandleAssignment(node)                 generate iloc for AST assignment */
/* static void NonCharacterAssignment(node)                                */
/*                                        generate iloc for NCA            */
/* static void CharacterAssignment(node)  generate iloc for CA             */
/* evalCharExpr(expr, desc, counter, max) evaluate string expression, load */
/*                                        iloc registers w/ length & addr  */

/* general notes */
/* HandleAssignment (HA) chooses NCA (NonCharacterAssignment) or 
 * CharacterAssignment (CA) based type(s) of son(s).
 * Haven't checked out NCA very thouroughly.
 * CA passes most work to other function.  evalCharExpr (eCE) generates
 * iloc registers holding the length and address of the expression and
 * passes them back in struct CharDesc.  If the right-hand-side (RHS)
 * has a concat operator, eCE passes back a list of struct CharDesc.
 * CA creates space for 100 final expression on RHS (ie., 99 concats/line) */

/* forward declarations */

static void NonCharacterAssignment(AST_INDEX);
static void CharacterAssignment(AST_INDEX);

/* HandleAssignment()
 *
 *	generate iloc for a vanilla assignment statement.
 *
 */
void HandleAssignment(AST_INDEX	node) 
  // AST_INDEX node;
{
   int type;

   type = gen_get_converted_type(gen_ASSIGNMENT_get_lvalue(node));
   if (type != TYPE_CHARACTER)
      NonCharacterAssignment(node);
   else
      CharacterAssignment(node);
} /* HandleAssignment */




/* generate code for a non-character assignment */
static void NonCharacterAssignment(AST_INDEX	node)
   // AST_INDEX node;
{
   AST_INDEX lhs;
   AST_INDEX rhs;
   int	t_lhs, lhs_index, lhs_type, rhs_index, rhs_type, Var, Var_type;
   int  AReg;
   char *comment;

   lhs = gen_ASSIGNMENT_get_lvalue(node);	/* get the sons		*/
   rhs = gen_ASSIGNMENT_get_rvalue(node);

   t_lhs = gen_get_node_type(lhs);		/* and their node types	*/

   /* logic goes like this */
   rhs_index = getExprInReg( rhs );
   rhs_type = fst_my_GetFieldByIndex(ft_SymTable, rhs_index, SYMTAB_TYPE);
   
   if (t_lhs == GEN_IDENTIFIER)
   {
     lhs_index = getIndex(lhs);
     lhs_type = fst_my_GetFieldByIndex(ft_SymTable, lhs_index, SYMTAB_TYPE);
     
     if (fst_my_GetFieldByIndex(ft_SymTable, lhs_index, SYMTAB_STORAGE_CLASS) &
	                                                           SC_NO_MEMORY)
     { /* no store needed */
       generate_move(lhs_index, rhs_index, lhs_type);
     }

     else
     { /* need to generate a Store */
       (void) sprintf(error_buffer, "Store into '%s'", 
		(char *) fst_my_GetFieldByIndex(ft_SymTable, lhs_index, SYMTAB_NAME));
       generate(0, NOP, 0, 0, 0, error_buffer);
       AReg = getAddressInRegister(lhs_index);	/* guarded above */
       generate_store(AReg, rhs_index, lhs_type, lhs_index, "&unknown");
       
       /* and the extraneous move ... (see below) */
       generate_move(lhs_index, rhs_index, lhs_type);
     }
   }
   else if (t_lhs == GEN_SUBSCRIPT)
   {
     Var = getIndex(gen_SUBSCRIPT_get_name(lhs));
     Var_type = fst_my_GetFieldByIndex(ft_SymTable, Var, SYMTAB_TYPE);
     /* now, generate the store (and the convert, if necessary	*/
     if (Var_type != rhs_type)
	rhs_index = getConversion(rhs_index, Var_type);

     lhs_index = getSubscriptLValue(lhs);
     comment = GenDepComment(lhs);
     generate_store(lhs_index, rhs_index, rhs_type, Var, comment);
     free(comment);

     /* This move is extraneous from a correctness sense. 	*/
     /* It puts in the move that a peephole optimizer would,	*/
     /* with the sure knowledge that it's dead code if the	*/
     /* peephole couldn't do it!				*/
     Var = StrTempReg("!", lhs_index, rhs_type);
     generate_move(Var, rhs_index, rhs_type);
   }
   else if (t_lhs == GEN_SUBSTRING)
   {
     Var = getIndex(gen_SUBSTRING_get_substring_name(lhs));
     Var_type = fst_my_GetFieldByIndex(ft_SymTable, Var, SYMTAB_TYPE);
     
     /* now, generate the store (and the convert, if necessary	*/
     if (Var_type != rhs_type)
	rhs_index = getConversion(rhs_index, Var_type);

     lhs_index = getSubstringAddress(lhs);
     generate_store(lhs_index, rhs_index, rhs_type, Var,"&unknown");

     /* This move is extraneous from a correctness sense. 	*/
     /* It puts in the move that a peephole optimizer would,	*/
     /* with the sure knowledge that it's dead code if the	*/
     /* peephole couldn't do it!				*/
     Var = StrTempReg("!", lhs_index, rhs_type);
     generate_move(Var, rhs_index, rhs_type);
   }
} /* NonCharacterAssignment */




/* generate code for a character assignment */
static void CharacterAssignment(AST_INDEX	node)
  // AST_INDEX node;
{
  AST_INDEX lhs;
  AST_INDEX rhs;

  struct CharDesc RHS[MAX_CONCATS], LHS[2];
  int	nextRHS = 0;
  int	nextLHS = 0;

  RHS[nextRHS].addr = -1; 	/* initial condition */
  LHS[nextLHS].addr = -1;

  lhs   = gen_ASSIGNMENT_get_lvalue(node);
  rhs   = gen_ASSIGNMENT_get_rvalue(node);

  /* evaluate and check LHS */
  evalCharExpr(lhs, LHS, &nextLHS, 2);

  /* same for RHS */
  evalCharExpr(rhs, RHS, &nextRHS, MAX_CONCATS);

  /* and copy the string(s) */
  generate_move_string(LHS,RHS);
} /* CharacterAssignment */




/* evaluate a character expression */
void evalCharExpr(AST_INDEX	expr, struct CharDesc desc[], int *counter, int maximum)
//      AST_INDEX expr; 
//      int *counter;
//      struct CharDesc desc[];
//      int maximum;
{
  int type = gen_get_node_type(expr);
  int i;

  switch(type)
  {
  case GEN_CONSTANT:
    i    = getIndex(expr);
    desc[*counter].addr = getAddressInRegister(i);
    desc[*counter].misc  = StringLength(i);
    desc[*counter].description = CHAR_NORMAL;
    (*counter)++;
    desc[*counter].addr = END_OF_CHAR_LIST;
    break;
  case GEN_BINARY_CONCAT:
    if ( (*counter) + 2 >= maximum) {
      ERROR("evalCharExpr","Too many expression in concatenation expression",FATAL);
      return;
    }
    evalCharExpr(gen_BINARY_CONCAT_get_rvalue1(expr),desc,counter,maximum);
    evalCharExpr(gen_BINARY_CONCAT_get_rvalue2(expr),desc,counter,maximum);
    break;
  case GEN_IDENTIFIER:
    i = getIndex(expr);
    desc[*counter].addr = getAddressInRegister(i);
    desc[*counter].misc = fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_CHAR_LENGTH);
    if (desc[*counter].misc == STAR_LEN) {
      desc[*counter].misc = StrTempReg("s-len",i,TYPE_INTEGER);
      desc[*counter].description = CHAR_UNKNOWN_LEN;
    }
    else
      desc[*counter].description = CHAR_NORMAL;
    (*counter)++;
    desc[*counter].addr = END_OF_CHAR_LIST;
    break;
  case GEN_SUBSCRIPT:
    i = getIndex(gen_SUBSCRIPT_get_name(expr));
    desc[*counter].addr = getSubscriptLValue(expr);
    desc[*counter].misc = fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_CHAR_LENGTH);
    if (desc[*counter].misc == STAR_LEN) {
      desc[*counter].misc = StrTempReg("s-len",i,TYPE_INTEGER);
      desc[*counter].description = CHAR_UNKNOWN_LEN;
    }
    else
      desc[*counter].description = CHAR_NORMAL;
    (*counter)++;
    desc[*counter].addr = END_OF_CHAR_LIST;
    break;
  case GEN_SUBSTRING:
    desc[*counter].addr = getSubstringAddress(expr);
    desc[*counter].misc = getSubstringLength(expr);
    desc[*counter].description = CHAR_UNKNOWN_LEN;
    (*counter)++;
    desc[*counter].addr = END_OF_CHAR_LIST;
    break;
  case GEN_INVOCATION:
    /* desc[*counter].addr not used */
    desc[*counter].addr = ~END_OF_CHAR_LIST;  /* could be anything */
    desc[*counter].misc = expr;
    desc[*counter].description = CHAR_FUNCTION;
    (*counter)++;
    desc[*counter].addr = END_OF_CHAR_LIST;
    break;
  default:
    ERROR("CharacterAssignment",
	  "RHS of character assignment is too complex for AI today",
	  SERIOUS);
    ERROR("CharacterAssignment", 
	  "No code will be generated for the statement", SERIOUS);
    return;
  }

} /* evalCharExpr */
