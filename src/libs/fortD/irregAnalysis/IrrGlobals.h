/* $Id: IrrGlobals.h,v 1.14 1997/03/11 14:28:30 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef IrrGlobals_h
#define IrrGlobals_h

/**********************************************************************
 * This class contains globals needed by the Irregular part of the
 * Distribted Memory compiler. 
 * (Generic) dh->irr points to an instance of this class;  somewhat ugly, but it
 * allows us to hook irregular globals into Dist_Globals w/o making 
 * the defintion of Dist_Globals dependent on Irr_Globals.
 *
 * NOTE
 * ====
 * As of Dec. '92, there are two symbol table structures around in the
 * Dist Mem world, 
 *
 * 1. the symbol table created within dc ("dc-table"), containing SNODE's,
 *
 * 2. the global symbol table created by John Mellor-Crummey ("global table").
 *
 * Right now there is some redundancy between these, and they are not
 * kept consisten w/ each other.  Eventually, only the global table should
 * survive, augmented w/ a trimmed down version of SNODES which only contains
 * dist-mem specific info.
 * To facilitate this later move, and b/c it's a good idea to keep this
 * kind of functionality in one place anyway, everything accessing the
 * symbol table should be a method of IrrGlobals.
 *
 * 1/24/93 RvH: Note that a consistent symbol table is also needed for
 * the generation of cfg/ssa information.
 */

/**********************************************************************
 * Revision History:
 * $Log: IrrGlobals.h,v $
 * Revision 1.14  1997/03/11 14:28:30  carr
 * newly checked in as revision 1.14
 *
 * Revision 1.14  94/03/21  13:29:25  patton
 * fixed comment problem
 * 
 * Revision 1.13  94/02/27  20:14:33  reinhard
 * Added value-based distributions.
 * Make CC happy.
 * See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.
 * 
 * Revision 1.18  1994/02/27  19:42:29  reinhard
 * Tweaks to make CC happy.
 *
 */


/**********************************************************************
 *
 * - To improve compile time and to keep things as untangled as
 *   possible, the files included here should only be the ones which
 *   absolutely have to be included whenever IrrGlobals.h is included.
 *
 *   I.e., if a file <file.C> already includes IrrGlobals.h and needs
 *   an additional file <inc.h>, but <inc.h> is not necessarily needed
 *   by other files including IrrGlobals.h, then <inc.h> should just
 *   be included by <file.C> and not be placed here.
 *
 * - Of course, IrrGlobals.h should only be included by those files
 *   which really need class IrrGlobals.
 *
 *   I.e., IrrGlobals.h should NOT be included just b/c it includes
 *   all these other header files.
 */

#ifndef context_h
#include <libs/support/database/context.h>
#endif

#ifndef FortTextTree_h
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#endif

#ifndef val_enum_h
#include <libs/moduleAnalysis/valNum/val_enum.h>
#endif

#ifndef cfg_h
#include <libs/moduleAnalysis/cfg/cfg.h>
#endif

#ifndef Mall_h
#include <libs/fortD/irregAnalysis/Mall.h>
#endif

/*-------------------- GLOBAL DECLARATIONS --------------------------*/

class AST_SSA_Graph;
class AST_Set;
class CommDFProblem;
class Exec_ht;
class Fd_opts;
class FortranDInfo;
class Inspec_ht;
class IrrSymTab;
class OnHomeTable;
class S_desc_ht;
typedef struct snode SNODE;

/**********************************************************************
 * class IrrGlobals
 */
class IrrGlobals
{
public:
  IrrGlobals(FortTree     my_ft,                  // Constructor
	     FortTextTree my_ftt, 
	     Fd_opts      *my_fd_opts,
	     FortranDInfo *my_fd);
  ~IrrGlobals();                                  // Destructor 

  // Access functions
  AST_INDEX getRoot_node()      const { return root_node; }
  Boolean   getDo_all_arrays()  const;
  Boolean   getDo_irreg()       const { return do_irreg; }
  Boolean   getIs_main()        const { return is_main; }
  Boolean   getSplit_comm()     const;
  Boolean   getSave_irreg()     const;
  const Fd_opts *getFd_opts()   const { return fd_opts; }
  const Boolean *getFlags()     const { return flags; }
  FortTextTree  getFtt()        const { return ftt; }
  CfgInstance   getCfg()        const { return cfg; }
  Values        getVal()        const;
  AST_SSA_Graph *getSsa_graph() const { return ssa_graph; }
  S_desc_ht     *getRefs()          { return refs; }
  Inspec_ht     *getInsps()         { return insps; }
  Exec_ht       *getExecs()         { return execs; }
  CommDFProblem *getComm_df() const { return comm_df; }
  IrrSymTab     *getSt()            { return st; }
  AST_Set       *getExec_loops()    { return exec_loops; }
  OnHomeTable   *getOn_home_table() { return on_home_table; }
  Generic       getHidden_ValDecompInfo() const {
    return hidden_ValDecompInfo; }
  void          setNeed_mall(int type) { need_mall[type] = true; }

  char    *Ast2Str(AST_INDEX node) const;   // Unparse AST index
  void    collect();                        // Collect refs
  void    analyze();                        // Do all analysis
  void    gen_code();                       // Do the transformation
  void    add_init_stmt(AST_INDEX node);         // Add init stmt
  Boolean query_irreg_ref(AST_INDEX node) const; // Query <refs>
  Boolean needs_reg_comm(AST_INDEX node);        // Need reg comm ?
  void    register_loops(AST_INDEX loop_node);
  char    *user_dist_up_str(SNODE *sp);   // Compute an upper bound
  void    do_mall();              // Handle dynamic arrays

private:
  Boolean ssa_init(Boolean with_values = false);  // Compute ssa/cfg info
  void    ssa_delete();           // Destroy ssa/cfg info
  void    init_options ();        // Initialize flags
  Boolean check_module ();        // (Type-)check module
  Boolean collect_refs ();        // Collect (irreg) refs
  void    collect_exec_loops();   // Collect all exec loops
  AST_INDEX mall_size_node(int type); // Pick work array size
  AST_INDEX find_ex_stmt() const; // Find first executable node

  // Private fields:
  CfgInstance   cfg;            // Access to cfg info
  CommDFProblem *comm_df;       // Communication data flow information
  Boolean       do_irreg;       // Do irregular part ?
  AST_Set       *exec_loops;    // Set of executor loops
  Exec_ht       *execs;         // Hash table of executors
  FortranDInfo  *fd;            // General Fortran D info
  Fd_opts       *fd_opts;       // Command line options
  Boolean       *flags;         // Command line option flags
  FortTree      ft;             // Fortran tree
  FortTextTree  ftt;            // Text/structure of FortTree
  AST_INDEX     init_node;      // Node for initialization stmts
  Inspec_ht     *insps;         // Hash table of inspectors
  Mall          *mall;          // Dynamic array handling
  Boolean       need_mall[MallType_cnt];  // Any dynamic <type> arrs ?
  OnHomeTable   *on_home_table; // ON_HOME info
  S_desc_ht     *refs;          // Hash table of irreg refs
  AST_INDEX     root_node;      // Root of program AST
  AST_SSA_Graph *ssa_graph;     // Access to use/def info
  IrrSymTab     *st;            // Access to symbol table info
  Generic       hidden_ValDecompInfo;  // Value based decompositions
  Boolean       is_main;        // Is this the main procedure ?
};

typedef IrrGlobals *IrrGlobals_ptr;

#endif
