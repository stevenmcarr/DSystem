/* $Id: rsd_sec.ansi.c,v 1.29 1997/03/11 14:28:19 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/



/***********************************************************************/
/* author : Seema Hiranandani                                          */
/* the routines in this file build the rsd sets for all the rhs        */
/* occuring in  assignment statements assuming the iteration sets for  */
/* the statements have been generated                                  */
/* this information is stored in the side array                        */
/***********************************************************************/

#include <libs/fortD/codeGen/private_dc.h>
#include <libs/frontEnd/ast/groups.h>
#include <assert.h>

struct dc_build_param;

/*------------------------- extern definitions ------------------------*/

EXTERN(Boolean, dc_local_access,(Dist_Globals *dh, SNODE *sp, Rsd_section*rsd));     
                                        /* boundary_msgs.c */ 
EXTERN(Boolean,   needs_reg_comm,     (Generic irr, AST_INDEX node));

EXTERN(void, SDDF_StoreDepEdgeForRef, (AST_INDEX rsideName,
				       DG_Edge* edge));

/*------------------------- global definitions ------------------------*/

void dc_comm(Dist_Globals *dh);

/*------------------------- local definitions -------------------------*/

STATIC(Rsd_set, *dc_alloc_rsd_set,(Dist_Globals *dh));
STATIC(Boolean, coalesce_rsds,(Dist_Globals *dh, Rsd_set *rset, Rsd_section *rsd_sect,
                               Rsd_section *carried_rsd, Iter_set *iset));
STATIC(void, aggreg_msgs,(Dist_Globals *dh, Rsd_set_info *rset_info));
STATIC(Rsd_set, *aggreg_rsds,());
STATIC(Boolean, can_aggreg_rsds,());
STATIC(Boolean, perfect_align,(SNODE *sp1, SNODE *sp2));

STATIC(int, rsd_get_level,(AST_INDEX id, Dist_Globals *dh, Boolean *indep));
STATIC(int, rsd_build_rhs,(AST_INDEX rside, struct dc_build_param *param));
STATIC(int, rsd_build_stmt,(AST_INDEX stmt, int level, Dist_Globals *dh));
STATIC(int, rsd_save_loop,(AST_INDEX stmt, int level, Dist_Globals *dh));
STATIC(Rsd_set_info, *rsd_aggregate,(int level, Dist_Globals *dh));
STATIC(Comm_type, rsd_comm_type,(SNODE *lhs_sp, SNODE *rhs_sp, Subs_list *lhs_subs,
                                 Subs_list *rhs_subs, Iter_set *iset));
STATIC(Comm_type, rsd_comm_meet,());
STATIC(Rsd_set, *rsd_find_set,(Rsd_set_info *rset_info, SNODE *sp));
STATIC(void, dc_calc_overlap,(Dist_Globals *dh, SNODE *sp, Rsd_section *rsd_sect));
STATIC(void, dc_calc_buffer,(Dist_Globals *dh, SNODE *sp, Rsd_section *r_section));
STATIC(Boolean, loop_part,(AST_INDEX loop, Dist_Globals *dh));
STATIC(void, mark_rsd_loop,(Dist_Globals *dh, AST_INDEX stmt, Rsd_set_info *rset_info,
                            FortD_LI *fli));
STATIC(Boolean, dc_rsd_contiguous,(Rsd_section *rsd1, Rsd_section *rsd2, int dim));

/*------------------------- struct definitions -------------------------*/

struct dc_build_param   /* for use by dc_build_rsd_section() */
{
  Dist_Globals *dh;
  Iter_set *iset;
  S_group *sgroup;
};


/********************/
/* global functions */
/********************/

/*---------------------------------------------------------------------

    dc_comm()      Builds all RSDs representing needed communication

    Communication analysis phase of the Fortran D compiler

*/

void
dc_comm(Dist_Globals *dh)
{
  int i, j;
  AST_INDEX loop;
  Rsd_set_info *rset_info;

  /* build RSDs for statements in loops */

  bzero(dh->rsd_loop_list, sizeof(Rsd_set_info) * MAXLOOP);
  for (i = 0; i < dh->numdoloops; i++)
  {
    loop = dh->doloops[i];
    if (loop_level(loop) == 1)
      walk_statements(loop, LEVEL1, (WK_STMT_CLBACK)rsd_build_stmt, 
                      (WK_STMT_CLBACK)rsd_save_loop, (Generic)dh);
  }

  /* build RSDs for non-loop statements */

  bzero(dh->rsd_loop_list, sizeof(Rsd_set_info) * MAXLOOP);
  for (i = 0; i < dh->sgroup.group_num; i++)
  {
    for (j = 0; j < dh->sgroup.groups[i]->size; j++)
      (void) rsd_build_stmt(dh->sgroup.groups[i]->stmt[j], 1, dh);

    /* nonloop stmt always generates RSDs at level 1 */
    rset_info = rsd_aggregate(1, dh);
    dh->sgroup.groups[i]->rset = rset_info;

    /* mark location of message for all rsds in set */
    if (dh->in_ded)
      mark_rsd_loop(dh, dh->sgroup.groups[i]->stmt[0], rset_info, NULL);
  }

}

/*******************/
/* local functions */
/*******************/

/*--------------------------------------------------------------------

    rsd_save_loop()     Save RSDs collected for loop

*/

static int
rsd_save_loop(AST_INDEX stmt, int level, Dist_Globals *dh)
{
  Rsd_set_info *rset_info;
  FortD_LI *fli;
  int i;

  if (is_loop(stmt))
  {
    /* save all RSDs at this loop level in side array */

    rset_info = rsd_aggregate(loop_level(stmt), dh);

    /* mark loop type & location of message for all rsds for D editor */
    if (dh->in_ded)
      mark_rsd_loop(dh, stmt, rset_info, (FortD_LI*)get_info(dh->ped, stmt, type_fd));

    put_info(dh->ped, stmt, type_dc, (Generic)rset_info);
  }

  return WALK_CONTINUE;
}


/*--------------------------------------------------------------------

    rsd_aggregate()

    aggregate RSDs at loop/statement group 

*/

static Rsd_set_info *
rsd_aggregate(int level, Dist_Globals *dh)
{
  int i;
  Rsd_set_info *rset_info;

  /* save all RSDs at this loop level in side array */

  rset_info = (Rsd_set_info *) dc_get_mem(dh, sizeof(Rsd_set_info));
  bcopy(dh->rsd_loop_list + level, rset_info, sizeof(Rsd_set_info));

  /* clear Rsd_set_info at this level */

  for (i = 0; i < MAXREF; i++)
    dh->rsd_loop_list[level].rsd_s[i] = NULL;
  dh->rsd_loop_list[level].num_ref = 0;

  /* merge messages to same processor (from diff arrays) */

  if (dh->opts.mesg_aggregate)
    aggreg_msgs(dh, rset_info);

  return rset_info;
}

/*--------------------------------------------------------------------

    rsd_build_stmt()

    calculate all RSDs for one statement

*/

static int
rsd_build_stmt(AST_INDEX stmt, int level, Dist_Globals *dh)
{
  Iter_set *iset;
  struct dc_build_param param;

  if (is_assignment(stmt) || is_if(stmt) || is_logical_if(stmt))
  {
    /* collect info about lhs */
    param.dh = dh;
    param.iset = (Iter_set *) get_info(dh->ped, stmt, type_dc);
    param.sgroup = (S_group *) get_info(dh->ped, stmt, type_fd);
    /* assert (param.iset != (Iter_set *) NO_DC_INFO); */
    /* 9/25/93 RvH: apparently don't get Iter_set's for conditionals */
    if (param.iset == (Iter_set *) NO_DC_INFO) {
      return WALK_CONTINUE;
    }
    assert (param.sgroup != (S_group *) NO_FD_INFO);

    /* walk the statement looking for nonlocal rhs */

    if (is_assignment(stmt))
    {
      stmt = gen_ASSIGNMENT_get_rvalue(stmt);
      walk_expression(stmt, (WK_EXPR_CLBACK)rsd_build_rhs, NULL, (Generic)&param);
    }
    else if (is_logical_if(stmt))
    {
      stmt = gen_LOGICAL_IF_get_rvalue(stmt);
      walk_expression(stmt, (WK_EXPR_CLBACK)rsd_build_rhs, NULL, (Generic)&param);
    }
    else if (is_if(stmt))
    {
      stmt = gen_IF_get_guard_LIST(stmt);
      stmt = list_first(stmt);
      while (stmt != AST_NIL)
      {
        walk_expression(gen_GUARD_get_rvalue(stmt), 
                        (WK_EXPR_CLBACK)rsd_build_rhs, NULL, (Generic)&param);
        stmt = list_next(stmt);
      }
    }

  }
  return WALK_CONTINUE;
}


/*------------------------------------------------------------------------

  rsd_build_rhs()    Build RSD representing message for one rhs

  1) characterize communication for rhs  
  2) build RSD representing all data accessed by ref   
  3) use data dependence to decide where to build & insert RSD   
  4) determine whether carried RSD needed at deeper loop level   
  5) merge with other RSDs if possible, else store RSD 

*/

static int 
rsd_build_rhs(AST_INDEX rside, struct dc_build_param *param)
{
  Dist_Globals *dh;
  Iter_set *iset;
  Rsd_section *r_section, *carried_rsd;
  Subs_list *subscript;
  Rsd_set *rset, *old_rset;
  int comm_level, build_level, i, offset;
  Boolean indep;
  SNODE *sp;
  Mesg_type mtype;
  Comm_type ctype;
  Rsd_set_info *rset_list;

  dh = param->dh;

  /* Do not build RSD if it is not a subscript OR does not need regular
   * communication (b/c, for example, it's irregular or local)
   */
  if ((!is_subscript(rside)) || 
      (!dh->in_ded && !needs_reg_comm(dh->irr, rside)))
    return WALK_CONTINUE;

  /*-------------------------------------------------------*/
  /* 1) characterize communication caused by ref */

  iset = param->iset;

  sp = findadd2(rside, 0, 0, dh);
  if (!sp || !sp->decomp)       /* rhs is a local array, ignore */
    return WALK_CONTINUE;  

  subscript = (Subs_list *) get_info(dh->ped, rside, type_ref);
  if (subscript == (Subs_list *) NO_REF)
    die_with_message("rsd_build_rhs(): Missing ref info");

  ctype = rsd_comm_type(iset->lhs_sp, sp, iset->sinfo, subscript, iset);

  if (ctype == FD_COMM_NONE)     
    return WALK_CONTINUE;

  /*-------------------------------------------------------*/
  /* 2) build RSD representing all data accessed by ref    */

  /* build the rsd for the middle section  */

  r_section = dt_alloc_rsd(PED_DT_INFO(dh->ped));
  rsd_build_section(r_section, subscript, &iset->set, 1);

  /* need to convert constant subscript to local indices */
  if (ctype == FD_COMM_SEND_RECV)
  {
    /* find subscript in distributed dimension */

    for (i = 0; i < sp->numdim; i++)
    {
      if ((sp_is_part(sp,i) != FD_DIST_LOCAL) &&
          (subscript->subs[i].stype == SUBS_ZIV))
      {
         offset = iset->proc[0] * sp_block_size1(sp,i);

         if (r_section->subs[i].type == RSD_CONSTANT)
         {
           r_section->subs[i].constant -= offset;
         }
         else if (r_section->subs[i].type == RSD_RANGE)
         {
           r_section->subs[i].lo_b -= offset;
           r_section->subs[i].up_b -= offset;
           r_section->subs[i].begin -= offset;
         }
      }
    }
  }

  /* don't check COMM_BCAST when multiple executors, since   */
  /* the rsd has not been converted into local indices       */

  if ((ctype != FD_COMM_BCAST || iset->oneproc))
  {
    /* otherwise if only local accesses, then done */

    if (dc_local_access(dh, sp, r_section))
      return WALK_CONTINUE;
  }

  /* if nonlocal data accessed, calc & store overlap info */
  if (ctype == FD_COMM_SHIFT)
    dc_calc_overlap(dh, sp, r_section); 
  else
    dc_calc_buffer(dh, sp, r_section);

  /*-------------------------------------------------------*/
  /* 3) decide where to build RSD and insert the message   */

  /* decide COMMLEVEL  - the loop level for inserting the RSD */
  /*        MESG_TYPE  - the type of dependence causing mesg  */
  /*        BUILDLEVEL - the loop level for building the RSD  */

  comm_level = rsd_get_level(gen_SUBSCRIPT_get_name(rside), dh, &indep);

  if (indep)
  {
    mtype = FD_MESG_INDEP;
    build_level = comm_level;
  }
  else if (loop_part(iset->set.loops[comm_level-1].loop_index,dh))
  {
    if (ctype != FD_COMM_SHIFT)
      printf("rsd_build_rhs(): Unsupported comm for partitioned loop\n");

    mtype = FD_MESG_CARRIED_PART;
    build_level = comm_level;
  }
  else
  {
    mtype = FD_MESG_CARRIED_ALL;
    build_level = comm_level + 1;
  }

  /*-------------------------------------------------------*/
  /* 4) determine whether carried RSD is needed            */

  carried_rsd = NULL;
  for (i = 0; (i < subscript->dims) && !carried_rsd; i++)
  {
    if (subscript->subs[i].stype < build_level - 1)
    {
      /* build the carried rsd at the build_level */
      carried_rsd = dt_alloc_rsd(PED_DT_INFO(dh->ped));
      rsd_build_section(carried_rsd, subscript, &iset->set, build_level);
    }
  }

  /*-------------------------------------------------------------*/
  /* 5) find RSD list for this var (at comm level) if they exist */
  /*    merge with other RSDs if possible, else store RSD        */

  if (comm_level == MAXINT)   /* communication at sgroup */
  {
    if (!param->sgroup)
    {
      printf("rsd_build_rhs(): missing stmt group\n");
      return WALK_CONTINUE;
    }

    if (!param->sgroup->rset)
    {
      param->sgroup->rset = (Rsd_set_info *) 
        dc_get_mem(dh, sizeof(Rsd_set_info));
      bzero(param->sgroup->rset, sizeof(Rsd_set_info));
    }
    rset_list = param->sgroup->rset;
  }
  else                        /* communication at loop   */
  {
    rset_list = dh->rsd_loop_list + comm_level;
  }

  old_rset = rset = rsd_find_set(rset_list, sp);

  /* Check whether we can merge RSD with any of     */
  /* the RSDs currently at this loop level, if any  */

  while (dh->opts.mesg_coalesce && rset)    /* go thru list of RSDs */
  {
    /* merge only if same message & communication type */

    if ((rset->mtype == mtype) && (rset->ctype == ctype))
    {
      if (coalesce_rsds(dh, rset, r_section, carried_rsd, iset))
      {
        /* add rhs as one of the refs merged into this RSD set */

        rset->subs[rset->num_subs] = rside; 
        rset->sinfo[rset->num_subs++] = subscript; 

        /* ensure that the largest nonlocal section is in sinfo[0] */

        for (i = 0; i < sp->numdim; i++)
        {
          if (sp_is_part(sp,i) != FD_DIST_LOCAL)
          {
            if (((subscript->subs[i].constant > 0) && 
                 (subscript->subs[i].constant >
                   rset->sinfo[0]->subs[i].constant)) ||
                ((subscript->subs[i].constant < 0) && 
                 (subscript->subs[i].constant <
                   rset->sinfo[0]->subs[i].constant)))
            {
              /* need to swap sinfo[0] and sinfo[rset->num_subs-1] */

              rside = rset->subs[rset->num_subs-1];
              subscript = rset->sinfo[rset->num_subs-1];
              rset->subs[rset->num_subs-1] = rset->subs[0];
              rset->sinfo[rset->num_subs-1] = rset->sinfo[0];
              rset->subs[0] = rside;
              rset->sinfo[0] = subscript;

              break;    /* fall out of loop */
            }
          }
        }

        /* if needed for D editor, store RSD at ref, mark sgroup */
        if (dh->in_ded)
        {
          put_info(dh->ped, gen_SUBSCRIPT_get_name(rside), type_dc, (Generic)rset); 
          if (param->iset)
            param->iset->nonlocal_refs++;
          if (param->sgroup)
            param->sgroup->nonlocal_refs++;
        }

        return WALK_CONTINUE;
      }
    }

    rset = rset->rsd_next;
  }

  /*-----------------------------------------------------------------*/
  /* if we reach here, RSD is not merged, add it to the list of RSDs */

  rset = dc_alloc_rsd_set(dh);

  rset->rs = r_section;
  rset->rs_carried = carried_rsd;
  rset->iterset = iset;
  rset->subs[0] = rside;
  rset->sinfo[0] = subscript;
  rset->num_subs = 1;
  rset->num_merge = 0;
  rset->build_level = build_level;
  rset->mtype = mtype;
  rset->ctype = ctype;
  rset->sp = sp;
  rset->rsd_merge = NULL;
  rset->rsd_next = NULL;

  /* if previous refs to this var at level, append to end of list */
  if (old_rset)  
  {
    while (old_rset->rsd_next)
      old_rset = old_rset->rsd_next;

    old_rset->rsd_next = rset;
  }

  /* else first ref to this var at level,store rsd into the rsd list */
  else
  {
    rset_list->rsd_s[rset_list->num_ref++] = rset;
  }

  /* if needed for D editor, store RSD at ref, mark sgroup */
  if (dh->in_ded)  
  {
    put_info(dh->ped, gen_SUBSCRIPT_get_name(rside), type_dc, (Generic)rset); 
    if (param->iset)
      param->iset->nonlocal_refs++;
    if (param->sgroup)
      param->sgroup->nonlocal_refs++;
  }

  return WALK_CONTINUE;
}


/*------------------------------------------------------------------------

  rsd_find_set()       Find rsd set for var at given loop & level

*/

static Rsd_set *
rsd_find_set(Rsd_set_info *rset_info, SNODE *sp)
{
  Rsd_set *rset;
  int i;

  for (i = rset_info->num_ref - 1; i >= 0; i--)
  {
    rset = rset_info->rsd_s[i];
    if (rset && (rset->sp == sp))
      return rset;
  }
  return (Rsd_set *) NULL;
}

/*------------------------------------------------------------------------

  rsd_comm_meet()       Calculate MEET of two comm types

*/

static Comm_type 
rsd_comm_meet(Comm_type c1, Comm_type c2)
{
  if (c1 == c2)
    return c1;

  if (c1 == FD_COMM_NONE)
    return c2;

  if (c2 == FD_COMM_NONE)
    return c1;

  if (c1 == FD_COMM_SHIFT)
    return c2;

  if (c2 == FD_COMM_SHIFT)
    return c1;

  return FD_COMM_UNKNOWN;
}


/*------------------------------------------------------------------------

  rsd_comm_type()  Determine the type of communication caused by rhs

  Try to rely on syntactic comparison as much as possible.

  One exception - when lhs is known to be executed by only one proc
                  return SEND_RECV, not BROADCAST.

*/

static Comm_type 
rsd_comm_type(SNODE *lhs_sp, SNODE *rhs_sp, Subs_list *lhs_subs, 
              Subs_list *rhs_subs, Iter_set *iset)
{
  SNODE *decomp_sp;
  Subs_data *rsub, *lsub, *rsubs[MAXSUBS], *lsubs[MAXSUBS];
  Comm_type ctype, comm[MAXSUBS];
  int i, j;

  if (iset->allproc)
  {
    ctype = sp_decomp(rhs_sp) ? FD_COMM_BCAST : FD_COMM_NONE;
    return ctype;
  }

  if (lhs_sp->decomp != rhs_sp->decomp)
    printf("Warning: arrays with different decomposition\n");

  decomp_sp = rhs_sp->decomp;

  /* iterate for each dimension of the decomposition */

  for (i = 0; i < decomp_sp->numdim; i++)
  {
    comm[i] = FD_COMM_NONE;
    rsubs[i] = NULL;
    lsubs[i] = NULL;

    if (sp_is_part(decomp_sp, i) != FD_DIST_LOCAL)
    {
       rsub = NULL;
       lsub = NULL;

       /* get lhs & rhs dims aligned with distributed dim of decomp */
       for (j = 0; j < rhs_sp->numdim; j++)
       {
         if (sp_ddim(rhs_sp,j) == i+1)
           rsub = rhs_subs->subs + j;
       }
       for (j = 0; j < lhs_sp->numdim; j++)
       {
         if (sp_ddim(lhs_sp,j) == i+1)
           lsub = lhs_subs->subs + j;
       }
       rsubs[i] = rsub;
       lsubs[i] = lsub;

       if (!rsub || !lsub)         /* escape for now */
         return FD_COMM_UNKNOWN;

       /* figure out what type of communication will result */

       if (lsub->stype < SUBS_SIV)         /* case 1: lhs = SIV */
       {
         if (rsub->stype == lsub->stype)     /* rhs = same SIV */
         {
           if (rsub->coeffs[rsub->stype] != lsub->coeffs[lsub->stype])
           {
             comm[i] = FD_COMM_BCAST;
           }
           else if (rsub->constant != lsub->constant)
             comm[i] = FD_COMM_SHIFT;
         }
         else if (rsub->stype == SUBS_ZIV)   /* rhs = ZIV */
         {
           if (iset->oneproc)
             comm[i] = FD_COMM_SEND_RECV;
           else
             comm[i] = FD_COMM_BCAST;
         }
         else if (rsub->stype < SUBS_SIV)    /* rhs = different SIV */
         {
           comm[i] = FD_COMM_BCAST;

           for (j = 0; j < i; j++)
           {
             if (lsubs[j] && (lsubs[j]->stype == rsubs[i]->stype) &&
                             (rsubs[j]->stype == lsubs[i]->stype))
             {
               comm[i] = FD_COMM_TRANSPOSE;
               comm[j] = FD_COMM_TRANSPOSE;
             }
           }
         }
         else                                /* rhs = too complex */
         {
           comm[i] = FD_COMM_UNKNOWN;
         }
       }

       else if (lsub->stype == SUBS_ZIV)  /* case 2: lhs = ZIV */ 
       {
         if (rsub->stype == SUBS_ZIV)        /* rhs = ZIV */
         {
           if (rsub->constant != lsub->constant)
             comm[i] = FD_COMM_SEND_RECV;
         }
         else if (rsub->stype < SUBS_SIV)    /* rhs = SIV */
         {
           comm[i] = FD_COMM_REDUCE;
         }
         else                                /* rhs = too complex */
         {
           comm[i] = FD_COMM_UNKNOWN;
         }
       }
       else                                /* case 3: lhs = complex */
       {
         if (rsub->stype == SUBS_ZIV)        /* rhs = ZIV */
         {
           comm[i] = FD_COMM_BCAST;
         }
         else
         {
           comm[i] = FD_COMM_UNKNOWN;
         }
       }
    }
  }

  /* assign to RSD most complex COMM pattern */

  ctype = FD_COMM_NONE;
  for (i = 0; i < decomp_sp->numdim; i++)
  {
    ctype = rsd_comm_meet(ctype, comm[i]);
  }

  return ctype;
}


/*------------------------------------------------------------------------

  rsd_get_level()  Calculates loop level for RSD representing comm

    commlevel of a reference =                                      

       MAX(deepest loop-carried cross-processor true dependence,     
           deepest common loop surrounding both src & sink of        
           the loop-independent cross-processor true dependence)   

    See: Balasundaram et al. in Proceedings of DMCC5, Apr 1990      

    Also need to know whether dependence was from a carried
    or loop indep dependence, since it affects code generation.

    Returns: commlevel & CARRIED/INDEP                              
                                                                
*/

static int
rsd_get_level(AST_INDEX id, Dist_Globals *dh, Boolean *indep)
{
  int ref, depth, commlevel, lvl, i;
  AST_INDEX source, sink, loops[MAXLOOP], prev_loop;
  EDGE_INDEX edge;
  DG_Edge *Earray;      /* array of all DG edges    */
  DG_Edge *final_edge = (DG_Edge*) NULL;

  commlevel = 1;        /* default = outermost loop & INDEP */
  *indep = true;        /* i.e., defined outside loop nest  */

  /* if no dependences, then create RSD at outermost loop */

  if ((ref = get_info(dh->ped, id, type_levelv)) == NO_LEVELV) {
    SDDF_StoreDepEdgeForRef(id, (DG_Edge*) NULL); 
    return commlevel;
  }

  /* look at all dependences edges with id as sink */

  Earray = dg_get_edge_structure( PED_DG(dh->ped));

  for (edge = dg_first_sink_ref( PED_DG(dh->ped), ref);
       edge >= 0;
       edge = dg_next_sink_ref( PED_DG(dh->ped), edge))
  {
    if (id != Earray[edge].sink)
      die_with_message("rsd_get_level(): DG error, ref_list invalid");

    if (Earray[edge].type == dg_true)
    {
      /* calculate commlevel for loop-independent dep */

      if (Earray[edge].level == LOOP_INDEPENDENT)
      {
        source = Earray[edge].src;
        sink = id;

        /* get all loops around source */

        depth = 0;
        while (source != AST_NIL)
        {
          source = tree_out(source);
          if (is_loop(source))
            loops[depth++] = source;
        }

        /* compare with loops around sink */

        prev_loop = AST_NIL;

        while (sink != AST_NIL)
        {
          sink = tree_out(sink);

          if (!is_loop(sink))
            continue;

          for (i = 0; i < depth; i++)
          {
            if (sink == loops[i])       /* found it!    */
            {
              lvl = depth - i;  /* real loop level */

              if ((lvl == depth) && (prev_loop == AST_NIL))
              {
                /* loop indep dep in same loop body    */
                /* this is deepest possible dependence */

                *indep = true;
		SDDF_StoreDepEdgeForRef(id, &Earray[edge]);
                return MAXINT;
              }
              else
              {
                lvl++;  /* communicate at next inner loop */

                if (lvl > commlevel)
                {
                  commlevel = lvl;
                  *indep = true;
                }
              }

	      final_edge = &Earray[edge]; /* VSA: remember for later use */
              break;    /* no need to check further */
            }
          }

          prev_loop = sink;
        }
      }

      /* calculate commlevel for loop-carried dep */

      else if (Earray[edge].level >= commlevel)
      {
        commlevel = Earray[edge].level;
        *indep = false;
	final_edge = &Earray[edge];	/* VSA: remember for later use */
      }
    }
  }

  SDDF_StoreDepEdgeForRef(id, final_edge); /* final_edge may be NULL */
  
  return commlevel;
}

/*----------------------------------------------------------------------

  coalesce_rsds()   Merge RSDs for same array

 1) Test whether RSDs should be merged
 2) Merge RSDs if no precision lost
 3) Return whether merge occurred

 To avoid loss of precision, for now merge RSDS only if same direction.

*/

static Boolean 
coalesce_rsds(Dist_Globals *dh, Rsd_set *rset, Rsd_section *rsd_sect, 
              Rsd_section *carried_rsd, Iter_set *iset)
{
  int i;
  Rsd_section *result_rsd, *merge_rsd;
  SNODE *sp;

  merge_rsd = rset->rs;
  sp = rset->sp;

  /* check all distributed dims */

  for (i = sp->numdim - 1; i >= 0; i--)
  {
    switch (sp_is_part(sp, i))
    {
      case FD_DIST_BLOCK:
        /* If same range type (i.e. positive, negative, or both) then okay */
        if (dc_range_type(dc_rsd_lower(rsd_sect, i), 
                           dc_rsd_upper(rsd_sect, i)) != 
            dc_range_type(dc_rsd_lower(merge_rsd, i), 
                           dc_rsd_upper(merge_rsd, i)))
          return false;

        if (!dc_rsd_contiguous(rsd_sect,merge_rsd,sp->numdim))
          return false;
        break;

      case FD_DIST_CYCLIC:
        /* If dims identical then okay */
        if (bcmp(rsd_sect->subs+i,merge_rsd->subs+i,sizeof(Rsd_data)))
          return false;
        break;

      default:
        break;
    }
  }

  /* conditions have been met, coalesce RSDs */

  result_rsd = dt_alloc_rsd(PED_DT_INFO(dh->ped));
  rsd_merge(result_rsd, rsd_sect, merge_rsd);
  rset->rs = result_rsd;

  /* also coalesce carried RSDs, if any */

  if (!rset->rs_carried)
    rset->rs_carried = carried_rsd;
  else if (carried_rsd)
  {
    result_rsd = dt_alloc_rsd(PED_DT_INFO(dh->ped));
    rsd_merge(result_rsd, carried_rsd, rset->rs_carried);
    rset->rs_carried = result_rsd;
  }

  /* use deepest iteration set */

  if (iset->set.level > rset->iterset->set.level)
    rset->iterset = iset;

  return (true);
}


/*--------------------------------------------------------------------

   dc_rsd_contiguous()   Decide whether two RSDs are contiguous 

*/

static Boolean
dc_rsd_contiguous(Rsd_section *rsd1, Rsd_section *rsd2, int dim)
{
  int i, up1, up2;

  for (i = 0; i < dim; i++)
  {
    up1 = dc_rsd_upper(rsd1, i);
    up2 = dc_rsd_upper(rsd2, i);

    if (up1 > up2)  /* rsd1 bigger */
    {
      if (dc_rsd_lower(rsd1, i) > (up2 + 1))
        return false;
    }
    else if (up1 < up2)   /* rsd2 bigger */
    {
      if (dc_rsd_lower(rsd2, i) > (up1 + 1))
        return false;
    }
  }

  return true;
}


/*--------------------------------------------------------------------

   can_aggreg_rsds()   

   Decide whether two RSDs for different arrays 
   may be merged into a single message 

*/

static Boolean
can_aggreg_rsds(Rsd_set *from, Rsd_set *to)
{
  int level, dims, i;

  /* 1) check that general stats are equivalent */

  if ((from->build_level != to->build_level) ||
      (from->mtype       != to->mtype) ||
      (from->ctype       != to->ctype))
     return false;

  /* 2) check that Iter_sets are equivalent */

  /* woefully inadequate, but good enough for this week's demo :-) */

  level = from->iterset->set.level;
  if (level != to->iterset->set.level)
    return false;

  for (i = 0; i < level; i++)
  {
    if (from->iterset->type[i] != to->iterset->type[i])
      return false;
  }

  /* 3) check that RSD sections are equivalent */

  dims = from->rs->dims;
  if (dims != to->rs->dims)
    return false;

  /* bitwise compare of all used subscript dimensions */
  if (bcmp(from->rs->subs, to->rs->subs, sizeof(Rsd_data) * dims))
    return false;

  return true;
}


/*--------------------------------------------------------------------

    aggreg_rsds()

    Compare sets of RSDs for two perfectly aligned arrays, merge 
    them where possible.

    Returns:  list of remaining unmerged RSDs 
*/

static Rsd_set*
aggreg_rsds(Rsd_set *rset_from, Rsd_set *rset_to)
{
  Rsd_set *rsds[MAXREF], *from, *to, *result;
  int i, num;

  /* first store away all RSDs from rset_from in rsds */
  num = 0;
  for (from = rset_from; from; from = from->rsd_next)
    rsds[num++] = from;

  for (i = 0; i < num; i++)     /* disconnect list of from RSDs */
    rsds[i]->rsd_next = NULL;

  /* merge RSDs in from list where possible */

  for (i = 0; i < num; i++)
  {
    /* if already merged with another RSD, then RSD should    */
    /* not be able to find another mergeable RSD (in theory)  */

    from = rsds[i];
    if (from->num_merge)
      continue;

    /* compare from RSD with list of RSDs in to list */

    for (to = rset_to; to; to = to->rsd_next)
    {
      if (can_aggreg_rsds(from, to))
      {
        /* insert from RSD into merge list of to RSD */

        from->rsd_merge = to->rsd_merge;
        from->rsd_top = to;      /* point to first RSD in aggreg list */
        to->rsd_merge = from;
        to->num_merge++;

        /* zap from RSD, go to next from RSD */

        rsds[i] = NULL;
        break;
      }
    }
  }

  /* find, relink, & return list of unmerged RSDS */

  result = NULL;
  for (i = num - 1; i >= 0; i--)
  {
    if (rsds[i])
    {
      rsds[i]->rsd_next = result;
      result = rsds[i];
    }
  }

  return result;
}


/*--------------------------------------------------------------------

    aggreg_msgs()

    Merge all rsds from same rsd_set so that only
    one message is sent per processor.

*/

static void
aggreg_msgs(Dist_Globals *dh, Rsd_set_info *rset_info)
{
  Rsd_set *rset_from, *rset_to;
  int i, j;

  /* start merging from end of list - we don't need to check last one */
  for (i = rset_info->num_ref - 1; i > 0; i--)
  {
    rset_from = rset_info->rsd_s[i];

    /* for each RSD set, try to merge into RSD sets in front */

    for (j = 0; (j < i) && rset_from; j++)
    {
      if (i != j)  /* don't merge with self */
      {
        rset_to = rset_info->rsd_s[j];

        /* for now, only merge msgs for perfectly aligned arrays */

        if (perfect_align(rset_from->sp, rset_to->sp))
          rset_from = aggreg_rsds(rset_from, rset_to);
      }
    }

    /* store results of all attempted merges */

    rset_info->rsd_s[i] = rset_from;
  }
}


/*---------------------------------------------------------------------

  perfect_align()    Determine whether two arrays are perfectly aligned

*/

static Boolean
perfect_align(SNODE *sp1, SNODE *sp2)
{
  int i;

  if (sp1->decomp != sp2->decomp)
    return false;

  if (sp1->perfect_align && sp2->perfect_align)
    return true;

  if (sp1->numdim != sp2->numdim)
    return false;

  /* check that alignment is the same in each relevant dimension */

  for (i = 0; i < sp1->decomp->numdim; i++)
  {
    if ((sp_align_stype(sp1, i) != sp_align_stype(sp2, i)) ||
        (sp_align_index(sp1, i) != sp_align_index(sp2, i)) ||
        (sp_align_offset(sp1,i) != sp_align_offset(sp2,i)) ||
        (sp_align_coeff(sp1, i) != sp_align_coeff(sp2, i)))
      return false;
  }

  return true;
}

/*---------------------------------------------------------------------

  dc_alloc_rsd_set()     Allocate & initialize Rsd_set

*/


static Rsd_set *
dc_alloc_rsd_set(Dist_Globals *dh)
{
  int i;
  Rsd_set *rset;

  rset = (Rsd_set *) dc_get_mem(dh, sizeof(Rsd_set));
  bzero(rset, sizeof(Rsd_set));

  for (i = 0; i < MAXSUBS; i++)
    rset->subs[i] = AST_NIL;

  rset->num_subs = 0;
  return rset;
}


/*----------------------------------------------------------------------

  dc_calc_overlap()

  Update the accessed parts of the distributed arrays, 
  will be used for computing overlap regions.

*/
  
static void
dc_calc_overlap(Dist_Globals *dh, SNODE *sp, Rsd_section *rsd_sect)
{
  int i, ref_min, ref_max;

  for (i = 0; i < sp->numdim; ++i)
  {
    if (rsd_sect->subs[i].type == RSD_RANGE)
    {
      ref_min = rsd_sect->subs[i].lo_b;
      ref_max = rsd_sect->subs[i].up_b;

      if ((ref_min != MININT) && (ref_min < sp_min_access(sp,i)))
      {
        if (!dh->in_ped)
          printf("F77D: Overlap for %s exceeded estimate\n", sp->id);

        sp_put_min_access(sp, i, ref_min);          
      }
      if ((ref_max != MAXINT) && (ref_max > sp_max_access(sp, i)))
      {
        if (!dh->in_ped)
          printf("F77D: Overlap for %s exceeded estimate\n", sp->id);

        sp_put_max_access(sp, i, ref_max);
      }
    }
  }
}


/*--------------------------------------------------------------------

    dc_calc_buffer()  Calculate temporary buffer requirements

*/

static void
dc_calc_buffer(Dist_Globals *dh, SNODE *sp, Rsd_section *r_section)
{
}

/*----------------------------------------------------------------------

  loop_part()   Test whether loop iterations are partitioned

*/

static Boolean
loop_part(AST_INDEX loop, Dist_Globals *dh)
{
  FortD_LI *fli;

  fli = (FortD_LI *) get_info(dh->ped, loop, type_fd);
  if (fli != (FortD_LI *) NO_FD_INFO)
    return fli->localized;

  printf("loop_part(): unrecognized loop\n");
  return false;
}

/*--------------------------------------------------------------------

    mark_rsd_loop()     Mark message location for RSD set
                        Mark pipelined loops
*/

static void
mark_rsd_loop(Dist_Globals *dh, AST_INDEX stmt, Rsd_set_info *rset_info, FortD_LI *fli)
{
  Rsd_set  *rset, *rset1;
  int i;

  /* decide on execution type of loop, if any */
  if (fli)
  {
    if (fli->dist_type == FD_DIST_LOCAL)
      fli->ltype = FD_LOOP_REPLICATED;
    else if (fli->iset->oneproc)
      fli->ltype = FD_LOOP_ONEPROC;
    else
      fli->ltype = FD_LOOP_PARALLEL;
  }

  /* mark locations & types of RSDs */
  for (i = 0; i < rset_info->num_ref; i++)
  {
    rset = rset_info->rsd_s[i];
    while (rset)
    {
      rset1 = rset;
      rset = rset->rsd_next;
      while (rset1)
      {
        if (fli)
        {
          if (rset1->mtype == FD_MESG_CARRIED_PART)
          {
            fli->ltype = FD_LOOP_PIPELINED;
            
            switch (rset1->ctype)
            {
              case FD_COMM_SHIFT:
              case FD_COMM_SEND_RECV:
                fli->num_c_part_send++;
                break;

              default:
                printf("mark_rsd_loop(): Unexpected pipelined message\n");
            }
          }
          else if (rset1->mtype == FD_MESG_CARRIED_ALL)
          {
            switch (rset1->ctype)
            {
              case FD_COMM_SHIFT:
              case FD_COMM_SEND_RECV:
                fli->num_c_all_send++;
                break;
  
              case FD_COMM_BCAST:
                fli->num_c_all_bcast++;
                break;
  
              case FD_COMM_GATHER:
                fli->num_c_all_gather++;
                break;
  
              default:
                /* not yet handled or counted */
                break;
            }
          }
          else /* (rset1->mtype == FD_MESG_INDEP) */
          {
            switch (rset1->ctype)
            {
              case FD_COMM_SHIFT:
              case FD_COMM_SEND_RECV:
                fli->num_indep_send++;
                break;
  
              case FD_COMM_BCAST:
                fli->num_indep_bcast++;
                break;
  
              case FD_COMM_GATHER:
                fli->num_indep_gather++;
                break;
  
              default:
                /* not yet handled or counted */
                break;
            }
          }
        }

        rset1->location = stmt;
        rset1 = rset1->rsd_merge;
      }
    }
  }
}

/* eof */

