/* $Id: PEDInterface.C,v 1.1 1997/06/25 14:41:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/**************************************************************************
  PEDInterface.c -- These routines provide the ParaScope interface for
  the data partitioning tool and performance estimator.

  (c) Rice University, Caltech C3P and CRPC.

  Author: Vas, July 1990.

  Modification History:
 **************************************************************************/

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/fort.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/symtab.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/perf.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>

/* forward declarations */
/*AST_INDEX  get_invocation_stmt_list();*/

/*---------------------------------------------------------------------------
   process_parameter_stmt --    Initialize the sym_rent fields for those
   variables whose values are defined in the parameter statement.
  ---------------------------------------------------------------------------*/
void
process_parameter_stmt(AST_INDEX stmt)
{
  AST_INDEX    plist, curr, pid, pval;
  static char  name[20], vstr[20];
  Boolean      newp = false;
  int          value, pindex, sindex;

  plist = gen_PARAMETER_get_param_elt_LIST(stmt);
  for (curr = list_first(plist); curr != AST_NIL; curr = list_next(curr)) {
    value = 0;
    pid = gen_PARAM_ELT_get_name(curr);
    pval = gen_PARAM_ELT_get_rvalue(curr);
    sprintf(name, "%s", gen_get_text(pid));
    sprintf(vstr, "%s", gen_get_text(pval));
    sindex = sym_search(name, &newp);
    if (is_constant(pval)) {
      /* enter value into symtab */
      sscanf(vstr, "%d", &value);
      /* enter the value into rent field 0 */
      if (newp)
	set_sym_rent(sindex, 0, value);
    }
    else {
      /* check if the rvalue was defined by an earlier parameter stmt */
      pindex = sym_search(vstr, &newp);
      if (NOT(newp)) {
	value = sym_rent(pindex, 0);
	if (value != MINUS_INFTY)
	  set_sym_rent(sindex, 0, value);
      }
    }
  }

}


/*--------------------------------------------------------------------
   get_call_info -- ParaScope interface to get info about subroutine
   call site. Returns name of subroutine, parameter list, number of
   actual parameters and list of stmts in body of subroutine.
 ---------------------------------------------------------------------*/
void
get_call_info(AST_INDEX stmt, char *name, AST_INDEX *parmlist, 
              int *numparms, AST_INDEX *body)
{
  AST_INDEX   invocation, name_id;

  if (! is_call(stmt)) {
    return;
  }
  invocation = gen_CALL_get_invocation(stmt);
  name_id = gen_INVOCATION_get_name(invocation);
  sprintf(name, "%s", gen_get_text(name_id));
  *parmlist = gen_INVOCATION_get_actual_arg_LIST(invocation);
  *numparms = list_length(*parmlist);
  *body = get_invocation_stmt_list(stmt, name);
}

/*--------------------------------------------------------------------
   get_function_info -- ParaScope interface to get info about function
   call. Returns name of the function, actual parameter list, number
   of parameters and list of stmts in the body of the function.
 ---------------------------------------------------------------------*/
void
get_function_info(AST_INDEX stmt, char *name, AST_INDEX *parmlist, 
                  int *numparms, AST_INDEX *body)
{
  AST_INDEX   name_id;

  *parmlist = AST_NIL;
  *numparms = 0;
  
  if (! is_invocation(stmt) && ! is_subscript(stmt)) {
    /* Note: due to an Rn bug, Ped sometimes gets confused
       between function invocations and subscript references! */
    return;
  }

  if (is_invocation(stmt)) {
    name_id = gen_INVOCATION_get_name(stmt);
    sprintf(name, "%s", gen_get_text(name_id));
    *parmlist = gen_INVOCATION_get_actual_arg_LIST(stmt);
    *numparms = list_length(*parmlist);
    *body = get_invocation_stmt_list(stmt, name);
  }
  else {
    name_id = gen_SUBSCRIPT_get_name(stmt);
    sprintf(name, "%s", gen_get_text(name_id));
    *parmlist = gen_SUBSCRIPT_get_rvalue_LIST(stmt);
    *numparms = list_length(*parmlist);
    *body = get_invocation_stmt_list(stmt, name);
  }
  
}

/*------------------------------------------------------------------------
   get_invocation_stmt_list -- return the list of stmts in the body of the
   subroutine / function whose name is "name"
  ------------------------------------------------------------------------*/
AST_INDEX
get_invocation_stmt_list(AST_INDEX stmt, char *name)
{
  AST_INDEX   curr, sname_id, list;
  static char sname[100];

  list = AST_NIL;

  if (Program_Root == AST_NIL) {
    /* find root of program */
    for (curr = stmt; curr != AST_NIL; curr = out(curr)) {
      if (is_program(curr))
	break;
    }
    if (curr == AST_NIL)
      return (list);
    else
      Program_Root = curr;  /* set global variable Program_Root */
  }

  for (curr = list_next(Program_Root); curr != AST_NIL; curr = list_next(curr)) {
    if (is_subroutine(curr)) {
      sname_id = gen_SUBROUTINE_get_name(curr);
      sprintf(sname, "%s", gen_get_text(sname_id));
      if (strcmp(name, sname) == 0) {
	list = gen_SUBROUTINE_get_stmt_LIST(curr);
	break;
      }
    }
    if (is_function(curr)) {
      sname_id = gen_FUNCTION_get_name(curr);
      sprintf(sname, "%s", gen_get_text(sname_id));
      if (strcmp(name, sname) == 0) {
	list = gen_FUNCTION_get_stmt_LIST(curr);
	break;
      }
    }
  }

  return (list);

}

/*-------------------------------------------------------------------------
   get_assignment_info -- ParaScope interface for assignment stmt info.
   Returns AST_INDEX of lhs and rhs of assignment stmt.
 -------------------------------------------------------------------------*/
void
get_assignment_info(AST_INDEX stmt, AST_INDEX *left, AST_INDEX *right)
{

  if (is_assignment(stmt)) {
    *left = gen_ASSIGNMENT_get_lvalue(stmt);
    *right = gen_ASSIGNMENT_get_rvalue(stmt);
  }
  else {
    *left = AST_NIL;
    *right = AST_NIL;
  }

}

/*------------------------------------------------------------------------
   get_nesting_level -- returns the nesting level of stmt in the ParaScope
   abstract syntax tree representation of the program.
  ------------------------------------------------------------------------*/
int
get_nesting_level(AST_INDEX stmt)
{
  int        i = 0;
  AST_INDEX  curr;
  
  if (is_statement(stmt)) {
    curr = out(stmt);
    while (curr != AST_NIL) {
      i++;
      curr = out(curr);
    }
    if (i <= 0)
      return (-1);
    else
      return (i-1);
  }
  else
    return (-1);  /* not a valid Fortran statement */
}

/*-------------------------------------------------------------------------
   resolve_symbols -- takes a variable number of symbols (i.e., their
   AST_INDEXs as arguments, and returns their values. Note: if the values 
   are not known statically, the user is prompted for an estimate of the 
   max value of the symbol in question.
 -------------------------------------------------------------------------*/
void
resolve_symbols(AST_INDEX stmt, int num_symbols, AST_INDEX *parms, int *values)
{
  int          i, rc, k = 1;
  static char  symbol_str[200], text[20];
  static int   symbol_index[20];
  int          first_time = 1, val, s;
  Boolean      newp = false;
  AST_INDEX    parm_id;

  sprintf(symbol_str, "\0");
  
  /* process the parameters one by one */
  for (i = 1; i <= num_symbols; i++) {
    parm_id = parms[i];
    sprintf(text, "\0");

    if (parm_id == AST_NIL) {
      /* blank symbol */
      continue;
    }
    else if (is_constant(parm_id)) {
      sprintf(text, "%s", gen_get_text(parm_id));
      rc = sscanf(text, "%d", &(values[i]));
      if (rc <= 0 && strlen(text) > (size_t)0) {
	/* add symbol to symbol_str */
      }
      else  continue;
    }
    else if (is_identifier(parm_id)) {
      sprintf(text, "%s", gen_get_text(parm_id));
      /* check the symtab to see if its value has been set in
	 a parameter stmt */
      s = sym_search(text, &newp);
      if (NOT(newp)) {
	val = sym_rent(s, 0);
	if (val != MINUS_INFTY) {
	  values[i] = val;
	  continue;
	}
      }
    }
    else {
      /* this must be an expression */
      /* check if the expression evaluates to a constant */
	sprintf(text, " <expr>");
    }

    if (strlen(text) > (size_t)0) {
      /* add symbol to symbol_str */
      if (first_time == 1) {
	first_time = 0;
      }
      else {
	strcat(symbol_str, ", ");
      }
      strcat(symbol_str, text);
      symbol_index[k++] = i;
    }

  }

  /* invoke PED dialog */
  if (strlen(symbol_str) > (size_t)0) {
    resolve_symbols_dialog(stmt, k-1, symbol_str, symbol_index, values);
  }
  
}

/*-------------------------------------------------------------------------
  expr_eval -- modified version of Chau-Wen's expression evaluator. Returns
  true if expr evaluates to a constant. Note: this version only handles
  linear expressions consisting of + and - operators.
 --------------------------------------------------------------------------*/



