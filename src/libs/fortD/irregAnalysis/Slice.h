/* $Id: Slice.h,v 1.13 1997/03/11 14:28:35 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef Slice_h
#define Slice_h

/**********************************************************************
 * Revision History:
 * $Log: Slice.h,v $
 * Revision 1.13  1997/03/11 14:28:35  carr
 * newly checked in as revision 1.13
 *
 * Revision 1.13  94/03/21  13:33:31  patton
 * fixed comment problem
 * 
 * Revision 1.12  94/02/27  20:15:01  reinhard
 * Added value-based distributions.
 * Make CC happy.
 * See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg flies for details.
 * 
 * Revision 1.14  1994/02/27  19:44:51  reinhard
 * Tweaks to make CC happy.
 *
 * Revision 1.13  1994/01/18  19:51:22  reinhard
 * Updated include paths according to Makefile change.
 * Handle value-based distributions.
 *
 * Revision 1.12  1993/10/06  18:27:43  reinhard
 * Made exec's based on Slices instead of S_desc's.
 *
 */

#include <strstream.h>

#ifndef cfgval_h
#include <libs/moduleAnalysis/cfgValNum/cfgval.h>
#endif

#ifndef AST_ht_h
#include <libs/frontEnd/ast/AST_ht.h>
#endif

#ifndef CommDFUniv_h
#include <libs/fortD/irregAnalysis/CommDFUniv.h>
#endif

/*------------------- FORWARD DECLARATIONS ------------------*/

class Inspec;
class Exec;
class S_desc;
class IrrGlobals;

/**********************************************************************
 * This class represents the Slice induced by a certain subscript of
 * an indirect reference (represented by an S_descriptor).
 */
class Slice
{
public:
  Slice();                   // Constructor
  ~Slice();                  // Destructor

  // Access functions
  void       putInsp(Inspec *my_insp) { insp = my_insp; }
  void       putVe(SubscValEntry *my_ve) { ve = my_ve; }
  S_desc     *getRef()        const { return ref; }
  AST_INDEX  getSubsc_node()  const { return subsc_node; }
  const char *getSubsc_name() const { return subsc_name; }
  const char *getLoc_cnt_name() const;         // "x$offsize"
  ValNumber  getVn()          const { return vn; }
  SubscValEntry *getVe()      const { return ve; }
  AST_INDEX  getLimit_node()  const { return limit_node; }
  int        getLimit_level() const { return limit_level; }
  Boolean    getXpnd_flag()   const { return xpnd_flag; }
  Boolean    getAux_flag()    const { return aux_flag; }
  Boolean    getCnt_flag()    const { return cnt_flag; }
  AST_INDEX  getStmts_node()  const { return stmts_node; }
  AST_INDEX  getLoc_size_node() const { return loc_size_node; }
  int        getNew_subsc_size() const { return new_subsc_size; }
  AST_INDEX  getTarget_node()       const;
  const char *getSched_name()       const;
  const char *getTab_name()         const;
  CfgNodeId  get_unique_def_cn()    const;
  const char *get_unique_def_name() const;
  
  void  init(S_desc *my_ref, int my_dim,   // Initialization
	     AST_INDEX my_subsc_node,
	     IrrGlobals *my_di);
  void  genInspec();                       // Add myself to inspec
  void  genExec();                         // Add myself to exec
  void  tree_changed();                    // Adapt to AST changes
  void  catSubscript(ostrstream &buf,
		     int level = -1);        // Print, eg., "1:100:2"
  void  update_subsc(const char *ivar_name); // Update subscript
  AST_INDEX genStmts_node();                 // Determine the Slice
  AST_INDEX gen_localize();
  AST_INDEX gen_extend();

private:
  void      find_limit_node();         // Determine limit of slice
  void      init_subsc();
  void      init_new_subsc();
  int       find_limit_level(Values val, ValNumber vn);
  AST_INDEX gen_count_slice();        // Generate counting slice
  void      genNew_subsc_size();      // Compute size of trace
  const char *getNew_subsc_loc_name() const;   // "n1$loc"
  const char *getNew_subsc_glob_name() const;  // "n1$glob"
  const char *getInd_name() const;             // "n1$cnt" [or "i"]
  const char *getCnt_name() const;             // "n1$cnt" [or "100"]

  // Private fields
  Boolean       aux_flag;       // Loop not normal -> aux ind var
  Boolean       cnt_flag;       // Loop not const -> counting slice
  IrrGlobals    *di;            // Global vars of Irr Dist Mem compiler
  int           dim;            // ... in it's <dim>'th dimension ...
  Exec          *exec;          // Executor (starts at <limit_node>)
  AST_INDEX     glob_size_node; // Global size of subscript; "25"
  Inspec        *insp;          // Inspector (prepended at <target_node>)
  Boolean       ivar_flag;      // Subscript is an induction variable ?
  int           limit_level;    // Loop nesting level of <limit_node>
  // About <limit_node>:
  // xpnd_flag = true  ==>  Need slice from ref back to <limit_node>
  // ivar_flag = true  ==>  Subscript is ind var from <limit_node> loop
  AST_INDEX     limit_node;
  AST_INDEX     loc_size_node;  // Local size of subscript; "100"
  int           new_subsc_size; // Size of new subscript; "25"
  S_desc        *ref;           // This is the slice for refs <ref> ...
  CoVarPair     *rv;            // Loop index variable coefficients
  AST_INDEX     stmts_node;     // Statements in Slice
  const char    *str;           // String of the subscript; eg, "n1+3"
  const char    *subsc_name;    // Name of original subscript; "n1"
  AST_INDEX     subsc_node;     // ... the subscript <subsc_node>
  AST_INDEX     target_node;    // Where to prepend <insp>
  SubscValEntry *ve;            // Info associated w/ vn
  ValNumber     vn;             // Value number of subscript
  Boolean       xpnd_flag;      // Subscript is to be expanded ?
};

typedef Slice *Slice_ptr;

#endif
