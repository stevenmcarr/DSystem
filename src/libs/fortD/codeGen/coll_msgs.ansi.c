/* $Id: coll_msgs.ansi.c,v 1.26 1997/03/11 14:28:14 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/



/*---------------------------------------------------------------------

    coll_msgs.c    Generates code for performing broadcasts,
                   individual send/recv, and reductions 

    Author : Seema Hiranandani 

*/

#include <libs/fortD/codeGen/private_dc.h>

/* #include <libs/fortD/performance/staticInfo/MesgStaticSDDF.h>  For SDDF instr. */

/*-------------------external declarations-------------------*/

/* The above include causes weird compilation problems so import */
/* declarations of the next two functions directly */

EXTERN(void, GetMessageParams,(Dist_Globals *dh,
			       AST_INDEX loop,	/* associated loop */
			       Rsd_set *rset));	/* RSD(s) sent in msg */
EXTERN(void,
   GetMessageSendParamsForReduc, (Dist_Globals* dh,
				  Reduc_set* reducSet,  /* info re data sent */
				  AST_INDEX reducStmt));/* reduction call */

EXTERN(void, init_msg_info,(Dist_Globals *dh));
EXTERN(void, compute_proc_offsets,(Dist_Globals *dh, Rsd_set *rset, 
                                   Rsd_section *rsd_sect));
EXTERN(int, *compute_send_data,(Dist_Globals *dh, Rsd_set *rset));
EXTERN(void, compute_recv_data,(Dist_Globals *dh, Rsd_set *rset, Iter_type msg_type,
                                AST_INDEX loop));
EXTERN(void, insert_indep,(Dist_Globals *dj, AST_INDEX loop, Iter_type msg_type, 
                            Rsd_set *rset));
EXTERN(AST_INDEX, ast_get_sub,(AST_INDEX subs, int dim));
EXTERN(AST_INDEX, ast_merge_ifs,(AST_INDEX if1));
EXTERN(AST_INDEX, broadcast_scalar_recv,(Dist_Globals *dh, AST_INDEX *aux));
EXTERN(AST_INDEX, broadcast_scalar_send,(Dist_Globals *dh, AST_INDEX *aux));
EXTERN(int, *dc_alloc_msgbuf,(Dist_Globals *dh, enum FORM form, Boolean zero));

/*-------------------global declarations-------------------*/

void dc_collective_msgs(Dist_Globals *dh, AST_INDEX loop, Rsd_set *rset);
void dc_reduction_msgs(Dist_Globals *dh, AST_INDEX loop, int level);
void dc_send_recv_msgs(Dist_Globals *dh, AST_INDEX loop, Rsd_set *rset);
AST_INDEX make_sendproc(Dist_Globals *dh, Rsd_set *rset);
int dc_array_size(SNODE *sp);

/*------------------local declarations---------------------*/

STATIC(AST_INDEX, make_reduc_call,(Dist_Globals *dh, Iter_set *iset));
STATIC(AST_INDEX, make_init_assignment,(Dist_Globals *dh, Iter_set *iset));
STATIC(AST_INDEX, make_reduc_assignment,(Dist_Globals *dh, Iter_set *iset));
/*STATIC(void, replace_reduction_var,());*/
STATIC(void, dc_broadcast_msgs,(Dist_Globals *dh, AST_INDEX loop, Rsd_set *rset));
STATIC(void, insert_broadcast,(Dist_Globals *dh, AST_INDEX loop, Rsd_set *rset));
STATIC(int, check_reduction,(AST_INDEX stmt, int level, Dist_Globals *dh));
STATIC(int, replace_var,(AST_INDEX expr, char *lhs));
STATIC(int, var2buf,(Dist_Globals *dh, Rsd_set *rset, int idx));

/*******************/
/* Global Routines */
/*******************/

/*--------------------------------------------------------------------

    dc_collective_msgs()  driver routine for collective communications

*/

void
dc_collective_msgs(Dist_Globals *dh, AST_INDEX loop, Rsd_set *rset)
{

  switch (rset->ctype)
  {
    case FD_COMM_BCAST:
      dc_broadcast_msgs(dh, loop, rset);
      break;

    case FD_COMM_TRANSPOSE:
      printf("F77D: Transpose not yet implemented\n");
      break;

    case FD_COMM_REDUCE:
      printf("F77D: Gather not yet implemented\n");
      break;
  }
}

/*--------------------------------------------------------------------

    dc_reduction_msgs()  driver routine for communicating reductions

*/

void
dc_reduction_msgs(Dist_Globals *dh, AST_INDEX loop, int level)
{
  walk_statements(loop, level, (WK_STMT_CLBACK)check_reduction, NULL, (Generic)dh);
}

/*--------------------------------------------------------------------

  make_sendproc()  Determine processor performing send/broadcast

*/

AST_INDEX
make_sendproc(Dist_Globals *dh, Rsd_set *rset)
{
  int i, j, k, bksize1, total_proc;
  Subs_data *rsub;
  AST_INDEX sender, sub;
  Dist_type dtype;

  /* find subscript in distributed dimension */

  dtype = FD_DIST_LOCAL;
  for (i = 0; i < rset->sp->numdim; i++)
  {
    dtype = sp_is_part(rset->sp, i);
    if (dtype != FD_DIST_LOCAL)
      break;
  }

  for (j = 0; j < rset->sp->numdim; j++)
  {
    if (sp_ddim(rset->sp, j) == i + 1)
    {
      rsub = rset->sinfo[0]->subs + j;
      break;
    }
  }

  /* calculate owner (proc doing send) */

  switch (dtype)
  {
  case FD_DIST_LOCAL:
    die_with_message("make_sendproc(): array not distributed");

  case FD_DIST_BLOCK:
    bksize1 = sp_block_size1(rset->sp, i);
    if (rsub->stype == SUBS_ZIV)
    {
      sender = pt_gen_int((rsub->constant - 1) / bksize1);
    }
    else 
    {
      if (rsub->stype < SUBS_SIV)
      {
        sub = pt_gen_ident(rset->iterset->set.loops[rsub->stype].ivar);
        if (rsub->coeffs[rsub->stype] != 1)
          sub = pt_gen_mul(pt_gen_int(rsub->coeffs[rsub->stype]), sub);
        if (rsub->constant > 1)
          sub = pt_gen_add(sub, pt_gen_int(rsub->constant-1));
        else if (rsub->constant < 1)
          sub = pt_gen_sub(sub, pt_gen_int(1-rsub->constant));
      }
      else    /* give up, grab subscript from original AST */
      {
        sub = ast_get_sub(rset->subs[0], i); /* seema */
        sub = pt_gen_sub(tree_copy(sub), pt_gen_int(1));
      }
      gen_put_parens(sub, 1);
      sender = gen_BINARY_DIVIDE(sub, pt_gen_int(bksize1));
      gen_put_parens(sender, 1);
    }
    break;

  case FD_DIST_CYCLIC:
    total_proc = sp_num_blocks(rset->sp, i);
    if (rsub->stype == SUBS_ZIV)
    {
      sender = pt_gen_int((rsub->constant - 1) % total_proc);
    }
    else 
    {
      if (rsub->stype < SUBS_SIV)
      {
        sub = pt_gen_ident(rset->iterset->set.loops[rsub->stype].ivar);
        if (rsub->coeffs[rsub->stype] != 1)
          sub = pt_gen_mul(pt_gen_int(rsub->coeffs[rsub->stype]), sub);
        if (rsub->constant > 1)
          sub = pt_gen_add(sub, pt_gen_int(rsub->constant-1));
        else if (rsub->constant < 1)
          sub = pt_gen_sub(sub, pt_gen_int(1-rsub->constant));
      }
      else   /* give up, grab subscript from original AST */
      {
        sub = ast_get_sub(rset->subs[0], i); /* seema */
        sub = pt_gen_sub(tree_copy(sub), pt_gen_int(1));
      }
      sender = list_create(sub);
      sender = list_insert_last(sender, pt_gen_int(total_proc));
      sender = pt_gen_invoke("MOD", sender);
    }
    break;

  default:
    die_with_message("make_sendproc(): distribution not yet supported");
  }

/*  sender = gen_BINARY_EQ(ast_get_logical_myproc(dh), sender); */
  return sender;
}


/*--------------------------------------------------------------------

  array_size()   Find size of array, including overlaps

*/

int
dc_array_size(SNODE *sp)
{
  int i, size;

  size = 1;
  for (i = 0; i < sp->numdim; i++)
     size *= sp_get_upper(sp, i) - sp_get_lower(sp, i) + 1;

  return size;
}


/******************/
/* Local Routines */
/******************/


/*--------------------------------------------------------------------

    dc_broadcast_msgs()   Insert broadcast messages

    Assume only 1D distribution for now

*/

static void
dc_broadcast_msgs(Dist_Globals *dh, AST_INDEX loop, Rsd_set *rset)
{
  int idx, *bufsize_p;
  Rsd_set *saveRset = rset;	/* 6/19/94, VSA: need *rset to compute SDDF*/

  init_msg_info(dh);
  dh->rs_parts[dh->proc][0].total_procs = 1;    /* only 1 message */
  /* proc doing bcast (VA: logical proc) */
  dh->bcast =  gen_BINARY_EQ(ast_get_logical_myproc(dh),make_sendproc(dh, rset));

  compute_proc_offsets(dh, rset,
                       rset->rs_carried ? rset->rs_carried : rset->rs);

  bufsize_p = compute_send_data(dh, rset);
  compute_recv_data(dh, rset, Iter_simple, AST_NIL);
  insert_broadcast(dh, loop, rset);

  /* modify references  */

  idx = 1;
  while (rset)
  {
    idx += var2buf(dh, rset, idx);
    rset = rset->rsd_merge;
  }

  if (idx > *bufsize_p)     /* update buffer size */ 
    *bufsize_p = idx-1;

  GetMessageParams(dh, loop, saveRset); /* 6/17/94, VSA: make static SDDF */

  /* clear some stmts  */

  dh->send_stmts = AST_NIL;
  dh->recv_stmts = AST_NIL;
  dh->buftype = AST_NIL;
  dh->bufpoint = AST_NIL;

}

/*--------------------------------------------------------------------

    dc_send_recv_msgs()   Insert individual send & recv message

*/

void
dc_send_recv_msgs(Dist_Globals *dh, AST_INDEX loop, Rsd_set *rset)
{
  int idx, *bufsize_p;
  Rsd_set *saveRset = rset;	/* 6/19/94, VSA: need *rset to compute SDDF*/

  init_msg_info(dh);

  dh->rs_parts[dh->proc][0].total_procs = 1;    /* only 1 message */
   /* proc doing send */
  dh->bcast =  gen_BINARY_EQ(ast_get_logical_myproc(dh),make_sendproc(dh, rset));

  compute_proc_offsets(dh, rset,
                       rset->rs_carried ? rset->rs_carried : rset->rs);

  bufsize_p = compute_send_data(dh, rset);
  compute_recv_data(dh, rset, Iter_simple, loop);
  insert_indep(dh, loop, Iter_simple, rset);

  /* modify references  */

  idx = 1;
  while (rset)
  {
    idx += var2buf(dh, rset, idx);
    rset = rset->rsd_merge;
  }

  if (idx > *bufsize_p)     /* update buffer size */
    *bufsize_p = idx-1;

  /* 6/17/94, VSA: Added to generate SDDF record */
  GetMessageParams(dh, loop, saveRset);

  /* clear some stmts  */

  dh->send_stmts = AST_NIL;
  dh->recv_stmts = AST_NIL;
  dh->buftype = AST_NIL;
  dh->bufpoint = AST_NIL;

}


/*--------------------------------------------------------------------

  var2buf()    Convert variable refs into buffer refs

  Returns:  Number of buffer elements accessed by rset.

  For now, assume only vector broadcast

*/

static int
var2buf(Dist_Globals *dh, Rsd_set *rset, int idx)
{
  Rsd_section *r_sec;
  int i, j, dim, level, offset, rsd_size, lower_bd, upper_bd, start;
  AST_INDEX node, addr;
  Loop_data *ldata;

  /* 0 = size, 1 = lower bound, 2 = upper bound, 3 = prod of prev sizes */
  int bounds[MAXLOOP][4];

  r_sec = rset->rs_carried ? rset->rs_carried : rset->rs;

  dim = 0;
  rsd_size = 1;

  for (i = 0; i < r_sec->dims; i++)
  {
    switch (r_sec->subs[i].type)
    {
      case RSD_CONSTANT:        /* single constant  */
      case RSD_EXPR:    /* single index var */
        bounds[i][0] = 1;
        break;

      case RSD_NO_DATA: /* unknown RSD      */
      case RSD_EXPR_RANGE:      /* index var range  */
      case RSD_BOTTOM:  /* bottom           */
        printf("var2buf(): Unknown RSD type\n");
        /* fall through for now */

      case RSD_RANGE:   /* constant range   */
        lower_bd = dc_rsd_lower(r_sec, i);
        upper_bd = dc_rsd_upper(r_sec, i);

        if (lower_bd == MININT)    /* symbolic lower bound */
        {
          /* just use array size for now */
          lower_bd = sp_get_lower(rset->sp,i);
        }

        if (upper_bd == MAXINT)    /* symbolic upper bound */
        {
          /* just use array size for now */
          upper_bd = sp_get_upper(rset->sp,i);
        }

        bounds[i][1] = lower_bd;
        bounds[i][2] = upper_bd;
        bounds[i][0] = upper_bd - lower_bd + 1;
        rsd_size *= bounds[i][0];
        dim++;

        /* if r_sec has been shifted into local indices,    */
        /* must convert them back to global indices         */

        if (rset->ctype == FD_COMM_SEND_RECV)
        {
          if (sp_is_part(rset->sp, i) != FD_DIST_LOCAL)
          {
            offset = rset->iterset->proc[0]*sp_block_size1(rset->sp,i);
            bounds[i][1] += offset;
            bounds[i][2] += offset;
          }
        }

        break;
    }

    bounds[i][3] = i ? bounds[i - 1][3] * bounds[i - 1][0] : 1;
  }

  /* if element message, only one possibility for buffer */
  if (!dim)
  {
    node = list_create(pt_gen_int(idx));
    node = gen_SUBSCRIPT(tree_copy(dh->bufpoint), node);

    for (i = 0; i < rset->num_subs; i++)
      tree_replace(rset->subs[i], tree_copy(node));

    return rsd_size;
  }

  /* else loop thru all actual references causing nonlocal accesses & */
  /* try to determine what part of RSD is accessed by the reference */
  for (i = 0; i < rset->num_subs; i++)
  {
    addr = AST_NIL;
    start = idx;

    /* loop thru each subscript in reference */
    for (j = 0; j < r_sec->dims; j++)
    {
      if (bounds[j][0] > 1)
      {
        level = rset->sinfo[i]->subs[j].stype;

        /* if constant, calculate offset directly */
        if (level == SUBS_ZIV)
        {
          if (offset = rset->sinfo[i]->subs[j].constant - bounds[j][1])
            start += offset * bounds[j][3];
        }

        /* if index var, must check loop bounds when calculating offset */
        else
        {
          ldata = rset->iterset->set.loops + level;
          node = pt_gen_ident(ldata->ivar);

          if (ldata->lo.type == Expr_constant)
          {
            lower_bd = ldata->lo.val;
            lower_bd += rset->sinfo[i]->subs[j].constant;

            if (offset = lower_bd - bounds[j][1])
              start += offset * bounds[j][3];

            /* some offset is necessary when loop lower bound is not 0 */
            start -= bounds[j][1] * bounds[j][3];
          }
          else
          {
            node = pt_gen_sub(node,tree_copy(ldata->lo.ast));
            gen_put_parens(node,1);
          }

          if (bounds[j][3] > 1)
          {
            node = pt_gen_mul(node, pt_gen_int(bounds[j][3]));
            gen_put_parens(node, 1);
          }
          addr = (addr == AST_NIL) ? node : pt_gen_add(node, addr);
        }
      }
    }

    if (addr == AST_NIL)
      addr = pt_gen_int(start);
    else if (start > 0)
      addr = pt_gen_add(addr, pt_gen_int(start));
    else if (start < 0)
      addr = pt_gen_sub(addr, pt_gen_int(-start));

    addr = pt_simplify_expr(addr);
    addr = gen_SUBSCRIPT(tree_copy(dh->bufpoint), list_create(addr));
    tree_replace(rset->subs[i], addr);
  }

  return rsd_size;
}


/*--------------------------------------------------------------------

  insert_broadcast()  Insert broadcast

*/

static void
insert_broadcast(Dist_Globals *dh, AST_INDEX loop, Rsd_set *rset)
{
  AST_INDEX node, node2, stmts;
  FortD_LI *fli;

  node = dh->bcast;
  while ((node != AST_NIL) && !is_guard(node))
    node = tree_out(node);

  node2 = gen_GUARD(AST_NIL, AST_NIL, dh->recv_stmts);
  (void) list_insert_after(node, node2);

  if (rset->mtype == FD_MESG_INDEP)
  {
    (void) list_insert_before(loop, dh->send_stmts);

    node = tree_out(node);       /* get to IF from guard */
    node2 = ast_merge_ifs(node);  /* merge with preceding IF if legal */

    if (dh->send_sync != AST_NIL)
    {
      node = dc_ins_send_sync(dh, rset, node2);
      (void) list_insert_after(node, dh->send_sync);
    }

    if (dh->recv_sync != AST_NIL)
    {
      node = dc_ins_recv_sync(dh, rset, node2);
      (void) list_insert_before(node, dh->recv_sync);
    }
  }
  else if (rset->mtype == FD_MESG_CARRIED_ALL)
  {
    fli = (FortD_LI *) get_info(dh->ped, loop, type_fd);
    stmts = (fli->init == AST_NIL) ?
      list_first(gen_DO_get_stmt_LIST(loop)) : list_next(fli->init);

    (void) list_insert_before(stmts, dh->send_stmts);

    node = tree_out(node);       /* get to IF from guard */
    node = ast_merge_ifs(node);  /* merge with preceding IF if legal */

    if (dh->recv_sync != AST_NIL)
    {
      if (dh->head_recv == AST_NIL)
        dh->head_recv = dh->recv_sync;
      else
        (void) list_insert_last(dh->head_recv, dh->recv_sync);
    }

    /* update location of first stmt in loop */
    dh->loop_first = list_first(gen_DO_get_stmt_LIST(loop));

    /* place synchronization for unbuffered messages */

    if (dh->send_sync != AST_NIL)
    {
      node = dc_ins_send_sync(dh, rset, dh->loop_first);
      (void) list_insert_after(node, dh->send_sync);
    }
  }
}

/*--------------------------------------------------------------------

  update_owners()      Use broadcast to update results after 
                       relaxing "owner computes" rule 

*/

static void
update_owners(Dist_Globals *dh, AST_INDEX stmt, AST_INDEX *aux)
{
  S_group *sgroup;
  AST_INDEX guard, slist, node;

  sgroup = (S_group *) get_info(dh->ped, stmt, type_fd);

  if ((sgroup->guard == AST_NIL) || !is_if(sgroup->guard))
  {
    printf("update_owners(): missing guard\n");
    return;
  }

  guard = gen_IF_get_guard_LIST(sgroup->guard);
  guard = list_first(guard);
  slist = gen_GUARD_get_stmt_LIST(guard);
  node = broadcast_scalar_send(dh, aux);
  list_insert_last(slist, node);

  node = broadcast_scalar_recv(dh, aux);
  if (list_next(guard) == AST_NIL)
  {
    node = gen_GUARD(AST_NIL, AST_NIL, node);
    list_insert_after(guard, node);
  }
  else
  {
    guard = list_next(guard);
    slist = gen_GUARD_get_stmt_LIST(guard);
    list_insert_last(slist, node);
  }
}


/*****************************************************************/
/* called by dc_reduction_msgs---look for reductions             */
/*****************************************************************/
static int
check_reduction(AST_INDEX stmt, int level, Dist_Globals *dh)
{
  Iter_set *iset;
  AST_INDEX reduc_call = AST_NIL;
  AST_INDEX loop, init_stmt, final_stmt;
  char buf[MAX_NAME];
  SNODE *sp;

  if (!is_assignment(stmt))
    return WALK_CONTINUE;

  iset = (Iter_set *) get_info(dh->ped, stmt, type_dc);
  if (iset == (Iter_set *) NO_DC_INFO)  /* no info for stmt */
    return WALK_CONTINUE;

  /* get the iteration set and check if it corresponds to a reduction */

  if (!iset->reduc_set || iset->reduc_set->done)
    return WALK_CONTINUE;

  iset->reduc_set->done = true;

  if (iset->reduc_set->local)
  {
     update_owners(dh, stmt, iset->reduc_set->aux);
  }
  else
  {
    /* generate a call to the appropriate reduction routine */
    reduc_call = make_reduc_call(dh, iset);

    /* generate initialization and final assignment if needed */
    loop = iset->reduc_set->loop;

    if (!iset->reduc_set->init)
    {
      /* add decl for new var */
      sp = findadd2(iset->reduc_set->lhs, 0, 0, dh);
      sprintf(buf, "%s$", iset->reduc_set->lhs_name);
      dc_new_var(dh, pt_gen_ident(buf), sp->fform);

      /* replace all instances of the reduction var id with id$  */
      walk_expression(stmt, NULL, (WK_EXPR_CLBACK)replace_var,
                      (Generic)iset->reduc_set->lhs_name);

      /* generate an initialization */
      init_stmt = make_init_assignment(dh, iset);
      list_insert_before(loop, init_stmt);

      /* generate a final assignment */
      final_stmt = make_reduc_assignment(dh, iset);

      list_insert_after(loop, final_stmt);
    }

    /* insert the call after the loop */
    list_insert_after(loop, reduc_call);
  }

  GetMessageSendParamsForReduc(dh, iset->reduc_set, reduc_call);
  GetMessageParams(dh, loop, (Rsd_set *) NULL);
  
  return WALK_CONTINUE;
}


/*--------------------------------------------------------------------

    make_reduc_call()  generate call to the appropriate reduction routine

    FD_NX:    call GSSUM(var, elem, buf)

    FD_CMMD:  var = cmmd_reduce_double(var, cmmd_combiner_dadd)
              call cmmd_reduce_double_v(src, dest, combiner, stride, elem)
*/

static AST_INDEX
make_reduc_call(Dist_Globals *dh, Iter_set *iset)
{
  SNODE *sp;
  Reduc_type reduc_type;
  char buf[MAX_NAME], *reduc_suffix;
  AST_INDEX node, var, bufname, stride, combiner;
  int size;
  Buf_info *bptr;

  /*--------------------------------------------------------------*/
  /* generate name & number of elements of variable to be reduced */

  sp = findadd2(iset->reduc_set->lhs, 0, 0, dh);

  if (is_subscript(iset->reduc_set->lhs))
  {
    size = dc_array_size(sp);
    var = tree_copy(gen_SUBSCRIPT_get_name(iset->reduc_set->lhs));
    iset->reduc_set->init = true;  /* assume no initializations needed */
  }
  else
  {
    /* if an initialization is found then id = id else id = id$ */
    size = 1;
    strcpy(buf, iset->reduc_set->lhs_name);
    if (!iset->reduc_set->init)
      strcat(buf, "$");
    var = pt_gen_ident(buf);
  }

  /*----------------------------------------------------*/
  /* generate the call to the correct reduction routine */

  switch (dh->arch)
  {
    case FD_NX:

      switch (iset->reduc_set->rtype)
      {
        case FD_REDUC_TIMES:
          reduc_suffix = "PROD";
          break;
        case FD_REDUC_PLUS:
          reduc_suffix = "SUM";
          break;
        case FD_REDUC_MIN:
          reduc_suffix = "LOW";
          break;
        case FD_REDUC_MAX:
          reduc_suffix = "HIGH";
          break;
        default:
          die_with_message("Reduction type not yet handled \n");
          break;
      }

      switch (sp->fform)
      {
        case INTTYPE:
          strcpy(buf, "GI");
          strcat(buf, reduc_suffix);
          bufname = pt_gen_ident(dh->bufs.i.name);
          bptr = &dh->bufs.i;
          break;

        case REAL:
          strcpy(buf, "GS");
          strcat(buf, reduc_suffix);
          bufname = pt_gen_ident(dh->bufs.r.name);
          bptr = &dh->bufs.r;
          break;

        case DOUBLE_P:
          strcpy(buf, "GD");
          strcat(buf, reduc_suffix);
          bufname = pt_gen_ident(dh->bufs.dp.name);
          bptr = &dh->bufs.dp;
          break;

        default:
          die_with_message("Reduction type not handled \n");
      }

      /* mark buffer used & its size */

      bptr->active[0] = true;
      if (bptr->size[0] < size)
        bptr->size[0] = size;

      node = list_create(var);
      node = list_insert_last(node, pt_gen_int(size));
      node = list_insert_last(node, bufname);
      node = pt_gen_call(buf, node);
      break;

    case FD_CMMD:    /* reductions for CMMD */

      switch (iset->reduc_set->rtype)
      {
        case FD_REDUC_TIMES:
          reduc_suffix = "times";
          break;
        case FD_REDUC_PLUS:
          reduc_suffix = "add";
          break;
        case FD_REDUC_MIN:
          reduc_suffix = "min";
          break;
        case FD_REDUC_MAX:
          reduc_suffix = "max";
          break;
        default:
          die_with_message("Reduction type not yet handled \n");
          break;
      }

      strcpy(buf, "cmmd_combiner_");
      switch (sp->fform)
      {
        case INTTYPE:
          strcat(buf, reduc_suffix);
          combiner = pt_gen_ident(buf);
          strcpy(buf, "cmmd_reduce");
          if (size > 1)
            stride = pt_gen_int(4);
          break;

        case REAL:
          strcat(buf, "f");
          strcat(buf, reduc_suffix);
          combiner = pt_gen_ident(buf);
          strcpy(buf, "cmmd_reduce_float");
          if (size > 1)
            stride = pt_gen_int(4);
          break;

        case DOUBLE_P:
          strcat(buf, "d");
          strcat(buf, reduc_suffix);
          combiner = pt_gen_ident(buf);
          strcpy(buf, "cmmd_reduce_double");
          if (size > 1)
            stride = pt_gen_int(8);
            strcat(buf, "_v");
          break;

        default:
          die_with_message("Reduction type not handled \n");
      }

      if (size > 1)
      {
        /* call cmmd_reduce_double_v(src, dest, combiner, stride, elem) */

        strcat(buf, "_v");
        node = list_create(tree_copy(var));
        node = list_insert_last(node, tree_copy(var));
        node = list_insert_last(node, combiner);
        node = list_insert_last(node, stride);
        node = list_insert_last(node, pt_gen_int(size));
        node = pt_gen_call(buf, node);
      }
      else
      {
        /* var = cmmd_reduce_double(var, cmmd_combiner_dadd) */

        node = list_create(tree_copy(var));
        node = list_insert_last(node, combiner);
        node = pt_gen_invoke(buf, node);
        node = gen_ASSIGNMENT(AST_NIL, var, node);
      }
      break;
  }

  return node;
}


/***************************************************************/
/* place the assignment statement just after the call to       */
/* the reduce statement                                        */
/* e.g. x = x$ + x  or  x = x$ * x                             */
/***************************************************************/
static AST_INDEX
make_reduc_assignment(Dist_Globals *dh, Iter_set *iset)
{
  AST_INDEX lhs, rhs;
  char *name, name1[MAX_NAME];
  AST_INDEX s1, s2;

  name = iset->reduc_set->lhs_name;

  strcpy(name1, name);
  strcat(name1, "$");

  /* make the parameter list */
  s1 = pt_gen_ident(name1);
  s2 = pt_gen_ident(name);

  lhs = tree_copy(s2);

  switch (iset->reduc_set->rtype)
  {
    case FD_REDUC_PLUS:
      rhs = gen_BINARY_PLUS(s1, s2);
      break;

    case FD_REDUC_TIMES:
      rhs = gen_BINARY_TIMES(s1, s2);
      break;

    default:
      die_with_message("Reduction type not handled");
  }

  return (gen_ASSIGNMENT(AST_NIL, lhs, rhs));
}

/*****************************************************************/
/* place the initialization statement just before the loop after */
/* the reduce statement is placed                                */
/* e.g. x$ = real(1) in the case of a multiplication              */
/*      x$ = real(0) in the case of an addition                   */
/*****************************************************************/
static AST_INDEX
make_init_assignment(Dist_Globals *dh, Iter_set *iset)
{
  AST_INDEX rhs;
  char name1[MAX_NAME];
  Reduc_type reduc_type;
  SNODE *sp;

  /* make lhs  */
  sp = findadd2(iset->reduc_set->lhs, 0, 0, dh);
  reduc_type = iset->reduc_set->rtype;
  strcpy(name1, iset->reduc_set->lhs_name);
  strcat(name1, "$");

  /* make rhs */
  switch (reduc_type)
  {
    case FD_REDUC_PLUS:
      switch (sp->fform)
      {
        case INTTYPE:
          rhs = pt_gen_int(0);
          break;

        case REAL:
          rhs = gen_CONSTANT();
          gen_put_text(rhs, "0.0", STR_CONSTANT_REAL);
          break;

        case DOUBLE_P:
          rhs = gen_CONSTANT();
          gen_put_text(rhs, "0.0", STR_CONSTANT_DOUBLE_PRECISION);
          break;

        default:
          die_with_message("Reduction type not handled \n");
          break;
      }
      break;


    case FD_REDUC_TIMES:
      switch (sp->fform)
      {
        case INTTYPE:
          rhs = pt_gen_int(1);
          break;

        case REAL:
          rhs = gen_CONSTANT();
          gen_put_text(rhs, "1.0", STR_CONSTANT_REAL);
          break;

        case DOUBLE_P:
          rhs = gen_CONSTANT();
          gen_put_text(rhs, "1.0", STR_CONSTANT_DOUBLE_PRECISION);
          break;

        default:
          die_with_message("Reduction type not handled \n");
          break;
      }
      break;

    default:
      die_with_message("Reduction type not supported yet \n");
      break;
  }

  return (gen_ASSIGNMENT(AST_NIL, pt_gen_ident(name1), rhs));
}


/*****************************************************************/
/* In the reduction statement replace the original scalar        */
/* declaration id with id$                                       */
/*****************************************************************/

/*****************************************************************/
/* called by walk_expression---replace id with id$               */
/*****************************************************************/
static int
replace_var(AST_INDEX expr, char *lhs)
{
  AST_INDEX expr2;
  char lhs2[MAX_NAME];

  if (is_identifier(expr))
  {
    if (!(strcmp(lhs, gen_get_text(expr))))
    {
      expr2 = gen_IDENTIFIER();
      strcpy(lhs2, lhs);
      strcat(lhs2, "$");
      gen_put_text(expr2, lhs2, STR_IDENTIFIER);
      tree_replace(expr, expr2);
    }
  }
  return WALK_CONTINUE;
}









