/* $Id: S_desc.C,v 1.16 1997/03/11 14:28:34 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/**********************************************************************
 * class S_desc member functions
 */

/**********************************************************************
 * Revision History:
 * $Log: S_desc.C,v $
 * Revision 1.16  1997/03/11 14:28:34  carr
 * newly checked in as revision 1.16
 *
Revision 1.16  94/03/21  14:00:43  patton
fixed comment problem

Revision 1.15  94/02/27  20:14:44  reinhard
Added value-based distributions.
Make CC happy.
See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.

 * Revision 1.18  1994/02/27  19:44:28  reinhard
 * Tweaks to make CC happy.
 *
 * Revision 1.17  1994/01/18  19:50:22  reinhard
 * Updated include paths according to Makefile change.
 * Handle value-based distributions.
 *
 * Revision 1.16  1993/10/06  18:26:45  reinhard
 *  Made exec's based on Slices instead of S_desc's.
 *
 * Revision 1.15  1993/10/04  15:38:39  reinhard
 * Const'd params.
 */

#include <assert.h>

#include <libs/fortD/irregAnalysis/IrrGlobals.h>
#include <libs/fortD/irregAnalysis/IrrSymTab.h>
#include <libs/fortD/irregAnalysis/S_desc.h>
#include <libs/fortD/irregAnalysis/Slice.h>
#include <libs/fortD/irregAnalysis/ValDecomp.h>

/*------------------- GLOBAL DECLARATIONS -------------------*/

extern const char *valType[];          // values.C

EXTERN(AST_INDEX, assert_is_subscript, (AST_INDEX node))
EXTERN(AST_INDEX, find_outmost_loop,   (AST_INDEX node));
EXTERN(AST_INDEX, mergeStmts,          (AST_INDEX node1,
					AST_INDEX node2));
EXTERN(const char *, ref2varName,      (AST_INDEX ref_node));


/*------------------- LOCAL DECLARATIONS --------------------*/

EXTERN(Generic, genS_desc, (AST_INDEX array_node, Generic gen_arg));


/**********************************************************************
 * Constructor
 */
S_desc::S_desc(AST_INDEX my_array_node, irr_type my_kind,
	       IrrGlobals *my_di)
: array_name    (ref2varName(my_array_node)),
  array_node    (assert_is_subscript(my_array_node)),
  bit           (RefsKeyNil),
  di            (my_di),
  is_irreg      (false),
  kind          (my_kind),
  str           (di->Ast2Str(array_node)),
  subsc_list    (gen_SUBSCRIPT_get_rvalue_LIST(array_node)),
  vd            ((Generic)(((ValDecompInfo*)di->getHidden_ValDecompInfo())
                           ->ref2ValDecomp(array_node))),
  distrib_irreg ((Boolean) (vd != 0)),
  rank          (list_length(subsc_list)),
  slices        (new Slice[rank])
  //limit_node  (AST_NIL),
{
}


/**********************************************************************
 * Destructor
 * No deallocation of AST's yet
 */
S_desc::~S_desc()
{
  delete [] slices;
}
     
						       
/**********************************************************************
 * 012993RvH: wait until templates are supported ...
 */
Generic
genS_desc(AST_INDEX array_node, Generic gen_arg)
{
  S_desc_arg_type *args   = (S_desc_arg_type*) gen_arg;
  S_desc          *s_desc = new S_desc(array_node, args->kind, args->di);

  return (Generic) s_desc;
}


/**********************************************************************
 * putVd()   Supply ValDecomp info
 */
void
S_desc::putVd(Generic my_vd)
{
  vd            = my_vd;
  distrib_irreg = true;
}


/**********************************************************************
 * Install target node for communication stmt
 */
// 4/5/93 RvH: This does not work if there are multiple gathers for
// the same subscript value number (since then a target node gets
// defined multiple times).  Therefore, retire this routine for now.
// New heuristic: put the inspector for ALL communication stmts ahead
// of the first GATHER
//void
//S_desc::putTarget_node(AST_INDEX my_target_node)
//{
//  target_node = my_target_node;
//}


/**********************************************************************
 * Lookup name.
 */
const char *
S_desc::getName() const
{
  const char *name = gen_get_text(gen_SUBSCRIPT_get_name(array_node)); 

  return name;
}
     
						       
/**********************************************************************
 * Lookup symbol table index.
 */
fst_index_t
S_desc::getVar() const
{
  const char  *name = getName();
  fst_index_t var   = name ?
    fst_QueryIndex(di->getSt()->getSymtab(), (char *) name) : -1;

  return var;
}


/**********************************************************************
 * Lookup value number.
 */
ValNumber
S_desc::getVn() const
{
  ValNumber vn = cfgval_get_val(di->getCfg(), array_node);

  return vn;
}


/**********************************************************************
 * Lookup value number of subscript.
 */
ValNumber
S_desc::getSubscVal() const
{
  ValNumber   sub_vn;
  CfgInstance cfg     = di->getCfg();
  Values      val     = cfgval_get_values(cfg);
  ValNumber   vn      = getVn();
  ValType     vn_type = val_get_val_type(val, vn);

  switch (vn_type)
  {
  case VAL_ARRAY:
    sub_vn = val_get_subs(val, vn);
    vn     = sub_vn;
    break;

  case VAL_VARIANT:
    cerr << "WARNING: S_desc::getSubscVal(): Reference \"" << str
      << "\" got array value number " << vn
	<< " of type VAL_VARIANT, unable to retrieve subscript.\n";
    break;

  default:
    cerr << "WARNING: S_desc::getSubscVal(): "
      << "Reference \"" << str << "\" got subscript value number "
	  << vn << ".\n";
  }

  if (vn < VAL_ZERO)
  {
    cerr << "WARNING: S_desc::getSubscVal(): "
      << "Reference \"" << str << "\" got subscript value number "
	<< vn << " of type " << valType[(int) vn_type] << ".\n";
  }

  return vn;
}


/**********************************************************************
 * Lookup SSA node id.
 */
SsaNodeId
S_desc::getSsaId() const
{
  SsaNodeId sn = ssa_node_map(di->getCfg(), array_node);

  return sn;
}


/**********************************************************************
 * Lookup CFG node id.
 */
CfgNodeId   
S_desc::getCfgId() const
{
  CfgNodeId cn = ssa_get_cfg_parent(di->getCfg(), getSsaId());

  return cn;
}


/**********************************************************************
 * getOutmost_loop_node()  Find outermost enclosing loop.
 */
AST_INDEX
S_desc::getOutmost_loop_node() const
{
  AST_INDEX outmost_loop_node = find_outmost_loop(array_node);

  return outmost_loop_node;
}


/**********************************************************************
 * get_subsc_node()  Construct string like "x(n1$arr(1:100))".
 */
AST_INDEX
S_desc::get_subsc_node(int dim) const
{
  AST_INDEX node;

  assert ((dim >= 0) && (dim < rank));
  node = list_retrieve(subsc_list, dim+1);

  return node;
}


/**********************************************************************
 * getSlice()
 */
Slice *
S_desc::getSlice(int dim)
{
  assert ((dim >= 0) && (dim < rank));

  return &slices[dim];
}


/**********************************************************************
 * catPortion()  Construct string like "x(n1$arr(1:100))".
 */
void
S_desc::catPortion(ostrstream &buf, int level) const
{
  if (!is_irreg) {
    buf << "[";
  }

  buf << getName() << "(";
  for (int i = 0;  i < rank;  i++) // Loop over subscripts
  {
    slices[i].catSubscript(buf, level);
    if (i < rank-1)
      buf << ", ";
  }
  buf << ")";

  if (!is_irreg) {
    buf << "]";
  }
}


/**********************************************************************
 * genExec()  Generate the Executor.
 */
void
S_desc::genExec()
{
  //if (limit_node != AST_NIL)           // Is there a surrounding DO ?
  //{
  //  exec = di->getExecs()->gen_entry_by_AST(limit_node);
  //  exec->add_S_desc(this);           // Add myself to executor
  //}
  //else
  //{
  //  exec = NULL;
  //}

  for (int i = 0; i < rank; i++)            // For all subscripts
  {
    slices[i].genExec();
  }
}


/**********************************************************************
 * genSlices()  Generate the slices.
 */
void
S_desc::genSlices()
{
  int       i;
  AST_INDEX subsc_node;
  //int       lim_level, limit_level = 0;

  // Each subscript position gets it's own slice
  for (i = 0, subsc_node = list_first(subsc_list); 
       i < rank;
       i++, subsc_node = list_next(subsc_node))
  {
    slices[i].init(this, i, subsc_node, di);

    // Do we have to expand this subscript ?
    if (slices[i].getXpnd_flag())
    {
      is_irreg = true;

      // Determine the nesting level of the slice for this subscript
      //lim_level = slices[i].getLimit_level();

      // Check whether the last slice pushed the limit level out;
      // ie., whether the last slice goes back to a shallower loop
      // nesting level than the slices encountered so far.
      //if ((limit_level == 0) || (lim_level < limit_level))
      //{
	//limit_level = lim_level;
	//limit_node  = slices[i].getLimit_node();
      //}
    }
  }
}


/**********************************************************************
 * tree_changed()  Update after AST changes.
 *
 * This may be called, for example, when the regular compiler has
 * altered the AST after this slice has been initialized.
 */
void
S_desc::tree_changed()
{
  str = di->Ast2Str(array_node);
  vd  = (Generic)(((ValDecompInfo*)di->getHidden_ValDecompInfo())
		  ->ref2ValDecomp(array_node));
  distrib_irreg = ((Boolean) (vd != 0));

  // For all subscripts
  for (int i = 0; i < rank; i++)
  {
    slices[i].tree_changed();
  }
}


/**********************************************************************
 * genInspec()  Generate the inspec.
 */
void
S_desc::genInspec()
{
  //if (target_node == AST_NIL)
  //{
  //  cerr << "WARNING: S_desc::genInspec(): target node = AST_NIL" <<
  //    " for \"" << str << "\", array_node = " << array_node << ".\n";
  //}
  //else

  for (int i = 0; i < rank; i++)            // For all subscripts
  {
    //slices[i].genInspec(target_node);
    slices[i].genInspec();
  }
}


/**********************************************************************
 * update_subsc()  Update subscripts
 */
/*
void
S_desc::update_subsc(const char *ivar_name)
{
  if (is_irreg)               // Change subscript only when needed
  {
    for (int i = 0; i < rank; i++)            // For all subscripts
    {
      slices[i].update_subsc(ivar_name);
    }
  }
  else
  {
    cerr << "WARNING: S_desc::update_subsc(): " <<
      "Attempt to change subscript of regular reference " << str <<
	", AST_INDEX = " << array_node << ".\n";
  }
}
*/


/**********************************************************************
 * S_desc_apply()  Apply method to class member.
 */
/*
void
S_desc_apply(AST_htEntry *entry, va_list ap)
{
  Forall_ftype forall_func;  // The function to apply
  S_desc_ftype action;       // The function to apply (typecasted)
  S_desc       *ref;         // The argument of the function
    
  forall_func = va_arg(ap, Forall_ftype);
  action      = (S_desc_ftype) forall_func;
  ref         = (S_desc *)(entry->getData());
  ref->apply(action);
  va_end(ap);
}
*/
