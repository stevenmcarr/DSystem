/* $Id: dc_loops.ansi.c,v 1.9 1997/03/11 14:28:15 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/



/*---------------------------------------------------------------------

    dc_loops.c   Routines to handle loops & index conversions

    Author : Chau-Wen Tseng 

*/

#include <libs/fortD/codeGen/private_dc.h>

struct conv_param;

/*------------------------- extern definitions ------------------------*/

EXTERN(void, dc_new_var,(Dist_Globals *dh, AST_INDEX node, enum FORM type));   
                                            /* dc.c */
EXTERN(void, dc_find_stmt_groups,(Dist_Globals *dh, AST_INDEX stmt_list, 
                                  Stmt_groups *sgroup, int gnum, int lvl, 
                                  Boolean nonloop));       
                                             /* bc.c */
EXTERN(Iter_set, *dc_alloc_iter_set,(Dist_Globals *dh));    /* bc.c */
EXTERN(Boolean,   query_irreg_ref,  (Generic irr, AST_INDEX node));
EXTERN(const char *,  ValDecomp_get_loc2glob_name, (Generic hidden_vd));

/*------------------------- global definitions ------------------------*/

void dc_loop_info(Dist_Globals *dh);
void dc_indices(Dist_Globals *dh);

/*------------------------- local definitions -------------------------*/

/*STATIC(int, collect_isets,());*/

STATIC(void, build_local_indices,(struct conv_param *param));
STATIC(void, build_global_indices,(struct conv_param *param));

STATIC(int, conv_indices_stmt,(AST_INDEX stmt, int level, 
                               struct conv_param *param));
STATIC(void, conv_subs,(AST_INDEX node, struct conv_param *param));
STATIC(void, conv_id,(AST_INDEX node, struct conv_param *param));
STATIC(int, conv_indices_expr,(AST_INDEX node, struct conv_param *param));

/*------------------------- local structures -------------------------*/

#define MAXGVAR 256

struct conv_param
{
  Dist_Globals *dh;
  Iter_set *iset;
  Loop_list *l_list;
  Loop_data *ldata[LOOPTOTAL];

  int glo_var_num;
  int glo_loop_num;
  AST_INDEX loop_vars[LOOPTOTAL];
  AST_INDEX old_glo_vars[MAXGVAR];
  AST_INDEX glo_vars[MAXGVAR];
  int glo_blk[MAXGVAR];
  Dist_type glo_dist[MAXGVAR];

  int loc_var_num;
  int loc_loop_num;
  AST_INDEX old_loc_vars[MAXGVAR];
  AST_INDEX loc_vars[MAXGVAR];
  int loc_blk[MAXGVAR];
  Dist_type loc_dist[MAXGVAR];

  AST_INDEX offs[MAXGVAR];
  int offsets[MAXGVAR];
  int off_idx;
};

/*******************/
/* Global Routines */
/*******************/

/*--------------------------------------------------------------------

    dc_loop_info()

    Categorize loops to determine whether their bounds can be 
    reduced, or whether global<->local index translation is needed.

    For each loop:

       1) partition body of loop into statement groups
       2) classify loop according to groups
       3) store in Fortran D Loop Info structure
*/

void
dc_loop_info(dh)
  Dist_Globals *dh;
{
  int i, j, depth;
  FortD_LI *fli;
  Reduc_set *reduc_s;

  for (i = 0; i < dh->numdoloops; i++)
  {
    /* allocate Fortran D Loop Info structure */
    fli = (FortD_LI *) dc_get_mem(dh, sizeof(FortD_LI));
    bzero(fli, sizeof(FortD_LI));
    fli->depth = depth = loop_level(dh->doloops[i]) - 1;
    put_info(dh->ped, dh->doloops[i], type_fd, (Generic)fli);

    /* partition body of loop into statement groups */
    dc_find_stmt_groups(dh, gen_DO_get_stmt_LIST(dh->doloops[i]), 
                         &fli->sgroup, 0, depth, false);

    /* classify loop - by default, assume first group is only group */
    fli->uniform = true;
    fli->iset = fli->sgroup.groups[0]->iset;

    /* 2/8/94 RvH: Don't blow up if no stmt group is found
     * NOTE: this blindly replicates execution, it does not test,
     *       for example, against guarded assignments to distributed
     *       Variables.
     */
    if (fli->iset)
    {
      fli->cyclic_type = fli->iset->cyclic[depth];
      fli->user_type = fli->iset->user[depth];
      fli->localized = (fli->bksize = fli->iset->bksize[depth]);
    }
    else
    {
      fli->localized = false;
    }

    if (fli->localized)
    {
      /* 12/3/93 RvH: Grubby hack for determining value-based
	 decomposition ... */
      /* if (fli->sgroup.groups[0]->iset->set.loops[depth].up.type
	  == Expr_simple_sym) */
      if (fli->user_type != Uiter_none)
      {
	fli->dist_type = FD_DIST_USER;
      }
      else
      {
	fli->dist_type = (fli->cyclic_type == Citer_none)
	  ? FD_DIST_BLOCK
	    : FD_DIST_CYCLIC;
      }
    }
    else
    {
      fli->dist_type = FD_DIST_LOCAL;
    }

    /* mark loops with nonlocal reductions, if any are found at this level */
    if (dh->in_ded)
    {
      for (j = 0; j < fli->sgroup.group_num; j++)
      {
        reduc_s = fli->sgroup.groups[j]->iset->reduc_set;
        if (reduc_s && !reduc_s->local && (reduc_s->loop == dh->doloops[i]))
          fli->num_reduc++;        
      }
    }

    /* if multiple statement groups, then non-uniform partition */
    if (fli->sgroup.group_num > 1)
    {
      fli->uniform = false;

      /* find non-partitioned statement group, if any */
      for (j = 0; j < fli->sgroup.group_num; j++)
      {
        if (fli->sgroup.groups[j]->iset->allproc ||
            (!fli->sgroup.groups[j]->iset->bksize[depth] &&
             !fli->sgroup.groups[j]->iset->oneproc))
          break;
      }

      /* if global stmt group found, the entire loop must be executed */
      if (j < fli->sgroup.group_num)
      {
        fli->iset = fli->sgroup.groups[j]->iset;
        fli->bksize = 0;
        fli->localized = false;
        fli->dist_type = FD_DIST_LOCAL;
        fli->cyclic_type = Citer_none;
      }

      /* else must union different partitioned stmt groups */
      else
      {
        /* for now just force loop to be executed on all procs */

        fli->iset = dc_alloc_iter_set(dh);
        fli->iset->allproc = true;
        bcopy(get_info(dh->ped, dh->doloops[i], type_ref), 
              &fli->iset->set, sizeof(Loop_list)); 
        fli->bksize = 0;
        fli->localized = false;
        fli->dist_type = FD_DIST_LOCAL;
        fli->cyclic_type = Citer_none;
      }
    }
  }
}


/*--------------------------------------------------------------------

    dc_indices()   Convert local <-> global indices as needed.
*/

void
dc_indices(dh)
  Dist_Globals *dh;
{
  struct conv_param param;

  bzero(&param, sizeof(struct conv_param));
  param.dh = dh;

  /* collect info on all occurences of loop index vars in program */
  walk_statements(dh->root, LEVEL1, (WK_STMT_CLBACK)conv_indices_stmt, NULL, 
                  (Generic)&param);

  build_global_indices(&param);
  build_local_indices(&param);

}


/*******************/
/* Local Routines  */
/*******************/

/*----------------------------------------------------------------------

  conv_indices_stmt()      Helper function for dc_indices()

*/

static int
conv_indices_stmt(stmt, level, param)
  AST_INDEX stmt;
  int level;
  struct conv_param *param;
{
  Iter_set *iset;

  if (!is_assignment(stmt) && !is_if(stmt) && !is_logical_if(stmt))
    return WALK_CONTINUE;

  /* collect info about lhs */
  iset = (Iter_set *) get_info(param->dh->ped, stmt, type_dc);

  if (iset != (Iter_set *) NO_DC_INFO)
  {
    param->iset = iset;
    param->l_list = &iset->set;

    if (is_assignment(stmt))
    {
      walk_expression(stmt, (WK_EXPR_CLBACK)conv_indices_expr, NULL, (Generic)param);
    }
    else if (is_logical_if(stmt))
    {
      stmt = gen_LOGICAL_IF_get_rvalue(stmt);
      walk_expression(stmt, (WK_EXPR_CLBACK)conv_indices_expr, NULL, (Generic)param);
    }
    else if (is_if(stmt))  /* convert indices in guard expressions */
    {
      stmt = gen_IF_get_guard_LIST(stmt);
      stmt = list_first(stmt);
      while (stmt != AST_NIL)
      {
        walk_expression(gen_GUARD_get_rvalue(stmt), 
                        (WK_EXPR_CLBACK)conv_indices_expr, NULL, (Generic)param);
        stmt = list_next(stmt);
      }
    }
  }

  return WALK_CONTINUE;
}

/*----------------------------------------------------------------------

  conv_indices_expr()      Convert indices in expression

*/

static int
conv_indices_expr(node, param)
  AST_INDEX node;
  struct conv_param *param;
{
  if (is_identifier(node)) 
    conv_id(node, param);
  else if (is_subscript(node))
    conv_subs(node, param);
  return WALK_CONTINUE;
}


/*----------------------------------------------------------------------

  conv_subs()      Convert local indices <-> global indices in subscripts

  Walks expressions in statement, looking for ZIV subscripts
  Converts constants into local indices when encountered.

  Need to revamp this entire routine to be more flexible.
  Should not depend on simple subscripts...

*/

static void
conv_subs(node, param)
     AST_INDEX node;
     struct conv_param *param;
{
  Dist_Globals *dh;
  AST_INDEX    subs, new, newsub, ref_node;
  int          i, j, val, lvl, add;
  Subs_list    *subscript;
  SNODE        *sp;
  char         buf[MAX_NAME];
  Loop_data    *ldata;
  Dist_type    dtype;
  FortD_LI     *fli;
  Boolean      is_irreg;

  dh = param->dh;
  sp = findadd2(node, 0, 0, dh);

  if (!sp || !sp->decomp)             /* array is not distributed */
    return;

  subscript = (Subs_list *) get_info(dh->ped, node, type_ref);
  if (subscript == (Subs_list *) NO_REF)
    return;

  /* find subscript in distributed dimension */
  ref_node = node;
  node     = gen_SUBSCRIPT_get_rvalue_LIST(node);
  node     = list_first(node);

  for (i = 0; (i < sp->numdim) && (node != AST_NIL); i++)
  {
    subs = node;
    node = list_next(node);

    if (subscript->subs[i].stype == SUBS_ZIV)
    {
      if (param->iset->oneproc)
      {
        switch (sp_is_part(sp, i))
        {
	case FD_DIST_BLOCK:
	  val = ((subscript->subs[i].constant - 1) %
		 sp_block_size1(sp, i)) + 1;
	  list_insert_after(subs, pt_gen_int(val));
	  list_remove_node(subs);
	  break;
	  
	case FD_DIST_CYCLIC:
	  val = ((subscript->subs[i].constant - 1) /
		 sp_num_blocks(sp, i)) + 1;
	  list_insert_after(subs, pt_gen_int(val));
	  list_remove_node(subs);
	  break;
	  
	case FD_DIST_LOCAL:
	  break;
        }
      }
    }
    else if ((lvl = subscript->subs[i].stype) < SUBS_SIV)
    {
      ldata = subscript->loop_nest->loops + lvl;
      
      fli = (FortD_LI *) get_info(dh->ped, ldata->loop_index, type_fd);
      
      switch (dtype = sp_is_part(sp, i))
      {
      case FD_DIST_LOCAL:
	if (fli->localized)  /* loop index var is local */
	{
	  /* construct eventual replacement for variable */
	  sprintf(buf, "%s$glo", ldata->ivar);
	  new = pt_gen_ident(buf);
	  
	  if ((add = subscript->subs[i].constant) > 0)
	    newsub = pt_gen_add(new, pt_gen_int(add));
	  else if (add < 0)
	    newsub = pt_gen_sub(new, pt_gen_int(-add));
	  else
	    newsub = new;
	  
	  param->glo_vars[param->glo_var_num] =  newsub;
	  
	  /* store variable to be replaced later */  
	  param->old_glo_vars[param->glo_var_num++] = subs;   
	  
	  for (j = 0; j < param->glo_loop_num; j++)   /* now store loop */
	  {
	    if (ldata->loop_index == param->ldata[j]->loop_index)
	      return;   /* loop previously stored */
	  }
	  
	  /* store loop & name of global index var */
	  param->ldata[param->glo_loop_num] = ldata;
	  param->loop_vars[param->glo_loop_num] = new;
	  param->glo_dist[param->glo_loop_num] = FD_DIST_LOCAL;
	  param->glo_blk[param->glo_loop_num++] = 0;
	}
	break;
	
      case FD_DIST_BLOCK:
      case FD_DIST_CYCLIC:
      default:
	if (!fli->localized)  /* loop index var is global */
	{
	  /* construct eventual replacement for variable */
	  sprintf(buf, "%s$my", ldata->ivar);
	  new = pt_gen_ident(buf);
	  
	  if ((add = subscript->subs[i].constant) > 0)
	    newsub = pt_gen_add(new, pt_gen_int(add));
	  else if (add < 0)
	    newsub = pt_gen_sub(new, pt_gen_int(-add));
	  else
	    newsub = new;
	  
	  param->loc_vars[param->loc_var_num] =  newsub;
	  
	  /* store variable to be replaced later */  
	  param->old_loc_vars[param->loc_var_num++] = subs;   
	  
	  for (j = 0; j < param->loc_loop_num; j++)   /* now store loop */
	  {
	    if (ldata->loop_index == param->ldata[j]->loop_index)
	      return;   /* loop previously stored */
	  }
	  
	  /* store loop & name of global index var */
	  param->ldata[param->loc_loop_num] = ldata;
	  param->loop_vars[param->loc_loop_num] = new;
	  param->loc_dist[param->loc_loop_num] = dtype;
	  param->loc_blk[param->loc_loop_num++] = sp_block_size1(sp, i);
	}
	break;
      }
    }
    else 
    {
      if (sp_is_part(sp, i) != FD_DIST_LOCAL)
      {
	is_irreg = query_irreg_ref(dh->irr, ref_node);
	
	if (!is_irreg)
	{
	  printf("conv_subs(): subscript too complex\n");
	}
      }
    }
  }
}


/*----------------------------------------------------------------------

  conv_id()      Convert local indices to global indices

  Walks expressions in statement, looking for non-subscript index
  variables in the RHS.  Convert them into global indices when
  encountered.

*/

static void
conv_id(node, param)
  AST_INDEX node;
  struct conv_param *param;
{
  Loop_list *l_list;
  char *name;
  int i, j;
  AST_INDEX loop, new;
  char buf[MAX_NAME];
  FortD_LI *fli;
  SNODE *sp;

  /* ignore all ids in subscripts */

  new = node;
  while ((new = tree_out(new)) != AST_NIL)
  {
    if (is_subscript(new))
    {
      sp = findadd2(new, 0, 0, param->dh);
      if (sp && sp->decomp)   /* array is distributed */
        return;
    }
  }

  /* scalar variable in RHS expression, check whether loop index var */

  l_list = param->l_list;
  name = gen_get_text(node);
  for (i = 0; i < l_list->level; i++)
  {
    if (!strcmp(name,l_list->loops[i].ivar))
      break;
  }
  if (i == l_list->level)   /* not index var, finished */
    return;

  loop = l_list->loops[i].loop_index;

  fli = (FortD_LI *) get_info(param->dh->ped, loop, type_fd);

  if (fli->localized)    /* loop is partitioned */
  {
    /* construct eventual replacement for variable */
    sprintf(buf, "%s$glo", name);
    new = pt_gen_ident(buf);
    param->glo_vars[param->glo_var_num] =  new;

    /* store variable to be replaced later */  
    param->old_glo_vars[param->glo_var_num++] = node;   

    for (j = 0; j < param->glo_loop_num; j++)   /* now store loop */
    {
      if (loop == param->ldata[j]->loop_index)  /* loop already stored */
        return;
    }

    /* store loop & name of global index var */
    param->ldata[param->glo_loop_num] = l_list->loops+i;
    param->loop_vars[param->glo_loop_num] = new;
    param->glo_dist[param->glo_loop_num] = fli->dist_type;
    param->glo_blk[param->glo_loop_num++] = fli->bksize;
  }
}


/*--------------------------------------------------------------------

    build_local_indices()    Handles creation of local indices

*/

static void
build_local_indices(param)
  struct conv_param *param;
{
  int i, j;
  AST_INDEX node, newivar, stmts, loop;
  char buf[MAX_NAME];
  Dist_Globals *dh;
  FortD_LI *fli;

  dh = param->dh;

  /* replace global indices with local indices */
  for (i = 0; i < param->loc_var_num; i++)
    tree_replace(param->old_loc_vars[i], param->loc_vars[i]);

  /* calculate value of local indices */
  for (i = 0; i < param->loc_loop_num; i++)
  {
    loop = param->ldata[i]->loop_index;
    newivar = param->loop_vars[i];
    fli = (FortD_LI *) get_info(dh->ped, loop, type_fd);
    fli->loc_ast = newivar;

    /* dc_new_var(dh, tree_copy(newivar), INTTYPE); */ /* register name */

    switch (param->loc_dist[i])
    {
    case FD_DIST_BLOCK:

      /* look for matching "off$x" for offset */
      for (j = 0; (j < param->off_idx) && 
                  (param->offsets[j] != param->loc_blk[i]); j++)
        ;

      /* not found, construct new "off$x" */
      if (j == param->off_idx)
      {
        sprintf(buf, "off$%d", param->off_idx++);
        param->offsets[j] = param->loc_blk[i];
        param->offs[j] = pt_gen_ident(buf);
        dc_new_var(dh, tree_copy(param->offs[j]), INTTYPE);  /* register name */

        /* create & insert "off$x = my$p * bk" */
        node = ast_get_logical_myproc(dh);
        node = pt_gen_mul(node, pt_gen_int(param->offsets[j]));
        node = gen_ASSIGNMENT(AST_NIL, param->offs[j], node);
        list_insert_before(dh->fortD_ph, node);
      }

      /* create stmt "i$loc = i - off$x" at head of loop "i" */
  
      node = pt_gen_ident(param->ldata[i]->ivar);
      node = pt_gen_sub(node, tree_copy(param->offs[j]));
      node = gen_ASSIGNMENT(AST_NIL, tree_copy(newivar), node);
      stmts = gen_DO_get_stmt_LIST(loop);
      list_insert_first(stmts, node);
      if (fli->init == AST_NIL)
        fli->init = node;
      break;

    case FD_DIST_CYCLIC:
      /* create stmt "i$loc = ((i-1)/n$p)+1" at head of loop "i" */
  
      node = pt_gen_ident(param->ldata[i]->ivar);
      node = pt_gen_sub(node, pt_gen_int(1));
      gen_put_parens(node,1);
      node = pt_gen_div(node, pt_gen_int(dh->numprocs));
      gen_put_parens(node,1);
      node = pt_gen_add(node, pt_gen_int(1));
      node = gen_ASSIGNMENT(AST_NIL, tree_copy(newivar), node);
      stmts = gen_DO_get_stmt_LIST(loop);
      list_insert_first(stmts, node);
      if (fli->init == AST_NIL)
        fli->init = node;
      break;
    }
  }
}


/*--------------------------------------------------------------------

    build_global_indices()    Handles creation of global indices

*/

static void
build_global_indices(param)
  struct conv_param *param;
{
  int i, j;
  AST_INDEX     node, newivar, stmts, loop;
  char          buf[MAX_NAME];
  Dist_Globals  *dh;
  FortD_LI      *fli;
  Generic       hidden_vd;
  const char    *loc2glob_name;

  dh = param->dh;

  /* replace local indices with global indices */
  for (i = 0; i < param->glo_var_num; i++)
    tree_replace(param->old_glo_vars[i], param->glo_vars[i]);

  /* calculate value of global indices */
  for (i = 0; i < param->glo_loop_num; i++)
  {
    loop = param->ldata[i]->loop_index;
    newivar = param->loop_vars[i];
    fli = (FortD_LI *) get_info(dh->ped, loop, type_fd);
    fli->glo_ast = newivar;

    /* dc_new_var(dh, tree_copy(newivar), INTTYPE); */ /* register name */

    switch (param->glo_dist[i])
    {
    case FD_DIST_BLOCK:

      /* look for matching "off$x" for offset */
      for (j = 0; (j < param->off_idx) && 
                  (param->offsets[j] != param->glo_blk[i]); j++)
        ;

      /* not found, construct new "off$x" */
      if (j == param->off_idx)
      {
        sprintf(buf, "off$%d", param->off_idx++);
        param->offsets[j] = param->glo_blk[i];
        param->offs[j] = pt_gen_ident(buf);
        dc_new_var(dh, tree_copy(param->offs[j]), INTTYPE);  /* register name */

        /* create & insert "off$x = my$p * bk" */
        node = ast_get_logical_myproc(dh);
        node = pt_gen_mul(node, pt_gen_int(param->offsets[j]));
        node = gen_ASSIGNMENT(AST_NIL, param->offs[j], node);
        list_insert_before(dh->fortD_ph, node);
      }

      /* create stmt "i$glo = i + off$x" at head of loop "i" */
  
      stmts = gen_DO_get_stmt_LIST(loop);
      node = pt_gen_ident(param->ldata[i]->ivar);
      node = pt_gen_add(node, tree_copy(param->offs[j]));
      node = gen_ASSIGNMENT(AST_NIL, tree_copy(newivar), node);
      list_insert_first(stmts, node);
      if (fli->init == AST_NIL)
        fli->init = node;
      break;

    case FD_DIST_CYCLIC:
      /* create stmt "i$glo = ((i-1)*bk)+1+my$p" at head of loop "i" */
  
      stmts = gen_DO_get_stmt_LIST(loop);
      node = pt_gen_ident(param->ldata[i]->ivar);
      node = pt_gen_sub(node, pt_gen_int(1));
      gen_put_parens(node,1);
      node = pt_gen_mul(node, pt_gen_int(param->loc_blk[i]));
      gen_put_parens(node,1);
      node = pt_gen_add(node, pt_gen_int(1));
      node = pt_gen_add(node, ast_get_logical_myproc(dh));
      node = gen_ASSIGNMENT(AST_NIL, tree_copy(newivar), node);
      list_insert_first(stmts, node);
      if (fli->init == AST_NIL)
        fli->init = node;
      break;

    case FD_DIST_USER:
       /* create stmt "i$glo = atomD$loc2glob(i)" at head of loop "i" */

      hidden_vd     = fli->sgroup.groups[0]->iset->irr_decomp[fli->depth];
      loc2glob_name = ValDecomp_get_loc2glob_name(hidden_vd);
;
      stmts = gen_DO_get_stmt_LIST(loop);
      node = pt_gen_ident(param->ldata[i]->ivar);
      node = gen_SUBSCRIPT(pt_gen_ident((char*) loc2glob_name),
			   list_create(node));
      node = gen_ASSIGNMENT(AST_NIL, tree_copy(newivar), node);
      list_insert_first(stmts, node);
      if (fli->init == AST_NIL)
        fli->init = node;
     break;
    }
  }
}

#ifdef OLD_CODE

/*--------------------------------------------------------------------

  Look for triangular loops 

*/

struct tri_params
{
  Dist_Globals *dh;
  Loop_list *linfo;
};


/*--------------------------------------------------------------------

  mark_loop_local()    Mark loop as local

*/

static int
mark_loop_local(node, param)
  AST_INDEX node;
  struct tri_params *param;
{
  Dist_Globals *dh;
  Loop_list *linfo;
  int i, j;
  char *name;

  if (!is_identifier(node))
    return WALK_CONTINUE;

  dh = param->dh;
  linfo = param->linfo;

  name = gen_get_text(node);

  for (i = 0; i < linfo->level - 1; i++)
  {
    if (!strcmp(name, linfo->loops[i].ivar))
    {
      for (j = 0; j < dh->numdoloops; j++)
      {
        if (dh->doloops[j] == linfo->loops[i].loop_index)
        {
          dh->loop_local[j] = true;
          return WALK_CONTINUE;
        }
      }

      printf("mark_loop_local(): unrecognized loop\n");
    }
  }

  return WALK_CONTINUE;
}


/*--------------------------------------------------------------------

  dc_find_triangular()    Find & mark triangular/trapezoidal loops

*/

static void
dc_find_triangular(dh)
  Dist_Globals *dh;
{
  int i, level;
  Loop_list *linfo;
  struct tri_params param;

  for (i = 0; i < dh->numdoloops; i++)
  {
    linfo = (Loop_list *) get_info(dh->ped, dh->doloops[i], type_ref);
    if (linfo == (Loop_list *) NO_REF)
      die_with_message("iter_build_assign(): missing loop info");

    level = linfo->level-1;

    param.dh = dh;
    param.linfo = linfo;

    if (linfo->loops[level].lo.type != Expr_constant)
      walk_expression(linfo->loops[level].lo.ast, 
                      mark_loop_local, NULL, &param);

    if (linfo->loops[level].up.type != Expr_constant)
      walk_expression(linfo->loops[level].up.ast, 
                      mark_loop_local, NULL, &param);
  }
}


#endif

/* eof */
