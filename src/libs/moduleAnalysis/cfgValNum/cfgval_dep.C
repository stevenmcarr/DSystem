/* $Id: cfgval_dep.C,v 1.9 1997/03/11 14:35:38 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/moduleAnalysis/cfgValNum/cfgval.i>
#include <libs/moduleAnalysis/dependence/dependenceTest/dep_dt.h>

static void extract_loop_info(CfgInstance cfg, CfgNodeId cn, Loop_vals &lv);

EXTERN(void, cfgval_dep_test, (CfgInstance cfg));

void cfgval_dep_test(CfgInstance cfg)
{
    SsaNodeId   sn;

    for (sn = ssa_get_first_node(cfg);
         sn != SSA_NIL;
         sn = ssa_get_next_node(cfg, sn))
    {
        SsaType type = SSA_node(cfg, sn)->type;

        if (type == SSA_SUBSCRIPT)
        {
	    for (SsaNodeId kid = SSA_node(cfg,sn)->subUses;
		 kid != SSA_NIL;
		 kid = SSA_node(cfg,kid)->nextSsaSib)
	    {
		if (!is_null_node(SSA_node(cfg, kid)->refAst))
		{
		    CoVarPair *cp =
			cfgval_dep_parse(cfg,
					 SSA_node(cfg, kid)->refAst);
		    val_dep_free(cp);
		}
	    }
        }
    }
}

Loop_vals * cfgval_loop_from_cfg(CfgInstance cfg, CfgNodeId cn)
{
    assert((cn > CFG_NIL) && (cn < cfg_node_max(cfg)));

    Loop_vals *lv = new Loop_vals;

    TarjTree intervals = cfg_get_intervals(cfg);

    switch(tarj_type(intervals, cn))
    {
      case TARJ_NOTHING:
      case TARJ_ACYCLIC:
	delete lv;
	lv = NULL;
        break;

      case TARJ_IRREDUCIBLE:
	lv->loop_ast = CFG_node(cfg, cn)->astnode;
	lv->lo = lv->up = lv->step = VAL_BOTTOM;
	lv->ivar = NULL;
	lv->fwd  = false;
	lv->rev  = false;
	break;

      case TARJ_INTERVAL:
	extract_loop_info(cfg, cn, *lv);
	break;

      default:
	assert(false);	// should not be any other possibilities
	break;
    }
    return lv;
}

CoVarPair * cfgval_dep_parse(CfgInstance cfg, AST_INDEX sub)
{
    //  Find closest surrounding loop, get its level, and allocate
    //  that many elements for the returned vector
    //
    CfgNodeId loop = cfg_containing_loop(cfg, sub);
    int depth = 0;
    if (loop != CFG_NIL)
	depth = tarj_level(cfg_get_intervals(cfg), loop);

    ValNumber right = cfgval_evaluate(cfg, sub);
    ValTable *Vp = V(cfg);

    CoVarPair *rv = val_dep_parse(Vp, right, depth);

    return rv;
}

void cfgval_dep_parse_sub(CfgInstance cfg, Subs_data *sub, int depth)
{
  ValNumber val = cfgval_evaluate(cfg, sub->sym);
  ValTable *Vp  = V(cfg);

  val_dep_parse_sub(Vp, val, depth, sub);
}  

void cfgval_dep_parse_loop_bound(CfgInstance cfg, 
				 int         depth,
				 AST_INDEX   bound_ast,
				 Expr       *bound_expr, 
				 ValNumber  *bound_val, 
				 int       **bound_vec)
{
  *bound_val = cfgval_evaluate(cfg, bound_ast);
  ValTable *Vp = V(cfg);

  val_dep_parse_loop_bound(Vp, *bound_val, depth, 
			   bound_ast, bound_expr, bound_vec);
}  

static void extract_loop_info(CfgInstance cfg, CfgNodeId cn, Loop_vals &lv)
{
    //  cn is the header of a reducible loop.  Find the loop bound information.
    //  If not a DO loop, assume start and stride are 1, upper bound unknown.
    //
    lv.loop_ast = CFG_node(cfg, cn)->astnode;

    SsaNodeId sn = CFG_node(cfg, cn)->ssaKids;

    for (;
	 (sn != SSA_NIL) && (SSA_node(cfg, sn)->type != SSA_PHI);
	 sn = SSA_node(cfg, sn)->nextCfgSib)
    {
	//  All the loop initialization nodes come before any PHI.
	//	SSA_INDUCTIVE, SSA_LOOP_INIT, SSA_LOOP_BOUND, SSA_LOOP_STEP
	//
	if (SSA_node(cfg, sn)->type == SSA_INDUCTIVE)
	    break;
    }
    if ((sn != SSA_NIL) && (SSA_node(cfg, sn)->type == SSA_INDUCTIVE))
    {
	lv.ivar = SsaSymText(cfg, SSA_node(cfg, sn)->name);

	lv.ival = cfgval_build(cfg, sn);
	lv.rev  = (Boolean)ve_flipped(VE(cfg,lv.ival));

	ValNumber bds = ve_bounds(VE(cfg,lv.ival));
	lv.fwd  = (Boolean)ve_simple(VE(cfg,bds));
	lv.lo   = ve_lo(VE(cfg,bds));
	lv.up   = ve_hi(VE(cfg,bds));
	lv.step = ve_step(VE(cfg,bds));
    }
    else	// not an inductive loop
    {
	lv.ival = lv.up = VAL_BOTTOM;
	lv.lo = lv.step = VAL_ONE;
	lv.ivar = NULL;
	lv.fwd  = false;
	lv.rev  = false;
    }
}
