/* $Id: ssa_node.C,v 1.1 1997/06/25 15:10:55 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *
 *
 * -- ssa_node.c
 *
 *	   This function contains functions for building SSA nodes for 
 *	   existing CFG nodes (real references).  This is the 1st phase
 *	   of SSA construction.
 */

#include <string.h>
#include <libs/moduleAnalysis/ssa/ssa_private.h>
#include <libs/moduleAnalysis/ssa/idfa.h>
#include <libs/frontEnd/ast/groups.h>
#include <libs/moduleAnalysis/ssa/ssa_ids.h>
#include <libs/moduleAnalysis/ssa/gstack.h>
#include <libs/moduleAnalysis/cfg/tarjan.h>

/*
 * LEFT/RIGHT operand of binary operation for get_operand()
 */
#define LEFT 0
#define RIGHT 1

STATIC(void, ssa_build_ref_node,(CfgInstance cfg, CfgNodeId cfgId));
STATIC(Boolean, kill_if_killable,());
STATIC(void, do_cilist,(CfgInstance cfg, SsaNodeId cfgId, AST_INDEX astId));
STATIC(void, do_implied_do,());
STATIC(void, do_inductive_loop,(CfgInstance cfg, CfgNodeId cfgId, 
                                AST_INDEX astId));
STATIC(void, add_list,(CfgInstance cfg, CfgNodeId cfgId, SsaNodeId ssaParent,
                       AST_INDEX astId, SsaType type));
STATIC(SsaNodeId, add_nodes,(CfgInstance cfg, CfgNodeId cfgId,
                             SsaNodeId ssaParent, AST_INDEX astId, SsaType type));
STATIC(void, add_sub_nodes,());
STATIC(void, add_CALL,(CfgInstance cfg, CfgNodeId cfgId, SsaNodeId ssaParent,
                       AST_INDEX siteAst));
STATIC(void, add_DUMMY_GLOBAL,(CfgInstance cfg, CfgNodeId cfgId, SsaNodeId ssaParent));
STATIC(void, add_alt_entry_guard,(CfgInstance cfg));
/* static void add_common_vars_to_def_list(); */
STATIC(void, add_substr_exprs,(CfgInstance cfg, SsaNodeId ssaParent, AST_INDEX astId));
STATIC(AST_INDEX, get_operand,(AST_INDEX astId, int son));

/*
 *  Maintain induction variables in the symbol table.
 *  Entry is either CFG_NIL or the CfgNodeId of the most recent
 *  loop with that induction variable.
 *
 *  Never need to clear this field for regular DO loops, because
 *  we can check whether or not the loop includes the statement 
 *  we're checking.
 *
 *  But we do need to clear for implied DO loops; they're limited
 *  to within a single statement.
 */
#define SSA_IVARS "SSA_IVARS: CfgNodeId"

static void init_ivar_info(CfgInstance cfg)
{
    fst_InitField(cfg->symtab, SSA_IVARS, (Generic) CFG_NIL, NULL);
}

static void kill_ivar_info(CfgInstance cfg)
{
    fst_KillField(cfg->symtab, SSA_IVARS);
}

static void enter_ivar(CfgInstance cfg, fst_index_t ivar, CfgNodeId header)
{
    fst_PutFieldByIndex(cfg->symtab, ivar, SSA_IVARS, (Generic) header);
}

/*  true if ivar is the variable for an active loop 
 */
static Boolean check_ivar(CfgInstance cfg, fst_index_t ivar, CfgNodeId node)
{
    CfgNodeId header;

    header = (CfgNodeId) fst_GetFieldByIndex(cfg->symtab, ivar, SSA_IVARS);

    if (header == CFG_NIL)
	return false;
    else
	return BOOL(TARJ_contains(cfg_get_intervals(cfg), header, node));
}



/****************************************************************************
 **			   PUBLIC FUNCTION				   **
 ****************************************************************************/

/*
 * -- ssa_build_ref_nodes
 *
 *	  Build SSA nodes for all the actual cfg nodes references in 
 *	  the current instance (cfg)
 */
void ssa_build_ref_nodes(CfgInstance cfg)
{
    CfgNodeId i;

    init_ivar_info(cfg);

    /*
     * Now build SSA nodes for real references
     */
    for (i = cfg_get_first_node(cfg);
	 i != CFG_NIL;
	 i = cfg_get_next_node(cfg, i))
    {
	if (cfg_is_reachable(cfg, i))
	    ssa_build_ref_node(cfg, i);
    }

    kill_ivar_info(cfg);

} /* end of ssa_build_ref_nodes() */





/*
 * -- ssa_build_ref_node
 *
 *	  Build SSA nodes for actual cfg node references
 */
static void
ssa_build_ref_node(CfgInstance cfg, CfgNodeId cfgId)
{
    AST_INDEX astId;		/* AST index corresponding to the CFG node */
    NODE_TYPE astType;		/* AST node type of astId */
    SsaNodeId head = SSA_NIL;	/* SSA node for whole CFG node, if built */
    AST_INDEX rvalue;		/* children of astId */

    astId = CFG_node(cfg, cfgId)->astnode; 

    if (CFG_node(cfg, cfgId)->ssaKids != SSA_NIL)
    {
	return;
    }

    astType = ast_get_node_type(astId);

    if (loop_stmt(astId))
    {
	astId = gen_get_control(astId);

	if (is_inductive(astId))
	{
	    do_inductive_loop(cfg, cfgId, astId);
	}
	else if (is_conditional(astId))
	{
	    (void) add_nodes(cfg, cfgId, SSA_NIL,
			     gen_CONDITIONAL_get_rvalue(astId),
			     SSA_GUARD_LOGICAL);
	}
	else /* is_repetitive(astId) */
	{
	    (void) add_nodes(cfg, cfgId, SSA_NIL,
			     gen_REPETITIVE_get_rvalue(astId),
			     SSA_GUARD_INTEGER);
	}
    }
    else switch (astType)
    {
      case GEN_ENTRY:
	add_alt_entry_guard(cfg);
	/*
	 *  fallthrough intentionally to next cases
	 */
      case GEN_FUNCTION:
      case GEN_SUBROUTINE:
      case GEN_PROGRAM:
	/*
	 *  entry point of executable subprogram --
	 *  represent initial values of globals (formals and vars in common)
	 */
	add_DUMMY_GLOBAL(cfg, cfgId, SSA_NIL);
	break;

      case GEN_GLOBAL:
      case GEN_BLOCK_DATA:
	break;

      case GEN_ASSIGN:
	(void) ssa_init_node(cfg, cfgId, SSA_NIL,
			     gen_ASSIGN_get_lbl_ref(astId), SSA_DEF,
			     SsaGetSym(cfg, gen_ASSIGN_get_name(astId)));
	break;

      case GEN_ASSIGNMENT:
	/* LHS */
	head = add_nodes(cfg, cfgId, SSA_NIL,
			 gen_ASSIGNMENT_get_lvalue(astId), SSA_DEF);
	/* RHS */
	(void) add_nodes(cfg, cfgId, head,
			 gen_ASSIGNMENT_get_rvalue(astId), SSA_USE);
	break;
	
      case GEN_RETURN:
	rvalue = gen_RETURN_get_rvalue(astId);
	if (is_null_node(rvalue))
	    break;
	
	/* else */
	(void) add_nodes(cfg, cfgId, SSA_NIL, rvalue, SSA_ALT_RETURN);

	break;

      case GEN_COMPUTED_GOTO:
	(void) add_nodes(cfg, cfgId, SSA_NIL, 
			 gen_COMPUTED_GOTO_get_rvalue(astId),
			 SSA_GUARD_INTEGER);
	break;
	
      case GEN_ARITHMETIC_IF:
	(void) add_nodes(cfg, cfgId, SSA_NIL,
			 gen_ARITHMETIC_IF_get_rvalue(astId),
			 SSA_GUARD_INTEGER);
	break;

      case GEN_ASSIGNED_GOTO:
	(void) add_nodes(cfg, cfgId, SSA_NIL,
			 gen_ASSIGNED_GOTO_get_name(astId), SSA_GUARD_INTEGER);
	break;

      case GEN_LOGICAL_IF:
	(void) add_nodes(cfg, cfgId, SSA_NIL,
			 gen_LOGICAL_IF_get_rvalue(astId), SSA_GUARD_LOGICAL);
	break;

      case GEN_GUARD:
	if (!is_null_node(gen_GUARD_get_rvalue(astId)))
	    (void) add_nodes(cfg, cfgId, SSA_NIL,
			     gen_GUARD_get_rvalue(astId), SSA_GUARD_LOGICAL);
	break;

      case GEN_PRINT:
	/* 
	 *	check format identifier for usable exp, add output vars
	 */
	(void) add_nodes(cfg, cfgId, SSA_NIL,
			 gen_PRINT_get_format_identifier(astId), SSA_USE);

	add_list(cfg, cfgId, SSA_NIL,
		 gen_PRINT_get_data_vars_LIST(astId), SSA_USE);
	break;

      case GEN_WRITE:
	/*
	 *	check cilist for usable exps, add output vars
	 */
	do_cilist(cfg, cfgId, gen_WRITE_get_kwd_LIST(astId));
	
	add_list(cfg, cfgId, SSA_NIL,
		 gen_WRITE_get_data_vars_LIST(astId), SSA_USE);
	break;

      case GEN_READ_SHORT:
	/*
	 *	check format identifier for usable exp, add def for input vars
	 */
	(void) add_nodes(cfg, cfgId, SSA_NIL,
			 gen_READ_SHORT_get_format_identifier(astId),
			 SSA_USE);
	
	add_list(cfg, cfgId, SSA_NIL,
		 gen_READ_SHORT_get_data_vars_LIST(astId), SSA_DEF);
	break;

      case GEN_READ_LONG:
	/*
	 *	check cilist for usable exps, add def for input vars
	 */
	do_cilist(cfg, cfgId, gen_READ_LONG_get_kwd_LIST(astId));
	add_list(cfg, cfgId, SSA_NIL,
		 gen_READ_LONG_get_io_LIST(astId), SSA_DEF);
	break;


      case GEN_CLOSE:
	do_cilist(cfg, cfgId, gen_CLOSE_get_kwd_LIST(astId));
	break;

      case GEN_OPEN:
	do_cilist(cfg, cfgId, gen_OPEN_get_kwd_LIST(astId));
	break;

      case GEN_INQUIRE:
	do_cilist(cfg, cfgId, gen_INQUIRE_get_kwd_LIST(astId));
	break;

      case GEN_BACKSPACE_SHORT:
	(void) add_nodes(cfg, cfgId, SSA_NIL,
			 gen_BACKSPACE_SHORT_get_unit_identifier(astId),
			 SSA_USE);
	break;

      case GEN_BACKSPACE_LONG:
	do_cilist(cfg, cfgId, gen_BACKSPACE_LONG_get_kwd_LIST(astId));
	break;

      case GEN_ENDFILE_SHORT:
	(void) add_nodes(cfg, cfgId, SSA_NIL,
			 gen_ENDFILE_SHORT_get_unit_identifier(astId),
			 SSA_USE);
	break;

      case GEN_ENDFILE_LONG:
	do_cilist(cfg, cfgId, gen_ENDFILE_LONG_get_kwd_LIST(astId));
	break;

      case GEN_REWIND_SHORT:
	(void) add_nodes(cfg, cfgId, SSA_NIL,
			 gen_REWIND_SHORT_get_unit_identifier(astId),
			 SSA_USE);
	break;

      case GEN_REWIND_LONG:
	do_cilist(cfg, cfgId, gen_REWIND_LONG_get_kwd_LIST(astId));

	break;

      case GEN_DATA:
	/*
	 *  Evaluation of expressions is for later phases
	 *  (constant propagator or value numberer) to mess with.
	 */
	break;
 

      case GEN_COMMON:
      case GEN_TASK_COMMON:
	/*
	 *  Add on-entry pseudo-defs to common (and task_common) 
	 *  vars later, hanging them off the subprogram entry points.
	 */
	break;


      case GEN_CALL:
	/*
	 *  add exps for actual args passed, any args (possibly) def'd 
	 *  by called routine, any common vars whose value is 
	 *  (possibly) changed by called routine
	 */
	add_CALL(cfg, cfgId, SSA_NIL, gen_CALL_get_invocation(astId));
	break;
 

      case GEN_TASK:
	/*
	 * is_task: same as call
	 */
	add_CALL(cfg, cfgId, SSA_NIL, gen_TASK_get_invocation(astId));
	break;

      default:
	break;

    } /* end of switch on cfg node type */

} /* end of ssa_build_ref_node() */




/****************************************************************************
 **		      PRIVATE FUNCTIONS: do_*()				   **
 ****************************************************************************/

/*
 *  do_inductive_loop
 *
 *  Add special ssa nodes for inductive loop -- init, bound, stride, etc.
 */
static void do_inductive_loop(CfgInstance cfg, CfgNodeId cfgId, AST_INDEX astId)
{
    SsaNodeId index, guard;
    AST_INDEX ivar;
    fst_index_t ivarSym;
	
    /*
     *	SSA_GUARD_INDUCTIVE represents computation of loop trip count
     */
    guard = ssa_init_node(cfg, cfgId, SSA_NIL, astId,
			  SSA_GUARD_INDUCTIVE, SSA_NIL_NAME);

    /*
     *  SSA_INDUCTIVE represents computation of induction variable
     */
    ivar = gen_INDUCTIVE_get_name(astId);
    ivarSym = SsaGetSym(cfg, ivar);
    enter_ivar(cfg, ivarSym, cfgId);

    index = ssa_init_node(cfg, cfgId, guard, ivar, SSA_INDUCTIVE, ivarSym);

    /*
     *  If we want to add SSA_ETA, do it hereabouts?
     *  (should go at the sink of the loop-exit edge).
     *  Nope, go ahead and add in the loop that adds phis,
     *  to handle both indexes and variants with the same code.
     */

    /*
     *  The LOOP_STEP may have a null ast node, but add the SSA node
     *  anyway for a placekeeper.  add_sub_nodes will just ignore it.
     */
    (void) add_nodes(cfg, cfgId, index,
		     gen_INDUCTIVE_get_rvalue3(astId), SSA_LOOP_STEP);

    /* insert LOOP_BOUND expr before LOOP_STEP */
    (void) add_nodes(cfg, cfgId, index,
		     gen_INDUCTIVE_get_rvalue2(astId), SSA_LOOP_BOUND);

    /* separate expression for LOOP_INIT, before LOOP_BOUND */
    (void) add_nodes(cfg, cfgId, index,
		     gen_INDUCTIVE_get_rvalue1(astId), SSA_LOOP_INIT);

} /* end of do_inductive_loop() */







/* 
 * -- do_cilist
 *
 *	  add SSA nodes for (potentially reducible and/or control flow useful) 
 *	  control information list in i/o and file positioning statements 
 */
static void do_cilist(CfgInstance cfg, SsaNodeId cfgId, AST_INDEX astId)
{
    AST_INDEX specify = AST_NIL;
    AST_INDEX query   = AST_NIL;
    NODE_TYPE astType;	 /* AST node type of astId*/

    for ( astId = list_last(astId); 
	  !is_null_node(astId); 
	  astId = list_prev(astId) ) 
    {
	astType = ast_get_node_type(astId);

	switch (astType) 
	{
	  case GEN_UNIT_SPECIFY:   /* is_unit_specify() */
	    specify = gen_UNIT_SPECIFY_get_unit_identifier(astId);
	    break;

	  case GEN_FILE_SPECIFY:   /* is_file_specify() */
	    specify = gen_FILE_SPECIFY_get_rvalue(astId);
	    break;
	    
	  case GEN_FMT_SPECIFY:	   /* is_fmt_specify() */
	    specify = gen_FMT_SPECIFY_get_format_identifier(astId);
	    break;
	    
	  case GEN_REC_SPECIFY:	   /* is_rec_specify() */
	    specify = gen_REC_SPECIFY_get_rvalue(astId);
	    break;

	  case GEN_STATUS_SPECIFY: /* is_status_specify() */
	    specify = gen_STATUS_SPECIFY_get_rvalue(astId);
	    break;

	  case GEN_ACCESS_SPECIFY: /* is_access_specify() */
	    specify = gen_ACCESS_SPECIFY_get_rvalue(astId);
	    break;

	  case GEN_FORM_SPECIFY:   /* is_form_specify() */
	    specify = gen_FORM_SPECIFY_get_rvalue(astId);
	    break;

	  case GEN_RECL_SPECIFY:   /* is_recl_specify() */
	    specify = gen_RECL_SPECIFY_get_rvalue(astId);
	    break;

	  case GEN_BLANK_SPECIFY:  /* is_blank_specify() */
	    specify = gen_BLANK_SPECIFY_get_rvalue(astId);
	    break;

	  case GEN_IOSTAT_QUERY:   /* is_iostat_query() */
	    query = gen_IOSTAT_QUERY_get_lvalue(astId);
	    break;

	  case GEN_EXIST_QUERY:	   /* is_exist_query() */
	    query = gen_EXIST_QUERY_get_lvalue(astId);
	    break;

	  case GEN_OPENED_QUERY:   /* is_opened_query() */
	    query = gen_OPENED_QUERY_get_lvalue(astId);
	    break;

	  case GEN_NUMBER_QUERY:   /* is_number_query() */
	    query = gen_NUMBER_QUERY_get_lvalue(astId);
	    break;

	  case GEN_NAMED_QUERY:	   /* is_named_query() */
	    query = gen_NAMED_QUERY_get_lvalue(astId);
	    break;

	  case GEN_NAME_QUERY:	   /* is_name_query() */
	    query = gen_NAME_QUERY_get_lvalue(astId);
	    break;

	  case GEN_ACCESS_QUERY:   /* is_access_query() */
	    query = gen_ACCESS_QUERY_get_lvalue(astId);
	    break;
	    
	  case GEN_SEQUENTIAL_QUERY: /* is_sequential_query() */
	    query = gen_SEQUENTIAL_QUERY_get_lvalue(astId);
	    break;

	  case GEN_DIRECT_QUERY:   /* is_direct_query() */
	    query = gen_DIRECT_QUERY_get_lvalue(astId);
	    break;

	  case GEN_FORM_QUERY:	   /* is_form_query() */
	    query = gen_FORM_QUERY_get_lvalue(astId);
	    break;

	  case GEN_FORMATTED_QUERY: /* is_formatted_query() */
	    query = gen_FORMATTED_QUERY_get_lvalue(astId);
	    break;

	  case GEN_UNFORMATTED_QUERY: /* is_unformatted_query() */
	    query = gen_UNFORMATTED_QUERY_get_lvalue(astId);
	    break;

	  case GEN_RECL_QUERY:	    /* is_recl_query() */
	    query = gen_RECL_QUERY_get_lvalue(astId);
	    break;

	  case GEN_BLANK_QUERY:	    /* is_blank_query() */
	    query = gen_BLANK_QUERY_get_lvalue(astId);
	    break;

	  case GEN_NEXTREC_QUERY:   /* is_nextrec_query() */
	    query = gen_NEXTREC_QUERY_get_lvalue(astId);
	    break;

	  default:
	    break;
	}

	if (!is_null_node(specify))
	{
	    (void) add_nodes(cfg, cfgId, SSA_NIL, specify, SSA_USE);
	    specify = AST_NIL;
	}
	if (!is_null_node(query))
	{
	    (void) add_nodes(cfg, cfgId, SSA_NIL, query, SSA_DEF);
	    query = AST_NIL;
	}
    } /* end of for each node in the AST list */

} /* end of do_cilist() */



/*
 * -- do_implied_do
 *
 *	     insert expression for any part of an implied_do node
 *	     which has a variable reference
 */
static void do_implied_do(CfgInstance cfg, CfgNodeId cfgId, SsaNodeId ssaParent, 
                          AST_INDEX astId, SsaType type)
{
    AST_INDEX item;
    SsaNodeId index;
    AST_INDEX ivar;
    fst_index_t ivarSym;

    /*
     *  Insert SSA_ETA first so that it comes last.
     *  The list of the SSA nodes under the CFG parent will ultimately be
     *
     *		SSA_LOOP_*, SSA_INDUCTIVE, ...uses..., SSA_ETA
     *
     *	Unlike for real DO loops, the regular SSA construction is sufficient
     *  to hook these up properly.
     */
    ivar = gen_IMPLIED_DO_get_name(astId);
    ivarSym = SsaGetSym(cfg, ivar);
    enter_ivar(cfg, ivarSym, cfgId);

    (void) ssa_init_node(cfg, cfgId, SSA_NIL, AST_NIL, SSA_ETA, ivarSym);

    /*
     *  Do the list of elements next so they get the definition from
     *  the implied do (they get added to the list of SsaKids first,
     *  and so are later in the list).
     */
    for (item = list_last(gen_IMPLIED_DO_get_imp_elt_LIST(astId));
	 !is_null_node(item);
	 item = list_prev(item) )
    {
	if (is_implied_do(item))
	    do_implied_do(cfg, cfgId, ssaParent, item, type);
	else 
	    (void) add_nodes(cfg, cfgId, ssaParent, item, type);
    }
    /*
     *  We've handled all the inner scope where the implied DO var
     *  is protected
     */
    enter_ivar(cfg, ivarSym, CFG_NIL);

    index = ssa_init_node(cfg, cfgId, SSA_NIL,
			  ivar, SSA_INDUCTIVE, ivarSym);

    /*
     *  check initial, final, and step expressions for "real" var uses
     */
    /*
     *  The LOOP_STEP may have a null ast node, but add the SSA node
     *  anyway for a placekeeper.  add_sub_nodes will just ignore it.
     */
    (void) add_nodes(cfg, cfgId, index,
		     gen_IMPLIED_DO_get_rvalue3(astId), SSA_LOOP_STEP);

    /* insert LOOP_BOUND expr before LOOP_STEP */
    (void) add_nodes(cfg, cfgId, index,
		     gen_IMPLIED_DO_get_rvalue2(astId), SSA_LOOP_BOUND);

    /* separate expression for LOOP_INIT, before LOOP_BOUND */
    (void) add_nodes(cfg, cfgId, index,
		     gen_IMPLIED_DO_get_rvalue1(astId), SSA_LOOP_INIT);

} /* end of do_implied_do() */




/****************************************************************************
 **		      PRIVATE FUNCTIONS: add_*()			   **
 ****************************************************************************/

/*
 * -- add_sub_nodes
 *
 *	    add subordinate uses, under defAst, to list for ssaParent
 */
static void add_sub_nodes(CfgInstance cfg, SsaNodeId ssaParent, AST_INDEX defAst)
{
    CfgNodeId cfgId;	/* CFG node corresponding to the ssaId */

    if (ssaParent == SSA_NIL) return;

    cfgId = SSA_node(cfg, ssaParent)->cfgParent;

    /* 
     *	add exps for actual args if nonconstant, 
     *	try to eval if constant and fcn is intrinsic
     */
    if (is_invocation(defAst))
	add_CALL(cfg, cfgId, ssaParent, defAst);
    
    /* 
     *	take care of subexpressions.
     */
    else if (is_operator(defAst)) 
    {
	(void) add_nodes(cfg, cfgId, ssaParent, get_operand(defAst, LEFT),
			 SSA_USE);
	
	if ((!is_unary_minus(defAst)) && (!is_unary_not(defAst))) 
	{
	    (void) add_nodes(cfg, cfgId, ssaParent, get_operand(defAst, RIGHT),
			     SSA_USE);
	}
    }
    
    else if ((is_constant(defAst)) ||
	     (is_complex_constant(defAst)) ||
	     (is_label_ref(defAst)) ||
	     (is_null_node(defAst)) ||
	     /* else node is null ast with guard ssa node */
	     (is_star(defAst))
	     /* default arg to read, write */
	     )
	/*
	 *  Do nothing -- constant expression
	 */
	;
    
    /* 
     * weird node to be in expression! 
     * (or i didn't think of it - not out of the question :-) 
     */
    else
	fprintf(stderr, 
		"add_sub_nodes failure:	 nodetype %s at CFG node %d\n",
		get_AST_type_text(defAst), cfgId);
    
} /* end of add_sub_nodes() */






/*
 *  add_nodes
 *
 *  Take reference and add an SsaNode for it, if necessary.
 *  If a use, add to the subUses list of ssaId (if it exists)
 *  and add it to the ssaKids of cfgId.
 *
 *  If just a variable reference, and an SSA_USE, 
 *  just add the variable to the inVars of ssaId.
 */
static SsaNodeId add_nodes(CfgInstance cfg, CfgNodeId cfgId, SsaNodeId ssaParent, 
                           AST_INDEX astId, SsaType type)
{
    fst_index_t var;
    SsaNodeId ssaId = SSA_NIL;

    if (ssaParent != SSA_NIL)
	cfgId = SSA_node(cfg, ssaParent)->cfgParent;

    if (is_identifier(astId))
    {
	var = SsaGetSym(cfg, astId);

	if (FS_IS_MANIFEST_CONSTANT(cfg->symtab, var))
	{
	    /*
	     *  PARAMETER constant, build placeholder if needed
	     */
	    if (!((type == SSA_USE) && ssa_is_use(cfg, ssaParent)))
	    {
		ssaId = ssa_init_node(cfg, cfgId, ssaParent,
				      astId, type, SSA_NIL_NAME);
	    }
	}
	else
	{
	    ssaId = ssa_init_node(cfg, cfgId, ssaParent, astId, type, var);
	}
    }
    else if (is_subscript(astId))
    {
	var    = SsaGetSym(cfg, gen_SUBSCRIPT_get_name(astId));

	ssaId = ssa_init_node(cfg, cfgId, ssaParent, astId, type, var);

	add_list(cfg, cfgId, ssaId,
		 gen_SUBSCRIPT_get_rvalue_LIST(astId), SSA_SUBSCRIPT);
    }
    else if (is_substring(astId))
    {
	var    = SsaGetSym(cfg, gen_SUBSTRING_get_substring_name(astId));

	ssaId = ssa_init_node(cfg, cfgId, ssaParent, astId, type, var);

	add_substr_exprs(cfg, ssaId, astId);
    }
    else if (is_star(astId))
	/*
	 *  Dummy argument to read, write specification -- do nothing
	 */
	;
    else
    {
	/*
	 *  This keeps us from adding trivial nodes for SSA_USE, and
	 *  from adding multiple nodes for function invocations.
	 */
	if (!((type == SSA_USE) && ssa_is_use(cfg, ssaParent)))
	{
	    ssaId = ssa_init_node(cfg, cfgId, ssaParent,
				  astId, type, SSA_NIL_NAME);
	    ssaParent = ssaId;
	}
	
	add_sub_nodes(cfg, ssaParent, astId);
    }

    return ssaId;

} /* end of add_nodes() */




/* 
 * -- add_DUMMY_GLOBAL
 *
 *	  add ssa node for DUMMY_GLOBAL
 */
static void add_DUMMY_GLOBAL(CfgInstance cfg, CfgNodeId cfgId, SsaNodeId ssaParent)
{
    AST_INDEX astId;

    astId = CFG_node(cfg, cfgId)->astnode; 

    /*
     *  On entry the type is IP_IN; on invocation, GLOBAL_MOD
     */
    if (new_instance(astId) || is_entry(astId))
    {
	(void) ssa_init_node(cfg, cfgId, ssaParent,
			     AST_NIL, SSA_IP_IN, DUMMY_GLOBAL(cfg));
    }
    else
    {
	(void) ssa_init_node(cfg, cfgId, ssaParent,
			     AST_NIL, SSA_IP_MOD, DUMMY_GLOBAL(cfg));
	(void) ssa_init_node(cfg, cfgId, ssaParent,
			     AST_NIL, SSA_IP_REF, DUMMY_GLOBAL(cfg));
    }
} /* end of add_DUMMY_GLOBAL() */




/* 
 * -- add_alt_entry_guard
 *
 *	add SSA_GUARD_ALT_ENTRY at cfg->start for choice of entry
 *
 *	this should be only SSA node at cfg->start
 */
static void add_alt_entry_guard(CfgInstance cfg)
{
    if (CFG_node(cfg, cfg->start)->ssaKids == SSA_NIL)
	/*
	 *  Not added yet
	 */
	(void) ssa_init_node(cfg, cfg->start, SSA_NIL,
			     AST_NIL, SSA_GUARD_ALT_ENTRY, SSA_NIL_NAME);

} /* end of add_alt_entry_guard() */








/* 
 * -- add_list
 * 
 * 	add SSA nodes for a list of references
 * 	(e.g., in a READ or WRITE, subscript indices, etc.)
 */
static void add_list(CfgInstance cfg, CfgNodeId cfgId, SsaNodeId ssaParent, 
                     AST_INDEX astId, SsaType type)
{
    /*
     *  Assume that type is one of SSA_DEF, SSA_USE, SSA_SUBSCRIPT
     *
     *  Need to add a new parent node for list types in order to handle
     *  function invocations correctly (had a problem with trying to add
     *  both SSA_USE/SSA_SUBSCRIPT and SSA_CALL nodes for the same
     *  invocation ast node).
     */
    if (type != SSA_DEF)
    {
        ssaParent = ssa_init_node(cfg, cfgId, ssaParent, astId, type,
                                  SSA_NIL_NAME);
        type = SSA_USE;
    }

    for ( astId = list_last(astId); 
	  !is_null_node(astId); 
	  astId = list_prev(astId)) 
    {
	/*
	 *  add ssa nodes generated by implied_do elements
	 */
	if (is_implied_do(astId))
	{
	    do_implied_do(cfg, cfgId, ssaParent, astId, type);
	}
	else
	{
	    (void) add_nodes(cfg, cfgId, ssaParent, astId, type);
	}

    } /* end of for each ast node in the list */

} /* end of add_list() */



/*
 *  -- reference_arg
 *
 *	return variable name if the reference is something that can be 
 *	modified as an actual argument
 */
static fst_index_t reference_arg(CfgInstance cfg, AST_INDEX arg, CfgNodeId cfgId)
{
    AST_INDEX id;
    fst_index_t var;

    if (is_identifier(arg))
	id = arg;
    else if (is_subscript(arg))
	id = gen_SUBSCRIPT_get_name(arg);
    else if (is_substring(arg))
	id = gen_SUBSTRING_get_substring_name(arg);
    else
	return SSA_NIL_NAME;

    var = SsaGetSym(cfg, id);

    if (check_ivar(cfg, var, cfgId)) return SSA_NIL_NAME;

    if ((fst_GetFieldByIndex(cfg->symtab, var, SYMTAB_OBJECT_CLASS)
	 & OC_IS_DATA)
	&&
	(fst_GetFieldByIndex(cfg->symtab, var, SYMTAB_STORAGE_CLASS)
	 != SC_CONSTANT))
    {
	return var;
    }
    else
	return SSA_NIL_NAME;
}


/*
 * -- add_CALL
 *
 *	add SSA nodes for actual args to invocation
 *
 *	SSA_ACTUAL is for call-by-value (cannot be modified)
 *		-- need to be careful when simplifying SSA_ACTUAL
 *		   that we don't replace it with a reference
 *		-- ambiguity:   some SSA_ACTUALs are call-by-value
 *				others are call-by-ref but not modified
 *
 *	SSA_IP_MOD is for call-by-reference (can be modified)
 *		mods simulated as (use & kill)
 *
 *	also add node for DUMMY_GLOBAL
 */
static void add_CALL(CfgInstance cfg, CfgNodeId cfgId, SsaNodeId ssaParent, 
                     AST_INDEX siteAst)
{
    SsaNodeId callId;
    AST_INDEX argList, arg;
    Boolean haveAltReturns = false;
    int count;

    /*
     *  Want to have only one SSA node per AST_INDEX...
     */
    if ((ssaParent != SSA_NIL) &&
	(SSA_node(cfg, ssaParent)->refAst == siteAst) &&
	(!ssa_is_loop_type(cfg, ssaParent)))
    {
	if (ssa_is_def(cfg, ssaParent))
	    die_with_message("add_CALL: parent of call is def\n");

	callId = ssaParent;
	SSA_node(cfg, callId)->type = SSA_CALL;
    }
    else
    {
	/*
	 *  Avoid multiple SSA nodes per AST_INDEX
	 */
	if ((ssaParent != SSA_NIL) &&
	    ssa_is_loop_type(cfg, ssaParent) &&
	    (SSA_node(cfg, ssaParent)->refAst == siteAst))
	{
	    ssa_node_zap_map(cfg, siteAst);
	    SSA_node(cfg, ssaParent)->refAst = AST_NIL;
	}

	callId = ssa_init_node(cfg, cfgId, ssaParent,
			       siteAst, SSA_CALL, SSA_NIL_NAME);
    }

    if (!idfaIsPure(cfg, siteAst))
    {
	add_DUMMY_GLOBAL(cfg, cfgId, callId);
    }

    argList = gen_INVOCATION_get_actual_arg_LIST(siteAst);

    for (arg = list_last(argList); 
	 !is_null_node(arg);
	 arg = list_prev(arg) ) 
    {
	if (is_return_label(arg))
	{
	    haveAltReturns = true;
	}
	else
	{
	    fst_index_t ref_arg = reference_arg(cfg, arg, cfgId);

	    if (ref_arg != SSA_NIL_NAME)
	    {
		/*
		 *  Make sure there's only one SsaNode mapped with each 
		 *  AST_INDEX.  If need both SSA_IP_MOD and SSA_IP_REF, 
		 *  give the AST_INDEX to the MOD and put it second in the
		 *  ssaKids list (i.e., added first) so that the REF isn't
		 *  hooked to the MOD definition. Also makes sure subordinate 
		 *  edges are associated only with one of the MOD or REF.
		 */
                if (idfaArgIsMod(cfg, siteAst, arg))
                {
                    (void) add_nodes(cfg, cfgId, callId, arg, SSA_IP_MOD);

                    if (idfaArgIsRef(cfg, siteAst, arg))
                    {
                        (void) ssa_init_node(cfg, cfgId, callId,
                                             AST_NIL, SSA_IP_REF, ref_arg);
                    }
                }
                else
                {
                    if (idfaArgIsRef(cfg, siteAst, arg))
                    {
                        (void) add_nodes(cfg, cfgId, callId, arg, SSA_IP_REF);
                    }
                }
	    }
	    else
	    {
		if (idfaArgIsRef(cfg, siteAst, arg))
		{
		    /*
		     *  SSA_ACTUAL and SSA_IP_REF are really the same thing,
		     *  the former has to be protected against being converted
		     *  to call-by-ref.
		     */
		    (void) add_nodes(cfg, cfgId, callId, arg, SSA_ACTUAL);
		}
	    }
	}
    } /* end of foreach actual argument */

    /*
     *  add GUARD_ALT_RETURN here so that it is first in the cfgParent's
     *  ssaKids list, and in the CALL's subUses
     *
     *  assume that typechecking is correct and we don't have alternate
     *  return labels for a function invocation, just for call sites
     */
    if (haveAltReturns)
	(void) ssa_init_node(cfg, cfgId, callId, AST_NIL,
			     SSA_GUARD_ALT_RETURN, SSA_NIL_NAME);

} /* end of add_CALL() */



/****************************************************************************
 **		      PRIVATE FUNCTIONS: get_*()			   **
 ****************************************************************************/

/* 
 * -- add_substr_exprs
 *
 *	       add exps for substring begin/end expressions.  
 *	       return last item in list generated 
 */
static void add_substr_exprs(CfgInstance cfg, SsaNodeId ssaParent, AST_INDEX astId)
{
    AST_INDEX val;

    ssaParent = ssa_init_node(cfg, CFG_NIL, ssaParent, astId, SSA_SUBSCRIPT,
			      SSA_NIL_NAME);

    val = gen_SUBSTRING_get_rvalue1(astId);

    (void) add_nodes(cfg, SSA_node(cfg, ssaParent)->cfgParent, ssaParent,
		     val, SSA_USE);

    val = gen_SUBSTRING_get_rvalue2(astId);

    (void) add_nodes(cfg, SSA_node(cfg, ssaParent)->cfgParent, ssaParent,
		     val, SSA_USE);

} /* end of add_substr_exprs() */






/*
 * -- get_operand
 *
 *	    return ast node corresponding to son (LEFT, RIGHT) of operator node
 */
static AST_INDEX get_operand(AST_INDEX astId, int son)
{
    NODE_TYPE astType;

    if( (son != LEFT) && (son != RIGHT) )
	die_with_message("get_operand: bogus son indicator\n");

    astType = ast_get_node_type(astId);
    
    switch(astType)
    {
      case GEN_UNARY_MINUS:	/* is_unary_minus() */
	return (gen_UNARY_MINUS_get_rvalue(astId));

      case GEN_UNARY_NOT:	/* is_unary_not() */
	return (gen_UNARY_NOT_get_rvalue(astId));

      case GEN_BINARY_EXPONENT: /* is_binary_exponent() */
	if (son == LEFT)
	    return (gen_BINARY_EXPONENT_get_rvalue1(astId));
	else
	    return (gen_BINARY_EXPONENT_get_rvalue2(astId));

      case GEN_BINARY_TIMES:	/* is_binary_times() */
	if (son == LEFT)
	    return (gen_BINARY_TIMES_get_rvalue1(astId));
	else
	    return (gen_BINARY_TIMES_get_rvalue2(astId));

      case GEN_BINARY_DIVIDE:	/* is_binary_divide() */
	if (son == LEFT)
	    return (gen_BINARY_DIVIDE_get_rvalue1(astId));
	else
	    return (gen_BINARY_DIVIDE_get_rvalue2(astId));

      case GEN_BINARY_PLUS:	/* is_binary_plus() */
	if (son == LEFT)
	    return (gen_BINARY_PLUS_get_rvalue1(astId));
	else
	    return (gen_BINARY_PLUS_get_rvalue2(astId));

      case GEN_BINARY_MINUS:	/* is_binary_minus() */
	if (son == LEFT)
	    return (gen_BINARY_MINUS_get_rvalue1(astId));
	else
	    return (gen_BINARY_MINUS_get_rvalue2(astId));

      case GEN_BINARY_CONCAT:	/* is_binary_concat() */
	if (son == LEFT)
	    return (gen_BINARY_CONCAT_get_rvalue1(astId));
	else
	    return (gen_BINARY_CONCAT_get_rvalue2(astId));

      case GEN_BINARY_AND:	/* is_binary_and() */
	if (son == LEFT)
	    return (gen_BINARY_AND_get_rvalue1(astId));
	else
	    return (gen_BINARY_AND_get_rvalue2(astId));

      case GEN_BINARY_OR:	/* is_binary_or() */
	if (son == LEFT)
	    return (gen_BINARY_OR_get_rvalue1(astId));
	else
	    return (gen_BINARY_OR_get_rvalue2(astId));

      case GEN_BINARY_EQ:	/* is_binary_eq() */
	if (son == LEFT)
	    return (gen_BINARY_EQ_get_rvalue1(astId));
	else
	    return (gen_BINARY_EQ_get_rvalue2(astId));

      case GEN_BINARY_NE:	/* is_binary_ne() */
	if (son == LEFT)
	    return (gen_BINARY_NE_get_rvalue1(astId));
	else
	    return (gen_BINARY_NE_get_rvalue2(astId));

      case GEN_BINARY_GE:	/* is_binary_ge() */
	if (son == LEFT)
	    return (gen_BINARY_GE_get_rvalue1(astId));
	else
	    return (gen_BINARY_GE_get_rvalue2(astId));

      case GEN_BINARY_GT:	/* is_binary_gt() */
	if (son == LEFT)
	    return (gen_BINARY_GT_get_rvalue1(astId));
	else
	    return (gen_BINARY_GT_get_rvalue2(astId));

      case GEN_BINARY_LE:	/* is_binary_le() */
	if (son == LEFT)
	    return (gen_BINARY_LE_get_rvalue1(astId));
	else
	    return (gen_BINARY_LE_get_rvalue2(astId));

      case GEN_BINARY_LT:	/* is_binary_lt() */
	if (son == LEFT)
	    return (gen_BINARY_LT_get_rvalue1(astId));
	else
	    return (gen_BINARY_LT_get_rvalue2(astId));

      case GEN_BINARY_EQV:	/* is_binary_eqv() */
	 if (son == LEFT)
	    return (gen_BINARY_EQV_get_rvalue1(astId));
	else
	    return (gen_BINARY_EQV_get_rvalue2(astId));

      case GEN_BINARY_NEQV:	/* is_binary_neqv() */
	if (son == LEFT)
	    return (gen_BINARY_NEQV_get_rvalue1(astId));
	else
	    return (gen_BINARY_NEQV_get_rvalue2(astId));

      default:
	fprintf(stderr, "Error in get_operand(): Unknown AST type\n");
	exit(-1);
    }

} /* end of get_operand() */





/****************************************************************************
 **	       PRIVATE FUNCTIONS: MISCELLANEOUS HELPERS			   **
 ****************************************************************************/


/*
 *  Return true if we delete the node,
 *  false if there is some reason to keep it.
 */
static Boolean kill_if_killable(CfgInstance cfg, SsaNodeId ssaId)
{
    SsaType type = SSA_node(cfg, ssaId)->type;

    /*
     *  Delete some use nodes and array nodes which don't use any variables...
     *  ...also don't want to delete RHS of assignment
     */
    if (((type == SSA_USE)||ssa_ignored(cfg, ssaId))
	&& 
	!(SSA_node(cfg, ssaId)->fanIn)
	&&
	(SSA_node(cfg, SSA_node(cfg, ssaId)->ssaParent)->type != SSA_DEF)
	)
    {
	ssa_node_free(cfg, ssaId);
	return true;
    }
    else
	return false;

} /* end kill_if_killable */







