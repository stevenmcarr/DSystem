/* $Id: jumptrans.C,v 1.10 1997/03/11 14:35:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * 
 * This module implements a set of routines which are used for building
 * symbolic jump functions. Given an expression appearing in the AST
 * (perhaps one of the 'actual' parameters at a call site, or an
 * expression appearing as the upper bound of a loop), these routines try
 * to form a "jump function" for the expression, by using the results of
 * symbolic analysis. In this case "jump function" means an expression
 * tree composed of constants, variables (formal parameters), arithmetic
 * operators, and a special BOTTOM node. The representation of this
 * symbolic jump function is abstracted, so as to make this code
 * independent of the code which implements the jump function. A callback
 * scheme is used, where the client of this interface passes in a set of
 * functions which are used to construct the resulting jump function.
 * 
 * Why not just build jump functions using the AST, you ask? The point of
 * having this interface at all is to be able to write out jump functions
 * to a file for interprocedural analysis. The AST is a big heavyweight
 * representation and can be difficult to work with if you have lots of
 * little tree fragments instead of a single tree.
 * 
 * The current implementation of this code requires that it have access to
 * private header files for value numbering.
 * 
 * The object of building a jump function is to get an expression which
 * contains constants and formal parameters, and to be able to detect
 * values which are "unknowable". Consider the following example:
 * 
 *	do i = 1, (q + r - l)
 *	   ...
 *	enddo
 * 
 * In this case, value numbering might be able to evaluate 'r' and 'l' as
 * constants, and we would get back a new expression of just 'q' plus a
 * constant (this might prove useful in later interprocedural analysis).
 * On the other hand, 'q' might be a value read in from a file, in which
 * case we would like to know this (by getting back an expression tree
 * containing BOTTOM).
 * 
 * Another place where this would be useful would be writing out jump
 * functions for interprocedural constant propagation. You want to use the
 * symbolic analysis to get as much information as you can about
 * expressions in an actual list, but then you want to write that
 * information out to a file in a separate form, since the value numbering
 * stuff won't be available in the program compiler.
 *
 * Author: N. McIntosh
 */

#include <libs/moduleAnalysis/cfgValNum/cfgval.i>
#include <libs/frontEnd/ast/forttypes.h>
#include <libs/moduleAnalysis/cfgValNum/jumptrans.h>

/*
 * The following structure acts as a place where we can dump things that
 * we need for using the symbolic analysis routines. See the comment
 * for jump_trans_init() for an explanation of the 'clone' field.
 */

typedef struct JumpTransInfo_struct_tag {

  /* -- these are initialized once per module and then don't change -- */
  FortTree ft;			/* FortTree for module being analyzed */
  CfgInfo cfgGlobals;		/* CFG global information */
  jumptransinfo_callbacks callbacks;

  /* -- these change when we look at a new function with the module -- */
  AST_INDEX rootnode;		/* prog/subr/function node to be analyzed */
  AST_INDEX formals;		/* formals list for subr/prog/func */
  int num_formals;		/* number of formals in list */
  char *name;			/* name of function/subroutine/program */
  CfgInstance cfg;		/* cfg for current module */
  SymDescriptor sd;		/* symbol table for function */

}	JumpTransInfo_struct;

/*
 * This enumeration is used internally to record what type of expression
 * we're looking at. Actual expressions need to be treated differently
 * from arbitrary expressions, due to the way in which the values for
 * actual parameters are represented.
 */
typedef enum TransExprType_enum {
  TRANSEXPR_ACTUAL,
  TRANSEXPR_LOOP_BOUND,
  TRANSEXPR_CONDITIONAL } TransExprType;

/* Forward declarations
*/
static JumpTransPtr
expr_to_jumpfunction(JumpTransInfo info,
		     AST_INDEX expr,
		     TransExprType etype,
		     char *called_function,
		     int which_actual);
					
static JumpTransPtr
val_to_jumpfunction(ValNumber v,
		    JumpTransInfo info,
		    TransExprType etype);
		   
/*------------------------------------------------------------------------*/

/*
 * Initialize the jumpfunction generation interface. Given a forttree,
 * sets up the symbolic analysis stuff. Also stores the set of callbacks
 * to use for jumpfunction construction.
 * 
 * This routine is called once per module. Should be eventually followed by a
 * call to jump_trans_free to deallocate everything.
 */

JumpTransInfo jump_trans_init(FortTree ft, jumptransinfo_callbacks *callbacks)
{
  JumpTransInfo info = new JumpTransInfo_struct;
  
  /*
   * Record the FortTree and then call the CFG/SSA/val_num routines to set
   * things up. Be careful about the ordering of the SSA/VAL stuff --
   * rumor has it that ordering is significant.
   */
  info->ft = ft;
  info->cfgGlobals = cfg_Open(ft);
  cfgval_Open(info->cfgGlobals, false);
  ssa_Open(info->cfgGlobals,
	   /* ipInfo    */ 0,
	   /* ipSmush   */ false,
	   /* doArrays  */ false,
	   /* doDefKill */ false,
	   /* doGated   */ false);
  info->callbacks = *callbacks;
  
  /* Return the initialized structure
   */
  return info;
}

/*
 * Called when we look at a new function. Requests a new CFG, etc. for the
 * module. Also saves information about the formals.
 */

void jump_trans_newfunction(JumpTransInfo info,
			    AST_INDEX rootnode)
{
  AST_INDEX name_node, f;
  int i;

  info->rootnode = rootnode;
  info->formals = get_formals_in_entry(rootnode);
  f = list_first(info->formals);  /* count the formals */
  for (i = 0; f != ast_null_node; i++)
    f = list_next(f);
  info->num_formals = i;
  name_node = get_name_in_entry(rootnode);
  info->name = gen_get_text(name_node);
  info->cfg = cfg_get_inst(info->cfgGlobals, info->name);
  info->sd = ft_SymGetTable(info->ft, info->name);
  cfgval_build_table(info->cfg);
}

/*
 * Dump debugging information. In fact, this stuff has nothing to
 * do with most of the code here, it's just a bunch of calls to 
 * the val/cfg/ssa stuff.
 */

void jump_trans_dump_debug_info(FortTree ft)
{
  CfgInfo cfgGlobals;
  CfgInstance cfg;
  
  cfgGlobals = cfg_Open(ft);
  cfgval_Open(cfgGlobals, false);
  ssa_Open(cfgGlobals,
	   /* ipInfo    */ 0,
	   /* ipSmush   */ false,
	   /* doArrays  */ false,
	   /* doDefKill */ false,
	   /* doGated   */ false);
  
  for (cfg = cfg_get_first_inst(cfgGlobals); cfg;
       cfg = cfg_get_next_inst(cfg)) {
    cfgval_build_table(cfg);
    printf("## dump of CFG\n");
    cfg_dump(cfg);
  }
  printf("## dump of tree\n");
  tree_print(ft_Root(ft));
  return;
}

/*
 * Cleanup routine to be called when you are done with the jumptrans_
 * interface.
 */

void jump_trans_free(JumpTransInfo info)
{
  cfgval_Close(info->cfgGlobals);
  ssa_Close(info->cfgGlobals);
  cfg_Close(info->cfgGlobals);
  delete info;
}

/*
 * Given an expression appearing as a loop bound or step, examine the
 * expression and return a jump function for it.
 */

JumpTransPtr
loopbound_expr_to_jumpfunction(AST_INDEX expr, JumpTransInfo info)
{
  return expr_to_jumpfunction(info, expr, TRANSEXPR_LOOP_BOUND,
			      "<dummy>", 0);
}

/*
 * Given an expression appearing as a loop bound or step, examine the
 * expression and return a jump function for it.
 */

JumpTransPtr
conditional_expr_to_jumpfunction(AST_INDEX expr, JumpTransInfo info)
{
  return expr_to_jumpfunction(info, expr, TRANSEXPR_CONDITIONAL,
			      "<dummy>", 0);
}

/*
 * Given the AST corresponding to the actual list at a call site, return
 * an array of jump functions, one per actual. The array "ret_arr" is
 * assumed to be an array with 'num_actuals' locations; it is filled in as
 * we go along.
 */

void actual_list_to_jumpfunction_list(char *fname,
				      AST_INDEX actual_list,
				      int num_actuals,
				      JumpTransPtr *ret_arr,
				      JumpTransInfo info)
{
  AST_INDEX actual;
  int i = 0;

  if (num_actuals == 0)
    return;

  actual = list_first(actual_list);
  while (actual != ast_null_node) {
    ret_arr[i] = expr_to_jumpfunction(info, actual, TRANSEXPR_ACTUAL,
				      fname, i+1);
    actual = list_next(actual);
    ++i;
  }
}

/*
 * For a given expression, request information from the symbolic analysis
 * routines about the expression in order to try to characterize it. If
 * the expression is a simple function of formals, then translate the
 * expression into an equivalent jumpfunction tree.
 */

static
JumpTransPtr
expr_to_jumpfunction(JumpTransInfo info,
		    AST_INDEX ex,
		    TransExprType etype,
		    char *called_function,
		    int which_actual)
{
  CfgInstance cfg = info->cfg;
  ValNumber v;
  
  /*
   * Special case: start by checking to see if this is an identifier
   * appearing in a PARAMETER statement. If it is, then we should be able
   * to evaluate it just by using some symbol table routines.
   */
  if (ast_get_node_type(ex) == GEN_IDENTIFIER) {
    char *text = gen_get_text(ex);
    int indx = fst_QueryIndex(info->sd, text);
    int value;
    if (FS_IS_MANIFEST_CONSTANT(info->sd, indx)) {
      if (fst_GetFieldByIndex(info->sd, indx, SYMTAB_PARAM_STATUS) ==
	  PARAM_VALUE_DEFINED) {
	value = fst_GetFieldByIndex(info->sd, indx, SYMTAB_PARAM_VALUE);
	return (*(info->callbacks.create_constant))(value);
      } else {
	if (evalConstantIntExpr(info->sd, ex, &value) < 0)
	  return (*(info->callbacks.create_bottom))();
	else
	  return (*(info->callbacks.create_constant))(value);
      }
    }
  }

  /*
   * Call a routine to get a value number for the expression.
   */
  v = cfgval_get_val(cfg, ex);
  
  /*
   * One possibility is that we will get a VAL_RETURN value number type.
   * This indicates that we have received the value number of the actual
   * _after_ the call (i.e. assuming that it's modified), not the value
   * flowing into the call. If this is the case, request the value number
   * flowing into the call.
   */
  if (etype == TRANSEXPR_ACTUAL)
    if (ve_type(VE(cfg, v)) == VAL_RETURN)
      v = ve_input(VE(cfg, v));
  
  /*
   * Build the jumpfunction for it
   */
  return val_to_jumpfunction(v, info, etype);
}

/*
 * This routine relies on private structures, macros, etc. from the value
 * numbering code. It would be great it we could rewrite it using only the
 * utilites provided in val_public.h.
 */

static
JumpTransPtr
val_to_jumpfunction(ValNumber v,
		    JumpTransInfo info,
		    TransExprType etype)
{
  CfgInstance cfg = info->cfg;
  ValEntry *ve = &(VE(cfg, v));
  
  /* dummy ref to defeat compiler warnings (the variable 've' is just
   * around so that we can look at it in the debugger.
   */
  if (!cfg) fprintf(stderr, "yow!\n", ve);
  
  /*
   * Is it an integer constant?
   */
  if (ve_type(VE(cfg, v)) == VAL_CONST &&
      ve_expType(VE(cfg, v)) == TYPE_INTEGER)
    return (*(info->callbacks.create_constant))((int) ve_const(VE(cfg, v)));

  /*
   * Is it a logical constant?
   */
  if (etype == TRANSEXPR_CONDITIONAL &&
      ve_type(VE(cfg, v)) == VAL_CONST &&
      ve_expType(VE(cfg, v)) == TYPE_LOGICAL)
    return (*(info->callbacks.create_constant))((int) ve_const(VE(cfg, v)));

  /*
   * If we hit an actual at this point, it should be due to a parameter
   * modified by a previous call.
   */
  if (ve_type(VE(cfg, v)) == VAL_RETURN &&
      ve_expType(VE(cfg, v)) == TYPE_INTEGER)
    {
      /* fprintf(stderr, "actual modified by previous call.\n"); */
      return (*(info->callbacks.create_bottom))();
    }
  
  /*
   * Check to see if this is of type VAL_ENTRY. This corresponds to the
   * value flowing in from an (unmodified) formal parameter or common
   * block variable.
   */
  if (ve_type(VE(cfg, v)) == VAL_ENTRY &&
      ve_expType(VE(cfg, v)) == TYPE_INTEGER) {
    int i, num_formals;
    char *formal_name;
    fst_index_t ind, ind2, formal_ind;
    AST_INDEX form;
    int objectClass;

    /* Get the symbol table index for this value. 
    */
    ind = cfgval_get_ftsym(cfg, v);

    /* Is this the dummy global? If so, treat it as bottom.
     */
    if (ind == DUMMY_GLOBAL(cfg))
      return (*(info->callbacks.create_bottom))();

    /*
     * We now check for two possibilities. The first possibility is that
     * the entry corresponds to a formal parameter. The second possibility
     * that we want to look for is that the entry corresponds to an
     * unequivalenced common block variable.
     */
    objectClass = fst_GetFieldByIndex(info->sd, ind, SYMTAB_OBJECT_CLASS);
    if ((objectClass & OC_IS_FORMAL_PAR) &&
	(objectClass & OC_IS_DATA)) {

      /*
       * The entry is a formal parameter. Search through the list of
       * formals for it.
       */
      form = list_first(info->formals);
      num_formals = info->num_formals;
      for (i= 0; i < num_formals; i++) {
	formal_name = gen_get_text(form);
	formal_ind = fst_Index(info->sd, formal_name);
	if (ind == formal_ind) 
	  break;
	form = list_next(form);
      }
      if (i == num_formals) { /* couldn't find it */
	fprintf(stderr,
		"jumptrans: found ENTRY, but couldn't locate formal.\n");
	return (*(info->callbacks.create_bottom))();
      }
    
      /*
       * We've got it now, so let's take a look at it.
       */
      switch (gen_get_node_type(form)) {

	case GEN_IDENTIFIER:
	{
	  char *text = gen_get_text(form);
	  int typ = gen_get_real_type(form);
	  int oc = fst_GetField(info->sd, text, SYMTAB_OBJECT_CLASS);
	  
	  if (oc & OC_IS_DATA && typ == TYPE_INTEGER)
	    return (*(info->callbacks.create_var))(gen_get_text(form));
	  else {
	    /*
	    fprintf(stderr, "formal param in expression is not integer var\n");
	    */
	    return (*(info->callbacks.create_bottom))();
	  }
	}
	
	case GEN_STAR:
	default:
	{
	  /* This should never happen */
	  fprintf(stderr, "%s:%d internal error\n",
		  __FILE__, __LINE__);
	  fprintf(stderr, " -- unknown or '*' formal\n");
	  return (*(info->callbacks.create_bottom))();
	}
      }
    } else if (objectClass & OC_IS_COMMON_NAME) {
      int offset, length;
      ValNumber cnameval;
      /* char *common_name; */
      EquivalenceClass *eq;
      
      /*
       * Here we checks things out to see if we can use this common block
       * variable. Don't take it if it looks like we have equivalencing
       * going on.
       */
      offset = (int) ve_offset(VE(cfg, v));
      length = (int) ve_length(VE(cfg, v));
      cnameval = (ValNumber) ve_name(VE(cfg, v));
      /* common_name = (char *) &(ve_string(VE(cfg, cnameval))); */
      eq = fst_Symbol_Offset_Size_To_VarSymbols(info->sd, ind,
						offset, length);
      if (!eq) {
	fprintf(stderr, "%s:%d error -- can't get Equiv Class.\n",
		__FILE__, __LINE__);
	return (*(info->callbacks.create_bottom))();
      }
      if (eq->members != 1)
	return (*(info->callbacks.create_bottom))();
      else {
	char *varname = (char *) fst_GetFieldByIndex(info->sd,
						     eq->member[0],
						     SYMTAB_NAME);
	return (*(info->callbacks.create_var))(varname);
      }
    }
  }
  
  /*
   * Is it a tree node? Recur if so.
   */
  if (ve_type(VE(cfg, v)) == VAL_OP) {
    ValNumber son1, son2;
    ValOpType opt = (ValOpType)ve_opType(VE(cfg, v));
    JumpTransPtr leftson, rightson;
    int ast_optype;
    
    /*
     * Examine the operator to make sure it's kosher.
     */
    switch (opt) {
      case VAL_OP_PLUS: ast_optype = GEN_BINARY_PLUS; break;
      case VAL_OP_MINUS: ast_optype = GEN_BINARY_MINUS; break;
      case VAL_OP_TIMES: ast_optype = GEN_BINARY_TIMES; break;
      case VAL_OP_DIVIDE: ast_optype = GEN_BINARY_DIVIDE; break;
      default: {
	if (etype != TRANSEXPR_CONDITIONAL)
	  return (*(info->callbacks.create_bottom))();
	else switch(opt) {
	  case VAL_OP_NOT: ast_optype = GEN_UNARY_NOT; break;
	  case VAL_OP_AND: ast_optype = GEN_BINARY_AND; break;
	  case VAL_OP_OR: ast_optype = GEN_BINARY_OR; break;
	  case VAL_OP_GE: ast_optype = GEN_BINARY_GE; break;
	  case VAL_OP_GT: ast_optype = GEN_BINARY_GT; break;
	  case VAL_OP_LE: ast_optype = GEN_BINARY_LE; break;
	  case VAL_OP_LT: ast_optype = GEN_BINARY_LT; break;
	  case VAL_OP_EQ: ast_optype = GEN_BINARY_EQ; break;
	  case VAL_OP_NE: ast_optype = GEN_BINARY_NE; break;
	  default: {
	    return (*(info->callbacks.create_bottom))();
	  }
	}
      }
    }
    
    /*
     * Recur.  Special case the NOT operator, which only
     * has one child.
     */
    son1 = ve_left(VE(cfg, v));
    if (opt != VAL_OP_NOT)
      son2 = ve_right(VE(cfg, v));
    leftson = val_to_jumpfunction(son1, info, etype);
    if (opt != VAL_OP_NOT)
      rightson = val_to_jumpfunction(son2, info, etype);
    else
      rightson = (*(info->callbacks.create_constant))(1);
    return (*(info->callbacks.create_op))((ValOpType)ast_optype, leftson, rightson);
  }
  
  /*
   * At this point, we don't know what the heck it is. Time to give up.
   */
  return (*(info->callbacks.create_bottom))();
}
