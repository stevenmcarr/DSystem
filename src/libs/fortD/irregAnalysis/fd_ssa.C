/* $Id: fd_ssa.C,v 1.12 1997/03/11 14:28:38 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/**********************************************************************
 * Access to cfg/ssa related information (use/def edges etc.)
 * For further information, see ft_analysis/ssa/README.ssa
 */

/**********************************************************************
 * Revision History:
 * $Log: fd_ssa.C,v $
 * Revision 1.12  1997/03/11 14:28:38  carr
 * newly checked in as revision 1.12
 *
Revision 1.12  94/03/21  13:39:38  patton
fixed comment problem

Revision 1.11  94/02/27  20:15:34  reinhard
Added value-based distributions.
Make CC happy.
See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.

 * Revision 1.12  1994/02/27  19:48:03  reinhard
 * Added parameter to fd_ssa_init().
 *
 * Revision 1.11  1994/01/18  19:52:58  reinhard
 * Updated include paths according to Makefile change.
 * Handle value-based distributions.
 *
 * Revision 1.10  1993/10/04  15:42:11  reinhard
 * Const'd param.
 *
 * Revision 1.9  1993/09/25  23:04:41  reinhard
 * Robustified slice generation.
 *
 * Revision 1.8  1993/09/02  18:54:47  reinhard
 * Incorporated paco's changes.
 *
Revision 1.9  93/08/18  04:18:32  paco
Removed calls to ssa_edge_map;
added useIpVals argument to cfgval_Open

Revision 1.8  93/07/13  18:12:05  reinhard
See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg for details.

 * Revision 1.6  93/04/22  19:25:19  reinhard
 * Added cfg_split_critical_edges() call.
 * 
 * Revision 1.5  1993/04/08  18:51:43  reinhard
 * Robustified simple slice generator.
 *
 * Revision 1.4  1993/03/31  20:17:37  reinhard
 * Removed cfg_build_cfg_cds() call.
 *
 */

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>
#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/ast.h>
#include <libs/frontEnd/ast/astlist.h>
#include <include/frontEnd/astnode.h>
#include <include/frontEnd/astrec.h>
#include <libs/frontEnd/ast/groups.h>
#include <libs/moduleAnalysis/ssa/ssa.h>
#include <libs/moduleAnalysis/cfgValNum/cfgval.h>

/*------------------- GLOBAL DECLARATIONS -------------------*/

EXTERN(void, cfg_split_critical_edges, (CfgInstance cfg));

/*------------------- LOCAL DECLARATIONS --------------------*/

EXTERN(CfgInfo,   fd_ssa_init, (FortTree ft, Boolean with_values));
EXTERN(void,      fd_ssa_cleanup, (CfgInfo cfgGlobals));
EXTERN(AST_INDEX, fd_ssa_slice, (CfgInstance cfg,
				 AST_INDEX start_node,
				 AST_INDEX stop_node));
EXTERN(AST_INDEX, genLongComment, (const char *cmt_str));
EXTERN(CfgNodeId, fd_ssa_get_unique_def_cn, (CfgInstance     cfg,
					     AST_INDEX start_node));
EXTERN(const char *, fd_ssa_get_unique_def_name, (CfgInstance     cfg,
						  AST_INDEX start_node));


/**********************************************************************
 * fd_ssa_init()  Generate the cfg/ssa information needed by Fortran D 
 *                compiler
 */
CfgInfo
fd_ssa_init(FortTree ft, Boolean with_values)
{
  CfgInfo     cfgGlobals;
  CfgInstance cfg;
  Generic     ipInfo    = 0;   // No interprocedural information yet
  Boolean     ipSmush   = false;
  Boolean     doArrays  = true;
  Boolean     doDefKill = true;
  Boolean     doGated   = true;
  Boolean     dumpSort  = false; // Set this to true for debugging

  cfgGlobals = cfg_Open(ft);
  for (cfg = cfg_get_first_inst(cfgGlobals);
       cfg;
       cfg = cfg_get_next_inst(cfg))
  {
    cfg_split_critical_edges(cfg);
  }

  ssa_Open(cfgGlobals, ipInfo, ipSmush, doArrays, doDefKill, doGated);
  cfgval_Open(cfgGlobals, /* useIpVals = */ false);

  for (cfg = cfg_get_first_inst(cfgGlobals);
       cfg;
       cfg = cfg_get_next_inst(cfg))
  {
    (void) cfg_get_predom(cfg);
    (void) cfg_get_postdom(cfg);
    (void) cfg_get_intervals(cfg);  // Compute (sorted) intervals

    if (with_values)
    {
      cfgval_build_table(cfg);        // Perform value numbering
    }

    if (dumpSort)
      cfg_sorted_dump(cfg);
  }

  return cfgGlobals;
}


/**********************************************************************
 * fd_ssa_cleanup()  Delete cfg/ssa information.
 *
 * NOTE: 4/19/93 RvH: ssa_Close() bombs if the AST has been modified
 *       after cfgGlobals have been constructed.
 */
void
fd_ssa_cleanup(CfgInfo cfgGlobals)
{
  cfgval_Close(cfgGlobals);
  ssa_Close(cfgGlobals);
  cfg_Close(cfgGlobals);
}


/**********************************************************************
 * fd_ssa_slice()  Given an expression <start_node>, trace all
 *                 contributing stmts until <stop_node> is hit.
 * Note the ordering of start and stop: in the code, the <stop_node>
 * is executed BEFORE the <start_node> !
 */
AST_INDEX
fd_ssa_slice(CfgInstance cfg,
	     AST_INDEX start_node,
	     AST_INDEX stop_node)
{
  AST_INDEX node, stmts_node, src_node, def_node, label_node;
  SsaEdgeId start_ssa_edge;
  SsaEdgeId start_ssa_node;
  SsaNodeId src_ssa_node;

  stmts_node = list_create(AST_NIL); // Start a list of stmts

  // 930121 RvH: since this is more Raja's turf, this just traces back
  // one stmt to demonstrate use of ssa info.

  // ASSUMPTION: <start_node> is simple identifier, like "n1"
  if (ssa_node_exists(cfg, start_node)
      && ((start_ssa_node = ssa_node_map(cfg, start_node))
	  != SSA_NIL)
      && ((start_ssa_edge = ssa_first_in(cfg, start_ssa_node))
	  != SSA_NIL)
      && ((src_ssa_node   = ssa_edge_src(cfg, start_ssa_edge))
	  != SSA_NIL)
      && ((src_node       = ssa_node_to_ast(cfg, src_ssa_node))
	  != AST_NIL)
      && ((def_node       = ast_get_father(src_node))
	  != AST_NIL)
      && is_assignment(def_node))
  {
    node = tree_copy(def_node);

    // Don't copy labels
    if (labelled_stmt(node))
    {
      label_node = gen_get_label(node);
      if (label_node != AST_NIL)
      {
        pt_tree_replace(label_node, AST_NIL);
      }
    }
  }
  else
  {
    node = genLongComment("--<< WARNING: fd_ssa_slice(): Could not generate slice >>--");
  }
  (void) list_insert_last(stmts_node, node);

  return stmts_node;
}


/**********************************************************************
 * fd_ssa_get_unique_def_cn()  Given an expression <start_node>,
 *                             try to find a unique definition
 */
CfgNodeId
fd_ssa_get_unique_def_cn(CfgInstance cfg,
			 AST_INDEX   start_node)
{
  AST_INDEX src_node, def_node;
  SsaEdgeId start_ssa_edge;
  SsaEdgeId start_ssa_node;
  SsaNodeId src_ssa_node;
  CfgNodeId def_cn = CFG_NIL;

  // 93/12/7 RvH: This should be Raja's job ...

  // ASSUMPTION: <start_node> is simple identifier, like "n1"
  if (ssa_node_exists(cfg, start_node)
      && ((start_ssa_node = ssa_node_map(cfg, start_node))
	  != SSA_NIL)
      && ((start_ssa_edge = ssa_first_in(cfg, start_ssa_node))
	  != SSA_NIL)
      && ((src_ssa_node   = ssa_edge_src(cfg, start_ssa_edge))
	  != SSA_NIL)
      && ((src_node       = ssa_node_to_ast(cfg, src_ssa_node))
	  != AST_NIL)
      && ((def_node       = ast_get_father(src_node))
	  != AST_NIL)
      && is_assignment(def_node))
  {
    def_cn = ssa_get_cfg_parent(cfg, src_ssa_node);
  }

  return def_cn;
}


/**********************************************************************
 * fd_ssa_get_unique_def_cn()  Given an expression <start_node>,
 *                             try to find a unique definition
 */
const char *
fd_ssa_get_unique_def_name(CfgInstance cfg,
			   AST_INDEX   start_node)
{
  AST_INDEX  def_node, rhs_node, name_node;
  CfgNodeId  def_cn;
  const char *def_name = NULL;

  // 93/12/7 RvH: This should be Raja's job ...

  def_cn = fd_ssa_get_unique_def_cn(cfg, start_node);

  if (def_cn != CFG_NIL)
  {
    if (((def_node       = cfg_node_to_ast(cfg, def_cn))
	 != AST_NIL)
	&& is_assignment(def_node)
	&& ((rhs_node = gen_ASSIGNMENT_get_rvalue(def_node))
	    != AST_NIL)
	&& is_subscript(rhs_node)
	&& ((name_node = gen_SUBSCRIPT_get_name(rhs_node))
	    != AST_NIL))
    {
      def_name = gen_get_text(name_node);
    }
  }

  return def_name;
}
