/* $Id: Inspec.C,v 1.14 1997/03/11 14:28:28 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*********************************************************************
 * class Inspec member functions
 */

/**********************************************************************
 * Revision History:
 * $Log: Inspec.C,v $
 * Revision 1.14  1997/03/11 14:28:28  carr
 * newly checked in as revision 1.14
 *
Revision 1.14  94/03/21  14:03:07  patton
fixed comment problem

Revision 1.13  94/02/27  20:14:24  reinhard
Added value-based distributions.
Make CC happy.
See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.

 * Revision 1.15  1994/02/27  19:40:55  reinhard
 * Tweaks to make CC happy.
 *
 * Revision 1.14  1994/01/18  19:46:48  reinhard
 * Updated include paths according to Makefile change.
 *
 * Revision 1.13  1993/10/04  15:35:57  reinhard
 * Extended class SubscValEntry.
 *
 * Revision 1.12  1993/09/25  15:31:42  reinhard
 * Split new_subsc_name into new_subsc_loc_name, new_subsc_glob_name.
 *
 */

#include <assert.h>

#include <libs/support/sets/Sparse_Set.h>
#include <libs/support/tables/StringHashTable.h>
#include <libs/fortD/irregAnalysis/Inspec.h>
#include <libs/fortD/irregAnalysis/IrrGlobals.h>
#include <libs/fortD/irregAnalysis/IrrSymTab.h>
#include <libs/fortD/irregAnalysis/S_desc.h>
#include <libs/fortD/irregAnalysis/Slice.h>
//#include <libs/fort_d/irreg/analyse.h>


/*------------------- GLOBAL DECLARATIONS -------------------*/

EXTERN(AST_INDEX, genLongComment,       (const char *cmt_str));
EXTERN(Boolean,   is_list_or_NIL,       (AST_INDEX node));
EXTERN(AST_INDEX, mergeStmts,           (AST_INDEX node1,
					 AST_INDEX node2));
EXTERN(AST_INDEX, move_before_comments, (AST_INDEX node));
EXTERN(AST_INDEX, prepend_blank_line,   (AST_INDEX node));

/*------------------- LOCAL DECLARATIONS --------------------*/

EXTERN(Generic,   genInspec, (AST_INDEX target_node, Generic gen_arg));

/**********************************************************************
 * Constructor
 */
Inspec::Inspec(AST_INDEX my_target_node, IrrGlobals *my_di)
: di             (my_di),
  slices         (new Sparse_Set()),
  stmts_node     (AST_NIL),
  target_node    (my_target_node),
  ve_set         (new Sparse_Set()),
  ve_slices_ht   (new AST_ht())

  //vals           (NULL),
  //new_subsc_loc_name (NULL),
  //new_subsc_glob_name (NULL),
  //sched_name     (NULL),
  //loc_cnt_name   (NULL),
  //si_cnt         (0),
  //vi_cnt         (0),
  //si2vi          (NULL),
  //vii_cnt        (NULL),
  //vi2sis         (NULL),
{
}


/**********************************************************************
 * Destructor
 * No deallocation of AST's yet
 */
Inspec::~Inspec()
{
  delete slices;
  delete ve_set;
  delete ve_slices_ht;
  //delete vals;
  //delete si2vi;
  //delete vii_cnt;

  //for (int vi = 0; vi < vi_cnt; vi++)
  //{
  //  delete new_subsc_loc_name[vi];
  //  delete new_subsc_glob_name[vi];
  //  delete sched_name[vi];
  //  delete loc_cnt_name[vi];
  //  delete vi2sis[vi];
  //}

  //delete new_subsc_loc_name;
  //delete new_subsc_glob_name;
  //delete sched_name;
  //delete loc_cnt_name;
  //delete vi2sis;
}


/**********************************************************************
 * 012993RvH: wait until templates are supported ...
 */
Generic
genInspec(AST_INDEX target_node, Generic gen_arg)
{
  IrrGlobals *di   = (IrrGlobals*) gen_arg;
  Inspec     *insp = new Inspec(target_node, di);

  return (Generic) insp;
}


/**********************************************************************
 * add_Slice()  Add <s_des> to this inspector.
 */
void 
Inspec::add_Slice(Slice *slice)
{
  // Make sure there are no conflicts w/ where this insp should go
  //assert(target_node == slice->getTarget_node());

  *slices += (Element) slice;
}


/**********************************************************************
 * install()  Prepend inspector to <target_node>
 */
void 
Inspec::install()
{
  // Make sure we have a place to put inspector
  assert(target_node != AST_NIL);

  // Sort out the given slices
  sort_slices();

  // Generate the stmts of this inspector
  genStmts_node();

  // Declare expanded variables
  declXpandedVars();

  // Don't want to move comments up
  target_node = move_before_comments(target_node);
  target_node = prepend_blank_line(target_node);

  // Prepend <stmts_node> to <target_node>
  // WARNING: this peels the list of <stmts_node>; ie., <stmts_node>
  // now becomes just the first stmt of the list
  stmts_node  = list_insert_before(target_node, stmts_node);
}


/**********************************************************************
 * sort_slices()  Sort out the given slices.
 */
void 
Inspec::sort_slices()
{
  Slice           *sl;
  SubscValEntry   *ve;
  IrrSymTab       *st;
  Sparse_Set      *ve_slices;       // Set of slices w/ this vn
  Sparse_Set_Iter slice_iter(*slices);

  st = di->getSt();

  // Loop over all slices
  while (sl = (Slice *) slice_iter())
  {
    // Get subscript value info
    ve = sl->getVe();

    // New subscript value number ?
    if (!ve_set->query_entry((Element) ve))
    {
      // Add to our set of subscript value numbers
      *ve_set += (Element) ve;

      // Create set of array names that might have to be expanded
      ve_slices = new Sparse_Set;
      ve_slices_ht->add_entry((AST_INDEX) ve, (Generic) ve_slices);
    }
    else
    {
      ve_slices = (Sparse_Set *)
	ve_slices_ht->get_entry_by_AST((AST_INDEX) ve);
    }

    // Add Slice to set of Slices for this ve
    *ve_slices += (Element) sl;
  }
}


/* OLD VERSION:
void 
Inspec::sort_slices()
{
  ValNumber val;          // Value number of inspected subscript
  const char *subsc_name; // Name of subscript ("n1")
  const char *ref_name;   // Name of data array of reference ("x")
  Boolean   is_new_val;   // Do we have val yet ?
  Boolean   is_new_var;   // Do we have var for this subsc val yet?
  int       si, vi, vii, other_si;
  Slice     *sl;
  IrrSymTab *st;
  fst_index_t var, other_var;

  // Determine how many slices this insp is resonsible for
  si_cnt = slices.count();

  // Allocate space for all slices
  slices         = new Slice_ptr[si_cnt];
  vals           = new ValNumber[si_cnt];
  new_subsc_loc_name = new char_ptr[si_cnt];
  new_subsc_glob_name = new char_ptr[si_cnt];
  sched_name     = new char_ptr[si_cnt];
  loc_cnt_name   = new char_ptr[si_cnt];
  si2vi          = new int[si_cnt];
  vi2sis         = new int_ptr[si_cnt];
  vii_cnt        = new int[si_cnt];

  // Loop over all slices
  for (si = 0; si < si_cnt; si++)
  {
    sl = (Slice *) slice_set.get_entry_by_index(si);

    // Copy the set of slices into a (faster) array
    slices[si] = sl;

    // Check whether we have this subscript value number yet
    val = sl->getVn();
    for (is_new_val = true, vi = -1;
	 is_new_val && (++vi < vi_cnt);
	 is_new_val = (val != vals[vi]));

    // vi now points either to redundant slice or to new slice
    si2vi[si] = vi;

    if (is_new_val)   // We don't have this subscript val num yet ?
    {
      vi_cnt++;

      vi2sis[vi]  = new int[si_cnt];
      vii_cnt[vi] = 0;
      vals[vi]    = val;

      // Access name of subscript ("n1")
      subsc_name = sl->getSubsc_name();

      // Access name of data array ("x")
      ref_name = sl->getRef()->getArray_name();

      st = di->getSt();
      // Generate name for new local subscript ("n1$loc")
      new_subsc_loc_name[vi] = st->gen_fresh_name(subsc_name, "$loc");

      // Generate name for new global subscript ("n1$glob")
      new_subsc_glob_name[vi] = st->gen_fresh_name(subsc_name, "$glob");

      // Generate name of schedule ("x$sched")
      sched_name[vi] = st->gen_fresh_name(ref_name, "$sched");

      // Generate name for # of nonlocal references ("x$offsize")
      loc_cnt_name[vi] = st->gen_fresh_name(ref_name, "$offsize");
    }

    // Determine whether we already have this var for this subsc val
    var = sl->getRef()->getVar();
    for (is_new_var = true, vii = -1;
	 is_new_var && (++vii < vii_cnt[vi]);)
    {
      other_si   = vi2sis[vi][vii];
      other_var  = slices[other_si]->getRef()->getVar();
      is_new_var = (var != other_var);
    }

    // We don't have this var for this subsc val yet ?
    if (is_new_var)
    {
      vii_cnt[vi]++;
      vi2sis[vi][vii] = si;
    }

    // Supply the slice w/ names of schedule & local # of elements
    sl->putNew_subsc_loc_name(new_subsc_loc_name[vi]);
    sl->putNew_subsc_glob_name(new_subsc_glob_name[vi]);
    sl->putSched_name(sched_name[vi]);
    sl->putLoc_cnt_name(loc_cnt_name[vi]);
  }
}
*/


/**********************************************************************
 * declXpandedVars()  Declare expanded vars
 */
void 
Inspec::declXpandedVars()
{
  //int si, vi;
  SubscValEntry   *ve;
  Sparse_Set_Iter ve_iter(*ve_set);
  Sparse_Set      *ve_slices;

  // Generate trace arrays of all subscript vals
  //for (vi = 0; vi < vi_cnt; vi++)
  //{
  //  si = vi2sis[vi][0];
  //  slices[si]->declXpandedVars();    // Generate trace arrays
  //}

  while (ve = (SubscValEntry *) ve_iter())
  {
    ve_slices = (Sparse_Set *)
      ve_slices_ht->get_entry_by_AST((AST_INDEX) ve);
    ve->declXpandedVars(*ve_slices);    // Generate trace arrays
  }
}


/**********************************************************************
 * ve2sl()  Pick a slice for ve
 */
Slice *
Inspec::ve2sl(SubscValEntry *ve)
{
  Slice      *sl;
  Sparse_Set *ve_slices = (Sparse_Set *)
    ve_slices_ht->get_entry_by_AST((AST_INDEX) ve);

  assert (ve_slices->count() > 0);
  sl = (Slice *) ve_slices->get_entry_by_index(0);

  return sl;
}


/**********************************************************************
 * ve2sl_cnt()  # of slices for ve
 */
int
Inspec::ve2sl_cnt(SubscValEntry *ve)
{
  Sparse_Set *ve_slices = (Sparse_Set *)
    ve_slices_ht->get_entry_by_AST((AST_INDEX) ve);

  return ve_slices->count();
}


/**********************************************************************
 * genStmts_node()  Generate the stmts of this inspector
 */
void 
Inspec::genStmts_node()
{
  AST_INDEX       node;
  Sparse_Set_Iter ve_iter(*ve_set);
  SubscValEntry   *ve;
  Slice           *sl;
  Sparse_Set      *ve_slices;
  const char      *array_name;
  StringHashTable array_names;
  //int       vi, vii, si;

  // Add a Slice for each different subscript value number
  //for (vi = 0; vi < vi_cnt; vi++)
  while (ve = (SubscValEntry *) ve_iter())
  {
    //si   = vi2sis[vi][0];
    //node = slices[si]->genStmts_node();
    sl   = ve2sl(ve);
    node = sl->genStmts_node();
    stmts_node = mergeStmts(stmts_node, node);
  }

  // Add LOCALIZE's
  while (ve = (SubscValEntry *) ve_iter())
  {
    sl   = ve2sl(ve);
    node = sl->gen_localize();
    stmts_node = list_insert_last(stmts_node, node);
  }

  // Add EXTEND's
  while (ve = (SubscValEntry *) ve_iter())
  {
    ve_slices = (Sparse_Set *)
      ve_slices_ht->get_entry_by_AST((AST_INDEX) ve);
    //for (vii = 0; vii < vii_cnt[vi]; vii++)
    for (Sparse_Set_Iter slice_iter(*ve_slices);
	 sl = (Slice *) slice_iter();)
    {
      //si   = vi2sis[vi][vii];
      //node = slices[si]->gen_extend();

      // Want to extend one slice for each different array name
      array_name = sl->getRef()->getArray_name();
      if (!array_names.query_entry((char *) array_name))
      {
	array_names.add_entry((char *) array_name);
	node = sl->gen_extend();
	stmts_node = list_insert_last(stmts_node, node);
      }
    }
  }

  // Generate a Comment identifying inspector
  node       = gen_inspec_cmt();
  stmts_node = list_insert_first(stmts_node, node);
  node       = genLongComment("--<< END Inspector >>--");
  stmts_node = list_insert_last(stmts_node, node);
}


/**********************************************************************
 * gen_inspec_cmt()  Generate a comment
 *                   "--<< Inspector for x(n1), ... , y(n2) >>--".
 */
AST_INDEX
Inspec::gen_inspec_cmt()
{
  AST_INDEX  cmt_list;
  ostrstream cmt_buf;
  char       *cmt_str;

  cmt_buf << "--<< Inspector for ";
  catSlicesBuf(cmt_buf);
  cmt_buf << " >>--" << ends;
  cmt_str = cmt_buf.str();
  cmt_list = genLongComment(cmt_str);
  delete cmt_str;
 
  return cmt_list;
}


/**********************************************************************
 * catSlicesBuf()  Append a string of the slices of this Inspec
 *               to a given buffer
 */
void
Inspec::catSlicesBuf(ostrstream &slices_buf)  // Append to this buffer
{
  //int si, vi;
  //Sparse_Set      *ve_slices;       // Set of slices w/ this vn
  int             ve_slices_cnt;
  SubscValEntry   *ve;
  Slice           *sl;
  Sparse_Set_Iter ve_iter(*ve_set);
  Boolean         need_comma = false;

  // Iterate over subscript vals
  //for (vi = 0; vi < vi_cnt; vi++)  
  while (ve = (SubscValEntry *) ve_iter())
  {
    if (need_comma) {
      slices_buf << ", ";
    }
    else
    {
      need_comma = true;
    }

    sl            = ve2sl(ve);
    ve_slices_cnt = ve2sl_cnt(ve);

    // Indicate if more then one subscript; eg, "[7*]"
    //if (vii_cnt[vi] > 1)
    if (ve_slices_cnt > 1)
    {
      slices_buf << "[" << ve_slices_cnt << "*]";
    }

    // Print subscript of first slice
    //si = vi2sis[vi][0];
    sl->catSubscript(slices_buf);
  }
}
  

/**********************************************************************
 * Inspec_apply()  Apply method to class member.
 */
/*
void
Inspec_apply(AST_htEntry *entry, va_list ap)
{
  Forall_ftype forall_func;  // The function to apply
  Inspec_ftype action;       // The function to apply (typecasted)
  Inspec       *insp;        // The argument of the function
    
  forall_func = va_arg(ap, Forall_ftype);
  action      = (Inspec_ftype) forall_func;
  insp        = (Inspec *)(entry->getData());
  insp->apply(action);
  va_end(ap);
}
*/
