/* $Id: cfgval.i,v 1.9 1997/03/11 14:35:37 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * -- cfgval.i
 * 
 *          This file is the header file for value graphs attached to 
 *		a CFG/SSA object.
 * 
 */


#ifndef cfgval_i
#define cfgval_i

#include <libs/moduleAnalysis/valNum/val.i>
#include <libs/moduleAnalysis/ssa/ssa_private.h>
#include <libs/moduleAnalysis/cfg/cfg_utils.h>
#include <libs/frontEnd/ast/forttypes.h>
#include <libs/moduleAnalysis/cfgValNum/cfgval.h>
#include <libs/moduleAnalysis/valNum/val_ip.h>

/*
 *  Fields borrowed from CFG
 */
typedef struct val_stuff_struct {
    ValIP       vip;
    ValPassMap  *entryVals;
    ValPassMap  *passNodes;
} *ValStuff;

/*
 *  Induce a compilation error if ssa_stuff_struct is too big
 *  -- array dimension cannot be zero or negative.
 */
char cfgval_foo_junk[(VAL_WORK_SLOTS * sizeof(Generic)) -
		     sizeof(struct val_stuff_struct) +1];


/*
 *  I == integer
 *  S == SsaNodeId
 *  C == CfgNodeId
 *  V == ValNumber
 *
 *  These are the value types with special interpretations when attached
 *  to the CFG/SSA:
 *
 *  field[0](type) field[3]	field[4]    field[5]	field[6]
 *  -------------- --------	--------    --------	--------
 *  VAL_VARIANT    occur S
 *  VAL_PHI	   stmt C	arity I     kid[0] ...
 *  VAL_TEST	   occur S	itest V     test_type I
 *
 *  VAL_VARIANT for unanalyzable computations; SsaNodeId guarantees no match
 *  VAL_PHI     is only comparable with other VAL_PHIs at same statement.
 *  VAL_TEST    captures grungy Fortran test depending on statement.
 *
 *  see val.i for more explanation
 */

EXTERN(ValPassMap *, cfgval_build_passmap, (CfgInstance cfg));

inline ValIP *VIP(CfgInstance cfg)
{
    ValIP *vip = &(((ValStuff)cfg->valStuff)->vip);
    if (!(vip->pass)) vip->pass = cfgval_build_passmap(cfg);
    return vip;
}

inline Values &V(CfgInstance cfg)
{
    return (((ValStuff)cfg->valStuff)->vip.values);
}

inline ValStuff CFGVAL_stuff(CfgInstance cfg)
{
    return (ValStuff)cfg->valStuff;
}

inline ValEntry &VE(CfgInstance cfg, ValNumber id)
{
    return ((*(V(cfg)))[id]);
}

//inline int CFGVAL_matched(CfgInstance cfg)
//{
//    return (((ValStuff)cfg->valStuff)->valMatched);
//}
inline Generic CFGVAL_parms(CfgInstance cfg)
{
    return (cfg->cfgGlobals->valParms);
}
inline Boolean CFGVAL_useIpVals(CfgInstance cfg)
{
    return *((Boolean *)(cfg->cfgGlobals->valParms));
}

EXTERN(ValNumber, cfgval_lookup_entry, (CfgInstance cfg,
					char *entryPt,
					fst_index_t var));
EXTERN(ValNumber, cfgval_lookup_variant, (CfgInstance cfg, 
					  SsaNodeId occur));
EXTERN(ValNumber, cfgval_lookup_ok_mu, (CfgInstance cfg,
					SsaNodeId mu,
					ValNumber initVal,
					ValNumber iterVal));

EXTERN(ValOpType, cfgval_op_tree2val,	(int treeOp));
EXTERN(int, cfgval_op_val2tree,		(ValOpType valOp));

EXTERN(void, cfgval_OpenOne,		(Generic junk, CfgInstance cfg));
EXTERN(void, cfgval_CloseOne,		(Generic junk, CfgInstance cfg) );

EXTERN(ValNumber, cfgval_evaluate,   (CfgInstance cfg, AST_INDEX refAst));
EXTERN(int, cfgval_table_usage,      (CfgInstance cfg));

EXTERN(ValNumber, cfgval_build_subscripted, (CfgInstance cfg,
					     SsaNodeId   ref,
					     SsaNodeId   rhs));
EXTERN(void, cfgval_constraints, (CfgInstance cfg));
#endif /* ! cfgval_i */
