/* $Id: local_decomp.C,v 1.18 2001/09/14 18:28:15 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//-------------------------------------------------------------------------
// author   : Seema Hiranandani
//
// content  : The routines in this file store the decomposition reaching 
//            each left hand side and right hand side of an assignment 
//            statement in the side array as an SNODE structure. This
//            will be used during code generation to determine symbol
//            table information for variables and arrays.
//            Future Extensions: I/O, conditional statements, symbolic
//            information, irregular distributions, array indirection,
//            reshaping.............
// date     : August 1992
//-------------------------------------------------------------------------

//-------------------
// include files 

#include <libs/fortD/misc/FortD.h>
#include <libs/fortD/driver/driver.h>
#include <libs/moduleAnalysis/dependence/utilities/side_info.h>
#include <libs/support/tables/symtable.h>
#include <libs/frontEnd/ast/treeutil.h>
#include <libs/fortD/localInfo/LocalOverlap.h>
#include <libs/fortD/localInfo/fd_symtable.h>
#include <libs/frontEnd/fortTree/fortsym.h>

#include <libs/ipAnalysis/problems/fortD/FortDNumProcsAnnot.h>

#undef DISTRIBUTE

//---------forward declarations---------//
static SNODE* get_sp (FortranDHashTableEntry *array_entry, FortranDHashTableEntry *decomp_entry,  OverlapList*, AST_INDEX node, FD_Composition_HT *f_annot);
static int    store_sp_side_array(AST_INDEX node, Generic f);
static int    dc_compute_decomp  (AST_INDEX stmt, int level, Generic f);
static void   map_overlap_info(SNODE*, OverlapList*);
static int    dc_store_decomp(AST_INDEX stmt, int level, Generic f);

//----------extern declarations----------//
EXTERN(Boolean,   directive_dominates_ref, (AST_INDEX dir_node,
					    AST_INDEX ref_node));
EXTERN(void, map_decomp_to_sp, (FortranDHashTableEntry *decomp_entry,
                           SNODE *sp, int d_index));
EXTERN(void, map_set_to_sp,  (SNODE *sp,
                           FortranDHashTableEntry *array_entry,
                           FortranDHashTableEntry *decomp_entry,
                           int a_index, int d_index));

void map_decomp(SNODE *sp, int numberOfProcessors);

//-----------------------------------------------------------------------
// walk the procedure looking for fortran d statements and array reference
// variables
//-----------------------------------------------------------------------
void
dc_compute_local_decomp(AST_INDEX node, FD_Composition_HT *f)
{
  walk_statements(node, LEVEL1, dc_compute_decomp, NULL, Generic(f));
  walk_statements(node, LEVEL1, dc_store_decomp, NULL, Generic(f));
}

//-----------------------------------------------------------------------
// look for the type statement and store the 
//-----------------------------------------------------------------------
static int dc_compute_decomp(AST_INDEX stmt, int level, Generic f)
{
  FortranDInfo      *fdi;
  SymDescriptor     symtable;
  int               distrib_index;
  FD_Composition_HT *fd = (FD_Composition_HT*)f;

  switch(gen_get_node_type(stmt))
  {
  case GEN_COMMENT:
    if (fd->proc()->proc_annot->f->FortranD_dir(stmt))
    {
      symtable = fd->proc()->symtable();
      fdi      = fd->proc()->proc_annot->f;
      fdi->StoreFortranD(stmt, fd->proc()->get_ast(), symtable, true);

      // 12/3/93 RvH: Store current Distribution to allow multiple
      //              intraprocedural distributions
      if (fdi->comment_type == (enum FDtype)DISTRIBUTE)
      {
	distrib_index = ((DecompListEntry*)(fdi->getDlist()->Last()))->id_number;
	put_info(fd->Ped(fd->proc()), stmt, type_fd, distrib_index);
      }
    }
    break;
  }
  return WALK_CONTINUE;
}
//-----------------------------------------------------------------------
// for every rhs and lhs store the symbol table entry in the side array
//-----------------------------------------------------------------------
static int
dc_store_decomp(AST_INDEX stmt, int level, Generic f)
{
 FD_Composition_HT *fd = (FD_Composition_HT*)f;

 switch(gen_get_node_type(stmt))
 {
  case GEN_TYPE_STATEMENT:
  case GEN_COMMON:
   walk_expression(stmt, store_sp_side_array, NULL, f);
   break;

  case GEN_ASSIGNMENT:
   store_sp_side_array(gen_ASSIGNMENT_get_lvalue(stmt), (Generic)fd);
   walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),
   store_sp_side_array, NULL, (Generic)fd);
   break;

  case GEN_IF:
  case GEN_LOGICAL_IF:
   walk_expression(stmt, store_sp_side_array, NULL, (Generic)fd);
   return WALK_SKIP_CHILDREN;
 }

 return WALK_CONTINUE;
}

//-----------------------------------------------------------------------
// store the symbol table entry for the node
//-----------------------------------------------------------------------
static int
store_sp_side_array(AST_INDEX node, Generic fd)
{
 SNODE *sp;
 FD_Composition_HT *f_annot = (FD_Composition_HT*)fd;
 OverlapList *overlap_info = 0;
 int fortd_type, numdim, index, i;
 char* name;
 SymDescriptor sym_t;

 if(!is_identifier(node) && (!is_subscript(node)))
 return WALK_CONTINUE;

 if (is_subscript(node))
  node  = gen_SUBSCRIPT_get_name(node);

//------------------------------------------------------------ 
// get the entry from the symbol table
// convert the information to represent a symbol table used in 
// the Fortran D compiler.
//------------------------------------------------------------ 

 FortranDHashTableEntry *array_entry =  
           f_annot->proc()->proc_annot->f->GetEntry(gen_get_text(node));

 if(array_entry != NULL)
  {                 

  FortranDHashTableEntry *decomp_entry =
           f_annot->proc()->proc_annot->f->GetEntry(array_entry->d->dec_name);
  overlap_info = f_annot->proc()->proc_annot->f->overlap_info;
  sp = get_sp(array_entry, decomp_entry, overlap_info, node, f_annot);

  PedInfo ped = f_annot->Ped(f_annot->proc());
  put_info(ped, node, type_fd, (int)sp);
  }
 else
  //---- get the information from the global symbol table ----//
  {
  sym_t = f_annot->proc()->symtable();
  name = gen_get_text(node);
  index = fst_Index(sym_t, name);
  numdim = fst_GetFieldByIndex(sym_t, index, SYMTAB_NUM_DIMS);

  fortd_type = fst_GetFieldByIndex(sym_t, index, SYMTAB_TYPE);
  enum FORM type = form_type(fortd_type);
  sp = (SNODE*) get_mem(sizeof(SNODE), "LocalDecomp");

  init_var_snode(sp, numdim, type);

  ArrayBound *a = 
           (ArrayBound*) fst_GetFieldByIndex(sym_t, index, SYMTAB_DIM_BOUNDS);

  memset(sp, 0, sizeof(SNODE));

  init_var_snode(sp, numdim, type);

  for(i=0;i<numdim;++i)
  {
   if(fst_bound_is_const_lb(a[i]))
    {
      int lb = a[i].lb.value.const_val;
      sp_put_lower(sp, i, lb);
    }
   if(fst_bound_is_const_ub(a[i]))
    {
      int up = a[i].ub.value.const_val;
      sp_put_upper(sp, i, up);
    }
   }
  put_info(f_annot->Ped(f_annot->proc()), node, type_fd, (int)sp);
 }


  return WALK_CONTINUE;
}


//-----------------------------------------------------------------------
// convert the entry to a form that fits the symbol table used in dc
//-----------------------------------------------------------------------
static SNODE*
get_sp(FortranDHashTableEntry *array_entry,
       FortranDHashTableEntry *d_entry,
       OverlapList            *overlap_info,
       AST_INDEX              node,
       FD_Composition_HT      *f_annot)
{
  int               j;
  int               align_index, index;
  SNODE             *sp;
  DistribEntryArray *da;
  int               dist_index      = -1;
  int               potential_index = -1;

  //----------------------
  // get the align index
  
  for(j=0;j<array_entry->d->a_index;++j)
  {
    if(array_entry->d->align_info[j]->state == ACTIVE)
      align_index = j;
  }
  
  //-------------------------
  // get the distribute index
  
  for(j=0;j<array_entry->d->dist_index;++j)
  {
    index = array_entry->d->distrib_index[j];
    da = d_entry->d->distrib_info[index];
    //if (da->state ==  ACTIVE)
    //{
    potential_index = index;

    // Distribution directive should dominate reference
    if (directive_dominates_ref(da->node, node))
    {
      if (dist_index != -1)
      {

	// Distribution directive should NOT dominate previous directive
	if (!directive_dominates_ref(da->node, d_entry->d
				     ->distrib_info[dist_index]->node))
	{
	  dist_index = potential_index;
	}
      }
      else
      {
	dist_index = potential_index;
      }
    }
  }

  if (dist_index == -1)
  {
    cout << "WARNING: get_sp(): no dominating DISTRIBUTION.\n";
    if (potential_index == -1)
    {
      cout << "WARNING: get_sp(): no reaching DISTRIBUTION.\n";
    }
    else
    {
      dist_index = potential_index;
    }
  }

  // 12/3/93 RvH: Retrieve *current* distribution to allow multiple
  //              intraprocedural distributions
  //dist_index = (int) get_info(f_annot->Ped(f_annot->proc()), node, type_fd);
  
  sp = array_entry->sp(align_index, dist_index);



  FortDNumProcsAnnot *nProcsAnnot = (FortDNumProcsAnnot *) 
    f_annot->callGraph->GetAnnotation(FORTD_NUM_PROCS_ANNOT, true);

  assert(nProcsAnnot);

  
  //--------------------------------------------------
  // check to see if the decomposition has an SNODE
  // if not, fill it with information
  //--------------------------------------------------
  if (sp_numdim(d_entry->sp(0,dist_index)) == 0)
  {
    map_decomp_to_sp(d_entry,d_entry->sp(0,dist_index), dist_index);
    map_decomp(d_entry->sp(0,dist_index), nProcsAnnot->numberProcs);
  }
  
  //--------------------------------------------------
  // map sp->decomp to the decompostion's SNODE
  // fill in the  SNODE and store it
  //--------------------------------------------------
  if(sp_decomp(sp)  == NULL)
  {
    sp->decomp = d_entry->sp(0, dist_index);
    map_set_to_sp(array_entry->sp(align_index, dist_index), array_entry,
		  d_entry, align_index, dist_index);  
  }
  
  //----------------------------------------------------------
  // map the align, distrib, decomp id numbers and context
  // this information is used by the interface to map back to
  // the location where the decompositions are defined
  //----------------------------------------------------------
  sp_put_align_id(sp,array_entry->d->align_info[align_index]->id_number);
  sp_put_distrib_id(sp,d_entry->d->distrib_info[dist_index]->id_number);
  sp_put_decomp_id(sp,d_entry->d->decomp_id_number);

#if 0
  sp_put_context(sp, d_entry->d->decomp_context);
#endif

  //-----------------------------------------------------------
  // map the overlap information
  // if the access information has not been added, put it in
  //-----------------------------------------------------------
  if (!sp_access_done(sp))
  {
    map_overlap_info(sp, overlap_info);
  }

  return(sp);
}

//-----------------------------------------------------------
// map the overlap information to the min and max access entries
// int the SNODE
//-----------------------------------------------------------
static void map_overlap_info(SNODE *sp, OverlapList *overlap_info)
{
 int max_access, min_access;
 overlap_ent *entry;
 sp_access(sp, true);
 for(int i=0;i<sp->numdim;++i)
 {
   switch (sp_is_part(sp, i))
    {
     case FD_DIST_BLOCK:
     entry = overlap_info->get_entry(sp->id);
     if(entry != 0)
     {
     if(entry->upper[i] != 0)
     {
     max_access = sp_max_access(sp, i) + entry->upper[i];
     sp_put_max_access(sp, i, max_access);
     }

     if(entry->lower[i] != 0)
     {
     min_access = sp_min_access(sp, i) + entry->lower[i];
     sp_put_min_access(sp, i,  min_access);
     }
     break;
    }
     default:
     break;
    }
 }
}
