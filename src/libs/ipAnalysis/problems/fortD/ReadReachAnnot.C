/* $Id: ReadReachAnnot.C,v 1.5 1997/03/11 14:35:05 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//--------------------------------------------------------------------
// author : Seema Hiranandani
// contents : routines to read fortran d interprocedural annotations
// date : since May 1992
//--------------------------------------------------------------------
#include <libs/fortD/misc/ReadInterInfo.h>
#include <libs/ipAnalysis/problems/fortD/ReachAnnot.h>
#include <libs/fortD/localInfo/fd_symtable.h>

//-------------------------
// forward declarations

void map_set_to_sp        (SNODE *sp,
                           FortranDHashTableEntry *array_entry, 
                           FortranDHashTableEntry *decomp_entry, 
                           int a_index, int d_index);

static void map_distarrays(SNODE *array_sp, SNODE *decomp_sp);

void map_decomp_to_sp     (FortranDHashTableEntry *decomp_entry,
                           SNODE *sp, int d_index);

void map_decomp(SNODE *sp, int numberOfProcessors);

#if 0

// does not work. never called for this reason. 
// 13 September 1994: even if this was uncommented, map_decomp will not work 
// because the number of parameters it takes have been changed. 
// -- John Mellor-Crummey

//---------------------------------------------------------------------------
// Attach a Fortran D hash table as an annotation to the procedure node.
// It contains the decomposition, align, distribute information for each
// procedure
//---------------------------------------------------------------------------
void FD_Reach_Annot::read(FormattedFile& port)
{
  int entries,     num_params, dim,       distrib_entries;
  int a_numdim,   dist_type, align_count, set_count;
  int a_index, distrib_index, align_index, d_index;
  int i, j, k, type;

  // SNODE *sp,  *array_sp;
  char name[NAME_LENGTH],       decomp_name[NAME_LENGTH];
  char align_name[NAME_LENGTH], formal_name[NAME_LENGTH];
  char temp_name[NAME_LENGTH];
  char begin_set_entry[NAME_LENGTH], end_fd_annot[NAME_LENGTH];
  
  // FortranDInfo *f;
  FortranDHashTableEntry *d1;
  FortranDHashTableEntry *formal_entry, *d_entry;
  FortranDHashTableEntry *array_entry,  *decomp_entry;

  AlignList *align_list =  new AlignList();

//  ReadAnnotation *an = new ReadAnnotation(FD_REACH);
  f = new FortranDInfo();

  port.Read(temp_name, NAME_LENGTH);

//-------------------------------
// read the number of parameters
   port.Read(num_params);

//-------------------------------
// read number of decomp_entries 
   port.Read(entries);

   while(entries-- > 0)
   {
    port.Read(name, NAME_LENGTH);
    port.Read(dim);
    
    d1 = f->AddDecomp(name, DECOMPTYPE, dim);

    for(i=0; i<dim; ++i)
     read_typedescript(port, d1->d->idtype[i]);

//-------------------------------------------------------------------
// read number of distrib entries associated with the decomposition
// followed by the distribution information
//-------------------------------------------------------------------
    port.Read(distrib_entries);
    if (distrib_entries > 1)
     printf("cloning not implemented, multiple distributions not handled \n");
  
   while(distrib_entries-- > 0)
   {
    d_index = d1->d->d_index;
    for (j=0;j<dim;++j)
    {
     port.Read(dist_type);
     if (dist_type == FD_DIST_LOCAL)
   
     {put_distrib_info2((Dist_type)dist_type,0, 
                       &(d1->d->distrib_info[d_index]->distinfo[j]));}

     else if ((dist_type == FD_DIST_BLOCK) || (dist_type == FD_DIST_CYCLIC) ||
              (dist_type == FD_DIST_BLOCK_CYCLIC))
   
     {put_distrib_info((Dist_type) dist_type, 
                      &(d1->d->distrib_info[d_index]->distinfo[j]));}
     }

// map the decomposition entry to an SNODE structure

   map_decomp_to_sp(d1, &d1->d->sp[0][d_index], d_index);
   map_decomp(&d1->d->sp[0][d_index]);
   ++d1->d->d_index;
   }
  }

//-------------------------------------------------------------------
// read the align count, followed by alignment information
//-------------------------------------------------------------------  
   port.Read(align_count);
   while(align_count-- > 0)
   {
   port.Read(decomp_name, NAME_LENGTH);
   port.Read(align_name, NAME_LENGTH);
   port.Read(a_numdim); 
   port.Read(type);   
   array_entry = f->GetEntry(align_name);
   decomp_entry = f->GetEntry(decomp_name);

   if(!array_entry)
      array_entry = f->AddDecomp(align_name, ARRAYTYPE, a_numdim);
      array_entry->d->dec_name = ssave(decomp_name);
      array_entry->type = (FORM) type;

//--------------------------------------------------------
// store the arrays aligned to the decomposition in a list

    decomp_entry->d->name_info->append_entry(align_name);
    a_index =  array_entry->d->a_index;

  for(j=0;j<a_numdim;++j)
  {
   read_typedescript(port, array_entry->d->idtype[j]);
  }
   array_entry->d->align_info[a_index]->Read(port, a_numdim);
   ++array_entry->d->a_index;
 }

//-------------------------------------------------------------------
//  read the set information for the parameters
//-------------------------------------------------------------------
  port.Read(begin_set_entry, NAME_LENGTH);
  for(k = 0; k < FD_MAX_PARAM; ++k)
   {
   port.Read(set_count);
    if (set_count != 0)
    {
     port.Read(formal_name, NAME_LENGTH);
    }
   
    for(i=0; i<set_count;++i)
    {
     port.Read(decomp_name);
     port.Read(align_index);
     port.Read(distrib_index);
     formal_entry = f->GetEntry(formal_name);
     d_entry = f->GetEntry(decomp_name);
     if (formal_entry != 0)
     {
      int dist_index = formal_entry->d->dist_index;
      formal_entry->d->distrib_index[dist_index] = distrib_index;
      formal_entry->d->dist_index++;

//-----------------------------------
// map the formal entry to an SNODE

      map_set_to_sp(&formal_entry->d->sp[align_index][dist_index],
                    formal_entry, d_entry, align_index, distrib_index);
     }
    }
  }
   port.Read(end_fd_annot, NAME_LENGTH);

//-----------------------------------
// append the annotation to the node

//   n->add_annotation(FD_REACH, an);
}
#endif


//-------------------------------------------------------------------------
// map a decomposition entry stored in FortranDInfo to an SNODE structure
//-------------------------------------------------------------------------
void 
map_decomp_to_sp(FortranDHashTableEntry *decomp_entry,SNODE *sp, int d_index)
{
 int i;

 strcpy(sp->id, decomp_entry->name());
 sp->fform       =  decomp_entry->fform;
 sp->numdim      =  decomp_entry->numdim;
 sp->idtype[0]   =  &decomp_entry->d->idtype[0];
 sp->distinfo[0] =  &decomp_entry->d->distrib_info[d_index]->distinfo[0];

 for(i=1; i < decomp_entry->numdim; ++i)
 {
  sp->idtype[i]   = &decomp_entry->d->idtype[i];
  sp->distinfo[i] = &decomp_entry->d->distrib_info[d_index]->distinfo[i];
 }
}
//-------------------------------------------------------------------------
// add additional information to sp to reflect inter dimensional alignment,
// blocksizes, and array bounds which are computed from the information
// stored during the interprocedural analysis phase
//-------------------------------------------------------------------------
void
map_set_to_sp(SNODE *sp,
                   FortranDHashTableEntry *array_entry, 
                   FortranDHashTableEntry *decomp_entry, 
                   int a_index, int distrib_index)
{
 int i = 0;
 AlignEntry *entry;

 strcpy(sp->id, array_entry->name());

 sp->fform          = array_entry->type;
 sp->fform1         = array_entry->fform;
 sp->numdim         = array_entry->numdim;

 for(entry = array_entry->d->align_info[a_index]->first_entry();
     entry != NULL; entry=array_entry->d->align_info[a_index]->next_entry())
 {
 sp->align[i]       = entry->sub_info();
 ++i;
 }

 sp->idtype[0]      = &array_entry->d->idtype[0];


//-----------------------------------------------------------------
// compute the distribution information from the align information
//-----------------------------------------------------------------
 sp->distinfo[0]   = (DIST_INFO*) get_mem(sizeof(DIST_INFO), "read_inter");
 sp->perfect_align  = array_entry->d->align_info[a_index]->perfect_align;

 for(i=1; i < array_entry->numdim; ++i)
 {

  sp->idtype[i]     = &array_entry->d->idtype[i];
  sp->distinfo[i] =  (DIST_INFO*) get_mem(sizeof(DIST_INFO), "read_inter");
 }
 sp->decomp         = &decomp_entry->d->sp[0][distrib_index];      
 map_distarrays(sp, sp->decomp);
}


//--------------------------------------------------------------------
// this routine calculates the processor layout for each          
// distributed array based on the decomposition layout            
// modified by JDO 04 June 1991                                   
// handle array replication and alignment of arrays with transposed
// decompositions, e.g., ALIGN X(i,j) with D(j,i)                
//--------------------------------------------------------------------
static void
map_distarrays(SNODE *array_sp, SNODE *decomp_sp)
{
  // DIST_INFO *decomp_dist, *array_dist;
  int i, j, ddim, arraysize, size;

  /*--------------------------------------------*/
  /* find distribution for each array dimension */
  if (array_sp->perfect_align)
  {
   for (i= 0; i < array_sp->numdim; ++i)
   {
    if (get_align_info(array_sp,i) == NULL)
     array_sp->align[i] =
            (SUBSCR_INFO*) get_mem(sizeof(SUBSCR_INFO), "read_inter");
    sp_put_align_index(array_sp, i, i+1);
    sp_put_align_type(array_sp, i, ALIGN_PERFECT);
   }
  }

  for (i = 0; i < array_sp->numdim; i++)
  {

    /*--------------------------------------------------*/
    /* default access is the entire local array section */
    sp_put_min_access(array_sp, i,sp_get_lower(array_sp, i));
    sp_put_max_access(array_sp, i,sp_get_upper(array_sp, i));

    /*----------------------------------------------------*/
    /* find decomp dim that the array dim is aligned with */

    for (j = 0, ddim = 0; j < decomp_sp->numdim; j++)
    {
      if (sp_get_align_index(array_sp,j) == i+1)
      {
        if (!ddim)
          ddim = j+1;
        else
          die_with_message("Diagonal alignment not yet supported");
      }
    }


    if (!ddim)  /* no decomp dimension found */
    {
      sp_put_dist_type(array_sp, i, FD_DIST_LOCAL);
    }
    else
    {
      /*---------------------------------------------------------*/
      /* copy distribution from appropriate dimension of decomp  */

      sp_put_ddim(array_sp, i, ddim);
      sp_put_dist_type(array_sp,i, sp_is_part(decomp_sp, ddim-1));
      sp_put_num_blocks(array_sp, i, sp_num_blocks(decomp_sp,ddim-1));
      sp_put_bksize(array_sp, i, sp_bksize(decomp_sp, ddim-1));
      arraysize =  sp_get_upper(array_sp,i) - 
                   sp_get_lower(array_sp, i) + 1;

      size = sp_num_blocks(array_sp, i);

      /*--------------------------------------------*/
      /* calculate local array section sizes */
      /* kernel code taken from store_size() */

      if (size)
      {
        if (!(arraysize % size))
        {
          sp_put_block_size1(array_sp, i,  arraysize / size);
          sp_put_block_size2(array_sp, i,  sp_block_size1(array_sp, i));
          sp_put_block_size1(decomp_sp, ddim-1, arraysize/size);
          sp_put_block_size2(decomp_sp, ddim-1, sp_block_size1(array_sp,i));
        }
        else
        {
          sp_put_block_size1(array_sp, i,  arraysize / size + 1);
          sp_put_block_size2(array_sp, i,  arraysize - 
                             sp_block_size1(array_sp, i) * (size - 1));
 
          sp_put_block_size1(decomp_sp, ddim-1, arraysize/size + 1);
          sp_put_block_size2(decomp_sp, i,  arraysize - 
                             sp_block_size1(array_sp, i) * (size - 1));
        }

        /* Reduce the access range for this dimension if distributed */
 
        switch (sp_is_part(array_sp,i))
        {
          case FD_DIST_BLOCK:
            decomp_local_bounds(array_sp, i, sp_max_access_ptr(array_sp, i),
            sp_min_access_ptr(array_sp, i));
            break;

          case FD_DIST_CYCLIC:
            sp_put_min_access(array_sp, i, 1);
            sp_put_max_access(array_sp, i, sp_block_size1(array_sp,i));
            break;
          case FD_DIST_LOCAL:
            sp_put_block_size1(array_sp, i, 0);
            sp_put_block_size2(array_sp, i, 0);
            sp_put_min_access(array_sp,i,sp_get_lower(array_sp, i));
            sp_put_max_access(array_sp,i, sp_get_upper(array_sp, i));
        }
      }
    }
  }
}

/********************************************************************/
void
map_decomp(SNODE *sp, int nproc)
{
  int distdim, i;
  // TYPEDESCRIPT *typeform;

  distdim = 0;                           /* # of distributed dims      */

  /* count up # of unallocated distributed dimensions        */
  /* also check # of processors remaining after distribution */

  for (i = 0; i < sp->numdim; i++)
  {
    if (sp_is_part(sp, i) != FD_DIST_LOCAL)
    {
      if (sp_num_blocks(sp,i))
      {
        nproc /= sp_num_blocks(sp,i);
        if (!nproc)
          die_with_message("not enough processors for DISTRIBUTE");
      }
      else
        distdim++;
    }
  }

  /*-----------------------------------------------------*/
  /* decide how to allocate remaining processors         */
  /* assume just 2D processor grid for current prototype */

  // 11/9/93 RvH: Want to allow n$proc = 1
  //if (!distdim || nproc == 1)   /* no more processors to allocate */
  if (!distdim)   /* no more processors to allocate */
    return;                     /* return immediately             */

  else if (distdim == 2)
  {
    if (nproc < 4)              /* only enough for 1 dim, allocate the
                                 * remaining */
      distdim = 1;              /* processors to the 1st distributed dimension   */

    else                        /* else divide by half (not best, but use for
                                 * now) */
      nproc /= 2;
  }
  else if (distdim > 2)
  {
    die_with_message("Too many unallocated distributed dimensions");
  }

  /*-------------------------------*/
  /* allocate remaining processors */

  for (i = 0; i < sp->numdim; i++)
  {
    /* find unallocated distributed dimension */

    if ((sp_is_part(sp,i) != FD_DIST_LOCAL) &&
       !(sp_num_blocks(sp,i)))
       sp_put_num_blocks(sp, i, nproc);
  }
}
