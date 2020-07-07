/* $Id: Exec.h,v 1.12 1997/03/11 14:28:27 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef Exec_h
#define Exec_h

/**********************************************************************
 * This class defines the Executor.
 */

/**********************************************************************
 * Revision History:
 * $Log: Exec.h,v $
 * Revision 1.12  1997/03/11 14:28:27  carr
 * newly checked in as revision 1.12
 *
 * Revision 1.12  94/03/21  13:49:14  patton
 * fixed comment problem
 * 
 * Revision 1.11  94/02/27  20:14:24  reinhard
 * Added value-based distributions.
 * Make CC happy.
 * See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.
 * 
 * Revision 1.14  1994/02/27  19:40:43  reinhard
 * Moved from Str_ht to NamedGenericTable.
 *
 * Revision 1.13  1994/01/18  19:46:34  reinhard
 * Updated include paths according to Makefile change.
 *
 * Revision 1.12  1993/10/06  18:26:31  reinhard
 *  Made exec's based on Slices instead of S_desc's.
 *
 */

#include <strstream>

using namespace std;

#ifndef AST_ht_h
#include <libs/frontEnd/ast/AST_ht.h>
#endif

/*------------------- EXTERNAL DECLARATIONS -----------------*/

class Sparse_Set;

/*-------------------- FORWARD DECLARATIONS -------------------------*/

class Exec;
class Slice; 
class IrrGlobals;


/*-------------------- TYPES ----------------------------------------*/

typedef Exec *Exec_ptr;
typedef void (Exec::*Exec_ftype)();


/*-------------------- GLOBAL DECLARATIONS --------------------------*/

EXTERN(Generic, genExec, (AST_INDEX target_node, Generic gen_arg));
EXTERN(void,    Exec_apply, (AST_htEntry *entry, va_list ap));


/**********************************************************************
 * class Exec
 */
class Exec
{
public:
  Exec(AST_INDEX my_loop_node, IrrGlobals *my_di);  // Constructor
  ~Exec();                                          // Destructor

  // Access functions
  AST_INDEX getLoop_node()       { return loop_node; }
  Boolean   getLoop_cnst_flag()  { return loop_cnst_flag; }
  int       getIter_cnt()        { return iter_cnt; }

  void      add_Slice(Slice *sl);     // Note <sl> as irregular
  void      install();                 // Convert loop into executor
  void      apply(Exec_ftype action) { (this->*action)(); }

private:
  AST_INDEX gen_exec_cmt();                // Generate header comment
  //void      sort_slices();               // Sort out the given slices
  void      convert_subs();                // Convert subscripts
  void      catSlicesBuf(ostrstream &slices_buf); // Generate string of slices

  // Private fields
  IrrGlobals *di;              // Global vars of Irr Dist Mem compiler
  int        iter_cnt;         // # of iterations (if constant)
  AST_INDEX  iter_cnt_node;    // AST for # of iterations
  Boolean    loop_cnst_flag;   // Loop has const # of iterations ?
  AST_INDEX  loop_node;        // Header of loop
  Boolean    loop_normal_flag; // Loop is in normal form ?
  Sparse_Set *slices;          // Set used for initializing exec
  AST_INDEX  stmts_node;       // Code of executor
  //S_desc_ptr *slices;            // List of slices for which this is
                               //   the executor; eg. 'x(n1)'
  //int        slice_cnt;          // # of slices
};
 
/**********************************************************************
 * 1/29/93 RvH: wait until templates are supported ...
 */
class Exec_ht : public AST_ht {
public:
  // Constructor
  Exec_ht(IrrGlobals *di) : AST_ht (genExec, (Generic) di) {};

  void     add_entry(AST_INDEX my_key_node, Exec *my_data) {
    AST_ht::add_entry(my_key_node, (Generic) my_data);
  }
  Boolean  query_entry(AST_INDEX query_node) {
    return (Boolean) AST_ht::query_entry(query_node);
  }
  Exec     *get_entry_by_AST(AST_INDEX query_node) {
    return (Exec *) AST_ht::get_entry_by_AST(query_node);
  }
  Exec     *gen_entry_by_AST(AST_INDEX query_node) {
    return (Exec *) AST_ht::gen_entry_by_AST(query_node);
  }
  //void forall(Exec_ftype forall_func)
  //{
  //  AST_ht::forall(Exec_apply, (Action_ftype) forall_func);
  //}
};
#endif
