/* $Id: CommDFUniv.h,v 1.8 1997/03/11 14:28:26 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef CommDFUniv_h
#define CommDFUniv_h

/**********************************************************************
 * Classes used during construction of the universe used by
 * communication data flow analysis.
 */

/**********************************************************************
 * Revision History:
 * $Log: CommDFUniv.h,v $
 * Revision 1.8  1997/03/11 14:28:26  carr
 * newly checked in as revision 1.8
 *
 * Revision 1.8  94/03/21  13:27:19  patton
 * fixed comment problem
 * 
 * Revision 1.7  94/02/27  20:14:22  reinhard
 * Added value-based distributions.
 * Make CC happy.
 * See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.
 * 
 * Revision 1.12  1994/02/27  19:40:04  reinhard
 * Tweaks to make CC happy.
 *
 * Revision 1.11  1994/01/18  19:45:53  reinhard
 * Updated include paths according to Makefile change.
 * Handle value-based distributions.
 *
 * Revision 1.10  1993/10/04  15:33:06  reinhard
 * Extended class SubscValEntry.
 *
 * Revision 1.9  1993/09/25  15:31:23  reinhard
 * Added class SubscValEntry.
 *
 * Revision 1.8  1993/09/02  18:44:58  reinhard
 * Const'ed star(), affects(), etc.
 *
 * Revision 1.7  1993/07/13  23:01:31  reinhard
 * Added no_duplicates() method.
 *
 */

#undef is_open
#include <strstream.h>

#ifndef cfgval_h
#include <libs/moduleAnalysis/cfgValNum/cfgval.h>
#endif

#ifndef _VECTOR_SET_
#include <libs/support/sets/VectorSet.h>
#endif

#ifndef AST_ht_h
#include <libs/frontEnd/ast/AST_ht.h>
#endif

/*------------------- EXTERNAL DECLARATIONS -----------------*/

class IrrSymTab;
class S_desc;
class S_desc_ht;
class Sparse_Set;

/*------------------- FORWARD DECLARATIONS ------------------*/

class RefsKeys;
class RefsKey;
class RefKey;
class ValVar;
class SubscValEntry;
class SubscValEntry_ht;
class SubscValEntry_ht_Iter;

/*------------------- TYPES ---------------------------------*/

typedef int      RefsKeyId;
typedef S_desc   *S_desc_ptr;
typedef RefKey   *RefKey_ptr;
typedef RefsKey  *RefsKey_ptr;
typedef RefsKeys *RefsKeys_ptr;
typedef SubscValEntry *SubscValEntry_ptr;


/*-------------------- GLOBAL DECLARATIONS --------------------------*/

EXTERN(Generic, genSubscValEntry, (AST_INDEX target_node,
				   Generic gen_arg));

/*------------------- CONSTANTS -----------------------------*/

static const RefsKeyId RefsKeyNil = -1;


/**********************************************************************
 *  Sorting key for data flow bit entries
 */
class ValVar
{
public:
  fst_index_t var;    // Symbol table index
  ValNumber   val;    // Value number
};


/**********************************************************************
 *  Info associated w/ a subscript value number
 */
class SubscValEntry
{
public:
  SubscValEntry(ValNumber my_val, RefsKeys *my_refs_keys); // Constructor
  ~SubscValEntry();   // Destructor

  const char *getNew_subsc_loc_name () const {
    return new_subsc_loc_name; }
  const char *getNew_subsc_glob_name() const {
    return new_subsc_glob_name; }
  const char *getLoc_cnt_name       () const { return loc_cnt_name; }
  const char *getAux_cnt_name       () const { return aux_cnt_name; }
  const char *getSched_name         () const { return sched_name; }
  const char *getTab_name           () const { return tab_name; }
  CfgNodeId  getTarget_cn           () const { return target_cn; }
  void       add_target_cn(CfgNodeId next_target_cn);
  void       putKey(RefsKey *my_refs_key) { refs_key = my_refs_key; }
  void       gen_names(const char *subsc_name, const char *array_name);
  void       declXpandedVars(Sparse_Set &slices);

  //void      add_slice(Slice *slice) { slices += (Element) slice; }
  //Slice     *get_slice(int index = 0) const {
  //  assert (slices.count() > index);
  //  return slices.get_entry_by_index(index);
  //}

private:
  RefsKey   *refs_key;     // Handle on RefsKey w/ this entry
  RefsKeys  *refs_keys;    // Handle on RefsKeys
  CfgNodeId target_cn;     // Where the inspector goes
  ValNumber val;           // Subscript value number
  //Sparse_Set slices;       // Set of slices w/ this vn

  const char *aux_cnt_name;        // Subsubscript in inspec; "i$"
  const char *loc_cnt_name;        // Local # elements; "x$loc_cnt"
  const char *new_subsc_glob_name; // New (global) subscript; "n1$glob"
  const char *new_subsc_loc_name;  // New (local) subscript; "n1$loc"
  const char *sched_name;          // Name of schedule; "x$sched"
  const char *tab_name;            // Name of trans table; "x$tab"
};


/**********************************************************************
 * An Iterator.  Usage:
 *
 *    SubscValEntry_ht *ht;
 *    SubscValEntry    *elmt;
 *    ...
 *    for (SubscValEntry_ht_Iter iter(ht); elmt = iter();) { ... }
 *
 */
class SubscValEntry_ht_Iter : public AST_ht_Iter {
public:
  SubscValEntry_ht_Iter(SubscValEntry_ht* my_ht)
    : AST_ht_Iter ((AST_ht *) my_ht) {}

  SubscValEntry *operator() () {
    return (SubscValEntry *) AST_ht_Iter::operator()();
  }
};


/**********************************************************************
 * 1/29/93 RvH: wait until templates are supported ...
 */
class SubscValEntry_ht : public AST_ht {
public:
  // Constructor
  SubscValEntry_ht(RefsKeys *di) : AST_ht (genSubscValEntry, (Generic) di)
  {};
  
  ~SubscValEntry_ht() {                           // Destructor
    SubscValEntry *elmt;
    SubscValEntry_ht_Iter iter(this);

    while (elmt = iter()) delete elmt;
  }
  
  void     add_entry(ValNumber my_key_node, SubscValEntry *my_data) {
    AST_ht::add_entry((AST_INDEX) my_key_node, (Generic) my_data);
  }
  
  Boolean  query_entry(ValNumber query_node) {
    return (Boolean) AST_ht::query_entry((AST_INDEX) query_node);
  }
  
  SubscValEntry   *get_entry_by_vn(ValNumber query_node) {
    return (SubscValEntry *)
      AST_ht::get_entry_by_AST((AST_INDEX) query_node);
  }
  
  SubscValEntry *gen_entry_by_vn(ValNumber query_node) {
    return (SubscValEntry *)
      AST_ht::gen_entry_by_AST((AST_INDEX) query_node);
  }
  
  //void forall(SubscValEntry_ftype forall_func)
  //{
  //  AST_ht::forall(SubscValEntry_apply, (Action_ftype) forall_func);
  //}
};


/**********************************************************************
 *  All keys and their references
 */
class RefsKeys
{
public:
  RefsKeys(S_desc_ht *my_refs, CfgInstance my_cfg,   // Constructor
	   IrrSymTab *my_st);
  ~RefsKeys();                                       // Destructor

  // Access functions
  CfgInstance      getCfg    ()     const { return cfg; }
  RefsKeyId        *getVal2id()     const { return val2id; }
  VectorSet        getAll    ()     const { return *all; }
  SubscValEntry_ht *getSubsc_vals()       { return &subsc_vals; }
  IrrSymTab        *getSt    ()     const { return st; }
  RefsKey_ptr      getRefsKey(int id) const;

  RefsKeyId   ValNum2Key(ValNumber val) const;  // ValNumber->RefsKeyId
  RefsKeyId   SsaNode2Key(SsaNodeId sn) const;  // SsaNodeId->RefsKeyId
  void        dump() const;                     // Dump refs keys
  void        catKeysStr(const VectorSet &set,  // Print keys in set
			 ostrstream &refs_buf,
			 int level = -1) const;
  CfgNodeId   get_lca(CfgNodeId a, CfgNodeId b) const; // Find lca

  // Operations on "Sets of portions" (See LCPC '92 paper)
  VectorSet  star     (RefsKeyId id)   const;              
  VectorSet  star     (const VectorSet &set) const;
  VectorSet  affects  (const VectorSet &set) const;
  VectorSet  contains (const VectorSet &set) const;
  VectorSet  touches  (const VectorSet &set) const;
  VectorSet  no_duplicates (const VectorSet &set) const;
  VectorSet  cn2IND  (CfgNodeId cn) const;
  VectorSet  sn2IND  (CfgNodeId cn) const;

private:
  VectorSet        *all;        // Set containing ALL keys
  int              bit_count;   // # of diferent keys
  CfgInstance      cfg;         // Access to cfg info
  RefsKey_ptr      *keys;       // Reference keys
  DomTree          predom;      // Predom tree (for placing inspecs)
  S_desc_ht        *refs;       // References
  IrrSymTab        *st;         // Access to symbol table info
  SubscValEntry_ht subsc_vals;  // Subscript value numbers
  int              val_cnt;     // # of value numbers
  RefsKeyId        *val2id;     // ValNumber->RefsKeyId table
  //int              subsc_val_count; // # of different subsc vals
};


/**********************************************************************
 *  All references with this key
 */
class RefsKey
{
public:
  RefsKey(RefsKeyId my_id, RefKey_ptr *ref_keys,    // Constructor 
	  int my_count, RefsKeys *my_refs_keys);
  ~RefsKey();                                       // Destructor

  fst_index_t   getVar() const { return key.var; }  // Get symtab index
  ValNumber     getVn()  const { return key.val; }  // Get value number
  SubscValEntry *getVe(int dim) const;
  //void          putVe(SubscValEntry *my_ve) { ve = my_ve; }

  void         update_val2id(RefsKeyId *val2id, int val_cnt);
  void         place_Inspecs(CfgNodeId next_target_cn);
  Boolean      needs_irreg_comm() const;
  S_desc       *get_ref() const;
  void         dump() const;                          // Dump contents
  void         catKeyStr(ostrstream &refs_buf,        // Print key
			 int level = -1) const;

private:
  int           count;      // # of refs this entry corresponds to
  RefsKeyId     id;         // Bitvector postion
  ValVar        key;        // Sorting key for data flow bit entries
  int           rank;       // # of dims
  S_desc_ptr    *refs;      // Reference(s) this entry corresponds to
  RefsKeys      *refs_keys; // Handle on RefsKeys
  SubscValEntry **ve;       // Info for value numbers of subscripts
};


/**********************************************************************
 *  One reference and its key
 */
class RefKey
{
public:
  RefKey(S_desc_ptr my_ref);  // Constructor 
  ~RefKey();                  // Destructor

  S_desc_ptr getRef()  { return ref; }
  ValVar     *getKey() { return &key; }

private:
  S_desc_ptr ref;             // Reference this entry corresponds to
  ValVar     key;             // Sorting key for data flow bit entries
};
#endif
