/* $Id: bounds.ansi.c,v 1.16 1997/03/11 14:28:13 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/



/**********************************************************************

 bounds.c    Instantiate work partition through loop bounds reduction
                                                          
 Author : Seema Hiranandani                                           

 There are two cases for instantiating the work partition.

 1) Loop bounds are reduced, possibly with symbolics expressions:

    Dimension in distributed array has index variable

      do i = L, U
         A(i) = ...
      enddo

         |
         V

    [For BLOCK distributions]

      lb$1=max((my$p*blocksize)+1,L)-my$p*blocksize
      ub$1=min((my$p+1)*blocksize,U)-my$p*blocksize

      do i = lb$1, ub$1
         A(i) = ...
      enddo

    [For CYCLIC distributions]

      lb$1 = ((L-1)/n$p)+1
      m1 = MOD(L-1,n$p)
      if (my$p < m1)
        lb$1 = lb$1 + 1

      ub$1 = ((U-1)/n$p)+1
      m2 = MOD(U-1,n$p)
      if (my$p > m2)
        ub$1 = ub$1 - 1

      do i = lb$1, ub$1
         A(i) = ...
      enddo


    The situation is slightly more complicated if there is an offset:

      do i = L, U
         A(i+c) = ...
      enddo

         |
         V

    [For BLOCK distributions]

      lb$1=max((my$p*blocksize)+1-c,L)-my$p*blocksize
      ub$1=min((my$p+1)*blocksize-c,U)-my$p*blocksize
      do i = lb$1, ub$1
         A(i+c) = ...
      enddo

    [For CYCLIC distributions]

         ??

 2) Guards are generated if :

  A. Some processors do not execute any iterations.  Symbolic
     bounds may still be needed, though.

      do i = L, U
         A(i) = ...
      enddo

         |
         V

    [For BLOCK distributions]

      lb$1=max((my$p*blocksize)+1,L)-my$p*blocksize
      ub$1=min((my$p+1)*blocksize,U)-my$p*blocksize
      if ((my$p .gt. ___ ) .and. (my$p .lt. ___ )) then
        do i = lb$1, ub$1
           A(i) = ...
        enddo
      endif


  B. If dimension in distributed array has constant:

      do i = L, U
         A(K) = ...
      enddo

         |
         V

    [For BLOCK distributions]

      if (my$p .eq. ((K-1)/blocksize) ) then
        do i = L, U
           A(K - my$p*blocksize) = ...
        enddo
      endif

    [For CYCLIC distributions]

      if (my$p .eq. ((K-1)%blocksize) ) then
        do i = L, U
           A(K/blocksize) = ...
        enddo
      endif

  C. If loop must remain in global indices even though its 
     iterations are partitioned for some statements.  Local
     indices must also be explicitly created.

      do i = L, U
         A(i) = ...
         j = ...
      enddo

         |
         V

    [For BLOCK distributions]

      do i = L, U
         if (i .gt. my$p*blocksize) .and. 
              (i .le. (my$p+1)*blocksize)) then
           i$loc = i - my$p*blocksize
           A(i$loc) = ...
         endif
         j = ...
      enddo

    [For CYCLIC distributions]

      do i = L, U
         if (my$p .eq. MOD(i-1,P)) then
           i$loc = (i-1)/blocksize+1
           A(i$loc) = ...
         endif
         j = ...
      enddo


***********************************************************************/


#include <libs/fortD/codeGen/private_dc.h>

struct sgroup_param;

/*------------------------- extern definitions ------------------------*/

EXTERN(AST_INDEX, ast_get_sub,(AST_INDEX subs, int dim));

/*------------------------- global definitions ------------------------*/

void dc_bounds(Dist_Globals *dh);
AST_INDEX dc_outer_perf(AST_INDEX node);
AST_INDEX gen_owner(Dist_Globals *dh, AST_INDEX ref, SNODE *sp, Subs_list *slist);
AST_INDEX gen_offset(AST_INDEX expr, int offset);
AST_INDEX ast_merge_ifs(AST_INDEX if1);

/*--------------------------- static definitions ----------------------*/

STATIC(void, gen_blk_bounds,(Dist_Globals *dh, AST_INDEX loop, Iter_set *iset,
                             int depth, AST_INDEX *lb, AST_INDEX *ub));
STATIC(void, gen_blk_lguard,(Dist_Globals *dh, AST_INDEX loop, Iter_set *iset,
                             int depth));
STATIC(AST_INDEX, gen_blk_ub,(Dist_Globals *dh, Iter_set *iset, int loop_num, 
                              int dim));
STATIC(AST_INDEX, gen_blk_lb,(Dist_Globals *dh, Iter_set *iset, int loop_num,
                              int dim));

STATIC(void, gen_cyc_bounds,(Dist_Globals *dh, AST_INDEX loop, FortD_LI *fli, 
                             AST_INDEX *lb, AST_INDEX *ub));

STATIC(void, gen_user_bounds,(Dist_Globals *dh, AST_INDEX loop, FortD_LI *fli, 
                             AST_INDEX *lb, AST_INDEX *ub));

STATIC(void, guard_loop,(Dist_Globals *dh, FortD_LI *fli));
STATIC(void, guard_nonloop,(Dist_Globals *dh));

STATIC(Boolean, is_common_sgroup,(AST_INDEX stmt, S_group *sgroup));
STATIC(int, common_sgroup,(AST_INDEX stmt, int level, struct sgroup_param *sparam));

/*------------------------- struct definitions -----------------------*/

struct sgroup_param    /* hold arguments for walk_statements() */
{
  S_group *sgroup;
  Boolean same;
};


/********************/
/* global functions */
/********************/


/*--------------------------------------------------------------------

    dc_bounds()

    main routine to partition program using "owner computes" rule

*/

void
dc_bounds(Dist_Globals *dh)
{
  int i;
  AST_INDEX stmt, loop, ub[LOOPTOTAL], lb[LOOPTOTAL];
  char strbuf[MAX_NAME];
  Iter_set *iset;
  FortD_LI *fli;

  for (i = 0; i < LOOPTOTAL; i++)
    lb[i] = ub[i] = AST_NIL;

  /*---------------------------------------------------------*/
  /* calculate local loop bounds & their boundary conditions */

  for (i = dh->numdoloops - 1; i >= 0; i--)
  {
    loop = dh->doloops[i];

    fli = (FortD_LI *) get_info(dh->ped, loop, type_fd);

    /*---------------------------------*/
    /* reduce loop bounds if necessary */

    if (fli->localized)
    {
      switch (fli->dist_type)
      {
      case FD_DIST_BLOCK:
        iset = fli->sgroup.groups[0]->iset;
        gen_blk_bounds(dh, loop, iset, fli->depth, lb, ub);
        gen_blk_lguard(dh, loop, iset, fli->depth);
        break;

      case FD_DIST_CYCLIC:
        gen_cyc_bounds(dh, loop, fli, lb, ub);
        break;

      case FD_DIST_USER:
        gen_user_bounds(dh, loop, fli, lb, ub);
        break;

      default:
        break;
      }
    }

    /*----------------------------*/
    /* insert guards if necessary */

    /* 4/9/94 RvH: Check first whether we have an iset at all */
    else if ((fli->iset) && (fli->iset->oneproc))
    {
      gen_blk_lguard(dh, loop, fli->iset, fli->depth);
    }

    if (!fli->uniform)
      guard_loop(dh, fli);
  }

  /*------------------------------------------------------*/
  /* insert all boundary conditions for local loop bounds */

  stmt = dh->fortD_ph;

  for (i = 0; lb[i] != AST_NIL; i++)
  {
    sprintf(strbuf, "lb$%d", i+1);
    dc_new_var(dh, pt_gen_ident(strbuf), INTTYPE);

    if (!is_place_holder(lb[i]))
    {
      (void) list_insert_before(stmt, 
        gen_ASSIGNMENT(AST_NIL, pt_gen_ident(strbuf), lb[i]));
    }
  }

  for (i = 0; ub[i] != AST_NIL; i++)
  {
    sprintf(strbuf, "ub$%d", i+1);
    dc_new_var(dh, pt_gen_ident(strbuf), INTTYPE);

    if (!is_place_holder(ub[i]))
    {
      (void) list_insert_before(stmt, 
        gen_ASSIGNMENT(AST_NIL, pt_gen_ident(strbuf), ub[i]));
    }
  }

  /* insert guard for each group of non-loop statements */

  guard_nonloop(dh);
}

/*--------------------------------------------------------------------

    dc_outer_perf()  Find outermost perfectly nested loop

*/

AST_INDEX
dc_outer_perf(AST_INDEX node)
{
  AST_INDEX loop, outer_loop;

  /* start at first enclosing loop */

  while ((node != AST_NIL) && !is_loop(node))
    node = tree_out(node);
  if (node == AST_NIL)  
    return AST_NIL;
  else
    loop = node;

  while (true)  
  {
    /* get next enclosing loop  */

    node = tree_out(loop);
    while ((node != AST_NIL) && !is_loop(node))
      node = tree_out(node);

    if (node == AST_NIL)  
      return loop;
    else
      outer_loop = node;

    node = gen_DO_get_stmt_LIST(outer_loop);
    node = list_first(node);

    /* look for imperfectly nested stmt  */

    while (is_comment(node))
      node = list_next(node);

    if (node != loop)
      return loop;

    node = list_next(node);

    while (is_comment(node))
      node = list_next(node);

    if (node != AST_NIL)
      return loop;

    loop = outer_loop;
  }
}

/*-------------------------------------------------------------------

  gen_owner()   Create AST for owner of reference 
  		VA: actually, creates logical if: if (owner .eq. node)
*/

AST_INDEX
gen_owner(Dist_Globals *dh, AST_INDEX ref, SNODE *sp, Subs_list *slist)
{
  Subs_data *rsub;
  int i, total_proc, bksize;
  Dist_type type;
  AST_INDEX node;

  for (i = 0; i < slist->dims; i++)
  {
    type = sp_is_part(sp, i);
    if (type != FD_DIST_LOCAL)
      break;
  }

  switch (type)
  {
    case FD_DIST_LOCAL:
      printf("gen_owner(): no distributed dimensions\n");
      return AST_NIL;

    case FD_DIST_BLOCK:
      bksize = sp_block_size1(sp, i);
      rsub = slist->subs + i;

      if (rsub->stype == SUBS_ZIV)
      {
        node = pt_gen_int((rsub->constant - 1) / bksize);
      }
      else 
      {
        if (rsub->stype < SUBS_SIV)
        {
          node = pt_gen_ident(slist->loop_nest->loops[rsub->stype].ivar);
          if (rsub->coeffs[rsub->stype] != 1)
            node = pt_gen_mul(pt_gen_int(rsub->coeffs[rsub->stype]), node);
          if (rsub->constant > 1)
            node = pt_gen_add(node, pt_gen_int(rsub->constant-1));
          else if (rsub->constant < 1)
            node = pt_gen_sub(node, pt_gen_int(1-rsub->constant));
        }
        else
        {
          node = pt_gen_sub(tree_copy(ast_get_sub(ref,i)), pt_gen_int(1));
        }

        if (!is_identifier(node))
          gen_put_parens(node,1);

        node = pt_gen_div(node, pt_gen_int(bksize));
      }
      break;

    case FD_DIST_CYCLIC:
      total_proc = sp_num_blocks(sp, i);
      rsub = slist->subs + i;

      if (rsub->stype == SUBS_ZIV)
      {
        node = pt_gen_int((rsub->constant - 1) % total_proc);
      }
      else 
      {
        if (rsub->stype < SUBS_SIV)
        {
          node = pt_gen_ident(slist->loop_nest->loops[rsub->stype].ivar);
          if (rsub->coeffs[rsub->stype] != 1)
            node = pt_gen_mul(pt_gen_int(rsub->coeffs[rsub->stype]), node);
          if (rsub->constant > 1)
            node = pt_gen_add(node, pt_gen_int(rsub->constant-1));
          else if (rsub->constant < 1)
            node = pt_gen_sub(node, pt_gen_int(1-rsub->constant));
        }
        else
        {
          node = pt_gen_sub(tree_copy(ast_get_sub(ref,i)), pt_gen_int(1));
        }
        node = list_create(node);
        node = list_insert_last(node, pt_gen_int(total_proc));
        node = pt_gen_invoke("MOD", node);
      }
      break;

    default:
      printf("gen_owner(): distribution not yet supported\n");
      return AST_NIL;
  }

  node = gen_BINARY_EQ(ast_get_logical_myproc(dh), node);
  return node;
}


/*-------------------------------------------------------------------

  gen_offset()   Create AST for offset plus AST 

*/

AST_INDEX
gen_offset(AST_INDEX expr, int offset)
{
  int off, val;
  AST_INDEX t1, t2;

  if (is_binary_plus(expr))
  {
    t1 = gen_BINARY_PLUS_get_rvalue1(expr);
    t2 = gen_BINARY_PLUS_get_rvalue2(expr);

    if (!ast_eval(t1, &val))
    {
      offset += val;
      expr = tree_copy(t2);
    }
    else if (!ast_eval(t2, &val))
    {
      offset += val;
      expr = tree_copy(t1);
    }
  }
  else if (is_binary_minus(expr))
  {
    t1 = gen_BINARY_MINUS_get_rvalue1(expr);
    t2 = gen_BINARY_MINUS_get_rvalue2(expr);

    if (!ast_eval(t1, &val))
    {
      offset += val;
      expr = pt_gen_sub(pt_gen_int(offset), expr);
      expr = tree_copy(expr);
      gen_put_parens(expr,1);
      return expr;
    }
    else if (!ast_eval(t2, &val))
    {
      offset -= val;
      expr = tree_copy(t1);
    }
  }
  else
  {
    expr = tree_copy(expr);
  }

  if (offset > 0)
    expr = pt_gen_add(expr, pt_gen_int(offset));
  else if (offset < 0)
    expr = pt_gen_sub(expr, pt_gen_int(-offset));

  expr = tree_copy(expr);
  if (is_identifier(expr) || is_constant(expr))
    gen_put_parens(expr,0);
  else
    gen_put_parens(expr,1);

  return expr;
}


/*--------------------------------------------------------------------

  ast_merge_ifs()   Merges IF stmt with preceding IF stmt if legal

  Returns resulting IF stmt

*/

AST_INDEX
ast_merge_ifs(AST_INDEX if1)
{
  AST_INDEX if2, g1, g2, node1, node2, c1, temp;

  /* find matching IF stmt, if it precedes argument */

  while (!is_if(if1))
  {
    if ((if1 = tree_out(if1)) == AST_NIL)
      return AST_NIL;
  }

  if2 = list_prev(if1);
  while (is_comment(if2))
    if2 = list_prev(if2);

  if ((if2 == AST_NIL) || !is_if(if2))
    return if1;

  g1 = list_first(gen_IF_get_guard_LIST(if1));
  g2 = list_first(gen_IF_get_guard_LIST(if2));

  node1 = gen_GUARD_get_rvalue(g1);
  node2 = gen_GUARD_get_rvalue(g2);

  if (!ast_equiv(node1,node2))
    return if1;

  /* move intervening comments into body of IF, if any */

  node1 = gen_GUARD_get_stmt_LIST(g1);
  c1 = list_prev(if1);
  while (is_comment(c1))
  {
    temp = c1;
    c1 = list_prev(c1);
    list_remove_node(temp);
    list_insert_first(node1, temp);
  }
  list_remove_node(if1);  /* remove original IF */

  /* merge stmt lists of IF */

  node2 = gen_GUARD_get_stmt_LIST(g2);
  node2 = list_append(node2, node1);
  gen_GUARD_put_stmt_LIST(g2, node2);

  /* merge stmt lists of ELSE */

  g1 = list_next(g1);
  if (node1 != AST_NIL)
  {
    node2 = list_next(g2);
    if (node2 == AST_NIL)
    {
      list_insert_after(g2,g1);
    }
    else
    {
      g2 = node2;
      node1 = gen_GUARD_get_stmt_LIST(g1);
      node2 = gen_GUARD_get_stmt_LIST(g2);
      node2 = list_append(node2, node1);
      gen_GUARD_put_stmt_LIST(g2, node2);
    }
  }

  return if2;
}

/*******************/
/* local functions */
/*******************/

/*-------------------------------------------------------------------

  gen_blk_bounds()    Generate bounds for loop w/ BLOCK distribution

*/

static void
gen_blk_bounds(Dist_Globals *dh, AST_INDEX loop, Iter_set *iset, int depth, 
               AST_INDEX *lb, AST_INDEX *ub)
{
  int i, dim;
  AST_INDEX lb_name, lb_ast, ub_name, ub_ast;
  char strbuf[MAX_NAME];
  Iter_type itype;

  itype = iset->type[depth];
  dim = iset->dim_num[depth];

  lb_name = AST_NIL;
  ub_name = AST_NIL;

  /*---------------------------------*/
  /* lower bound */

  if ((itype == Iter_simple) || 
      (itype == Iter_post) ||
      (itype == Iter_mid_only) ||
      (itype == Iter_post_only))
  {
    /* constant lower bound */
    lb_name = pt_gen_int(iset->set.loops[depth].lo.val);
  }
  else
  {
    /* variable lower bound */
    lb_ast = gen_blk_lb(dh, iset, depth, dim);

    for (i = 0; lb[i] != AST_NIL; i++)
    {
       if (ast_equiv(lb_ast,lb[i]))
       {
         tree_free(lb_ast);
         sprintf(strbuf, "lb$%d", i+1);
         lb_name = pt_gen_ident(strbuf);
       }
    }

    if (lb_name == AST_NIL)
    {
      lb[i] = lb_ast;
      sprintf(strbuf, "lb$%d", i+1);
      lb_name = pt_gen_ident(strbuf);
    }
  }


  /*---------------------------------*/
  /* upper bound */

  if ((itype == Iter_simple) || 
      (itype == Iter_pre) ||
      (itype == Iter_mid_only) ||
      (itype == Iter_post_only))
  {
    /* constant upper bound */
    ub_name = pt_gen_int(iset->set.loops[depth].up.val);
  }
  else
  {
    /* variable upper bound */
    ub_ast = gen_blk_ub(dh, iset, depth, dim);

    for (i = 0; ub[i] != AST_NIL; i++)
    {
       if (ast_equiv(ub_ast,ub[i]))
       {
         tree_free(ub_ast);
         sprintf(strbuf, "ub$%d", i+1);
         ub_name = pt_gen_ident(strbuf);
       }
    }

    if (ub_name == AST_NIL)
    {
      ub[i] = ub_ast;
      sprintf(strbuf, "ub$%d", i+1);
      ub_name = pt_gen_ident(strbuf);
    }
  }

  /* if loop has neg step, reverse bounds */

  tree_replace(iset->set.loops[depth].lo.ast, 
               iset->set.loops[depth].rev ? ub_name : lb_name);

  tree_replace(iset->set.loops[depth].up.ast, 
               iset->set.loops[depth].rev ? lb_name : ub_name);
}


/*------------------------------------------------------------------

  gen_blk_ub()      computes the local upper bound for BLOCK loops

  returns: min((my$p+1)*blocksize-offset,ub)-my$p*blocksize

*/

static AST_INDEX
gen_blk_ub(Dist_Globals *dh, Iter_set *iset, int loop_num, int dim)
{
  AST_INDEX bk, node, node2;
  int offset;

  bk = pt_gen_int(sp_block_size1(iset->lhs_sp, dim)); /* block size */

  node = pt_gen_add(ast_get_logical_myproc(dh),pt_gen_int(1));
  gen_put_parens(node,1);
  node = pt_gen_mul(node, tree_copy(bk));

  offset = iset->sinfo->subs[dim].constant; /* assumes perfect alignment */
  if (offset < 0)
  {
    gen_put_parens(node,1);
    node = pt_gen_add(node,pt_gen_int(-offset));
  }
  else if (offset > 0)
  {
    gen_put_parens(node,1);
    node = pt_gen_sub(node,pt_gen_int(offset));
  }

  node2 = list_create(tree_copy(iset->set.loops[loop_num].rev ? 
                          iset->set.loops[loop_num].lo.ast :
                          iset->set.loops[loop_num].up.ast));
  node = pt_gen_invoke("min", list_append(list_create(node), node2));
  node2 = pt_gen_mul(ast_get_logical_myproc(dh),bk);
  gen_put_parens(node2,1);
  node = pt_gen_sub(node,node2);

  return node;
}


/*------------------------------------------------------------------

  gen_blk_lb()      computes the local lower bound for BLOCK loops

  returns: max((my$p*blocksize)+1-offset,lb)-my$p*blocksize

*/

static AST_INDEX
gen_blk_lb(Dist_Globals *dh, Iter_set *iset, int loop_num, int dim)
{
  AST_INDEX bk, node, node2;
  int offset;

  bk = pt_gen_int(sp_block_size1(iset->lhs_sp, dim)); /* block size */

  node = pt_gen_mul(ast_get_logical_myproc(dh),tree_copy(bk));
  gen_put_parens(node,1);
  node = pt_gen_add(node, pt_gen_int(1));

  offset = iset->sinfo->subs[dim].constant; /* assumes perfect alignment */
  if (offset < 0)
  {
    gen_put_parens(node,1);
    node = pt_gen_add(node,pt_gen_int(-offset));
  }
  else if (offset > 0)
  {
    gen_put_parens(node,1);
    node = pt_gen_sub(node,pt_gen_int(offset));
  }

  node2 = list_create(tree_copy(iset->set.loops[loop_num].rev ? 
                          iset->set.loops[loop_num].up.ast :
                          iset->set.loops[loop_num].lo.ast));
  node = pt_gen_invoke("max", list_append(list_create(node), node2));
  node2 = pt_gen_mul(ast_get_logical_myproc(dh),bk);
  gen_put_parens(node2,1);
  node = pt_gen_sub(node,node2);

  return node;
}


/*-------------------------------------------------------------------

  gen_blk_lguard()    Generate guards for BLOCK loops

*/

static void
gen_blk_lguard(Dist_Globals *dh, AST_INDEX loop, Iter_set *iset, int depth)
{
  int MaxProc, pre, post;
  AST_INDEX guard, node, prev, next, lst;
  FortD_LI *fli;

  guard = AST_NIL;
  pre = iset->pre_idle[depth];
  post = iset->post_idle[depth];
  MaxProc = sp_num_blocks(iset->lhs_sp,iset->dim_num[depth]) - 1;

  if ((pre + post) == (MaxProc))
  {
    guard = gen_BINARY_EQ(ast_get_logical_myproc(dh), pt_gen_int(pre));
  }
  else
  {
    if (pre)
      guard = gen_BINARY_GT(ast_get_logical_myproc(dh), pt_gen_int(pre - 1));

    if (post)
    {
      node = gen_BINARY_LT(ast_get_logical_myproc(dh),
                          pt_gen_int(MaxProc - post + 1));
      
      guard = pre ? gen_BINARY_AND(node, guard) : node;
    }
  }

  /* now insert IF statement around enclosing perfectly nested loops */

  if (guard != AST_NIL)
  {
    loop = dc_outer_perf(loop);

    prev = list_prev(loop); /* Remember the loop's location. */
    next = list_next(loop);
    lst = list_head(loop);

    (void) list_remove_node(loop);    /* detach loop from the program */

    node = gen_GUARD(AST_NIL, guard, list_create(loop));
    node = gen_IF(AST_NIL, AST_NIL, list_create(node));

    if (prev != AST_NIL)      /* Place the if after the loop's */
      (void) list_insert_after(prev, node); /* previous location. */
    else if (next != AST_NIL)
      (void) list_insert_before(next, node);
    else         /* empty list */
      (void) list_insert_first(lst, node);

    node = ast_merge_ifs(node); /* merge with preceding IF stmt if legal */

    fli = (FortD_LI *) get_info(dh->ped, loop, type_fd);
    fli->guard = node;   /* store guard */
  }
}


/*-------------------------------------------------------------------

  gen_cyc_bounds()    Generate bounds for loop w/ CYCLIC distribution

      lb$1 = ((L-1)/n$p)+1
      m1 = MOD(L-1,n$p)
      if (my$p < m1)
        lb$1 = lb$1 + 1

      ub$1 = ((U-1)/n$p)+1
      m2 = MOD(U-1,n$p)
      if (my$p > m2)
        ub$1 = ub$1 - 1
*/

static void
gen_cyc_bounds(Dist_Globals *dh, AST_INDEX loop, FortD_LI *fli, AST_INDEX *lb, 
               AST_INDEX *ub)
{
  Iter_set *iset;
  int depth, i;
  AST_INDEX lb_name, ub_name, node, node2, minus1;
  char strbuf[MAX_NAME];

  iset = fli->sgroup.groups[0]->iset;
  depth = fli->depth;

  /* create symbolic lower bound if needed */

  if ((fli->cyclic_type == Citer_all) || 
      (fli->cyclic_type == Citer_lo))
  {
    for (i = 0; lb[i] != AST_NIL; i++)
      ;
    lb[i] = gen_PLACE_HOLDER();
    sprintf(strbuf, "lb$%d", i+1);
    lb_name = pt_gen_ident(strbuf);

    /* create  "lb$1 = ((L-1)/n$p)+1" */

    minus1 = gen_offset(iset->set.loops[depth].lo.ast, -1);
    node = pt_gen_div(minus1, pt_gen_int(dh->numprocs));
    gen_put_parens(node,1);
    node = pt_gen_add(node, pt_gen_int(1));
    node = gen_ASSIGNMENT(AST_NIL, tree_copy(lb_name), node);
    list_insert_before(loop, node);

    /* create "if (my$p < MOD(L-1,n$p)) lb$1 = lb$1 + 1" */

    node2 = pt_gen_add(tree_copy(lb_name),pt_gen_int(1));
    node2 = gen_ASSIGNMENT(AST_NIL, tree_copy(lb_name), node2);
    node2 = list_create(node2);

    node = tree_copy(minus1);
    gen_put_parens(node,0);
    node = list_create(node);
    node = list_insert_last(node, pt_gen_int(dh->numprocs));
    node = pt_gen_invoke("MOD", node);
    node = gen_BINARY_LT(ast_get_logical_myproc(dh),node);
    node = gen_LOGICAL_IF(AST_NIL,node,node2);
    list_insert_before(loop, node);
  }
  else
  {
    lb_name = pt_gen_int(iset->set.loops[depth].lo.val);
  }

  /* create symbolic upper bound if needed */

  if ((fli->cyclic_type == Citer_all) || 
      (fli->cyclic_type == Citer_up))
  {
    for (i = 0; ub[i] != AST_NIL; i++)
      ;
    lb[i] = gen_PLACE_HOLDER();
    sprintf(strbuf, "ub$%d", i+1);
    ub_name = pt_gen_ident(strbuf);

    /* create  "ub$1 = ((U-1)/n$p)+1" */

    minus1 = gen_offset(iset->set.loops[depth].up.ast, -1);
    node = pt_gen_div(minus1, pt_gen_int(dh->numprocs));
    gen_put_parens(node,1);
    node = pt_gen_add(node, pt_gen_int(1));
    node = gen_ASSIGNMENT(AST_NIL, tree_copy(ub_name), node);
    list_insert_before(loop, node);

    /* create "if (my$p > MOD(U-1,n$p)) ub$1 = ub$1 - 1" */

    node2 = pt_gen_sub(tree_copy(ub_name),pt_gen_int(1));
    node2 = gen_ASSIGNMENT(AST_NIL, tree_copy(ub_name), node2);
    node2 = list_create(node2);

    node = tree_copy(minus1);
    gen_put_parens(node,0);
    node = list_create(node);
    node = list_insert_last(node, pt_gen_int(dh->numprocs));
    node = pt_gen_invoke("MOD", node);
    node = gen_BINARY_GT(ast_get_logical_myproc(dh),node);
    node = gen_LOGICAL_IF(AST_NIL,node,node2);
    list_insert_before(loop, node);
  }
  else
  {
    ub_name = pt_gen_int(iset->set.loops[depth].up.val);
  }

  /* if loop has neg step, reverse bounds */

  tree_replace(iset->set.loops[depth].lo.ast, 
               iset->set.loops[depth].rev ? ub_name : lb_name);

  tree_replace(iset->set.loops[depth].up.ast, 
               iset->set.loops[depth].rev ? lb_name : ub_name);

}
/*-------------------------------------------------------------------

  gen_user_bounds()    Generate bounds for loop w/ USER distribution

  12/3/93 RvH: For now, assume value based decomposition
*/

static void
gen_user_bounds(Dist_Globals *dh,
		AST_INDEX     loop,
		FortD_LI     *fli,
		AST_INDEX    *lb, 
		AST_INDEX    *ub)
{
  Iter_set  *iset;
  int        depth;
  AST_INDEX  ub_name;

  iset = fli->sgroup.groups[0]->iset;
  depth = fli->depth;

  /* Create symbolic upper bound */

  if (iset->set.loops[depth].up.type != Expr_simple_sym)
  {
    printf("gen_user_bounds(): Expected symbolic upper bound ...\n");
    return;
  }

  ub_name = pt_gen_ident(iset->set.loops[depth].up.str);
  tree_replace(iset->set.loops[depth].up.ast, ub_name);
}


/*-------------------------------------------------------------------

  guard_nonloop()   Insert guard for each group of non-loop statements 

*/

static void
guard_nonloop(Dist_Globals *dh)
{
  int i, j;
  AST_INDEX place_holder, node, guard;
  Iter_set *iset;

  place_holder = gen_PLACE_HOLDER();

  for (i = 0; i < dh->sgroup.group_num; i++)
  {
    iset = dh->sgroup.groups[i]->iset;

    if (!iset || !iset->oneproc)
      continue;

    list_insert_before(dh->sgroup.groups[i]->stmt[0], place_holder);

    node = list_create(AST_NIL);

    for (j = dh->sgroup.groups[i]->size - 1; j >= 0; j--)
    {
      list_remove_node(dh->sgroup.groups[i]->stmt[j]);
      node = list_insert_first(node, dh->sgroup.groups[i]->stmt[j]);
    }

    guard = gen_BINARY_EQ(ast_get_logical_myproc(dh), 
                          pt_gen_int(iset->proc[0]));
    node = gen_GUARD(AST_NIL, guard, node);
    node = gen_IF(AST_NIL, AST_NIL, list_create(node));
    list_insert_before(place_holder, node);
    list_remove_node(place_holder);

    node = ast_merge_ifs(node); /* merge with preceding IF stmt if legal */
    dh->sgroup.groups[i]->guard = node;    /* store guard */
  }

}


/*-------------------------------------------------------------------

  guard_loop()   Insert guard for each group of 
                 statements in loop as needed.
*/

static void
guard_loop(Dist_Globals *dh, FortD_LI *fli)
{
  int i, j, lvl, idx;
  AST_INDEX ph, node, out, guard, slist[MAX_SGROUP_SIZE];
  Iter_set *iset;

  ph = gen_PLACE_HOLDER();

  for (i = 0; i < fli->sgroup.group_num; i++)
  {
    iset = fli->sgroup.groups[i]->iset;
    if ((iset == fli->iset) || 
         !is_diff_isets(iset, fli->iset, fli->depth))
      continue;

    /* if iteration set for statement group differs    */
    /* from that for loop, must insert guard for group */

    lvl = fli->depth + 1;
    idx = 0;

    /* collect all stmts in group at proper loop level */

    for (j = 0; j < fli->sgroup.groups[i]->size; j++)
    {
      node = fli->sgroup.groups[i]->stmt[j];

      /* grab biggest scope that encloses stmt group */
      while (true)
      {
        out = tree_out(node);
        if ((loop_level(out) < lvl) || 
            !is_common_sgroup(out, fli->sgroup.groups[i]))
          break;
        node = out;
      }
      if (!idx || (node != slist[idx-1]))
        slist[idx++] = node;
    }

    /* put stmts in body of new IF, replace with IF stmt */

    node = list_create(AST_NIL);
    list_insert_before(slist[0], ph);

    while (idx--)
    {
        list_remove_node(slist[idx]);
        node = list_insert_first(node, slist[idx]);
    }

    if (fli->localized)
      printf("guard_loop(): guard for localized loop\n");

    guard = gen_owner(dh, iset->lhs, iset->lhs_sp, iset->sinfo);

    guard = gen_GUARD(AST_NIL, guard, node);
    guard = gen_IF(AST_NIL, AST_NIL, list_create(guard));
    list_insert_before(ph, guard);
    list_remove_node(ph);

    node = ast_merge_ifs(node); /* merge with preceding IF stmt if legal */
    fli->sgroup.groups[i]->guard = node;    /* store guard */
  }

  tree_free(ph);
}

/*----------------------------------------------------------------

  is_common_sgroup()    Whether stmts included in a compound
                        stmt belong to the same statement group

*/

static Boolean 
is_common_sgroup(AST_INDEX stmt, S_group *sgroup)
{
  struct sgroup_param sparam;

  sparam.sgroup = sgroup;
  sparam.same = true;
  walk_statements(stmt, LEVEL1, (WK_STMT_CLBACK)common_sgroup, NULL, (Generic)&sparam);
  return sparam.same;
}

/*----------------------------------------------------------------

  common_sgroup()      Helper function for is_common_sgroup()

*/

static int 
common_sgroup(AST_INDEX stmt, int level, struct sgroup_param *sparam)
{
  int i;

  if (is_assignment(stmt))
  {
    for (i = 0; i < sparam->sgroup->size; i++)
    {
      if (stmt == sparam->sgroup->stmt[i])
        return WALK_CONTINUE;
    }

    sparam->same = false;
    return WALK_ABORT;
  }
  return WALK_CONTINUE;
}


/* eof */
