/* $Id: cfgval_build.C,v 1.14 1997/03/11 14:35:37 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <assert.h>

#include <libs/moduleAnalysis/cfgValNum/cfgval.i>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/moduleAnalysis/ssa/idfa.h>
#include <libs/moduleAnalysis/valNum/val_pass.h>
#include <libs/ipAnalysis/problems/symbolic/SymTrans.h>
#include <libs/ipAnalysis/problems/symbolic/SymConstraints.h>

STATIC(ValNumber, build_list_val, (CfgInstance cfg, SsaNodeId node));
STATIC(ValNumber, build_array_val, (CfgInstance cfg, SsaNodeId node));
STATIC(void, unbuild_iterative, (CfgInstance cfg, SsaNodeId node, int level));
STATIC(Boolean, loop_var_search, (CfgInstance cfg, SsaNodeId sn, SsaNodeId phi));
STATIC(void, const_stats, (CfgInstance cfg));

static int get_call_nodenum(CfgInstance cfg, SsaNodeId callSN)
{
    return ft_NodeToNumber(cfg->cfgGlobals->ft, SSA_node(cfg, callSN)->refAst);
}
static char *get_entry_name(CfgInstance cfg, CfgNodeId entryCN)
{
    AST_INDEX idnode = get_name_in_entry(CFG_node(cfg, entryCN)->astnode);
    return SsaSymText(cfg, SsaGetSym(cfg, idnode));
}
//
//  get_var_info
//	Get data used in tagging references to interprocedural variables
//
static void get_var_info(CfgInstance cfg, 
			 fst_index_t index,
			 fst_index_t &name,
			 int &offset, 
			 int &length,
			 ExpType &expType)
{
    name = (fst_index_t) fst_GetFieldByIndex(cfg->symtab, 
					     index, SYMTAB_PARENT);
    if (name == SSA_NIL_NAME) name = index;
    offset = fst_GetFieldByIndex(cfg->symtab, index, SYMTAB_EQ_OFFSET);
    length = fst_GetFieldByIndex(cfg->symtab, index, SYMTAB_SIZE);
    expType = fst_GetFieldByIndex(cfg->symtab, index, SYMTAB_TYPE);
}


//
//  cfgval_build_table() 
//	This function goes through all the nodes 
//	and builds the value graph from the expressions in the nodes
//
void cfgval_build_table(CfgInstance cfg)
{
    SsaNodeId	sn;

    for (sn = ssa_get_first_node(cfg);
	 sn != SSA_NIL;
	 sn = ssa_get_next_node(cfg, sn))
    {
	SSA_node(cfg,sn)->value= cfgval_build(cfg,sn);
    }
    /* const_stats(cfg); */
}

//
//  build_subvals
//	Used by cfgval_build_ip to build values for interprocedurally
//	interesting stuff -- crudely, anything hanging off a CALL but
//	not stuck inside of a SUBSCRIPT.
//
static void build_subvals(CfgInstance cfg, SsaNodeId sn)
{
    if (SSA_node(cfg,sn)->type == SSA_SUBSCRIPT) return;

    for (SsaNodeId kid = SSA_node(cfg,sn)->subUses;
	 kid != SSA_NIL;
	 kid = SSA_node(cfg,kid)->nextSsaSib)
    {
	(void) build_subvals(cfg, kid);
    }

    if (SSA_node(cfg,sn)->type == SSA_CALL)
    {
	for (SsaNodeId kid = SSA_use(cfg,sn)->subDefs;
	     kid != SSA_NIL;
	     kid = SSA_node(cfg,kid)->nextSsaSib)
	{
	    (void) build_subvals(cfg, kid);
	}
    }
    else
    {
	(void) cfgval_build(cfg, sn);
    }
}

//
//  cfgval_build_ip
//	only build value numbers that might be interesting to interprocedural
//	analysis -- tests, subscripts, passed/return values
//
void cfgval_build_ip(CfgInstance cfg)
{
    SsaNodeId	sn;

    for (sn = ssa_get_first_node(cfg);
	 sn != SSA_NIL;
	 sn = ssa_get_next_node(cfg, sn))
    {
	SsaType type = SSA_node(cfg, sn)->type;

	if (ssa_is_guard(cfg, sn) ||
	    // (type == SSA_SUBSCRIPT) ||
	    (type == SSA_CALL))
	{
	    build_subvals(cfg, sn);
	}
	else if (SSA_node(cfg,sn)->cfgParent == cfg->end)
	{
	    for (SsaEdgeId se = SSA_node(cfg,sn)->defsIn;
		 se != SSA_NIL;
		 se = SSA_edge(cfg,se)->inNext)
	    {
		(void) cfgval_chase_edge(cfg, se);
	    }
	}
    }
}


Boolean val_get_ip_const(CfgInstance cfg, AST_INDEX site,
			 fst_index_t name,	/* invocation or entry node */
			 int *constVal)
{
    Boolean isConstant;

    if(!SSA_ipInfo(cfg))
	return false;

    isConstant = idfaNameIsConst(cfg, site, name);
    if (isConstant)
	(*constVal) = idfaNameGetConst(cfg, site, name);

    return isConstant;
}


//
//  cfgval_build
//	Build a value number for the SSA node, chasing SSA edges and 
//	building other values as needed.  Cache the result in the "value"
//	field of the SSA node.
//
ValNumber cfgval_build(CfgInstance cfg, SsaNodeId ssaId)
{
    ValNumber 	retVal;
    fst_index_t	index;
    AST_INDEX	astHolder;
    SsaNodeId	ssaHolder;
    SsaNodeId	ssaParent;
    CfgNodeId	stmt;
    int		constVal;
    fst_index_t name;
    int offset, length;
    ExpType	expType;
    
    if (SSA_node(cfg,ssaId)->value != VAL_NIL)
	return SSA_node(cfg,ssaId)->value;

    index = SSA_node(cfg, ssaId)->name;

    if (ssa_is_subarray_type(cfg, ssaId))
    {
	if (SSA_doArrays(cfg)) retVal = build_array_val(cfg, ssaId);
	else		       retVal = VAL_BOTTOM;
    }
    else if ((index != SSA_NIL_NAME) && 
	     (FS_IS_ARRAY(cfg->symtab, index) ||
	      !ssa_var_covers_eq(cfg, index)) &&
	     !SSA_doArrays(cfg))
    {
        retVal = VAL_BOTTOM;
    }
    else switch(SSA_node(cfg,ssaId)->type)
    {
	//  See SSA_IP_REF below with other use types...
	//

      case SSA_IP_MOD:	/* potential indirect use/mod via call site */
			/* or function invocation                   */

	ssaParent = SSA_node(cfg, ssaId)->ssaParent;	// the call site

	if (index == DUMMY_GLOBAL(cfg))
	{
	    ValNumber valIn = cfgval_chase_edge(cfg,
						SSA_node(cfg, ssaId)->defsIn);
	    retVal = val_lookup_return(*V(cfg), TYPE_UNKNOWN,
				       tarj_level(cfg_get_intervals(cfg),
						  SSA_node(cfg,
							   ssaId)->cfgParent),
				       get_call_nodenum(cfg, 
							SSA_node(cfg,
							    ssaId)->ssaParent),
				       SsaSymText(cfg, DUMMY_GLOBAL(cfg)),
				       0, 0, valIn);
	}
	else if (val_get_ip_const(cfg, SSA_node(cfg, ssaParent)->refAst,
				  index, &constVal))
	{
	    /*
	     *  What about the value passed in?  No place to put it?
	     *  Things really interested in passed value can get it from
	     *  the SSA node.
	     */
	    expType = fst_GetFieldByIndex(cfg->symtab, index, SYMTAB_TYPE);
	    retVal = val_lookup_const(*V(cfg), expType, constVal);
	}
	else
	{
	    get_var_info(cfg, index, name, offset, length, expType);

	    retVal = VAL_NIL;

	    if (SSA_ipInfo(cfg) && CFGVAL_useIpVals(cfg))
	    {
		if (is_null_node(SSA_node(cfg,ssaId)->refAst))
		{
#ifdef ELIM_LINK_HACK
		    //  global or static variable -- look up by name and offset
		    //
		    retVal = trans_ret_cfgval(SSA_ipInfo(cfg), V(cfg),
					      cfg_get_inst_name(cfg),
					      get_call_nodenum(cfg, 
						  SSA_node(cfg,
							   ssaId)->ssaParent),
					      SsaSymText(cfg, name),
					      offset, length, cfg);
#else
		    assert(0);
#endif
		}
		else
		{
#ifdef ELIM_LINK_HACK
		    //  reference actual -- look up by "" and parm #
		    //
		    retVal = trans_ret_cfgval(SSA_ipInfo(cfg), V(cfg),
					      cfg_get_inst_name(cfg),
					      get_call_nodenum(cfg, 
						  SSA_node(cfg,
							   ssaId)->ssaParent),
					      "",
					      list_element(SSA_node(cfg,
							     ssaId)->refAst),
					      length, cfg);
#else
		    assert(0);
#endif
		}
	    }

	    if (retVal == VAL_NIL)
	    {
		ValNumber valIn = cfgval_chase_edge(cfg,
						    SSA_node(cfg, 
							     ssaId)->defsIn);
		retVal = val_lookup_return(*V(cfg),
					   expType,
					   tarj_level(cfg_get_intervals(cfg),
						      SSA_node(cfg,
							  ssaId)->cfgParent),
					   get_call_nodenum(cfg, 
							    SSA_node(cfg,
							     ssaId)->ssaParent),
					   SsaSymText(cfg, name),
					   offset, length, valIn);
	    }
	}
	break;

      case SSA_IP_IN:	/* value on entry of formal, 
			   common block var, local static */

	stmt = SSA_node(cfg, ssaId)->cfgParent;

	retVal = cfgval_lookup_entry(cfg, get_entry_name(cfg, stmt), index);

	break;

      case SSA_INDUCTIVE:
      { 
	  ValNumber align, lo, hi, step, rngVal;
	  Boolean   flipped = false;
	  align = lo = hi = step = rngVal = VAL_NIL;	
	  ssaHolder = SSA_node(cfg,ssaId)->subUses;

	  expType = ast_get_real_type(SSA_node(cfg,ssaId)->refAst);	
	  for (ssaHolder = SSA_node(cfg,ssaId)->subUses;
	       ssaHolder != SSA_NIL;
	       ssaHolder = SSA_node(cfg,ssaHolder)->nextSsaSib)
	  {
	      ValNumber tmpVal;
	      tmpVal = cfgval_build(cfg, ssaHolder);
	      switch(SSA_node(cfg,ssaHolder)->type)
	      {
		case SSA_LOOP_INIT:
		  align = lo = tmpVal;
		  break;
		case SSA_LOOP_BOUND:
		  hi = tmpVal;
		  break;
		case SSA_LOOP_STEP:
		  step = tmpVal;
		  break;
		default:
		  fprintf(stderr,"Big error in SSA_INDUCTIVE\n");
		  break;
	      }
	  }
	  if (lo == hi)
	  {
	      retVal = lo;
	      break;
	  }
	  if (step == VAL_NIL)
	  {
	      step  = VAL_ONE;
	      align = VAL_ZERO;
	  }

	  rngVal = val_lookup_range(*V(cfg), lo, hi, align, step);

	  //  Ultimately change this to just return if rngVal is VAL_TOP
	  //  or constant.  But right now we want to look for bugs.
	  //
	  if (!val_is_value(rngVal))
	      die_with_message("cfgval_build: bogus range value\n");

	  if (ve_lo(VE(cfg,rngVal)) != lo) /* flipped */
	      flipped = true;

	  retVal = val_lookup_ivar(*V(cfg),
				   tarj_level(cfg_get_intervals(cfg),
					      SSA_node(cfg, ssaId)->cfgParent),
				   rngVal, flipped);
	  break;
      }

      case SSA_ETA:
      {
	  TarjTree loops = cfg_get_intervals(cfg);
	  CfgNodeId exitDest, exitBr;
	  CfgEdgeId exitEdge;

	  /*
	   *  Need to build a chain of VAL_ETA nodes, one for each loop exited.
	   *  If the gamma for the exit test has not been built, create a 
	   *  variant value for the test.
	   */
	  retVal = cfgval_chase_edge(cfg, SSA_node(cfg, ssaId)->defsIn);

	  exitDest = SSA_node(cfg, ssaId)->cfgParent;
	  exitEdge = CFG_node(cfg, exitDest)->ins;
	  exitBr   = CFG_edge(cfg, exitEdge)->src;
	  /*
	   *  Find the header of the loop exited
	   *  (which may or may not be the same as the exit branch)
	   *  -- irreducible loops are handled pessimistically.
	   */
	  if (tarj_type(loops, exitBr) == TARJ_INTERVAL)
	  {
	      stmt = exitBr;
	  }
	  else if (tarj_type(loops, exitBr) == TARJ_ACYCLIC)
	  {
	      stmt = tarj_outer(loops, exitBr);
	  }
	  else
	  {
	      retVal = VAL_BOTTOM;
	      break;
	  }

	  for (;
	       (stmt != CFG_NIL) &&
	       (tarj_level(loops, stmt) > tarj_level(loops, exitDest));
	       stmt = tarj_outer(loops, stmt))
	  {
	      ValNumber testVal;

	      if ((!SSA_loopPredicates(cfg)) ||
		  (SSA_loopPredicates(cfg)[stmt] == SSA_NIL))
	      {
		  retVal = VAL_BOTTOM;
		  break;
	      }
	      testVal = cfgval_build(cfg, SSA_loopPredicates(cfg)[stmt]);

	      retVal = val_lookup_eta(*V(cfg), testVal, retVal);
	  }
	  break;
      }

      case SSA_DEF:
      {
	  Boolean flag = false;	

	  //  Why the following?  Ensures that SSA DEF nodes without 
	  //  AST get BOTTOM (or VARIANT) values.  In fact, should such
	  //  nodes exist at all?
	  //
	  if (SSA_node(cfg,ssaId)->refAst == ast_null_node)
	  {
	      ssaHolder = SSA_NIL;
	  }
	  else
	  {
	      ssaHolder = SSA_node(cfg,ssaId)->subUses;
	  }

	  for (;
	       ssaHolder != SSA_NIL;
	       ssaHolder = SSA_node(cfg,ssaHolder)->nextSsaSib)
	  {
	      //
	      //  There may be subuses for LHS -- array subscripts (??) --
	      //  and for RHS -- the defined value (RESULT or CALL).
	      //
	      //  The array subscripts case should be split off long before,
	      //  so we should only be worried about RHS here.
	      //
	      if (SSA_node(cfg,ssaHolder)->type == SSA_USE ||
		  SSA_node(cfg,ssaHolder)->type == SSA_CALL)
	      {
		  retVal = cfgval_build(cfg,ssaHolder);
		  flag = true;
		  break;
	      }
	  }

	  if (flag)
	      break;

	  retVal = VAL_BOTTOM;
	  break;
      }

      case SSA_PHI:
      {
	  CfgNodeId loop = SSA_node(cfg,ssaId)->cfgParent;
	  TarjType loopType = tarj_type(cfg_get_intervals(cfg), loop);

	  if ((loopType == TARJ_INTERVAL) || (loopType == TARJ_IRREDUCIBLE))
	  {
	      int level = tarj_level(cfg_get_intervals(cfg), loop);
	      SsaNodeId iterNode;
	      ValNumber initVal, iterVal;

	      //  Loop merge -- guaranteed by preprocessing of the 
	      //  CFG to have exactly two inputs: initial and iterative.
	      //
	      SsaEdgeId edge;

	      for (edge = SSA_node(cfg,ssaId)->defsIn;
		   edge != SSA_NIL;
		   edge = SSA_edge(cfg,edge)->inNext)
	      {
		  SsaNodeId source = SSA_edge(cfg,edge)->source;

		  if (tarj_contains(cfg_get_intervals(cfg), loop,
				    SSA_node(cfg,source)->cfgParent))
		  {
		      iterNode = source;
		  }
		  else
		  {
		      //  Need to handle case where source and sink
		      //  variables of the edge are different.
		      //
		      initVal = cfgval_chase_edge(cfg, edge);
		  }
	      }
	      if ((loopType == TARJ_IRREDUCIBLE) || 
		  (index == DUMMY_GLOBAL(cfg)) ||
		  loop_var_search(cfg, iterNode, ssaId))
	      {
		  //  Be pessimistic.
		  //
		  SSA_node(cfg, ssaId)->value = retVal = 
		      val_lookup_var_mu(*V(cfg), ssaId, initVal, level);

		  //  Build iterative value, even though this is self-
		  //  dependent in a way that keeps us from hashing on it.
		  //
		  iterVal = cfgval_build(cfg, iterNode);
		  ve_varIter(VE(cfg, retVal)) = iterVal;

		  ValEntry *iter = &(VE(cfg, iterVal));
		  if ((ve_type(*iter) != VAL_OP) ||
		      (ve_opType(*iter) != VAL_OP_PLUS) ||
		      !val_is_const(V(cfg), ve_left(*iter)) ||
		      (ve_right(*iter) != retVal))
		  {
		      break;
		  }

		  //  If we get here, then (perhaps because of ip info)
		  //  we have what looks like an inductive value
		  //
		  retVal = cfgval_lookup_ok_mu(cfg, ssaId, initVal,
					       val_binary(V(cfg), VAL_OP_PLUS,
							  ve_left(*iter),
						      val_lookup_mu_ph(*V(cfg),
								       level)));

		  //  Reset values to VAL_NIL for those nodes encountered
		  //  in building the increment function
		  //
		  unbuild_iterative(cfg, iterNode, ssaId);

		  break;
	      }
	      else // TARJ_INTERVAL and nice iterative value
	      {
		  //
		  //  Placeholder for increment computation
		  //
		  SSA_node(cfg, ssaId)->value = val_lookup_mu_ph(*V(cfg),
								 level);

		  //  If ssaId is self-dependent, but in a nice way,
		  //  iterVal will involve the mu placeholder.
		  //  Otherwise, iterVal will be invariant, because we 
		  //  chickened out with loop_var_search().
		  //
		  iterVal = cfgval_build(cfg, iterNode);

		  retVal = cfgval_lookup_ok_mu(cfg, ssaId, 
					       initVal, iterVal);

		  //  Reset values to VAL_NIL for those nodes encountered
		  //  in building the increment function
		  //
		  unbuild_iterative(cfg, iterNode, ssaId);
		  break;
	      }
	  }
      }
	/*
	 *  else fall through to similar handling for SSA_GAMMA
	 */
      case SSA_GAMMA:
      {
	  int i,nArgs;
	  ValNumber *args;
	  ValNumber testVal;
	  SsaEdgeId edge;
	  Boolean reducible = true;	
	  int expType;

	  nArgs = SSA_node(cfg,ssaId)->fanIn;

	  if (SSA_node(cfg,ssaId)->type == SSA_PHI)
	  {
	      args = (ValNumber *) get_mem(nArgs * sizeof(ValNumber),
					   "cfgval_build.c: phi args array");
	      for (i = 0, edge = SSA_node(cfg,ssaId)->defsIn;
		   i < nArgs; 
		   i++, edge = SSA_edge(cfg,edge)->inNext)
	      {
		  args[i] = cfgval_chase_edge(cfg,edge);
	      }
	  }
	  else /* gamma */
	  {
	      int label_offset;
	      SsaNodeId test;
	      CfgNodeId testStmt;
	      CdBranchType stype;

	      test = SSA_node(cfg,ssaId)->subUses;
	      testStmt = SSA_node(cfg, test)->cfgParent;
	      nArgs = CFG_node(cfg, testStmt)->fanOut;
	      stype = cfg_branch_type(cfg, testStmt);
	      testVal = cfgval_build(cfg,test);

	      switch (stype)
	      {
		case CD_LOGICAL_IF:
		case CD_DO_LOOP:
		case CD_COMPUTED_GOTO:
		case CD_CASE:
		case CD_ALTERNATE_RETURN:
		  /* labels start with 0 */
		  label_offset = 0;
		  break;

		case CD_ALTERNATE_ENTRY:
		case CD_ARITHMETIC_IF:
		case CD_ASSIGNED_GOTO:
		case CD_OPEN_ASSIGNED_GOTO:
		  /* labels start with 1 */
		  label_offset = -1;
		  break;

		case CD_I_O:
		  /* labels are [-1,0,1] or [0,1] or [-1,0] or [0] */
		  label_offset = 1;
		  nArgs = 3;
		  break;

		default:
		  die_with_message("cfgval_build: bogus branch type for gamma\n");
	      }
	      if ((stype != CD_LOGICAL_IF) && (stype != CD_DO_LOOP))
		  testVal = val_lookup_test(*V(cfg), testVal, stype, test);

	      args = (ValNumber *) get_mem(nArgs * sizeof(ValNumber),
					   "cfgval_build.c: gamma args array");
	      int_set(args, nArgs, VAL_TOP);

	      for (edge = SSA_node(cfg,ssaId)->defsIn;
		   edge != SSA_NIL;
		   edge = SSA_edge(cfg,edge)->inNext)
	      {
		  SsaNodeId source = SSA_edge(cfg,edge)->source;

		  if (source != SSA_NIL)
		  {
		      ValNumber choice;

		      if (source == SSA_LOOP_TRUE)
			  choice = VAL_TRUE;
		      else if (source == SSA_LOOP_FALSE)
			  choice = VAL_FALSE;
		      else
			  choice = cfgval_chase_edge(cfg,edge);

		      args[SSA_edge(cfg, edge)->pathEdge + label_offset] =
			  choice;
		  }
	      }
	      if ((stype != CD_DO_LOOP) &&
		  (ve_type(VE(cfg,testVal)) == VAL_CONST))
	      {
		  retVal = args[ve_const(VE(cfg,testVal)) + label_offset];
		  free_mem((void *)args);
		  break;
	      }
	  }
		
	  for (i=1; i< nArgs; i++)
	  {
	      if (args[i] != args[0])
	      {
		  reducible = false;
		  break;	
	      }
	  }

	  if (reducible)
	  {
	      retVal = args[0];
	      free_mem((void *)args);
	      break;
	  }

	  /*
	   *  Should make sure the argument order conforms to the assumed order?
	   */
	  i = 0;
	  while((i < nArgs) &&
		((args[i] == VAL_TOP) || (args[i]==VAL_BOTTOM)))
	      i++;

	  fst_index_t vId = SSA_node(cfg, ssaId)->name;

	  if ((vId != SSA_NIL_NAME) && (vId != DUMMY_GLOBAL(cfg)))
	  {
	      expType = fst_GetFieldByIndex(cfg->symtab, vId, SYMTAB_TYPE);
	  }
	  else
	  {
	      if (i == nArgs) /* all args = TOP or BOTTOM */
		  expType = TYPE_UNKNOWN;
	      else
		  expType = ve_expType(VE(cfg,args[i]));
	  }

	  if (SSA_node(cfg,ssaId)->type == SSA_PHI)
	      retVal = val_lookup_phi(*V(cfg), expType,
				      SSA_node(cfg,ssaId)->cfgParent,
				      nArgs, args);
	  else /* gamma */
	      retVal = val_lookup_gamma(*V(cfg), expType,
					testVal, nArgs, args);

	  free_mem((void *)args);

	  break;
      }

      case SSA_LOOP_STEP:
	if (SSA_node(cfg,ssaId)->refAst == ast_null_node &&
	    SSA_node(cfg,ssaId)->defsIn == SSA_NIL)
	{
	    retVal = VAL_ONE;
	    break;
	}
	/* else fall through */
      case SSA_IP_REF:
      case SSA_GUARD_LOGICAL:
      case SSA_GUARD_INTEGER:
      case SSA_ACTUAL:
      case SSA_ALT_RETURN:
      case SSA_LOOP_INIT:
      case SSA_LOOP_BOUND:
      case SSA_USE:
	if (SSA_node(cfg,ssaId)->refAst != ast_null_node)
	    retVal = cfgval_evaluate(cfg, SSA_node(cfg,ssaId)->refAst);
	else if (SSA_node(cfg,ssaId)->defsIn != SSA_NIL)
	{
	    SsaEdgeId edge = SSA_node(cfg, ssaId)->defsIn;
	    if (SSA_edge(cfg, edge)->inNext == SSA_NIL)
		retVal = cfgval_chase_edge(cfg, edge);
	    else
	    {
		/*
		 *  aggregate use
		 */
		for (; edge != SSA_NIL; edge = SSA_edge(cfg, edge)->inNext)
		    (void) cfgval_chase_edge(cfg, edge);

		retVal = VAL_BOTTOM;
	    }
	}
	else if (FS_IS_ARRAY(cfg->symtab, index))
		retVal = VAL_BOTTOM;
	else
	{
	    retVal = VAL_BOTTOM;
	}
	break;

      case SSA_GUARD_INDUCTIVE:
      {
	  /*
	   *  Want the trip count of the loop for the guard.
	   */
	  astHolder = SSA_node(cfg,ssaId)->refAst;	/* the INDUCTIVE */
	  astHolder = gen_INDUCTIVE_get_name(astHolder);/* the ivar */
	  retVal = cfgval_get_val(cfg,astHolder);	/* the VAL_IVAR */

	  ValType vt = ve_type(VE(cfg, retVal));

	  if (vt == VAL_IVAR)
	      retVal = ve_bounds(VE(cfg, retVal));	/* the VAL_RANGE */
	  else
	  {
	      if (vt != VAL_CONST)
		  die_with_message("cfgval_build: neither range nor const%s\n",
				   " for inductive.");
	  }

	  if (ve_type(VE(cfg, retVal)) == VAL_RANGE)
	  {
	      /*
	       *  Want to see if can evaluate trip count, but
	       *  punt for now.  While the ve_bounds info precisely
	       *  represents the index values, it is more convenient
	       *  to get val#s for the SSA_LOOP_* nodes and evaluate 
	       *  max(int((hi-lo+step)/step),0)
	       */
	      retVal = VAL_BOTTOM;
	  }
	  else if (ve_type(VE(cfg, retVal)) == VAL_CONST)
	  {
	      retVal = VAL_ONE;
	  }
	  else
	  {
	      retVal = VAL_BOTTOM;
	  }
	  
	  break;
      }

      case SSA_GUARD_ALT_RETURN:
      case SSA_GUARD_ALT_ENTRY:
      case SSA_GUARD_CASE:
	retVal = VAL_BOTTOM;
	break;

      case SSA_SUBSCRIPT:	/* parent of individual subscripts */
	retVal = SSA_doArrays(cfg) ? build_list_val(cfg, ssaId) : VAL_BOTTOM;
	break;

      case SSA_CALL:		/* parent of ip references */
	retVal = VAL_BOTTOM;
	break;

      default:
	fprintf(stderr,"big problems in cfgval_build: %d %d\n",
		SSA_node(cfg,ssaId)->type,ssaId);
	break;
    }	

    if (retVal == VAL_BOTTOM) 
	retVal = cfgval_lookup_variant(cfg, ssaId);

    SSA_node(cfg,ssaId)->value = retVal;
    return retVal;
}
	
/*
 *  Get the value number for the source of an edge
 */
ValNumber cfgval_chase_edge(CfgInstance cfg, SsaEdgeId edgeId)
{
    SsaNodeId source,sink;

    fst_index_t sinkVar, srcVar, name;
    int offset, length;
    ExpType	expType;

    source = SSA_edge(cfg,edgeId)->source;
    sink = SSA_edge(cfg,edgeId)->sink;

    sinkVar = SSA_node(cfg, sink)->name;
    srcVar  = SSA_node(cfg, source)->name;

    if (srcVar == DUMMY_GLOBAL(cfg))
    {
	if ((sinkVar != SSA_NIL_NAME) && (sinkVar != DUMMY_GLOBAL(cfg)))
	{
	    get_var_info(cfg, sinkVar, name, offset, length, expType);

	    return val_lookup_dummy(*V(cfg), expType,
				    cfgval_build(cfg, source),
				    SsaSymText(cfg, name),
				    offset, length);
	}
    }
    else if (srcVar != sinkVar)
    {
	//  Must be equivalenced variables...
	//  Say bottom unless same type and length.
	//  (should improve this later for array vars,
	//   but that requires a closer look at subscripts and 
	//   equivalence offsets)
	//
	int srcLen;
	ExpType srcType;
	get_var_info(cfg, sinkVar, name, offset, length, expType);
	get_var_info(cfg, srcVar,  name, offset, srcLen, srcType);

	if ((length != srcLen) || (expType != srcType))
	{
	    return VAL_BOTTOM;
	}
    }
    return cfgval_build(cfg, source);
}					 // end cfgval_chase_edge

ValNumber cfgval_evaluate(CfgInstance cfg, AST_INDEX refAst)
{
    SsaNodeId sn;
    ValNumber leftVal, rightVal;
    int constVal;

    if (refAst == ast_null_node)
	return VAL_BOTTOM;

    if (!fastEvalConstantIntExpr(cfg->symtab, refAst, &constVal))
    {
	/*
	 *  0 returned means constant result in constVal
	 */
	return val_lookup_const(*V(cfg), ast_get_real_type(refAst),
				constVal);
    }
    else if (is_constant(refAst))
    {
	/*
	 *  REAL or other non-integer constant
	 *  -- should do some checking here for floats that can
	 *     be precisely represented as integers.
	 */
	return val_lookup_text(*V(cfg), ast_get_real_type(refAst),
				gen_get_text(refAst));
    }
    else if (is_identifier(refAst))
    {
	fst_index_t index = SsaGetSym(cfg, refAst);
	if (FS_IS_MANIFEST_CONSTANT(cfg->symtab, index))
	{
	    //  Should probably do some caching of this info...
	    //
	    return cfgval_evaluate(cfg, 
				   (AST_INDEX)fst_GetFieldByIndex(cfg->symtab, 
								  index,
								  SYMTAB_EXPR));
	}

        sn = ssa_node_map(cfg, refAst);

        if (sn != SSA_NIL)
        {
            return cfgval_chase_edge(cfg, SSA_node(cfg, sn)->defsIn);
        }

	/*
	 *  Need to add handling for non-integer constants, especially
	 *  those with integer values.
	 */
	if (!FS_IS_ARRAY(cfg->symtab, index) && 
	    !FS_IS_MANIFEST_CONSTANT(cfg->symtab, index))
	{
	    /*
	     *  Look out for entry values used in declarations of 
	     *  formal parameters.
	     */
	    AST_INDEX decl;
	    ValNumber retVal;
	    // fst_index_t name;
	    // int offset, length, expType;

	    for (decl = out(refAst); decl != AST_NIL; decl = out(decl))
	    {
		switch(gen_get_node_type(decl))
		{
		  case GEN_DIMENSION:
		  case GEN_TYPE_STATEMENT:
		  case GEN_ARRAY_DECL_LEN:
		    /*
		     *  Assume this is declaration of formal parameter array
		     *  and return entry value of variable
		     */
		    retVal = cfgval_lookup_entry(cfg,
						 cfg_get_inst_name(cfg),
						 index);
		    if (retVal == VAL_NIL)
			return VAL_BOTTOM;
		    else
			return retVal;

		  default:
		    /* empty statement */ ;
		}
	    }
	    fprintf(stderr, "cfgval_evaluate: non-constant ref with no node\n");
	}
	return VAL_BOTTOM;
    }
    else if (cfgval_op_tree2val(gen_get_node_type(refAst)) != 
	     (ValOpType) VAL_BOT_TYPE)
    {
	AST_INDEX left,right;
	SsaNodeId leftSsa, rightSsa;

	/*
	 * Note: at this point we are assuming it's an expression, so try
	 * to pick it apart.
	 */
	
	left = right = AST_NIL;	
	left = ast_get_son_n(refAst,1);
	if (ast_get_son_count(refAst) == 2)
	    right = ast_get_son_n(refAst,2);
		
	leftSsa = ssa_node_map(cfg,left);
	
	if (leftSsa != SSA_NIL)
	    leftVal = cfgval_build(cfg,leftSsa);
	else
	    leftVal = cfgval_evaluate(cfg,left);

	if (right != ast_null_node)
	{
	    rightSsa = ssa_node_map(cfg,right);			

	    if (rightSsa != SSA_NIL)
		rightVal = cfgval_build(cfg,rightSsa);
	    else 
		rightVal = cfgval_evaluate(cfg,right);

	    return val_binary(V(cfg), 
			      cfgval_op_tree2val(gen_get_node_type(refAst)),
			      leftVal, rightVal);
	}
	else
	    return val_unary(V(cfg), 
			     cfgval_op_tree2val(gen_get_node_type(refAst)),
			     leftVal);
    }
    else if (is_subscript(refAst) || is_substring(refAst))
    {
	return cfgval_build(cfg, ssa_node_map(cfg, refAst));
    }
    else
    {
        /* we don't know how to evaluate these */
        return VAL_BOTTOM;
    }
}

static Boolean loop_var_search(CfgInstance cfg, SsaNodeId sn, SsaNodeId phi)
{
    if ((sn == SSA_NIL) ||					// dead end
	(sn == phi) ||						// self cycle ok
	(tarj_level(cfg_get_intervals(cfg), SSA_node(cfg, sn)->cfgParent) <
	 tarj_level(cfg_get_intervals(cfg), SSA_node(cfg, phi)->cfgParent)))
								// out of loop
    {
	return false;
    }

    SsaType type = SSA_node(cfg, sn)->type;

    if ((type == SSA_INDUCTIVE) ||	// a DO-loop variable
	(type == SSA_ETA) ||		// exit value of inner-loop variant
	(type == SSA_IP_MOD))		// interprocedural return value
    {
	return true;
    }

    CfgNodeId loop = SSA_node(cfg,sn)->cfgParent;
    TarjType loopType = tarj_type(cfg_get_intervals(cfg), loop);

    if ((SSA_node(cfg, sn)->type == SSA_PHI) &&
	((loopType == TARJ_INTERVAL) ||
	 (loopType == TARJ_IRREDUCIBLE)))
    {
	//  Depends on another loop-variant value
	//
	return true;
    }

    Boolean result = false;

    SsaNodeId kid;

    for (kid = SSA_node(cfg,sn)->subUses;
         !result && (kid != SSA_NIL);
	 kid = SSA_node(cfg,kid)->nextSsaSib)
    {
	result = (Boolean)(result || loop_var_search(cfg, kid, phi));
    }
    SsaEdgeId input;

    for (input = SSA_node(cfg,sn)->defsIn;
         !result && (input != SSA_NIL);
         input = SSA_edge(cfg,input)->inNext)
    {
	result = (Boolean)(result || 
			   loop_var_search(cfg, SSA_edge(cfg,input)->source,
					   phi));
    }
    return result;
}



static void unbuild_iterative(CfgInstance cfg, SsaNodeId sn, SsaNodeId phi)
{
    SsaNodeId kid;
    SsaEdgeId input;
    ValNumber val;
    ValType valType;

    if ((sn == SSA_NIL) ||					// dead end
	(sn == phi) ||						// self cycle ok
	(tarj_level(cfg_get_intervals(cfg), SSA_node(cfg, sn)->cfgParent) <
	 tarj_level(cfg_get_intervals(cfg), SSA_node(cfg, phi)->cfgParent)))
								// out of loop
    {
	return;
    }

    val = SSA_node(cfg, sn)->value;

    if (val == VAL_NIL) return;

    valType = ve_type(VE(cfg, val));

    if((valType == VAL_VAR_MU) ||
       (valType == VAL_OK_MU) ||
       (valType == VAL_ETA) ||
       (valType == VAL_IVAR)) 
    {
	return;
	/*
	 *  die_with_message("unbuild_iterative: %s\n",
	 *                   "bogus type.");
	 *
	 *  maybe not so bogus...
	 */
    }

    SSA_node(cfg, sn)->value = VAL_NIL;

    for (kid = SSA_node(cfg,sn)->subUses;
         kid != SSA_NIL;
	 kid = SSA_node(cfg,kid)->nextSsaSib)
    {
	unbuild_iterative(cfg, kid, phi);
    }
    for (input = SSA_node(cfg,sn)->defsIn;
         input != SSA_NIL;
         input = SSA_edge(cfg,input)->inNext)
    {
	unbuild_iterative(cfg, SSA_edge(cfg,input)->source, phi);
    }
}


//  build_list_val
//	Take an SSA node whose corresponding AST node represents a list.
//	Build and return a VAL_LIST.  Right now list lenght is limited by
//	maximum number of array subscript dimensions.
//
static ValNumber build_list_val(CfgInstance cfg,
				SsaNodeId   list)
{
    //  Get values for subscripts
    //
    if (list == SSA_NIL) 
	die_with_message("build_list_val: nil list.\n");

    ValNumber  list_vals[MAXDIM];    /* MAXDIM from dep/dg_instance.h */
    AST_INDEX  list_list;	     /* list of AST_INDEX's */
    AST_INDEX  list_node;            /* current AST node */
    int        max_indx;             /* length of list */
    int        indx;                 /* current list index */

    //  Build vector of subscript value numbers
    //
    list_list = SSA_node(cfg,list)->refAst;
    max_indx = list_length(list_list);
    assert (max_indx <= MAXDIM);

    for (indx = 0; indx < max_indx; indx++)
    {
	list_node       = list_retrieve(list_list, indx+1);

        SsaNodeId list_sn = ssa_node_map(cfg, list_node);

        if (list_sn == SSA_NIL)
                list_vals[indx] = cfgval_evaluate(cfg, list_node);
        else
                list_vals[indx] = cfgval_build(cfg, list_sn);
    }

    return val_lookup_list(*V(cfg), max_indx, list_vals);
}


/*
 *  build_array_val
 *
 *	Build value number for subscripted array reference.  Takes the
 *	old array value and subscripts as inputs.
 *
 *		If a definition, returned val# represents a modified
 *		whole-array value in which the subscripted element is 
 *		replaced with the rhs value.
 *
 *		If a use, returned val# represents the value of the 
 *		subscripted array element.
 *
 *	The passed SsaNodeId "ref" is a subscripted reference.  The 
 *	previous array definition should be its unique incoming 
 *	definition (defsIn).  The subscript list should be attached
 *	to an SSA_SUBSCRIPT node in the subUses list.  The rhs (if this
 *	is a definition) should be an SSA_RESULT or SSA_CALL in the 
 *	subUses list.  If this is some sort of strange SSA_DEF where
 *	there is no rhs (such as read from a file), then use a VAL_VARIANT
 *	for the rhs value.
 */
static ValNumber build_array_val(CfgInstance cfg,
				 SsaNodeId   ref)
{
    /*
     *  First extract stuff from the subUses list
     */
    SsaNodeId rhs = SSA_NIL;
    ValNumber subs = VAL_NIL;

    for (SsaNodeId subUse = SSA_node(cfg, ref)->subUses;
	 subUse != SSA_NIL;
	 subUse = SSA_node(cfg, subUse)->nextSsaSib)
    {
	switch(SSA_node(cfg, subUse)->type)
	{
	  case SSA_CALL:
	  case SSA_USE:
	    rhs = subUse;
	    break;

	  case SSA_SUBSCRIPT:
	    subs = build_list_val(cfg, subUse);
	    break;

	  default:
	    break;
	}
    }

    //  Get value for Right-Hand Side (if this is a def)
    //
    ValNumber rhsVal;

    if (ssa_is_def(cfg, ref))
    {
	if (rhs == SSA_NIL) 
	    //
	    //  We have some sort of I/O or mod via procedure
	    //	call, so give up.  Maybe someday we can do something
	    //	about analyzing subarray defs here, but not yet.
	    //
	    rhsVal = cfgval_lookup_variant(cfg, ref);
	else
	    rhsVal = cfgval_build(cfg, rhs);
    }
    else
	rhsVal = VAL_NIL;

    //  Get value number for reaching array definition
    //
    SsaEdgeId state = SSA_node(cfg, ref)->defsIn;

    if (SSA_edge(cfg, state)->inNext != SSA_NIL)
	die_with_message("build_array_val: too many reaching array defs\n");

    ValNumber stateVal = cfgval_chase_edge(cfg, state);

    //  Return final value number for array use/def
    //
    ExpType expType = fst_GetFieldByIndex(cfg->symtab, 
					  SSA_node(cfg, ref)->name, 
					  SYMTAB_TYPE);
    return val_lookup_array(*V(cfg), expType,
			    stateVal, rhsVal, subs);
}

ValPassMap *cfgval_build_passmap(CfgInstance cfg)
{
    ValPassMap *passed = new ValPassMap;
    SsaNodeId  sn;
    int site, parm;

    //  Build values passed at call sites
    //
    for (sn = ssa_get_first_node(cfg);
         sn != SSA_NIL;
         sn = ssa_get_next_node(cfg, sn))
    {
        SsaType type = SSA_node(cfg, sn)->type;

	if (type == SSA_ACTUAL)
	{
	    SSA_node(cfg,sn)->value = cfgval_build(cfg,sn);

	    site = get_call_nodenum(cfg, SSA_node(cfg, sn)->ssaParent);
	    parm = list_element(SSA_node(cfg, sn)->refAst);
	    ValNumber v = cfgval_build(cfg, sn);

	    passed->add_entry(site, parm, v);
	}
	else if ((type == SSA_IP_REF) || (type == SSA_IP_MOD))
	{
	    site = get_call_nodenum(cfg, SSA_node(cfg, sn)->ssaParent);
	    ValNumber v;

	    if (!is_null_node(SSA_node(cfg, sn)->refAst))
	    {
		//  reference actual -- repeated hacking on this codes
		//  means that the AST_INDEX may be associated with either
		//  the SSA_IP_REF or the SSA_IP_MOD, so look up the value
		//  on the incoming edge for safety.
		//
		v = cfgval_chase_edge(cfg, SSA_node(cfg, sn)->defsIn);

		parm = list_element(SSA_node(cfg, sn)->refAst);
		passed->add_entry(site, parm, v);
	    }
	    else if (type == SSA_IP_REF)  // ignore SSA_IP_MOD without ast
	    {
		//  implicitly passed global variable (may add some 
		//  reference formals here by accident, but it's safe 
		//  enough)
		//
		fst_index_t name;
		int offset, length;
		ExpType expType;
		get_var_info(cfg, SSA_node(cfg, sn)->name, 
			     name, offset, length, expType);

		v = cfgval_build(cfg, sn);

		passed->add_entry(site, SsaSymText(cfg, name), offset, v);
	    }
	    else if (SSA_node(cfg,sn)->name == DUMMY_GLOBAL(cfg))
	    {
		//  placeholder for invisible global variables
		//
		v = cfgval_chase_edge(cfg, SSA_node(cfg, sn)->defsIn);

		passed->add_entry(site, SsaSymText(cfg, DUMMY_GLOBAL(cfg)),
				  0, v);
	    }
	}
    }

    //  Build values "passed" at return from this procedure
    //
    for (sn = CFG_node(cfg, cfg->end)->ssaKids;
	 sn != SSA_NIL;
	 sn = SSA_node(cfg, sn)->nextCfgSib)
    {
	if (SSA_node(cfg, sn)->type == SSA_USE)
	{
	    //  We have the ref-on-exit node for a variable;
	    //  determine the variable and its underlying representative...
	    //
	    ValNumber v = cfgval_build(cfg, sn);

	    fst_index_t index = SSA_node(cfg, sn)->name;
	    
	    fst_index_t name;
	    int offset, length;
	    ExpType expType;
	    get_var_info(cfg, index, name, offset, length, expType);

	    //  Now see if it is a formal parameter or common block var
	    //  (otherwise we don't care about its value on exit)
	    //
	    //  Do this by name, not parm position, for actuals because
	    //  their position depends on which entry point was used.
	    //
	    if ((FS_IS_DUMMY_PARAM_FOR_ENTRY_OR_PROCEDURE(cfg->symtab, name)) ||
		(fst_GetFieldByIndex(cfg->symtab, index, SYMTAB_STORAGE_CLASS)& 
		 SC_GLOBAL) ||
		(name == DUMMY_GLOBAL(cfg)))
	    {
		passed->add_entry(SsaSymText(cfg, name), offset, v);
	    }
	}
    }

    return passed;
}

void cfgval_build_passNodes(CfgInstance cfg)
{
    SsaNodeId  sn;
    int site, parm;

    ValPassMap *passed = CFGVAL_stuff(cfg)->passNodes = new ValPassMap;

    //  This is a bit of a hack now, using a data structure originally
    //  for ValNumbers to store SSA nodes for passed values
    if (VAL_NIL != SSA_NIL)
	die_with_message("cfgval_build_passNodes: broken hack, %s\n",
			 "VAL_NIL != SSA_NIL");

    //  Build values passed at call sites
    //
    for (sn = ssa_get_first_node(cfg);
         sn != SSA_NIL;
         sn = ssa_get_next_node(cfg, sn))
    {
        SsaType type = SSA_node(cfg, sn)->type;

	if (type == SSA_ACTUAL)
	{
	    site = get_call_nodenum(cfg, SSA_node(cfg, sn)->ssaParent);
	    parm = list_element(SSA_node(cfg, sn)->refAst);

	    passed->add_entry(site, parm, sn);
	}
	else if ((type == SSA_IP_REF) || (type == SSA_IP_MOD))
	{
	    //  Currently, this may add extra entries when there are both
	    //  types of nodes for a parameter.  This should cause no harm,
	    //  and protects us from relying on the ast index being tied
	    //  to one node or the other.
	    //
	    AST_INDEX refAst;

	    //  get passed value
	    //
	    site = get_call_nodenum(cfg, SSA_node(cfg, sn)->ssaParent);

	    refAst = SSA_node(cfg, sn)->refAst;

	    if (!is_null_node(refAst))
	    {
		//  reference actual -- note that the AST_INDEX for 
		//  a reference actual is associated with the SSA_IP_REF 
		//  not with the SSA_IP_MOD (unless the value is not 
		//  used, in which case we don't need to add it here)
		//
		parm = list_element(refAst);
		passed->add_entry(site, parm, sn);
	    }
	    else 
	    {
		//  implicitly passed global variable
		//
		fst_index_t name;
		int offset, length;
		ExpType expType;
		get_var_info(cfg, SSA_node(cfg, sn)->name, 
			     name, offset, length, expType);

		passed->add_entry(site, SsaSymText(cfg, name), offset, sn);
	    }
	}
    }

    //  Build values "passed" at return from this procedure
    //
    for (sn = CFG_node(cfg, cfg->end)->ssaKids;
	 sn != SSA_NIL;
	 sn = SSA_node(cfg, sn)->nextCfgSib)
    {
	if (SSA_node(cfg, sn)->type == SSA_USE)
	{
	    fst_index_t index = SSA_node(cfg, sn)->name;
	    
	    fst_index_t name;
	    int offset, length;
	    ExpType expType;
	    get_var_info(cfg, index, name, offset, length, expType);

	    passed->add_entry(SsaSymText(cfg, name), offset, sn);
	}
    }

    return;
}

ValNumber cfgval_translate_passed(void *v_cfg, int site, char *name, 
				  int offset, int length)
{
    CfgInstance cfg = (CfgInstance) v_cfg;

    ValPassMap *passNodes = CFGVAL_stuff(cfg)->passNodes;

    if (!passNodes)
    {
	cfgval_build_passNodes(cfg);
	passNodes = CFGVAL_stuff(cfg)->passNodes;
    }
    SsaNodeId sn = passNodes->query_entry(site, name, offset);

    if (sn == SSA_NIL) return VAL_BOTTOM;

    if (SSA_node(cfg, sn)->type == SSA_IP_MOD)
    {
	return cfgval_chase_edge(cfg, SSA_node(cfg, sn)->defsIn);
    }
    else
    {
	return cfgval_build(cfg, sn);
    }
}

static void const_stats(CfgInstance cfg)
{
    SsaNodeId	sn;
    int		intVal;

    //  variables		that		integral constants
    //	    and						or
    //  root expressions	are		textual constants (string/real)
    //
    int varConst, expConst, varText, expText;
    varConst = expConst = varText = expText = 0;

    for (sn = ssa_get_first_node(cfg);
	 sn != SSA_NIL;
	 sn = ssa_get_next_node(cfg, sn))
    {
	AST_INDEX exp = SSA_node(cfg, sn)->refAst;

	if (!is_null_node(exp) && ssa_is_use(cfg, sn))
	{
	    ValNumber v = cfgval_build(cfg,sn);

	    if (val_is_value(v)) 
	    {
		ValType vt = val_get_val_type(V(cfg), v);

		if (vt == VAL_CONST)
		{
		    if (fastEvalConstantIntExpr(cfg->symtab, exp, &intVal))
		    {
			//  i.e., if not *obviously* constant, take credit
			//  for it.
			//
			if (is_identifier(exp))
			{
			    //  Count explicit variable references
			    //  (pseudo-defs don't count)
			    //
			    varConst++;
			}
			else
			{
			    //  Count root expressions
			    //  (internal expressions don't get nodes)
			    //
			    expConst++;
			}
		    }
		}
		else if (vt == VAL_TEXT)
		{
		    if (is_identifier(exp))
		    {        
			if (!FS_IS_MANIFEST_CONSTANT(cfg->symtab,
						     SsaGetSym(cfg, exp)))
			{
			    //  A variable, not a named constant
			    //
			    varConst++;
			}
		    }
		    else if (!is_constant(exp))
		    {
			//  Count root expressions
			//  (internal expressions don't get nodes)
			//
			expConst++;
		    }
		}
	    }
	}
    }
    printf("\tinteger constant vars: %d\n", varConst);
    printf("\tinteger constant exps: %d\n", expConst);
    // fprintf(stderr, "textual constant vars: %d\n", varText);
    // fprintf(stderr, "textual constant exps: %d\n", expText);
}
