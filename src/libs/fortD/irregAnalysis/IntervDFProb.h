/* $Id: IntervDFProb.h,v 1.8 1997/03/11 14:28:29 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef IntervalDFProb_h
#define IntervalDFProb_h

/**********************************************************************
 * Interval data flow analysis.
 */

/**********************************************************************
 * Revision History:
 * $Log: IntervDFProb.h,v $
 * Revision 1.8  1997/03/11 14:28:29  carr
 * newly checked in as revision 1.8
 *
 * Revision 1.8  94/03/21  13:19:04  patton
 * fiexed comment problem
 * 
 * Revision 1.7  94/02/27  20:14:27  reinhard
 * Added value-based distributions.
 * Make CC happy.
 * See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.
 * 
 * Revision 1.9  1994/02/27  19:42:00  reinhard
 * Robustified GiveNTake.
 *
 * Revision 1.8  1994/01/18  19:47:28  reinhard
 * Updated include paths according to Makefile change.
 *
 * Revision 1.7  1993/09/02  18:47:44  reinhard
 * Robustified GiveNTake framework.
 *
 * Revision 1.6  1993/07/13  23:03:52  reinhard
 * Added support for entry/exit code generation.
 * Major overhaul of framework.
 *
 */

#include <iostream.h>
#ifndef general_h
#include <libs/support/misc/general.h>
#endif
#ifndef   Cfg_h
#include <libs/moduleAnalysis/cfg/Cfg.h>
#endif
#ifndef _VECTOR_SET_
#include <libs/support/sets/VectorSet.h>
#endif


/*------------------- FORWARD DECLARATIONS ------------------*/

class IntervalDFProblem;
class CfgAnnotPtrIter;
class CfgAnnot;
class GiveTake;

/*------------------- TYPES ---------------------------------*/

typedef CfgAnnot  *CfgAnnotPtr;
typedef GiveTake  *GiveTakePtr;
typedef void      *CfgNodeAnnotPtr;

// Another hack to make up for the lack of templates:
// The following types are just for documentary purposes and meant
// to be casted away with actual types defined in derived classes.

// 3/22/93 RvH: Why does the compiler break down on this cast ?
typedef void       (CfgAnnot::*InsensFunc)(CfgAnnotPtrIter *meet_annots);
typedef void       (CfgAnnot::*SensFunc)(CfgAnnotPtrIter *meet_annots);
//typedef const void  *InsensFunc; // Do flow insensitive analysis on node
//typedef const void  *SensFunc;   // Do flow sensitive analysis on node
//typedef const void  *SetAccessFunc;
typedef const void (GiveTake::*SetAccessFunc)();
typedef const void  *GtAccessFunc;
typedef const VectorSet  &(GiveTake::*GtSetAccessFunc)();
typedef void  (GiveTake::*GtSensFunc)(CfgAnnotPtrIter *meet_annot_iter,
				      GiveTakePtr annot_v);

typedef const VectorSet &(*SetAccessApplyFunc)(SetAccessFunc  set_access_v,
					       CfgAnnotPtr    annot_v);
typedef GiveTakePtr (*GtAccessApplyFunc)(GtAccessFunc  Gt_access_v,
					 CfgAnnotPtr   annot_v);
typedef void      *AccessApplyFunc;

// 3/29/93 RvH: why does this not work ?
//typedef VectorSet &(VectorSet::*SetReduceOp)(VectorSet &);
typedef  enum { Union, Intersect } SetReduceOp;
typedef  enum { TarjFirstChild, TarjLastChild, TarjHeader, TarjExitHeader, Any } LifteeType;


/*------------------- CONSTANTS -------------------------------*/

static const GtAccessFunc  GtAccessNil = (GtAccessFunc) -1;


/**********************************************************************
 * class IntervalDFProblem
 */
class IntervalDFProblem {			 
public:
  IntervalDFProblem(CfgInstance        my_cfg,           // Constructor
		    SetAccessApplyFunc my_set_access_apply,
		    GtAccessApplyFunc  my_gt_access_apply);
  virtual             ~IntervalDFProblem();              // Destructor

  // Access functions
  CfgInstance         getCfg()         const { return cfg; };
  TarjTree            getIntervals()   const { return intervals; };
  CfgAnnotPtr         getAnnot(int cn) const;
  void                putAnnot(int cn, CfgAnnotPtr ann);

  // Pure virtual functions
  virtual void        apply_insens(SensFunc        insens,
				   CfgAnnotPtr     me_v,
				   CfgAnnotPtrIter *meet_annots) = 0;
  virtual void        apply_sens(SensFunc        sens,
				 CfgAnnotPtr     me_v,
				 CfgAnnotPtrIter *meet_annots,
				 CfgAnnotPtr     liftee_v) = 0;
  virtual CfgAnnotPtr init_annot(CfgNodeId  node) = 0;

  void            init_annots(TraversalOrder order = PreOrder);
  void            give_n_take(GtAccessFunc gt_access);

  CfgAnnotPtrIter *gen_meet_annots(CfgNodeId cn,
				   Meetees meetees) const;
  CfgAnnotPtrIter *gen_meet_gts(CfgNodeId cn,
				Meetees meetees) const;
  GiveTake        *ann2gt(CfgAnnot *ann);
  Boolean         is_liftee(CfgNodeId cn, LifteeType type) const;
  CfgNodeId       get_liftee(CfgNodeId cn, LifteeType type) const;
  CfgAnnot        *get_liftee_ann(CfgNodeId  cn,
				  LifteeType do_lift,
				  LifteeType is_lift = Any) const;
  GiveTake        *get_liftee_gt(CfgNodeId  cn,
				 LifteeType do_lift,
				 LifteeType is_lift = Any);
  void            dump_annots(TraversalOrder order = PreOrder) const;

protected:
  CfgInstance cfg;        // Control Flow Graph
  TarjTree    intervals;  // Tarjan intervals
  int         level_max;  // Max loop nesting level
  int         node_cnt;   // # control flow nodes
  CfgAnnotPtr *annot;     // Annotations for each control flow node

  // Protected methods
  SetAccessApplyFunc set_access_apply; // Helper to retrieve sets
  GtAccessApplyFunc  gt_access_apply;  // Helper to retrieve GiveTake's
  GtAccessFunc       cur_gt_access;    // Access to current GiveTake's
  void        gt_solve_combined(GtSensFunc     sens1,
				GtSensFunc     sens2,
				TraversalOrder order = PreOrder);
  void        gt_solve_sens(GtSensFunc     sens,
			    TraversalOrder order = PreOrder,
			    int            only_level = -1);
  void        solve_sens(SensFunc sens,
			 TraversalOrder order = PreOrder);
  void        solve_insens(InsensFunc insens,
			   TraversalOrder order = PreOrder);
  LifteeType  select_LiftPredicate(TraversalOrder order);
  LifteeType  select_LifteeFunc(TraversalOrder order);
};


/**********************************************************************
 * class CfgAnnot
 */
class CfgAnnot {
public:
  // Constructor
  CfgAnnot(IntervalDFProblem *my_problem, CfgNodeId my_cn);

  // Access functions
  IntervalDFProblem *const getProblem () const { return problem; }
  CfgNodeId  getCn  ()  const { return cn; }
  Boolean    is_root()  const;
  Boolean    is_end()   const;
  Boolean    no_node()  const { return (Boolean) (node == AST_NIL); }
  int        getLevel() const;

  // Virtual functions
  virtual  VectorSet getAll() const = 0;
  virtual  void      dump  () const = 0;

  Boolean         is_liftee(LifteeType type);
  GiveTake        *get_liftee_gt(LifteeType type);
  CfgAnnotPtrIter *gen_meet_gts(Meetees meetees) const;

protected:
  IntervalDFProblem *const problem;
  CfgNodeId  cn;           // Current CFG node
  AST_INDEX  node;         // Corresponding AST node (may be AST_NIL)
};


/**********************************************************************
 * class CfgAnnotPtrIter  An iterator over the annotations of the
 *                        predecessors/successors of a CFG node.
 */
class CfgAnnotPtrIter {
public:
  // Constructor
  CfgAnnotPtrIter(const CfgInstance  &cfg,
		  CfgNodeId          cn,
		  const IntervalDFProblem  *my_problem,
		  AccessApplyFunc    my_access_apply,
		  Meetees            meetees,
		  GtAccessFunc       my_gt_access = GtAccessNil);

  void        reset(CfgNodeId cn = CFG_NIL);
  CfgAnnotPtr operator() ();
  VectorSet   reduceSet(SetAccessFunc set_access, SetReduceOp op);
  VectorSet   interSet(SetAccessFunc set_access);
  VectorSet   unionSet(SetAccessFunc set_access);

private:
  CfgMeetIter             iter;
  const IntervalDFProblem *problem;
  AccessApplyFunc         access_apply;
  GtAccessFunc            gt_access;

  const VectorSet &get_set(SetAccessFunc set_access, CfgAnnotPtr ann);
};


/**********************************************************************
 * class GiveTake   Annotation for a Gice-and-Take data flow pass.
 */
class GiveTake {
  friend ostream &
    operator << (ostream &os, const GiveTake &gt);
  
public:
  GiveTake();  // Constructor
  
  // Access functions
  const CfgAnnot *const getAnn  () const { return ann; }
  const char     *const getName () const { return name; }
  Boolean         getForward    () const { return forward; }
  const VectorSet &getBLOCK     () const { return BLOCK; }
  const VectorSet &getGIVE_loc  () const { return GIVE_loc; }
  const VectorSet &getTAKEN     () const { return TAKEN; }
  const VectorSet &getGIVEN_in  () const { return GIVEN_in; }
  const VectorSet &getGIVEN     () const { return GIVEN; }
  const VectorSet &getGIVEN_out () const { return GIVEN_out; }
  const VectorSet &getSTEAL_loc () const { return STEAL_loc; }
  const VectorSet &getSTEAL_out_loc () const { return STEAL_out_loc; }
  const VectorSet &getBLOCK_loc () const { return BLOCK_loc; }
  const VectorSet &getTAKE      () const { return TAKE; }
  const VectorSet &getTAKE_loc  () const { return TAKE_loc; }
  const VectorSet &getGEN_entry () const { return GEN_entry; }
  const VectorSet &getGEN_exit  () const { return GEN_exit; }
  const VectorSet &getMOVE_entry_in  () const { return MOVE_entry_in; }
  const VectorSet &getMOVE_exit_in   () const { return MOVE_exit_in; }
  const VectorSet &getMOVE_entry_out () const { return MOVE_entry_out; }
  const VectorSet &getMOVE_exit_out  () const { return MOVE_exit_out; }
  
  const VectorSet &getGIVEN_late_in  () const { return GIVEN_late_in; }
  const VectorSet &getGIVEN_late_out () const { return GIVEN_late_out; }
  const VectorSet &getGEN_entry_late () const { return GEN_entry_late; }
  const VectorSet &getGEN_exit_late  () const { return GEN_exit_late; }
  const VectorSet &getMOVE_entry_late_in () const { return MOVE_entry_late_in; }
  const VectorSet &getMOVE_exit_late_in  () const { return MOVE_exit_late_in; }
  const VectorSet &getMOVE_entry_late_out () const { return MOVE_entry_late_out; }
  const VectorSet &getMOVE_exit_late_out  () const { return MOVE_exit_late_out; }
  void dump() const { cout << *this; }
  
  void init(CfgAnnot *my_ann, const char *my_name);
  void instantiate(Boolean my_forward, VectorSet my_TAKE_init,
		   VectorSet my_STEAL_init,
		   VectorSet my_GIVE_init);
  // 2/17/94 RvH: constructor as default argument not implemented yet !
  void instantiate(Boolean my_forward, VectorSet my_TAKE_init,
		   VectorSet my_STEAL_init)
  {
    VectorSet my_GIVE_init;
    instantiate(my_forward, my_TAKE_init, my_STEAL_init, my_GIVE_init);
  }
  void sens_STEAL(CfgAnnotPtrIter *meet_annots, GiveTakePtr liftee);
  void sens_TAKEN(CfgAnnotPtrIter *meet_annots, GiveTakePtr liftee);
  void sens_GIVEN(CfgAnnotPtrIter *meet_annots, GiveTakePtr liftee);
  void sens_RES(CfgAnnotPtrIter *meet_annots, GiveTakePtr liftee);
  void sens_GEN(CfgAnnotPtrIter *meet_annots, GiveTakePtr liftee);
  
private:
  CfgAnnot   *ann;
  const char *name;
  Boolean   forward;
  VectorSet STEAL;                   // Local data flow variables
  VectorSet TAKE;
  VectorSet GIVE;
  VectorSet STEAL_init;
  VectorSet TAKE_init;
  VectorSet GIVE_init;
  VectorSet STEAL_loc;              // Global data flow variables
  VectorSet STEAL_out_loc;
  VectorSet GIVE_loc;
  VectorSet BLOCK;
  VectorSet BLOCK_loc;
  VectorSet TAKE_loc;
  VectorSet TAKEN;
  VectorSet TAKEN_out;
  VectorSet GIVEN_in;
  VectorSet GIVEN;
  VectorSet GIVEN_out;
  VectorSet GIVEN_late_in;
  VectorSet GIVEN_late;
  VectorSet GIVEN_late_out;
  VectorSet MOVE_entry_in;
  VectorSet MOVE_exit_in;
  VectorSet MOVE_entry_out;
  VectorSet MOVE_exit_out;
  VectorSet MOVE_entry_late_in;
  VectorSet MOVE_exit_late_in;
  VectorSet MOVE_entry_late_out;
  VectorSet MOVE_exit_late_out;
  VectorSet RES_entry;                    // Result data flow variables
  VectorSet RES_exit;
  VectorSet GEN_entry;
  VectorSet GEN_exit;
  VectorSet RES_entry_late;
  VectorSet RES_exit_late;
  VectorSet GEN_entry_late;
  VectorSet GEN_exit_late;
};
#endif
