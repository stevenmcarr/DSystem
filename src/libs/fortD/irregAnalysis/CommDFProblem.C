/* $Id: CommDFProblem.C,v 1.8 1997/03/11 14:28:25 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/**********************************************************************
 * Communication data flow analysis.
 */

/**********************************************************************
 * Revision History:
 * $Log: CommDFProblem.C,v $
 * Revision 1.8  1997/03/11 14:28:25  carr
 * newly checked in as revision 1.8
 *
Revision 1.8  94/03/21  13:21:32  patton
fixed comment problem

Revision 1.7  94/02/27  20:14:16  reinhard
Added value-based distributions.
Make CC happy.
See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.

 * Revision 1.16  1994/02/27  19:38:28  reinhard
 * Tweaks to make CC happy.
 *
 * Revision 1.15  1994/01/18  19:44:05  reinhard
 * Updated include paths according to Makefile change.
 * Handle value-based distributions.
 *
 */

#include <assert.h>
#include <iostream.h>

#include <libs/moduleAnalysis/cfgValNum/cfgval.h>
#include <libs/support/sets/VectorSet.h>
#include <libs/fortD/irregAnalysis/CommDFProblem.h>
#include <libs/fortD/irregAnalysis/CommDFUniv.h>
#include <libs/fortD/irregAnalysis/IrrGlobals.h>
#include <libs/fortD/irregAnalysis/IrrSymTab.h>
#include <libs/fortD/irregAnalysis/S_desc.h>
#include <libs/fortD/irregAnalysis/Slice.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>

/*------------------- GLOBAL DECLARATIONS -------------------*/

EXTERN(void,      attach_cfg_node,    (CfgInstance cfg,
				       CfgNodeId   cn,
				       AST_INDEX   new_node,
				       Boolean     prepend,
				       Boolean skip_comments = true));
EXTERN(AST_INDEX, cfg_node_to_nearest_ast, (CfgInstance cfg,
					    CfgNodeId   cn,
					    Boolean     postdom));
EXTERN(CfgNodeId, cfg_postdom_succ,   (CfgInstance cfg, CfgNodeId cn));
EXTERN(CfgNodeId, cfg_predom_pred,    (CfgInstance cfg, CfgNodeId cn));
EXTERN(AST_INDEX, genLongComment,     (const char *cmt_str));
EXTERN(void,      gt_gen_comm_stmt,   (const GiveTake &gt,
				       Boolean prepend));
EXTERN(Boolean,   is_reduction_type,  (NODE_TYPE node));
EXTERN(AST_INDEX, mergeStmts,         (AST_INDEX node1,
				       AST_INDEX node2));
EXTERN(char *,    prefix_type,        (int type, const char *str));


/*------------------- LOCAL DECLARATIONS --------------------*/

EXTERN(const VectorSet &, CommDFAnnot_set_access_apply,
       (SetAccessFunc set_access_v, CfgAnnotPtr annot_v));
EXTERN(GiveTakePtr, CommDFAnnot_gt_access_apply,
       (GtAccessFunc gt_access_v, CfgAnnotPtr annot_v));

/*------------------- LOCAL CONSTANT DEFINITIONS ------------*/

const char    *Comm_dir_names[] = { "Send", "Recv", "Send/Recv" };
const char    *Gt_names[]   = { "gather", "scatter",
				"scatter_add", "scatter_mult" };
const Boolean Gt_is_red[]   = { false, false, true, true };
const Boolean Gt_forward[]  = { true, false, false, false };
const int     Gt_init_red[] = { 0, 0, 0, 1 };


/*********************************************************************
 *                                                                   *
 *           Methods of class CommDFProblem                          *
 *                                                                   *
 *********************************************************************/


/**********************************************************************
 * Constructor
 */
CommDFProblem::CommDFProblem(CfgInstance   my_cfg,
			     S_desc_ht     *refs,
			     IrrSymTab     *st,
			     Boolean       my_split_comm,
			     Boolean       my_high_level)
: IntervalDFProblem(my_cfg,
		    (SetAccessApplyFunc) &CommDFAnnot_set_access_apply,
		    (GtAccessApplyFunc)  &CommDFAnnot_gt_access_apply),
  dump_i     (Gt_cnt),
  high_level (my_high_level),
  iter       (new IntervalGraphIter(cfg)),
  red_init_ind_name (NULL),
  refs_keys  (new RefsKeys(refs, cfg, st)),
  split_comm (my_split_comm)
{
}


/**********************************************************************
 * Destructor
 */
CommDFProblem::~CommDFProblem()
{
  delete refs_keys;
  delete iter;
}


/**********************************************************************
 * get_red_init_ind_name()
 */
const char *
CommDFProblem::get_red_init_ind_name()
{
  if (!red_init_ind_name)
  {
    red_init_ind_name = refs_keys->getSt()->gen_fresh_name("$init");
    assert(refs_keys->getSt()->declInt(red_init_ind_name));
  }

  return red_init_ind_name;
}

/**********************************************************************
 * init_annot()  Initialize the annotation of one CFG node.
 */
CfgAnnotPtr
CommDFProblem::init_annot(CfgNodeId node)
{
  CommDFAnnot *annot = new CommDFAnnot(this, node);

  return (CfgAnnotPtr) annot;
}


/**********************************************************************
 * apply_insens()  Apply a given flow insensitive method to a given
 *                 annotation.
 */
void
CommDFProblem::apply_insens(InsensFunc      insens_v,
			    CfgAnnotPtr     me_v,
			    CfgAnnotPtrIter *meet_annots)
{
  CommDFInsensFunc insens = (CommDFInsensFunc) insens_v;
  CommDFAnnotPtr   me     = (CommDFAnnotPtr) me_v;
 
  (me->*insens)(meet_annots);
}
 
 
/**********************************************************************
 * apply_sens()  Apply a given flow sensitive method to a given
 *               annotation.
 */
void
CommDFProblem::apply_sens(SensFunc        sens_v,
			  CfgAnnotPtr     me_v,
			  CfgAnnotPtrIter *meet_annots,
			  CfgAnnotPtr     liftee_v)
{
  CommDFSensFunc sens   = (CommDFSensFunc) sens_v;
  CommDFAnnotPtr me     = (CommDFAnnotPtr) me_v;
  CommDFAnnotPtr liftee = (CommDFAnnotPtr) liftee_v;

  (me->*sens)(meet_annots, liftee);
}


/**********************************************************************
 * forall_annots()  Traverse all annotations and apply <method>.
 *                  Default order is bottom-up, left-to-right
 */
void
CommDFProblem::forall_annots(CommDFAnnotMethod method,
			     TraversalOrder    order) const
{
  CommDFAnnot *ann;                   // Current annotation

  iter_reset(order);
  while ((ann = iter_next()) != NULL) {
    (ann->*method)();
  }
}


/**********************************************************************
 * iter_reset()  Initialize iterator traversing all annotations.
 *               Default order is bottom-up, left-to-right
 */
void
CommDFProblem::iter_reset(TraversalOrder order) const
{
  (*iter).reset(order);
}


/**********************************************************************
 * iter_next()  Retrieve next annotation.
 */
CommDFAnnot *
CommDFProblem::iter_next() const
{
  CfgNodeId   cn = (*iter)();         // Current node
  CommDFAnnot *ann;                   // Current annotation

  //ann = (cn == CFG_NIL) ? NULL : (CommDFAnnot *) getAnnot(cn);
  ann = (cn == CFG_NIL) ? NULL : getAnnot(cn);

  return ann;
}


/**********************************************************************
 * solve()
 */
void
CommDFProblem::solve()
{
  CommDFAnnot *ann;                   // Current annotation

  init_annots();

  iter_reset();
  while ((ann = iter_next()) != NULL) {
    ann->initGATH();
  }
  give_n_take((void *) Gather);

  iter_reset();
  while ((ann = iter_next()) != NULL) {
    ann->initSCATT();
  }
  give_n_take((void *) Scatter);
  give_n_take((void *) Scatter_add);
  give_n_take((void *) Scatter_mult);
}


/**********************************************************************
 * gen_comm()
 */
void
CommDFProblem::gen_comm()
{
  forall_annots(&CommDFAnnot::gen_comm_stmts, PreOrder);
  forall_annots(&CommDFAnnot::insert_comm_stmts, PreOrder);
}


/**********************************************************************
 * dump()  Dump universe and annots
 */
void
CommDFProblem::dump(Gt_index my_dump_i, Boolean all)
{
  refs_keys->dump();    // Data flow universe

  dump_i = my_dump_i;
  dump_annots();        // CFG annotations
  dump_i = Gt_cnt;

  if (all) {
    dump_cfg();
    cfgval_Dump(0, cfg);                  // Values
    tree_print(cfg_get_inst_root(cfg));   // AST
  }
}


/**********************************************************************
 * dump_cfg()  
 */
void
CommDFProblem::dump_cfg()
{
  // Intervals
  tarj_print(cfg_get_intervals(cfg), cfg_start_node(cfg));

  // Note: to extract the cfg topology, do "egrep '(node|label)'"
  cfg_sorted_dump(cfg);                 // Control flow graph
}


/**********************************************************************
 * get_first_GATHER_node()  Find first annotation (in PreOrder) that
 *                          has non-empty GATHER
 */
AST_INDEX
CommDFProblem::get_first_GATHER_node() const
{  
  CommDFAnnot *ann;                // Current annotation
  CfgNodeId   cn;                  // Current node
  int         i;                   // Current GiveTake instance
  AST_INDEX   prepend_node = AST_NIL;
  Boolean     no_comm      = true;

  iter_reset();
  while (no_comm && ((ann = iter_next()) != NULL))
  {
    for (i = 0; no_comm && (i < Gt_cnt); i++)
    {
      no_comm = (Boolean) (ann->getGt((Gt_index) i)->getGEN_entry().isEmpty() &&
	ann->getGt((Gt_index) i)->getGEN_exit().isEmpty());
    }
  }
  
  if (ann == NULL) {
    cerr << "WARNING: CommDFProblem::get_first_GATHER_node(): " <<
      "no GATHER found !\n";
  } else {
    cn = ann->getCn();
    prepend_node = cfg_node_to_nearest_ast(cfg, cn, true);
  }

  return prepend_node;
}


/*********************************************************************
 *                                                                   *
 *           Methods of class CommDFAnnot                            *
 *                                                                   *
 *********************************************************************/


/**********************************************************************
 * Constructor
 */
CommDFAnnot::CommDFAnnot(CommDFProblem *my_problem,
			 CfgNodeId     my_cn)
: CfgAnnot(my_problem, my_cn),
  append_node  (AST_NIL),
  comm_problem (my_problem),
  prepend_node (AST_NIL),
  refs_keys    (my_problem->getRefs_keys()),
  target_node  (AST_NIL)
{
  RefsKeyId   id;
  CfgInstance cfg = problem->getCfg();
  NODE_TYPE   reduction;
  int         i;

  // Compute REF and DEF
  // Loop through children SSA nodes
  for (SsaNodeId sn = ssa_first_cfg_kid(cfg, cn);
       sn != SSA_NIL;
       sn = ssa_next_cfg_kid(cfg, sn))
  {
    id  = refs_keys->SsaNode2Key(sn);     // Map SsaNodeId -> RefsKeyId

    if (id != RefsKeyNil) {
      if (ssa_is_use(cfg, sn))
      {
	REF <<= id;
      }
      else if (ssa_is_def(cfg, sn))
      {
	DEF <<= id;
      }
    }

    // Track definitions of idirection arrays
    if (ssa_node_type(cfg, sn) == SSA_DEF)
    {
      IND = refs_keys->sn2IND(sn);
    }
  }

  //IND = refs_keys->cn2IND(cn);

  // Compute other local flow variables

  // Check whether this is a reduction;
  // recognize +, -, *, / as reductions
  reduction = get_reduction_type();
  switch (reduction) {
  case GEN_BINARY_PLUS:
  case GEN_BINARY_MINUS:
    ADD = DEF;
    break;
  case GEN_BINARY_TIMES:
  case GEN_BINARY_DIVIDE:
    MULT = DEF;
    break;
  }
  RED = ADD | MULT;

  // See comment at RefsKeys::no_duplicates() method
  DEF_NODUP = refs_keys->no_duplicates(DEF);

  // Initialize GiveTake instances
  for (i = 0; i < Gt_cnt; i++)
  {
    gt[(Gt_index) i].init(this, Gt_names[(Gt_index) i]);
  }
}


/**********************************************************************
 * getAll()
 */
VectorSet
CommDFAnnot::getAll() const
{
  return refs_keys->getAll();
}


/**********************************************************************
 * initGATH()
 */
void
CommDFAnnot::initGATH()
{
  VectorSet STEAL_init;

  if (is_root())
  {
    // If this is the CFG root annotation, then kill everything here
    STEAL_init = getAll();
  }
  else
  {
    // Redefining the index set or array contents blocks GATHER's
    STEAL_init = IND | refs_keys->touches(DEF);
  }

  // 6/26/93: If we replace "touches(DEF)" with "affects(DEF)", then
  // internal forwarding does not work, even within a loop iteration.
  // Might have to come up w/ something that forwards internally just
  // up to a certain nesting level (eg, forward only within loop)
  gt[Gather].instantiate
    (true,             // GATHER's are shifted UP
     REF - RED,        // GATHER what is referenced & not reduced
     STEAL_init,
     refs_keys->contains(DEF_NODUP));  // Data come for free when defined locally
}


/**********************************************************************
 * initSCATT()
 */
void
CommDFAnnot::initSCATT()
{
  VectorSet GIVE_init;

  // Gathering or redefining the index set  blocks all SCATTER's 
  const VectorSet &GATH_GEN = refs_keys->touches(gt[Gather].getGEN_entry())
    |  refs_keys->touches(gt[Gather].getGEN_exit());
  const VectorSet &BLOCK    = GATH_GEN | IND;

  if (is_end())
  {
    // 9/27/93 RvH: Should have the set of parameters / commons here
    //GIVE_init = getAll();
  }

  gt[Scatter].instantiate
    (false,            // SCATTER's are shifted DOWN
     DEF - RED,        // SCATTER what is defined & not reduced
     BLOCK | refs_keys->touches(REF) // References block SCATTER's
     | refs_keys->affects(RED),     // Reductions block SCATTER's
     GIVE_init);

  gt[Scatter_add].instantiate
    (false,            // SCATTER_ADD's are shifted DOWN
     ADD,              // SCATTER_ADD what is just added to
     BLOCK | refs_keys->affects(REF -   // References outside of
				ADD),   // ADD's blocks SCATTER_ADD's
     GIVE_init);

  gt[Scatter_mult].instantiate
    (false,            // SCATTER_MULT's are shifted DOWN
     MULT,             // SCATTER_MULT what is just multiplieed to
     BLOCK | refs_keys->affects(REF -   // References outside of
				MULT),  // MULT's blocks SCATTER_MULT's
     GIVE_init);
}


/**********************************************************************
 * get_reduction_type()  Detect reductions.
 *
 * NOTE:
 * Reductions have to be of the form
 *       <ref> = <ref> <op> <exp>,
 * where
 *       <op>  is +, *, etc.,
 * and
 *       <exp> is an expression not containing any reference
 *             to the data array of <ref>.
 *
 * If <exp> is not just a single reference, it has to be parenthesized.
 *
 * EXAMPLES:
 * x(n1(i)) = x(n1(i)) + y(n2(i))             OK
 * x(n1(i)) = x(n1(i)) + x(n2(i))             NOT OK
 * x(n1(i)) = x(n1(i)) + (y(n2(i)) + z(3*i))  OK
 * x(n1(i)) = x(n1(i)) + y(n2(i)) + z(3*i)    NOT OK
 *       
 */
// 4/3/93 RvH: Why does const-ing this function cause a problem w/
// the assignment to REDUCEE ?
NODE_TYPE
CommDFAnnot::get_reduction_type() const
{
  VectorSet   REDUCEE;
  RefsKeyId   lhs_id;
  RefsKey_ptr lhs_key;
  fst_index_t lhs_var;
  AST_INDEX   assignment_node;
  SsaNodeId   rhs_sn;
  AST_INDEX   rhs_node;
  AST_INDEX   left_op_node;
  SsaNodeId   left_op_sn;
  RefsKeyId   left_op_id;
  RefsKeyId   op_id;
  RefsKey_ptr op_key;
  fst_index_t op_var;
  NODE_TYPE   reduction_type;
  CfgInstance cfg          = problem->getCfg();
  Boolean     is_reduction = false;
  NODE_TYPE   rhs_type     = 0;

  REDUCEE = REF & DEF;     // Intersect lhs and rhs refs
  if (!REDUCEE.isEmpty())  // Any refs both on lhs and rhs ?
  {
    VectorSetI iter(&REDUCEE);
    lhs_id = iter.test() ? iter.elem : RefsKeyNil;
    iter++;

    is_reduction = (Boolean)

      // Make sure REDUCEE contains exactly 1 element
      ((lhs_id != RefsKeyNil)
       && !iter.test()

       // Get assignment AST_INDEX
       && is_assignment(assignment_node = cfg_node_to_ast(cfg, cn))

       // Get rhs AST_INDEX
       && ((rhs_node = gen_ASSIGNMENT_get_rvalue(assignment_node))
	   != AST_NIL)

       // Make sure rhs is reduction operation
       && is_reduction_type(rhs_type = ast_get_node_type(rhs_node))

       // Get AST_INDEX of left operand of rhs
       && ((left_op_node = gen_BINARY_PLUS_get_rvalue1(rhs_node))
	   != AST_NIL)

       // Get SSA node of left operand of rhs
       && ((left_op_sn = ssa_node_map(cfg, left_op_node))
	   != SSA_NIL)

       // Make sure id of left operand of rhs is same as for lhs
       && ((left_op_id = refs_keys->SsaNode2Key(left_op_sn))
	   == lhs_id)

       // Get key of lhs
       && ((lhs_key = refs_keys->getRefsKey(lhs_id))
	   != NULL)

       // Retrieve lhs var symbol table index
       && (lhs_var = lhs_key->getVar()));

    rhs_sn = left_op_sn;
    while (is_reduction

	   // Get SSA node of next operand
	   && ((rhs_sn = ssa_next_subnode(cfg, rhs_sn))
	       != SSA_NIL))
    {
      is_reduction = (Boolean)

	// Get id of operand
	(((op_id = refs_keys->SsaNode2Key(left_op_sn))
	  != RefsKeyNil)

	 // Get key of operand
	 && ((op_key = refs_keys->getRefsKey(op_id))
	     != NULL)

	 && ((op_var = op_key->getVar())
	     == lhs_var));
    }
  }

  reduction_type = is_reduction ? rhs_type : 0;

  return reduction_type;
}


/**********************************************************************
 * dump()  Print the given annotation.
 */
void
CommDFAnnot::dump(Gt_index gti) const
{
  int i;

  if (gti == Gt_cnt)
  {
    gti = comm_problem->getDump_i();
  }

  cout << "CommDFAnnot for CfgNode " << cn << ":" <<
    "\n\tREF:     " << REF << "\tDEF:  " << DEF <<
      "\n\tADD:     " << ADD << "\tMULT: " << MULT <<
	"\tRED:     " << RED << endl;

  if (gti == Gt_cnt)         // Print all GiveTake instances ?
  {
    for (i = 0; i < Gt_cnt; i++)
    {
      cout << gt[i];
    }
  }
  else                    // Print a particular GiveTake instance
  {
    cout << gt[gti];
  }
}


/**********************************************************************
 * CommDFAnnot_set_access_apply()  Apply the given set access function
 *                                 to the given annotation.
 */
const VectorSet &
CommDFAnnot_set_access_apply(SetAccessFunc set_access_v,
			     CfgAnnotPtr annot_v)
{
//  CommDFAnnotSetAccessFunc set_access =
//    (CommDFAnnotSetAccessFunc) set_access_v;
//  CommDFAnnotPtr           annot      = (CommDFAnnotPtr) annot_v;
//  const VectorSet          &set       = (annot->*set_access)();
  const VectorSet& set = *(new VectorSet);

  return set;
}


/**********************************************************************
 * CommDFAnnot_gt_access_apply()  Apply the given set access function
 *                                to the given annotation.
 */
GiveTakePtr
CommDFAnnot_gt_access_apply(GtAccessFunc gt_access_v,
			    CfgAnnotPtr  annot_v)
{
  Gt_index gt_access = (Gt_index) gt_access_v;
  CommDFAnnotPtr annot     = (CommDFAnnotPtr) annot_v;
  GiveTakePtr    gt        = annot->getGt(gt_access);

  return gt;
}


/**********************************************************************
 * place_Inspecs()  Place inspector.
 *                  Take lca of results or consumption
 */
void
CommDFAnnot::place_Inspecs()
{
  RefsKey   *refs_key;
  int       i;
  VectorSet SET;

  if (!is_root())
  {
    for (i = 0; i < Gt_cnt; i++)
    {
      //for(VectorSetI en(&(gt[i].getGEN_entry())); en.test(); en++)
      //{
      //  refs_key = refs_keys->getRefsKey(en.elem);
      //  refs_key->place_Inspecs(cn);
      //}
      //
      //for(VectorSetI ex(&(gt[i].getGEN_exit())); ex.test(); ex++)
      //{
      //  refs_key = refs_keys->getRefsKey(ex.elem);
      //  refs_key->place_Inspecs(cn);
      //}

      if (Gt_forward[i])
      {
	SET = gt[i].getGEN_entry() | gt[i].getGEN_exit();
      }
      else
      {
	SET = gt[i].getTAKEN();
      }
      
      for(VectorSetI en(&SET); en.test(); en++)
      {
	refs_key = refs_keys->getRefsKey(en.elem);
	refs_key->place_Inspecs(cn);
      }
    }
  }
}


/**********************************************************************
 * gen_red_init_stmts()  Generate initialization of buffered elements
 *                       for reductions (scatter_add, etc.)
 */
void
CommDFAnnot::gen_red_init_stmts()
{
  Gt_index    gti;                      // Current GiveTake instance
  int         i;
  RefsKey     *refs_key;
  int         level;
  int         init_value;
  const char  *iter_name;
  VectorSet   init_set;
  ostrstream  buf;
  char        *cmt_str;
  const char  *name;
  AST_INDEX   node, node2, lo_node, hi_node, body_node;
  CfgNodeId   comm_cn;
  S_desc      *ref;
  const Slice *slice;
  GiveTake    *header_gt;
  CfgNodeId   header_cn     = tarj_outer(problem->getIntervals(), cn);
  int         needs_comma   = false;
  Boolean     prepend       = true;
  Boolean     skip_comments = true;
  AST_INDEX   stmts_node    = AST_NIL;
  CfgInstance cfg           = problem->getCfg();

  if (header_cn != CFG_NIL)
  {
    for (i = (int) Scatter_add; i <= (int) Scatter_mult; i++)
    {
      gti         = (Gt_index) i;
      GiveTake &g = gt[gti];

      // Heuristic: Initialize whatever is taken here but not at header
      header_gt = comm_problem->getAnnot(header_cn)->getGt(gti);
      //init_set = g.getGIVEN() - g.getGIVEN_in();
      init_set = g.getTAKE() - header_gt->getTAKE();
      if (!init_set.isEmpty())
      {
	iter_name = comm_problem->get_red_init_ind_name();
	init_value = Gt_init_red[gti];

	level     = getLevel();
	buf << "--<< " << Gt_names[gti] << " initialization for {";

	for( VectorSetI iter(&init_set); iter.test(); iter++ )
	{
	  refs_key = refs_keys->getRefsKey(iter.elem);
	  if (refs_key->needs_irreg_comm())
	  {
	    refs_key->catKeyStr(buf, level);
	    ref      = refs_key->get_ref();
	    if (ref->getRank() != 1)
	    {
	      cerr << "WARNING: CommDFAnnot::gen_red_init_stmts(): "
		<< "Attempt to initialize for reduction of "
		  << ref->getStr() << ", a " << ref->getRank()
		    << "-d array, need 1-d.\n";
	    }
	    else
	    {
	      // Generate assignment "x($init) = 0"
	      slice   = ref->getSlice();
	      name    = ref->getArray_name();
	      node    = pt_gen_ident((char *)name);        // "x"
	      node2   = pt_gen_ident((char *)iter_name);   // "$init"
	      node2   = list_create(node2);
	      node    = gen_SUBSCRIPT(node, node2);        // "x($init)"
	      node2   = pt_gen_int(init_value);            // "0"
	      node    = gen_ASSIGNMENT(AST_NIL, node, node2);
	      body_node = list_create(node);

	      // Generate "do $init = x$onsize+1, x$onsize+x$offsize"
	      node    = slice->getLoc_size_node();
	      node    = tree_copy(node);                 // "x$onsize"
	      node2   = pt_gen_int(1);
	      node    = pt_gen_add(node, node2);         // "x$onsize+1"
	      lo_node = pt_simplify_expr(node);
	      node    = slice->getLoc_size_node();
	      node    = tree_copy(node);                 // "x$onsize"
	      name    = slice->getLoc_cnt_name();  
	      node2   = pt_gen_ident((char*) name);      // "x$offsize"
	      node    = pt_gen_add(node, node2);
	      hi_node = pt_simplify_expr(node);  // "x$onsize+x$offsize"
	      node    = pt_gen_ident((char *)iter_name); // "$init" 
	      node    = gen_INDUCTIVE(node, lo_node, hi_node, AST_NIL);
	      node    = gen_DO(AST_NIL, AST_NIL, AST_NIL, node,
			       body_node);
	      node    = list_create(node);
	      stmts_node = mergeStmts(stmts_node, node);
	    }
	  }
	}

	//refs_keys->catKeysStr(init_set, buf, level);
	if (needs_comma)
	{
	  buf << ", ";
	}
	needs_comma = true;
	buf << "} >>--";
	buf << ends;
	cmt_str = buf.str();
	node = genLongComment(cmt_str);
	stmts_node = list_append(node, stmts_node);
	delete cmt_str;

	comm_cn = find_comm_cn(prepend);
	attach_cfg_node(cfg, comm_cn, stmts_node, prepend,
			skip_comments);
      }
    }
  }
}

/**********************************************************************
 * gen_comm_stmts()  Generate the communication comments for this
 *                   annotation.
 *
 *  Comm type              Append    Prepend
 *
 * SCATTER         Early   Entry      Exit
 * SCATTER Send    Late    Entry      Exit
 * SCATTER Recv    Early   Entry      Exit
 * GATHER          Early   Exit       Entry
 * GATHER Send     Early   Exit       Entry
 * GATHER Recv     Late    Exit       Entry
 */
void
CommDFAnnot::gen_comm_stmts()
{
  Gt_index   gti;                      // Current GiveTake instance
  int        i;
  Boolean    forward, prepend;
  const char *name;
  VectorSet  send_set, recv_set;
  Boolean    split_comm = comm_problem->getSplit_comm();

  for (i = Gt_cnt-1; i >= 0; i--)            // GATHER after SCATTER's
  {
    gti         = (Gt_index) i;
    GiveTake &g = gt[gti];
    name        = Gt_names[gti];
    forward     = g.getForward();

    // Split the communication code into SEND and RECV ?
    if (split_comm)
    {
      // No stmts at this node ?
      if (no_node())
      {
	prepend   = true;
	if (forward)
	{
	  send_set = g.getGEN_entry() | g.getGEN_exit();
	  recv_set = g.getGEN_entry_late() | g.getGEN_exit_late();
	}
	else
	{
	  send_set = g.getGEN_exit_late() | g.getGEN_entry_late();
	  recv_set = g.getGEN_exit() | g.getGEN_entry();
	}
	gen_send_recv(name, prepend, send_set, recv_set);
      }
      else
      {
	// Generate communication for prepend_node
	prepend   = true;
	send_set  = forward ? g.getGEN_entry() : g.getGEN_exit_late();
	recv_set  = forward ? g.getGEN_entry_late() : g.getGEN_exit();
	gen_send_recv(name, prepend, send_set, recv_set);
	
	// Generate communication for append_node
	prepend   = false;
	send_set  = forward ? g.getGEN_exit() : g.getGEN_entry_late();
	recv_set  = forward ? g.getGEN_exit_late() : g.getGEN_entry();
	gen_send_recv(name, prepend, send_set, recv_set);
      }
    }
    else
    {
      // Generate communication for prepend_node
      prepend  = true;
      send_set = forward ? g.getGEN_entry() : g.getGEN_exit();
      gen_comm_stmt(send_set, name, Both, prepend);

      // Generate communication for append_node
      prepend  = false;
      send_set = forward ? g.getGEN_exit() : g.getGEN_entry();
      gen_comm_stmt(send_set, name, Both, prepend);
    }
  }
}


/**********************************************************************
 * gen_send_recv()
 */
void
CommDFAnnot::gen_send_recv(const char *name,
			   Boolean    prepend,
			   VectorSet  send_set,
			   VectorSet  recv_set) const
{
  VectorSet both_set;

  both_set  = send_set & recv_set;
  send_set -= both_set;
  recv_set -= both_set;
  gen_comm_stmt(send_set, name, Send, prepend);
  gen_comm_stmt(both_set, name, Both, prepend);
  gen_comm_stmt(recv_set, name, Recv, prepend);
}


/**********************************************************************
 * gen_comm_node()  Generate a communication comment of the form
 *
 *                  C      --<< SCATTER_ADD recv {x(n1$arr(1:50))} >>--
 *                                 ^^^^^^   ^^^^      ^^^^^
 *                                 <name>   <cmt>     <SET>
 *
 * ... or "call fscatter_add(x(x$onsize), x(1), x$sched)
 */
AST_INDEX
CommDFAnnot::gen_comm_node(const VectorSet &SET,
                           Comm_dir comm, const char *name) const
{
  ostrstream    buf;
  int           level;
  int           type;
  char          *cmt_str;
  S_desc        *ref;
  const RefsKey *refs_key;
  const Slice   *slice;
  const char    *sched_name;
  const char    *array_name;
  const char    *function_name;
  AST_INDEX     cmt_node, node, arg_node, loc_size_node;
  const char    *cmt = Comm_dir_names[comm];

  level = getLevel();

  if (!comm_problem->getHigh_level())
  {
    buf << "--<< ";
  }
  buf << name;
  if (strlen(cmt) > 0) {
    buf << " " << cmt;
  }
  buf << " {";
  refs_keys->catKeysStr(SET, buf, level);
  buf << "}";
  if (!comm_problem->getHigh_level())
  {
    buf << " >>--";
  }
  buf << ends;
  cmt_str = buf.str();
  cmt_node = genLongComment(cmt_str);
  delete cmt_str;

  if (!comm_problem->getHigh_level())
  {
    // "call fscatter_add(x(x$onsize + 1), x(1), x$sched)"
    for( VectorSetI i(&SET); i.test(); i++ )
    {
      refs_key = refs_keys->getRefsKey(i.elem);
      if (refs_key->needs_irreg_comm())
      {
        ref = refs_key->get_ref();
        if (ref->getRank() != 1)
        {
          cerr << "WARNING: CommDFAnnot::gen_comm_node(): "
            << "Attempt to generate irreg comm for " << ref->getStr()
              << ", a " << ref->getRank() << "-d array, need 1-d.\n";
        }
        else
        {
          array_name    = ref->getArray_name();
	  type          = refs_keys->getSt()->get_type(array_name);
	  function_name = prefix_type(type, name);
          slice         = ref->getSlice();
          sched_name    = slice->getSched_name();
          loc_size_node = slice->getLoc_size_node();
          arg_node = tree_copy(loc_size_node);           // "x$onsize"
          arg_node = pt_gen_add(arg_node, pt_gen_int(1)); // "x$onsize+1"
          arg_node = pt_simplify_expr(arg_node);
          arg_node = list_create(arg_node);
          node     = pt_gen_ident((char *)array_name); // "x"
          node     = gen_SUBSCRIPT(node, arg_node); // "x(x$onsize+1)"
          node     = list_create(node);
          arg_node = pt_gen_int(1);                    // "1"
          arg_node = list_create(arg_node);
          arg_node = gen_SUBSCRIPT(pt_gen_ident((char*) array_name),
                                   arg_node);          // "x(1)"
          node     = list_insert_last(node, arg_node);
          arg_node = pt_gen_ident((char*)sched_name); // "x$sched"
          node     = list_insert_last(node, arg_node);
          node     = pt_gen_invoke((char*) function_name, node);
          node     = gen_CALL(AST_NIL, node);
          cmt_node = list_insert_last(cmt_node, node);
        }
      }
    }
  }

  return cmt_node;
}


/**********************************************************************
 * gen_comm_stmt()  Generate communication statement
 *                  and prepend/append it to the AST corresponding to
 *                  the CFG node of this annotation.
 *
 * We also want to do sends before receives.
 * Therefore, if we are supposed to prepend a Send to <cn>, <cn> has
 * only one (forward edge) predecessor, <only_pred_cn>, and <cn> is the
 * only successor of <only_pred_cn>, then we shift the Send up by
 * appending it to <only_pred_cn>.
 * Similarly we try to shift Recv's down.
 */
void
CommDFAnnot::gen_comm_stmt(const VectorSet &SET, const char *name,
			   Comm_dir comm, Boolean prepend) const
{
  AST_INDEX   cmt_node;
  CfgNodeId   comm_cn;

  if (!is_root() && !SET.isEmpty())
  {
    cmt_node = gen_comm_node(SET, comm, name);

    // 7/5/93 RvH: Disabled to make sure Scatters are finished before Gathers
    //comm_cn = cn;
    //if (prepend && (comm == Send)) {              // Shift Send's up
    //  do {
    //    comm_cn = cfg_predom_pred(cfg, comm_cn);
    //  } while ((comm_cn != CFG_NIL)
    //       && (comm_problem->getAnnot(comm_cn)->no_node()));
    //} else { 
    //  if (!prepend && (comm == Recv)) {          // Shift Recv's down
    // do {
    //  comm_cn = cfg_postdom_succ(cfg, comm_cn);
    //} while ((comm_cn != CFG_NIL)
    //	 && (comm_problem->getAnnot(comm_cn)->no_node()));
    //  }
    //}
    //if (comm_cn == CFG_NIL) {
    //  comm_cn = cn;
    //} else {
    //  if (comm_cn != cn) {
    //prepend = !prepend;
    //  }
    //}

    comm_cn = find_comm_cn(prepend);

    comm_problem->getAnnot(comm_cn)->add_comm_node(cmt_node, prepend);
  }
}


/**********************************************************************
 * find_comm_cn()
 */
CfgNodeId
CommDFAnnot::find_comm_cn(Boolean &prepend) const
{
  CfgNodeId   comm_cn = CFG_NIL;
  CfgInstance cfg     = problem->getCfg();

  // Try to avoid generating code at not-yet existing nodes
  comm_cn = cn;
  if (comm_problem->getAnnot(comm_cn)->no_node())  // No AST node ?
  {
    // Try to find predominating successor that has an AST node
    while ((comm_cn != CFG_NIL)
	   && comm_problem->getAnnot(comm_cn)->no_node())
    {
      comm_cn = cfg_predom_pred(cfg, comm_cn);
    }

    if (comm_cn != CFG_NIL)                // Found a predominator ?
    {
      prepend = false;                     // Append to predominator
    }
    else
    {
      // Try to find postdominating predecessor that has an AST node
      comm_cn = cn;
      while ((comm_cn != CFG_NIL)
	     && comm_problem->getAnnot(comm_cn)->no_node())
      {
	comm_cn = cfg_postdom_succ(cfg, comm_cn);
      }
      if (comm_cn != CFG_NIL)             // Found a postdominator ?
      {
	prepend = true;                   // Prepend to postdominator
      }
      else
      {
	comm_cn = cn;                     // Switch back to old cn
      }
    }
  }

  return comm_cn;
}


/**********************************************************************
 * add_comm_node()
 */
void
CommDFAnnot::add_comm_node(AST_INDEX node, Boolean prepend)
{
  if (prepend)
  {
    prepend_node = list_append(prepend_node, node);
  }
  else
  {
    append_node = list_append(append_node, node);
  }
}


/**********************************************************************
 * insert_comm_stmts()
 */
void
CommDFAnnot::insert_comm_stmts()
{
  CfgInstance cfg           = problem->getCfg();
  Boolean     skip_comments = false;

  if (no_node())
  {
    prepend_node = list_append(prepend_node, append_node);
    append_node  = AST_NIL;
  }

  if (prepend_node != AST_NIL)
  {
    attach_cfg_node(cfg, cn, prepend_node, true, skip_comments);
  }

  if (append_node != AST_NIL)
  {
    attach_cfg_node(cfg, cn, append_node, false, skip_comments);
  }
}

