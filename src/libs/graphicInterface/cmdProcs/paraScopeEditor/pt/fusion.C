/* $Id: fusion.C,v 1.3 1997/06/27 17:55:17 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/***************************************************************************/
/*                                                                         */
/*   fusion.c - code to perform loop fusion                                  */
/*                                                                         */
/*   Sep 1990 cwt  created                                                 */
/*                                                                         */
/***************************************************************************/

#include <string.h>
#include <include/bstring.h>
#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/groups.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/pt/private_pt.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>
#include <libs/moduleAnalysis/dependence/dependenceTest/dep_dt.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>

/*--------*/
/* macros */

#define SYM_EQ2(x,y)      ((*(x) == *(y)) && !strcmp((x), (y)))

struct fuse_params
{
  PedInfo ped;
  AST_INDEX loop1;
  AST_INDEX loop2;
  int fuse_result;
  int level;
  DG_Edge *DG;
  Boolean carry;
};

static int fuse_id;     /* generate unique id   */
STATIC(AST_INDEX, enclosing_loop,(AST_INDEX x, AST_INDEX y));
STATIC(int, check_fuse,(AST_INDEX stmt, int level, struct fuse_params *param));
STATIC(int, fuse_loops,(AST_INDEX stmt, int level, struct fuse_params *param));

/*-------------------------------------------------------------------

  pt_fuse_test()    Determine legality, estimate profitablity

  Returns:

  */

int
pt_fuse_test(PedInfo ped, AST_INDEX loop)
{
  AST_INDEX loop1, loop2;
  AST_INDEX control;
  AST_INDEX lo_bound1, up_bound1, step1;
  AST_INDEX lo_bound2, up_bound2, step2;
  int temp1, temp2;
  struct fuse_params f_param;

  /*----------------------------------------------*/
  /* first check whether following loop exists    */

  loop1 = loop;
  loop2 = list_next(loop1);
  while (is_comment(loop2))
    loop2 = list_next(loop2);

  if ((loop2 == AST_NIL) || !is_loop(loop2))
    return FUSE_LOOP;

  /*--------------------------*/
  /* now check loop bounds    */

  control = gen_DO_get_control(loop1);
  lo_bound1 = gen_INDUCTIVE_get_rvalue1(control);
  up_bound1 = gen_INDUCTIVE_get_rvalue2(control);
  step1 = gen_INDUCTIVE_get_rvalue3(control);

  control = gen_DO_get_control(loop2);
  lo_bound2 = gen_INDUCTIVE_get_rvalue1(control);
  up_bound2 = gen_INDUCTIVE_get_rvalue2(control);
  step2 = gen_INDUCTIVE_get_rvalue3(control);

  /* compare lower bounds */

  if (pt_eval(lo_bound1, &temp1) || pt_eval(lo_bound2, &temp2))
  {
    if (NOT(pt_expr_equal(lo_bound1, lo_bound2)))
      return FUSE_BOUND;
  }
  else if (temp1 != temp2)
  {
    return FUSE_BOUND;
  }

  /* compare upper bounds */

  if (pt_eval(up_bound1, &temp1) || pt_eval(up_bound2, &temp2))
  {
    if (NOT(pt_expr_equal(up_bound1, up_bound2)))
      return FUSE_BOUND;
  }
  else if (temp1 != temp2)
  {
    return FUSE_BOUND;
  }

  /* compare step */

  temp1 = temp2 = 1;    /* default if no step specified */

  if ((step1 != AST_NIL) && pt_eval(step1, &temp1))
  {
    /* 1st step is symbolic, 2nd step is const  */
    if ((step2 == AST_NIL) || NOT(pt_eval(step2, &temp2)))
      return FUSE_STEP;

    /* 1st & 2nd step have different symbolic expressions   */
    if (NOT(pt_expr_equal(step1, step2)))
      return FUSE_STEP;
  }
  else if ((step2 != AST_NIL) && pt_eval(step2, &temp2))
  {
    /* 1st step is const, 2nd step is symbolic  */
    return FUSE_STEP;
  }
  else if (temp1 != temp2)
  {
    /* 1st & 2nd step have different const values   */
    return FUSE_STEP;
  }

  /*----------------------------------------------*/
  /* now check for fusion-preventing dependences  */

  /* give up for now for loops not enclosed in same loop */
  if (enclosing_loop(loop1, loop2) == AST_NIL)
    return pt_fuse_outer((FortTree)PED_FT(ped), PED_INFO(ped), PED_DT_INFO(ped), 
                         loop1, loop2);

  f_param.ped = ped;
  f_param.loop1 = loop1;
  f_param.loop2 = loop2;
  f_param.fuse_result = FUSE_OK;
  f_param.level = loop_level(loop1);
  f_param.DG = dg_get_edge_structure( PED_DG(ped));

  walk_statements(loop1, f_param.level, (WK_STMT_CLBACK)check_fuse, NULL, (Generic)&f_param);

  return f_param.fuse_result;
}


/*--------------------------------------------------------------------

    check_fuse()        Check for fusion-preventing dependences

    Only loop-independent deps with source in first loop need
    to be checked.  Any dependences carried on outer or inner
    loops will not be affected by fusion.  There cannot be any
    loop-independent dependences with sink in the first loop.

*/

static int
check_fuse(AST_INDEX stmt, int level, struct fuse_params* param)
{
  int vector;
  int edge_idx;
  int result;
  DG_Edge *Edge;
  AST_INDEX src, sink;
  Subs_list *src_subs;
  Subs_list *sink_subs;

  /* if no dependences on this statement, continue */

  if ((vector = get_info(param->ped, stmt, type_levelv)) == UNUSED)
    return WALK_CONTINUE;

  for (edge_idx = dg_first_src_stmt( PED_DG(param->ped), vector, LOOP_INDEPENDENT);
       edge_idx != UNUSED;
       edge_idx = dg_next_src_stmt( PED_DG(param->ped), edge_idx))
  {
    Edge = param->DG + edge_idx;

    /* check for TRUE/ANTI/OUTPUT deps */

    if ((Edge->type == dg_true) ||
        (Edge->type == dg_anti) ||
        (Edge->type == dg_output))
    {
      for (sink = Edge->sink; sink != AST_NIL; sink = tree_out(sink))
      {
        if (sink == param->loop2) /* if sink of dep enclosed by loop2 */
        {
          src_subs = (Subs_list *) 
             get_info(param->ped, dt_ast_sub(Edge->src), type_ref);
          sink_subs = (Subs_list *) 
             get_info(param->ped, dt_ast_sub(Edge->sink), type_ref);

          result = pt_fuse_prev(PED_DT_INFO(param->ped), src_subs, sink_subs,
               src_subs->loop_nest, sink_subs->loop_nest, param->level);

          if (result == FUSE_DEP_ILL)
          {
            param->fuse_result = FUSE_DEP_ILL;
            return WALK_ABORT;
          }
          else if (result == FUSE_OK_CARRY)
          {
            param->fuse_result = FUSE_OK_CARRY;
          }
        }
      }
    }
  }

  return WALK_CONTINUE;
}


/*--------------------------------------------------------------------

    pt_fuse_prev()  Determine whether dependence is fusion-preventing

    Analyze references as if loop was already fused at <level>

    Returns:         if Fusion results in:
        FUSE_OK            no fusion-preventing or carried dependence
        FUSE_OK_CARRY      carried dependence on fused loop
        FUSE_DEP_ILL       fusion-preventing dependence
*/


int
pt_fuse_prev(DT_info* dt, Subs_list* src, Subs_list* sink, 
             Loop_list* loops1, Loop_list* loops2, int level)
  /* level - level of loop being considered for fusion */
{
  DG_Edge Edge;
  int i;

  dt_analyze( dt, &Edge, NULL, src, sink, loops1, loops2, level, true);

  if (Edge.dt_type == DT_UNKNOWN)      /* unknown dep */
    return FUSE_DEP_ILL;

  if (Edge.dt_type == DT_NONE)         /* no dep */
    return FUSE_OK;

  if (gen_is_dt_ALL_EQ(&Edge))         /* only loop-indep dep */
    return FUSE_OK;

  /* check all loops enclosing loop being fused                */
  /* if any of them force dependences to be at an outer loop,  */
  /* then dependence is not fusion-preventing                  */

  for (i = 0; i < level - 1; i++)
  {
    if (Edge.dt_data[i] < DDATA_BASE)   /* dep direction */
    {
      /* if dep is carried on outer loop only */
      if ((Edge.dt_data[i] == DDATA_LT) || 
          (Edge.dt_data[i] == DDATA_GT) ||
          (Edge.dt_data[i] == DDATA_NE))
        return FUSE_OK;
    }
    else /* dep distance */
    {
      /* if non-zero distance, dep is carried on outer loop only */
      if (Edge.dt_data[i])    
        return FUSE_OK;
    }
  }

  /* Now check dependence actually at level of loop being fused.  If  */
  /* resulting dep reverses the source & sink, it's fusion-preventing */

  if (Edge.dt_data[i] < DDATA_BASE)   /* dep direction */
  {
    /* ">" is fusion-preventing */
    if ((Edge.dt_data[i] == DDATA_GT) || 
        (Edge.dt_data[i] == DDATA_GE) ||
        (Edge.dt_data[i] == DDATA_ANY) || 
        (Edge.dt_data[i] == DDATA_NE))
      return FUSE_DEP_ILL;

    /* even if not fusion-preventing, will cause loop-carried dependence */
    return FUSE_OK_CARRY;
  }
  else /* dep distance */
  {
    if (Edge.dt_data[i] < 0)    
      return FUSE_DEP_ILL;
    else if (Edge.dt_data[i] > 0)  /* dep not preventing, but is carried */
      return FUSE_OK_CARRY;
    else
      return FUSE_OK;
  }
}


/*-----------------------------------------------------------------------

	pt_fuse_outer()	  Discover effect of fusing two outermost loops

    Returns:         if Fusion results in:
        FUSE_OK            no fusion-preventing or carried dependence
        FUSE_OK_CARRY      carried dependence on fused loop
        FUSE_DEP_ILL       fusion-preventing dependence

*/
int
pt_fuse_outer(FortTree ft, SideInfo* infoPtr, DT_info* dt_info, 
              AST_INDEX loop1, AST_INDEX loop2)
{
  int i, j, result, edge_result;
  Subs_list **s1, **s2;
  int		 refCount1;
  int		 refCount2;
  Dg_ref_info	*refArray1;
  Dg_ref_info	*refArray2;

  result = FUSE_OK;

  dg_ref_info_and_count(ft, loop1, &refCount1, &refArray1 );  /* get refs in 1st loop */
  dg_ref_info_and_count(ft, loop2, &refCount2, &refArray2 );  /* get refs in 2nd loop */

  /* cache Subs_list info */
  s1 = (Subs_list **) get_mem(sizeof(Subs_list *) * refCount1, "dt");
  s2 = (Subs_list **) get_mem(sizeof(Subs_list *) * refCount2, "dt");
  bzero((char*)s1, sizeof(Subs_list *) * refCount1);
  bzero((char*)s2, sizeof(Subs_list *) * refCount2);

  for (i = 0; i < refCount1; i++)
  {
    for (j = 0; j < refCount2; j++)
    {
      /* Test if either ref is not USE and     */
      /* both refer to the same array variable */

      if ( ( (dg_ref_info_def(&(refArray1[i])) != T_USE) 
	    || (dg_ref_info_def(&(refArray2[j])) != T_USE) ) 
	  && is_subscript(tree_out( dg_ref_info_node(&(refArray1[i])) )) 
	  && (refArray1[i].index == refArray2[j].index) )
	  /**********************************************
	  && SYM_EQ2(dg_ref_info_sym(&(refArray1[i])), 
		     dg_ref_info_sym(&(refArray2[j])) ) )
	  **********************************************/
      {
        if (!s1[i])
        {
          s1[i] = (Subs_list *) dg_get_info( infoPtr, 
                     dt_ast_sub(dg_ref_info_node(&(refArray1[i]))), type_ref);
        }
        if (!s2[j])
        {
          s2[j] = (Subs_list *) dg_get_info( infoPtr, 
                     dt_ast_sub(dg_ref_info_node(&(refArray2[j]))), type_ref);
        }

        edge_result = pt_fuse_prev( dt_info, s1[i], s2[j], 
          s1[i]->loop_nest, s2[j]->loop_nest, 1);

        if (edge_result == FUSE_DEP_ILL) 
        {
          free_mem((void*)s1);
          free_mem((void*)s2);
	  dg_ref_info_free( refArray1 );
	  dg_ref_info_free( refArray2 );
          return FUSE_DEP_ILL;
        }
        else if (edge_result == FUSE_OK_CARRY)
          result = FUSE_OK_CARRY;
      }
    }
  }

  free_mem((void*)s1);
  free_mem((void*)s2);
  dg_ref_info_free( refArray1 );
  dg_ref_info_free( refArray2 );
  return result;
} /* end_pt_fuse_outer */


/*--------------------------------------------------------------------

    enclosing_loop()        Find deepest loop enclosing X and Y

    Returns AST_NIL if no such loop is found

*/

static AST_INDEX
enclosing_loop(AST_INDEX x, AST_INDEX y)
{
  AST_INDEX ptr, loops[MAXLOOP];
  int i, depth;

  depth = 0;
  for (ptr = x; ptr != AST_NIL; ptr = tree_out(ptr))
  {
    if (is_loop(ptr))
      loops[depth++] = ptr;
  }

  for (ptr = y; ptr != AST_NIL; ptr = tree_out(ptr))
  {
    if (is_loop(ptr))
    {
      for (i = 0; i < depth; i++)
      {
        if (loops[i] == ptr)
          return ptr;
      }
    }
  }

  return AST_NIL;
}


/*--------------------------------------------------------------------

  pt_fuse()     Perform loop fusion

  */

void
pt_fuse(PedInfo ped, AST_INDEX loop)
{
  AST_INDEX loop1, loop2;
  AST_INDEX slist1, slist2;
  AST_INDEX node, pholder, temp;
  AST_INDEX control1, control2, lbl1, lbl2, new_ivar;
  char *ivar1, *ivar2;
  char buf[100];
  DG_Edge *edgeptr;
  int e, next, vector1, vector2, l, level, flow;

  /*--------------------------------------*/
  /* find both loops              */

  loop1 = loop;
  loop2 = list_next(loop1);
  while (is_comment(loop2))
    loop2 = list_next(loop2);

  slist1 = gen_DO_get_stmt_LIST(loop1);
  slist2 = gen_DO_get_stmt_LIST(loop2);

  /*----------------------------------*/
  /* adjust index vars if necessary   */

  control1 = gen_DO_get_control(loop1);
  ivar1 = gen_get_text(gen_INDUCTIVE_get_name(control1));

  control2 = gen_DO_get_control(loop2);
  ivar2 = gen_get_text(gen_INDUCTIVE_get_name(control2));

  if (strcmp(ivar1, ivar2))
  {
    /* ivars differ, replace both with unique id    */

    sprintf(buf, "%s$%d", ivar1, fuse_id++);
    new_ivar = pt_gen_ident(buf);

    pt_var_replace(loop1, ivar1, new_ivar);
    pt_var_replace(loop2, ivar2, new_ivar);
  }

  /*---------------------------------------------------*/
  /* Update control dependences with src loop2         */
  /* to be on the header of loop1              */

  level = loop_level(loop1);
  edgeptr = dg_get_edge_structure( PED_DG(ped));
  vector1 = get_info(ped, loop1, type_levelv);
  vector2 = get_info(ped, loop2, type_levelv);

  for (l = LOOP_INDEPENDENT; l <= level; l++)
  {
    for (e = dg_first_src_stmt( PED_DG(ped), vector2, l); e != NIL; e = next)
    {
      next = dg_next_src_stmt( PED_DG(ped), e);
      dg_delete_edge( PED_DG(ped), e);

      if ((edgeptr[e].type == dg_control) &&
          (edgeptr[e].src == edgeptr[e].sink))
      {
        dg_free_edge( PED_DG(ped), edgeptr, e);
      }
      else
      {
        edgeptr[e].src = loop1;
        edgeptr[e].src_vec = vector1;
        dg_add_edge( PED_DG(ped), e);
      }
    }
  }

  /*---------------------------------------------------*/
  /* Delete control dependences with sink loop2        */

  for (l = LOOP_INDEPENDENT; l <= level; l++)
  {
    for (e = dg_first_sink_stmt( PED_DG(ped), vector2, level); e != NIL; e = next)
    {
      next = dg_next_sink_stmt( PED_DG(ped), e);
      dg_delete_free_edge( PED_DG(ped), e);
    }
  }

  /*--------------------------------------*/
  /* If DO <LABEL>, correct the header    */

  lbl1 = gen_DO_get_lbl_ref(loop1);
  lbl2 = gen_DO_get_lbl_ref(loop2);

  if ((lbl1 != AST_NIL) || (lbl2 != AST_NIL))
  {
    /* 

    Should check to see whether CONTINUE for first DO CONTINUE
    loop is used as a target of some other GOTO in the loop.
    If it isn't, then delete it.   For now just leave continue
    in place.

    flow = el_cflow( PED_LI(ped));
    if ((flow == STRUCT) || (flow == NOFLOW))

    */

    node = list_last(slist1);
    if (is_continue(node))
      list_remove_node(node);

    if (lbl2 != AST_NIL)
    {
      lbl2 = tree_copy(lbl2);
      tree_replace(lbl1, lbl2);
    }
    else
      tree_replace(lbl1, AST_NIL);
  }

  /*--------------------------------------*/
  /* merge body of loop2 into loop1       */

  node = list_last(slist1);
  pholder = gen_PLACE_HOLDER();
  list_insert_after(node, pholder);

  for (node = list_first(slist2); node != AST_NIL; node = temp)
  {
    temp = list_next(node);
    list_remove_node(node);
    list_insert_before(pholder, node);
  }

  /*----------------------*/
  /* get rid of loop2     */

  list_remove_node(loop2);
  list_remove_node(pholder);


}


/*--------------------------------------------------------------------

  pt_fuse_all()     Perform loop fusion on all loops 

  Fuse all loops in body using 1-pass greedy algorithm.

  Carry flag indicates whether loops should be 
    fused if the resulting fused loop will carry dependences.

*/

void
pt_fuse_all(PedInfo ped, AST_INDEX body, Boolean carry)
{
  struct fuse_params param;

  param.ped = ped;
  param.carry = carry;

  walk_statements(body, loop_level(body), (WK_STMT_CLBACK)fuse_loops, NULL, (Generic)&param);
}


/*--------------------------------------------------------------------

  fuse_loops()     Perform loop fusion on all  

*/

static int
fuse_loops(AST_INDEX stmt, int level, struct fuse_params* param)
{
  int flag;
  Boolean fused_loop;

  if (!is_loop(stmt))
    return WALK_CONTINUE;

  /* try to fuse all contiguous loops at this level */

  fused_loop = false;

  while (true)
  {
    flag = pt_fuse_test(param->ped, stmt);

    if ((flag == FUSE_OK) || (param->carry && (flag == FUSE_OK_CARRY)))
    {
      pt_fuse(param->ped, stmt);
      fused_loop = true;
    }
    else
    {
      /* if fused outermost loop, must update deps */

      if (fused_loop && (loop_level(stmt) == 1))
        ped_dt_update(param->ped, stmt);

      return WALK_CONTINUE;
    }
  }
}

