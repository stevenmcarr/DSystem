/* $Id: IrrSymTab.C,v 1.9 2001/09/14 18:30:15 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*********************************************************************
 * class IrrSymTab member functions
 */

/**********************************************************************
 * Revision History:
 * $Log: IrrSymTab.C,v $
 * Revision 1.9  2001/09/14 18:30:15  carr
 * Update for RH 7.1
 *
 * Revision 1.8  1997/03/11 14:28:30  carr
 * newly checked in as revision 1.8
 *
Revision 1.8  94/03/21  13:15:16  patton
fixed comment problem

Revision 1.7  94/02/27  20:14:34  reinhard
Added value-based distributions.
Make CC happy.
See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.

 * Revision 1.8  1994/02/27  19:42:36  reinhard
 * Tweaks to make CC happy.
 * Break too long lines into smaller ones.
 * Added is_distributed(), get_rank().
 *
 * Revision 1.7  1994/01/18  19:48:49  reinhard
 * Updated include paths according to Makefile change.
 * Handle value-based distributions.
 */

#include <strstream.h>
#include <stdlib.h>
#include <assert.h>
#include <libs/fortD/irregAnalysis/IrrSymTab.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>
#include <libs/frontEnd/prettyPrinter/ft2text.h>
#include <libs/support/tables/StringHashTable.h>
#include <libs/fortD/irregAnalysis/analyse.h>
#include <math.h>

/*------------------- GLOBAL DECLARATIONS -------------------*/

EXTERN(AST_INDEX,     append_blank_line, (AST_INDEX node));
EXTERN(AST_INDEX,     find_stmt_list,  (AST_INDEX node));
EXTERN(AST_INDEX,     genComment,      (const char *cmt_str));
EXTERN(SymDescriptor, get_symtab,      (FortTree ft, AST_INDEX root));
EXTERN(const char *,  ref2varName,     (AST_INDEX ref_node));
EXTERN(STR_INDEX,     root2name,       (AST_INDEX node));
EXTERN(Boolean,       val_is_irreg2,   (Values    val,
					ValNumber vn));

/*------------------- LOCAL CONSTANTS -----------------------*/

// The declared size of an unknown array dimension
static const int DynamicArraySize = 1;

// Max # of digits for single array dimension
static const int MAX_DIGITS = 20;

// Maximum allowed length for a type statement
static const int MAX_XLINE_LENGTH = 20*60;

// Initial length for a type statement
static const int INIT_XLINE_LENGTH = 15;


/**********************************************************************
 * Constructor
 */
IrrSymTab::IrrSymTab()
:
 decls_exist   (false),
 decls_node    (AST_NIL),
 fd            (NULL),
 ft            (NULL),
 ints_node     (AST_NIL),              // No int scalar declared yet
 params_node   (AST_NIL),              // No parameters yet
 string_ht     (new StringHashTable())
{
  for (int i = 0; i < NIHASH; i++) {
    ihash[i] = NULL;
  }

  for (int i = 0; i < MallType_cnt; i++)
  {
    arrs_node[i] = AST_NIL;            // No arrays declared yet
  }
}


/**********************************************************************
 * Destructor
 */
IrrSymTab::~IrrSymTab()
{
}


/**********************************************************************
 * init()  Initialization function.
 */
void
IrrSymTab::init(FortTree my_ft)
{
  AST_INDEX root_node;
  STR_INDEX name;

  ft         = my_ft;
  root_node  = ft_Root(ft);
  name       = root2name(root_node);
  symtab     = ft_SymGetTable(ft, string_table_get_text(name));
}


/**********************************************************************
 * is_irreg_and_distrib()
 */
Boolean
IrrSymTab::is_irreg_and_distrib(AST_INDEX node, CfgInstance cfg)
{
  AST_INDEX     subsc_node;
  AST_INDEX     subsc_list;
  ValNumber     vn;
  int           dim      = 0;
  Values        val      = cfgval_get_values(cfg);
  Boolean       is_irreg = false;

  // 5/25/93 RvH: The handling of unsubscripted array references
  //              probably needs some more work
  if (is_subscript(node))  // Does it have a subscript ?
  {
    // Loop over all subscripts
    subsc_list = gen_SUBSCRIPT_get_rvalue_LIST(node);
    for (subsc_node = list_first(subsc_list);
	 !is_irreg && (subsc_node != AST_NIL);
	 subsc_node = list_next(subsc_node))
    {
      if (is_distributed(node, dim++))
      {
	// Check whether subscript is irregular
	vn       = cfgval_get_val(cfg, subsc_node);
	is_irreg = val_is_irreg2(val, vn);
      }
    }
  }

  return is_irreg;
}


/**********************************************************************
 * is_distributed()
 */
Boolean
IrrSymTab::is_distributed(AST_INDEX node, int dim)
{
  Boolean                result;
  FortranDHashTableEntry *sp;
  const char             *array_name;

  // 2/16/94 RvH: Should replace this w/ calls to FortDInterface
  array_name = ref2varName(node);
  sp         = fd->GetEntry((char*)array_name);
  result     = (Boolean) ((sp != 0) && (sp->d->dec_name != 0));
  if (result && (dim >= 0))
  {
    result = (Boolean) (sp->d->sp[0][0].distinfo[dim]->distr_type != FD_DIST_LOCAL);
  }

  return result;
}


/**********************************************************************
 * is_local()
 */
Boolean
IrrSymTab::is_local(const char *name)
{
  // For now assume that all vars in string_ht are local
  Boolean result = string_ht->query_entry((char *)name);

  return result;
}


/**********************************************************************
 * get_size_node()  Get AST for number of elements.
 *                  dim = 0:   For all dimensions
 *                  dim != 0:  Only for dimension <dim>
 */
AST_INDEX
IrrSymTab::get_size_node(const char *name, int dim)
{
  fst_index_t index;
  ArrayBound  *ab;
  int         next_size, i, last_dim;
  AST_INDEX   size_node      = AST_NIL;
  AST_INDEX   next_size_node = AST_NIL;

  index  = fst_Index(symtab, (char *)name);
  ab = (ArrayBound*) fst_GetFieldByIndex(symtab, index,
                                         SYMTAB_DIM_BOUNDS);
  last_dim = dim ? dim
    : fst_GetFieldByIndex(symtab, index, SYMTAB_NUM_DIMS);
  size_node = pt_gen_int(1);

  // dim = 0:  Iterate i from 0 to <number of dims> - 1
  // dim != 0: Iterate i from <dim> to <dim>
  for (i = dim; i < last_dim; i++)
  {
    // Assume lower array bound 1; o/w dynamic accesses have to be
    // converted further than they currently are
    if (fst_bound_is_const_ub(ab[i]))
    {
      next_size      = ab[i].ub.value.const_val;
      next_size_node = pt_gen_int(next_size);
    }
    else
    {
      next_size_node = ab[i].ub.value.ast;
    }
    size_node = pt_simplify_expr(pt_gen_mul(next_size_node, size_node));
  }

  return size_node;
}


/**********************************************************************
 * get_type()
 */
int
IrrSymTab::get_type(const char *name)
{
  fst_index_t index;
  int         type;

  index = fst_Index(symtab, (char *)name);
  type  = fst_GetFieldByIndex(symtab, index, SYMTAB_TYPE);

  return type;
}


/**********************************************************************
 * get_elmt_size()
 */
int
IrrSymTab::get_elmt_size(const char *name)
{
  fst_index_t index;
  int         type, size;

  index = fst_Index(symtab, (char *)name);
  type  = fst_GetFieldByIndex(symtab, index, SYMTAB_TYPE);
  size  = type_to_size_in_bytes[type];

  return size;
}


/**********************************************************************
 * get_rank()
 */
int
IrrSymTab::get_rank(const char *name)
{
  fst_index_t index;
  int         rank;

  index = fst_Index(symtab, (char *)name);
  rank  = fst_GetFieldByIndex(symtab, index, SYMTAB_NUM_DIMS);

  return rank;
}


/**********************************************************************
 * get_fresh_int()  Generate fresh integer
 */
const char *
IrrSymTab::gen_fresh_int_name()
{
  const char *name = gen_fresh_name("i$");

  return name;
}
/*
char *
IrrSymTab::getInt(int index)
{

  assert(index >= 0 && index < MAX_INTS);

  if NOT(ints_declared[index])        // This scalar int not declared yet ?
  {
    ints[index] = new char[3];        // Our scalars are called
    (void) strcpy(ints[index], "i$"); // i$, j$, k$, ...
    ints[index][0] += index;
    assert(declInt(ints[index]));
    ints_declared[index] = true;
  }

  return ints[index];
}
*/


/**********************************************************************
 * gen_fresh_name()  Generate a new variable name, like "x$arr3"
 */
const char * 
IrrSymTab::gen_fresh_name(const char *id_name,   // Base name; "x"
			  const char *ext_name)  // Extension; "$arr"
{
  const int cnt_max = 1000;                // Max # of tries
  int   cnt     = 1;
  int   len     = strlen(id_name) + strlen(ext_name);
  int   max_len = len + 4;
  char  *name   = new char[max_len];

  (void) strcpy(name, id_name);
  (void) strcat(name, ext_name);

  // If <id_name><ext_name> is already present, then try
  // <id_name><ext_name><cnt>, with 2 <= <cnt> < cnt_max.
  while ((cnt < cnt_max) && string_ht->query_entry(name))
  {
    sprintf(&name[len], "%d", ++cnt);
  }

  if (cnt == cnt_max)
  {
    cerr << "WARNING gen_fresh_name(): Exceeded " << cnt_max <<
      " number of tries for declaring " << name << ".\n";
  }
  else
  {
    string_ht->add_entry(name);
  }

  return name;
}


/**********************************************************************
 * break_line_if_needed()  Prevent too long lines
 */
void
IrrSymTab::break_line_if_needed(AST_INDEX &node, int &len)
{
  AST_INDEX stmt_node, type_node, new_node, new_stmt_node;

  if (len > MAX_XLINE_LENGTH)
  {
    assert(is_list(node));
    stmt_node     = ast_get_father(node);
    assert(is_type_statement(stmt_node));
    type_node     = tree_copy(gen_TYPE_STATEMENT_get_type_len(stmt_node));
    new_node      = list_create(AST_NIL);
    tree_replace(node, new_node);
    new_stmt_node = gen_TYPE_STATEMENT(AST_NIL, type_node, node);
    ft_SetComma(new_stmt_node, false);
    list_insert_before(stmt_node, new_stmt_node);
    node          = new_node;
    len           = INIT_XLINE_LENGTH;
  }
}


/**********************************************************************
 * declInt()  Declare scalar int
 */
Boolean                      // No conflicts w/ existing declarations ?
IrrSymTab::declInt(const char* name)
{
  AST_INDEX name_node;                 // name node of scalar int
  Boolean   success = false;

  if (findName(name) != NULL)          // Var already declared ?
  {
    cerr << "WARNING IrrSymTab::declInt(): " << name <<
	" is already declared.\n";
  }
  else
  {
    name_node = pt_gen_ident((char *) name);
    
    if (ints_node == AST_NIL)           // No scalar int declared yet ?
    {
      // Create a list of scalar ints
      ints_node = list_create(AST_NIL);
      addDecl(gen_TYPE_STATEMENT(AST_NIL, 
				 gen_TYPE_LEN(gen_INTEGER(), AST_NIL), 
				 ints_node));
      ints_len = INIT_XLINE_LENGTH;
    }

    // Typechecker requires scalars to be zero-dim arrays
    break_line_if_needed(ints_node, ints_len);
    ints_node = list_insert_last(ints_node, gen_ARRAY_DECL_LEN(
		     name_node, AST_NIL, AST_NIL, AST_NIL));  
    string_ht->add_entry((char *) name);    // Add to table of local vars
    // Determine length of "<name>, "
    ints_len += strlen(name)+2;
    success = true;
  }

  return success;
}


/**********************************************************************
 * declIntParam()  Declare scalar int and make it a parameter
 */
Boolean                      // No conflicts w/ existing declarations ?
IrrSymTab::declIntParam(const char* name, AST_INDEX val)
{
  AST_INDEX param_elt_node;            // Parameter element
  Boolean   success = false;
   
  if (declInt(name))                   // Can declare integer ?
  {
    if (params_node == AST_NIL)           // No parameters yet ?
    {
      // Create a list of params
      params_node = list_create(AST_NIL);
      decls_node  = list_insert_after(decls_node,
				      gen_PARAMETER(AST_NIL,
						    params_node));
    }
    param_elt_node = gen_PARAM_ELT(pt_gen_ident((char *) name), val);
    params_node    = list_insert_last(params_node, param_elt_node);  
    success        = true;
  }

  return success;
}


/**********************************************************************
 * decl1dArr() Declare 1-dimensional array
 */
Boolean                      // No conflicts w/ existing declarations ?
IrrSymTab::decl1dArr(const char* name,     // Name
                     int         size,     // # of elements
                     int         type)     // Type (default: Integer)
{
  SNODE     *sp;
  AST_INDEX arr_node;
  AST_INDEX type_node;
  Boolean   no_conflict = true;
  Boolean   gen_new     = true;

  // Should this be a dynamic array ?
  if (size == UNKNOWN_SIZE)
  {
    // 6/9/93 RvH: For now, just declare it as a normal array w/ some
    //             bogus size.
    size = DynamicArraySize;
  }

  sp = findName(name);
  if (sp != NULL)                           // Var already declared ?
  {
    // Make sure the previous declaration matches in dimensionality
    no_conflict = (Boolean) (sp_is_array(sp) && (sp_numdim(sp) == 1));
      
    if (no_conflict)
    {                   // Expand size of declared array if necessary
      sp_put_lower(sp, 0, min(sp_get_lower(sp, 0), 1));
      sp_put_upper(sp, 0, max(sp_get_upper(sp, 0), size));
    }
    else
    {
      cerr << "WARNING IrrSymTab::decl1dArr(): " <<
	"declaration conflict on " << name << ".\n";
    }
  }
  else
  {                                  // Var is not declared yet
    if (arrs_node[type] == AST_NIL)    // No int array declared yet ?
    {
      type_node = gen_node(MallType_asts[type]);
      arrs_node[type] = list_create(AST_NIL); // List of arrs
      addDecl(gen_TYPE_STATEMENT(AST_NIL, 
                                 gen_TYPE_LEN(type_node, AST_NIL),
                                 arrs_node[type]));
      arrs_len[type] = INIT_XLINE_LENGTH;   // Start w/ conservative estimate
    }

    string_ht->add_entry((char *) name);    // Add to table of local vars
    arr_node      = gen1dArrDecl(name, size);
    // 4/29/93 RvH: this is now taken care of by the regular compiler
    // 4/30/93 RvH: actually we need this to exclude duplicates,
    //              BUT to avoid later conflicts w/ the regular world,
    //              we now have to create (& destroy) our own ihash
    store_sym_table(arr_node, ARRAYTYPE); // Generate new SNODE

    break_line_if_needed(arrs_node[type], arrs_len[type]);
    arrs_node[type] = list_insert_last(arrs_node[type], arr_node);
    // Determine length of "<name>(<size>), "
    arrs_len[type] += strlen(name)+5+int(floor(log10(size)));
  }

  return no_conflict;
}


/**********************************************************************
 * decl1dArr() Declare 1-dimensional array
 *             This version is based on already existing AST nodes
 *             for name & size.
 *             NOTE: these nodes are copied before use.
 */
Boolean                      // No conflicts w/ existing declarations ?
IrrSymTab::decl1dArr(AST_INDEX name_node, // Name
                     AST_INDEX size_node, // # of elements
                     int       type)     // Type (default: Integer)
{
  SNODE     *sp;
  Boolean   no_conflict = true;
  Boolean   gen_new     = true;
  AST_INDEX arr_node, type_node, dim_list;
  char      *name = gen_get_text(name_node);

  sp = findName(name);
  if (sp != NULL)                           // Var already declared ?
  {
    cerr << "WARNING IrrSymTab::decl1dArr(): " <<
	"declaration conflict on " << name << ".\n";
  }
  else
  {                                  // Var is not declared yet
    if (arrs_node[type] == AST_NIL)    // No int array declared yet ?
    {
      type_node = gen_node(MallType_asts[type]);
      arrs_node[type] = list_create(AST_NIL); // List of scalar int_arrs
      addDecl(gen_TYPE_STATEMENT(AST_NIL, 
                                 gen_TYPE_LEN(type_node, AST_NIL),
                                 arrs_node[type]));
    }

    string_ht->add_entry(name);       // Add to table of local vars
    dim_list = list_create(gen_DIM(AST_NIL, tree_copy(size_node)));
    arr_node = gen_ARRAY_DECL_LEN(tree_copy(name_node), AST_NIL,
                                  dim_list, AST_NIL);
    // 4/29/93 RvH: this is now taken care of by the regular compiler
    // 4/30/93 RvH: actually we need this to exclude duplicates,
    //              BUT to avoid later conflicts w/ the regular world,
    //              we now have to create (& destroy) our own ihash
    store_sym_table(arr_node, ARRAYTYPE); // Generate new SNODE

    arrs_node[type] = list_insert_last(arrs_node[type], arr_node);    
  }

  return no_conflict;
}


/*********************************************************************
 *  Private Methods                                                  *
 *********************************************************************/


/**********************************************************************
 * find_decls_node()  Find the place for putting declarations.
 */
AST_INDEX
IrrSymTab::find_decls_node(AST_INDEX root_node)
{
  AST_INDEX node, prev;

  node = list_last(find_stmt_list(root_node));

  // 4/29/93 RvH: copied from dc_storage()
  for (prev = list_prev(node);
       (prev != AST_NIL) && !is_type_statement(prev) && !is_common(prev);
       node = prev, prev = list_prev(node))
    ;

  return node;
}


/**********************************************************************
 * findName()  Find variable of given Name.
 */
SNODE *
IrrSymTab::findName(const char *id) const
{
  SNODE   *sp   = findadd((char *) id, false, 0, (SNODE**) ihash);
  Boolean found = (Boolean) (sp && !strcmp(sp->id, id));

  return (found ? sp : NULL);
}


/**********************************************************************
 * addDecl()  Add a declaration to AST.
 */
void
IrrSymTab::addDecl(AST_INDEX decl_node)
{
  if (!decls_exist) {                   // No declarations yet ?
    decls_node  = find_decls_node(ft_Root(ft));
    decls_node  = gen_decls_comment(decls_node);  // Insert comment
    decls_exist = true;
 }

  // The following cosmetic hack derived from looking at
  // /rn/src/ned_cp/FortTextTree/FortParse1/gram1.y
  ft_SetComma(decl_node, false);
  decls_node = list_insert_after(decls_node, decl_node);
}


/**************************************************************/
/* Is it an array ?                                           */
/* NOTE:                                                      */
/* SHOULD BE MOVED INTO fd_symtable.c                         */
/**************************************************************/
Boolean
IrrSymTab::sp_is_array(SNODE *sp)
{
  return (Boolean) (sp->fform == ARRAYTYPE);
}


/*********************************************************************
 * store variables found in the declaration list in the symbol table
 * 121592RvH: from decomp.c, where it was declared static and used dh
 */
void
IrrSymTab::store_sym_table(AST_INDEX node, enum FORM type)
{
  AST_INDEX curr, name_id, dimlist, upperb, lowerb;
  const char *name;
  SNODE *sp;
  int i, upperbound, lowerbound;
  int numdim = 0;

  switch (gen_get_node_type(node))
  {
    case GEN_IDENTIFIER:
      name = gen_get_text(node);
      if (sp = findadd((char *) name, 1, 0, ihash))
        init_var_snode(sp, numdim, type);
      break;

    case GEN_ARRAY_DECL_LEN:
      name_id = gen_ARRAY_DECL_LEN_get_name(node);
      name = gen_get_text(name_id);
      dimlist = gen_ARRAY_DECL_LEN_get_dim_LIST(node);

      if (dimlist == AST_NIL)  /* is actually a scalar */
      {
        if (sp = findadd((char *) name, 1, 0, ihash))
          init_var_snode(sp, numdim, type);
        break;
      }

      numdim = list_length(dimlist);

      sp = findadd((char *) name, 1, 0, ihash);
      init_var_snode(sp, numdim, type);
      sp->numdim = numdim;

      curr = list_first(dimlist);

      for (i = 0; i < numdim; i++)
      {
        /*** if the upper bound of the array is a constant ***/
        /***       store its value in the symbol table     ***/

        upperb = gen_DIM_get_upper(curr);
        lowerb = gen_DIM_get_lower(curr);

        if (is_constant(upperb))
        {
          upperbound = atoi(gen_get_text(upperb));
          sp_put_upper(sp,i, upperbound);
        }
        else
        {
	  // 9/18/93 RvH: Want to allow symbolic constants
          cerr << "WARNING IrrSymTab::store_sym_table(): "
	    << "non-constant size for array " << name << ".\n";
        }

        if ((lowerb != AST_NIL) && is_constant(lowerb))
        {
          lowerbound = atoi(gen_get_text(lowerb));
          sp_put_lower(sp, i, lowerbound);
        }
        if(lowerb == AST_NIL)
				{
        sp_put_lower(sp, i, 1);
			  }
        curr = list_next(curr);
      }
      break;
  }
}


/**********************************************************************
 * gen1dArrDecl()  Generate an AST used to declare a 1-dimensional array
 */
AST_INDEX
IrrSymTab::gen1dArrDecl(const char *name, int size)
{
  return genVarDecl(name, 1, &size);
}


/**********************************************************************
 * genVarDecl()  Generate an AST to be used in a variable declaration
 */
AST_INDEX               // constructed AST
IrrSymTab::genVarDecl(const char *name,  // variable name
		      int  rank,   // # of dimensions (0 for scalar)
		      int  *dims)  // extent of each dimension
{
  AST_INDEX name_node, size_node, arr_node, dim_node, dim_list;
  int       dim;
  //char *size_str, str[MAX_DIGITS];
  //int  size_str_len;

  assert (rank >= 0  &&  rank < MAXDIM);

  if (rank == 0)        // Scalars have to be 0-dimensional arrays
    dim_list = AST_NIL;
  else {      
    dim_list = list_create(AST_NIL);
    for (dim = 0; dim < rank; dim++)
    {   
      size_node    = pt_gen_int(dims[dim]);
      dim_node     = gen_DIM(AST_NIL, size_node);
      dim_list     = list_insert_last(dim_list, dim_node);
    }
  }

  name_node = pt_gen_ident((char *) name);
  arr_node  = gen_ARRAY_DECL_LEN(name_node, AST_NIL, dim_list, AST_NIL);

  return arr_node;
}


/**********************************************************************
 * gen_decls_comment()  Insert comments for declarations
 */
AST_INDEX
IrrSymTab::gen_decls_comment(AST_INDEX decls_node)
{
  AST_INDEX node;

  decls_node = append_blank_line(decls_node);
  node = genComment("--<< Fortran D/irreg variable declarations >>--");
  decls_node = list_insert_after(decls_node, node);
  
  node = genComment("--<< END Fortran D/irreg variable declarations >>--");
  node = list_insert_after(decls_node, node);
  (void) append_blank_line(node);

  return decls_node;
}


