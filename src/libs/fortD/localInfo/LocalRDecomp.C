/* $Id: LocalRDecomp.C,v 1.28 1997/03/11 14:28:42 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>
#include <string.h>
#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#include <libs/frontEnd/ast/ast_include_all.h>
#include <libs/frontEnd/fortTree/fortsym.h>
#include <libs/frontEnd/ast/forttypes.h>
#include <libs/frontEnd/ast/treeutil.h>

//#include <InitInfo.h>

#include <libs/fortD/performance/staticInfo/SDDF_Instrumentation.h>

#undef is_open

/* C++ includes for mod/ref information */

#include <libs/ipAnalysis/ipInfo/iptree.h>
   /* stream.h must be after iptree.h due to redefinition  */
   /* of certain routines by system files on the RS6000    */
   /* machines.                                            */
//#include <sys/stream.h>
#if 0
#include <libs/ipAnalysis/ipInfo/module.h>
#endif
#include <libs/ipAnalysis/ipInfo/iptypes.h>
#include <libs/fortD/misc/FortD.h>
#include <libs/ipAnalysis/ipInfo/CallSite.h>

/*------------------- GLOBAL DECLARATIONS -------------------*/

//EXTERN(Boolean,   match_pattern_insens,        (const char *&str,
//						const char *pattern));
//EXTERN(void,      ValDecompInfo_add_directive, (DecEntry     *d,
//						struct dc_id *id,
//						AST_INDEX    node));

#define  FD_DECOMP_LEN 13
#define  FD_ALIGN_LEN  5
#define  FD_DIST_LEN 10

/*------------------ LOCAL DECLARATIONS ---------------------*/

STATIC(void, convert_ph, (DecEntry *d, int dim_num, struct dc_expr *expr, 
                          int maxdim, AST_INDEX node));
STATIC(char*, parse_expr, (char *str, struct dc_expr *expr));
STATIC(char*, parse_subs, (char *str, struct dc_subs *subs));
STATIC(char*, parse_id, (char *str, struct dc_id *id));
STATIC(char*, parse_list, (char *str, struct dc_list *list));
// These prototypes can't be in the file because the Solaris 
// compiler complains about two external definitions.
//EXTERN(int, strcasecmp, (const char *str, const char* str));
//EXTERN(int, strncasecmp, (const char *str, const char* str, int n));

//-------------------------------------------------------------
// Returns true if it is a FortranD directive                           
// else return false                                                    
//-------------------------------------------------------------
Boolean FortranDInfo::FortranD_dir(AST_INDEX stmt)
{
  str_t = gen_get_text(gen_COMMENT_get_text(stmt));

  /* check whether CALL is a Fortran D specification */

  if (!strncasecmp("decomposition", str_t, FD_DECOMP_LEN))
   {
    comment_type = DECOMPOSITION;
    return true;
   }
  else if (!strncasecmp("align", str_t, FD_ALIGN_LEN))
   {
    comment_type = ALIGN;
    return true;
   }
  
  else if (!strncasecmp("distribute", str_t, FD_DIST_LEN))
   {
   comment_type = DISTRIBUTE;
   return true;
   }
   
  else 
  {
   comment_type = NODEC;
   return(false);
  }
}


//-------------------------------------------------------------
// Store Decomposition Information                           
//-------------------------------------------------------------
void FortranDInfo::StoreDecomposition(AST_INDEX node)
{
  char *name;
  int j, numdim;
  struct dc_list list;
  FortranDHashTableEntry *d1 = NULL;

  char *str = str_t+FD_DECOMP_LEN;

  while (*str == ' ')  /* eat spaces */
    str++;

  /* parse list of decompositions */

  str = parse_list(str, &list);
  if (*str){
   cout << "store_decomposition(): Illegal list of decompositions\n";
   exit(0);
   }
  /* get decomposition name */
  for (j = 0; j < list.num; j++)
  {
    name = list.id[j].str;

   if (list.id[j].ident){
       cout << "DECOMPOSITION decl missing size"; 
       exit(0); 
    }

    numdim = list.id[j].subs.num;
   
   d1 = GetEntry(name); 
   if(!d1){
    d1 = AddDecomp(name, DECOMPTYPE, numdim);
    namedlist->append_entry(name);
   }
  else 
   {
    cout << "name of DECOMPOSITION is not unique";
    exit(0);
   }
    d1->d->AddDecEntry(list.id[j], numdim);
    d1->d->AddDecompIdNumber(node, ft);
#if 0
    d1->d->AddDecompContext(contxt);
#endif
   }
  // 94/6/17 MAA Added this to gather SDDF information
  SD_Get_Decomp_Info(ft,node,d1);
}

//------------------------------------------------------------
// Store Alignment information                               
//------------------------------------------------------------
void FortranDInfo::StoreAlign(SymDescriptor sym_t, AST_INDEX node, AST_INDEX proc_node)
{
  struct dc_list arrays;
  struct dc_id wth;
  struct dc_id decomp;
  int j;                 /* number of arrays to align */
  char *str = str_t + FD_ALIGN_LEN;
  while (*str == ' ')  /* eat spaces */
    str++;

  str = parse_list(str, &arrays);  /* parse ALIGN statement */
  str = parse_id(str, &wth);
  str = parse_id(str, &decomp);

  while (*str == ' ')  /* eat spaces */
    str++;

  if (*str)
    cout << "store_alignment(): Illegal ALIGN syntax\n";

  for (j = 0; j < arrays.num; j++)
     check_align_array(arrays.id+j, sym_t, proc_node);

  if (NOT(wth.ident) || (strcasecmp("with", wth.str)))
    cout << "store_alignment(): Illegal ALIGN syntax\n";

  check_align_decomp(&decomp, &arrays,  node);
}

//----------------------------------------------------------------
//  check_align_array()   check & process array in ALIGN statement
//----------------------------------------------------------------
void 
FortranDInfo::check_align_array(struct dc_id *id, SymDescriptor sym_t, AST_INDEX  proc_node)

{
  int i, numdim, index;
  char *name;
  char abuf[2];
  Boolean perfect_align = false;
  
  /* check first alignment argument, must be global array        */
  /* if it is simply an identifier then it is perfectly aligned  */
  /* with respect to a decomposition                             */

  name = id->str;
  if (id->ident)
    perfect_align = true;

  /*--------------------------------------------------------*/
  /* look up in symbol table */

    index = fst_Index(sym_t, name);

    numdim = fst_GetFieldByIndex(sym_t, index,
                                   SYMTAB_NUM_DIMS);
     
   FortranDHashTableEntry *array_name = GetEntry(name);
   if (!array_name){
    array_name = AddDecomp(name, ARRAYTYPE, numdim);
    namealist->append_entry(name);
    }

//-----------------------------------------
// get the array bounds information   
//-----------------------------------------
   ArrayBound *a = 
           (ArrayBound*) fst_GetFieldByIndex(sym_t, index, SYMTAB_DIM_BOUNDS);
  
  for(i=0;i<numdim;++i)
  {
   Expr_type expr_lb = Expr_complex;
   Expr_type expr_up = Expr_complex;
   int lb, up;
   if(fst_bound_is_const_lb(a[i]))
    {
     expr_lb = Expr_constant;
     lb = a[i].lb.value.const_val;
    }

   if(fst_bound_is_const_ub(a[i]))
    {
    expr_up = Expr_constant;
    up = a[i].ub.value.const_val;
    }
   array_name->d->AddBounds(i, up, lb, expr_lb, expr_up);

//---------------------------------------------------
// check if it is a global variable
// if yes store it in  a list so that decomposition 
// information gets propagated at callsites
//---------------------------------------------------

   CheckAndStoreCommonBlock(name, sym_t, 
                            gen_get_text(get_name_in_entry(proc_node)));

  }

//------------------------------------------------------------
// get the type information
//------------------------------------------------------------
   int type = fst_GetFieldByIndex(sym_t, index, SYMTAB_TYPE);
   array_name->put_form(form_type(type));

//-------------------------------------------------------------
// check placeholders for array, must be of form A(i,j,k,...) 
//-------------------------------------------------------------
  if (!perfect_align)
  {
   numdim = id->subs.num;
/*    if (numdim != sp->numdim)
      die_with_message("wrong # of dims for array in ALIGN");
*/
    abuf[1] = '\0';
    for (i = 0; i < numdim; i++)
    {
      abuf[0] = 'i' + i;

      if (strcasecmp(abuf, id->subs.expr[i].str)){
       cout << "wrong placeholder for ALIGN"; 
       exit(0); }
    }
  }
}

//-------------------------------------------------------------
// Store Distribution information                            
//-------------------------------------------------------------
void FortranDInfo::StoreDistrib(AST_INDEX node,
				Boolean   in_codegen)
{
  struct dc_list distrib;
  int j;                 /* number of arrays to align */
  char *name, *str;
  FortranDHashTableEntry *sp = NULL;

  str =  str_t + FD_DIST_LEN;
  while (*str == ' ')  /* eat spaces */
    str++;

  str = parse_list(str, &distrib);  /* parse DISTRIBUTE statement */

  if (*str)
    cout << "store_distribution(): Illegal DISTRIBUTE syntax\n";

  for (j = 0; j < distrib.num; j++)
  {  
    name = distrib.id[j].str;
   if (!(sp = GetEntry(name))) {
    cout << "DECOMPOSITION " << name << " not declared";
    exit(0);
    } 

    if (sp->fform != DECOMPTYPE) {
    cout << "Trying to DISTRIBUTE non-decomposition"; 
    exit(0);
    }
/*    if (distrib.id[j].ident || (distrib.id[j].subs.num != sp->numdim))
      die_with_message("store_distribution(): Illegal DISTRIBUTE syntax");
*/
    sp->d->AddDistribIdNumber(node, ft);
    sp->d->StoreDistribPattern(&distrib.id[j].subs, node);
    sp->d->AddDistribIndex(this);

    // 12/3/93 RvH Get Info for value based distributions here
    // (Although there must be a better place than this ...)
    if (in_codegen && (distrib.id[j].subs.expr[0].type == irreg_val))
    {
      // 2/28/93 RvH: This causes a linking problem
      //ValDecompInfo_add_directive(sp->d, &distrib.id[j], node);
    }
  }
  SD_Get_Distrib_Info(node,sp);
}

//---------------------------------------------------------------------
// check_align_decomp()   
//                check & process decomposition in ALIGN statement
//---------------------------------------------------------------------
void
FortranDInfo::check_align_decomp(struct dc_id *id, struct dc_list *arrays, AST_INDEX node)
{
  int i, j, numdim;
  char *name;
  FortranDHashTableEntry *dec_sp, *array_sp;
  
  name = id->str;
  
  /*--------------------------------------------------------*/
  /* look up in hash table */
  
  if (!(dec_sp = GetEntry(name)) ||
      (dec_sp->fform != DECOMPTYPE)){
    cout << "Trying to ALIGN with non-decomposition";
    exit(0);
  }
  
  numdim = dec_sp->getdim();           /* # of dims in decomp */
  
  for (j = 0; j < arrays->num; j++)
    {
      array_sp = GetEntry(arrays->id[j].str);
      
      array_sp->d->dec_name = ssave(dec_sp->name()); 
      
      /*--------------------------------------------------------*/
      /* check placeholders for decompositions */
      
      if (NOT(id->ident))
	{
	  if (numdim != id->subs.num){
	    cout << "check_align_decomp(): illegal ALIGN syntax";
	    exit(0);
	  }
	  
	  /* find dimensional alignment of decomp with respect to array */
	  array_sp->d->InitAlignInfo(numdim, node);
	  array_sp->d->AddAlignIdNumber(node, ft);
	  for(i =0; i< numdim; i++)
	    {
	      convert_ph(array_sp->d, i, id->subs.expr + i, array_sp->getdim(),node);
	    }
	  array_sp->d->a_index++;
	}
      
      /*--------------------------------------------------------------*/
      /* no placeholders specified, use default alignment if possible */
      
      //   if (numdim != array_sp->numdim) {
      //   cout << "default ALIGN illegal, different dimensional size";
      //   exit(0);
      //   }
      
      /*-----------------------------------------------------*/
      /* specify default alignment of decomp to/with array   */
      
      else
	{
	  array_sp->d->AddAlignIdNumber(node, ft);
	  array_sp->d->AddAlignInfo(numdim, node);
	}
      /*-------------------------------------------------------*/
      /* Add the list of array names to the decomposition entry*/
      
      dec_sp->d->AddDecompNameList(arrays); 
      SD_Get_Align_Info(node, dec_sp, array_sp);
    }
}

/*----------------------------------------------------------------

  StoreDistribPattern()  
  stores the distribution pattern for decomp
--------------------------------------------------------------------*/

void
DecEntry::StoreDistribPattern(struct dc_subs *subs, AST_INDEX node)
{
  int numdim, i;
  char *name;

  numdim = subs->num;
  /*  if (numdim != sp->numdim)
      die_with_message("StoreDistribPattern(): Illegal # of attributes");
      */
  distrib_info[d_index]->node = node;
  for (i = 0; i < numdim; i++)
  {
    name = subs->expr[i].str;
    
    if (subs->expr[i].type == irreg_val)
    {
      put_distrib_info3(FD_DIST_USER, name, 0,
			&(distrib_info[d_index]->distinfo[i]));
    }
    else
    {
      if (subs->expr[i].type != variable)
	cout << "StoreDistribPattern(): Illegal DISTRIBUTE attribute\n";
    
      if (!strcmp(":", name))  
      {
	put_distrib_info2(FD_DIST_LOCAL,0,
			  &(distrib_info[d_index]->distinfo[i]));
      }
      else if (!strcasecmp("block", name))
      {
	put_distrib_info(FD_DIST_BLOCK,
			 &(distrib_info[d_index]->distinfo[i]));
      }
      else if (!strcasecmp("cyclic", name))
      {
	put_distrib_info(FD_DIST_CYCLIC,
			 &(distrib_info[d_index]->distinfo[i]));
      }
      else if (!strcasecmp("block_cyclic", name))
      {
	put_distrib_info(FD_DIST_BLOCK_CYCLIC,
			 &(distrib_info[d_index]->distinfo[i]));
      }
      else
      {
	put_distrib_info3(FD_DIST_USER, name, 0,
			  &(distrib_info[d_index]->distinfo[i]));
      }
    }
  }
  ++d_index;
}

//---------------------------------------------------------------------
//  convert_ph()
// 
//  Add the alignment information based on the subscript expression
//  type
//---------------------------------------------------------------------

static void
convert_ph (DecEntry *d, int dim_num, struct dc_expr *expr, int maxdim, AST_INDEX node)
{
 /*----------------------------------------*/
 /* check for depth of placeholder */

  int index = expr->str[0] - 'i' + 1;

/*  if ((index > maxdim) || (expr->str[1] != '\0'))
    die_with_message("illegal ALIGN placeholder");
*/
  switch (expr->type)
  {
    case value:
      d->AddAlignInfo(dim_num, index, expr->val, 0,ALIGN_CONST);
      break;

    case variable:
     d->AddAlignInfo(dim_num, index, 0, 0,ALIGN_PERFECT);
     break;

    case plus:
      d->AddAlignInfo(dim_num, index, expr->val, 0,ALIGN_OFFSET);
      break;

    case unknown:
      cout << " illegal placeholder in ALIGN "; 
      exit(0);
      break;

    default:
      break;
  }
}


/*----------------------------------------------------------------------
  FortranDInfo::StoreReachDecomp()
  Store the list of decompositions for each of the parameters in the
  callsite. Attach the list of decompositions for globals that reach
  the callsite at the callsite.
*/
void FortranDInfo::StoreReachDecomp(CallSite *c_entry)
{
  ActualList *alist;
  ActualListEntry *a_entry;
  FortranDHashTableEntry *f;
  
  alist = c_entry->GetActuals();
  for(a_entry = alist->First(); a_entry != 0;
      a_entry = alist->Next()) {
    
    a_entry->fortd_set = (SinglyLinkedListIO*)(new FDSetEntryList());
    
    switch(a_entry->Type()){
    case VTYPE_PROCEDURE:
//  case VTYPE_COMMON_DATA:
    case VTYPE_STAR:
//  case VTYPE_COMPILER_TEMPORARY:
//  cout << "FortranD Compiler does not handle parameter type " << a_entry->Name()
//       << "\n"; 
      break;
      
    default:
      f = GetEntry(a_entry->Name());
      if (f == 0){
//   cout << "Variable " << a_entry->Name() << " has decomposition = TOP \n";
      }
      else {
	/* store decomposition, align, distribute */
// cout << "Variable " << a_entry->Name() << " has decomposition = not(TOP) \n";
#if 0
        /* also written by FortranDInfo::WriteCallInfo() */
	f->write(this, port);
#endif
	/* add set entries */
	AddSetInfo(f,(FDSetEntryList*)(a_entry->fortd_set));
      }
      break;
    }
  }
  StoreReachDecompGlobals( c_entry);
}

/*------------------------------------------------------------
 
  get_if()
  
  grab the if statement just outside the loop           

*/
AST_INDEX 
DecompListEntry::get_if(AST_INDEX node)
{
  node = tree_out(node);

  while (node != AST_NIL && !is_if(node))
    node = tree_out(node);

  return node;
}

/*-------------------------------------------------------------

  add_namelist()

  Add the names of entries in the ALIGN directive
  Add the name of the decomposition 
*/

void DecompListEntry::add_namelist(char *str_t)
{
  struct dc_list arrays;
  struct dc_id wth, decomp;
  int j;

  char *str = str_t + FD_ALIGN_LEN;
  while (*str == ' ')  /* eat spaces */
    str++;

  str = parse_list(str, &arrays);
  str = parse_id(str, &wth);
  str = parse_id(str,&decomp);

   if (*str)
    cout << "store_alignment(): Illegal ALIGN syntax\n";

   for (j   = 0; j < arrays.num; j++){
     nlist->append_entry(arrays.id[j].str);
    }

  decomp_name = ssave(decomp.str);
}


/*-------------------------------------------------------------

  intersect()

  perform an intersection of the namelists;

*/

NameList *DecompListEntry::intersect(NameList *align_namelist)  

{
  NameList *n;
  NameEntry *n_entry, *name_entry;
  Boolean done;
  n = new NameList();

 for(n_entry = align_namelist->first_entry();  n_entry != 0;
     n_entry = align_namelist->next_entry()) {
    done = false;
    while (!done){
    for(name_entry = nlist->first_entry(); name_entry != 0; 
        name_entry = nlist->next_entry()) {
      if(!strcmp(n_entry->name(), name_entry->name())) {
        n->append_entry(n_entry->name()); 
        nlist->delete_entry(name_entry);
        if(nlist->final_entry()) done = true;
      }
    }
   done = true;
  }
}
  return(n);
}
   
/*-------------------------------------------------------------

  invalidate()
    
   Part of the kill phase for an align directive
   invalidate the killed align directives resident 
   in the hash table
*/

void DecompListEntry::invalidate(FortranDInfo *fd, NameList *align_namelist)
{
 NameEntry *n_entry;
 FortranDHashTableEntry *fd_entry;
 Boolean done;
 int i;

 for(n_entry = align_namelist->first_entry();  n_entry != 0;
     n_entry = align_namelist->next_entry()) {
   
   fd_entry =  fd->GetEntry(n_entry->name());

   // get the align list to invalidate the entry
   done = false;
   i = 0;
   while(!done) {
     if(fd_entry->d->align_info[i]->node == invocation_node){
      done = true;
      fd_entry->d->align_info[i]->status(INACTIVE);
     }
    else 
    ++i;
    }
  }
} 

/*-------------------------------------------------------------

  delete_namelist()

   Part of the kill phase for an align directive
   delete the names from the decomposition's list of names   

*/
void
DecompListEntry::delete_namelist(FortranDInfo *fd, NameList *intersect_list)
{
 FortranDHashTableEntry *fd_entry;
 NameEntry *n_entry, *nme_entry;

  fd_entry =  fd->GetEntry(decomp_name); 

// get the namelist to remove the array entries that are killed

  for(n_entry = intersect_list->first_entry();  n_entry != 0;
      n_entry =  intersect_list->next_entry()) { 
   
    for(nme_entry = fd_entry->d->name_info->first_entry(); nme_entry != 0;
        nme_entry = fd_entry->d->name_info->next_entry()){
 
     if (nme_entry->name() == n_entry->name())
      fd_entry->d->name_info->delete_entry(nme_entry);
  }
 }
}
 

/*-------------------------------------------------------------

  add_decomp_name()

  Add the decomposition name to the  DecompListEntry for a
  distribute directive
  Restriction : only one decomposition specified in the 
  distribute statement
*/
void
DecompListEntry::add_decomp_name(char *str_t)
{
  struct  dc_list distrib;

  char *str =  str_t + FD_DIST_LEN;
  while (*str == ' ')  /* eat spaces */
    str++;

  str = parse_list(str, &distrib);  /* parse DISTRIBUTE statement */

  if (*str)
    cout << "store_distribution(): Illegal DISTRIBUTE syntax\n";

  if (distrib.num > 1) {
    cout << "store_distribution(): Only one decomposition supported \n";  
    exit(0);
   }
  decomp_name = ssave(distrib.id[0].str);
}

/*-------------------------------------------------------------
 
  invalidate()
   
  Invalidate the distribute entry in the hash table
*/
void
DecompListEntry::invalidate(FortranDInfo *fd)
{

 FortranDHashTableEntry *fd_entry;
 int i = 0;
 Boolean done = false;

  fd_entry =  fd->GetEntry(decomp_name);
    
    done = false;
    while(!done) {
     if (fd_entry->d->distrib_info[i]->node == invocation_node)
     {
      done = true;
      fd_entry->d->distrib_info[i]->status(INACTIVE);
     }
    else 
    ++i;
   }
}

/*-------------------------------------------------------------
  
   StoreFortranD()

   kill all the entries i.e. align and distribute statements
   store the Fortran D directive information 

   get the comment type and check if the statements are surrounded by
   the same if statement
 
   If yes then perform specific operations depending on comment type

   If ALIGN :  Generate a_set = interection arrays(d, d_entry)
   For every element in a_set, invalidate entry in the hash table if the
   entry exists
   Delete entries in a_set from the namelist of the decomposition

   If DISTRIBUTE : dtrue = d_entry->decomp_name = decomp_name
   If (dtrue) :
   invalidate the distribute entry in d_entry
   invalidate distribute entry in the hash table
   add a new distribute entry for the decomposition and update the
   information for the arrays in namelist to reflect the new distribution 

*/
void
FortranDInfo::StoreFortranD(AST_INDEX     node,
			    AST_INDEX     proc_node,
			    SymDescriptor sym_t,
			    Boolean       in_codegen)
{
  DecompListEntry *d, *d_entry;
  NameList        *i_namelist;
  Boolean         dtrue = false;

  d = new DecompListEntry(node, comment_type);
 
   switch(comment_type) {
    case DECOMPOSITION:
    StoreDecomposition(node);
    break;

    case ALIGN:
    d->add_namelist(str_t);
    break;
    
    case DISTRIBUTE:
    d->add_decomp_name(str_t);
    break;

    default:
    break;
    }    

 for(d_entry = dlist->first_entry(); d_entry != 0; 
     d_entry = dlist->next_entry())  {

  if (comment_type == d_entry->type() && (d->if_st() == d_entry->if_st()))
   {
   switch(comment_type) {
    case ALIGN:
     i_namelist =  d_entry->intersect(d->get_namelist());
     d_entry->invalidate(this, i_namelist);
     d_entry->delete_namelist(this, i_namelist);
     break;

    case DISTRIBUTE:
     if(d_entry->compare(d->name())) {
      d_entry->invalidate(this);
      dlist->delete_entry(d_entry);   /* the entry gets killed */
     }
    break;
     }
    }
   }

  switch(comment_type){
    case ALIGN:
    StoreAlign(sym_t, node, proc_node);
    break;
   
    case DISTRIBUTE:
    StoreDistrib(node, in_codegen);
    
    // 12/3/93 RvH: Want to allow multiple intraprocedural distributions
    //              ... but rather do it in dc_compute_decomp(), where
    //              we can access Ped
    //if(in_codegen)
    //{
      //store_decomp_sp(GetEntry(str_t));
      //distrib_index = d->id_number - 1;
      //put_info(f_annot->Ped(f_annot->proc()), node, type_fd, (int) indices);
    //}
    break;
   }
    dlist->append_entry(d);
}

 

//---------------------------------------------------------------
// Create FortranD Set information for each actual parameter
// SetEntry = (<decomp_name>, <align_index>, <distrib_index>)
//---------------------------------------------------------------
void FortranDInfo::AddSetInfo(FortranDHashTableEntry *fd, 
FDSetEntryList *fortd_set)
{
 int i,j;
 FortranDHashTableEntry *f;

     // go through the distrib_index structure, dist_index times 
     // get the distrib structure 
     // check the state field, if ACTIVE, for each dimension, print
     // dist_type


  if (!(f = GetEntry(fd->d->dec_name))) {
   cout << "DECOMPOSITION " << fd->d->dec_name << " not declared";
   exit(0); 
  }

  for(j=0;j<fd->d->a_index;++j){
   if(fd->d->align_info[j]->state == ACTIVE){
    for(i=0; i<fd->d->dist_index; ++i){
     if (f->d->distrib_info[fd->d->distrib_index[i]]->state == ACTIVE)
      fortd_set->append_entry(fd->d->dec_name, j, fd->d->distrib_index[i]);
    }  
   }
  }
}

//--------------------------------------------------------
// DistribEntryArray class writes itself to the database
//--------------------------------------------------------
int DistribEntryArray::write(FormattedFile& port, int numdim)
{
 int i;

 if (state == ACTIVE){
  for(i=0;i<numdim;++i){
  // cout << "Distribution Type = " << distinfo[i].dist_type << "\n";
  }
  }
  return 0; // success ?!
}

//----------------------------------------------------
// DecEntry class writes itself to the database
//----------------------------------------------------
void DecEntry::write(FortranDInfo *f, 
                     FormattedFile& port, enum FORM fform)
{
 int i;

 switch(fform) {
 case DECOMPTYPE:
 break;

     // go through the distrib_index structure, dist_index times 
     // get the distrib structure 
     // check the state field, if ACTIVE, for each dimension, print
     // dist_type

 case ARRAYTYPE:
 FortranDHashTableEntry *fd;

  if (!(fd = f->GetEntry(dec_name))) {
   cout << "DECOMPOSITION " << dec_name << " not declared";
   exit(0); 
  }

 for(i=0; i<dist_index; ++i){
   // cout << "Decomposition = " << fd->name() << "\n";
    fd->d->distrib_info[distrib_index[i]]->write(port, fd->getdim());
  }  
 break;
 }
}

//----------------------------------------------------------
// FortranDHashTableEntry writes itself out to the database
//----------------------------------------------------------
void FortranDHashTableEntry::write(FortranDInfo *f, FormattedFile& port)
{
 // cout << "name = " << name() << ", form = " << fform << "\n";
 
 d->write(f, port, fform);
}
 
/*----------------------------------------------------------------

  parse_expr()

*/

static char *
parse_expr(char *str, struct dc_expr *expr)
{
  char *s;

  while (*str == ' ')  /* eat up spaces */
    str++;

  if (*str == ':')     /* special case ":" as variable */
  {
    expr->type = variable;
    strcpy(expr->str, ":");
    return ++str;
  }

  // 12/3/93 RvH: Check for value-based decompositions
  if (!strncasecmp("value(", str, strlen("value(")))
  {
    str = &str[strlen("value(")];
    while (*str == ' ') str++; // Eat spaces    
    expr->type = irreg_val;
    s = expr->str;
    while (*str != ')')
      *s++ = *str++;
    *s = '\0';
    str++;      // Get past ')'

    return str;
  }

  if (isalpha(*str))
  {
    expr->type = variable;
    s = expr->str;
    while (isalpha(*str) || isdigit(*str) || (*str == '$'))
      *s++ = *str++;
    *s = '\0';

    if (*str == '+')
    {
      str++;
      while (*str == ' ')  /* eat up spaces */
        str++;

      if (!isdigit(*str))
      {
        printf("parse_expr(): Illegal expression\n");
        expr->type = unknown;
        return str;
      }

      expr->type = plus;
      sscanf(str, "%d", &expr->val);
      while (isdigit(*str))
        str++;
    }
    else if (*str == '-')
    {
      str++;
      while (*str == ' ')  /* eat up spaces */
        str++;

      if (!isdigit(*str))
      {
        printf("parse_expr(): Illegal expression\n");
        expr->type = unknown;
        return str;
      }

      expr->type = plus;
      sscanf(str, "%d", &expr->val);
      expr->val = -expr->val;

      while (isdigit(*str))
        str++;
    }
  }

  else if (isdigit(*str))
  {
    expr->type = value;
    sscanf(str, "%d", &expr->val);
    while (isdigit(*str))
      str++;
  }
  else
  {
    expr->type = unknown;
    printf("parse_expr(): Illegal expression\n");
    return str;
  }

  while (*str == ' ')  /* eat up spaces */
    str++;

  return str;
}

/*----------------------------------------------------------------

  parse_subs()

*/

static char *
parse_subs(char *str, struct dc_subs *subs)
{
  subs->num = 0;

  str++;  /* get past "(" */

  while (true)
  {
    str = parse_expr(str, subs->expr + subs->num++);

    while (*str == ' ')  /* eat up spaces */
      str++;

    if (*str == ')')
      return ++str;

    if (*str++ != ',')
    {
      printf("parse_subs(): Illegal syntax\n");
      return str;
    }
  }
}

/*----------------------------------------------------------------

  parse_id()

*/

static char *
parse_id(char *str, struct dc_id *id)
{
  char *s;
  
  while (*str == ' ')  /* eat up spaces */
    str++;
  
  s = id->str;
  while (isalpha(*str) || isdigit(*str) || (*str == '$'))
    *s++ = *str++;
  *s = '\0';
  
  if (*str == '(')
  {
    str = parse_subs(str, &id->subs);
    id->ident = false;
  }
  else
  {
    id->ident = true;     /* not a subscripted variable */
  }

  while (*str == ' ')  /* eat up spaces */
    str++;

  return str;  
}


/*----------------------------------------------------------------

  parse_list()

*/

static char *
parse_list(char *str, struct dc_list *list)
{
  list->num = 0;

  while (true)
  {
    str = parse_id(str, list->id + list->num++);

    while (*str == ' ')  /* eat up spaces */
      str++;

    if (*str != ',')
      return str;

    str++;
  }
}
