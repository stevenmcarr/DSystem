/* $Id: S_desc.h,v 1.14 1997/03/11 14:28:34 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef S_Desc_h
#define S_Desc_h

/**********************************************************************
 * This class defines the S_descriptor (TOC paper, R. Das et al.) 
 * which stores information about the irregular subscripts.
 *
 * Each S_descriptor may represent a reference with several indices
 * (represented by Slices).
 */

/**********************************************************************
 * Revision History:
 * $Log: S_desc.h,v $
 * Revision 1.14  1997/03/11 14:28:34  carr
 * newly checked in as revision 1.14
 *
 * Revision 1.14  94/03/21  13:31:31  patton
 * fixed comment problem
 * 
 * Revision 1.13  94/02/27  20:14:59  reinhard
 * Added value-based distributions.
 * Make CC happy.
 * See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.
 * 
 * Revision 1.17  1994/02/27  19:44:35  reinhard
 * Tweaks to make CC happy.
 *
 * Revision 1.16  1994/01/18  19:50:45  reinhard
 * Updated include paths according to Makefile change.
 * Handle value-based distributions.
 *
 * Revision 1.15  1993/10/06  18:26:55  reinhard
 * Made exec's based on Slices instead of S_desc's.
 *
 */

#include <strstream.h>

#ifndef fortsym_h
#include <libs/frontEnd/fortTree/fortsym.h>
#endif

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

class Exec;
class Inspec;
class IrrGlobals;
class S_desc;
class Slice;
//class S_desc_ht : public AST_ht;
class S_desc_ht;
class S_desc_ht_Iter;

/*------------------- TYPES ---------------------------------*/

typedef S_desc *S_desc_ptr;
typedef void (S_desc::*S_desc_ftype)();
typedef int  *int_ptr;          // used for arrays of ptrs
typedef char *char_ptr;         // used for arrays of ptrs
enum irr_type {lhs, rhs};       // The kind of irregular reference
typedef struct {                // Constructor parameters
  irr_type   kind; 
  IrrGlobals *di;
} S_desc_arg_type;

/*------------------- GLOBAL DECLARATIONS -------------------*/

EXTERN(Generic, genS_desc, (AST_INDEX array_node, Generic gen_arg));
EXTERN(void,    S_desc_apply, (AST_htEntry *entry, va_list ap));


/**********************************************************************
 * class S_desc
 */
class S_desc
{
public:
  S_desc(AST_INDEX my_array_node, irr_type my_kind,  // Constructor
	 IrrGlobals *my_di);
  ~S_desc();                                         // Destructor

  // Access functions
  void        putBit(RefsKeyId my_bit) { bit  = my_bit; }
  //void        putExec(Exec *my_exec)   { exec = my_exec; }
  irr_type    getKind()          const { return kind; }
  AST_INDEX   getArray_node()    const { return array_node; }
  const char  *getArray_name()   const { return array_name; }
  const char  *getStr()          const { return str; }
  RefsKeyId   getBit()           const { return bit; }
  Boolean     getIs_irreg()      const { return is_irreg; }
  //Exec        *getExec()       const { return exec; }
  //AST_INDEX   getLimit_node()  const { return limit_node; }
  int         getRank()          const { return rank; }
  Boolean     getDistrib_irreg() const { return distrib_irreg; }
  Generic     getVd()            const { return vd; }
  const char  *getName()       const; // Lookup name
  fst_index_t getVar()         const; // Lookup symbol table index
  ValNumber   getVn()          const; // Lookup value number
  ValNumber   getSubscVal()    const; // Lookup value number of subscript
  SsaNodeId   getSsaId()       const; // Lookup SSA node id
  CfgNodeId   getCfgId()       const; // Lookup CFG node id
  AST_INDEX   getOutmost_loop_node()  const;
  AST_INDEX   get_subsc_node(int dim = 0) const; // Get subscript AST
  Slice       *getSlice(int dim = 0);            // Get slice
  void        putVd(Generic my_vd);   // Supply ValDecomp info

  void catPortion(ostrstream &buf,
		  int level) const;   // Cat portion into buf
  void genExec();                     // Generate executor
  void genSlices();                   // Fillin data, generate slice
  void tree_changed();                // Adapt to AST changes
  void genInspec();                   // Generate inspector
  //void update_subsc(const char *ivar_name); // Install new subscript
  //void apply(S_desc_ftype action) { (*action)(); }
  void apply(S_desc_ftype action) { (this->*action)(); }

private:
  const char  *array_name;       // Name of data array; eg, "x"
  AST_INDEX   array_node;        // Data array
  RefsKeyId   bit;               // Bit position (used for dataflow)
  IrrGlobals  *di;               // Global vars of Dist Mem compiler
  Boolean     is_irreg;          // Is it irregular ?
  irr_type    kind;              // lhs / rhs
  const char  *str;              // Reference string; eg, "x(n1)"
  AST_INDEX   subsc_list;        // Subscripts (in original program)
                                 //   eg. 'n1' (=> n1$arr(1:100))
  Generic     vd;                // ValDecomp for irregular distribution
  Boolean     distrib_irreg;     // Is array distributed irregularly ?
  int         rank;              // # of subscripts
  Slice       *slices;           // <rank> slices
  //Exec        *exec;             // Executor (starts at <limit_node>)
  //AST_INDEX   limit_node;        // Outermost limit of subscript slices
};


/**********************************************************************
 * An Iterator.  Usage:
 *
 *    S_desc_ht *ht;
 *    S_desc    *elmt;
 *    ...
 *    for (S_desc_ht_Iter iter(ht); elmt = iter();) { ... }
 *
 */
class S_desc_ht_Iter : public AST_ht_Iter {
public:
  S_desc_ht_Iter(S_desc_ht* my_ht)
    : AST_ht_Iter ((AST_ht*) my_ht) {}

  S_desc *operator() () {
    return (S_desc *) AST_ht_Iter::operator()();
  }
};


/**********************************************************************
 * 1/29/93 RvH: wait until templates are supported ...
 */
class S_desc_ht : public AST_ht {
public:
  // Constructor
  S_desc_ht(IrrGlobals *di) : AST_ht (genS_desc, (Generic) di)
  {
    S_desc_arg_type* my_gen_arg = new S_desc_arg_type;
    my_gen_arg->di = di;
    gen_arg        = (Generic) my_gen_arg;
  };
  
  ~S_desc_ht() {                           // Destructor
    S_desc *elmt;
    S_desc_ht_Iter iter(this);

    delete (S_desc_arg_type*) gen_arg;
    while (elmt = iter()) delete elmt;
  }
  
  void     add_entry(AST_INDEX my_key_node, S_desc *my_data) {
    AST_ht::add_entry(my_key_node, (Generic) my_data);
  }
  
  Boolean  query_entry(AST_INDEX query_node) const {
    return (Boolean) AST_ht::query_entry(query_node);
  }
  
  S_desc   *get_entry_by_AST(AST_INDEX query_node) {
    return (S_desc *) AST_ht::get_entry_by_AST(query_node);
  }
  
  S_desc   *gen_entry_by_AST(AST_INDEX query_node, irr_type my_kind) {
    ((S_desc_arg_type*) gen_arg)->kind = my_kind;
    return (S_desc *) AST_ht::gen_entry_by_AST(query_node);
  }
  
  //void forall(S_desc_ftype forall_func)
  //{
  //  AST_ht::forall(S_desc_apply, (Action_ftype) forall_func);
  //}
};
#endif
