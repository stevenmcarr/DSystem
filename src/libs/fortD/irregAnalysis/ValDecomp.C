/* $Id: ValDecomp.C,v 1.4 1997/03/27 20:32:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/**********************************************************************
 * Information associated with value based decompositions.
 *
 * $Log: ValDecomp.C,v $
 * Revision 1.4  1997/03/27 20:32:56  carr
 * Alpha
 *
 * Revision 1.3  1997/03/11  14:28:36  carr
 * newly checked in as revision 1.3
 *
Revision 1.3  94/03/21  12:48:51  patton
fiexed comment problem

Revision 1.2  94/02/27  20:15:03  reinhard
Added value-based distributions.
Make CC happy.
See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.

 * Revision 1.2  1994/02/27  19:45:12  reinhard
 * Robustified value-based distributions.
 * Tweaks to make CC happy.
 *
 * Revision 1.1  1994/01/18  19:54:14  reinhard
 * Initial revision
 *
 */

/**********************************************************************
 * Revision History:
 * $Log: ValDecomp.C,v $
 * Revision 1.4  1997/03/27 20:32:56  carr
 * Alpha
 *
 * Revision 1.3  1997/03/11  14:28:36  carr
 * newly checked in as revision 1.3
 *
Revision 1.3  94/03/21  12:48:51  patton
fiexed comment problem

Revision 1.2  94/02/27  20:15:03  reinhard
Added value-based distributions.
Make CC happy.
See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.

 * Revision 1.2  1994/02/27  19:45:12  reinhard
 * Robustified value-based distributions.
 * Tweaks to make CC happy.
 *
 * Revision 1.1  1994/01/18  19:54:14  reinhard
 * Initial revision
 *
 */

#include <strstream>
using namespace std;
#include <libs/frontEnd/ast/astlist.h>
#include <include/frontEnd/astnode.h>
#include <include/frontEnd/astrec.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>
#include <libs/fortD/irregAnalysis/ValDecomp.h>
#include <libs/fortD/irregAnalysis/IrrSymTab.h>
#include <libs/frontEnd/ast/AST_ht.h>
#include <libs/fortD/irregAnalysis/analyse.h>
#include <libs/moduleAnalysis/ssa/ssa.h>
#include <libs/support/tables/NamedGeneric.h>

/*------------------- GLOBAL DECLARATIONS -------------------*/

EXTERN(CfgNodeId,    cfg_node_from_near_ast, (CfgInstance cfg,
					      AST_INDEX node));
EXTERN(AST_INDEX,    genLongComment,      (const char *cmt_str));
EXTERN(const char *, find_pattern_insens, (const char *str,
					   const char *pattern));
EXTERN(char *,    prefix_str,      (char prefix, const char *str));
EXTERN(char *,       prefix_type,         (int type, const char *str));
EXTERN(const char *, ref2varName,      (AST_INDEX ref_node));


/*------------------- LOCAL DECLARATIONS --------------------*/

EXTERN(void, ValDecompInfo_add_directive, (DecEntry     *d,
					   struct dc_id *id,
					   AST_INDEX    node));

/*------------------- LOCAL VARIABLES --------------------*/

// 1/20/94 RvH: This should go away when Dejan is done
static ValDecompInfo *global_ValDecompInfo = NULL;

typedef char *char_ptr;         // used for arrays of ptrs

/*********************************************************************
 *                                                                   *
 *           Methods of class ValDecompInfo                          *
 *                                                                   *
 *********************************************************************/

/**********************************************************************
 * Constructor
 */
ValDecompInfo::ValDecompInfo(IrrSymTab    *my_st,
			     FortranDInfo *my_fd)
: cfg    (NULL),
  fd     (my_fd),
  st     (my_st),
  str2vd (new NamedGenericTable())
{
  // Check whether this gets constructed only once
  if (global_ValDecompInfo)
  {
    // 1/20/94 RvH: Remove this when Dejan is done
    //cerr << "WARNING ValDecompInfo::ValDecompInfo(): "
    //  << "Multiple global instances.\n";
  }
  global_ValDecompInfo = this;
}

/**********************************************************************
 * Destructor
 */
ValDecompInfo::~ValDecompInfo()
{
  delete str2vd;
}

/**********************************************************************
 * add_directive()  Add a value-based distribution directive
 */
void
ValDecompInfo::add_directive(DecEntry     *d,
			     struct dc_id *id,
			     AST_INDEX    node)
{
  const char *decomp_name;
  ValDecomp  *vd;

  decomp_name = id->str;   // Name of decomposition

  // 12/3/93 RvH: For now, assume just one value-based distribution
  // per decomposition
  // Hash on name of decomposition
  //if (str2vd.query_entry(decomp_name))
  if (str2vd->QueryEntry(decomp_name))
  {
    // Duplicate distribution
    cerr << "WARNING ValDecompInfo::add_directive: "
      << "Multiple value-based distributions for " << decomp_name <<
	"; this is not yet supported.\n";
  }
  else
  {
    vd = new ValDecomp(this, decomp_name, d, id, node);
    //str2vd.add_entry(decomp_name, (Generic) vd);
    str2vd->AddEntry(decomp_name, (Generic) vd);
  }
}


/**********************************************************************
 * decompName2ValDecomp()
 */
ValDecomp *
ValDecompInfo::decompName2ValDecomp(const char *decomp_name)
{
  ValDecomp *vd;

  //if (!str2vd.query_entry(decomp_name))
  if (!str2vd->QueryEntry(decomp_name))
  {
    // Missing distribution
    cerr << "WARNING ValDecompInfo::decompName2ValDecomp: "
      << "Missing distribution for decomposition \"" << decomp_name
	<< "\".\n";
    vd = NULL;
  }
  else
  {
    //vd = (ValDecomp*) str2vd.get_entry_by_Str(decomp_name);
    vd = (ValDecomp*) str2vd->QueryEntry(decomp_name);
  }

  return vd;
}


/**********************************************************************
 * name2ValDecomp()  Check whether an array is affected by a
 *                   value-based distribution
 */
ValDecomp *
ValDecompInfo::name2ValDecomp(const char* array_name)
{
  FortranDHashTableEntry *sp;
  const char             *decomp_name;
  ValDecomp              *vd;

  //sp          = st->findName(array_name);
  sp          = fd->GetEntry((char*)array_name);
  //decomp_name = sp->decomp->id;  // Retrieve name of decomposition
  decomp_name = sp->d->dec_name;
  vd = decompName2ValDecomp(decomp_name);

  return vd;
}


/**********************************************************************
 * snode2ValDecomp()
 */
ValDecomp *
ValDecompInfo::snode2ValDecomp(SNODE *sp)
{
  const char *decomp_name;
  ValDecomp  *vd;

  decomp_name = sp->decomp->id;  // Retrieve name of decomposition
  vd = decompName2ValDecomp(decomp_name);

  return vd;
}


/**********************************************************************
 * ref2ValDecomp()
 */
ValDecomp *
ValDecompInfo::ref2ValDecomp(AST_INDEX node)
{
  //Str_ht_Iter iter(&str2vd);
  NamedGenericTableIterator iter(str2vd);
  ValDecomp   *vd;
  ValDecomp   *reaching_vd = NULL;

  // Loop over all ValDecomp elements
  //while (vd = (ValDecomp*) iter())
  for (;
       vd = (ValDecomp*) iter.Current();
       ++iter)
  {
    if (vd->reaches_ref(node))
    {
      if (reaching_vd)
      {
	cerr << "WARNING ValDecompInfo::ref2ValDecomp(): "
	  << "multiple distributions reaching node " << node << ".\n";
      }
      else
      {
	reaching_vd = vd;
      }
    }
  }

  return reaching_vd;
}


/**********************************************************************
 * gencode_redists()
 */
void
ValDecompInfo::gencode_redists()
{
  //Str_ht_Iter iter(&str2vd);
  NamedGenericTableIterator iter(str2vd);
  ValDecomp   *vd;

  // Loop over all ValDecomp elements
  //while (vd = (ValDecomp*) iter())
  for (;
       vd = (ValDecomp*) iter.Current();
       ++iter)
  {
    vd->gencode_redist();
    vd->decl_vars();
  }
  
  assert(st->declInt("init_ttable_with_proc"));
  assert(st->declInt("build_translation_table"));
}


/**********************************************************************
 * is_reg_dist()  Check whether <array_node> is reached by reg. dist.
 */
Boolean
ValDecompInfo::is_reg_dist(AST_INDEX array_node)
{
  Boolean    is_reg_dist;
  const char *name = ref2varName(array_node);

  // 12/3/93 RvH: VERY crude hack here: Check whether there are any
  //              irregular distributions
  //is_reg_dist = (Boolean) (str2vd.count() == 0);
  is_reg_dist = (Boolean) (str2vd->NumberOfEntries() == 0);

  return is_reg_dist;
}


/*********************************************************************
 *                                                                   *
 *           Methods of class ValDecomp                              *
 *                                                                   *
 *********************************************************************/


/**********************************************************************
 * Constructor
 */
ValDecomp::ValDecomp(ValDecompInfo  *my_vdi,
		     const char     *my_decomp_name,
		     DecEntry       *my_d,
		     struct dc_id   *my_id,
		     AST_INDEX      my_directive_node)
: cn                 (cfg_node_from_near_ast(vdi->getCfg(),
					     directive_node)),
  d                  (my_d),
  decomp_name        (my_decomp_name),
  directive_node     (my_directive_node),
  id                 (my_id),
  ndim               (0),
  orig_decomp_type   (1),              // = "BLOCK"
  reg_elmnt_cnt      (0),
  reg_elmnt_cnt_node (AST_NIL),
  val_names          (NULL),
  vdi                (my_vdi),
  weighed            (false),
  weight_name        (NULL)
{
  parse_directive();
  gen_names();

  switch (d->idtype->up.type)
  {
  case Expr_constant:
    reg_elmnt_cnt       = d->idtype->up.val;
    reg_elmnt_cnt_node  = pt_gen_int(reg_elmnt_cnt);
    break;

  case Expr_simple_sym:
    reg_elmnt_cnt_node  = pt_gen_ident(d->idtype->up.str);
    break;

  default:
    cerr << "WARNING ValDecomp::ValDecomp(): "
      << "could not determine reg_elmnt_cnt_node fore decomposition \""
	<< decomp_name << "\".\n";
  }
}


/**********************************************************************
 * Destructor
 */
ValDecomp::~ValDecomp()
{
  int i;
  
  for (i = 0;  i < ndim;  i++)
  {
    delete val_names[i];
  }
  
  delete val_names;
}


/**********************************************************************
 * parse_directive()
 */
void
ValDecomp::parse_directive()
{
  int        i, len;
  const char *found_str;
  const char *name;
  int        found = 0;
  const char *str = id->subs.expr[0].str;   // "dim=1,vals=x,weight=inb"
  
  // Determine dimensionality
  if (found_str = find_pattern_insens(str, "dim="))
  {
    found = sscanf((char*) found_str, "%d", &ndim);
    if (!found)
    {
      cerr << "WARNING ValDecomp::parse_directive: "
	<< "Could not parse ndim.\n";
    }
  }

  if (!found)
  {
    ndim = 1;                                // Default: ndim = 1
  }

  // Allocate val_names accordingly
  val_names = new char_ptr[ndim];

  if (!(found_str = find_pattern_insens(str, "vals=")))
  {
    found_str = str;
  }

  // Scan arrays, one for each dimension
  for (i = 0;  i < ndim;  i++)
  {
    name = found_str;
    len  = 0;
    while ((*found_str) && ((*found_str) != ',') && ((*found_str) != ')'))
    {
      found_str++;
      len++;
    }

    if (*name)
    {
      val_names[i] = new char[len+1];
      strncpy(val_names[i], name, len);
      val_names[i][len] = 0;
    }
    else
    {
      cerr << "WARNING ValDecomp::parse_directive: "
	<< "ndim = " << ndim << ", but found only " << i <<
	  " value arrays.\n";
      val_names[i] = NULL;
    }
  }

  // In case "weight=" is omitted, but there is an extra parameter,
  // then take that parameter as a weight array
  if (*found_str == ',')
  {
    weighed = true;
    found_str++;
    while (*found_str == ' ')
    {
      found_str++;
    }
  }
  else
  {
    weighed = (Boolean) ((found_str = find_pattern_insens(str, "weight=")) != 0);
  }

  if (weighed)
  { 
    name    = found_str;
    len     = 0;
    while ((*found_str) && ((*found_str) != ',') && ((*found_str) != ')'))
    {
      found_str++;
      len++;
    }

    if (len)
    {
      weight_name = new char[len+1];
      strncpy(weight_name, name, len);
      weight_name[len] = 0;
    }
    else
    {
      cerr << "WARNING ValDecomp::parse_directive: "
	<< "Could not parse weight_name.\n";
      weight_name = NULL;
    }
  }

  if ((found_str != 0) && (*found_str != 0))
  {
    cerr << "WARNING ValDecomp::parse_directive: "
      << "Did finish parsing \"" << str << "\".\n";
  }
}


/**********************************************************************
 * gen_names()
 */
void
ValDecomp::gen_names()
{
  IrrSymTab *st = vdi->getSt();

  // Generate name for map from local indices to processors
  loc2proc_name = st->gen_fresh_name(decomp_name, "$loc2proc");

  // Generate name for number of locally owned elmnts
  cnt_name = st->gen_fresh_name(decomp_name, "$cnt");

  // Generate name for map from local to global indices
  loc2glob_name = st->gen_fresh_name(decomp_name, "$loc2glob");

  // Generate name for translation table
  tab_name = st->gen_fresh_name(decomp_name, "$tab");

  // Generate name for communication schedule
  sched_name = st->gen_fresh_name(decomp_name, "$sched");
}


/**********************************************************************
 * gencode_redist()  Generate code for redistributing decomposition
 */
void
ValDecomp::gencode_redist()
{
  AST_INDEX  node, stmt_node;
  ostrstream cmt_buf;
  char       *cmt_str;

   // Generate comment "--<< Redistribute decomposition atomd >>--"
  cmt_buf << "--<< Redistribute decomposition \"" << decomp_name
    << "\" >>--" << ends;
  cmt_str   = cmt_buf.str();
  stmt_node = genLongComment(cmt_str);
  delete cmt_str;

  node = gencode_map();
  list_insert_last(stmt_node, node);

  node = gencode_moves();
  list_insert_last(stmt_node, node);

  node = genLongComment("--<< END Redistribute >>--");
  list_insert_last(stmt_node, node);

  list_insert_after(directive_node, stmt_node);
}


/**********************************************************************
 * gencode_map()  Generate code for creating map
 *
 * Example:
 *   atomd$cnt = natoms/my$p
 *   call fWeighCoorBisecMap(atomd$loc2proc, inb, atomd$cnt, 1, x)
 *   atomd$tab = init_ttable_with_proc (1, atomd$loc2proc, atomd$cnt)
 *   call ialloc(atomd$loc2glob, atomd$cnt)
 *   call remap_reg (atomd$tab, 1, atomd$sched, atomd$loc2glob, atomd$cnt)
 *   call free_table (atomd$tab)
 */
AST_INDEX
ValDecomp::gencode_map()
{
  AST_INDEX  stmt_node, node, arg_node;
  int        i;
  int        type;
  char       *function_name;

  // "atomd$cnt = natoms/my$p"
  arg_node = tree_copy(reg_elmnt_cnt_node);         // "natoms"
  node     = pt_gen_ident((char*)"n$p");            // "n$p"
  node     = pt_gen_div(arg_node, node);
  arg_node = pt_gen_ident((char*)cnt_name);         // "atomd$cnt"
  node     = gen_ASSIGNMENT(AST_NIL, arg_node, node);
  stmt_node = list_create(node);
  
  // "call fWeighCoorBisecMap(atomd$loc2proc, inb, atomd$cnt, 1, x)"
  arg_node = pt_gen_ident((char*)loc2proc_name);    // "atomd$loc2proc"
  arg_node = gen_SUBSCRIPT(arg_node, list_create(pt_gen_int(1)));
  node     = list_create(arg_node);
  if (weighed)
  {
    arg_node = pt_gen_ident((char*)weight_name);    // "inb"
    arg_node = gen_SUBSCRIPT(arg_node, list_create(pt_gen_int(1)));
    node     = list_insert_last(node, arg_node);
  }
  arg_node = pt_gen_ident((char*)cnt_name);         // "atomd$cnt"
  node     = list_insert_last(node, arg_node);
  arg_node = pt_gen_int(ndim);                      // "1" (# of dims)
  node     = list_insert_last(node, arg_node);
  for (i = 0;  i < ndim;  i++)
  {
    arg_node = pt_gen_ident((char*)val_names[i]);   // "x"
    arg_node = gen_SUBSCRIPT(arg_node, list_create(pt_gen_int(1)));
    node     = list_insert_last(node, arg_node);
  }

  type      = vdi->getSt()->get_type((char*)val_names[0]);
  function_name = prefix_type(type,
                              weighed ? "WeighCoorBisecMap"
                              : "CoorBisecMap");
  node      = pt_gen_call(function_name, node);
  stmt_node = list_insert_last(stmt_node, node);


  // "atomd$tab = init_ttable_with_proc (1, atomd$loc2proc, atomd$cnt)"
  arg_node = pt_gen_int(1);          // (Some Parti param ...)
  node     = list_create(arg_node);
  arg_node = pt_gen_ident((char*)loc2proc_name);     // "atomd$loc2proc"
  arg_node = gen_SUBSCRIPT(arg_node, list_create(pt_gen_int(1)));
  node     = list_insert_last(node, arg_node);
  arg_node = pt_gen_ident((char*)cnt_name);          // "atomd$cnt"
  node     = list_insert_last(node, arg_node);
  node     = pt_gen_invoke("init_ttable_with_proc", node);
  arg_node = pt_gen_ident((char*)tab_name);          // "atomd$tab"
  node     = gen_ASSIGNMENT(AST_NIL, arg_node, node);
  stmt_node = list_insert_last(stmt_node, node);
 

  // "call ialloc(atomd$loc2glob, atomd$cnt)"
  arg_node  = pt_gen_ident((char*)loc2glob_name);    // "atomd$loc2glob"
  node      = list_create(arg_node);

  // 12/10/93 RvH, 4am: apparently this is too small ...
  //arg_node  = pt_gen_ident((char*)cnt_name);         // "atomd$cnt"
  arg_node  = tree_copy(reg_elmnt_cnt_node);         // "natoms"
  node      = list_insert_last(node, arg_node);
  type      = TYPE_INTEGER;
  function_name = prefix_type(type, "alloc");
  node      = pt_gen_call(function_name, node);
  stmt_node = list_insert_last(stmt_node, node);


  // "call remap_reg (atomd$tab, 1, atomd$sched, atomd$loc2glob, atomd$cnt)"
  arg_node = pt_gen_ident((char*)tab_name);          // "atomd$tab"
  node     = list_create(arg_node);
  arg_node = pt_gen_int(orig_decomp_type);           // "1" (= BLOCK)
  node     = list_insert_last(node, arg_node);
  arg_node = pt_gen_ident((char*)sched_name);        // "atomd$sched"
  node     = list_insert_last(node, arg_node);
  arg_node = pt_gen_ident((char*)loc2glob_name);     // "atomd$loc2glob"
  arg_node = gen_SUBSCRIPT(arg_node, list_create(pt_gen_int(1)));
  node     = list_insert_last(node, arg_node);
  arg_node = pt_gen_ident((char*)cnt_name);          // "atomd$cnt"
  node     = list_insert_last(node, arg_node);
  node     = pt_gen_call("remap_reg", node);
  stmt_node = list_insert_last(stmt_node, node);
 

  // "call free_table (atomd$tab)"
  arg_node = pt_gen_ident((char*)tab_name);          // "atomd$tab"
  node     = list_create(arg_node);
  node     = pt_gen_call("free_table", node);
  stmt_node = list_insert_last(stmt_node, node);

  return stmt_node;
}


/**********************************************************************
 * gencode_move() Generate code for moving data
 *
 * Example:
 *   call fresize(x, atomd$cnt)
 *   call fgather(x, x, atomd$sched)
 *
 * If it's a multidimensional array, generate something like
 * "call ngather(partners(1,1), partners(1,1), atomD$sched, maxp*4)"
 */
AST_INDEX
ValDecomp::gencode_moves()
{
  AST_INDEX  node, arg_node, stmt_node, size_node, subsc_node;
  ostrstream resize_buf, gather_buf;
  char       *cmt_str;
  ArrayBound *ab;
  char       *name;
  int        st_ndim, type, i, elmt_size;
  NameEntry  *n;
  const char *function_name;
  fst_index_t index;
  Boolean    first_resize = true;
  Boolean    first_gather = true;
  AST_INDEX  resize_node = list_create(AST_NIL);
  AST_INDEX  gather_node = list_create(AST_NIL);
  IrrSymTab  *st       = vdi->getSt();
 
  resize_buf << "--<< Resize ";
  gather_buf << "--<< Shuffle ";

  for(n = d->name_info->first_entry(); n != 0;
      n = d->name_info->next_entry())
  {
    name = n->array_name;

    // Generate comment "--<< Resize "inb", "x", "f", "partners" >>--"
    if (first_resize)
    {
      first_resize = false;
    }
    else
    {
      resize_buf << ", ";
    }
    resize_buf << "\"" << name << "\"";

    type = st->get_type(name);
    if ((type > 0) && (type < MallType_cnt))
    {
      // Check for multidimensional arrays;
      // Assume first dimension is distributed, other dims replicated
      index     = fst_Index(st->getSymtab(), (char *)name);
      st_ndim   = fst_GetFieldByIndex(st->getSymtab(), index,
				      SYMTAB_NUM_DIMS);
      size_node = pt_gen_int(1);
      if (st_ndim > 1)
      {
	ab = (ArrayBound*) fst_GetFieldByIndex(st->getSymtab(), index,
					       SYMTAB_DIM_BOUNDS);
	// Loop over dimensions, except leftmost one
	for (i = 1;  i < st_ndim;  i++)
	{
	  // Assume lower bound 1 => extent = upper bound
	  switch (ab[i].ub.type) {
	  case symbolic_expn_ast_index:
	    node = ab[i].ub.value.ast;
	    break;

	  case constant:
	    node = pt_gen_int(ab[i].ub.value.const_val);
	    break;

	  default:
	    cerr << "WARNING ValDecomp::gencode_moves:"
	      << " cannot handle type = " << ab[i].ub.type << ".\n";
	    node = pt_gen_int(1);
	    break;
	  }
	  size_node = pt_gen_mul(size_node, node);
	}
      }

      // "call fresize(x, atomd$cnt)"
      function_name = prefix_type(type, "resize");
      arg_node      = pt_gen_ident((char*)name);        // "x"
      node          = list_create(arg_node);
      arg_node      = pt_gen_mul(size_node,             // "atomd$cnt"
				 pt_gen_ident((char*)cnt_name));
      arg_node      = pt_simplify_expr(arg_node);
      node          = list_insert_last(node, arg_node);
      node          = pt_gen_call((char*)function_name, node); // "fresize"
      resize_node     = list_insert_last(resize_node, node);

      // Determine whether this is defined yet
      if (needs_remap(index))
      {
	// Generate comment "--<< Shuffle "inb", "x", "f", "partners" >>--"
	if (first_gather)
	{
	  first_gather = false;
	}
	else
	{
	  gather_buf << ", ";
	}
	gather_buf << "\"" << name << "\"";

	// "call fgather(x, x, atomd$sched)"
        function_name = (st_ndim == 1) ? prefix_type(type, "gather")
          : prefix_str('n', "gather");
	subsc_node     = list_create(AST_NIL);
	for (i = 0;  i < st_ndim;  i++)
	{
	  subsc_node = list_insert_last(subsc_node, pt_gen_int(1));
	}
	arg_node = pt_gen_ident((char*)name);        // "x"
	arg_node = gen_SUBSCRIPT(arg_node, tree_copy(subsc_node));
	node     = list_create(arg_node);
	arg_node = pt_gen_ident((char*)name);        // "x"
	arg_node = gen_SUBSCRIPT(arg_node, subsc_node);
	node     = list_insert_last(node, arg_node);
	arg_node = pt_gen_ident((char*)sched_name);  // "atomd$sched"
	node     = list_insert_last(node, arg_node);
        if (st_ndim > 1)
        {          
          elmt_size = st->get_elmt_size(name);
	  arg_node  = pt_gen_mul(tree_copy(size_node),   // "maxp*4"
				 pt_gen_int(elmt_size));
	  arg_node  = pt_simplify_expr(arg_node);
          node      = list_insert_last(node, arg_node);
        }
	node     = pt_gen_call((char*)function_name, node); // "fgather"
	gather_node = list_insert_last(gather_node, node);
      }
    }
    else
    {
      cerr << "WARNING ValDecomp::gencode_moves(): type = " << type <<
	" for variable \"" << name << "\".\n";
    }
  }

  resize_buf << ends;
  cmt_str   = resize_buf.str();
  stmt_node = genLongComment(cmt_str);
  delete cmt_str;
  stmt_node = list_insert_last(stmt_node, resize_node);

  gather_buf << ends;
  cmt_str   = gather_buf.str();
  node      = genLongComment(cmt_str);
  delete cmt_str;
  stmt_node = list_insert_last(stmt_node, node);
  stmt_node = list_insert_last(stmt_node, gather_node);

  return stmt_node;
}


/**********************************************************************
 * needs_remap()  Check whether a variable needs to be dragged along
 *                or not
 */
Boolean
ValDecomp::needs_remap(fst_index_t index)
{
  SsaNodeId   sn;
  CfgNodeId   def_cn;
  Boolean     needs_remap = false;
  CfgInstance cfg         = vdi->getCfg();
  DomTree     predom      = cfg_get_predom(cfg);

  // 12/3/93 RvH: Better recompute cn ...
  cn = cfg_node_from_near_ast(cfg, directive_node);

  // 12/3/93 RvH: Something in ssa-land seems to be broken ... guard:
  //for (sn = ssa_first_var_def(cfg, index);
  //     (sn != SSA_NIL) && !needs_remap;
  //     sn = ssa_next_var_def(cfg, sn))
  for (sn = ssa_get_first_node(cfg);
       (sn != SSA_NIL) && !needs_remap;
       sn = ssa_get_next_node(cfg, sn))
  {
    if (ssa_node_name(cfg, sn) == index)
    {
      if (ssa_node_type(cfg, sn) == SSA_DEF)
      {
	def_cn      = ssa_get_cfg_parent(cfg, sn);
	
	// Definition not dominated by distribute directive ?
	needs_remap = (Boolean) !dom_is_dom(predom, cn, def_cn);
      }
    }
  }

  return needs_remap;
}


/**********************************************************************
 * reaches_ref()
 */
Boolean
ValDecomp::reaches_ref(AST_INDEX node)
{
  Boolean     reaches_ref;
  CfgInstance cfg    = vdi->getCfg();
  CfgNodeId   ref_cn = cfg_node_from_near_ast(cfg, node);
  DomTree     predom = cfg_get_predom(cfg);

  // 12/3/93 RvH: Better recompute cn ...
  cn          = cfg_node_from_near_ast(cfg, directive_node);

  // Reference dominated by distribute directive ?
  reaches_ref = dom_is_dom(predom, cn, ref_cn);

  return reaches_ref;
}


/**********************************************************************
 * decl_vars()
 */
void
ValDecomp::decl_vars()
{
  IrrSymTab  *st = vdi->getSt();
  Expr       *up = &(d->idtype[0].up);
  AST_INDEX  name_node, size_node;

  // 12/3/93 RvH: For now, get size info as follows:
  switch (up->type) {
  case Expr_simple_sym:
    size_node = pt_gen_ident((char*) up->str);
    break;

  case Expr_constant:
    size_node = pt_gen_int(up->val);
    break;

  default:
    cerr << "WARNING ValDecomp::decl_vars():"
      << " cannot handle up->type = " << up->type << ".\n";
    size_node = AST_NIL;
   break;
  }
  name_node = pt_gen_ident((char*) loc2proc_name);
  assert(st->decl1dArr(name_node, size_node));
    
  assert(st->declInt(cnt_name));
  assert(st->declInt(tab_name));
  assert(st->declInt(sched_name));
  assert(st->decl1dArr(loc2glob_name, UNKNOWN_SIZE));
}


/**********************************************************************
 * ValDecomp_add_directive()
 */
void
ValDecompInfo_add_directive(DecEntry     *d,
			    struct dc_id *id,
			    AST_INDEX    node)
{
  if (global_ValDecompInfo)
  {   
    global_ValDecompInfo->add_directive(d, id, node);
  }
  else
  {
    cerr << "WARNING ValDecompInfo_add_directive():"
      << " global_ValDecompInfo = 0.\n";
  }
}
