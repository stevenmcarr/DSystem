/* $Id: coll_comm.ansi.c,v 1.22 1997/03/11 14:28:14 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/



/*---------------------------------------------------------------------

    coll_comm.c    Performs analysis for recognizing and 
                   partitioning reductions

    Author : Seema Hiranandani 

*/

#include <libs/fortD/codeGen/private_dc.h>

struct cc_param
{
  Dist_Globals *dh;
  AST_INDEX node;
  Boolean result;
  Reduc_set *reduc_s;
};

/*-------------------extern declarations-------------------*/

EXTERN(AST_INDEX, dt_ast_stmt,(AST_INDEX node));
EXTERN(Iter_set, *iter_build_assign,(Dist_Globals *dh, AST_INDEX lhs, AST_INDEX loop));
EXTERN(Boolean, ast_equiv,(AST_INDEX left, AST_INDEX right));

/*-------------------global declarations-------------------*/

Iter_set *iter_build_reduction(Dist_Globals *dh, AST_INDEX lsh, AST_INDEX loop);
AST_INDEX get_dist_ref(Dist_Globals *dh, AST_INDEX expr);

/*-------------------local declarations-------------------*/

STATIC(Reduc_set, *find_reduc,(Dist_Globals *dh, AST_INDEX stmt, AST_INDEX lhs));
STATIC(Reduc_set, *find_reduc_minmax,(Dist_Globals *dh, AST_INDEX lhs, AST_INDEX rhs));
STATIC(Iter_set, *build_ld_balance_iter_set,(Dist_Globals *dh, AST_INDEX lhs,
                                             AST_INDEX loop));
STATIC(void, put_reduc,(Dist_Globals *dh, Reduc_set *reduc_s, AST_INDEX lhs));

STATIC(Reduc_set, *dc_alloc_reduc_set,(Dist_Globals *dh));
STATIC(AST_INDEX, get_reduc_loop,(AST_INDEX *loops, int num, AST_INDEX source));
STATIC(int, check_init_reduc,(AST_INDEX stmt, int level, struct cc_param *param));
STATIC(Boolean, is_local_reduc,(Dist_Globals *dh, Iter_set *iset));

STATIC(Boolean, ast_in_expr,(AST_INDEX node, AST_INDEX expr));
STATIC(int, compare_expr,(AST_INDEX node, struct cc_param *param));
STATIC(int, dist_ref,(AST_INDEX node, struct cc_param *param));
STATIC(void, check_reduction_init,(Dist_Globals *dh, Reduc_set *reduc_s));
STATIC(void, check_minmax_init,(AST_INDEX lhs, AST_INDEX *slist, Reduc_set *reduc_s));


/********************/
/* global functions */
/********************/

/*--------------------------------------------------------------------

  iter_build_reduction()    Build iteration set for reduction stmt

  Returns  NULL   if stmt is not a reduction
           iset   if stmt is a reduction
*/

Iter_set *
iter_build_reduction(Dist_Globals *dh, AST_INDEX lhs, AST_INDEX loop)
{
  Iter_set *iset, *iset2;
  SNODE *sp;
  Reduc_set *reduc_s;
  AST_INDEX stmt, *ast_ptr, init_stmts[32]; /* init stmts for reduction */
  int lvl;

  init_stmts[0] = AST_NIL;

  /* if lhs is a distributed array, for now don't handle as reduction */
  if ((sp = findadd2(lhs, 0, 0, dh)) && sp && sp->decomp)
    return NULL;

  /* check whether iset assigned from previously identified reduction */
  stmt = tree_out(lhs);
  iset = (Iter_set *) get_info(dh->ped, stmt, type_dc);
  if (iset != (Iter_set *) NO_DC_INFO)
    return iset;

  if (!(reduc_s = find_reduc(dh, stmt, lhs)))
    return NULL;

  if (reduc_s->ref != AST_NIL)  /* distributed rhs found */
  {
    iset = iter_build_assign(dh, reduc_s->ref, loop);

    /* store away symbol table ptr for "virtual" lhs */
    iset->lhs_sp = findadd2(reduc_s->ref, 0, 0, dh);
    iset->lhs = reduc_s->ref;
  }
  else   /* no distributed rhs found, purely local computation  */
  {
    return NULL; 
  }

  if ((reduc_s->rtype == FD_REDUC_MIN) || (reduc_s->rtype == FD_REDUC_MAX))
  {
    check_minmax_init(lhs, init_stmts, reduc_s);
  }
  else
  {
    put_reduc(dh, reduc_s, lhs);        /* find loop level of reduction */
    check_reduction_init(dh, reduc_s);  /* pre-existing initialization? */
  }

  iset->reduc_set = reduc_s;
  iset->reduc_set->local = is_local_reduc(dh, iset);

  if (!dh->opts.relax_owner_computes)   /* rely on relax owner computes */
    iset->allproc = true;               /* to do these optimizations    */

  /* also assign same iterset to initialization stmts */

  for (ast_ptr = init_stmts; *ast_ptr != AST_NIL; ast_ptr++)
  {
    iset2 = (Iter_set *) dc_get_mem(dh, sizeof(Iter_set));
    bcopy(iset, iset2, sizeof(Iter_set));
    lvl = loop_level(*ast_ptr);
    iset2->set.level = lvl;
    iset2->pre_set.level = lvl;
    iset2->post_set.level = lvl;
    put_info(dh->ped, *ast_ptr, type_dc, (Generic)iset2); /* store result */
  }

  return iset;
}

/*--------------------------------------------------------------------

  get_dist_ref()  Find distributed array variable in expression

*/

AST_INDEX
get_dist_ref(Dist_Globals *dh, AST_INDEX expr)
{
  struct cc_param param;

  param.node = AST_NIL;
  param.dh = dh;

  walk_expression(expr, (WK_EXPR_CLBACK)dist_ref, NULL, (Generic)&param);

  return param.node;
}


/*******************/
/* local functions */
/*******************/

/*--------------------------------------------------------------------

  ast_in_expr()    Whether AST "node" occurs in AST "expr"

*/

static Boolean
ast_in_expr(AST_INDEX node, AST_INDEX expr)
{
  struct cc_param param;

  param.node = node;
  param.result = false;

  walk_expression(expr, (WK_EXPR_CLBACK)compare_expr, NULL, (Generic)&param);

  return param.result;
}


/*--------------------------------------------------------------------

  dc_alloc_reduc_set()  build the reduction set

*/

static Reduc_set *
dc_alloc_reduc_set(Dist_Globals *dh)
{
  Reduc_set *reduc_s;

  reduc_s = (Reduc_set *) dc_get_mem(dh, sizeof(Reduc_set));
  bzero(reduc_s, sizeof(Reduc_set));

  reduc_s->rtype = FD_REDUC_NONE;
  reduc_s->lhs = AST_NIL;
  reduc_s->ref = AST_NIL;
  reduc_s->loop = AST_NIL;
  reduc_s->lhs_name = NULL;
  reduc_s->done = false;
  reduc_s->init = false;
  reduc_s->local = false;
  reduc_s->aux[0] = AST_NIL;
  return reduc_s;
}

/*--------------------------------------------------------------------

  find_reduc()  driver routine for identifying reductions

  There must be only one occurence of the lhs in the rhs.

*/

static Reduc_set *
find_reduc(Dist_Globals *dh, AST_INDEX stmt, AST_INDEX lhs)
{
  AST_INDEX rhs, rhs_ref, term1, term2;
  Reduc_set *reduc_s;
  Reduc_type rtype;

  /* determine whether part of the rhs matches the lhs    */
  rhs_ref = AST_NIL;
  rhs = gen_ASSIGNMENT_get_rvalue(stmt);

  switch (gen_get_node_type(rhs))
  {
  case GEN_BINARY_TIMES:
    rtype = FD_REDUC_TIMES;
    term1 = gen_BINARY_TIMES_get_rvalue1(rhs);
    term2 = gen_BINARY_TIMES_get_rvalue2(rhs);
    break;

  case GEN_BINARY_DIVIDE:
    rtype = FD_REDUC_DIV;
    term1 = gen_BINARY_DIVIDE_get_rvalue1(rhs);
    term2 = gen_BINARY_DIVIDE_get_rvalue2(rhs);
    break;

  case GEN_BINARY_PLUS:
    rtype = FD_REDUC_PLUS;
    term1 = gen_BINARY_PLUS_get_rvalue1(rhs);
    term2 = gen_BINARY_PLUS_get_rvalue2(rhs);
    break;

  case GEN_BINARY_MINUS:
    rtype = FD_REDUC_MINUS;
    term1 = gen_BINARY_MINUS_get_rvalue1(rhs);
    term2 = gen_BINARY_MINUS_get_rvalue2(rhs);
    break;

  case GEN_BINARY_AND:
    rtype = FD_REDUC_AND;
    term1 = gen_BINARY_AND_get_rvalue1(rhs);
    term2 = gen_BINARY_AND_get_rvalue2(rhs);
    break;

  case GEN_BINARY_OR:
    rtype = FD_REDUC_OR;
    term1 = gen_BINARY_OR_get_rvalue1(rhs);
    term2 = gen_BINARY_OR_get_rvalue2(rhs);
    break;

  case GEN_IDENTIFIER:
  case GEN_SUBSCRIPT:
  default:
    return find_reduc_minmax(dh, lhs, rhs);
  }

  if (ast_equiv(lhs, term1))
  {
    if (ast_in_expr(lhs, term2))
      return NULL;
    rhs_ref = get_dist_ref(dh, term2);
  }
  else if (ast_equiv(lhs, term2))
  {
    if (ast_in_expr(lhs, term1))
      return NULL;
    rhs_ref = get_dist_ref(dh, term1);
  }
  else 
    return NULL;

  reduc_s = dc_alloc_reduc_set(dh);
  reduc_s->rtype = rtype;
  reduc_s->lhs = lhs;
  reduc_s->ref = rhs_ref;

  if (is_subscript(lhs))                 /* get name of lhs of reduction */
    lhs = gen_SUBSCRIPT_get_name(lhs);
  reduc_s->lhs_name = gen_get_text(lhs); 

  return reduc_s;
}

/*--------------------------------------------------------------------

  check_minmax_init()  Find initialization stmts for min/max reduction

  Also check for auxiliary assignments (e.g. location of min/max)
*/

static void
check_minmax_init(AST_INDEX lhs, AST_INDEX *slist, Reduc_set *reduc_s)
{
  AST_INDEX stmt, a, a_lhs, aux_init[MAX_RED_AUX];
  int i, aux_num, aux_init_num;

  reduc_s->init = true;  /* always initialized */

  aux_num = 0;
  aux_init_num = 0;

  /* find all auxiliary assignments for min/max reduction */

  stmt = tree_out(lhs);
  for (a = list_next(stmt); a != AST_NIL; a = list_next(a))
  {
    if (is_assignment(a))
      reduc_s->aux[aux_num++] = gen_ASSIGNMENT_get_lvalue(a);
    else if (!is_comment(a))
      printf("check_minmax_init(): unidentified stmt in reduction\n");
  }
  for (a = list_prev(stmt); a != AST_NIL; a = list_prev(a))
  {
    if (is_assignment(a))
      reduc_s->aux[aux_num++] = gen_ASSIGNMENT_get_lvalue(a);
    else if (!is_comment(a))
      printf("check_minmax_init(): unidentified stmt in reduction\n");
  }
  reduc_s->aux[aux_num] = lhs;  /* for consistency, treat lhs as aux */
  reduc_s->aux[aux_num+1] = AST_NIL;  /* terminate list */

  stmt = tree_out(stmt);
  if (!is_guard(stmt))
  {
    printf("check_minmax_init(): not min/max reduction?\n");
    return;
  }

  stmt = tree_out(stmt);
  if (!is_if(stmt) && !is_logical_if(stmt))
  {
    printf("check_minmax_init(): not min/max reduction?\n");
    return;
  }

  *slist++ = stmt;  /* assign reduction iterset to IF stmt */

  stmt = tree_out(stmt);
  if (!is_loop(stmt))
  {
    printf("check_minmax_init(): reduction not in loop\n");
    return;
  }
  reduc_s->loop = stmt;

  /* match initializations stmts against body of reduction */

  for (a = list_prev(stmt); a != AST_NIL; a = list_prev(a))
  {
    if (is_assignment(a))
    {
      a_lhs = gen_ASSIGNMENT_get_lvalue(a);
      for (i = 0; i <= aux_num; i++)
      {
        if (ast_equiv(a_lhs,reduc_s->aux[i]))
        {
          aux_init[aux_init_num++] = a_lhs;
          break;
        }
      }
      if (i > aux_num)
        break;
    }
  }

  /* return initialization stmts found */

  for (i = 0; i < aux_num; i++)
    *slist++ = tree_out(reduc_s->aux[i]);
  for (i = 0; i < aux_init_num; i++)
    *slist++ = tree_out(aux_init[i]);
  *slist = AST_NIL;
}


/*--------------------------------------------------------------------

  find_reduc_minmax()  driver routine for identifying min/max reductions

  Look for reductions of the form:

   a = c(1)
   l = 1                   a = c(1)
   do i = 2,n              do i = 2,n
     if (a < c(i)) then      if (a < c(i)) a = c(i)
       a = c(i)            enddo
       l = i
     endif
   enddo

*/

static Reduc_set *
find_reduc_minmax(Dist_Globals *dh, AST_INDEX lhs, AST_INDEX rhs)
{
  AST_INDEX stmt, test, term1, term2, guard, slist;
  Reduc_set *reduc_s;

  stmt= tree_out(lhs);
  if (!is_assignment(stmt))
    return NULL;

  stmt= tree_out(stmt);
  if (!is_guard(stmt))
    return NULL;

  stmt= tree_out(stmt);
  if (is_if(stmt))
  {
    guard = gen_IF_get_guard_LIST(stmt);
    guard = list_first(guard);
    test = gen_GUARD_get_rvalue(guard);
    slist = gen_GUARD_get_stmt_LIST(guard);
    if (list_next(guard) != AST_NIL)
      return NULL;
  }
  else if (is_logical_if(stmt))
  {
    test = gen_LOGICAL_IF_get_rvalue(stmt);
    slist = gen_LOGICAL_IF_get_stmt_LIST(stmt);
  }
  else
    return NULL;

  /* match term1 to larger term, term2 to smaller term */
  switch (gen_get_node_type(test))
  {
  case GEN_BINARY_GT:
    term1 = gen_BINARY_GT_get_rvalue1(test);
    term2 = gen_BINARY_GT_get_rvalue2(test);
    break;

  case GEN_BINARY_GE:
    term1 = gen_BINARY_GE_get_rvalue1(test);
    term2 = gen_BINARY_GE_get_rvalue2(test);
    break;

  case GEN_BINARY_LT:
    term2 = gen_BINARY_LT_get_rvalue1(test);
    term1 = gen_BINARY_LT_get_rvalue2(test);
    break;

  case GEN_BINARY_LE:
    term2 = gen_BINARY_LE_get_rvalue1(test);
    term1 = gen_BINARY_LE_get_rvalue2(test);
    break;

  default:
    return NULL;
  }

  if (ast_equiv(term1,lhs))
  {
    if (!ast_equiv(term2,rhs))
      return NULL;

    reduc_s = dc_alloc_reduc_set(dh);
    reduc_s->rtype = FD_REDUC_MIN;
    
  }
  else if (ast_equiv(term2,lhs))
  {
    if (!ast_equiv(term1,rhs))
      return NULL;

    reduc_s = dc_alloc_reduc_set(dh);
    reduc_s->rtype = FD_REDUC_MAX;
  }
  else
    return NULL;

  reduc_s->lhs = lhs;
  reduc_s->ref = get_dist_ref(dh, rhs);

  if (is_subscript(lhs))                 /* get name of lhs of reduction */
    lhs = gen_SUBSCRIPT_get_name(lhs);
  reduc_s->lhs_name = gen_get_text(lhs); 

  return reduc_s;
}


/*--------------------------------------------------------------------

  dist_ref()  Helper function for get_dist_ref()

*/

static int
dist_ref(AST_INDEX node, struct cc_param *param)
{
  SNODE *sp;

  if (!is_subscript(node))
    return WALK_CONTINUE;

  if ((sp = findadd2(node, 0, 0, param->dh)) && sp && sp->decomp)
  {
    param->node = node;
    return WALK_ABORT;
  }

  return WALK_CONTINUE;
}


/********************************************************************/
/* this function finds all the true dependence whose source is the  */
/* reduction statement. The loop after which the communication call */
/* may be placed is the deepest loop carrying this dependence       */
/********************************************************************/
static void
put_reduc(Dist_Globals *dh, Reduc_set *reduc_s, AST_INDEX lhs)
{
  int ref, num, depth, level, i;
  AST_INDEX sink, source, sink_stmt, source_stmt, source_temp, loops[MAXLOOP];
  AST_INDEX loop_list[MAXLOOP];
  EDGE_INDEX edge;
  DG_Edge *Earray;

  source = lhs;
  source_stmt = dt_ast_stmt(source);
  ref = get_info(dh->ped, source, type_levelv);

  for (i = 0; i < MAXLOOP; ++i)
  {
    loops[i] = AST_NIL;
    loop_list[i] = AST_NIL;
  }

  /* look at all dependences edges with id as src */

  Earray = dg_get_edge_structure(PED_DG(dh->ped));
  num = 0;

  for (edge = dg_first_src_ref(PED_DG(dh->ped), ref);
       edge >= 0;
       edge = dg_next_src_ref(PED_DG(dh->ped), edge))
  {

    if (Earray[edge].type == dg_true)
    {

      /* get the sink and check if the statement is different */
      sink = Earray[edge].sink;
      sink_stmt = dt_ast_stmt(sink);

      if (sink_stmt != source_stmt)
      {
        /* get all loops around source */

        source_temp = source_stmt;
        depth = 0;
        while (source_temp != AST_NIL)
        {
          source_temp = tree_out(source_temp);
          if (is_loop(source_temp))
            loops[depth++] = source_temp;
        }

        /* compare with loops around sink */
        /****************************************************/
        /* get the  deepest loop surrounding the source     */
        /* and the sink.                                    */
        /* If there a deeper loop surrounding the source    */
        /* get that. communicate the reduction outside of   *
        /* this loop. If there is no deeper loop then       */
        /* communication needs to be inserted inside this   */
        /* loop between the source and the sink of the      */
        /* dependence                                       */
        /****************************************************/
        while (sink != AST_NIL)
        {
          sink = tree_out(sink);
          for (i = 0; i < depth; i++)
          {
            if (sink == loops[i])
            {
              level = loop_level(loops[i]);

              /* if not a loop independent dependence on the innermost loop */
              /* get the next deepest loop surrounding the source.          */
              /* communication may be  built outside this loop              */

              if (level != depth)
              {
                loop_list[num++] = loops[i - 1];
                break;
              }
              else
                die_with_message("Loop-independent cross-processor deps");
            }
          }
        }
      }
    }
  }
  reduc_s->loop = get_reduc_loop(loop_list, num, source_stmt);
}


/*******************************************************************/
/* this function returns the loop after which the reduction        */
/* collective communication primitive can be placed                */
/*******************************************************************/
static AST_INDEX
get_reduc_loop(AST_INDEX *loops, int num, AST_INDEX source)
{
  Boolean done = false;
  AST_INDEX stmt;
  int i, min_level;

  if (!num)
  {
    while (!done)
    {
      source = tree_out(source);
      if (is_loop(source))
      {
        if (loop_level(source) == 1)
        {
          done = true;
          return (source);
        }
      }
    }
  }

  else
  {
    min_level = 9999;
    for (i = 0; i < num; ++i)
    {
      if (loop_level(loops[i]) < min_level)
      {
        min_level = loop_level(loops[i]);
        stmt = loops[i];
      }
    }
    return (stmt);
  }

  printf("get_reduc_loop(): unreachable code\n");
  return AST_NIL;
}


/*******************************************************************/
/* this function determines whether an initialization is needed    */
/*******************************************************************/
static void
check_reduction_init(Dist_Globals *dh, Reduc_set *reduc_s)
{
  AST_INDEX loop, node;
  Boolean done = false;
  struct cc_param param;

  param.dh = dh;
  param.reduc_s = reduc_s;

  loop = reduc_s->loop;
  /* if it's not the outermost loop */

  if (loop_level(loop) != 1)
  {
    while (!done)
    {
      loop = tree_out(loop);
      if (is_loop(loop))
        done = true;
    }
    walk_statements(loop, LEVEL1, (WK_STMT_CLBACK)check_init_reduc, NULL, 
                    (Generic)&param);
  }

  node = list_prev(loop);
  while (!reduc_s->init && (node != AST_NIL))
  {
    if (is_assignment(node))
      (void) check_init_reduc(node, 1, &param);
    else if (!is_comment(node))
       return;
    node = list_prev(node);
  }
}

/*******************************************************************/
/* this function is called by walk statements to check if there is */
/* an initialization for a reduction                               */
/*******************************************************************/
static int
check_init_reduc(AST_INDEX stmt, int level, struct cc_param *param)
{
  AST_INDEX rhs;
  int int_val;
  double real_val;
  SNODE *sp;
  Reduc_set *reduc_s;
  char *str;

  reduc_s = param->reduc_s;

  if (stmt == reduc_s->loop)
    return (WALK_ABORT);

  if (!is_assignment(stmt))
    return (WALK_CONTINUE);

  if (!ast_equiv(gen_ASSIGNMENT_get_lvalue(stmt), reduc_s->lhs))
    return WALK_CONTINUE;

  rhs = gen_ASSIGNMENT_get_rvalue(stmt);

  sp = findadd2(reduc_s->lhs, 0, 0, param->dh);
  switch (reduc_s->rtype)
  {
    case FD_REDUC_PLUS:
      switch (sp->fform)
      {
        case REAL:
        case DOUBLE_P:
          if (is_constant(rhs))
          {
            str = gen_get_text(rhs);
            real_val = atof(str);
            if ((real_val == 0.0) || !strcmp(str,"0.0") ||   
                !strcmp(str,"0.") || !strcmp(str,"0"))
              reduc_s->init = true;
          }
          break;

        case INTTYPE:
          if (is_constant(rhs))
          {
            int_val = atoi(gen_get_text(rhs));
            if (!int_val)
              reduc_s->init = true;
          }
          break;
      }
      break;

    case FD_REDUC_TIMES:
      switch (sp->fform)
      {
        case REAL:
        case DOUBLE_P:
          if (is_constant(rhs))
          {
            str = gen_get_text(rhs);
            real_val = atof(str);
            if ((real_val == 1.0) || !strcmp(str,"1.0") ||   
                !strcmp(str,"1.") || !strcmp(str,"1"))
              reduc_s->init = true;
          }
          break;

        case INTTYPE:
          if (is_constant(rhs))
          {
            int_val = atoi(gen_get_text(rhs));
            if (int_val == 1)
              reduc_s->init = true;
          }
          break;

      }
      break;
  }

  return WALK_CONTINUE;
}


/*--------------------------------------------------------------------


*/

static Iter_set *
build_ld_balance_iter_set(Dist_Globals *dh, AST_INDEX lhs, AST_INDEX loop)
{
  die_with_message("build_ld_balance_iter_set(): Not yet supported");
  return NULL;
}


/*--------------------------------------------------------------------

  compare_expr()  Helper function for ast_in_expr()

*/

static int
compare_expr(AST_INDEX node, struct cc_param *param)
{
  if (ast_equiv(node, param->node))
  {
    param->result = true;
    return WALK_ABORT;
  }
  return WALK_CONTINUE;
}


/*--------------------------------------------------------------------

  is_local_reduc()    Whether reduction is performed by one processor

  { nonlocal }            { local }
  distribute A(block)     distribute A(:,block) 
  do i=1,n                do i=1,n
   s = s + A(i)             do j=1,n
  enddo                       s = s + A(j,i)
                            enddo
                          enddo
*/

static Boolean
is_local_reduc(Dist_Globals *dh, Iter_set *iset)
{
  int i, j, maxlvl, lvl;
  Subs_list *slist;

  if (!is_subscript(iset->lhs))
    return false;

  if (iset->reduc_set->loop == AST_NIL)
    return false;

  slist = (Subs_list *) get_info(dh->ped, iset->lhs, type_ref);
  maxlvl = slist->loop_nest->level;
  lvl = loop_level(iset->reduc_set->loop)-1;

  for (i = 0; i < slist->dims; i++)
  {
    if (sp_is_part(iset->lhs_sp, i) != FD_DIST_LOCAL)
    {
      if (slist->subs[i].stype >= SUBS_SYM)
        return false;

      for (j = lvl; j < maxlvl; j++)
      {
        if (slist->subs[i].coeffs[j])
          return false;
      }
    }
  }

  return true;
}


