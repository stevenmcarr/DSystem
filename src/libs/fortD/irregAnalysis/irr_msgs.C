/* $Id: irr_msgs.C,v 1.16 1997/03/11 14:28:38 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/**********************************************************************
 * This is the interface file between the "regular" and the
 * "irregular" part of the Fortran D world.  All functions called
 * directly from the "regular" compiler should be here.
 * This file replaces ped_cp/dc/irr_msgs.c 
 */

/**********************************************************************
 * Revision History:
 * $Log: irr_msgs.C,v $
 * Revision 1.16  1997/03/11 14:28:38  carr
 * newly checked in as revision 1.16
 *
Revision 1.16  94/03/21  13:41:20  patton
fixed comment problem

Revision 1.15  94/02/27  20:15:38  reinhard
Added value-based distributions.
Make CC happy.
See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.

 * Revision 1.16  1994/02/27  19:48:19  reinhard
 * Added dc_irreg_analyze().
 *
 * Revision 1.15  1994/01/18  19:53:14  reinhard
 *  Updated include paths according to Makefile change.
 * Handle value-based distributions.
 *
 */

#include <iostream.h>

#include <libs/frontEnd/ast/AST_Set.h>
#include <libs/fortD/irregAnalysis/IrrGlobals.h>
#include <libs/fortD/irregAnalysis/OnHomeTable.h>
#include <libs/fortD/irregAnalysis/ValDecomp.h>

/*------------------ GLOBAL DECLARATIONS --------------------*/

EXTERN(CfgNodeId, cfg_node_from_near_ast, (CfgInstance cfg,
					   AST_INDEX node));

/*------------------ LOCAL DECLARATIONS ---------------------*/

EXTERN(Generic, dc_irreg_init,      (FortTree     ft,
				     FortTextTree ftt,
				     Fd_opts      *fd_opts,
				     FortranDInfo *fd));
EXTERN(void,    dc_irreg_analyze, (Generic irr));
EXTERN(const char *,  ValDecomp_get_upper_bound, (Generic irr, SNODE *sp));
EXTERN(const char *,  ValDecomp_get_loc2glob_name, (Generic hidden_vd));
EXTERN(Generic,       ValDecomp_get_hidden_vd, (Generic irr, SNODE *sp));
EXTERN(void,    dc_irreg_partition, (Generic irr));
EXTERN(void,    dc_irreg_lhs,       (Generic irr, AST_INDEX lhs_node));
EXTERN(void,    dc_irreg_msgs,      (Generic irr, AST_INDEX loop,
				     int num_subs, AST_INDEX *subs));
EXTERN(void,    dc_irreg,           (Generic irr,
				     AST_INDEX irr_decls_node));
EXTERN(void,    dc_irreg_cleanup,   (Generic irr));
EXTERN(AST_INDEX, get_home_node,    (Generic irr, AST_INDEX lhs));
EXTERN(Boolean,   query_irreg_ref,  (Generic irr, AST_INDEX node));
EXTERN(Boolean,   needs_reg_comm,   (Generic irr, AST_INDEX node));
EXTERN(Boolean,   is_executor_loop, (Generic irr, AST_INDEX node));
EXTERN(Boolean,   directive_dominates_ref, (AST_INDEX dir_node,
					    AST_INDEX ref_node));


/*------------------ LOCAL VARIABLES ---------------------*/

// 12/3/93 RvH: Should get rid of this as soon as possible ...
// These should be used only for directive_dominates_ref(),
// which should be replaced by a saner scheme by Dejan
static CfgInstance global_cfg      = NULL;
//static Boolean     global_do_irreg = true;


/**********************************************************************
 * dc_irreg_init()    Initialize irregular part of compiler
 *                    This should be called before any other
 *                    dc_irreg_*() routines
 */
Generic
dc_irreg_init(FortTree     ft,
	      FortTextTree ftt,
	      Fd_opts      *fd_opts,
	      FortranDInfo *fd)
{
  IrrGlobals *di = new IrrGlobals(ft, ftt, fd_opts, fd);

  // 12/3/93 RvH: Should go away when Dejan is done
  global_cfg = di->getCfg();

  return Generic(di);
}


/**********************************************************************
 * dc_irreg_analyze() 
 */
void
dc_irreg_analyze(Generic irr)
{
  IrrGlobals *di = (IrrGlobals *) irr;

  di->collect();
  if (di->getDo_irreg())
  {
    di->analyze();
    if ((di->getFlags())[Code_before_reg])
    {
      di->gen_code();
    }
  }

  // 12/3/93 RvH: Should go away when Dejan is done
  //global_do_irreg = di->getDo_irreg();
  //global_cfg = global_do_irreg ? di->getCfg() : NULL;
}


/**********************************************************************
 * ValDecomp_get_upper_bound()  Given an irregular decomposition,
 *                              compute an upper loop bound.
 */
const char *
ValDecomp_get_upper_bound(Generic irr, SNODE *sp)
{
  IrrGlobals    *di  = (IrrGlobals *) irr;
  ValDecompInfo *vdi = (ValDecompInfo *) di->getHidden_ValDecompInfo();
  ValDecomp     *vd  = vdi->snode2ValDecomp(sp);
  const char    *ub_name;

  ub_name = (vd) ? vd->getCnt_name() : NULL;

  return ub_name;
}


/**********************************************************************
 * ValDecomp_get_hidden_vd()  
 */
Generic
ValDecomp_get_hidden_vd(Generic irr, SNODE *sp)
{
  IrrGlobals    *di  = (IrrGlobals *) irr;
  ValDecompInfo *vdi = (ValDecompInfo *) di->getHidden_ValDecompInfo();
  Generic        hidden_vd = (Generic) vdi->snode2ValDecomp(sp);
  
  return hidden_vd;
}


/**********************************************************************
 * ValDecomp_get_loc2glob_name()  Given an irregular decomposition,
 *                                return the loc2glob mapping
 */
const char *
ValDecomp_get_loc2glob_name(Generic hidden_vd)
{
  const char *loc2glob_name;
  ValDecomp  *vd;

  vd            = (ValDecomp *) hidden_vd;
  loc2glob_name = vd->getLoc2glob_name();

  return loc2glob_name;
}


/**********************************************************************
 * dc_irreg_partition()   Partition irregular computations
 */
void
dc_irreg_partition(Generic irr)
{
}


/**********************************************************************
 * dc_irreg_lhs()    Get & store lhs requiring runtime processing
 *                   Called during partitioning phase.
 */
void
dc_irreg_lhs(Generic irr, AST_INDEX ref_node)
{
  IrrGlobals *di = (IrrGlobals *) irr;

  (void) di->query_irreg_ref(ref_node);
}


/**********************************************************************
 * dc_irreg_msgs()    Get & store rhs refs requiring runtime processing
 *                    Called during message generation phase.
 */
void
dc_irreg_msgs(Generic   irr,   
	      AST_INDEX loop,  // loop, or 1st stmt of statement group
	      int       num_subs,
	      AST_INDEX *subs)
{
  IrrGlobals *di = (IrrGlobals *) irr;

  for (int i = 0; i < num_subs; i++) {
    (void) di->query_irreg_ref(subs[i]);
  }
}


/**********************************************************************
 * dc_irreg()   Driver routine for irregular communications
 */
void
dc_irreg(Generic irr, AST_INDEX irr_decls_node)
{
  IrrGlobals *di = (IrrGlobals *) irr;

  if (di->getDo_irreg())
  {
    //di->analyze();
    if (!di->getFlags()[Code_before_reg])
    {
      di->gen_code();
    }
  }
}


/**********************************************************************
 * dc_irreg_cleanup()   Free storage
 */
void
dc_irreg_cleanup(Generic irr)
{
  IrrGlobals *di = (IrrGlobals *) irr;

  di->do_mall();

  delete di;
}


/**********************************************************************
 * get_home_node()  Return lhs corresponding to enclosing ON_HOME
 *                 directive, or original lhs.
 */
AST_INDEX
get_home_node(Generic irr, AST_INDEX lhs)
{
  IrrGlobals  *di            = (IrrGlobals *) irr;
  OnHomeTable *on_home_table = di->getOn_home_table();
  AST_INDEX   home_node      = on_home_table->get_home_node(lhs);

  return home_node;
}


/**********************************************************************
 * query_irreg_ref()  Determines whether a subscript is irregular.
 *               This implies, for example, that communication is
 *               taken care of by the regular part of the compiler.
 */
Boolean
query_irreg_ref(Generic irr, AST_INDEX node)
{
  IrrGlobals  *di = (IrrGlobals *) irr;
  Boolean     is_irregular = di->query_irreg_ref(node);

  return is_irregular;
}


/**********************************************************************
 * needs_reg_comm()  Does <node> (potentially) need any communication
 *                   generated by the regular compiler ?
 */
Boolean
needs_reg_comm(Generic irr, AST_INDEX node)
{
  IrrGlobals  *di = (IrrGlobals *) irr;
  Boolean     result = di->needs_reg_comm(node);

  return result;
}


/**********************************************************************
 * is_executor_loop()
 */
Boolean
is_executor_loop(Generic irr, AST_INDEX node)
{
  IrrGlobals    *di = (IrrGlobals *) irr;
  const AST_Set *exec_loops = di->getExec_loops();
  Boolean       result      = exec_loops->query_entry(node);

  return result;
}


/**********************************************************************
 * directive_dominates_ref()
 * 12/3/93 RvH: Should go away when Dejan is done
 */
Boolean
directive_dominates_ref(AST_INDEX dir_node, AST_INDEX ref_node)
{
  CfgNodeId dir_cn, ref_cn;
  DomTree   predom;
  Boolean   dom;

  if (global_cfg && (dir_node != AST_NIL))
  {
    dir_cn = cfg_node_from_near_ast(global_cfg, dir_node);
    ref_cn = cfg_node_from_near_ast(global_cfg, ref_node);
    predom = cfg_get_predom(global_cfg);
    dom    = dom_is_dom(predom, dir_cn, ref_cn);
  }
  else
  {
    //if (global_do_irreg)
    //{
    //  cerr << "WARNING directive_dominates_ref(): " <<
	//"global_cfg undefined; dir_node = " << dir_node;
      //if (is_comment(dir_node))
      //{
	//cout << ", text = \"" <<
	//  gen_get_text(gen_COMMENT_get_text(dir_node)) << "\"";
      //}
      //cout << ", ref_node = " << ref_node << "\".\n";
    //}

    dom = true;
  }

  return dom;
}
