/* $Id: CommDFProblem.h,v 1.8 1997/03/11 14:28:25 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef CommDFProblem_h
#define CommDFProblem_h

/**********************************************************************
 * Communication data flow analysis.
 */

/**********************************************************************
 * Revision History:
 * $Log: CommDFProblem.h,v $
 * Revision 1.8  1997/03/11 14:28:25  carr
 * newly checked in as revision 1.8
 *
 * Revision 1.8  94/03/21  13:25:10  patton
 * fixed comment pro
 * blem
 * 
 * Revision 1.7  94/02/27  20:14:18  reinhard
 * Added value-based distributions.
 * Make CC happy.
 * See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.
 * 
 * Revision 1.11  1994/02/27  19:39:15  reinhard
 * Tweaks to make CC happy.
 *
 * Revision 1.10  1994/01/18  19:45:13  reinhard
 * Updated include paths according to Makefile change.
 * Handle value-based distributions.
 *
 */

#ifndef  IntervalDFProb_h
#include <libs/fortD/irregAnalysis/IntervDFProb.h>
#endif

/*------------------- EXTERNAL DECLARATIONS -----------------*/

class S_desc_ht;
class RefsKeys;

/*------------------- FORWARD DECLARATIONS ------------------*/

class CommDFAnnot;
class CommDFProblem;
class IrrSymTab;

/*------------------- TYPES ---------------------------------*/

typedef const VectorSet  &(CommDFAnnot::*CommDFAnnotSetAccessFunc)();
typedef GiveTakePtr (CommDFAnnot::*CommDFAnnotGtAccessFunc)();
typedef void        (CommDFAnnot::*CommDFInsensFunc)(CfgAnnotPtrIter
						     *meet_annot_iter);
typedef void       (CommDFAnnot::*CommDFSensFunc)(CfgAnnotPtrIter
						  *meet_annot_iter,
						  CfgAnnotPtr annot_v);
typedef void       (CommDFAnnot::*CommDFAnnotMethod)();
typedef CommDFAnnot *CommDFAnnotPtr;

enum Comm_dir { Send, Recv, Both };
extern const char *Comm_dir_names[];

// Gt_cnt gives the # of GiveTake instances
enum Gt_index { Gather, Scatter, Scatter_add, Scatter_mult, Gt_cnt };

extern const char *Gt_names[];
extern const Boolean Gt_is_red[];
extern const Boolean Gt_forward[];
extern const int Gt_init_red[];


/**********************************************************************
 * class CommDFProblem   Communication data flow problem
 */
class CommDFProblem : public IntervalDFProblem {
public:
  CommDFProblem(CfgInstance   my_cfg,         // Constructor
		S_desc_ht     *refs,
		IrrSymTab     *st,
		Boolean       my_split_comm,
		Boolean       my_high_level = true); 
  ~CommDFProblem();                           // Destructor

  // Access functions
  Boolean     getSplit_comm() const { return split_comm; }
  Boolean     getHigh_level() const { return high_level; }
  RefsKeys    *getRefs_keys() const { return refs_keys; };
  Gt_index    getDump_i()     const { return dump_i; };
  CommDFAnnot *getAnnot(int cn) const {
    return (CommDFAnnot*) IntervalDFProblem::getAnnot(cn); };

  // Virtual functions from IntervalDFProblem
  CfgAnnotPtr init_annot(CfgNodeId node);
  void        apply_insens(InsensFunc insens, CfgAnnotPtr me,
			   CfgAnnotPtrIter *meet_annots);
  void        apply_sens(SensFunc        sens,
			 CfgAnnotPtr     me_v,
			 CfgAnnotPtrIter *meet_annots,
			 CfgAnnotPtr     liftee_v);

  void        forall_annots(CommDFAnnotMethod method,
			    TraversalOrder    order = PreOrder) const;
  void        solve();                        // Compute annotations
  AST_INDEX   get_first_GATHER_node() const;  // Find 1st GATHER
  void        gen_comm();
  const char  *get_red_init_ind_name();
  void        dump(Gt_index my_dump_i = Gt_cnt,
		   Boolean all = false);
  void        dump_cfg();

private:
  void        iter_reset(TraversalOrder     // Iterating annotations
			 order = PreOrder) const;
  CommDFAnnot *iter_next() const;           // Get next annotation

  // Private fields
  Gt_index          dump_i;         // Which GiveTake instance to dump
  Boolean           high_level;     // Generate high level comm. stmts ?
  IntervalGraphIter *iter;          // Needed for iterating annotations
  const char        *red_init_ind_name; // Ind var for reduction init
  RefsKeys          *refs_keys;     // Data flow universe
  Boolean           split_comm;     // Split sends/recvs ?
};


/**********************************************************************
 * class CommDFAnnot   Communication data flow annotation for CFG node
 */
class CommDFAnnot : public CfgAnnot {  
public:
  CommDFAnnot(CommDFProblem *my_problem, CfgNodeId my_cn); // Constructor
  
  // Access functions
  const RefsKeys *getRefs_keys () const { return refs_keys; }
  GiveTake *getGt(Gt_index i) { return &gt[i]; }
  VectorSet getAll() const;
  
  void      initGATH();
  void      initSCATT();
  void      dump              () const { dump(Gt_cnt); }
  void      dump              (Gt_index i) const;
  NODE_TYPE get_reduction_type() const;
  void      gen_comm_stmts    ();
  void      insert_comm_stmts();
  void      place_Inspecs();
  void      gen_red_init_stmts();

private:
  void      gen_send_recv(const char *name,
			  Boolean    prepend,
			  VectorSet  send_set,
			  VectorSet  recv_set) const;
  void      gen_comm_stmt(const VectorSet &SET, const char *name,
			  Comm_dir comm, Boolean prepend) const;
  AST_INDEX gen_comm_node(const VectorSet &SET,
			  Comm_dir comm, const char *name) const;
  void      add_comm_node(AST_INDEX node, Boolean prepend);
  CfgNodeId find_comm_cn(Boolean &prepend) const;

  // Private fields
  VectorSet     ADD;            // Local data flow variables
  VectorSet     DEF;
  VectorSet     DEF_NODUP;
  VectorSet     IND;
  VectorSet     MULT;
  VectorSet     RED;
  VectorSet     REF;
  AST_INDEX     append_node;    // Communications to append
  CommDFProblem *const comm_problem;
  GiveTake      gt[Gt_cnt];
  AST_INDEX     prepend_node;   // Communications to prepend
  RefsKeys      *refs_keys;
  AST_INDEX     target_node;    // Where to prepend inspector
};
#endif
