/* $Id: bc.ansi.c,v 1.35 1997/03/27 20:32:26 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/



/*---------------------------------------------------------------------

    bc.c    Computes the computation partition for a Fortran D 
            program based on owner computes rule. 

    Author : Seema Hiranandani 

*/

#include <libs/fortD/codeGen/private_dc.h>
#include <libs/frontEnd/ast/groups.h>

/*------------------------- extern definitions ------------------------*/

EXTERN(Loop_info, *find_loop,(LI_Instance * LI, AST_INDEX lindex));          
                                                    /* li.c          */
EXTERN(AST_INDEX, dc_outer_perf,(AST_INDEX node));  /* bounds.c      */
EXTERN(const char *, ValDecomp_get_upper_bound,
       (Generic irr, SNODE *sp));
EXTERN(Generic,     ValDecomp_get_hidden_vd, (Generic irr, SNODE *sp));
EXTERN(void, dc_irreg_lhs,(Generic irr, AST_INDEX lhs_node));                      
                                                    /* irr_msgs.C    */
EXTERN(Iter_set, *iter_build_reduction,(Dist_Globals *dh, AST_INDEX lhs, 
                                        AST_INDEX loop));         
                                                    /* coll_comm.c   */
EXTERN(Boolean, is_private_var,(LI_Instance *LI, AST_INDEX var, AST_INDEX  loop));
                                                    /* LI_Instance.h */

EXTERN(AST_INDEX, get_home_node, (Generic irr, AST_INDEX lhs));

/*------------------------- global definitions ------------------------*/

void dc_partition(Dist_Globals *dh);
Iter_set *iter_build_assign(Dist_Globals *dh, AST_INDEX lhs, AST_INDEX loop);
Iter_set *dc_alloc_iter_set(Dist_Globals *dh);
Boolean is_diff_isets(Iter_set *iset1, Iter_set *iset2, int lvl);
void dc_find_stmt_groups(Dist_Globals *dh, AST_INDEX stmt_list, Stmt_groups *sgroup,
                         int gnum, int lvl, Boolean nonloop);

/*------------------------- local definitions ------------------------*/

STATIC(Iter_set, *iter_build_private,(Dist_Globals *dh, AST_INDEX lhs, AST_INDEX rhs,
                                      AST_INDEX loop));
STATIC(Iter_set, *iter_build_io,(Dist_Globals *dh, AST_INDEX stmt, AST_INDEX loop));

STATIC(void, iter_build_block,(Subs_list *sinfo, Loop_list *linfo, int dim, 
                               Iter_set *iset, Dist_Globals *dh, SNODE *sp));
STATIC(void, iter_build_cyclic,(Subs_list *sinfo, Loop_list *linfo, int dim,
                                Iter_set *iset, Dist_Globals *dh, SNODE *sp));
STATIC(void, iter_build_bkcyclic,(Subs_list *sinfo, Loop_list *linfo, int dim,
                                  Iter_set *iset, Dist_Globals *dh));
STATIC(void, iter_build_user,(Subs_list *sinfo, Loop_list *linfo, int dim,
                                  Iter_set *iset, Dist_Globals *dh, SNODE *sp));
STATIC(int, iter_build_stmt,(AST_INDEX stmt, int level, Dist_Globals *dh));

STATIC(Boolean, is_diff_loops,(Loop_data *loop1, Loop_data *loop2));
STATIC(Boolean, is_diff_expr,(Expr *expr1, Expr *expr2));

STATIC(int, find_stmt_groups,(AST_INDEX stmt, int level, Dist_Globals *dh));
STATIC(int, collect_exec_stmts,(AST_INDEX stmt, int level, AST_INDEX **execs));

/********************/
/* global functions */
/********************/

/*--------------------------------------------------------------------

    dc_partition()

    Partitions computation into iteration sets using "owner computes" rule

*/

void
dc_partition(Dist_Globals *dh)
{
  if (dh->numdoloops >= LOOPTOTAL)
    die_with_message("dc_partition(): max loop number exceeded");

  /* calculate iteration sets for all statements in loops    */
  /* also marks loops whose iterations are partitioned       */
  walk_statements_reverse(dh->root, LEVEL1, (WK_STMT_CLBACK)iter_build_stmt, NULL, 
                          (Generic)dh);

  /* find & group statements not in loops */
  walk_statements(dh->root, LEVEL1, (WK_STMT_CLBACK)find_stmt_groups, NULL, (Generic)dh);
}

/*--------------------------------------------------------------------

  iter_build_assign()    Build iteration set for assignment stmt

*/

Iter_set *
iter_build_assign(Dist_Globals *dh, AST_INDEX lhs, AST_INDEX loop)
{
  SNODE *sp;
  int i, j, ivar;
  Iter_set *iset;
  Subs_list *sinfo;
  Loop_list *linfo;

  /*-------------------------------------------------------------*/
  /* if scalar, all processors will execute the statement        */
  /*-------------------------------------------------------------*/

  if (!is_subscript(lhs))
  {
    iset = dc_alloc_iter_set(dh);
    iset->allproc = true;

    if (loop == AST_NIL)
      return iset;

    linfo = (Loop_list *) get_info(dh->ped, loop, type_ref);
    if (linfo == (Loop_list *) NO_REF)
      die_with_message("iter_build_assign(): missing loop info");

    bcopy(linfo, &iset->set, sizeof(Loop_list));
    return iset;
  }

  /*-------------------------------------------------------------*/
  /* calculate iteration for assignments to arrays               */
  /*-------------------------------------------------------------*/

  /* calculate iteration set based on subscript in each dim */

  sinfo = (Subs_list *) get_info(dh->ped, lhs, type_ref);
  if (sinfo == (Subs_list *) NO_REF)
    die_with_message("iter_build_assign(): missing subs info");

  linfo = sinfo->loop_nest;
  iset = dc_alloc_iter_set(dh);

  /* remember lhs used to generate iteration set */

  iset->lhs = lhs;
  iset->sinfo = sinfo;

  /* start with full iteration set as default */

  if (linfo)
  {
    bcopy(linfo, &iset->set, sizeof(Loop_list));
    bcopy(linfo, &iset->pre_set, sizeof(Loop_list));
    bcopy(linfo, &iset->post_set, sizeof(Loop_list));
  }

  sp = findadd2(lhs, 0, 0, dh);
  iset->lhs_sp = sp;    /* store away symbol table ptr for lhs */

  if (!sp || !sp->decomp)   /* if local array, all procs execute stmt */
    iset->allproc = true;
  else
  {
    for (i = 0; i < sinfo->dims; i++)
    {
    /*-------------------------------------------------------------------*/
    /* store information on the subscript in the dimension corresponding */
    /* to the loop index variable                                        */

      switch (sp_is_part(sp, i))
      {
      case FD_DIST_BLOCK:

        ivar = sinfo->subs[i].stype;
        if (ivar <= SUBS_SIV)   /* only SIV subscripts for now */
          iset->dim_num[ivar] = i;

        iter_build_block(sinfo, linfo, i, iset, dh, sp);

        break;

      case FD_DIST_CYCLIC:
        iter_build_cyclic(sinfo, linfo, i, iset, dh, sp);

      case FD_DIST_BLOCK_CYCLIC:
        iter_build_bkcyclic(sinfo, linfo, i, iset, dh);
        break;

      case FD_DIST_USER:
        iter_build_user(sinfo, linfo, i, iset, dh, sp);
        break;

      default:
        break;     /* no work needed */
      }
    }
  }

  return iset;
}


/*--------------------------------------------------------------------

    dc_alloc_iter_set()

    Allocate & return pointer to iter set

*/

Iter_set *
dc_alloc_iter_set(Dist_Globals *dh)
{
  int i;
  Iter_set *iset;

  iset = (Iter_set *) dc_get_mem(dh, sizeof(Iter_set));

  bzero(iset, sizeof(Iter_set));

  for (i = 0; i < MAXLOOP; i++)
  {
    iset->type[i]       = Iter_simple;
    iset->cyclic[i]     = Citer_none;
    iset->user[i]       = Uiter_none;
    iset->irr_decomp[i] = NULL;
  }

  iset->private_use = AST_NIL;
  iset->reduc_set   = NULL;     /* not really needed due to bzero() */
  iset->oneproc     = false;
  iset->allproc     = false;
  iset->reduction   = false;
  iset->lhs_private = false;

  return iset;
}


/*----------------------------------------------------------------------

 is_diff_isets()   Check whether two iteration sets are different

*/
Boolean
is_diff_isets(Iter_set *iset1, Iter_set *iset2, int lvl)
   /* lvl: -1 if all loops, else from 0...MAXLOOP-1 */
{
  int i;

/*
  if (iset1->allproc)
    return (iset2->allproc) ? false : true;
*/

  if (iset1->oneproc || iset2->oneproc)
  {
    if (!iset1->oneproc || !iset2->oneproc)
      return true;

    return (iset1->proc[0] != iset2->proc[0]);
  }

  for (i = 0; i < iset1->set.level; i++)
  {
    if ((lvl != -1) && (i != lvl))
      continue;

    if ((iset1->type[i] != iset2->type[i]) || 
        (iset1->cyclic[i] != iset2->cyclic[i]) ||
        (iset1->bksize[i] != iset2->bksize[i]))
      return true;

    if ((iset1->pre_idle[i] != iset2->pre_idle[i]) ||
        (iset1->post_idle[i] != iset2->post_idle[i]))
      return true;

    switch (iset1->type[i])
    {
      case Iter_simple:
        if (is_diff_loops(iset1->set.loops+i, iset2->set.loops+i))
          return true;
        break;

      case Iter_pre:
        if (is_diff_loops(iset1->set.loops+i, iset2->set.loops+i) ||
            is_diff_loops(iset1->pre_set.loops+i, iset2->pre_set.loops+i))
          return true;
        break;

      case Iter_post:
        if (is_diff_loops(iset1->set.loops+i, iset2->set.loops+i) ||
            is_diff_loops(iset1->post_set.loops+i, iset2->post_set.loops+i))
          return true;
        break;

      case Iter_pre_only:
        if (is_diff_loops(iset1->pre_set.loops+i, iset2->pre_set.loops+i))
          return true;
        break;

      case Iter_post_only:
        if (is_diff_loops(iset1->post_set.loops+i, iset2->post_set.loops+i))
          return true;
        break;

      case Iter_all:
        if (is_diff_loops(iset1->set.loops+i, iset2->set.loops+i) ||
            is_diff_loops(iset1->pre_set.loops+i, iset2->pre_set.loops+i) ||
            is_diff_loops(iset1->post_set.loops+i, iset2->post_set.loops+i))
          return true;
        break;

      default:
        printf("is_diff_iset : Case not handled \n");
        break;
    }
  }

  /* if it reaches this point then return false */
  return false;
}


/*--------------------------------------------------------------------

    dc_find_stmt_groups()     Partition stmts according to iteration sets

*/

void
dc_find_stmt_groups(Dist_Globals *dh, AST_INDEX stmt_list, Stmt_groups *sgroup, 
                    int gnum, int lvl, Boolean nonloop)
{
  Iter_set *iset;
  AST_INDEX stmt, exec[MAXSTMTS], *eptr, *last;

  if (!sgroup->groups[gnum])
    sgroup->groups[gnum] = (S_group *) dc_get_mem(dh, sizeof(S_group));

  bzero(exec, sizeof(AST_INDEX) * MAXSTMTS);

  last = exec;
  walk_statements(stmt_list, LEVEL1, (WK_STMT_CLBACK)collect_exec_stmts, NULL, 
                  (Generic)&last);

  for (eptr = exec; eptr != last; eptr++)
  {
    stmt = *eptr;
    if (!is_assignment(stmt) && !io_stmt(stmt) && 
        !is_if(stmt) && !is_logical_if(stmt))
      continue;

    iset = (Iter_set *) get_info(dh->ped, stmt, type_dc);

    if ((iset == (Iter_set *) NO_DC_INFO) ||
        (nonloop && (loop_level(stmt) > 0)))
    {
      if (sgroup->groups[gnum]->size > 0)  /* group is active   */
      {
        sgroup->group_num++;              /* end this group */
        gnum++;

        sgroup->groups[gnum] = (S_group *) dc_get_mem(dh, sizeof(S_group));
        sgroup->groups[gnum]->size = 0;
      }
    }
    else
    {
      if (sgroup->groups[gnum]->size > 0)  /* group is active */
      {
        /* if same iteration set as prev statement, append & continue */

        if (!is_diff_isets(iset, sgroup->groups[gnum]->iset, lvl))
        {
          sgroup->groups[gnum]->stmt[sgroup->groups[gnum]->size++] = stmt;
          put_info(dh->ped, stmt, type_fd, (Generic)sgroup->groups[gnum]);
          continue;
        }
        else
        {
          sgroup->group_num++;              /* end this group */
          gnum++;
        }
      }
      /* add statement to new group */
      sgroup->groups[gnum] = (S_group *) dc_get_mem(dh, sizeof(S_group));
      sgroup->groups[gnum]->size = 1;
      sgroup->groups[gnum]->stmt[0] = stmt;
      sgroup->groups[gnum]->iset = iset;
      put_info(dh->ped, stmt, type_fd, (Generic)sgroup->groups[gnum]);
    }
  }

 if (sgroup->groups[gnum]->size > 0)  /* group is active   */
   sgroup->group_num++;              /* end this group */
}


/*******************/
/* local functions */
/*******************/

/*--------------------------------------------------------------------

    iter_build_stmt()

    calculate entire iteration set for one statement

    build iteration set for stmt, then store in its info array

*/

static int
iter_build_stmt(AST_INDEX stmt, int level, Dist_Globals *dh)
{
  AST_INDEX lhs, rhs, loop, home_node;
  Iter_set *iset, *iset2;

  loop = dt_ast_loop(stmt);

  if (io_stmt(stmt))
  {
    iset = iter_build_io(dh, stmt, loop);
    put_info(dh->ped, stmt, type_dc, (Generic)iset);       /* store result */
  }
  else if (is_assignment(stmt))     
  {
    lhs = gen_ASSIGNMENT_get_lvalue(stmt);

    /*-------------------------------------------------------------*/
    /* Check for an enclosing ON_HOME directive                    */
    /*-------------------------------------------------------------*/

    /* 6/10/93 RvH: Currently, a "ON_HOME x(i)" directive works only if 
     * it encloses a loop that somewhere has a subscript "x(i)".
     * The strategy then is to retrieve <sinfo> from there, instead of
     * from the lhs of the current stmt.
     * get_home_node() resides in the irreg module for now.
     */

    home_node = dh->in_ded ? lhs : get_home_node(dh->irr, lhs);

    /* build iteration set based on type of computation */

    if ((home_node != lhs) || (loop == AST_NIL))
    {
      iset = iter_build_assign(dh, home_node, loop);
    }
    else
    {
      iset = iter_build_reduction(dh, lhs, loop);
      if (!iset)
      {
        rhs = gen_ASSIGNMENT_get_rvalue(stmt);
        iset = iter_build_private(dh, lhs, rhs, loop);
      }
      if (!iset)
        iset = iter_build_assign(dh, lhs, loop);
    }
    put_info(dh->ped, stmt, type_dc, (Generic)iset);       /* store result */

    /*  also store iset in all IF stmts guarding stmt */
    while (stmt != AST_NIL)
    {
      stmt = tree_out(stmt);
      if (is_if(stmt) || is_logical_if(stmt))
      {
        iset2 = (Iter_set *) get_info(dh->ped, stmt, type_dc);
        if (iset2 == (Iter_set *) NO_DC_INFO)
          put_info(dh->ped, stmt, type_dc, (Generic)iset);
        else if (is_diff_isets(iset, iset2, -1))
        {
          /* should form union of isets, just set to all for now */
          iset2 = dc_alloc_iter_set(dh);
          iset2->allproc = true;
          bcopy((const char *)get_info(dh->ped, loop, type_ref), 
                (char *)&iset2->set, sizeof(Loop_list)); 
          put_info(dh->ped, stmt, type_dc, (Generic)iset2);
        }
      }
    }
  }

  return WALK_CONTINUE;
}


/*--------------------------------------------------------------------

  iter_build_io()    Build iteration set for I/O statement

  For now just assume Processor 0 executes all I/O

*/

static Iter_set *
iter_build_io(Dist_Globals *dh, AST_INDEX stmt, AST_INDEX loop)
{
  Iter_set *iset;

  if (loop != AST_NIL)
    printf("iter_build_io(): I/O statement found in loop\n");

  iset = dc_alloc_iter_set(dh);
  iset->oneproc = true;
  return iset;
}


/*--------------------------------------------------------------------

  iter_build_private()    Build iteration set for assignment to private var

  Returns  NULL   if stmt is not an assignment to private var
           iset   if stmt is an assignment to private var
*/

static Iter_set *
iter_build_private(Dist_Globals *dh, AST_INDEX lhs, AST_INDEX rhs, AST_INDEX loop)
{
  Iter_set *iset;
  int sinks;
  AST_INDEX sink_stmt, old_stmt, sink_ref;
  Iter_set *sink_iters[TEMP_MAX];
  EDGE_INDEX edge;
  DG_Edge *Earray; /* array of all DG edges    */
  int ref;

  if (!is_private_var(PED_LI(dh->ped), lhs, loop))
    return NULL;

  sinks = 0;

  /* if private, then no loop-carried dependences &   */
  /* iteration set depends entirely on uses           */

  if ((ref = get_info(dh->ped, lhs, type_levelv)) == NO_LEVELV)
  {
   /* printf("iter_build_private(): missing uses\n"); */
  }
  else  /* look at all loop-independent true dependences with id as src */
  {
    Earray = dg_get_edge_structure(PED_DG(dh->ped));
    old_stmt = AST_NIL;

    for (edge = dg_first_src_ref(PED_DG(dh->ped), ref);
         edge >= 0;
         edge = dg_next_src_ref(PED_DG(dh->ped), edge))
    {
      if ((Earray[edge].level == LOOP_INDEPENDENT) &&
          (Earray[edge].type == dg_true))
      {
        sink_stmt = dt_ast_stmt(Earray[edge].sink);
        if (is_assignment(sink_stmt))
        {
          sink_ref = gen_ASSIGNMENT_get_lvalue(sink_stmt);

          if (sink_stmt == old_stmt)
            continue;

          old_stmt = sink_stmt;
          iset = (Iter_set *) get_info(dh->ped, sink_stmt, type_dc);
          if (iset != (Iter_set *) NO_DC_INFO)
            sink_iters[sinks++] = iset;
        }
      }
    }
  }

  /* if multiple uses are found, combine their iteration sets */

  if (!sinks)
  {
    /* printf("iter_build_private(): missing uses\n"); */
    iset = NULL;
  }
  else if (sinks == 1)
  {
    iset = dc_alloc_iter_set(dh);
    bcopy(sink_iters[0], iset, sizeof(Iter_set));
    iset->private_use = sink_ref;
  }
  else
  {
    /* just use the last one for now */
    iset = dc_alloc_iter_set(dh);
    bcopy(sink_iters[sinks - 1], iset, sizeof(Iter_set));
    iset->private_use = sink_ref;
    iset->lhs_private = true;   
  }

  return iset;
}


/*--------------------------------------------------------------------

    iter_build_block()

    calculate iteration set for one dimension of the lhs
    dimension is distributed with attribute = FD_BLOCK

    Works in four stages:

    1) get global iteration set from loop bounds
    2) translate into global index set using subscripts
    3) calculate local index sets using array decomposition info
    4) translate back into local iteration sets using inv subscripts

    Messy part is in computing various boundary conditions
    and idle processors, if any.

*/

static void
iter_build_block(Subs_list *sinfo, Loop_list *linfo, int dim, Iter_set *iset, 
                 Dist_Globals *dh, SNODE *sp)
{
  int lo, up, step;/* global iter set = loop iterations  */
  int lo2, up2, step2;  /* global index set = array locations */
  int lo3, up3;  /* local index set = array locations  */
  int lo4, up4;  /* local iter set = loop iterations   */

  int ivar, coeff, const_val;    /* subscript params */
  int array_lo, array_up;       /* array params     */
  int bksize1, bksize2;
  int diff, idle_proc, total_proc, proc, i;
  AST_INDEX outer;

  total_proc = sp_num_blocks(sp, dim);
  bksize1 = sp_block_size1(sp, dim);
  bksize2 = sp_block_size2(sp, dim);

  ivar = sinfo->subs[dim].stype;        /* find index var in subs dim */

  /*---------------------------------------------------*/
  /* mark loop in distributed dimension as partitioned */

  if (ivar == SUBS_ZIV) /* no index var found in loop, subs is const_val */
  {
    /* find out which processor needs to execute this loop */
    proc = (sinfo->subs[dim].constant - 1) / bksize1;

    /* find outermost perfectly nested loop */
    /* idle all other processors for this loop */

    outer = dc_outer_perf(iset->lhs);
    if (outer != AST_NIL)
    {
      i = loop_level(outer) - 1;

      iset->type[i] = Iter_mid_only;
      iset->pre_idle[i] = proc;
      iset->post_idle[i] = total_proc - proc - 1;

      iset->dim_num[i] = dim;  /* loop "i" now dependent on dimension "dim" */
    }

    iset->oneproc = true;
    iset->proc[0] = proc;

    return;
  }
  else if (ivar > SUBS_SIV)
  {
    dc_irreg_lhs(dh->irr, iset->lhs);  /* too complex, leave to irreg folks */
    return;
  }

  iset->bksize[ivar] = bksize1;  /* store size of blocked loop */

  /*--------------------------------------------------------------*/
  /* calculate RSD for dimension based on loop bounds & subscript */

  /* get some subscript info */

  coeff = sinfo->subs[dim].coeffs[ivar];
  const_val = sinfo->subs[dim].constant;

  /* start with global iteration set  */

  lo = linfo->loops[ivar].lo.val;
  up = linfo->loops[ivar].up.val;
  step = linfo->loops[ivar].step.val;

  /* translate to global index set */

  lo2 = (coeff * lo) + const_val;
  up2 = (coeff * up) + const_val;
  step2 = step * coeff;

  /*--------------------------------------------------------------*/
  /* compare RSD for dimension with scope of array dimension */

  sp = iset->lhs_sp;
  array_lo = sp_get_lower(sp, dim);
  array_up = sp_get_upper(sp, dim);

  if (lo2 < array_lo)   /* lo2 = global index  */
    lo2 = array_lo;

  if (up2 > array_up)   /* up2 = global index */
    up2 = array_up;

  /*********************************************************************/
  /* NOTE: I'm assuming that array & decomposition both start at 1 and */
  /* that both coeff * step are 1.  I want to graduate this decade :-) */
  /*********************************************************************/

  /* check whether only one processor will execute this statement */

  proc = (lo2 - 1) / bksize1;   /* processor that executes 1st iter */

  if (proc == (up2 - 1) / bksize1)
  {
    iset->type[ivar] = Iter_simple;

    /* calculate local index sets */

    lo3 = (lo2 - 1) % bksize1 + 1;
    up3 = (up2 - 1) % bksize1 + 1;

    /* convert to local iter sets */

    lo4 = (lo3 - const_val) / coeff;
    up4 = (up3 - const_val) / coeff;

    iset->set.loops[ivar].lo.val = lo4;
    iset->set.loops[ivar].up.val = up4;
    iset->pre_idle[ivar] = proc;
    iset->post_idle[ivar] = total_proc - proc - 1;

    iset->oneproc = true;
    iset->proc[0] = proc;

    return;
  }

  /*---------------------------------------------*/
  /* check whether pre boundary condition exists */

  if (diff = lo2 - array_lo)    /* lower bounds of array & norm loop differ */
  {
    iset->pre_idle[ivar] = diff / bksize1;      /* these procs do nothing */

    if (diff = diff % bksize1)  /* diff not MOD of local block size */
    {
      iset->type[ivar] = Iter_pre;      /* pre boundary condition */

      /* calculate local index sets */

      lo3 = array_lo + diff;
      up3 = array_lo + bksize1 - 1;

      /* convert to local iter sets */

      lo4 = (lo3 - const_val) / coeff;
      up4 = (up3 - const_val) / coeff;

      iset->pre_set.loops[ivar].lo.val = lo4;
      iset->pre_set.loops[ivar].up.val = up4;
    }

  }

  /*----------------------------------------------*/
  /* check whether post boundary condition exists */

  diff = array_up - up2;        /* diff between loop & array bounds */

  if (diff >= bksize2)  /* if last proc will be idle */
  {
    /* find which procs do nothing */

    diff -= bksize2;
    iset->post_idle[ivar] = diff / bksize1 + 1;

    /* diff not MOD of local block size */

    if (diff = diff % bksize1)
    {
      /* post iter set needed */

      if (iset->type[ivar] == Iter_pre)
        iset->type[ivar] = Iter_all;
      else
        iset->type[ivar] = Iter_post;

      /* calculate local index sets */

      lo3 = array_lo;
      up3 = array_lo + (bksize1 - diff) - 1;

      /* convert to local iter sets */

      lo4 = (lo3 - const_val) / coeff;
      up4 = (up3 - const_val) / coeff;

      iset->post_set.loops[ivar].lo.val = lo4;
      iset->post_set.loops[ivar].up.val = up4;
    }
  }
  /* last proc will not be idle   */
  else if (diff || (bksize1 != bksize2))
  {
    /* we know that post iter set must be needed */

    if (iset->type[ivar] == Iter_pre)
      iset->type[ivar] = Iter_all;
    else
      iset->type[ivar] = Iter_post;

    /* calculate local index sets */

    lo3 = array_lo;
    up3 = array_lo + (bksize2 - diff) - 1;

    /* convert to local iter sets */

    lo4 = (lo3 - const_val) / coeff;
    up4 = (up3 - const_val) / coeff;

    iset->post_set.loops[ivar].lo.val = lo4;
    iset->post_set.loops[ivar].up.val = up4;
  }

  /*------------------------------------------------*/
  /* check whether interior iteration set exists    */

  /* calc number of total & idle procs in dimension */

  idle_proc = iset->pre_idle[ivar] + iset->post_idle[ivar];

  switch (iset->type[ivar])
  {
    case Iter_pre:
      idle_proc++; /* including pre iter set */

      if (idle_proc > total_proc)
      {
        die_with_message("iter_build_block(): too many procs!");
      }
      else if (idle_proc == total_proc)
      {
        iset->type[ivar] = Iter_pre_only;
        return;
      }
      break;

    case Iter_post:
      idle_proc++; /* including post iter set */

      if (idle_proc > total_proc)
      {
        die_with_message("iter_build_block(): too many procs!");
      }
      else if (idle_proc == total_proc)
      {
        /* convert sole post set to pre set */

        iset->type[ivar] = Iter_pre_only;
        iset->pre_set.loops[ivar].lo.val =
          iset->post_set.loops[ivar].lo.val;

        iset->pre_set.loops[ivar].up.val =
          iset->post_set.loops[ivar].up.val;

        return;
      }
      break;

    case Iter_all:
      idle_proc++; /* including pre & post iter set */

      if (idle_proc > total_proc)
      {
        die_with_message("iter_build_block(): too many procs!");
      }
      else if (idle_proc == total_proc)
      {
/*        iset->type[ivar] = Iter_pre_post; */
        iset->type[ivar] = Iter_post_only;
        return;
      }
      break;
  }

  /*------------------------------------------------------*/
  /* if we reach here, then interior iteration set exists */

  /*   if (!idle_proc)           */
  /*     iset->allproc = true;   */

  /* calculate local index sets */

  lo3 = array_lo;
  up3 = array_lo + bksize1 - 1;

  /* convert to local iter sets */

  lo4 = (lo3 - const_val) / coeff;
  up4 = (up3 - const_val) / coeff;

  iset->set.loops[ivar].lo.val = lo4;
  iset->set.loops[ivar].up.val = up4;

}

/*--------------------------------------------------------------------

    iter_build_cyclic()

    calculate iteration set for one dimension of the lhs
    dimension is distributed with attribute = CYCLIC

*/

static void
iter_build_cyclic(Subs_list *sinfo, Loop_list *linfo, int dim, Iter_set *iset, 
                  Dist_Globals *dh, SNODE *sp)
{
  int ivar, total_proc, proc, i, bound, const_val;
  AST_INDEX outer;
  Citer_type ctype;

  total_proc = sp_num_blocks(sp, dim);

  ivar = sinfo->subs[dim].stype;        /* find index var in subs dim */

  /*---------------------------------------------------*/
  /* mark loop in distributed dimension as partitioned */

  if (ivar == SUBS_ZIV) /* no index var found in loop, subs is const_val */
  {
    /* find out which processor needs to execute this loop */
    proc = (sinfo->subs[dim].constant - 1) % total_proc;

    /* find outermost perfectly nested loop */
    /* idle all other processors for this loop */

    outer = dc_outer_perf(iset->lhs);
    if (outer != AST_NIL)
    {
      i = loop_level(outer) - 1;

      iset->type[i] = Iter_mid_only;
      iset->pre_idle[i] = proc;
      iset->post_idle[i] = total_proc - proc - 1;

      iset->dim_num[i] = dim;  /* loop "i" now dependent on dimension "dim" */
    }

    iset->oneproc = true;
    iset->proc[0] = proc;

    return;
  }
  else if (ivar > SUBS_SIV)
  {
    dc_irreg_lhs(dh->irr, iset->lhs);  /* too complex, leave to irreg folks */
    return;
  }

  /* iset->allproc = true; */  /* assume # iterations > # procs */

  iset->bksize[ivar] = sp_block_size1(sp, dim);  /* size of blocked loop */

  /*--------------------------------------------------------------*/
  /* calculate iteration set based on loop bounds & subscript */

  /* get some subscript info */

  if (sinfo->subs[dim].coeffs[ivar] != 1)  /* not yet supported */
  {
    printf("iter_build_cyclic(): Non-unit subscript coefficients\n");
    return;
  }

  if (linfo->loops[ivar].step.val != 1)  /* not yet supported */
  {
    printf("iter_build_cyclic(): Non-unit step\n");
    return;
  }

  const_val = sinfo->subs[dim].constant;

  ctype = Citer_all;  /* conservatively assume both boundary conditions */

  if (linfo->loops[ivar].lo.type == Expr_constant)
  {
    bound = linfo->loops[ivar].lo.val;

    /* no boundary condition on lower bound */
    if (((bound + const_val - 1) % total_proc) == 0)
    {
      iset->set.loops[ivar].lo.val = ((bound-1)/total_proc)+1;
      ctype = Citer_up;
    }
  }

  if (linfo->loops[ivar].up.type == Expr_constant)
  {
    bound = linfo->loops[ivar].up.val;

    /* no boundary condition on upper bound */
    if (((bound + const_val) % total_proc) == 0)
    {
      iset->set.loops[ivar].up.val = bound/total_proc;
      ctype = (ctype == Citer_up) ? Citer_simple : Citer_lo;
    }
  }

  iset->cyclic[ivar] = ctype;
}


/*--------------------------------------------------------------------

    iter_build_bkcyclic()

    calculate iteration set for one dimension of the lhs
    dimension is distributed with attribute = BLOCK_CYCLIC

*/

static void
iter_build_bkcyclic(Subs_list *sinfo, Loop_list *linfo, int dim, Iter_set *iset, 
                    Dist_Globals *dh)
{
  /* not yet supported */
}


/*--------------------------------------------------------------------

    iter_build_user()

    calculate iteration set for one dimension of the lhs
    12/3/93 RvH: For now, assume value-based decomposition

*/

static void
iter_build_user(Subs_list    *sinfo,
		Loop_list    *linfo,
		int           dim,
		Iter_set     *iset, 
		Dist_Globals *dh,
		SNODE        *sp)
{
  int ivar,  total_proc, proc, i, bound, const_val;
  AST_INDEX  outer;
  Uiter_type utype;

  ivar = sinfo->subs[dim].stype;        /* find index var in subs dim */


  /* iset->allproc = true; */  /* assume # iterations > # procs */

  iset->bksize[ivar] = sp_block_size1(sp, dim);  /* size of blocked loop */

  /*--------------------------------------------------------------*/
  /* calculate iteration set based on loop bounds & subscript */

  /* get some subscript info */

  if (sinfo->subs[dim].coeffs[ivar] != 1)  /* not yet supported */
  {
    printf("iter_build_user(): Non-unit subscript coefficients\n");
    return;
  }

  if (linfo->loops[ivar].step.val != 1)  /* not yet supported */
  {
    printf("iter_build_user(): Non-unit step\n");
    return;
  }

  const_val = sinfo->subs[dim].constant;

  if ((linfo->loops[ivar].lo.type != Expr_constant)
      || (linfo->loops[ivar].lo.val != 1)
      || (linfo->loops[ivar].up.type == Expr_constant))
  {
    printf("iter_build_user(): Sorry, need normal form loop!\n");
    return;
  }
  
  /* 12/3/93 RvH: Grab symbolic upper loop bound */
  iset->set.loops[ivar].up.type = Expr_simple_sym;
  iset->set.loops[ivar].up.str  =
    (char*) ValDecomp_get_upper_bound(dh->irr, sp);
  
  /* 12/8/93 RvH: Only value-based for now */
  utype                  = Uiter_value;
  iset->irr_decomp[ivar] = ValDecomp_get_hidden_vd(dh->irr, sp);

  iset->user[ivar] = utype;
}


/*------------------------------------------------------------------

 is_diff_loops()   Check whether two loops are different

*/

static Boolean
is_diff_loops(Loop_data *loop1, Loop_data *loop2)
{

  if (is_diff_expr(&loop1->lo, &loop2->lo) ||
      is_diff_expr(&loop1->up, &loop2->up) ||
      is_diff_expr(&loop1->step, &loop2->step))
    return true;

  return false;
}


/*--------------------------------------------------------------------

  is_diff_expr()    Compare two Expr structures

*/

static Boolean
is_diff_expr(Expr *expr1, Expr *expr2)
{
  if (expr1->type != expr2->type)
    return true;

  if (expr1->type == Expr_constant)
    return (expr1->val == expr2->val) ? false : true;

  return ast_equiv(expr1->ast, expr2->ast) ? false : true;
}


/*--------------------------------------------------------------------

    find_stmt_groups()   find & group statements not in loops

*/

static int 
find_stmt_groups(AST_INDEX stmt, int level, Dist_Globals *dh)
{
  AST_INDEX stmt_list;

  switch (gen_get_node_type(stmt))
  {
    case GEN_GLOBAL:
      stmt_list = gen_GLOBAL_get_subprogram_scope_LIST(stmt);
      break;

    case GEN_FUNCTION:
      stmt_list = gen_FUNCTION_get_stmt_LIST(stmt);
      break;

    case GEN_PROGRAM:
      stmt_list = gen_PROGRAM_get_stmt_LIST(stmt);
      break;

    case GEN_SUBROUTINE:
      stmt_list = gen_SUBROUTINE_get_stmt_LIST(stmt);
      break;

    default:
      return WALK_CONTINUE;
  }


  dc_find_stmt_groups(dh, stmt_list, &dh->sgroup, 
                        dh->sgroup.group_num, -1, true); 

  return WALK_SKIP_CHILDREN;
}

/*--------------------------------------------------------------------

    collect_exec_stmts()   find executable statements

*/

static int 
collect_exec_stmts(AST_INDEX stmt, int level, AST_INDEX **execs)
{
  if (is_if(stmt) || is_logical_if(stmt) || executable_stmt(stmt))
  {
    **execs = stmt;
    (*execs)++;
  }

  return WALK_CONTINUE;
}


/* eof */
