/* $Id: m2.ansi.c,v 1.22 1997/03/11 14:28:17 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/



/*---------------------------------------------------------------------

    m2.c       Utility routines for generating send/recv messages

    Author : Seema Hiranandani 

*/

#include <libs/fortD/codeGen/private_dc.h>
#include <math.h>
#include <assert.h>

struct vmp_params;

/*------------------------- global definitions ------------------------*/

AST_INDEX dc_send_guard(Dist_Globals *dh, SNODE *sp, int *offsetlist);
AST_INDEX dc_recv_guard(Dist_Globals *dh, SNODE *sp, int *offsetlist);
void dc_compute_proc_range(PedInfo ped, int *min, int *max, Rsd_set *rset, 
                           Iter_type msg_type);
AST_INDEX dc_vector_pipe_send(Dist_Globals *dh, Rsd_set *rset, AST_INDEX start);
AST_INDEX dc_vector_pipe_recv(Dist_Globals *dh, Rsd_set *rset, AST_INDEX start);
AST_INDEX dc_ins_send_sync(Dist_Globals *dh, Rsd_set *rset, AST_INDEX start);
AST_INDEX dc_ins_recv_sync(Dist_Globals *dh, Rsd_set *rset, AST_INDEX start);

/*------------------------- local definitions ------------------------*/

STATIC(Boolean, interfere_assign,(Dist_Globals *dh, Rsd_set *rset, AST_INDEX stmt,
                                  int level));
STATIC(Boolean, interfere_loop,(Dist_Globals *dh, Rsd_set *rset, AST_INDEX loop,
                                int level));
STATIC(int, check_stmt_interfere,(AST_INDEX stmt, int lvl, struct vmp_params *param));
STATIC(AST_INDEX, select_loop_level,(AST_INDEX x, int level));
STATIC(int, check_stmt_distance,(AST_INDEX x, AST_INDEX y));
STATIC(Boolean, is_msg,(AST_INDEX stmt, char *name));
STATIC(Boolean, ast_call_name,(AST_INDEX stmt, char *name));

/*----------------------- external definitions ------------------------*/

EXTERN(Boolean, rsd_intersecting,(Rsd_section* rsd1, Rsd_section* rsd2));

/*----------------------- structure definitions -----------------------*/


/* used to pass parameters to check_stmt_interfere() */

struct vmp_params
{
  Dist_Globals *dh;
  Rsd_set *rset;
  int level;
  Boolean result;
};

#define MAXDIST 99999


/*------------------*/
/* global functions */
/*------------------*/

/*-----------------------------------------------------------------------

  dc_send_guard()

*/

AST_INDEX
dc_send_guard(Dist_Globals *dh, SNODE *sp, int *offsetlist)
{
  AST_INDEX max_ast, min_ast, grd1;
  AST_INDEX lstring, utemp, ltemp, grd, sends, grd2;
  int pmax, pmin, i, MinProc, MaxProc;

  grd = grd1 = AST_NIL;

  if (sp->perfect_align)
    sp = sp->decomp;

  for (i = 0; i < sp->numdim; ++i)
  {
    if ((sp_is_part(sp, i) != FD_DIST_LOCAL) && offsetlist[i])
    {
      lstring = ast_get_logical_myproc(dh);

      MinProc = 0;
      MaxProc = sp_num_blocks(sp, i) - 1;

      pmax = dh->max[i] - offsetlist[i]-1;
      pmin = dh->min[i] - offsetlist[i]-1;

      min_ast = pt_gen_int(pmin-1);
      max_ast = pt_gen_int(pmax+1);

      if ((pmax == MaxProc) && (pmin == MinProc))
        grd1 = AST_NIL;

      else if (pmin == MinProc)
        grd1 = gen_BINARY_LT(tree_copy(lstring), max_ast);

      else if (pmax == MaxProc)
        grd1 = gen_BINARY_GT(tree_copy(lstring), min_ast);

      else if (pmin == pmax)
        grd1 = gen_BINARY_EQ(tree_copy(lstring), pt_gen_int(pmin));

      else
     /* min and max are somewhere between the range of MaxProc and MinProc */

      {
        utemp = gen_BINARY_LT(tree_copy(lstring), max_ast);
        ltemp = gen_BINARY_GT(tree_copy(lstring), min_ast);
        grd1 = gen_BINARY_AND(utemp, ltemp);
      }

      /* if there is a guard generated then check to see if it a guard
       already exists and it must be merged with it */

      if (grd1 != AST_NIL)
      {
        if (grd != AST_NIL)
        {
          grd2 = gen_BINARY_AND(tree_copy(grd1), tree_copy(grd));
          grd = grd2;
        }
        else
          grd = grd1;
      }
    }
  }

  return grd;
}


/**************************************************************************/
/* this function creates the guard for the crecv set if boundary         **/
/*   conditions exist                                                    **/
/* for each kind of message that is being sent                           **/
/**************************************************************************/
AST_INDEX
dc_recv_guard(Dist_Globals *dh, SNODE *sp, int *offsetlist)
{
  AST_INDEX grd;
  AST_INDEX lstring, utemp, ltemp, ifstmt, recv, max_ast, min_ast;
  int pmax, pmin;
  int i, MinProc, MaxProc;

  min_ast = max_ast = AST_NIL;
  grd = lstring = AST_NIL;
  utemp = ltemp = ifstmt = recv = AST_NIL;

  if (sp->perfect_align)
    sp = sp->decomp;

  MinProc = 0;
  for (i = 0; i < sp->numdim; ++i)
  {
    MaxProc = sp_num_blocks(sp, i) - 1;

    if (offsetlist[i])
    {
      lstring = ast_get_logical_myproc(dh);

      pmax = dh->max[i] - 1;
      pmin = dh->min[i] - 1;

      min_ast = pt_gen_int(pmin - 1);
      max_ast = pt_gen_int(pmax + 1);

      if ((pmax == MaxProc) && (pmin == MinProc))
        grd = AST_NIL;

      else if (pmin == MinProc)
        grd = gen_BINARY_LT(tree_copy(lstring), max_ast);

      else if (pmax == MaxProc)
        grd = gen_BINARY_GT(tree_copy(lstring), min_ast);

      else if (pmin == pmax)
        grd = gen_BINARY_EQ(tree_copy(lstring), pt_gen_int(pmin));

      else
      /* min and max are somewhere between the range of MaxProc and MinProc */
      {
        utemp = gen_BINARY_LT(tree_copy(lstring), max_ast);
        ltemp = gen_BINARY_GT(tree_copy(lstring), min_ast);
        grd = gen_BINARY_AND(utemp, ltemp);
      }

      return grd;
    }
  }

  printf("do_recv_guard(): generating guard for local RSD\n");
  return AST_NIL;
}


/***********************************************************************
 * this function generates the min and max processors that perform the
 * receiving of data
 * For each dimension of the array reference rhs, assignment statements
 * are produced.
 * Assignment statements are placed before any guards surrounding the
 * loop.
 *
 * rhs - AST node containing the array reference occuring in an
 *        assignment statement within loop
 * loop - AST node containing loop containing assignment statement
 *         requiring communication
 *
 * Assumption: The procedure is called only if assignment statements for
 *             the array in the array reference rhs are necessary.
 *           + For each loop nest, the dh->doloops array contains
 *             outermost loops then inner loops in sequential order.
************************************************************************/
void
dc_compute_proc_range(PedInfo ped, int *min, int *max, Rsd_set *rset, 
                      Iter_type msg_type)
{
  int dim;         /* dimension of the array for which := statements produced */
  SNODE *sp;       /* SNODE for array in rhs */
  int blocksize, pmax, pmin;
  int l_level;
  Loop_list *linfo;
  AST_INDEX loop;

  /* get info on original loop bounds */
  loop = dt_ast_loop(rset->subs[0]);
  linfo = (Loop_list *) get_info(ped, loop, type_ref);
  if (linfo == (Loop_list *) NO_REF)
    die_with_message("find_loops(): missing loop info");

  sp = rset->sp;

  /* Assume dc_compute_proc_range() called only if want min$proc and max$proc
   * assignment statements. */

  pmin = pmax = 0;

  for (dim = 0; dim < sp->numdim; dim++)
  {
    switch (sp_is_part(sp, dim))
    {
      case FD_DIST_BLOCK:

        l_level = rset->sinfo[0]->subs[dim].stype;
        if (l_level > SUBS_SIV)
        {
          printf("dc_compute_proc_range(): non SIV subscript\n");
          continue;
        }

        /*
        (void) sprintf(mininfo, "min$proc%d", dim + 1);
        minproc = gen_IDENTIFIER();
        gen_put_text(minproc, mininfo, STR_IDENTIFIER);

        (void) sprintf(maxinfo, "max$proc%d", dim + 1);
        maxproc = gen_IDENTIFIER();
        gen_put_text(maxproc, maxinfo, STR_IDENTIFIER);
        */

        blocksize = sp_block_size1(sp, dim);
        if (blocksize)
        {
          if (msg_type == Iter_simple || msg_type == Iter_all)
          {
            pmax = (linfo->loops[l_level].up.val - 1) / blocksize + 1;
            pmin = (linfo->loops[l_level].lo.val - 1) / blocksize + 1;
          }

          if (msg_type == Iter_pre)    /* includes the pre and mid set */
          {
            if (rset->iterset->type[l_level] != Iter_all) /* if no post set */
              pmax = (linfo->loops[l_level].up.val - 1) / blocksize + 1;
            else              /* if post set exists */
              pmax = (linfo->loops[l_level].up.val - 1) / blocksize;
            pmin = (linfo->loops[l_level].lo.val - 1) / blocksize + 1;
          }

          if (msg_type == Iter_post)    /* includes the post and mid set */
          {    
            pmax = (linfo->loops[l_level].up.val - 1) / blocksize + 1;
            if (!rset->iterset->pre_idle[l_level])
              pmin = 2;
            else
              pmin = rset->iterset->pre_idle[l_level] + 1;
          }

          if (msg_type == Iter_pre_only)        /* includes the pre set only */
            pmax = pmin = (linfo->loops[l_level].lo.val - 1) / blocksize + 1;

          if (msg_type == Iter_post_only)       /* includes the post set only */
            pmax = pmin = (linfo->loops[l_level].up.val - 1) / blocksize + 1;

          /* if message type = Iter_mid_only  */

          if (msg_type == Iter_mid_only)
          {
            if (rset->iterset->type[l_level] == Iter_pre)
            {
              pmax = (linfo->loops[l_level].up.val - 1) / blocksize + 1;
              if (!rset->iterset->pre_idle[l_level])
                pmin = 2;
              else
                pmin = rset->iterset->pre_idle[l_level] + 2;
            }

            else if (rset->iterset->type[l_level] == Iter_post)
            {
              pmax = (linfo->loops[l_level].up.val - 1) / blocksize;
              pmin = (linfo->loops[l_level].lo.val - 1) / blocksize + 1;
            }

            else if (rset->iterset->type[l_level] == Iter_all)
            {
              pmax = (linfo->loops[l_level].up.val - 1) / blocksize;
              if (!rset->iterset->pre_idle[l_level])
                pmin = 2;
              else
                pmin = rset->iterset->pre_idle[l_level] + 2;
            }
          }
        }

        min[dim] = pmin;
        max[dim] = pmax;
        break;

      default:
        break;
    }
  }
}

/*--------------------------------------------------------------------

  dc_vector_pipe_recv()  Perform Vector Message Pipelining

  Move vector RECV back, as far away from SEND as legal.
  Currently will not move vector RECV across loop iterations.

*/

AST_INDEX
dc_vector_pipe_recv(Dist_Globals *dh, Rsd_set *rset, AST_INDEX start)
{
  AST_INDEX node, closest;
  int i, level, distance, closest_dist;

  level = loop_level(start);   /* 1...MAXLOOP */
  if (is_loop(start))
    level--;

  closest_dist = MAXDIST;
  closest = start;

  /* check to ensure we don't pipeline past any recvs (conservative) */
  /* this places an upper bound on the amount of pipelining allowed  */

  node = start; 
  while (node != AST_NIL)
  {
    closest = node;

    if (is_call(node))
    {
      /* need interprocedural info for full support, for now just stop */
      break;
    }

    if (is_msg(node, dh->syscalls.crecv) ||
        is_msg(node, dh->syscalls.irecv) ||
        is_msg(node, dh->syscalls.brecv))
      break;

    node = list_next(node);
  }

  while (is_comment(list_prev(closest)))  /* don't push past comments */
    closest = list_prev(closest);

  if (closest == start)  /* won't get anywhere further */
    return start;

  closest_dist = check_stmt_distance(start, closest);

  /* check for closest use comprising RSD to determine amount */
  /* of vector mesg pipelining permitted by data dependences  */

  while (rset)
  {
    for (i = 0; i < rset->num_subs; i++)
    {
      node = select_loop_level(rset->subs[i], level);
      distance = check_stmt_distance(start, node);

      if (distance < closest_dist)
      {
        closest_dist = distance;
        closest = node;

        if (!distance)
          return closest;
      }
    }
    rset = rset->rsd_merge;
  }

  return closest;  
}




/*--------------------------------------------------------------------

  dc_vector_pipe_send()  Perform Vector Message Pipelining

  Move vector SEND back, as far away from RECV as legal.
  Currently will not move vector SEND across loop iterations.

*/

AST_INDEX
dc_vector_pipe_send(Dist_Globals *dh, Rsd_set *rset, AST_INDEX start)
{
  AST_INDEX insert;
  AST_INDEX stmt;
  int level;

  level = loop_level(start);   /* Rvec goes from 0...MAXLOOP-1 */
  if (level > 0) 
    level--;
  insert = start;
  stmt = start;

  while ((stmt = list_prev(stmt)) != AST_NIL)
  {
    /* if current statement interferes with the SEND,       */
    /* return last legal point located for inserting SEND   */

    if (stmt == dh->fortD_ph)  /* don't go back too far */
      return insert;

    if (is_msg(stmt, dh->syscalls.csend) || 
        is_msg(stmt, dh->syscalls.isend) || 
        is_msg(stmt, dh->syscalls.bcast) )     /* don't move past other sends */
      return insert;

    if (is_assignment(stmt))
    {
      if (interfere_assign(dh, rset, stmt, level))
        return insert;
    }
    else if (is_loop(stmt))
    {
      if (interfere_loop(dh, rset, stmt, level))
        return insert;
    }
    else if (is_call(stmt))
    {
      /* need interprocedural info for full support, for now just stop */

      return insert;
    }

    /* move insertion point back if noncomment, unless  */
    /* comment belongs to a previously inserted message */

    if (!is_comment(stmt) ||  
        !strncmp("--<<",gen_get_text(gen_COMMENT_get_text(stmt)), 4))
      insert = stmt;
  }

  return insert;
}


/*--------------------------------------------------------------------

  dc_ins_send_sync()  Insert synchronization for unbuffered send

  Move isend sync as far forward as legal.
  Currently will not move sync across loop iterations.

*/

AST_INDEX
dc_ins_send_sync(Dist_Globals *dh, Rsd_set *rset, AST_INDEX start)
{
  AST_INDEX insert;
  AST_INDEX stmt;
  int level;

  level = loop_level(start);   /* Rvec goes from 0...MAXLOOP-1 */
  if (level > 0) 
    level--;
  insert = start;
  stmt = start;

  while ((stmt = list_next(stmt)) != AST_NIL)
  {
    /* if current statement interferes with the SEND,       */
    /* return last legal point located for inserting SEND   */

    if (is_return(stmt) || is_stop(stmt))
      return insert;

    if (!dh->send_userbuf)  /* only check for interference if no user bufs */
    {
      if (is_assignment(stmt))
      {
        if (interfere_assign(dh, rset, stmt, level))
          return insert;
      }
      else if (is_loop(stmt))
      {
        if (interfere_loop(dh, rset, stmt, level))
          return insert;
      }
      else if (is_call(stmt))
      {
        return insert;  /* need interprocedural info, for now just stop */
      }
    }

    /* move insertion point forward if noncomment, unless  */
    /* comment belongs to a previously inserted message    */

    if (!is_comment(stmt) ||  
        !strncmp("--<<",gen_get_text(gen_COMMENT_get_text(stmt)), 4))
      insert = stmt;
  }

  return insert;
}

/*--------------------------------------------------------------------

  dc_ins_recv_sync()  Insert synchronization for unbuffered recv

  Move irecv sync as far back as legal.
  Currently will not move sync across loop iterations.

*/

AST_INDEX
dc_ins_recv_sync(Dist_Globals *dh, Rsd_set *rset, AST_INDEX start)
{
  AST_INDEX insert;
  AST_INDEX stmt;
  int level;

  level = loop_level(start);   /* Rvec goes from 0...MAXLOOP-1 */
  if (level > 0) 
    level--;
  insert = start;
  stmt = start;

  while ((stmt = list_prev(stmt)) != AST_NIL)
  {
    if (stmt == dh->fortD_ph)  /* don't go back too far */
      return insert;

    /* if current statement interferes with the RECV,       */
    /* return last legal point located for inserting RECV   */

    insert = stmt;
  }

  return insert;
}


/*------------------*/
/* local functions  */
/*------------------*/


/*--------------------------------------------------------------------

  interfere_assign()  

  Determine whether assignment statement interferes with RSD

*/

static Boolean
interfere_assign(Dist_Globals *dh, Rsd_set *rset, AST_INDEX stmt, int level)
{
  Rsd_vector *rvec;
  AST_INDEX lhs, rhs;
  char *lhs_name;
  Rsd_section *rsd;

  /* don't move communication past calls to timing routines */
  rhs = gen_ASSIGNMENT_get_rvalue(stmt);
  if (is_invocation(rhs))
  {
    rhs = gen_INVOCATION_get_name(rhs);
    if (!strcmp("dclock", gen_get_text(rhs)))
      return true;
  }

  rvec = (Rsd_vector *) get_info(dh->ped, stmt, type_ref);
  if (rvec == (Rsd_vector *) NO_REF)
    return false;

  lhs = gen_ASSIGNMENT_get_lvalue(stmt);
  lhs = gen_SUBSCRIPT_get_name(lhs);
  lhs_name = gen_get_text(lhs);

  /* look at each array aggregated in this message */

  while (rset)
  {
    if (!strcmp(lhs_name, rset->sp->id))
    { 
      rsd = rset->rs_carried ? rset->rs_carried : rset->rs;

      /* if rsds intersect then interference exists */
      if (rsd_intersecting(rvec->lhs[level], rsd))
        return true;
    }

    rset = rset->rsd_merge; 
  }
 
  return false;
}


/*--------------------------------------------------------------------

  interfere_loop()  

  Determine whether loop nest interferes with RSD

*/

static Boolean
interfere_loop(Dist_Globals *dh, Rsd_set *rset, AST_INDEX loop, int level)
{
  struct vmp_params param;

  param.dh = dh;
  param.rset = rset;
  param.level = level;
  param.result = false;

  walk_statements(loop, LEVEL1, (WK_STMT_CLBACK)check_stmt_interfere, NULL, 
                  (Generic)&param);
  return param.result;
}


/*--------------------------------------------------------------------

  check_stmt_interfere()    Helper function for interfere_loop()

  Determine whether statement in loop nest interferes with RSD

*/
static int
check_stmt_interfere(AST_INDEX stmt, int lvl, struct vmp_params *param)
{
  if (is_assignment(stmt) &&
      interfere_assign(param->dh, param->rset, stmt, param->level))
  {
    param->result = true;
    return WALK_ABORT;
  }

  return WALK_CONTINUE;
}


/*--------------------------------------------------------------------

  check_stmt_distance()    Determine # of statements separating X & Y

  X,Y must be at the same loop level, with X preceding Y

*/

static int
check_stmt_distance(AST_INDEX x, AST_INDEX y)
{
  int distance;

  distance = 0;

  while ((x != y) && (x != AST_NIL))
  {
    x = list_next(x);

    if (!is_comment(x))
      distance++;
  }

  return (x == AST_NIL) ? -1 : distance;
}


/*--------------------------------------------------------------------

  select_loop_level()   Get statement enclosing X at a given loop level 

*/

static AST_INDEX 
select_loop_level(AST_INDEX x, int level)
{
  AST_INDEX out;

  assert (level >= 1);
  assert (loop_level(x) >= level);

  out = x;
  while (x != AST_NIL) 
  {
    if (is_loop(x) && (loop_level(x) == level))
      return out;

    out = x;
    x = tree_out(x);
  }

  return AST_NIL;   /* should not get here! */
}


/*--------------------------------------------------------------------

  is_msg()  Determine whether stmt contains SEND or RECV

*/

static Boolean
is_msg(AST_INDEX stmt, char *name)
{
  AST_INDEX guard;

  if (!name)
    return false;

  if (is_if(stmt))
  {
    guard = gen_IF_get_guard_LIST(stmt);
    guard = list_first(guard);
    while (guard != AST_NIL)
    {
      stmt = gen_GUARD_get_stmt_LIST(guard);
      stmt = list_first(stmt);

      while (stmt != AST_NIL)
      {
        if (ast_call_name(stmt, name))
          return true;

        stmt = list_next(stmt);
      }

      guard = list_next(guard);
    }
  }
  else if (ast_call_name(stmt, name))
    return true;

  return false;
}

/*--------------------------------------------------------------------

  ast_call_name()  Determine whether CALL is to "name"

*/

static Boolean
ast_call_name(AST_INDEX stmt, char *name)
{
  if (is_call(stmt))
  {
    stmt = gen_CALL_get_invocation(stmt);
    stmt = gen_INVOCATION_get_name(stmt);

    if (!strcmp(gen_get_text(stmt), name))
      return true;
  }
  return false;
}

