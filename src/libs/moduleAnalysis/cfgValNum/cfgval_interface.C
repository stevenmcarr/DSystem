/* $Id: cfgval_interface.C,v 1.9 1997/03/11 14:35:38 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/***************************************************************************

	val_interface.c -- mostly interface routines

	by:  Tim Mullin					July 1991
	updates:

	Changed val_{def,exp}_{val,tree,int,is_const} to
		cfgval_get_{val,int,is_const}, based on Dan Grove's
		val_get_val.				paco, August 1992
	

****************************************************************************/

#include <libs/moduleAnalysis/cfgValNum/cfgval.i>

/************************************************************/
/*                                                          */
/* cfgval_get_val() returns the value number of an ast node */
/*                                                          */
/* simplified to rely on val_evaluate for expressions       */
/*                                        paco Feb 1993     */
/*                                                          */
/************************************************************/
ValNumber cfgval_get_val(CfgInstance cfg, AST_INDEX node)
{
    SsaNodeId ssaId;
    ssaId = ssa_node_map(cfg,node);
	
    if (ssaId == SSA_NIL)
    {
	return cfgval_evaluate(cfg, node);
    }
    else
    {
	/*
	 *  Value number the SSA node corresponding to the AST node
	 */
	return cfgval_build(cfg, ssaId);
    }
}	



/*************************************************************/
/*                                                           */
/* cfgval_get_int() returns the integer value of an ast node */
/* that is a constant.  Returns zero if it isn't a           */
/* constant.                                                 */
/*                                                           */
/*************************************************************/
long cfgval_get_int(CfgInstance cfg, AST_INDEX node)
{
    ValNumber		result;

    /* if the node is simply a constant, return its value */
    if (is_constant(node))
        return (long)atoi(gen_get_text(node));

    result = cfgval_get_val(cfg, node);

    if (val_is_const(V(cfg),result))
    {
        return ve_const(VE(cfg,result));
    }
    else 
        return 0;
}



/*******************************************************/
/*                                                     */
/* cfgval_get_is_const() returns true if the node is a */
/* constant, otherwise returns false                   */
/*                                                     */
/*******************************************************/
Boolean cfgval_get_is_const(CfgInstance cfg, AST_INDEX node)
{
    ValNumber		result;

    /* if the node is simply a constant, return its value */
    if (is_constant(node))
        return true;

    result = cfgval_get_val(cfg, node);

    if (val_is_const(V(cfg),result))
        return true;
    else 
        return false;
}


fst_index_t cfgval_get_ftsym(CfgInstance cfg, ValNumber valNum)
{
    if ((valNum == VAL_BOTTOM) || (valNum == VAL_TOP) ||
        (valNum == VAL_NIL) ||
	(!(ve_type(VE(cfg, valNum)) == VAL_ENTRY) &&
	!(ve_type(VE(cfg, valNum)) == VAL_RETURN)))
        return(SYM_INVALID_INDEX);

    else
	return fst_QueryIndex((cfg)->symtab, 
			      ve_string(VE(cfg, ve_name(VE(cfg, valNum)))));

} /* end of cfgval_get_ftsym() */


/*******************************************************/
/*                                                     */
/* cfgval_get_values()  returns values                 */
/*                                                     */
/*******************************************************/
Values cfgval_get_values(CfgInstance cfg)
{
    return V(cfg);
}


//  cfgval_table_max
//
//	returns integer guaranteed to be greater than ((int) v) for any
//	ValNumber v -- but this may be invalidated for any ValNumbers 
//	built after the call to cfgval_table_max.
//
int cfgval_table_max(CfgInstance cfg)
{
    return val_table_max(V(cfg));
}
