/* $Id: AST_SSA_Graph.C,v 1.9 1997/03/11 14:28:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/**********************************************************************
 * Class for AST_INDEX based accesses to the CFG/SSA graph,
 * derived from class AST_Graph.
 */
/**********************************************************************
 * Revision history:
 * $Log: AST_SSA_Graph.C,v $
 * Revision 1.9  1997/03/11 14:28:24  carr
 * newly checked in as revision 1.9
 *
Revision 1.9  94/03/21  14:07:33  patton
fixed comment problem

Revision 1.8  94/02/27  20:14:11  reinhard
Added value-based distributions.
Make CC happy.
See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.

 * Revision 1.10  1994/02/27  19:37:30  reinhard
 * Added Boolean with_values parameter.
 *
 * Revision 1.9  1994/01/18  19:43:00  reinhard
 * Updated include paths according to Makefile change.
 *
 * Revision 1.8  1993/09/02  18:41:48  reinhard
 * Made constructor ANSI C++ compatible.
 *
 * Revision 1.7  1993/06/09  23:42:01  reinhard
 * Cleaned up include hierarchy.
 *
 */
#include <assert.h>
#include <libs/fortD/irregAnalysis/AST_SSA_Graph.h>
#include <libs/moduleAnalysis/cfgValNum/cfgval.h>


/*------------------- GLOBAL DECLARATIONS -------------------*/

EXTERN(void,      fd_ssa_cleanup, (CfgInfo cfgGlobals));
EXTERN(CfgInfo,   fd_ssa_init, (FortTree ft, Boolean with_values));
EXTERN(AST_Set*,  SSA_ipreds_gen,(AST_INDEX node, Generic graph));

/**********************************************************************
 * Constructor if cfg is computed already.
 */
AST_SSA_Graph::AST_SSA_Graph(CfgInstance my_cfg)
: AST_Graph ((Data_gen_ftype) &SSA_ipreds_gen, (Generic) my_cfg)
{
}


/**********************************************************************
 * Constructor if cfg is not computed yet.
 */
AST_SSA_Graph::AST_SSA_Graph(FortTree ft, Boolean with_values)
: AST_Graph ((Data_gen_ftype) &SSA_ipreds_gen)
{
  cfgGlobals = fd_ssa_init(ft, with_values);
  graph      = (Generic) cfg_get_first_inst(cfgGlobals);
  ipreds_ht  = new AST_Set_ht(walk_func, graph);
}


/**********************************************************************
 * Destructor
 */
AST_SSA_Graph::~AST_SSA_Graph()
{
  fd_ssa_cleanup(cfgGlobals);  //930125 RvH: bombs
}


/**********************************************************************
 * Given an AST_INDEX <node> and a graph, produce it's immediate
 * predeccessors <ipreds>
 * <node>  can refer to either an expression or a statement.
 * <preds> is a set of statements.
 *
 * 
 * EXAMPLE:
 * ========
 *
 * There are some slightly tricky issues about what exactly constitutes
 * an immediately preceeding statements, in particular in the presence
 * of control dependeces.
 *
 * 2/10/93 RvH: For now it seems reasonable to try to always generate
 * a self-contained set of statements; ie, when a statement is directly
 * control dependent on some control construct (like if-else-endif or
 * do-enddo), then return enough of the whole construct to generate a
 * syntactically correct program.
 * For example, <ipreds> for first stmt of the then-branch of an
 * if-else-endif construct contains both the if and the endif stmts
 * (but not the else stmt).
 * 
 * In the following, assume that the AST_INDEX of an expression is
 * its line number.
 *
 * 1  procedure loop(nde1, nde2, x, y)
 * 2  do i = 1, 50
 * 3    if (i < 26) then
 * 4      j = i + 25
 * 5      n1 = nde1(j)
 * 6    else
 * 7      j = i - 25
 * 8      n1 = nde2(j)
 * 9    endif
 * 10   y(n1) = x(n1) * 2
 * 11 enddo
 *
 * In line 10, let the occurences of n1 in y(n1) and x(n1) have the
 * AST_INDEX's 12 and 13, respectively.
 *
 * <node> | <ipreds>
 * -----------------
 *   12   | 5, 8
 *    5   | 1 [nde1], 4 [j]    [Might want 3, 9 in here already ?]
 *    8   | 1 [nde2], 7 [j]    [Might want 3, 6, 9 in here already ?]
 *    1   | <empty>
 *    4   | 2, 11 [i], 3, 9    [immediate control dependence]
 *    7   | 2, 11 [i], 3, 6, 9 [immediate control dependences]
 *    2   | <empty>
 *    3   | 2, 11
 *    6   | 3
 *    9   | 3
 *   11   | 2
 */
AST_Set *
SSA_ipreds_gen(AST_INDEX node, Generic graph)
{
  CfgInstance cfg = (CfgInstance) graph;
  AST_Set     *result;

  result = new AST_Set;
  // Raja: Predecessor generation goes here

  return result;
};
