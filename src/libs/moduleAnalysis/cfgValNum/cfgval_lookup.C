/* $Id: cfgval_lookup.C,v 1.7 1997/03/11 14:35:39 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/moduleAnalysis/cfg/cfg.h>
#include <libs/moduleAnalysis/ssa/ssa.h>
#include <libs/moduleAnalysis/cfgValNum/cfgval.i>
#include <libs/moduleAnalysis/valNum/val_pass.h>

ValNumber cfgval_lookup_entry(CfgInstance cfg,
			      char *entryPt,
			      fst_index_t var)
{
    if (var == DUMMY_GLOBAL(cfg))
    {
	return val_lookup_entry(*V(cfg), TYPE_UNKNOWN,
				entryPt,
				SsaSymText(cfg, var),
				0, 0);
    }

    int length, offset;
    ExpType expType;
    fst_index_t rep, gName;

    rep = ssa_var_rep_extent(cfg, var, &length);
    length = fst_GetFieldByIndex(cfg->symtab, var, SYMTAB_SIZE);
    expType = fst_GetFieldByIndex(cfg->symtab, var, SYMTAB_TYPE);

    if (var != rep)
    {
	if (length != fst_GetFieldByIndex(cfg->symtab, rep, SYMTAB_SIZE))
	    return VAL_BOTTOM;

	if (expType != fst_GetFieldByIndex(cfg->symtab, var, SYMTAB_TYPE))
	    return VAL_BOTTOM;
    }
    //  Since rep covers var, same length implies same offset
    //  (but not vice versa).
    //
    offset = fst_GetFieldByIndex(cfg->symtab, var, SYMTAB_EQ_OFFSET);

    gName = (fst_index_t) fst_GetFieldByIndex(cfg->symtab, 
					     rep, SYMTAB_PARENT);
    if (gName == SSA_NIL_NAME) gName = rep;

    // if (get_ip_const(cfg, CFG_node(cfg, stmt)->astnode,
    //		var, &constVal))
    // {
    //	expType = fst_GetFieldByIndex(cfg->symtab, var, SYMTAB_TYPE);
    //	returnval_lookup_const(*V(cfg), expType, constVal);
    // }
    if (CFGVAL_stuff(cfg)->entryVals)
    {
	ValPassMap *entryVals = CFGVAL_stuff(cfg)->entryVals;

	//  Clean up any type confusion (no op if type is correct)
	//  This assumes confusion caused by sloppy IP analysis
	//  and not by sloppy Fortran programmers.
	//
	if (entryVals->query_entry(SsaSymText(cfg, gName), offset)
	    != VAL_NIL)
	{
	    ValNumber rv;
	    rv = val_convert(V(cfg),
			     entryVals->query_entry(SsaSymText(cfg, gName), 
						    offset),
			     expType);
	    if (rv != VAL_BOTTOM) return rv;
	}
    }
    //  else or didn't return just above
    //
    return val_lookup_entry(*V(cfg), expType, entryPt,
			    SsaSymText(cfg, gName), offset, length);
}

ValNumber cfgval_lookup_variant(CfgInstance cfg,
				SsaNodeId occur)
{
    AST_INDEX astNode = SSA_node(cfg, occur)->refAst;
    fst_index_t name = SSA_node(cfg, occur)->name;
    ExpType expType;

    if ((name != SSA_NIL_NAME) &&
	(name != DUMMY_GLOBAL(cfg)))
    {
	expType = fst_GetFieldByIndex(cfg->symtab, name, SYMTAB_TYPE);
    }
    else
    {
	expType = (is_null_node(astNode) || is_list(astNode))?
	    TYPE_UNKNOWN : gen_get_real_type(astNode);
    }

    return val_lookup_variant(*V(cfg),
			      expType,
			      occur,
                              tarj_level(cfg_get_intervals(cfg),
                                         SSA_node(cfg, occur)->cfgParent));

} /* end of cfgval_lookup_variant() */

ValNumber cfgval_lookup_ok_mu(CfgInstance cfg, SsaNodeId mu,
			      ValNumber initVal, ValNumber iterVal)
{
    CfgNodeId loop = SSA_node(cfg,mu)->cfgParent;
    int level = tarj_level(cfg_get_intervals(cfg), loop);
    
    //  Check to see if iterVal fits the form (constant + mu_placeholder)
    //
    ValEntry *iter = &(VE(cfg, iterVal));
    if ((ve_type(*iter) != VAL_OP) ||
	(ve_opType(*iter) != VAL_OP_PLUS) ||
	!val_is_const(V(cfg), ve_left(*iter)) ||
	(ve_type(VE(cfg, ve_right(*iter))) != VAL_MU_PH) ||
	//
	//  this last condition would really be a bug!
	//
	(ve_level(VE(cfg, ve_right(*iter))) != level))
    {
	return val_lookup_ok_mu(*V(cfg), initVal, iterVal, level);
    }

    //  Get loop information
    //
    Loop_vals *loopInfo = cfgval_loop_from_cfg(cfg, loop);

    if (loopInfo->ivar == NULL)
    {
	//  not inductive DO
	//
	return val_lookup_ok_mu(*V(cfg), initVal, iterVal, level);
    }

    //  Check that step S for ivar divides the iterative constant C
    //
    ValNumber coeff = val_binary(V(cfg), VAL_OP_DIVIDE, 
				 ve_left(*iter), loopInfo->step);
    if (!val_is_const(V(cfg), coeff))
	return val_lookup_ok_mu(*V(cfg), initVal, iterVal, level);

    if (loopInfo->rev) coeff = val_unary(V(cfg), VAL_OP_MINUS, coeff);

    //  Return value is initVal + flipped * (C/S) * (ivar - start)
    //                            ^^^^^^^^^^^^^^^
    //                                 coeff
    //
    ValNumber start = loopInfo->rev? loopInfo->up : loopInfo->lo;

    return val_binary(V(cfg), VAL_OP_PLUS, initVal,
		      val_binary(V(cfg), VAL_OP_TIMES, coeff,
				 val_binary(V(cfg), VAL_OP_MINUS,
					    loopInfo->ival, start)));
}
