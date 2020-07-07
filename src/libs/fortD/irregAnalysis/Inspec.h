/* $Id: Inspec.h,v 1.14 1997/03/11 14:28:28 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef Inspec_h
#define Inspec_h

/**********************************************************************
 * This class defines the Inspector for one or several references
 *
 * NOTE: There is at most ONE inspector for each loop; ie.,
 * each inspector may perform preprocessing for SEVERAL indirect
 * references (represented by S_descriptors).
 * Each S_descriptor may represent a reference with several indices
 * (represented by Slices).
 */

/**********************************************************************
 * Revision History:
 * $Log: Inspec.h,v $
 * Revision 1.14  1997/03/11 14:28:28  carr
 * newly checked in as revision 1.14
 *
 * Revision 1.14  94/03/21  13:51:30  patton
 * fixed comment problem
 * 
 * Revision 1.13  94/02/27  20:14:25  reinhard
 * Added value-based distributions.
 * Make CC happy.
 * See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.
 * 
 * Revision 1.11  1994/02/27  19:41:26  reinhard
 * Tweaks to make CC happy.
 *
 * Revision 1.10  1993/10/04  15:36:08  reinhard
 * Extended class SubscValEntry.
 *
 * Revision 1.9  1993/09/25  15:32:21  reinhard
 * Split new_subsc_name into new_subsc_loc_name, new_subsc_glob_name.
 *
 * Revision 1.8  1993/09/02  18:46:50  reinhard
 * Pass ostrstream's by reference now.
 *
 * Revision 1.7  1993/07/13  23:02:59  reinhard
 * Added new_subsc_name field.
 *
 * Revision 1.6  1993/06/09  23:46:13  reinhard
 * Inspectors are now based on Slices, not on S_desc's.
 *
 */

#include <strstream>

using namespace std;

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef rn_varargs_h
#include <include/rn_varargs.h>
#endif

#ifndef AST_ht_h
#include <libs/frontEnd/ast/AST_ht.h>
#endif


/*-------------------- GLOBAL DECLARATIONS --------------------------*/

class IrrGlobals;
class Slice; 
class S_desc; 
class Sparse_Set;
class SubscValEntry;

/*------------------- FORWARD DECLARATIONS ------------------*/

class Inspec;


/*------------------- TYPES ---------------------------------*/

typedef Inspec *Inspec_ptr;
typedef void (Inspec::*Inspec_ftype)();
 

/*------------------- GLOBAL DECLARATIONS -------------------*/

EXTERN(Generic, genInspec, (AST_INDEX target_node, Generic gen_arg));
EXTERN(void,    Inspec_apply, (AST_htEntry *entry, va_list ap));


/**********************************************************************
 * class Inspec
 */
class Inspec
{
public:
  Inspec(AST_INDEX my_target_node, IrrGlobals *my_di); // Constructor
  ~Inspec();                                           // Destructor

  // Access functions
  AST_INDEX getTarget_node() { return target_node; }

  void      add_Slice(Slice *slice);    // Add stmts from <slice>
  void      install();                  // Generate & install inspec
  void      catSlicesBuf(ostrstream &slices_buf); // Generate string of slices
  void      apply(Inspec_ftype action) { (this->*action)(); }

private:
  Slice     *ve2sl(SubscValEntry *ve);    // Pick a slice for ve
  int       ve2sl_cnt(SubscValEntry *ve); // # of slices for ve
  void      sort_slices();                // Sort out the given slices
  void      declXpandedVars();            // Declare expanded vars
  void      genStmts_node();              // Generate the stmts
  AST_INDEX gen_inspec_cmt();             // Generate header comment

  // Private fields
  IrrGlobals *di;               // Global vars of Irr Dist Mem compiler
  Sparse_Set *slices;         // Set used for initializing inspec
  AST_INDEX  stmts_node;     // Code of inspector
  AST_INDEX  target_node;    // Where to prepend this code
  Sparse_Set *ve_set;           // Set of subscript value infos
  AST_ht     *ve_slices_ht;     // For each ve, set of slices

  //Slice      **slices;       // List of slices for which this is
                             //   the inspector; eg. 'x(n1)'
  //ValNumber  *vals;          // Value #'s of subscripts inspected
  // "si"  = internal index of Slice (several si's per vi possible),
  //         indexing <slices>
  // "vi"  = internal index of subscript value number,
  //         indexing <vals>, <sched_name>, <loc_cnt_name>, etc.
  // "vii" = internal index of slice for a subscript value number,
  //         indexing 2nd dimension of <vi2sis>
  //int        si_cnt;           // # of slices
  //int        vi_cnt;           // # of different <vals>
  //int        *si2vi;           // vi for this si
  //int        *vii_cnt;         // vii_cnt[vi] = # slices for val vals[vi]
  //int        **vi2sis;         // si's for this vi
  //char       **new_subsc_loc_name; // Name of new subscript ("n1$loc")
  //char       **new_subsc_glob_name; // Name of new subscript ("n1$glob")
  //char       **sched_name;     // Name of schedule ("x$sched")
  //char       **loc_cnt_name;   // Local # elemnts ("x$loc_cnt")
};
 

/**********************************************************************
 * 012993RvH: wait until templates are supported ...
 */
class Inspec_ht : public AST_ht {
public:
  // Constructor
  Inspec_ht(IrrGlobals *di) : AST_ht (genInspec, (Generic) di) {};

  void     add_entry(AST_INDEX key_node, Inspec *data) {
    AST_ht::add_entry(key_node, (Generic) data);
  }
  Boolean  query_entry(AST_INDEX query_node) {
    return (Boolean) AST_ht::query_entry(query_node);
  }
  Inspec   *get_entry_by_AST(AST_INDEX query_node) {
    return (Inspec *) AST_ht::get_entry_by_AST(query_node);
  }
  Inspec   *gen_entry_by_AST(AST_INDEX query_node) {
    return (Inspec *) AST_ht::gen_entry_by_AST(query_node);
  }
  //void forall(Inspec_ftype forall_func) {
  //  AST_ht::forall(Inspec_apply, (Action_ftype) forall_func);
  //}
};
#endif
