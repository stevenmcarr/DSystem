/* $Id: boundary_msgs.ansi.c,v 1.13 1997/03/11 14:28:13 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/



/**********************************************************************/
/* author: Seema Hiranandani                                          */
/* This routine is the driver for the boundary condition for          */
/* communications                                                     */
/* this function performs communication for loop structures that      */
/* have constant original bounds and symbolic modified bounds         */
/* the following needs to be done:                                    */
/* set up the guards correctly                                        */
/* the bounds reduction is already done                               */
/*  solve column and row partitions, worry about block-block          */
/* after that                                                         */
/**********************************************************************/

#include <libs/fortD/codeGen/private_dc.h>

/*-----------------------------extern definitions----------------------*/

EXTERN(void, print_rsds,(Dist_Globals *dh, AST_INDEX loop, Rsd_set *rset, 
                         Iter_type msg_type));

/*-----------------------------global definitions----------------------*/

void dc_boundary_msgs(Dist_Globals *dh, AST_INDEX loop, Rsd_set *rset);
Boolean dc_local_access(Dist_Globals *dh, SNODE *sp, Rsd_section*rsd);

/*-----------------------------local definitions-----------------------*/

STATIC(Boolean, one_dim_distr,(Dist_Globals *dh, SNODE *sp));
STATIC(Boolean, check_comm_middle,(Dist_Globals *dh, SNODE *sp, 
                                   Rsd_section *mid_rsd, Rsd_section *bound_rsd));
STATIC(Boolean, dc_check_comm_block,(Dist_Globals *dh, SNODE *sp, int dim,
                                     Rsd_section *rsd));

STATIC (Rsd_section*, rsd_build_section_all, (Dist_Globals *dh, Rsd_set *rset,
                                              Loop_list *loop_set));

/********************/
/* global functions */
/********************/

/*------------------------------------------------------------------------

  dc_local_access()    Returns true if only nonlocal accesses in RSD

*/

Boolean
dc_local_access(Dist_Globals *dh, SNODE *sp, Rsd_section *rsd)
{
  int i;

  for (i = 0; i < sp->numdim; i++)
  {
    if (dc_check_comm_block(dh, sp, i, rsd))
      return false;
  }

  return true;
}

/*******************************************************************/
/* currently it will handle only one distributed dimension         */
/* this assumes that it is one dimensional and that its passed the */
/* is_simple_comm test                                             */
/*******************************************************************/
void
dc_boundary_msgs(Dist_Globals *dh, AST_INDEX loop, Rsd_set *rset)
{
  Rsd_section *r_section, *r_section1, *mid_rsd;
  Subs_list *subscript;
  int i, dim;
  SNODE *sp;
  Boolean iter_pre_build, iter_post_build, iter_pre_post_build;
  Boolean is_comm, is_comm1;
  Iter_set *iset;
  Loop_list *pre_set, *post_set;

  /* get the list of rsd structures             */
  /* first check for those whose loop_level = 1 */
  /* check if the  iter set type                */
  /* handle Iter_pre                            */
  /* Iter_post                           */
  /* Iter_pre_post                       */

  is_comm = is_comm1 = false;

  subscript = (Subs_list *) get_info(dh->ped, rset->subs[0], type_ref);
  if (subscript == (Subs_list *) NO_REF)
    die_with_message("rsd_build_rhs(): Missing ref info");

  iset = rset->iterset;
  sp = rset->sp;

  /* check that only one dimension is distributed and it  */
  /* has the same middle iterset as the right hand side   */

  if (NOT(one_dim_distr(dh, sp)))
  {
    printf("dc_boundary_msgs(): Multi-dim distributions not handled\n");
    return;
  }

/*   get the rsd for the pre set and check if there is any communication
     If there is then check to see if it has the same overlap as the
     set. If it does then generate communications as though there were
     no overlaps.
*/
  mid_rsd = rset->rs;

  for (i = 0; i < MAXLOOP; ++i)
  {
    iter_pre_build = iter_post_build = iter_pre_post_build = false;
    is_comm = is_comm1 = false;

    switch (iset->type[i])
    {
      case Iter_pre:    /* pre set and set  used */
        pre_set = &(iset->pre_set);
        r_section = rsd_build_section_all(dh, rset, pre_set);
        rset->rs_pre = r_section;
        iter_pre_build = true;
        break;

      case Iter_post:   /* post set and set used */
        post_set = &(iset->post_set);
        r_section1 = rsd_build_section_all(dh, rset, post_set);
        rset->rs_post = r_section1;
        iter_post_build = true;
        break;

      case Iter_pre_post:       /* both pre and post set used */
        pre_set = &(iset->pre_set);
        r_section = rsd_build_section_all(dh, rset, pre_set);
        rset->rs_pre = r_section;

        post_set = &(iset->post_set);
        r_section1 = rsd_build_section_all(dh, rset, post_set);
        rset->rs_post = r_section1;
        iter_pre_post_build = true;
        break;

      case Iter_all:    /* boundary condition, pre, post and mid set used */
        pre_set = &(iset->pre_set);
        r_section = rsd_build_section_all(dh, rset, pre_set);
        rset->rs_pre = r_section;

        post_set = &(iset->post_set);
        r_section1 = rsd_build_section_all(dh, rset, post_set);
        iter_pre_post_build = true;
        rset->rs_post = r_section1;
        break;

    }

    /* the rsds, pre_set and post_set are built   */

    if (iter_post_build || iter_pre_build || iter_pre_post_build)
    {
      is_comm = is_comm1 = false;
      for (dim = 0; dim < sp->numdim; ++dim)
      {
        if (iter_pre_build && !is_comm)
          is_comm = dc_check_comm_block(dh, sp, dim, r_section);
        else if (iter_post_build && !is_comm1)
          is_comm1 = dc_check_comm_block(dh, sp, dim, r_section1);
        else if (iter_pre_post_build)
        {
          if (!is_comm)
            is_comm = dc_check_comm_block(dh, sp, dim, r_section);
          if (!is_comm1)
            is_comm1 = dc_check_comm_block(dh, sp, dim, r_section1);
        }
      }
    }

    /* if communication check if its the same as the middle section */

    if (iter_pre_build && is_comm)
    {
      if (check_comm_middle(dh, sp, mid_rsd, r_section))
        print_rsds(dh, loop, rset, Iter_pre);

      /* communication is the same */

      else
      {
        print_rsds(dh, loop, rset, Iter_mid_only);
        print_rsds(dh, loop, rset, Iter_pre_only);
      }

      /* generate the communications for the middle procs and the boundary
       * processors */

    }

    if (iter_post_build && is_comm1)
    {
      if (check_comm_middle(dh, sp, mid_rsd, r_section1))
        print_rsds(dh, loop, rset, Iter_post);
      else
      {
        print_rsds(dh, loop, rset, Iter_mid_only);
        print_rsds(dh, loop, rset, Iter_post_only);
      }
    }

    if (iter_pre_post_build)
    {
      if (is_comm && is_comm1)
      {            /* both pre and post communication */

        /* case1:  check if both pre and post comm are the same    */

        if (check_comm_middle(dh, sp, mid_rsd, r_section1) &&
            (check_comm_middle(dh, sp, mid_rsd, r_section)))
          print_rsds(dh, loop, rset, Iter_mid_only);

        /* case2: if not, check if one of them can be combined      */

        else if (check_comm_middle(dh, sp, mid_rsd, r_section1))
        {
          print_rsds(dh, loop, rset, Iter_post);
          print_rsds(dh, loop, rset, Iter_pre_only);
        }
        else if (check_comm_middle(dh, sp, mid_rsd, r_section))
        {
          print_rsds(dh, loop, rset, Iter_pre);
          print_rsds(dh, loop, rset, Iter_post_only);
        }
        else
        {
          print_rsds(dh, loop, rset, Iter_mid_only);
          print_rsds(dh, loop, rset, Iter_pre_only);
          print_rsds(dh, loop, rset, Iter_post_only);
        }
      }

      else
      {            /* if is_comm or is_comm1 */
        if (is_comm)
        {
          if (check_comm_middle(dh, sp, mid_rsd, r_section))
            print_rsds(dh, loop, rset, Iter_pre);
          else
          {
            print_rsds(dh, loop, rset, Iter_mid_only);
            print_rsds(dh, loop, rset, Iter_pre_only);
          }
        }
        if (is_comm1)
        {
          if (check_comm_middle(dh, sp, mid_rsd, r_section1))
            print_rsds(dh, loop, rset, Iter_post);
          else
          {
            print_rsds(dh, loop, rset, Iter_mid_only);
            print_rsds(dh, loop, rset, Iter_post_only);
          }
        }
      }
    }

    /* if no communication for pre and post sections check if comm exists for
     * the middle section  */

    if (!is_comm && !is_comm1)
    {
      for (dim = 0; dim < sp->numdim; ++dim)
      {
        if (dc_check_comm_block(dh, sp, dim, mid_rsd))
        {
          if (iter_post_build && !is_comm1)
            print_rsds(dh, loop, rset, Iter_mid_only);
          if (iter_pre_build && !is_comm)
            print_rsds(dh, loop, rset, Iter_mid_only);
          if (iter_pre_post_build)
          {
            if (!is_comm && !is_comm1)
              print_rsds(dh, loop, rset, Iter_mid_only);
          }
        }
      }
    }
  }
}


/*******************/
/* local functions */
/*******************/

/********************************************************/
/* counts the number of dimensions that are distributed */
/********************************************************/
static Boolean
one_dim_distr(Dist_Globals *dh, SNODE *sp)
{
  int i, num_dist_dim;

  num_dist_dim = 0;

  for (i = 0; i < sp->numdim; ++i)
    if (sp_is_part(sp, i) != FD_DIST_LOCAL)
      ++num_dist_dim;
  if (num_dist_dim > 1)
    return (false);
  else
    return (true);
}


/******************************************************************/
/* check if the communication if the nonlocal section is the same */
/******************************************************************/
static Boolean
check_comm_middle(Dist_Globals *dh, SNODE *sp, Rsd_section *mid_rsd, 
                  Rsd_section *bound_rsd)
{

  int i, u1, u2, l1, l2, nonlocal_u1, nonlocal_u2, nonlocal_l1, nonlocal_l2,
   blocksize1;
  enum RANGETYPE atype;

  for (i = 0; i < sp->numdim; ++i)
  {
    u1 = dc_rsd_upper(mid_rsd, i);
    u2 = dc_rsd_upper(bound_rsd, i);
    l1 = dc_rsd_lower(mid_rsd, i);
    l2 = dc_rsd_lower(bound_rsd, i);

    if (sp_is_part(sp, i) == FD_DIST_BLOCK)
    {
      blocksize1 = sp_block_size1(sp, i);
      atype = dc_range_type(l1, u1);

      if (atype != dc_range_type(l2, u2))
        return (false);

      switch (atype)
      {
        case NEG_NEG:     /* negative */

          nonlocal_u1 = u1;
          nonlocal_u2 = u2;
          nonlocal_l1 = l1;
          nonlocal_l2 = l2;
          break;

        case POS_POS:     /* positive */
          nonlocal_u1 = u1 - blocksize1;
          nonlocal_u2 = u2 - blocksize1;
          if (l1 > blocksize1)
            nonlocal_l1 = l1;
          else
            nonlocal_l1 = 1;

          if (l2 > blocksize1)
            nonlocal_l2 = l2;
          else
            nonlocal_l2 = 1;
          break;

        case NEG_POS:     /* negative and positive */
          if (u1 > blocksize1)
            nonlocal_u1 = u1;
          else
            nonlocal_u1 = 1;

          if (u2 > blocksize1)
            nonlocal_u2 = u2;
          else
            nonlocal_u2 = 1;

          nonlocal_l1 = l1;
          nonlocal_l2 = l2;
          break;
      }

      if (nonlocal_u1 != nonlocal_u2 || nonlocal_l1 != nonlocal_l2)
      {
        return (false);
      }
    }
  }
  return (true);
}



/*--------------------------------------------------------------------

  dc_check_comm_block()   

  Determine whether communication is caused by an array dimension
  that is block-distributed.

*/
  
static Boolean 
dc_check_comm_block(Dist_Globals *dh, SNODE *sp, int dim, Rsd_section *rsd)
{
  int u1, l1, u2, l2, ld, idx;
  int blocksize1, blocksize2, offset;
  int decomp_startpos, decomp_endpos;


  /* compare and see if there is any need  for communication */

  switch (sp_is_part(sp, dim))
  {
    case FD_DIST_BLOCK:
  /* get the array dimensions, the decomposition dimensions, the
     structure of the rsds
  */
      l1 = sp_get_lower(sp, dim);
      u2 = dc_rsd_upper(rsd, dim);
      l2 = dc_rsd_lower(rsd, dim);
      idx = subs_get_index(sp, dim);
      ld = decomp_get_lower(sp, idx - 1);

      blocksize1 = sp_block_size1(sp, dim);
      blocksize2 = sp_block_size2(sp, dim);

      /* currently only alignment with offsets are handled */
      switch (dc_align_type(sp, dim))
      {
        /* unknown alignment */
        case ALIGN_UNKNOWN:
          die_with_message(" Unknown alignment expression ");
          break;

        /* perfect alignment */
        case ALIGN_PERFECT:
          if (blocksize1 == blocksize2)
          {
            if (u2 <= (blocksize1 + ld - 1) && u2 >= ld && l2 >= ld &&
                l2 <= (blocksize1 + ld - 1))
              return (false);
            u1 = l1 + blocksize1 - 1;
          }
          else
            die_with_message("Differing Blocksizes not handled");

          return BOOL((u1 != u2) || (l1 != l2));

        /* offset alignment  */
        case ALIGN_OFFSET:
          if (blocksize1 == blocksize2)
          {
            offset = -(get_align_offset(sp, dim));

            /* starting and ending position of the decomposition on
               a processor  */

            decomp_startpos = ld + offset;
            decomp_endpos = decomp_startpos + blocksize1 - 1;

            if ((decomp_startpos == l2) && (decomp_endpos == u2))
              return (false);
            else
              return (true);

            /* startpos = max(l1,decomp_startpos); */
          }

          else  /* blocksize1 != blocksize2 */
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
      return (false);
  }
}

/*--------------------------------------------------------------------------*/
/* if mid set has been coalesced then pre and post sets must also be        */
/* coalesced. This causes fewer messages to be sent                         */
/*--------------------------------------------------------------------------*/
static
Rsd_section* rsd_build_section_all(Dist_Globals *dh, Rsd_set *rset, Loop_list *loop_set)
{
 int count = 0;
 Subs_list *subscript;
 Rsd_section *r_section[10], *coal_section;
 coal_section = NULL;

/* r_section = (Rsd_section**)dc_get_mem(dh,sizeof(Rsd_section*)*rset->num_subs); */

 while (subscript = 
   ((Subs_list *) get_info(dh->ped, rset->subs[count],type_ref))) 
 {
   r_section[count] = dt_alloc_rsd(PED_DT_INFO(dh->ped));   
   rsd_build_section(r_section[count], subscript, loop_set, 1);
   if (count > 0) {
     coal_section = dt_alloc_rsd(PED_DT_INFO(dh->ped));   
     rsd_merge(coal_section, r_section[count-1], r_section[count]);
     r_section[count] = coal_section;
   	}
   ++count;
  
  if (count == 10) {
   r_section[0] = r_section[10];
   count = 1;
   }
  }
  if( count == 1)
    return(r_section[count-1]);
  else
    return(coal_section);
 }
