/* $Id: CommDFUniv.C,v 1.9 1997/03/27 20:32:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*********************************************************************
 */

/**********************************************************************
 * Revision History:
 * $Log: CommDFUniv.C,v $
 * Revision 1.9  1997/03/27 20:32:56  carr
 * Alpha
 *
 * Revision 1.8  1997/03/11  14:28:26  carr
 * newly checked in as revision 1.8
 *
Revision 1.8  94/03/21  14:19:12  patton
fixed comment problem

Revision 1.7  94/02/27  20:14:21  reinhard
Added value-based distributions.
Make CC happy.
See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.

 * Revision 1.13  1994/02/27  19:39:30  reinhard
 * Tweaks to make CC happy.
 *
 * Revision 1.12  1994/01/18  19:45:32  reinhard
 * Updated include paths according to Makefile change.
 * Handle value-based distributions.
 *
 */

#include <assert.h>
#include <iostream.h>
#include <stdlib.h>

#include <libs/support/sets/Sparse_Set.h>
#include <libs/fortD/irregAnalysis/CommDFUniv.h>
#include <libs/fortD/irregAnalysis/IrrGlobals.h>
#include <libs/fortD/irregAnalysis/IrrSymTab.h>
#include <libs/fortD/irregAnalysis/Slice.h>
#include <libs/fortD/irregAnalysis/S_desc.h>
#include <libs/fortD/irregAnalysis/analyse.h>


/*------------------- GLOBAL DECLARATIONS -------------------*/

EXTERN(int,     int_comp,       (int *first, int *second));
EXTERN(Boolean, vn_no_duplicates, (CfgInstance cfg, ValNumber vn));
EXTERN(void *,  qcompress,      (void* data, int nel, int size,
				 Compare_type comp, int &count));
EXTERN(int *,   qcount,         (void* data, int nel, int size,
				 Compare_type comp, int &count));
EXTERN(const char *,    ref2varName,     (AST_INDEX ref_node));

/*------------------- LOCAL DECLARATIONS --------------------*/

EXTERN(Generic, genSubscValEntry, (AST_INDEX target_node,
				   Generic gen_arg));
EXTERN(int,  ValVar_comp,     (ValVar *first, ValVar *second));
EXTERN(int,  RefKey_comp,     (RefKey *first, RefKey *second));
EXTERN(int,  RefKey_ptr_comp, (RefKey **first, RefKey **second));


/*********************************************************************
 *                                                                   *
 *           Methods of class SubscValEntry                          *
 *                                                                   *
 *********************************************************************/


/*********************************************************************
 * SubscValEntry  Constructor
 */
SubscValEntry::SubscValEntry(ValNumber my_val, RefsKeys *my_refs_keys)
: refs_key            (NULL),
  refs_keys           (my_refs_keys),
  target_cn           (CFG_NIL),
  val                 (my_val),
  aux_cnt_name        (NULL),
  loc_cnt_name        (NULL),
  new_subsc_glob_name (NULL),
  new_subsc_loc_name  (NULL),
  sched_name          (NULL),
  tab_name            (NULL)
{
}


/*********************************************************************
 * SubscValEntry  Destructor
 */
SubscValEntry::~SubscValEntry()
{
}


/**********************************************************************
 * 012993RvH: wait until templates are supported ...
 */
Generic
genSubscValEntry(AST_INDEX target_node, Generic gen_arg)
{
  RefsKeys      *di = (RefsKeys*) gen_arg;
  SubscValEntry *ve = new SubscValEntry((ValNumber) target_node, di);

  return (Generic) ve;
}


/*********************************************************************
 * gen_names()
 */
void
SubscValEntry::gen_names(const char *subsc_name,
			 const char *array_name)
{
  IrrSymTab *st;

  // Generate new names only if we don't have any yet
  if (!new_subsc_loc_name)
  {
    st = refs_keys->getSt();

    // Generate name for new local subscript ("n1$loc")
    new_subsc_loc_name = st->gen_fresh_name(subsc_name, "$loc");

    // Generate name for new global subscript ("n1$glob")
    new_subsc_glob_name = st->gen_fresh_name(subsc_name, "$glob");

    // Generate name for counter ("n1$cnt")
    aux_cnt_name = st->gen_fresh_name(subsc_name, "$cnt");

    // Generate name for # of nonlocal references ("x$offsize")
    loc_cnt_name = st->gen_fresh_name(array_name, "$offsize");

    // Generate name of schedule ("x$sched")
    sched_name = st->gen_fresh_name(array_name, "$sched");

    // Generate name of translation table ("x$tab")
    tab_name = st->gen_fresh_name(array_name, "$tab");
  }
}


/**********************************************************************
 * declXpandedVars()  Declare expanded vars.
 */
void
SubscValEntry::declXpandedVars(Sparse_Set &slices)
{
  Slice           *sl;
  int             size;
  Sparse_Set_Iter slice_iter(slices);
  IrrSymTab       *st            = refs_keys->getSt();
  int             new_subsc_size = 0;
  Boolean         aux_flag       = false;

  while (sl = (Slice *) slice_iter())
  {
    // The size of the new subscript is
    // - UNKNOWN_SIZE  if it is unknown in any of our slices
    // - the max of the size at each slice otherwise.
    if (new_subsc_size != UNKNOWN_SIZE)
    {
      size = sl->getNew_subsc_size();
      new_subsc_size = (size == UNKNOWN_SIZE) ? size
	: max(new_subsc_size, size);
    }

    // We have to declare aux_cnt_name ("n1$cnt") if it is needed
    // as an auxiliary induction variable (loop not normal) or
    // temporary for the # of loop iterations (loop bounds not constant)
    // in any slice.
    // 2/18/94: The merits of ***!!!!!STRONG!!!!!*** typechecking:
    aux_flag = (Boolean) (aux_flag || ((int) sl->getAux_flag()
				       || (int) sl->getCnt_flag()));
  }
  
  assert(st->decl1dArr(new_subsc_loc_name, new_subsc_size));
  assert(st->decl1dArr(new_subsc_glob_name, new_subsc_size));
  assert(st->declInt(sched_name));
  assert(st->declInt(tab_name));
  assert(st->declInt(loc_cnt_name));

  if (aux_flag)
  {
    assert(st->declInt(aux_cnt_name));
  }
}


/*********************************************************************
 * add_target_cn()  Add <next_target_cn> as a target for inspector
 *                  placement
 *
 * 1/20/94 RvH: Raja should do this
 */
void
SubscValEntry::add_target_cn(CfgNodeId next_target_cn)
{
  CfgInstance cfg     = refs_keys->getCfg();
  TarjTree    tarjans = cfg_get_intervals(cfg);

  // Take the LCA of the current target and the handed target
  target_cn = (next_target_cn == cfg_start_node(cfg))
    ? target_cn
      :((target_cn == CFG_NIL)
	? next_target_cn
	: refs_keys->get_lca(target_cn, next_target_cn));

  // Also, want to move to outermost loop level
  while (tarj_level(tarjans, target_cn) > 1)
  {
    target_cn = tarj_outer(tarjans, target_cn);
  }
}


/*********************************************************************
 *                                                                   *
 *           Methods of class RefsKeys                               *
 *                                                                   *
 *********************************************************************/


/*********************************************************************
 * RefsKeys  Constructor
 */
RefsKeys::RefsKeys(S_desc_ht *my_refs, CfgInstance my_cfg,
		   IrrSymTab *my_st)
: all     (new VectorSet),
  cfg     (my_cfg),
  predom  (cfg_get_predom(cfg)),
  refs    (my_refs),
  st      (my_st),
  subsc_vals (this),
  val_cnt (cfgval_table_max(cfg)),
  val2id  (new RefsKeyId[val_cnt])
{
  int        *ref_counts;  // # of occurences of each key in ref_keys
  S_desc_ptr ref;          // A particular reference
  int        refs_count;   // # of references
  RefKey_ptr *ref_keys;    // Keys for all references
  int        count;        // # of ref's with a particular key
  int        offset;       // First ref with a particular key
  RefsKeyId  i = 0;        // Position in bitvector
  ValNumber  vn;           // Value number
  //ValNumber  *raw_subsc_vals;  // Uncompressed subscript value numbers
  //ValNumber  *comp_subsc_vals; // Compressed subscript value numbers
  //RefsKey    *key;             // Pointer to one RefsKey
  //SubscValEntry *ve;    // Info for a subscript value number

  // Make sure we have a valid symbol table
  assert(st != NULL);

  // 1. Compute REF_KEYS, the set of { <ref>, <key> } pairs
  //    derived from the <refs> S_desc's
  refs_count = refs->count();
  ref_keys   = new RefKey_ptr[refs_count];
  for (S_desc_ht_Iter iter(refs); ref = iter(); i++) {
    ref_keys[i] = new RefKey(ref);
  }

  // 2. Sort keys, determine # of different keys (<bit_count>)
  //    and the # of occurences of each key (<ref_counts>)
  //    Primary sorting key:   <key>.<var>
  //    Secondary sorting key: <key>.<val>
  ref_counts = qcount(ref_keys, refs_count, sizeof(RefKey_ptr),
		      (Compare_type) &RefKey_ptr_comp, bit_count);

  // 3. Initialize <val2id>, the ValNumber->RefsKeyId mapping
  for (vn = 0;  vn < val_cnt;  val2id[vn++] = RefsKeyNil);

  // 4. Compute DF_BITS, the set of { <id>, <refs>, <key> } triples
  //    describing the <id>-th bit in our bitvectors.
  keys           = new RefsKey_ptr[bit_count];
  //raw_subsc_vals = new ValNumber[bit_count];
  offset         = 0;
  for (i = 0;  i < bit_count;  i++)    // Loop over keys
  {
    *all  <<= i;                       // Add key to universe
    count   = ref_counts[i];
    keys[i] = new RefsKey(i, &ref_keys[offset], count, this);
    offset += count;
    keys[i]->update_val2id(val2id, val_cnt);
    //raw_subsc_vals[i] = keys[i]->getVn(); // Collect subscript values
  }

  // 5. Compute the subscript value universe
  //qsort(raw_subsc_vals, bit_count, sizeof(ValNumber),
	//(Compare_type) &int_comp);
  //comp_subsc_vals = (ValNumber *)
  //  qcompress(raw_subsc_vals, bit_count, sizeof(ValNumber),
	//      (Compare_type) &int_comp, subsc_val_count);
  //for (i = 0;  i < subsc_val_count;  i++)    // Loop over subsc vals
  //{
  //  vn        = comp_subsc_vals[i];
  //  ve = new SubscValEntry(vn, this);
  //  subsc_vals.add_entry((AST_INDEX) vn, (Generic) ve);
  //}

  // 6. Supply each RefsKey w/ pointer to subscript value info
  //    and supply subscript value info w/ pointer to one RefsKey
  //for (i = 0;  i < bit_count;  i++)    // Loop over keys
  //{
  //  key = keys[i];
  //  vn  = key->getVn();
  //  ve = (SubscValEntry *) subsc_vals.
  //    get_entry_by_AST((AST_INDEX) vn);
  //  key->putVe(ve);
  //  ve->putKey(key);
  //}

  //delete raw_subsc_vals;
  delete ref_keys;
  delete ref_counts;
}


/*********************************************************************
 * RefsKeys  Destructor
 */
RefsKeys::~RefsKeys()
{
  for (RefsKeyId bit = 0;  bit < bit_count;  bit++) {
    delete keys[bit];
  }

  delete keys;
  delete val2id;

  //for (i = 0;  i < subsc_val_count;  i++)    // Loop over subsc vals
  //{
  //  delete (SubscValEntry *) subsc_vals.get_entry_by_index(i);
  //}
}


/*********************************************************************
 * 
 */
RefsKey_ptr
RefsKeys::getRefsKey(int id) const
{
  RefsKey_ptr refs_key = NULL;

  if (id < 0) {
    cerr << "WARNING: RefsKeys::getRefsKey(): id = " << id << " < 0.\n";
  } else if (id >= bit_count) {
    cerr << "WARNING: RefsKeys::getRefsKey(): id = " << id <<
      " > bit_count = " << bit_count << ".\n";
  } else {
    refs_key = keys[id];
  }

  return refs_key;
}


/*********************************************************************
 * RefsKeys  Map ValNumber -> RefsKeyId
 */
RefsKeyId
RefsKeys::ValNum2Key(ValNumber vn) const
{
  RefsKeyId id;

  if (vn >= val_cnt)
  {
    cerr << "WARNING: RefsKeys::ValNum2Key(): vn = " << vn
      << " >= val_cnt = " << val_cnt << ".\n";
    id = RefsKeyNil;
  }
  else
  {
    id = (vn == VAL_NIL) ? RefsKeyNil : val2id[vn];
  }

  return id;
}


/*********************************************************************
 * RefsKeys  Map SsaNodeId -> RefsKeyId
 */
RefsKeyId
RefsKeys::SsaNode2Key(SsaNodeId sn) const
{
  ValNumber vn;
  AST_INDEX ref_node;
  S_desc    *ref;
  RefsKeyId bit;
  RefsKeyId id = RefsKeyNil;
  
  if (sn == SSA_NIL)
  {
    cerr << "WARNING: RefsKeys::SsaNode2Key(): sn == SSA_NIL.\n";
  }
  else
  {
    // Make sure that this is an array (and not just a scalar which
    // has an array value number, like "a" after "a = x(i)")
    if (FS_IS_ARRAY(st->getSymtab(), ssa_node_name(cfg, sn)))
    {
      vn = cfgval_build(cfg, sn);     // Lookup value number
      id = ValNum2Key(vn);            // Map ValNumber -> RefsKeyId
      
      // 7/16/93 RvH: Since the array value numbering sometimes assigns
      // the same value number to different arrays (w/ the same subscript
      // value number), we rather look the key up directly from the ref.
      if (id != RefsKeyNil)
      {
	ref_node = ssa_node_to_ast(cfg, sn);
	if (ref_node != AST_NIL)
	{
	  ref = refs->get_entry_by_AST(ref_node);
	  if (ref)
	  {
	    bit = ref->getBit();
	    if (bit != RefsKeyNil)
	    {
	      if (id != bit)
	      {
		// If we get here, then we got a "wrong" <vn>
		cerr << "WARNING: SsaNode2Key(): id = " << id <<
		  " != bit = " << bit << " for \"" << ref->getStr() <<
		    "\", ref_node = " << ref_node << ".\n";
		id = bit;
	      }
	    }
	    else
	    {
	      cerr << "WARNING: SsaNode2Key(): id = " << id <<
		", ref_node = " << ref_node <<
		  " but no bit in ref; sn = " << sn << ".\n";
	    }
	  }
	  else
	  {
	    cerr << "WARNING: SsaNode2Key(): id = " << id <<
	      ", ref_node = " << ref_node << " but no ref; sn = " <<
		sn << ".\n";
	  }
	}
	else
	{
	  //cerr << "WARNING: SsaNode2Key(): id = " << id <<
	  //  " but ref_node = AST_NIL; sn = " << sn <<[ ".\n";
	  
	  // 9/1/93 RvH: If we get here, then we asked for the key of an SSA
	  // node that does not correspond to an actual stmt.
	  // For now, return the empty key in this case.
	  id = RefsKeyNil;
	}
      }
    }
  }
  
  return id;
}


/*********************************************************************
 * RefsKeys  Dump function
 */
void
RefsKeys::dump() const
{
  cout << "*** DUMPING RefsKeys ***\n";
  cout << "bit_count: " << bit_count << endl;
  for (RefsKeyId i = 0;  i < bit_count;  i++)        // Loop over keys
  {
    cout << "keys[" << i << "]:\n";
    keys[i]->dump();
  }
}


/*********************************************************************
 * RefsKeys  Print function
 */
void
RefsKeys::catKeysStr(const VectorSet &set, ostrstream &refs_buf,
		     int level) const
{
  Boolean not_first = false;

  for( VectorSetI i(&set); i.test(); ++i )
  {
    if (not_first) {
      refs_buf << ", ";
    } else {
      not_first = true;
    }
    keys[i.elem]->catKeyStr(refs_buf, level);
  }
}


/*********************************************************************
 * get_lca()
 */
CfgNodeId
RefsKeys:: get_lca(CfgNodeId a, CfgNodeId b) const 
{
  return dom_lca(predom, a, b);
}


/*********************************************************************
 * RefsKeys  star()  Computes the set of all portions which have the
 *                   same <var> as the portions <q>
 *                   (See LCPC '92 paper)
 */
VectorSet
RefsKeys::star(RefsKeyId q) const
{
  VectorSet   result;
  fst_index_t var = keys[q]->getVar();
  RefsKeyId   p;

  for(p = q; (p >= 0) && (keys[p]->getVar() == var); p--) {
    result <<= p;
  }

  for(p = q+1; (p < bit_count) && (keys[p]->getVar() == var); p++) {
    result <<= p;
  }

  return result;
}


/*********************************************************************
 * RefsKeys  star()  Computes the set of all portions which have the
 *                   same <var> as the portions in <set>
 *                   (See LCPC '92 paper)
 */
VectorSet
RefsKeys::star(const VectorSet &set) const
{
  VectorSet result;

  for(VectorSetI i(&set); i.test(); ++i)
  {
    result |= star(i.elem);
  }

  return result;
}


/*********************************************************************
 * RefsKeys  affects()  Computes the set of all portions which might
 *                      affect any portion in <set>
 *                      (See LCPC '92 paper)
 */
VectorSet
RefsKeys::affects(const VectorSet &set) const
{
  VectorSet result;

  result = star(set);             // Conservative approximation for now

  return result;
}


/*********************************************************************
 * RefsKeys  contains()  Computes the set of all portions which contain
 *                       any portion in <set>
 *                       (See LCPC '92 paper)
 */
VectorSet
RefsKeys::contains(const VectorSet &set) const
{
  VectorSet result;

  result = set;                   // Conservative approximation for now

  return result;
}


/*********************************************************************
 * RefsKeys  touches()  Computes the set of all portions which touch
 *                      any portion in <set>
 *                      (See LCPC '92 paper)
 */
VectorSet
RefsKeys::touches(const VectorSet &set) const
{
  VectorSet result = affects(set) - contains(set);

  return result;
}


/*********************************************************************
 * no_duplicates()  Computes the set of all portions that have no
 *                  duplicates.
 * Each portion p is in 1 of 2 categories.
 * 
 * Case 1) p might have duplicates within itself;
 *         eg, x(n(1:100)), when nothing is known about n
 *
 * Case 2) p can be proven to not have any duplicates within itself;
 *         eg, x(1:100).
 *
 * In Case 2), p can  safely be reused locally, w/o SCATTER/GATHER.
 *
 * Example:
 *
 * Given 4 processors and the following code:
 * do i = 1, 100 
 *    x(n(i)) = ... 
 * enddo
 *
 * Case 1:                             Case 2 (eg, n a permutation):
 *
 * do i = 1, 25                        do i = 1, 25   
 *    x(n(i)) = ...                       x(n(i)) = ...
 * enddo                               enddo
 * SCATTER x(n(1:100))
 * GATHER  x(n(1:100))
 * do i = 1, 25                        do i = 1, 25   
 *    ... = x(n(i))                    ... = x(n(i))
 * enddo                               enddo
 */
VectorSet
RefsKeys::no_duplicates(const VectorSet &set) const
{
  VectorSet   result;
  RefsKeyId   p;
  ValNumber   vn;
  RefsKey_ptr refs_key;
  VectorSetI  iter(&set);

  for (; iter.test(); ++iter)
  {
    p        = iter.elem;
    refs_key = getRefsKey(p);
    vn       = refs_key->getVn();
    if (vn_no_duplicates(cfg, vn)) {
      result <<= p;
    }
  }
  
  return result;
} 


/*********************************************************************
 * cn2IND()  Get set of affected nodes
 *            get_IND( Partners = ... ) = { x(Partners(i, j), ... }
 */
VectorSet
RefsKeys::cn2IND(CfgNodeId cn) const
{
  RefsKeyId   i;
  S_desc      *ref;
  int         j, rank;
  Slice       *sl;
  CfgNodeId   subs_def_cn;
  VectorSet   result;
  RefsKey_ptr refs_key;

  for (i = 0;  i < bit_count;  i++)        // Loop over keys
  {
    refs_key = getRefsKey(i);
    ref      = refs_key->get_ref();
    rank     = ref->getRank();
    for (j = 0;  j < rank;  j++)
    {
      sl          = ref->getSlice(j);
      subs_def_cn = sl->get_unique_def_cn();
      if (subs_def_cn == cn)
      {
	result <<= i;
      }
    }
  }
  
  return result;
}

/*********************************************************************
 * sn2IND()  Get set of affected nodes
 *            get_IND( Partners = ... ) = { x(Partners(i, j), ... }
 */
VectorSet
RefsKeys::sn2IND(SsaNodeId sn) const
{
  RefsKeyId   i;
  S_desc      *ref;
  int         j, rank;
  Slice       *sl;
  const char  *subs_def_name;
  VectorSet   result;
  RefsKey_ptr refs_key;
  const char  *arr_name;
  AST_INDEX   name_node = ssa_node_to_ast(cfg, sn);

  if (is_subscript(name_node))
  {
    arr_name = ref2varName(name_node);
    for (i = 0;  i < bit_count;  i++)        // Loop over keys
    {
      refs_key = getRefsKey(i);
      ref      = refs_key->get_ref();
      rank     = ref->getRank();
      for (j = 0;  j < rank;  j++)
      {
	sl            = ref->getSlice(j);
	subs_def_name = sl->get_unique_def_name();
	if ((subs_def_name) && (!strcmp(subs_def_name, arr_name)))
	{
	  result <<= i;
	}
      }
    }
  }
 
  return result;
}


/*********************************************************************
 *                                                                   *
 *           Methods of class RefsKey                                *
 *                                                                   *
 *********************************************************************/


/*********************************************************************
 * RefsKey  Constructor
 */
RefsKey::RefsKey(RefsKeyId my_id, RefKey_ptr *ref_keys,
                 int my_count, RefsKeys *my_refs_keys)
: count       (my_count),
  id          (my_id),
  key         (*(ref_keys[0]->getKey())),
  refs        (new S_desc_ptr[count]),
  refs_keys   (my_refs_keys)
{
  int              i, dim;
  S_desc           *ref;
  Slice            *slice;
  SubscValEntry_ht *subsc_vals;   // Subscript value numbers
  ValNumber        subsc_vn;
  const char       *subsc_name;
  const char       *array_name;
  Boolean          *xpnd_flags = new Boolean[count];

  assert(count > 0);

  // Initialize subscript data
  ref        = ref_keys[0]->getRef();
  rank       = ref->getRank();
  subsc_vals = refs_keys->getSubsc_vals();
  ve         = new SubscValEntry_ptr[rank];
  array_name = ref->getArray_name();
  for (dim = 0;  dim < rank;  dim++)
  {
    slice    = ref->getSlice(dim);
    subsc_vn = slice->getVn();
    if (subsc_vn < 0)
    {
      cerr << "WARNING: RefsKey::RefsKey(): subsc_vn = " << subsc_vn
        << " is invalid.\n";
    }
    ve[dim] = subsc_vals->gen_entry_by_vn(subsc_vn);
    ve[dim]->putKey(this);
    xpnd_flags[dim] = slice->getXpnd_flag();
    if (xpnd_flags[dim])
    {
      subsc_name = slice->getSubsc_name();
      ve[dim]->gen_names(subsc_name, array_name);
    }
  }

  // Loop over identical keys
  for (i = 0;  i < count;  i++)
  {
    // Make sure that really all handed references have same key
    assert(!RefKey_comp(ref_keys[0], ref_keys[i]));

    ref     = ref_keys[i]->getRef(); // Retrieve S_desc
    refs[i] = ref;                   // Add this S_desc to our refs
    ref->putBit(id);                 // Enter <id> into S_desc

    // Check consistency of <rank>
    if (ref->getRank() != rank)
    {
      cerr << "WARNING: RefsKey::RefsKey(): reference \""
	<< refs[0]->getStr() << "\" has rank " << rank
	  << ", but \"" << ref->getStr() << "\" has rank "
	    << ref->getRank() << ".\n";
    }

    for (dim = 0;  dim < rank;  dim++)
    {
      slice = ref->getSlice(dim);
      slice->putVe(ve[dim]);

      // Check consistency of <xpnd_flag>
      if (slice->getXpnd_flag() != xpnd_flags[dim])
      {
	cerr << "WARNING: RefsKey::RefsKey(): For subscript = " << dim
	  << ", reference \"" << refs[0]->getStr()
	    << "\" has xpnd_flag = " << xpnd_flags[dim]
	      << ", but \"" << ref->getStr() << "\" has xpnd_flag = "
		<< slice->getXpnd_flag() << ".\n";
      }
      //ve[dim]->add_slice(slice);
    }
  }
}


/*********************************************************************
 * RefsKey  Destructor
 */
RefsKey::~RefsKey()
{
  delete refs;
}


/*********************************************************************
 * RefsKey  getVe()
 */
SubscValEntry *
RefsKey::getVe(int dim) const
{
  assert ((dim >= 0) && (dim < rank));

  return ve[dim];
}


/*********************************************************************
 * RefsKey  update_val2id()
 */
void
RefsKey::update_val2id(RefsKeyId *val2id, int val_cnt)
{
  ValNumber vn;          // Value number
  
  for (int i = 0;  i < count;  i++)
  {
    vn = refs[i]->getVn();
    if ((vn < 0) || (vn >= val_cnt)) {
      cerr << "WARNING: RefsKeys(): vn = " << vn << ", "
	<< val_cnt << " = val_count\n";
    } else {
      val2id[vn] = id;                // Fill in <val2id>.
    }
  }
}


/*********************************************************************
 * RefsKey  place_Inspecs()
 */
void
RefsKey::place_Inspecs(CfgNodeId next_target_cn)
{
  int i;

  if (next_target_cn == CFG_NIL)
  {
    cerr << "WARNING: RefsKey::place_Inspecs(): "
      << "next_target_cn = CFG_NIL; id = " << id << ", ref = \""
        << refs[0]->getStr() << "\".\n";
  }

  for (i = 0; i < rank; i++)
  {
    ve[i]->add_target_cn(next_target_cn);
  }

  //for (int i = 0;  i < count;  i++)
  //{
  //  refs[i]->putTarget_node(target_node);
  //}
}


/*********************************************************************
 * RefsKey  needs_irreg_comm()
 */
Boolean
RefsKey::needs_irreg_comm() const
{
  return refs[0]->getIs_irreg();
}


/*********************************************************************
 * RefsKey  get_ref()
 */
S_desc *
RefsKey::get_ref() const
{
  return refs[0];
}


/*********************************************************************
 * RefsKey  Dump function
 */
void
RefsKey::dump() const
{
  //char ref_str[MAX_REF_LENGTH];
  const char *ref_str;

  cout << "\tid: " << id
    << "\tval: " << key.val
      << "\tvar: " << key.var
	<< "\tname: " << refs[0]->getName() << endl;
  cout << "\trefs[0:" << count-1 << "]->Subsc/CFG/SSA/AST:";
  for (int i = 0;  i < count;  i++) {
    ref_str = refs[i]->getStr();
    cout << " \"" << ref_str << "\"/" << refs[i]->getCfgId() << "/"
      << refs[i]->getSsaId() << "/" << refs[i]->getArray_node();
  }
  cout << endl;
}


/*********************************************************************
 * RefsKey  Print function
 */
void
RefsKey::catKeyStr(ostrstream &refs_buf, int level) const
{
  //AST_INDEX array_node = refs[0]->getArray_node();
  //catRefBuf(array_node, refs_buf);

  refs[0]->catPortion(refs_buf, level);   // Print only first key
}


/*********************************************************************
 *                                                                   *
 *           Methods of class RefKey                                 *
 *                                                                   *
 *********************************************************************/


/*********************************************************************
 * RefKey  Constructor
 */
RefKey::RefKey(S_desc_ptr my_ref)
: ref (my_ref)
{
  key.var = ref->getVar();
  key.val = ref->getSubscVal();
}


/*********************************************************************
 * RefKey  Destructor
 */
RefKey::~RefKey()
{
}


/*********************************************************************
 * ValVar_comp()  ValVar comparison function, as used by qsort().
 *                Primary sorting key:   <key>.<var>
 *                Secondary sorting key: <key>.<val>
 */
int
ValVar_comp(ValVar *first, ValVar *second)
{
  return (first->var == second->var) ? int(first->val - second->val)
    : int(first->var - second->var);
}


/*********************************************************************
 * RefKey_comp()  RefKey comparison function, as used by qsort().
 */
int
RefKey_comp(RefKey *first, RefKey *second)
{
  return ValVar_comp(first->getKey(), second->getKey());
}


/*********************************************************************
 * RefKey_ptr_comp()  RefKey comparison function, as used by qsort().
 */
int
RefKey_ptr_comp(RefKey **first, RefKey **second)
{
  return ValVar_comp((*first)->getKey(), (*second)->getKey());
}
