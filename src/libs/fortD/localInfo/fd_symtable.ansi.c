/* $Id: fd_symtable.ansi.c,v 1.9 1997/03/11 14:28:43 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <libs/fortD/localInfo/fd_symtable.h>
#include <libs/fortD/misc/FortDEnum.h>
/*******************************************************************/
/*  look for a name in the SNODE hashtable.
   return a pointer to the ident if found.
   if not found and the flag is true, then
   add the name to the table. If the space
   for the new SNODE cannot be allocated then a fatal error occurs
   modified by JDO 04 June 1991
   added search_sp so previous hash table entries are not lost
*******************************************************************/

SNODE *findadd(char *id, int flag, int level, SNODE *ihash[])
{
  SNODE *sp, *search_sp;
  char *cp;
  int hash;

  hash = 0;
  cp = id;

  while (*cp != '\0')
    hash += *cp++;

  search_sp = sp = ihash[hash &= HMASK];

  /* only lookup option */
  while (search_sp)
  {
    if (!strcmp(search_sp->id, id))
    {
      if (flag)    /* if i need to add a node to my symbol table */
      {
        if (search_sp->scopelev == level)       /* return null if id is decl */
          return (NULL);        /* in the same scope level   */
      }
      else if (!flag)
        return (search_sp);
    }
    search_sp = search_sp->next_el;     /* go through the entire linked list */
  }

  /* add a node to the symboltable */
  if (flag)
  {
    sp = (SNODE *) get_mem(sizeof(SNODE) + strlen(id) + 1, "dc");
    sp->next_el = ihash[hash];
    ihash[hash] = sp;
    sp->scopelev = level;
    sp->is_referenced = false;
    (void) strcpy(sp->id, id);
  }

  return (sp);
}

/*********************************************************************/
/* initialize the snode structure for variable given # of dimensions */
/*********************************************************************/
void
init_var_snode(SNODE *sp, int numdim, enum FORM type)
{
  int i;
  int size_a, size_tp, size_d;

  sp->fform = type;
  sp->numdim = numdim;
  sp->num_arrays = 0;
  sp->decomp = NULL;
  sp->scopelev = 0;
  sp->perfect_align = false;
  sp->done_assignment = false;

  /* if scalar */

  if (!numdim)
  {
    sp->fform = INTTYPE;
    sp->value = 0;
    return;
  }

  /* if array, allocate & initialize the typedescript structure */

  sp->fform1 = ARRAYTYPE;
  
  size_a = sizeof(SUBSCR_INFO);
  size_tp = sizeof(TYPEDESCRIPT);
  size_d = sizeof(DIST_INFO);

  for (i = 0; i < numdim; i++)
  {
    sp->align[i] = (SUBSCR_INFO *) get_mem(size_a, "fd_symtab");
    sp->idtype[i] = (TYPEDESCRIPT *) get_mem(size_tp, "fd_symtab");
    sp->distinfo[i] = (DIST_INFO *) get_mem(size_d, "fd_symtab");
    bzero(sp->align[i], size_a);
    bzero(sp->idtype[i], size_tp);
    bzero(sp->distinfo[i], size_d);
  }
}

/**************************************************************/
int
sp_numdim(SNODE *sp)
{
 return(sp->numdim);
}
/**************************************************************/
/* this returns the upper dimension of the decomposition the  */
/* array sp is mapped onto                                    */
/**************************************************************/
int
decomp_get_upper(SNODE *sp, int dim)
{
  return (sp->decomp->idtype[dim]->up.val);
}


/**************************************************************/
/* this returns the lower dimension of the decomposition the  */
/* array sp is mapped onto                                    */
/**************************************************************/
int
decomp_get_lower(SNODE *sp, int dim)
{
  return (sp->decomp->idtype[dim]->lo.val);
}

/**************************************************************/
SUBSCR_INFO *
get_align_info(SNODE *sp, int dim)
{
  return sp->align[dim];
}

/***************************************************************/
int sp_get_align_index(SNODE *sp, int dim)
{
 return get_align_info(sp,dim)->index;
}

/************************************************************/
void sp_put_align_index(SNODE *sp, int dim, int index)
{
 sp->align[dim]->index = index;
}

/************************************************************/
DIST_INFO* 
sp_get_dist_info(SNODE *sp, int dim)
{
  if (!sp)
    die_with_message("syntax error %s not declared \n", sp->id);

  return (sp->distinfo[dim]);
}

/**************************************************************/
/* return type of partition given the symbol table node       */
/**************************************************************/
Dist_type
sp_is_part(SNODE *sp, int dim)
{
  return sp->distinfo[dim]->distr_type;
}

/************************************************************/
void sp_put_dist_type(SNODE *sp, int dim, Dist_type distrib_type)
{

 if (!sp)
  die_with_message("NULL pointer passed to sp_put_dist_type \n");

  sp->distinfo[dim]->distr_type = distrib_type;
}

/**************************************************************/
void sp_put_block_size1(SNODE *sp, int dim, int blocksize)
{
 (sp_get_dist_info(sp,dim))->blocksize1 = blocksize;
}


/**************************************************************/
void sp_put_block_size2(SNODE *sp, int dim, int blocksize)
{
 (sp_get_dist_info(sp,dim))->blocksize2 = blocksize;
}


/**************************************************************/
int
sp_block_size1(SNODE *sp, int dim)
{
  return (sp_get_dist_info(sp, dim))->blocksize1;
}

/**************************************************************/
int sp_bksize(SNODE *sp, int dim)
{
  return sp_get_dist_info(sp, dim)->bksize;
}

/**************************************************************/
void sp_put_bksize(SNODE *sp, int dim, int bksize)
{
 sp_get_dist_info(sp, dim)->bksize = bksize;
}

/**************************************************************/
int
sp_num_blocks(SNODE *sp, int dim)
{
  return ((sp_get_dist_info(sp, dim))->size);
}

/**************************************************************/
void 
sp_put_num_blocks(SNODE *sp, int dim, int size)
{
 sp_get_dist_info(sp,dim)->size = size;
}

/**************************************************************/
int
sp_block_size2(SNODE *sp, int dim)
{
  return ((sp_get_dist_info(sp, dim))->blocksize2);
}

/*******************************************************/
void
sp_put_upper(SNODE *sp, int dim, int val)
{
  if (!sp)
    die_with_message(" syntax error %s not declared \n", sp->id);

  sp->idtype[dim]->up.val = val;
}

/*******************************************************/
void
sp_put_lower(SNODE *sp, int dim, int val)
{
  if (!sp)
    die_with_message(" syntax error %s not declared \n", sp->id);

  sp->idtype[dim]->lo.val = val;
}

/*******************************************************/
Expr* get_expr_lo(SNODE *sp, int dim)
{
 return  &sp->idtype[dim]->lo;
}

/*******************************************************/
Expr* get_expr_up(SNODE *sp, int dim)
{
  return &sp->idtype[dim]->up;
}

/*******************************************************/
int
sp_get_upper(SNODE *sp, int dim)
{
  if (!sp)
    die_with_message(" syntax error %s not declared \n", sp->id);

  return sp->idtype[dim]->up.val;
}



/**********************************************************************/
/* get the lower bound                                                */
/**********************************************************************/
int
sp_get_lower(SNODE *sp, int dim)
{
  if (!sp)
    die_with_message(" syntax error %s not declared \n", sp->id);

  return sp->idtype[dim]->lo.val;
}

/**********************************************************************/
int sp_ddim(SNODE *sp, int dim)
{
 return sp->distinfo[dim]->ddim;
}

/**********************************************************************/
void sp_put_ddim(SNODE *sp, int dim, int ddim)
{
 sp->distinfo[dim]->ddim = ddim;
}

/**********************************************************************/
int sp_max_access(SNODE *sp, int dim)
{
 return sp->distinfo[dim]->max_access;
}

/**********************************************************************/
int* sp_max_access_ptr(SNODE *sp, int dim)
{
 return &(sp->distinfo[dim]->max_access);
}

/**********************************************************************/
int* sp_min_access_ptr(SNODE *sp, int dim)
{
 return &(sp->distinfo[dim]->min_access);
}

/**********************************************************************/
int sp_min_access(SNODE *sp, int dim)
{
 return sp->distinfo[dim]->min_access;
}

/**************************************************************/
void sp_put_min_access(SNODE *sp, int dim, int val)
{ 
 sp_get_dist_info(sp, dim)->min_access = val;
}

/**********************************************************************/
void sp_put_max_access(SNODE *sp, int dim, int val)
{
 sp_get_dist_info(sp, dim)->max_access = val;
}

/**********************************************************************/
void sp_set_align_info(SNODE *sp, int dim, int index, int offset, 
                       int coeff, enum ALIGNTYPE stype)
{
 sp->align[dim]->index = index;
 sp->align[dim]->offset = offset;
 sp->align[dim]->coeff = coeff;
 sp->align[dim]->stype = stype;   

}

/**********************************************************************/
/*  this function returns the type of alignment                       */
/**********************************************************************/
enum ALIGNTYPE
dc_align_type(SNODE *sp, int dim)
{
  int ddim;

  ddim = sp->distinfo[dim]->ddim;
  if (!ddim)
  {
    printf("dc_align_type(): local dimension\n");
    return ALIGN_UNKNOWN;
  }
  return (get_align_info(sp, ddim - 1)->stype);
}

/**********************************************************************/
/*  this function returns the index the dimension is aligned with     */
/**********************************************************************/
int
subs_get_index(SNODE *sp, int dim)
{
  return sp->distinfo[dim]->ddim;
}

/**********************************************************************/
/*  this function returns the alignment offset                        */
/**********************************************************************/
int
get_align_offset(SNODE *sp, int dim)
{
  int ddim;

  ddim = sp->distinfo[dim]->ddim;
  if (!ddim)
  {
    printf("get_align_offset(): local dimension\n");
    return 0;
  }
  return (get_align_info(sp, ddim - 1)->offset);
}

/***********************************************************************/
enum ALIGNTYPE  sp_align_stype(SNODE *sp, int dim)
{
 return (get_align_info(sp, dim))->stype;
}

/***********************************************************************/
void sp_put_align_type(SNODE *sp, int dim, enum ALIGNTYPE type)
{

 sp->align[dim]->stype = type;
}


/***********************************************************************/
int sp_align_index(SNODE *sp, int dim)
{
 return (get_align_info(sp, dim))->index;
}


/***********************************************************************/
int sp_align_offset(SNODE *sp, int dim)
{
 return (get_align_info(sp, dim))->offset;
}

/***********************************************************************/
int sp_align_coeff(SNODE *sp, int dim)
{
 return (get_align_info(sp, dim))->coeff;
}

/***********************************************************************/
void put_distrib_info(Dist_type dtype, DIST_INFO *dist_info)
{
    dist_info->distr_type = dtype;
}

/***********************************************************************/
void put_distrib_info2(Dist_type dtype, int sz, DIST_INFO *dist_info)
{
    dist_info->distr_type = dtype;
    dist_info->size = sz;
}

/***********************************************************************/
void put_distrib_info3(Dist_type dtype, char *nme, int arg_pos, 
                       DIST_INFO *dist_info)
{
   dist_info->distr_type = dtype;
   dist_info->irreg_id = ssave(nme);
   dist_info->arg_position = arg_pos;
}

/***********************************************************************/
SNODE *sp_decomp(SNODE *sp)
{
  return sp->decomp;
}

/**********************************************************************/
/* This function returns the local dimensions of an array aligned to  */
/* a particular decomposition and the distribution is block.          */
/**********************************************************************/
void 
decomp_local_bounds(SNODE *sp, int dim, int *dupper, int *dlower)
{
  int dindex, ld, bk1, bk2, offset;

  dindex = subs_get_index(sp, dim);

/*  if (!dindex)   dimension is local */
    ld = decomp_get_lower(sp, dindex - 1);

  /* compare and see if there is any kind of alignment */

  switch (sp_is_part(sp, dim))
  {
    case FD_DIST_BLOCK:
      bk1 = sp_block_size1(sp, dim);
      bk2 = sp_block_size2(sp, dim);

      /* currently only alignment with offsets are handled */
      switch (dc_align_type(sp, dim))
      {
        /* unknown alignment */
        case ALIGN_UNKNOWN:
          die_with_message(" Unknown alignment expression ");
          break;

        /* perfect alignment */
        case ALIGN_PERFECT:
          if (bk1 == bk2)
          {
            *dupper = ld + bk1 - 1;
            *dlower = ld;
          }
          else
            die_with_message("Differing Blocksizes not handled");
          break;

        /* offset alignment  */
        case ALIGN_OFFSET:
          if (bk1 == bk2)
          {
            offset = -(get_align_offset(sp, dim));

      /* starting and ending position of the decomposition on a processor  */

            *dlower = ld + offset;
            *dupper = *dlower + bk1 - 1;

      /*      startpos = max(l1,decomp_startpos); */
          }

          else  /* bk1 != bk2 */
            die_with_message("Differing Blocksizes not handled");

          break;

        /* linear expression as alignment */
        case ALIGN_COEFF:
          die_with_message(" Alignment Expression not handled");
          break;

        default:
          die_with_message(" Alignment Expression not handled");
          break;
      }

    default:
      break;

  }
}

/***********************************************************************/
void sp_access(SNODE *sp, Boolean done)
{
sp->access = done;
}

/***********************************************************************/
Boolean sp_access_done(SNODE *sp)
{
 return sp->access;
}

/***********************************************************************/
void sp_put_distrib_id(SNODE *sp, int l)
{
 sp->distrib_id_number = l;
}

/***********************************************************************/
void sp_put_align_id(SNODE *sp, int l)
{
 sp->align_id_number = l;
}

/***********************************************************************/
void sp_put_decomp_id(SNODE *sp, int l)
{
  sp->decomp_id_number = l;
}

/***********************************************************************/
int sp_get_align_id (SNODE *sp, int l)
{
  return sp->align_id_number;
}

/***********************************************************************/
int sp_get_distrib_id (SNODE *sp)
{
 return sp->distrib_id_number;
}

/***********************************************************************/
int sp_get_decomp_id (SNODE *sp)
{
 return sp->decomp_id_number;
}

#if 0
/***********************************************************************/
void sp_put_context (SNODE *sp, Context cc)
{
 sp->c = cc;
}

/***********************************************************************/
Context sp_get_context (SNODE *sp)
{
 return sp->c;
}
#endif

