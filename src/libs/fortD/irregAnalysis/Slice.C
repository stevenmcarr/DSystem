/* $Id: Slice.C,v 1.15 1997/06/24 17:38:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*********************************************************************
 * class Slice member functions
 */

/**********************************************************************
 * Revision History:
 * $Log: Slice.C,v $
 * Revision 1.15  1997/06/24 17:38:57  carr
 * Support 64-bit Pointers
 *
 * Revision 1.14  1997/03/11  14:28:34  carr
 * newly checked in as revision 1.14
 *
Revision 1.14  94/03/21  13:53:38  patton
fixed comment problem

Revision 1.13  94/02/27  20:15:00  reinhard
Added value-based distributions.
Make CC happy.
See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.

 * Revision 1.20  1994/02/27  19:44:41  reinhard
 * Tweaks to make CC happy.
 *
 * Revision 1.19  1994/01/18  19:51:04  reinhard
 * Updated include paths according to Makefile change.
 * Handle value-based distributions.
 *
 * Revision 1.18  1993/10/06  18:27:33  reinhard
 * Made exec's based on Slices instead of S_desc's.
 *
 * Revision 1.17  1993/10/04  17:27:41  reinhard
 * Catch aux ind vars in catSubscript().
 *
 * Revision 1.16  1993/10/04  15:38:58  reinhard
 * Recognize aux ind var's as regular subscripts, other robustifications.
 *
 * Revision 1.15  1993/09/25  23:02:39  reinhard
 * Cleaned counter generation.
 *
 * Revision 1.14  1993/09/25  15:37:02  reinhard
 * Extended code generation.
 *
 */

#include <assert.h>
#include <iostream.h>
#include <string.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>
#include <libs/moduleAnalysis/valNum/Val.h>
#include <libs/frontEnd/ast/astlist.h>
#include <include/frontEnd/astnode.h>
#include <include/frontEnd/astrec.h>
#include <libs/fortD/irregAnalysis/Exec.h>
#include <libs/fortD/irregAnalysis/Inspec.h>
#include <libs/fortD/irregAnalysis/IrrGlobals.h>
#include <libs/fortD/irregAnalysis/IrrSymTab.h>
#include <libs/fortD/irregAnalysis/S_desc.h>
#include <libs/fortD/irregAnalysis/Slice.h>
#include <libs/fortD/irregAnalysis/analyse.h>
#include <libs/fortD/irregAnalysis/ValDecomp.h>

/*------------------- GLOBAL DECLARATIONS -------------------*/

EXTERN(AST_INDEX, assert_father_is_subscript, (AST_INDEX node));
EXTERN(AST_INDEX, begin_ast_stmt,       (AST_INDEX node));
EXTERN(void,      cat_CoVarPairs,       (CfgInstance  cfg,
					 FortTextTree ftt,
					 CoVarPair    *rv,
					 AST_INDEX    node,
					 int          level,
					 ostrstream   &buf));
EXTERN(void,      cat_CoVarPairs_field, (CfgInstance  cfg,
					 Values       val,
					 FortTextTree ftt,
					 CoVarPair    *rv,
					 AST_INDEX    node,
					 ValNumber    vn,
					 int          level,
					 int          max_level,
					 ostrstream   &buf,
					 AST_INDEX ind_node = AST_NIL));
EXTERN(void,      cat_loop_triplet,  (AST_INDEX loop_node,
				      ostrstream &buf));
EXTERN(AST_INDEX, cfg_node_to_predom_ast, (CfgInstance cfg,
					   CfgNodeId   cn));
EXTERN(AST_INDEX, copy_loop_nest,    (CfgInstance cfg,
				      AST_INDEX subsc_node,
				      AST_INDEX limit_node,
				      AST_INDEX &body_node,
				      Boolean &need_aux));
EXTERN(AST_INDEX, copy_loop_nest_simple, (CfgInstance cfg,
					  AST_INDEX subsc_node,
					  AST_INDEX limit_node,
					  AST_INDEX &body_node));
EXTERN(int,       count_nest_iters, (CfgInstance cfg,
				     AST_INDEX   subsc_node,
				     AST_INDEX   lim_node));
EXTERN(AST_INDEX, fd_ssa_slice, (CfgInstance cfg, AST_INDEX start_node,
				 AST_INDEX stop_node));
EXTERN(CfgNodeId, fd_ssa_get_unique_def_cn, (CfgInstance     cfg,
					     AST_INDEX start_node));
EXTERN(const char *, fd_ssa_get_unique_def_name, (CfgInstance     cfg,
						  AST_INDEX start_node));
EXTERN(AST_INDEX, find_enclosing_DO, (AST_INDEX node));
EXTERN(AST_INDEX, genLongComment,    (const char *cmt_str));
EXTERN(AST_INDEX, level2node, (Values val, CfgInstance cfg,
			       AST_INDEX start_node, int lim_level));
EXTERN(const char *, loop_gen_ivar,        (AST_INDEX node));
EXTERN(const char *, node2name,            (AST_INDEX expr_node));
EXTERN(char *,       prefix_type,         (int type, const char *str));
EXTERN(AST_INDEX, pt_loop_gen_cnt_node, (AST_INDEX loop_node));
EXTERN(Boolean,   val_is_irreg4,   (Values    val,
                                   ValNumber vn,
                                   Boolean   &ivar_flag,
                                   int       &limit_level));
EXTERN(Boolean,   val_is_irreg5,   (Values    val,
                                   ValNumber vn,
                                   Boolean   &ivar_flag,
                                   int       &limit_level,
                                   CoVarPair *&rv));


/**********************************************************************
 * Constructor
 */
Slice::Slice()
: aux_flag       (false),
  cnt_flag       (false),
  di             (NULL),
  dim            (0),
  exec           (NULL),
  glob_size_node (AST_NIL),
  insp           (NULL),
  ivar_flag      (false),
  limit_level    (-1),
  limit_node     (AST_NIL),
  loc_size_node  (AST_NIL),
  new_subsc_size (0),
  ref            (NULL),
  rv             (NULL),
  stmts_node     (AST_NIL),
  str            (NULL),
  subsc_name     (NULL),
  subsc_node     (AST_NIL),
  target_node    (AST_NIL),
  ve             (NULL),
  vn             (VAL_NIL),
  xpnd_flag      (false)
{
}


/**********************************************************************
 * Destructor
 * No deallocation of AST's yet
 */
Slice::~Slice()
{
  val_dep_free(rv);
}


/**********************************************************************
 * getTarget_node()  Retrieve target location for inspector.
 */
AST_INDEX
Slice::getTarget_node() const
{
  //AST_INDEX target_node = ref->getTarget_node();

  return target_node;
}


/**********************************************************************
 * get_unique_def_cn()  
 */
CfgNodeId
Slice::get_unique_def_cn() const
{
  CfgNodeId   def_cn;

  def_cn = fd_ssa_get_unique_def_cn(di->getCfg(), subsc_node);

  return def_cn;
}


/**********************************************************************
 * get_unique_def_name()  
 */
const char *
Slice::get_unique_def_name() const
{
  const char *def_name;

  def_name = fd_ssa_get_unique_def_name(di->getCfg(), subsc_node);

  return def_name;
}


/**********************************************************************
 * init()  Initialization
 *         This is pulled out of the constructor so that we can
 *         easiliy declare arrays of Slices
 *
 *
 *                     *** ASSUMPTIONS ***
 *
 * ... for now about subscripts:
 * (A1) Subscripts <subsc> are scalars (or constants, in which
 *      case no slice will be generated); 
 *      NOT something like x(ia(i)), instead
 *      {n1 = ia(i); x(n1) =...}
 * (A2) <subsc> does not change it's value between different uses
 *      within the same iteration;
 *      NOT {n1 = n1(i); x(n1) = ...; n1 = n1+10; y(n1) = ...}
 */
void
Slice::init(S_desc *my_ref, int my_dim, AST_INDEX my_subsc_node, 
	    IrrGlobals *my_di)
{
  subsc_node = assert_father_is_subscript(my_subsc_node);
  di         = my_di;
  ref        = my_ref;
  dim        = my_dim;
  glob_size_node = di->getSt()->get_size_node(ref->getArray_name());
  init_subsc();
  init_new_subsc();
  genNew_subsc_size();
}


/**********************************************************************
 * init_subsc()  Initialize subscript.    
 *               This is called from init(), tree_changed()
 */
void
Slice::init_subsc()
{
  vn         = cfgval_get_val(di->getCfg(), subsc_node);
  str        = di->Ast2Str(subsc_node);
  subsc_name = node2name(subsc_node);
}


/**********************************************************************
 * init_new_subsc()  Determine whether we have to expand subscript etc.
 *                   This is called from init()
 * from val.h:
 *
 * typedef struct {
 *   int       coeff; // coefficient of loop index variable
 *   ValNumber sym;   // non-inductive variant symbolics at same level
 * } CoVarPair;
 * 
 * val_dep_parse():
 *  Return vector of CoVarPair objects.  Return_value[0].coeff
 *  gives the constant added part of the subscript, 
 *  return_value[0].sym gives the loop-invariant symbolic 
 *  part.  Where there is no symbolic part, the sym field is
 *  given as VAL_ZERO.
 *  Length of pair vector is max(level, variance level of v),
 *  where d is the depth of the deepest containing loop.
 */
void
Slice::init_new_subsc()
{
  CfgInstance cfg     = di->getCfg();
  Values      val     = cfgval_get_values(cfg);
  ValType     vn_type = val_get_val_type(val, vn);

  // Determine whether we have to expand subscript

  xpnd_flag = val_is_irreg5(val, vn, ivar_flag, limit_level, rv);

  if (xpnd_flag)
  {
    limit_level = find_limit_level(val, vn);
    if (limit_level >= 0)
    {
      limit_node = level2node(val, cfg, subsc_node, limit_level);
    }
  }
}


/**********************************************************************
 * genNew_subsc_size()
 *               This is called from init(), tree_changed()
 */
void
Slice::genNew_subsc_size()
{
  if (limit_node != AST_NIL)
  {
    new_subsc_size = count_nest_iters(di->getCfg(), subsc_node, limit_node);
  }

  // We need a counting slice iff we don't know the # of iterations
  cnt_flag = (Boolean) (xpnd_flag && (new_subsc_size == UNKNOWN_SIZE));

  // We need an aux ind var iff the the subscript is not in a single,
  // normal loop.
  aux_flag = (Boolean) (xpnd_flag
			&& !((limit_node == begin_ast_stmt(subsc_node))
			     && is_do(limit_node)
			     && pt_loop_is_normal(limit_node)));
}


/**********************************************************************
 * tree_changed()  Update after AST changes.
 *
 * This may be called, for example, when the regular compiler has
 * altered the AST after this slice has been initialized.
 */
void
Slice::tree_changed()
{
  ValDecomp *vd;

  subsc_node = ref->get_subsc_node(dim);
  init_subsc();
  genNew_subsc_size();

  if (ref->getDistrib_irreg())
  {
    vd            = (ValDecomp*) ref->getVd();
    loc_size_node = pt_gen_ident((char*)vd->getCnt_name());
  }
  else
  {
    loc_size_node =  di->getSt()->get_size_node(ref->getArray_name());
  }
}


/**********************************************************************
 * find_limit_level(val, vn)
 *
 * Find the loop level of the limit node until which we trace back the
 * slice for this subscript.
 * The limit node is the outermost "significant loop," ie. the
 * outermost loop that contributes to a subscript.
 *
 * Example: for the slice on "n1" in the following code
 *
 * do i
 *   do j
 *     n1 = nde(i,j)
 *     x(n1) = ...
 *
 * the limit node is the "do i".
 *
 * 6/5/93 RvH: Current heuristic:
 * Assume that in an irregular reference <array>(<sub>),
 * <sub> is defined in a separate, preceeding stmt and consists of
 * an array lookup, indexed by loop indices of enclosing loops.
 */
int  
Slice::find_limit_level(Values val, ValNumber vn)
{
  ValNumber subs_vn;
  ValNumber sub_vn;
  Boolean   sub_ivar_flag = false;
  int       sub_level     = -1;
  int       lim_level     = -1;
  ValType   vn_type       = val_get_val_type(val, vn);

  
  // Is the current subscript an indirection array lookup ?
  if (xpnd_flag)
  {
    if (vn_type == VAL_ARRAY)
    {
      // Get the subsubscripts (ie, subscripts of indirection array)
      subs_vn = val_get_subs(val, vn);
      
      // Iterate through the subsubscripts
      for (ValueIterator iter(val, subs_vn);
	   (sub_vn = iter()) != VAL_NIL;)
      {
        (void) val_is_irreg4(val, sub_vn, sub_ivar_flag, sub_level);

	// Is the current subsubscript an induction variable ?
        // Does the current subsubscript look at a more outer loop
        // then the subsubscripts encountered so far ?
	if ((sub_ivar_flag)
            && ((lim_level < 0) || (sub_level < lim_level)))
        {
          lim_level = sub_level;
        }
      }
    }
    else
    {
      lim_level = val_get_level(val, vn);
      //cerr << "WARNING: Slice::find_limit_level(): " <<
      //  "cannot find limit level for subscript \"" << subsc_name <<
      //    "\" in reference \"" << ref->getStr() <<
      //      "\"; subsc_node = " << subsc_node << ".\n";
    }

    if (lim_level < 0)
    {
      lim_level = 1;
      cerr << "WARNING: Slice::find_limit_level(): " <<
        "cannot find limit level for subscript \"" << subsc_name <<
          "\" in reference \"" << ref->getStr() <<
            "\"; subsc_node = " << subsc_node << ".\n";
    }
  }
  else
  {
    cerr << "WARNING: Slice::find_limit_level(): " <<
      "subscript \"" << subsc_name << "\" in reference \"" <<
	ref->getName() << "\" should not be expanded; subsc_node = " <<
	  subsc_node << ".\n";
  }

  return lim_level;
}


/**********************************************************************
 * getNew_subsc_loc_name()
 */
const char*
Slice::getNew_subsc_loc_name() const
{
  return ve->getNew_subsc_loc_name();
}


/**********************************************************************
 * getNew_subsc_glob_name()
 */
const char*
Slice::getNew_subsc_glob_name() const
{
  return ve->getNew_subsc_glob_name();
}


/**********************************************************************
 * getLoc_cnt_name()
 */
const char*
Slice::getLoc_cnt_name() const
{
  return ve->getLoc_cnt_name();
}


/**********************************************************************
 * getSched_name()
 */
const char*
Slice::getSched_name() const
{
  return ve->getSched_name();
}


/**********************************************************************
 * getTab_name()
 */
const char*
Slice::getTab_name() const
{
  return ve->getTab_name();
}


/**********************************************************************
 * getInd_name()
 */
const char *
Slice::getInd_name() const
{
  return aux_flag ? ve->getAux_cnt_name() : loop_gen_ivar(limit_node);
}

/**********************************************************************
 * getCnt_name()
 *               //This is called from init(), genStmts_node()
 *
 * Eventually, when the counting/collecting slices work right,
 * "i$" in executor should be substituted w/ i$cnt as well.
 * Currently, this cannot be done since different slices get
 * different counters, even if they have the same counting slice.
 *
 * Note that we might need an aux ind var even for a fixed # of
 * iterations.
 */
const char *
Slice::getCnt_name() const
{
  const char *cnt_name;

  if (cnt_flag)
  {
    cnt_name = ve->getAux_cnt_name();
  }
  else
  {
    cerr << "WARNING: getCnt_name(): "
      << "Should use fixed new_subsc_size = " << new_subsc_size
	<< " instead.\n";
    cnt_name = NULL;
  }

  return cnt_name;
}


/**********************************************************************
 * genInspec()
 */
void
Slice::genInspec()
{
  CfgNodeId target_cn;   // Where the inspector goes

  // Expand the subscript of this slice ?
  if (xpnd_flag)
  {
    target_cn = ve->getTarget_cn();
    if (target_cn == CFG_NIL)
    {
      cerr << "WARNING: Slice::genInspec(): target_cn = CFG_NIL; "
        << " ref \"" << ref->getStr() << "\".\n";
    }
    else
    {
      target_node = cfg_node_to_predom_ast(di->getCfg(), target_cn);
      
      // Generate an inspector for this slice
      insp = di->getInsps()->gen_entry_by_AST(target_node);
      
      // Add myself to my inspector
      insp->add_Slice(this);
    }
  }
}


/**********************************************************************
 * genExec()
 */
void
Slice::genExec()
{
  if (limit_node != AST_NIL)
  {
    // Generate an executor for this slice
    exec = di->getExecs()->gen_entry_by_AST(limit_node);
    
    // Add myself to my executor
    exec->add_Slice(this);
  }
}


/**********************************************************************
 * catSubscript()  Construct subscript string.
 *                 "n1$arr(1:100)"   if <gen_high_level> == true,
 *                 "x$sched, x$loc_cnt [= "n1$arr(1:100)""   o.w.
 */
void
Slice::catSubscript(ostrstream &buf, int level)
{
  Boolean gen_high_level = di->getFlags()[Gen_high_level];

  // Will the subscript be expanded ?
  if (xpnd_flag)
  {
    if (gen_high_level)
    {
      buf << subsc_name;
    }
    else
    {
      // Generate "x$sched, x$loc_cnt [= "
      //buf << getSched_name() << ", " << getLoc_cnt_name() << " [= " <<
      buf << getNew_subsc_loc_name();
    }

    // Generate "j$arr(1:j$cnt)"
    buf << "(1:";      
    if (new_subsc_size == UNKNOWN_SIZE) {   // Size of trace known ?
      buf << getCnt_name();                      // "j$cnt"
    } else {
      buf << new_subsc_size;                // "1000"
    }
    buf << ")";
    
    //if (!gen_high_level)
    //{
    //  buf << "]";
    //}
  }
  else
  {
    // Is the subscript an induction variable ?
    if (ivar_flag) {
      // Generate "1:100"
      //cat_loop_triplet(limit_node, buf);
      if (rv)
      {
	if (level < 0) {
	  level = limit_level;
	}
	cat_CoVarPairs(di->getCfg(), di->getFtt(), rv, subsc_node,
		       level, buf);
      }
      else
      {
	buf << "AUX VAR";
      }
    }
    else
    {
      // Generate "5"
      buf << str;
    }
  }
}


/**********************************************************************
 * update_subsc()  Plug in new subsubscript for irregular subscript.
 */
void
Slice::update_subsc(const char *ivar_name)
{
  AST_INDEX old_subsc_node;

  if (xpnd_flag)       // Expand Subscript ?
  {
    if (!getNew_subsc_loc_name())
    {
      cerr << "WARNING: Slice::update_subsc(): "
        << "don't have new_subsc_loc_name for \"" << ref->getStr()
          << "\".\n";
    }
    else
    {
      old_subsc_node = subsc_node;

      // Generate new subscript for executor
      subsc_node = gen_SUBSCRIPT(pt_gen_ident((char*)getNew_subsc_loc_name()),
                                 list_create(pt_gen_ident((char*)ivar_name)));

      pt_tree_replace(old_subsc_node, subsc_node);
      // Insert new subscript before old one
      //(void) list_insert_before(old_subsc_node, subsc_node);

      // Remove old subscript
      //(void) list_remove_node(old_subsc_node);
      //tree_free(old_subsc_node);
    }
  }
}


/**********************************************************************
 * gen_localize()
 *
 * Generate two versions,
 *
 * 1. For regular distributions:
 *      call reglocalize(1, x$onsize*n$p, x$sched, j$glob(1), j$loc(1),
 *                       j$size, x$offsize, x$onsize, 1)"
 *
 * 2. For irregular distributions:
 *      x$tab = build_translation_table (1, atomd$loc2glob(1), atomd$my)
 *      call localize(x$tab, x$sched, j$glob(1), j$loc(1),
 *                       j$size, x$offsize, x$onsize, 1)"
 *      call free_table (x$tab)
 */
AST_INDEX
Slice::gen_localize()
{
  AST_INDEX  stmt_node, node, arg_node;
  ostrstream cmt_buf;
  ValDecomp  *vd;
  Boolean    is_reg_dist;     // True iff regular distribution
  ValDecompInfo *vdi = (ValDecompInfo *) di->getHidden_ValDecompInfo();

  // Generate comment "--<< REG_LOCALIZE(DISTRIB_OF(x), j$arr, j$cnt,
  // x$loc_cnt, x$sched) >>--"
  //cmt_buf << "--<< REG_LOCALIZE (DISTRIB_OF (" << ref->getArray_name()
  //  << "), " << new_subsc_name << ", " << getCnt_name() << ", "
  //    << getLoc_cnt_name() << ", " << getSched_name() << ") >>--" << ends;
  //cmt_str = cmt_buf.str();
  //node    = genLongComment(cmt_str);
  //delete cmt_str;

  //is_reg_dist = false;
  is_reg_dist = vdi->is_reg_dist(ref->getArray_node());

  if (is_reg_dist)
  {
    // Generate "call reglocalize(1, x$onsize*n$p, x$sched,
    //           . j$glob(1), j$loc(1), j$size, x$offsize, x$onsize, 1)"
    arg_node = pt_gen_int(1);
    node     = list_create(arg_node);                // "1" (= BLOCK)
    arg_node = tree_copy(glob_size_node);
    node     = list_insert_last(node, arg_node);     // "x$onsize*n$p"
    arg_node = pt_gen_ident((char*)getSched_name());
    node     = list_insert_last(node, arg_node);     // "x$sched"
    arg_node = list_create(pt_gen_int(1));
    arg_node = gen_SUBSCRIPT(pt_gen_ident((char*)getNew_subsc_glob_name()),
			     arg_node);
    node     = list_insert_last(node, arg_node);     // "j$glob(1)"
    arg_node = list_create(pt_gen_int(1));
    arg_node = gen_SUBSCRIPT(pt_gen_ident((char*)getNew_subsc_loc_name()),
			     arg_node);
    node     = list_insert_last(node, arg_node);     // "j$loc(1)"
    arg_node = cnt_flag ? pt_gen_ident((char*)getCnt_name())
      : pt_gen_int(new_subsc_size);
    node     = list_insert_last(node, arg_node);     // "j$size"
    arg_node = pt_gen_ident((char*)getLoc_cnt_name());
    node     = list_insert_last(node, arg_node);     // "x$offsize"
    arg_node = tree_copy(loc_size_node);
    node     = list_insert_last(node, arg_node);     // "x$onsize"
    arg_node = pt_gen_int(1);
    node     = list_insert_last(node, arg_node);     // "1"
    node     = pt_gen_call("reglocalize", node);
    stmt_node = list_create(node);
  }
  else
  {
    vd = vdi->name2ValDecomp(ref->getArray_name());    

    // Generate "x$tab = build_translation_table (1, atomd$loc2glob(1), atomd$my)"
    // sp = fd->GetEntry(name)
    arg_node = pt_gen_int(1);                        // "1" (= "type")
    node     = list_create(arg_node);
    arg_node = pt_gen_ident((char*)vd->getLoc2glob_name());
    arg_node = gen_SUBSCRIPT(arg_node, list_create(pt_gen_int(1)));
    node     = list_insert_last(node, arg_node);     // "atomd$loc2glob"
    arg_node = pt_gen_ident((char*)vd->getCnt_name());
    node     = list_insert_last(node, arg_node);     // "atomd$my"
    node     = pt_gen_invoke("build_translation_table", node);
    arg_node = pt_gen_ident((char*)getTab_name());   // "x$tab"
    node     = gen_ASSIGNMENT(AST_NIL, arg_node, node);
    stmt_node = list_create(node);
    

    // Generate "call localize(x$tab, x$sched,
    //           . j$glob(1), j$loc(1), j$size, x$offsize, x$onsize, 1)"
    arg_node = pt_gen_ident((char*)getTab_name());   // "x$tab"
    node     = list_create(arg_node);
    arg_node = pt_gen_ident((char*)getSched_name()); // "x$sched"
    node     = list_insert_last(node, arg_node);    
    arg_node = list_create(pt_gen_int(1));
    arg_node = gen_SUBSCRIPT(pt_gen_ident((char*)getNew_subsc_glob_name()),
			     arg_node);
    node     = list_insert_last(node, arg_node);     // "j$glob(1)"
    arg_node = list_create(pt_gen_int(1));
    arg_node = gen_SUBSCRIPT(pt_gen_ident((char*)getNew_subsc_loc_name()),
			     arg_node);
    node     = list_insert_last(node, arg_node);     // "j$loc(1)"
    arg_node = cnt_flag ? pt_gen_ident((char*)getCnt_name())
      : pt_gen_int(new_subsc_size);
    node     = list_insert_last(node, arg_node);     // "j$size"
    arg_node = pt_gen_ident((char*)getLoc_cnt_name());
    node     = list_insert_last(node, arg_node);     // "x$offsize"
    arg_node = tree_copy(loc_size_node);
    node     = list_insert_last(node, arg_node);     // "x$onsize"
    arg_node = pt_gen_int(1);
    node     = list_insert_last(node, arg_node);     // "1"
    node     = pt_gen_call("localize", node);
    stmt_node = list_insert_last(stmt_node, node);


    // Generate "call free_table (x$tab)"
    arg_node = pt_gen_ident((char*)getTab_name());   // "x$tab"
    node     = list_create(arg_node);
    node     = pt_gen_call("free_table", node);
    stmt_node = list_insert_last(stmt_node, node);
  }
  return stmt_node;
}


/**********************************************************************
 * gen_extend()
 */
AST_INDEX
Slice::gen_extend()
{
  ostrstream cmt_buf;
  int        type;
  AST_INDEX  arg_node;
  AST_INDEX  node = AST_NIL;
  const char *name, *function_name;

  name = ref->getArray_name();

  // Generate comment "--<< EXTEND (x, DIM = 1, SIZE = x$loc_cnt) >>--"
  //cmt_buf << "--<< EXTEND (" << name << ", DIM = " <<
  //  dim + 1 << ", SIZE = " << getLoc_cnt_name() << ") >>--" << ends;
  //cmt_str = cmt_buf.str();
  //node    = genLongComment(cmt_str);
  //delete cmt_str;

  // For now: handle only 1-d arrays
  if (dim > 0)
  {
    cerr << "WARNING: Slice::gen_extend(): Cannot handle dynamic "
      << dim+1 << "-d arrays; reference \"" << ref->getStr()
	<< "\".\n";
  }
  else
  {
    // Generate "call fresize(x, x$onsize + x$offsize)"
    node     = tree_copy(loc_size_node);       // "x$onsize"
    arg_node = pt_gen_ident((char*) getLoc_cnt_name()); // "x$offsize"
    node     = pt_gen_add(node, arg_node);     // "x$onsize + x$offsize"
    node     = list_create(node);
    arg_node = pt_gen_ident((char*) name); // "x"
    node     = list_insert_first(node, arg_node);
    type     = di->getSt()->get_type(name);
    function_name = prefix_type(type, "resize");
    node     = pt_gen_call((char*) function_name, node);

    // Note that we now need dynamic arrays
    di->setNeed_mall(type);
  }
    
  return node;
}
