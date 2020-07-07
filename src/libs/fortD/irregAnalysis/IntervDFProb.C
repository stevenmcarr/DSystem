/* $Id: IntervDFProb.C,v 1.10 1999/06/11 20:42:32 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/**********************************************************************
 * Interval data flow analysis.
 */

/**********************************************************************
 * Revision History:
 * $Log: IntervDFProb.C,v $
 * Revision 1.10  1999/06/11 20:42:32  carr
 * updated for Linux
 *
 * Revision 1.9  1999/03/07 17:31:45  carr
 * updated for Linux
 *
 * Revision 1.8  1997/03/11 14:28:28  carr
 * newly checked in as revision 1.8
 *
Revision 1.8  94/03/21  13:17:12  patton
fixed comment problem

Revision 1.7  94/02/27  20:14:26  reinhard
Added value-based distributions.
Make CC happy.
See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.

 * Revision 1.11  1994/02/27  19:41:38  reinhard
 * Robustified GiveNTake.
 *
 * Revision 1.10  1994/01/18  19:47:09  reinhard
 * Updated include paths according to Makefile change.
 *
 * Revision 1.9  1993/09/25  15:32:31  reinhard
 * Erased memory leak.
 *
 * Revision 1.8  1993/09/02  18:47:00  reinhard
 * Robustified GiveNTake framework.
 *
 * Revision 1.7  1993/07/13  23:03:10  reinhard
 * Added support for entry/exit code generation.
 * Major overhaul of framework.
 *
 */

#include <iostream>
#include <assert.h>
#include <libs/fortD/irregAnalysis/IntervDFProb.h>

using namespace std;
/*------------------- GLOBAL DECLARATIONS ---------------------*/

EXTERN(CfgNodeId, cfg_get_exit_header, (CfgInstance cfg,
					TarjTree tarjans, CfgNodeId cn));
EXTERN(AST_INDEX, cfg_node_to_near_ast, (CfgInstance cfg,
					 CfgNodeId   cn,
					 Boolean     add_empty_else
					 = false));
EXTERN(int,       tarj_level2,     (TarjTree tarjans, CfgNodeId cn));
EXTERN(int,       tarj_level_max,  (CfgInstance cfg));



/*********************************************************************
 *                                                                   *
 *           Methods of class IntervalDFProblem                      *
 *                                                                   *
 *********************************************************************/


/**********************************************************************
 * Constructor
 */
IntervalDFProblem::IntervalDFProblem(CfgInstance        my_cfg,
				     SetAccessApplyFunc my_set_access_apply,
				     GtAccessApplyFunc  my_gt_access_apply)
: cfg              (my_cfg),
  intervals        (cfg_get_intervals(cfg)),
  node_cnt         (cfg_node_max(cfg)),
  annot            (new CfgAnnotPtr[node_cnt]),
  set_access_apply (my_set_access_apply),
  gt_access_apply  (my_gt_access_apply),
  level_max        (tarj_level_max(cfg))
{
}


/**********************************************************************
 * Destructor
 */
IntervalDFProblem::~IntervalDFProblem()
{
  delete annot;
}


/**********************************************************************
 * getAnnot()  Range checking access function
 */
CfgAnnot *
IntervalDFProblem::getAnnot(int cn) const
{
  CfgAnnot *ann = ((cn >= 0) && (cn < node_cnt)) ? annot[cn] : NULL;

  return ann;
}


/**********************************************************************
 * putAnnot()  Range checking access function
 */
void
IntervalDFProblem::putAnnot(int cn, CfgAnnotPtr ann)
{
  assert((cn >= 0) && (cn < node_cnt));
  annot[cn] = ann;
}


/**********************************************************************
 * init_annots()  Initialize the annotations for all CFG nodes.
 */
void
IntervalDFProblem::init_annots(TraversalOrder order)
{
  CfgNodeId       cn;
  CfgAnnotPtr     ann;

  for (IntervalGraphIter iter(cfg, order);
       (cn = iter()) != CFG_NIL;)
  {
    ann = init_annot(cn);
    putAnnot(cn, ann);
  }
}


/**********************************************************************
 * give_n_take()
 */
void
IntervalDFProblem::give_n_take(GtAccessFunc gt_access)
{
  Boolean  forward;
  CfgAnnot *ann = annot[0]; // First annotation, to retrieve <forward>

  if (ann) // Are there any annotations ?
  {
    cur_gt_access = gt_access;
    forward       = ann2gt(ann)->getForward();

    //for (level = level_max; level >= 0; level--)
    //{
    //  gt_solve_sens(&GiveTake::sens_TAKEN,
	//	    forward ? ReversePreOrder : PostOrder, level);
    //  gt_solve_sens(&GiveTake::sens_STEAL,
	//	    forward ? PostOrder : ReversePreOrder, level);
    //}

    gt_solve_combined(&GiveTake::sens_TAKEN,
		      &GiveTake::sens_STEAL,
		      forward ? ReversePreOrder : PostOrder);
    gt_solve_sens(&GiveTake::sens_GIVEN,
		  forward ? PreOrder : ReversePostOrder);
    gt_solve_sens(&GiveTake::sens_RES,
		  forward ? PreOrder : ReversePostOrder);
    gt_solve_sens(&GiveTake::sens_GEN,
		  forward ? ReversePreOrder : PostOrder);
  }
}


/**********************************************************************
 * gt_solve_combined()
 */
void
IntervalDFProblem::gt_solve_combined(GtSensFunc     sens1,
				     GtSensFunc     sens2,
				     TraversalOrder order1)
{
  CfgNodeId   cn1;        // Current node for <sens1> (TAKEN)
  CfgNodeId   cn2;        // Current node for <sens2> (STEAL)
  CfgAnnot    *ann;       // Current annotation
  LifteeType  is_lift;    // Do we have to lift ?
  LifteeType  do_lift;    // Determine liftee
  GiveTakePtr gt;
  GiveTakePtr liftee_gt;  // Annotation of liftee
  Meetees     meetees1 = (order1 == PreOrder
			  || order1 == PostOrder) ? Preds : Succs;
  Meetees     meetees2 = (order1 == PreOrder
			  || order1 == PostOrder) ? Succs : Preds;
  Meetees     order2   = (order1 == PreOrder || order1 == PostOrder)
    ? ChildrenBackward : ChildrenForward;
  IntervalGraphIter iter1(cfg, order1);
  CfgMeetIter       iter2(cfg, CFG_NIL, order2);
  CfgAnnotPtrIter   meet1_gts(cfg, CFG_NIL, (const IntervalDFProblem *)this, 
			      (AccessApplyFunc) gt_access_apply,
			      meetees1, (GtAccessFunc) cur_gt_access);
  CfgAnnotPtrIter   meet2_gts(cfg, CFG_NIL, (const IntervalDFProblem *)this, 
			      (AccessApplyFunc)gt_access_apply,
			      meetees2,(GtAccessFunc) cur_gt_access);

  is_lift = select_LiftPredicate(order1);
  do_lift = select_LifteeFunc(order1);
  
  while ((cn1 = iter1()) != CFG_NIL)
  {
    // <cn1> is interval header ?
    if (is_liftee(cn1, TarjHeader))
    {
      // Iterate through children of <cn1> & compute <sens2> (STEAL)
      iter2.reset(cn1);
      while ((cn2 = iter2()) != CFG_NIL)
      {
	ann = getAnnot(cn2);
	gt  = ann2gt(ann);
	meet2_gts.reset(cn2);
	(gt->*sens2)(&meet2_gts, NULL);
      }
    }

    liftee_gt = get_liftee_gt(cn1, do_lift, is_lift);
    ann       = getAnnot(cn1);
    gt        = ann2gt(ann);
    meet1_gts.reset(cn1);
    (gt->*sens1)(&meet1_gts, liftee_gt);
  }
}


/**********************************************************************
 * gt_solve_sens()  Solve a flow sensitive data flow problem.
 *                  Walk all the CFG nodes in specified <order> and apply
 *                  <sens>.
 *
 *    <order>       Meetees     Liftee
 *
 * PreOrder          Preds    tarj_outer
 * PostOrder         Preds    tarj_inners_last
 * ReversePreOrder   Succs    tarj_inners
 * ReversePostOrder  Succs    tarj_outer
 */
void
IntervalDFProblem::gt_solve_sens(GtSensFunc     sens,
				 TraversalOrder order,
				 int            only_level)
{
  CfgNodeId         cn;         // Current node
  LifteeType        is_lift;    // Do we have to lift ?
  LifteeType        do_lift;    // Determine liftee
  GiveTakePtr       gt;
  GiveTakePtr       liftee_gt;  // Annotation of liftee
  Meetees           meetees = (order == PreOrder
			       || order == PostOrder) ? Preds : Succs;
  CfgAnnotPtrIter   meet_gts(cfg, CFG_NIL, (const IntervalDFProblem  *)this, 
			     (AccessApplyFunc)gt_access_apply,
			     meetees, (GtAccessFunc)cur_gt_access);
  IntervalGraphIter iter(cfg, order, only_level);

  is_lift = select_LiftPredicate(order);
  do_lift = select_LifteeFunc(order);
  
  while ((cn = iter()) != CFG_NIL)
  {
    liftee_gt  = get_liftee_gt(cn, do_lift, is_lift);
    gt         = ann2gt(getAnnot(cn));
    meet_gts.reset(cn);
    (gt->*sens)(&meet_gts, liftee_gt);
  }
}


/**********************************************************************
 * solve_sens()  Solve a flow sensitive data flow problem.
 *               Walk all the CFG nodes in specified <order> and apply
 *               <sens>.
 *
 *    <order>       Meetees     Liftee
 *
 * PreOrder          Preds    tarj_outer
 * PostOrder         Preds    tarj_inners_last
 * ReversePreOrder   Succs    tarj_inners
 * ReversePostOrder  Succs    tarj_outer
 */
void
IntervalDFProblem::solve_sens(SensFunc sens, TraversalOrder order)
{
  CfgNodeId       cn;         // Current node
  Meetees         meetees;    // "Preds" or "Succs"
  LifteeType      is_lift;    // Do we have to lift ?
  LifteeType      do_lift;    // Determine liftee
  CfgAnnotPtr     liftee_ann; // Annotation of liftee
  CfgAnnotPtrIter *meet_gts;
  
  meetees = (order == PreOrder || order == PostOrder) ? Preds : Succs;
  is_lift = select_LiftPredicate(order);
  do_lift = select_LifteeFunc(order);
  
  for (IntervalGraphIter iter(cfg, order);
       (cn = iter()) != CFG_NIL;)
  {
    liftee_ann  = get_liftee_ann(cn , do_lift, is_lift);
    meet_gts = new
      CfgAnnotPtrIter(cfg, cn, this,
		      (AccessApplyFunc) set_access_apply, meetees);
    apply_sens(sens, getAnnot(cn), meet_gts, liftee_ann);
    delete meet_gts;
  }
}


/**********************************************************************
 * solve_insens()  Solve a flow insensitive data flow problem.
 *                 Walk all the CFG nodes in specified <order> and apply
 *                 <insens>.
 */
void
IntervalDFProblem::solve_insens(InsensFunc insens, TraversalOrder order)
{
  CfgNodeId         cn;        // Current node
  CfgAnnotPtrIter   *meet_gts;
  Meetees           meetees;   // "Preds" or "Succs"
  IntervalGraphIter iter(cfg, order);

  meetees = (order == PreOrder || order == PostOrder) ? Preds : Succs;
  
  while ((cn = iter()) != CFG_NIL)
  {
    meet_gts = new
      CfgAnnotPtrIter(cfg, cn, this,
		      (AccessApplyFunc) set_access_apply, meetees);
    apply_insens(insens, getAnnot(cn), meet_gts);
    delete meet_gts;
  }
}


/**********************************************************************
 * gen_meet_annots()
 */
CfgAnnotPtrIter *
IntervalDFProblem::gen_meet_annots(CfgNodeId cn, Meetees meetees) const
{
  return new CfgAnnotPtrIter(cfg, cn,(const IntervalDFProblem  *)this, 
			    (AccessApplyFunc)set_access_apply,
			     meetees, (GtAccessFunc)0);
}


/**********************************************************************
 * gen_meet_gts()
 */
CfgAnnotPtrIter *
IntervalDFProblem::gen_meet_gts(CfgNodeId cn, Meetees meetees) const
{
  return new CfgAnnotPtrIter(cfg, cn, (const IntervalDFProblem  *)this, 
			     (AccessApplyFunc)gt_access_apply, meetees,
			     (GtAccessFunc)cur_gt_access);
}


/**********************************************************************
 * ann2gt()
 */
GiveTake *
IntervalDFProblem::ann2gt(CfgAnnot *ann)
{
  GiveTake *gt = gt_access_apply(cur_gt_access, ann);

  return gt;
}


/**********************************************************************
 * is_liftee()
 */
Boolean
IntervalDFProblem::is_liftee(CfgNodeId  cn,
			     LifteeType type) const
{
  Boolean result;

  switch (type) {
  case TarjHeader:
    result = tarj_is_header(intervals, cn);
    break;

  case  TarjFirstChild:
    result = tarj_is_first(intervals, cn);
    break;

  case  TarjLastChild:
    result = tarj_is_last(intervals, cn);
    break;

  default:
    cerr << "WARNING IntervalDFProblem::is_liftee(): " <<
      "unknown type " << type << endl;
  }

  return result;
}


/**********************************************************************
 * get_liftee()
 */
CfgNodeId
IntervalDFProblem::get_liftee(CfgNodeId  cn,
			      LifteeType type) const
{
  CfgNodeId liftee;

  switch (type) {
  case TarjHeader:
    liftee = tarj_outer(intervals, cn);
    break;

  case  TarjFirstChild:
    liftee = tarj_inners(intervals, cn);
    break;

  case  TarjLastChild:
    liftee = tarj_inners_last(intervals, cn);
    break;

  case  TarjExitHeader:
    liftee = cfg_get_exit_header(cfg, intervals, cn);
    break;

  default:
    cerr << "WARNING IntervalDFProblem::get_liftee(): " <<
      "unknown type " << type << endl;
  }

  return liftee;
}


/**********************************************************************
 * get_liftee_ann()
 */
CfgAnnot *
IntervalDFProblem::get_liftee_ann(CfgNodeId  cn,
				  LifteeType do_lift,
				  LifteeType is_lift) const
{
  CfgNodeId liftee;      // The header / first son / last son
  CfgAnnot  *liftee_ann; // Annotation of liftee

  liftee = ((is_lift == Any) || (is_liftee(cn, is_lift)))
    ? get_liftee(cn, do_lift)
      : CFG_NIL;
  liftee_ann = (liftee == CFG_NIL) ? 0 : getAnnot(liftee);

  return liftee_ann;
}


/**********************************************************************
 * get_liftee_gt()
 */
GiveTake *
IntervalDFProblem::get_liftee_gt(CfgNodeId  cn,
				 LifteeType do_lift,
				 LifteeType is_lift)
{
  CfgAnnot *liftee_ann = get_liftee_ann(cn, do_lift, is_lift);
  GiveTake *liftee_gt  = liftee_ann ? ann2gt(liftee_ann) : 0;

  return liftee_gt;
}


/**********************************************************************
 * select_LiftPredicate()  Given a TraversalOrder, select the predicate
 *                         that checks whether we have to lift data
 *                         flow information between levels.
 */
LifteeType
IntervalDFProblem::select_LiftPredicate(TraversalOrder order)
{
  LifteeType result;

  switch (order) {
  case PostOrder:
  case ReversePreOrder:
    result = TarjHeader;
    break;

  case PreOrder:
    result = TarjFirstChild;
    break;

  case ReversePostOrder:
    result = TarjLastChild;
    break;
   default:
    cerr << "WARNING IntervalDFProblem::select_LiftPredicate(): " <<
      "unknown order " << order << endl;
  }

  return result;
}


/**********************************************************************
 * select_LifteeFunc()  Given a TraversalOrder, select the function
 *                      that maps a CfgNodeId to a liftee.
 */
LifteeType
IntervalDFProblem::select_LifteeFunc(TraversalOrder order)
{
  LifteeType result;

  switch (order) {
  case PreOrder:
  case ReversePostOrder:
    result = TarjHeader;
    break;

  case PostOrder:
    result = TarjLastChild;
    break;

  case ReversePreOrder:
    result = TarjFirstChild;
    break;

  default:
    cerr << "WARNING IntervalDFProblem::select_LifteeFunc(): " <<
      "unknown order " << order << endl;
  }

  return result;
}


/**********************************************************************
 * dump_annots()  Dump annotations of all CFG nodes.
 */
void
IntervalDFProblem::dump_annots(TraversalOrder order) const
{
  CfgNodeId cn;

  cout << "\n*** DUMPING Control Flow Graph Annotations ***\n";

  for (IntervalGraphIter iter(cfg, order);
       (cn = iter()) != CFG_NIL;
       getAnnot(cn)->dump());
}



/*********************************************************************
 *                                                                   *
 *           Methods of class CfgAnnot                               *
 *                                                                   *
 *********************************************************************/

/**********************************************************************
 * Constructor
 */
CfgAnnot::CfgAnnot(IntervalDFProblem *my_problem, CfgNodeId my_cn)
: problem (my_problem),
  cn      (my_cn),
  node    (cfg_node_to_near_ast(problem->getCfg(), cn))
{
}


/**********************************************************************
 * is_root()
 */
Boolean
CfgAnnot::is_root() const
{
  CfgInstance cfg        = problem->getCfg();
  CfgNodeId   start_node = cfg_start_node(cfg);
  Boolean     is_root    = (Boolean) (cn == start_node);
  
  return is_root;
}


/**********************************************************************
 * is_end()
 */
Boolean
CfgAnnot::is_end() const
{
  CfgInstance cfg      = problem->getCfg();
  CfgNodeId   end_node = cfg_end_node(cfg);
  Boolean     is_end   = (Boolean) (cn == end_node);
  
  return is_end;
}


/**********************************************************************
 * getLevel()  Compute the Tarjan nesting level of the current node.
 */
int
CfgAnnot::getLevel() const
{
  TarjTree tarjans = cfg_get_intervals(problem->getCfg());
  int      level   = tarj_level2(tarjans, cn);

  return level;
}


/**********************************************************************
 * is_liftee()
 */
Boolean
CfgAnnot::is_liftee(LifteeType type)
{
  Boolean result;

  result = problem->is_liftee(cn, type);

  return result;
}


/**********************************************************************
 * get_liftee_gt()
 */
GiveTake *
CfgAnnot::get_liftee_gt(LifteeType type)
{
  GiveTake *result;

  result = problem->get_liftee_gt(cn, type);

  return result;
}


/**********************************************************************
 * gen_meet_gts()
 */
CfgAnnotPtrIter *
CfgAnnot::gen_meet_gts(Meetees meetees) const
{
  CfgAnnotPtrIter *gts = problem->gen_meet_gts(cn, meetees);

  return gts;
}


/*********************************************************************
 *                                                                   *
 *           Methods of class CfgAnnotPtrIter                        *
 *                                                                   *
 *********************************************************************/

/**********************************************************************
 * Constructor
 */
CfgAnnotPtrIter::CfgAnnotPtrIter(const CfgInstance  &cfg,
				 CfgNodeId          cn,
				 const IntervalDFProblem *my_problem,
				 AccessApplyFunc    my_access_apply,
				 Meetees            meetees,
				 GtAccessFunc       my_gt_access)
: iter          (cfg, cn, meetees),
  problem       (my_problem),
  access_apply  (my_access_apply),
  gt_access     (my_gt_access)
{
}


/**********************************************************************
 * get_set()  
 */ 
const VectorSet &
CfgAnnotPtrIter::get_set(SetAccessFunc set_access, CfgAnnotPtr ann)
{
  const VectorSet &set = (gt_access == GtAccessNil)
    ? (((SetAccessApplyFunc) access_apply)(set_access, ann))
      : ((((GtAccessApplyFunc) access_apply)(gt_access, ann))
	 ->*((GtSetAccessFunc) set_access))();

  return set;
}


/**********************************************************************
 * reset()  Reset iterator to first element
 */
void
CfgAnnotPtrIter::reset(CfgNodeId cn)
{
  iter.reset(cn);
}


/**********************************************************************
 * operator()  Iterator advance operator.
 */
CfgAnnotPtr
CfgAnnotPtrIter::operator() ()
{
  CfgNodeId   cn     = iter();
  CfgAnnotPtr result = problem->getAnnot(cn);
  
  return result;
}


/**********************************************************************
 * reduceSet()  Reduce Sets.
 *              <set_access>, applied to all annotations of the
 *              iterator, results in a set of sets, which is then
 *              reduced to a single set using <op>.
 */ 
VectorSet
CfgAnnotPtrIter::reduceSet(SetAccessFunc set_access, SetReduceOp op)
{
  CfgNodeId   cn;
  CfgAnnotPtr ann;
  VectorSet   set;

  iter.reset();           // Make sure we are at beginning of iterator
  cn = iter();
  if (cn != CFG_NIL)
  {
    ann = problem->getAnnot(cn);
    set = get_set(set_access, ann);
    while ((cn = iter()) != CFG_NIL)
    {
      ann                       = problem->getAnnot(cn);
      const VectorSet &next_set = get_set(set_access, ann);
      switch (op) {
      case Union:
	set |= next_set;
	break;

      case Intersect:
	set &= next_set;
	break;

      default:
	cerr << "WARNING CfgAnnotPtrIter::reduceSet(): unknown op \""
	  << op << "\".";
      }
    }
  }

  return set;
}


/**********************************************************************
 * interSet()  Intersect all Set's resulting from applying the given
 *             set access function to the annotations of the iterator.
 */ 
VectorSet
CfgAnnotPtrIter::interSet(SetAccessFunc set_access)
{
  // 3/29/93 RvH: why does this not work ?
  //return reduceSet(set_access, &VectorSet::operator&=);

  return reduceSet(set_access, Intersect);
}


/**********************************************************************
 * unionSet()  Union all Set's resulting from applying the given
 *             set access function to the annotations of the iterator.
 */ 
VectorSet
CfgAnnotPtrIter::unionSet(SetAccessFunc set_access)
{
  return reduceSet(set_access, Union);
}



/*********************************************************************
 *                                                                   *
 *           Methods of class GiveTake                               *
 *                                                                   *
 *********************************************************************/



/**********************************************************************
 * Constructor
 */ 
GiveTake::GiveTake()
{
}


/**********************************************************************
 * init()
 */ 
void 
GiveTake::init(CfgAnnot *my_ann, const char *my_name)
{
  ann  = my_ann;       // Annotation where I belong
  name = my_name;      // Name of GiveTake instance
}


/**********************************************************************
 * instantiate()
 */ 
void 
GiveTake::instantiate(Boolean my_forward, VectorSet my_TAKE_init,
		      VectorSet my_STEAL_init, VectorSet my_GIVE_init)
{
  forward    = my_forward;    // Forward or Backward problem ?
  TAKE_init  = my_TAKE_init;  // Bits taken (needed) right here
  STEAL_init = my_STEAL_init; // Bits stolen (killed) right here
  GIVE_init  = my_GIVE_init;  // Bits given (provided for free) right here
}


/**********************************************************************
 * sens_TAKEN()
 *
 * Compute TAKEN, the bits 
 *
 * NOTE: the comments (including this header) are written for a
 * forward problem (like GATHER);
 * for a backward problem (like SCATTER), the directions are reversed.
 */
void
GiveTake::sens_TAKEN(CfgAnnotPtrIter *succs,
		     GiveTakePtr     first_child)
{
  GiveTakePtr     last_child, exit_header;
  VectorSet       GIVEnSTEAL;
  CfgAnnotPtrIter *succsI = ann->gen_meet_gts(forward ? SuccsI : PredsI);
  CfgAnnotPtrIter *succsT = ann->gen_meet_gts(forward ? SuccsT : Preds);

  STEAL = STEAL_init;
  GIVE  = GIVE_init;
    
  // If node is header of an interval, then compute STEAL, TAKE, and
  // GIVE from its children.
  if (first_child)
  {
    last_child = ann->get_liftee_gt(forward ? TarjLastChild
				    : TarjFirstChild);

    // Kill everything that is killed in any child.
    //STEAL |= last_child->STEAL_loc;
    STEAL |= last_child->STEAL_out_loc;
    GIVE  |= last_child->GIVE_loc;
    BLOCK  = first_child->BLOCK_loc;
  }

  // Check whether there are things both stolen and given
  GIVEnSTEAL = GIVE & STEAL;
  if (!GIVEnSTEAL.isEmpty() && (ann->getCn() != 0))
  {
    cerr << "WARNING GiveTake::sens_TAKEN(): GIVE = " << GIVE <<
      " and STEAL = " << STEAL << " intersect at cn = " <<
	ann->getCn() << ".\n";
  }

  // Communication is blocked (cannot move past here) for data that are
  // either killed here or provided here.
  BLOCK  |= STEAL | GIVE;

  // Things are taken on exit if they are taken on entry by all Succ's
  TAKEN_out = succsI->interSet((SetAccessFunc) &GiveTake::getTAKEN);

  if (!forward)
  {
    // ... or if they are taken by the header of an incoming exit edge
    //TAKEN_out |= succsI->unionSet((SetAccessFunc) &GiveTake::getTAKEN)
    //- succs->unionSet((SetAccessFunc) &GiveTake::getTAKEN);
    exit_header = ann->get_liftee_gt(TarjExitHeader);
    if (exit_header)
    {
      TAKEN_out |= exit_header->TAKEN & exit_header->TAKEN_out;
    }
  }

  TAKE = TAKE_init;
  if (first_child)
  {
    // Take everything that is taken from the first child,
    // except things killed by any child.
    TAKE |= first_child->TAKEN - STEAL;
    TAKE |= first_child->TAKE_loc & (TAKEN_out - BLOCK);
  }

  // Things are taken here if they are either taken right here
  // or if they are taken on exit and not blocked here.
  TAKEN = TAKE | (TAKEN_out - BLOCK);

  BLOCK_loc = BLOCK;
  //BLOCK_loc |= succs->unionSet((SetAccessFunc) &GiveTake::getBLOCK_loc);
  BLOCK_loc |= succsT->unionSet((SetAccessFunc) &GiveTake::getBLOCK_loc);
  BLOCK_loc -= TAKE;

  TAKE_loc = succsT->unionSet((SetAccessFunc) &GiveTake::getTAKE_loc);
  if (first_child)
  {
    // Take everything that is taken from the first child,
    // except things killed by any child.
    TAKE_loc |= first_child->TAKE_loc;
  }
  TAKE_loc -= BLOCK;
  TAKE_loc |= TAKE;

  delete succsI;
  delete succsT;
}


/**********************************************************************
 * sens_STEAL()
 *
 * Compute STEAL, the bits 
 */
void
GiveTake::sens_STEAL(CfgAnnotPtrIter *preds,
		     GiveTakePtr     liftee)
{
  GiveTakePtr     exit_header;
  CfgAnnotPtrIter *predsI = ann->gen_meet_gts(forward ? PredsI : SuccsI);
					      //: Succs);

  GIVE_loc  = preds->interSet((SetAccessFunc) &GiveTake::getGIVE_loc);
  GIVE_loc |= GIVE | TAKE;
  GIVE_loc -= STEAL;

  //STEAL_loc  = predsI->unionSet((SetAccessFunc) &GiveTake::getSTEAL_loc);
  //STEAL_loc -= GIVE_loc;
  //STEAL_loc |= STEAL;

  STEAL_loc  = predsI->unionSet((SetAccessFunc) &GiveTake::getSTEAL_out_loc);
  if (forward)
  {
    exit_header = ann->get_liftee_gt(TarjExitHeader);
    if (exit_header)
    {
      STEAL_loc |= exit_header->STEAL_loc;
    }
  }
  STEAL_loc |= STEAL;
  STEAL_out_loc = STEAL_loc - GIVE_loc;

  delete predsI;
}


/**********************************************************************
 * sens_GIVEN()
 */
void
GiveTake::sens_GIVEN(CfgAnnotPtrIter *preds,
		     GiveTakePtr     header)
{
  if (!forward)
  {
    preds = ann->gen_meet_gts(SuccsI);
  }

  // Compute GIVEN
  if (header) {
    GIVEN_in = header->GIVEN;
  } else {
    GIVEN_in  = preds->interSet((SetAccessFunc) &GiveTake::getGIVEN_out);
    GIVEN_in |= TAKEN
      & preds->unionSet((SetAccessFunc) &GiveTake::getGIVEN_out);
  }
  GIVEN = GIVEN_in
    // Heuristic comes here:
    | TAKEN;
  //GIVEN_out = GIVE | (GIVEN - STEAL);
  GIVEN_out = (GIVE | GIVEN) - STEAL;
  
  // Compute GIVEN_late
  if (header) {
    GIVEN_late_in  = header->GIVEN_late;
  } else {
    GIVEN_late_in  = preds->interSet((SetAccessFunc)
				     &GiveTake::getGIVEN_late_out);
    GIVEN_late_in |= TAKEN & preds->unionSet((SetAccessFunc)
					     &GiveTake::getGIVEN_late_out);
  }
  GIVEN_late = GIVEN_late_in
    // Heuristic comes here:
    | TAKE;
  GIVEN_late_out = (GIVE | GIVEN_late) - STEAL;

  if (!forward)
  {
    delete preds;
  }
}


/**********************************************************************
 * sens_RES()
 */
void
GiveTake::sens_RES(CfgAnnotPtrIter *preds,
		   GiveTakePtr     liftee)
{
  CfgAnnotPtrIter *succs = ann->gen_meet_gts(forward ? Succs : Preds);
  
  // Compute RES, MOVE
  RES_entry = GIVEN - GIVEN_in;
  RES_exit  = succs->unionSet((SetAccessFunc) &GiveTake::getGIVEN_in);
  RES_exit -= GIVEN_out;
  
  MOVE_entry_in = preds->interSet((SetAccessFunc) &GiveTake::getMOVE_entry_out);
  MOVE_exit_in  = preds->interSet((SetAccessFunc) &GiveTake::getMOVE_exit_out);
  if (ann->no_node()) {
    MOVE_entry_out = MOVE_entry_in | RES_entry;
    MOVE_exit_out  = MOVE_exit_in | RES_exit;
  }
  
  // Compute RES_late, MOVE_late
  //RES_entry_late = TAKEN & (GIVEN_late - GIVEN_late_in);
  RES_entry_late = GIVEN_late - GIVEN_late_in;
  RES_exit_late  = succs->unionSet((SetAccessFunc) &GiveTake::getGIVEN_late_in);
  RES_exit_late -= GIVEN_late_out;
  
  MOVE_entry_late_in = preds->interSet((SetAccessFunc)
				       &GiveTake::getMOVE_entry_late_out);
  MOVE_exit_late_in  = preds->interSet((SetAccessFunc)
				       &GiveTake::getMOVE_exit_late_out);
  if (ann->no_node()) {
    MOVE_entry_late_out = MOVE_entry_late_in | RES_entry_late;
    MOVE_exit_late_out  = MOVE_exit_late_in | RES_exit_late;
  }

  delete succs;
}


/**********************************************************************
 * sens_GEN()
 */
void
GiveTake::sens_GEN(CfgAnnotPtrIter *succs,
		   GiveTakePtr     liftee)
{
  GEN_entry      = (MOVE_entry_in | RES_entry)
    - succs->interSet((SetAccessFunc) &GiveTake::getMOVE_entry_in);
  GEN_exit       = (MOVE_exit_in | RES_exit)
    - succs->interSet((SetAccessFunc) &GiveTake::getMOVE_exit_in);

  GEN_entry_late = (MOVE_entry_late_in | RES_entry_late)
    - succs->interSet((SetAccessFunc) &GiveTake::getMOVE_entry_late_in);
  GEN_exit_late  = (MOVE_exit_late_in | RES_exit_late)
    - succs->interSet((SetAccessFunc) &GiveTake::getMOVE_exit_late_in);
}


/**********************************************************************
 * operator <<
 */
ostream &
operator << (ostream &os, const GiveTake &gt)
{
  const VectorSet GIVEnSTEAL = gt.GIVE & gt.STEAL;
  Boolean         giveNsteal = (Boolean) !GIVEnSTEAL.isEmpty();

  os << "    " << gt.name << ":";
  os << "\n\tSTEAL_loc: " << gt.STEAL_loc <<
    "\tGIVE_loc:   " << gt.GIVE_loc <<
      "\tBLOCK_loc:   " << gt.BLOCK_loc <<
	"\tTAKE_loc:" << gt.TAKE_loc;
  os << "\n\tSTEAL:     " << gt.STEAL <<
    "\tGIVE:       " << gt.GIVE <<
      "\tBLOCK:       " << gt.BLOCK;
  os << "\n\tSTEAL_init:" << gt.STEAL_init <<
    "\tGIVE_init:  " << gt.GIVE_init <<
      "\tTAKE_init: " << gt.TAKE_init;

  if (giveNsteal)
  {
    os << "\n    Note: GIVEnSTEAL = " << GIVEnSTEAL;
  }

  os << "\n\tTAKE:      " << gt.TAKE <<
    "\tTAKEN:      " << gt.TAKEN <<
      "\tTAKEN_out:   " << gt.TAKEN_out;
  os << "\n\tGIVEN_in:  " << gt.GIVEN_in <<
    "\tGIVEN:      " << gt.GIVEN <<
      "\tGIVEN_out:   " << gt.GIVEN_out;
  os << "\n\tGIVEN_l_in:" << gt.GIVEN_late_in <<
    "\tGIVEN_l:    " << gt.GIVEN_late <<
      "\tGIVEN_l_out: " << gt.GIVEN_late_out;
  os << "\n\tRES_in:    " << gt.RES_entry <<
    "\tMOVE_in_in: " << gt.MOVE_entry_in <<
      "\tMOVE_in_out: " << gt.MOVE_entry_out;
  os << "\n\tRES_out:   " << gt.RES_exit <<
    "\tMOVE_out_in:" << gt.MOVE_exit_in <<
      "\tMOVE_out_out:" << gt.MOVE_exit_out;
  os << "\n\tRES_in_l:  " << gt.RES_entry_late <<
    "\tMOVE_in_l_in:" << gt.MOVE_entry_late_in <<
      "\tMOVE_in_l_out:" << gt.MOVE_entry_late_out;
  os << "\n\tRES_out_l: " << gt.RES_exit_late <<
    "\tMOVE_out_l_in:" << gt.MOVE_exit_late_in <<
      "\tMOVE_out_l_out:" << gt.MOVE_exit_late_out;
  os << "\n\tGEN_in:    " << gt.GEN_entry <<
    "\tGEN_out:" << gt.GEN_exit <<
      "\tGEN_in_l:" << gt.GEN_entry_late <<
	"\tGEN_out_l:" << gt.GEN_exit_late  << endl;

  return os;
}
