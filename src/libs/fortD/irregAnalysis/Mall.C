/* $Id: Mall.C,v 1.3 1997/03/11 14:28:31 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/*********************************************************************
 * class Mall member functions
 *
 * For a general description, see Mall.txt.
 */

/**********************************************************************
 * Revision History:
 * $Log: Mall.C,v $
 * Revision 1.3  1997/03/11 14:28:31  carr
 * newly checked in as revision 1.3
 *
Revision 1.3  94/03/21  13:08:04  patton
fixed comment problem

Revision 1.2  94/02/27  20:14:36  reinhard
Added value-based distributions.
Make CC happy.
See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.

 * Revision 1.5  1994/02/27  19:43:50  reinhard
 * Tweaks to make CC happy.
 *
 * Revision 1.4  1994/01/18  19:49:23  reinhard
 * Updated include paths according to Makefile change.
 * Handle value-based distributions.
 *
 * Revision 1.3  1993/10/04  15:37:48  reinhard
 * Const'd params.
 *
 * Revision 1.2  1993/09/25  23:02:59  reinhard
 * Initialized own_st.
 *
 * Revision 1.1  1993/09/25  15:39:27  reinhard
 * Initial revision
 *
 */

#include <strstream.h>
#include <assert.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>
#include <libs/fortD/irregAnalysis/Mall.h>
#include <libs/frontEnd/ast/AstIter.h>
#include <libs/frontEnd/prettyPrinter/ft2text.h>
#include <libs/fortD/localInfo/fd_symtable.h>
#include <libs/support/tables/NamedGeneric.h>
#include <libs/fortD/irregAnalysis/IrrSymTab.h>

/*------------------ GLOBAL DECLARATIONS --------------------*/

EXTERN(AST_INDEX,    find_last_stmt,  (AST_INDEX node));
EXTERN(char *,       prefix_str,      (char prefix, const char *str));
EXTERN(const char *, save_program,    (FortTree ft, FortTextTree ftt,
				       const char *extension));

/*------------------- LOCAL CONSTANT DEFINITIONS ------------*/

const char *MallType_names[]       = { "unknown", "integer", "real", 
				       "double", "complex", "logical" };
const char *MallType_prefix_strs[] = { "?", "i", "f", "d", "c", "l" };
const char MallType_prefixes[]     = { '?', 'i', 'f', 'd', 'c', 'l' };
const int  MallType_asts[]         = { 0, GEN_INTEGER, GEN_REAL,
				       GEN_DOUBLE_PRECISION, GEN_COMPLEX,
				       GEN_LOGICAL };
const char *Ext_names[]            = { "$type", "$wrk", "$wrk_size",
				       "$ind", "$size", "$newsize" };


/*********************************************************************
 *                                                                   *
 *           Methods of class Mall_entry                             *
 *                                                                   *
 *********************************************************************/

/**********************************************************************
 * Constructor
 */
Mall_entry::Mall_entry(const char *my_name,
		       const char *my_index_id,
		       const char *my_size_id,
		       int my_type)
: index_id (my_index_id),
  name     (my_name),
  size_id  (my_size_id),
  type     (my_type)
{
}


/**********************************************************************
 * Destructor
 */
Mall_entry::~Mall_entry()
{
}


/*********************************************************************
 *                                                                   *
 *           Methods of class Mall                                   *
 *                                                                   *
 *********************************************************************/

/**********************************************************************
 * Constructor
 */
Mall::Mall()
: dyn_arrs           (new NamedGenericTable()),
  free_node          (AST_NIL),
  ft                 (0),
  ftt                (0),
  new_size_temp_name (NULL),
  own_st             (false),
  root_node          (AST_NIL),
  st                 (NULL)
{
  int type;

  for (type = 0; type < MallType_cnt; type++)
  {
    init_node[type]       = AST_NIL;
    type_ident_name[type] = NULL;
    wrk_ident_node[type]  = AST_NIL;
    wrk_ident_name[type]  = NULL;
    wrk_size_node[type]   = AST_NIL;
    wrk_size_name[type]   = NULL;
  }
}


/**********************************************************************
 * Destructor
 */
Mall::~Mall()
{
  delete dyn_arrs;
  if (own_st)
  {
    delete st;
  }
}


/*********************************************************************
 * convert()  
 */
void
Mall::convert(FortTree my_ft, FortTextTree my_ftt, IrrSymTab *my_st)
{
  ft     = my_ft;
  ftt    = my_ftt;
  own_st = (Boolean) (my_st == NULL);

  // When called in standalone mode, then we have to generate our
  // own IrrSymTab.
  if (own_st)
  {
    st = new IrrSymTab();
    st->init(ft);
  }
  else
  {
    st = my_st;
  }
  
  root_node = ft_Root(ft);
  free_node = find_last_stmt(root_node);

  find_init_calls();
  find_alloc_calls();
  find_resize_calls();
  delete_decls();
  convert_refs();
}


/**********************************************************************
 * find_init_calls()  Collect stmts of the form
 *                    CALL <Type>PUTSIZE(<size>)
 */
void
Mall::find_init_calls()
{
  AST_INDEX stmt_node, args_list, size_node, int_node, ident_node;
  int       type;
  char      *name;
  AstIter   iter(root_node);   // Iterator for walking AST

  while ((stmt_node = iter()) != AST_NIL)
  {
    if (is_mall_call(stmt_node, "putsize", 1, type, args_list))
    {
      if (wrk_size_node[type] != AST_NIL)
      {
	// Duplicate initialization
	cerr << "WARNING Mall::find_init_calls(): "
	  << "duplicate '<Type>putsize' call; node = "
	    << stmt_node << ".\n";
      }
      else
      {
	// No temp var for resizing generated yet ?
	if (!new_size_temp_name)
	{
	  new_size_temp_name =
	    st->gen_fresh_name("", Ext_names[(int) NewSizeTemp]);
	  assert(st->declInt(new_size_temp_name));
	}

        // Any new allocations of <type> go before this node
        init_node[type] = list_next(stmt_node);

	// Generate "i$wrk", add to list of initialization args
	wrk_ident_name[type] =
	  st->gen_fresh_name(MallType_prefix_strs[type],
			     Ext_names[(int) WorkArray]);
	ident_node = pt_gen_ident((char *)wrk_ident_name[type]);
	list_insert_last(args_list, ident_node);

	// Declare "integer i$wrk(i$wrk_size)"
	size_node = list_first(args_list);
	assert(st->decl1dArr(ident_node, size_node, type));

	// Generate "integer i$type", "parameter(i$type = 1)"
	type_ident_name[type] =
	  st->gen_fresh_name(MallType_prefix_strs[type],
			     Ext_names[(int) TypeIndex]);
	int_node = pt_gen_int(type);
	st->declIntParam(type_ident_name[type], int_node);

        // Declare "integer ialloc, iresize"
	name = prefix_str(MallType_prefixes[type], "alloc");
	assert(st->declInt(name));
	name = prefix_str(MallType_prefixes[type], "resize");
	assert(st->declInt(name));
      }
    }
  }
}


/*********************************************************************
 * delete_decls()  Delete extra declarations
 */
void
Mall::delete_decls()
{
  AST_INDEX   node, new_node;
  const char  *name;
  Mall_entry  *dyn_arr;
  fst_index_t index;
  //Str_ht_Iter iter(&dyn_arrs);
  NamedGenericTableIterator iter(dyn_arrs);
  SymDescriptor symtab = st->getSymtab();

  //while (dyn_arr = (Mall_entry*) iter())
  for (;
       dyn_arr = (Mall_entry*) iter.Current();
       iter++)
  {
    name   = dyn_arr->getName();
    index  = fst_Index(symtab, (char *)name);
    node   = fst_GetFieldByIndex(symtab, index, SYMTAB_DIM_LIST);
    if (node)
    {
      // Change all dynamic arrays to 1-d array of size 1
      new_node = list_create(gen_DIM(AST_NIL, pt_gen_int(1)));
      pt_tree_replace(node, new_node);
    }
  }
}


/*********************************************************************
 * find_alloc_calls()  Collect all dynamic array allocation stmts
 */
void
Mall::find_alloc_calls()
{
  AST_INDEX node, stmt_node, args_list, name_node, size_node, next_node;
  int       type;
  char      *name;
  AstIter   iter(root_node);   // Iterator for walking AST

  while ((stmt_node = iter()) != AST_NIL)
  {
    if (is_mall_call(stmt_node, "alloc", 2, type, args_list))
    {
      name_node = list_first(args_list);
      name = gen_get_text(name_node);
      //if (dyn_arrs.query_entry(name))
      if (dyn_arrs->QueryEntry(name))
      {
	// Duplicate allocation
	cerr << "WARNING Mall::find_alloc_calls(): "
	  << "duplicate '<Type>alloc' call for '"
	    << name << "'; node = " << stmt_node << ".\n";
      }
      else
      {
        size_node = tree_copy(list_next(name_node));
        (void) gen_alloc(type, name, size_node, node);
        next_node = list_last(node);
        pt_tree_replace(stmt_node, node);
        
        // Restart iterator at new node
        iter.putCur_node(next_node);
      }
    }
  }
}


/*********************************************************************
 * gen_alloc()
 */
Mall_entry *
Mall::gen_alloc(int type, const char *name,
		AST_INDEX size_node, AST_INDEX &stmt_node)
{
  AST_INDEX  node, arg_node;
  const char *index_id, *size_id;
  Mall_entry *dyn_arr;

  // Construct new dyn_arrs entry
  index_id = st->gen_fresh_name(name, Ext_names[(int) Index]);
  size_id  = st->gen_fresh_name(name, Ext_names[(int) Size]);
  dyn_arr  = new Mall_entry(name, index_id, size_id, type);
  //dyn_arrs.add_entry((char *)name, (Generic) dyn_arr);
  dyn_arrs->AddEntry(name, (Generic) dyn_arr);

  // Declare "x$ind", "x$size"
  st->declInt(index_id);
  st->declInt(size_id);

  // Add "call free(f$type, x$ind + 1, x$size)"
  arg_node  = pt_gen_ident((char *)type_ident_name[type]);
  node      = list_create(arg_node);
  arg_node  = pt_gen_ident((char *)index_id);
  arg_node  = pt_gen_add(arg_node, pt_gen_int(1));
  node      = list_insert_last(node, arg_node);
  arg_node  = pt_gen_ident((char *)size_id);
  node      = list_insert_last(node, arg_node);
  node      = pt_gen_invoke("free", node);
  node      = gen_CALL(AST_NIL, node);
  free_node = list_insert_after(free_node, node);

  // Generate "x$size = size"
  arg_node  = pt_gen_ident((char *)size_id);
  node      = gen_ASSIGNMENT(AST_NIL, arg_node, size_node);
  stmt_node = list_create(node);

  // Replace "call falloc(x, size)"
  // by "x$ind = ialloc(f$type, size) - 1"
  arg_node = pt_gen_ident((char *)type_ident_name[type]);
  node     = list_create(arg_node);
  arg_node = pt_gen_ident((char *)size_id);
  node     = list_insert_last(node, arg_node);
  node     = pt_gen_invoke("ialloc", node);
  arg_node = pt_gen_int(1);
  node     = pt_gen_sub(node, arg_node);
  arg_node = pt_gen_ident((char *)index_id);
  node     = gen_ASSIGNMENT(AST_NIL, arg_node, node);
  stmt_node = list_insert_last(stmt_node, node);

  return dyn_arr;
}


/*********************************************************************
 * find_resize_calls()  Collect all dynamic array resizing stmts
 */
void
Mall::find_resize_calls()
{
  AST_INDEX  node, stmt_node, arg_node, args_list, name_node, size_node;
  int        type;
  const char *name;
  Mall_entry *dyn_arr;
  AstIter    iter(root_node);   // Iterator for walking AST

  while ((stmt_node = iter()) != AST_NIL)
  {
    if (is_mall_call(stmt_node, "resize", 2, type, args_list))
    {
      name_node = list_first(args_list);
      name    = gen_get_text(name_node);
      //dyn_arr = (Mall_entry *) dyn_arrs.get_entry_by_Str((char *)name);
      dyn_arr = (Mall_entry *) dyn_arrs->QueryEntry(name);

      if (!dyn_arr)
      {
	// Resize w/o allocation
	//cerr << "WARNING Mall::find_resize_calls(): "
	//  << "'<Type>resize' call for '" << name
	//    << "', but no allocation; node = " << stmt_node << ".\n";

        // Generate allocation code
        size_node = st->get_size_node(name);
        dyn_arr   = gen_alloc(type, name, size_node, node);
        (void) list_insert_before(init_node[type], node);
      }

      // Generate "$newsize = newsize" before call
      size_node = list_next(name_node);
      node = gen_ASSIGNMENT(AST_NIL,
			    pt_gen_ident((char *)new_size_temp_name),
			    tree_copy(size_node));
      list_insert_before(stmt_node, node);

      // Generate "x$size = $newsize" after call
      node = gen_ASSIGNMENT(AST_NIL,
			    pt_gen_ident((char *)dyn_arr->getSize_id()),
			    pt_gen_ident((char *)new_size_temp_name));
      list_insert_after(stmt_node, node);

      // Replace "call fresize(x, newsize)" by
      // "x$ind = iresize(f$type, x$ind + 1, x$size, $newsize) - 1"
      arg_node = pt_gen_ident((char *)type_ident_name[type]);
      node     = list_create(arg_node);

      arg_node = pt_gen_ident((char *)dyn_arr->getIndex_id());
      arg_node = pt_gen_add(arg_node, pt_gen_int(1));
      node     = list_insert_last(node, arg_node);

      arg_node = pt_gen_ident((char *)dyn_arr->getSize_id());
      node     = list_insert_last(node, arg_node);

      arg_node = pt_gen_ident((char *)new_size_temp_name);
      node     = list_insert_last(node, arg_node);

      node     = pt_gen_invoke("iresize", node);
      node     = pt_gen_sub(node, pt_gen_int(1));
      arg_node = pt_gen_ident((char *)dyn_arr->getIndex_id());
      node     = gen_ASSIGNMENT(AST_NIL, arg_node, node);
      pt_tree_replace(stmt_node, node);

      // Restart iterator at new node
      iter.putCur_node(node);
    }
  }
}


/*********************************************************************
 * convert_refs()  Convert all dynamic array references
 */
void
Mall::convert_refs()
{
  AST_INDEX   node, name_node, subsc_list, subsc_node, new_node, size_node;
  int         type, ndim, st_ndim, i;
  const char  *name;
  Mall_entry  *dyn_arr;
  fst_index_t index;
  ArrayBound  *ab;
  Boolean     only_stmts = false;
  AstIter     iter(root_node, only_stmts);   // Iterator for walking AST
  SymDescriptor symtab = st->getSymtab();

  while ((node = iter()) != AST_NIL)
  {
    if (is_subscript(node))
    {
      name_node = gen_SUBSCRIPT_get_name(node);
      name      = gen_get_text(name_node);
      //dyn_arr = (Mall_entry *) dyn_arrs.get_entry_by_Str((char *)name);
      dyn_arr = (Mall_entry *) dyn_arrs->QueryEntry(name);

      if (dyn_arr)
      {
	// General transformation like this: real x(n1, n2, ..., nk)
	//    x(i1, i2, ..., ik) =>
	////     f$wrk(x$ind + i1 + (n2*(i2 - 1 + (n3*(i3 - 1 + ...))))
	//     f$wrk(x$ind + i3 + (n3*(i2 - 1 + (n2*(i1 - 1 + ...))))
	// Assume only first dim is distributed
	subsc_list = gen_SUBSCRIPT_get_rvalue_LIST(node);
	ndim       = list_length(subsc_list);
	index      = fst_Index(symtab, (char *)name);
	st_ndim    = fst_GetFieldByIndex(symtab, index, SYMTAB_NUM_DIMS);
	if ((st_ndim) && (ndim != st_ndim))
	{
	  // partners(i,j) => i$wrk(partners$ind + i + j * maxp)
	  cerr << "WARNING Mall::convert_refs(): \"" << name
	    << "\" is " << st_ndim << "-d array, referenced as "
	      << ndim << "-d array; node = " << node << ".\n";
	}
	else
	{
	  // Replace "x(i)" by "f$wrk(x$ind + i)"
	  type     = dyn_arr->getType();
	  ab       = (ArrayBound*)
	    fst_GetFieldByIndex(symtab, index, SYMTAB_DIM_BOUNDS);

	  // Start w/ zero, should get simplified away
	  new_node   = pt_gen_int(0);

	  // Loop over dimensions, except leftmost one, inside out
	  for (i = 1;  i < ndim; i++)
	  {
	    subsc_node = list_retrieve(subsc_list, i);
	    // Assume lower bound 1 => extent = upper bound
	    switch (ab[i].ub.type) {
	    case symbolic_expn_ast_index:
	      size_node = ab[i].ub.value.ast;
	      break;

	    case constant:
	      size_node = pt_gen_int(ab[i].ub.value.const_val);
	      break;

	    default:
	      cerr << "WARNING Mall::convert_refs:"
		<< " cannot handle type = " << ab[i].ub.type << ".\n";
	      size_node = AST_NIL;
	      break;
	    }
	    node     = pt_gen_sub(tree_copy(subsc_node),   // "i_k - 1"
				  pt_gen_int(1));
	    node     = pt_gen_add(node, new_node);  // "i_k - 1 + ..."
	    new_node = pt_gen_mul(size_node, node); // "n_k*(i_k-1+ ...)"
	  }
	  new_node   = pt_gen_add(tree_copy(list_last(subsc_list)),
				  new_node);
	  node   = pt_gen_ident((char *)dyn_arr->getIndex_id());
	  node   = pt_gen_add(node, new_node);
	  node   = pt_simplify_expr(node);
	  node   = list_create(node);
	  pt_tree_replace(subsc_list, node);

	  node   = pt_gen_ident((char *)wrk_ident_name[type]);
	  pt_tree_replace(name_node, node);
	}
      }
    }
  }
}


/**********************************************************************
 * is_mall_call()  Try to match <node> against
 *                 "CALL <type><pattern>(<args_list>)"
 */
Boolean
Mall::is_mall_call(AST_INDEX  node,
		   const char *pattern,
		   int        arg_cnt,     // Expected # of args
		   int        &type,
		   AST_INDEX  &args_list)
{
  const char *callee_name;
  Boolean    found_match = false;

  if (is_call(node))
  {
    node        = gen_CALL_get_invocation(node);
    callee_name = gen_get_text(gen_INVOCATION_get_name(node));
    if (!strcmp(&callee_name[1], pattern))
    {
      type      = prefix2type(*callee_name);
      args_list = gen_INVOCATION_get_actual_arg_LIST(node);

      // Found "call <type><pattern>()"
      if (type == TYPE_UNKNOWN)
      {
	// Wrong prefix
	cerr << "WARNING Mall::is_mall_call(): "
	  << "found illegal prefix '"
	    << *callee_name << "' in call to '"
	      << callee_name << "'; node = " << node << ".\n";
      }
      else if (list_length(args_list) != arg_cnt)
      {
	// Wrong # of args
	cerr << "WARNING Mall::is_mall_call(): expected "
	  << arg_cnt << " argument(s) in memory init call to '"
	    << callee_name << "', found " << list_length(args_list)
	      << " args; node = " << node << ".\n";
      }
      else 
      {
	found_match = true;
      }
    }
  }

  return found_match;
}


/**********************************************************************
 * prefix2type()  Derive type from the callee prefix
 */
int
Mall::prefix2type(char prefix)
{
  int i;
  int type = TYPE_UNKNOWN;

  for (i = 0;
       (i < MallType_cnt) && (type == TYPE_UNKNOWN);
       i++)
  {
    if (prefix == MallType_prefixes[i])
    {
      type = i;
    }
  }

  return type;
}


