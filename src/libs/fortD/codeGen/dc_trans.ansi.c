/* $Id: dc_trans.ansi.c,v 1.23 1997/03/11 14:28:16 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/



/*---------------------------------------------------------------------

    dc_trans.c    Routines for applying source-to-source  
                  transformations in the Fortran D compiler

    Author : Seema Hiranandani 

*/

#include <libs/fortD/codeGen/private_dc.h>

/*------------------------- extern definitions ------------------------*/

EXTERN(Carried_deps, *dg_carried_deps,(DG_Instance *dg, SideInfo *infoPtr, 
                                       AST_INDEX loop));
EXTERN(Boolean, is_diff_isets,(Iter_set *iset1, Iter_set *iset2, int lvl));

EXTERN(Boolean,   is_executor_loop, (Generic irr, AST_INDEX node));

/*------------------------- global definitions ------------------------*/

void dc_do_distribution(Dist_Globals *dh);
void dc_pipe_loops(Dist_Globals *dh);
AST_INDEX dc_cgp_loop(Dist_Globals *dh, AST_INDEX loop, int pipe_idx, Rsd_set **pipe);
void dc_cgp_clear(Dist_Globals *dh);

/*------------------------- local definitions -------------------------*/

STATIC(void, dc_divide_distrib_loop,(PedInfo ped, Adj_List *adj_list, int level,
                                     AST_INDEX do_node1, Dist_Globals *dh));
STATIC(int, dc_patch_iter_set,(AST_INDEX stmt, int level, Dist_Globals *dh));
STATIC(void, dc_patch_distrib_loop,(AST_INDEX retrun_node, Adj_List *adj_list,
                                    Dist_Globals *dh));
STATIC(Boolean, cross_proc_loop,(Dist_Globals *dh, AST_INDEX loop));
STATIC(int, dc_cgp_granularity,(Dist_Globals *dh, AST_INDEX loop));
STATIC(Boolean, is_cgp_safe,(Dist_Globals *dh, AST_INDEX loop, int pipe_idx,
                             Rsd_set **pipe, int strip_level));

/*******************/
/* Global Routines */
/*******************/

/*---------------------------------------------------------------------

 dc_do_distribution

 Performs loop distribution to eliminate unnecessary guards

-------------------------------------------------------------------------*/
void
dc_do_distribution(Dist_Globals *dh)
{
  int i;

  for (i = dh->numdoloops - 1; i >= 0; --i)
    pt_do_user_distrib(dh->ped, dh->doloops[i],
                       dc_divide_distrib_loop, dc_patch_distrib_loop, (Generic)dh);
}


/*-----------------------------------------------------------------------

  dc_pipe_loops()  

  Apply loop interchange to minimize granularity of computation
  enclosed by cross-processor loops.  This maximizes the degree of
  pipeline parallelism.

  Algorithm for fine-grain pipelining:

     1) find interchangeable loop nests {L}
     2) label loops in {L} as cross-processor
     3) starting inside-out, interchange cross-processors loop
        inward if they enclose a non-cross-processor loop

*/

void
dc_pipe_loops(Dist_Globals *dh)
{
  int i, j, ltype[LOOPTOTAL], status[LOOPTOTAL], stype[LOOPTOTAL];
  Boolean cploop[LOOPTOTAL];

  bzero(ltype, sizeof(ltype));
  bzero(status, sizeof(status));
  bzero(stype, sizeof(stype));
  bzero(cploop, sizeof(cploop));

  /* find loops and determine their interchange status */

  /* i = j-1 --> loop i may enclose loop j OR precedes loop j       */
  /* i = j+1 --> loop i may be enclosed by loop j OR follows loop j */

  for (i = 0; i < dh->numdoloops; i++)
    pt_set_loop_type(dh->ped, dh->doloops[i], ltype+i, status+i, stype+i);

  /* find cross-processor loops */

  for (i = 0; i < dh->numdoloops; i++)
  {
    /* If loop is not an executor loop
     * AND it can be interchanged with outer loop
     *     OR it can be interchanged with inner loop
     */
    if ((dh->in_ded || !is_executor_loop(dh->irr, dh->doloops[i]))
        && ((ltype[i] == RECT_LOOP)
            || ((i > 0) && (ltype[i-1] == RECT_LOOP))))
    {
       cploop[i] = cross_proc_loop(dh, dh->doloops[i]);
    }
  }

  /* interchange cross-processor loops */

  for (j = dh->numdoloops - 1; j >= 0; j--)
  {
    i = j;

    while ((ltype[i] == RECT_LOOP) && cploop[i] && !cploop[i+1])
    {
      /* interchange if legal & CP loop enclosing non-CP loop */

      pt_switch_loops(dh->ped,dh->doloops[i],ltype[i],stype[i]);
      cploop[i] = false;
      cploop[i+1] = true;

      /* if needed, update enclosing loop */

      if ((i > 0) && (ltype[i-1] == RECT_LOOP) && cploop[i-1])
      {
        pt_set_loop_type(dh->ped, dh->doloops[i-1], 
                       ltype+i-1, status+i-1, stype+i-1);
      }

      /* update new inner loop */

      pt_set_loop_type(dh->ped, dh->doloops[i+1], ltype+i+1, 
         status+i+1, stype+i+1);

      dh->fgp[dh->fgp_idx++] = dh->doloops[i+1];  /* mark */

      i++;  /* go and check whether loop may be interchanged again */
    }
  }
}


/*-----------------------------------------------------------------------

  dc_cgp_loop()     Apply coarse-grain pipelining to loop 

  Apply strip-mine to loop enclosing cross-processor loop causing
  communications.  This creates an iterator loop where carried 
  messages may be inserted, decreasing the communication overhead
  at the expense of some pipeline parallelism.

  Returns:   AST_INDEX of iterator loop created, if any

  Sets:  dh->cgp_grain    granularity of cgp (size of strip selected)

         dh->cgp_size     AST_INDEX for strip size variable if # of 
                              iterations of iterator is not constant.  

         dh->cgp_up       AST_INDEX for upper bound of ivar2.  

         dh->cgp_ivar     AST_INDEX for index var of stripped loop "i"
 
         dh->cgp_ivar2    AST_INDEX for index var of iterator loop "i$"

*/

AST_INDEX
dc_cgp_loop(Dist_Globals *dh, AST_INDEX loop, int pipe_idx, Rsd_set **pipe)
{
  AST_INDEX outer_loop, control, lo, up, step, node, iup, isize;
  char buf[MAX_NAME];
  int i;

  dc_cgp_clear(dh);

  if (!pipe_idx || 
      !dh->opts.coarse_grain_pipe ||
      ((outer_loop = dt_ast_loop(loop)) == AST_NIL) ||
      !is_cgp_safe(dh, loop, pipe_idx, pipe, loop_level(outer_loop)) ||
      !(dh->cgp_grain = dc_cgp_granularity(dh, loop)))
    return loop;

  pt_strip_mine(dh->ped, outer_loop, dh->cgp_grain, NULL);

  outer_loop = dt_ast_loop(loop);

  control = gen_DO_get_control(outer_loop);     /* iterator loop info */
  lo = gen_INDUCTIVE_get_rvalue1(control);      /* lower loop bound */
  up = gen_INDUCTIVE_get_rvalue2(control);      /* upper loop bound */
  step = gen_INDUCTIVE_get_rvalue3(control);    /* loop step        */
  if (step != AST_NIL)
    return outer_loop;   /* nonunit step not yet handled */

  dh->cgp_ivar = tree_copy(gen_INDUCTIVE_get_name(control)); 
  dh->cgp_ivar2 = tree_copy(lo); 

  /* if upper bound non-constant, variable trip count for iterator */
  /* need to generate variable representing the # of trips         */

  /* replace "do i = i$, min(i$+step,up)" with "do i = i$, i$up" */

  sprintf(buf, "%sup", gen_get_text(lo));
  iup = pt_gen_ident(buf);
  tree_replace(up, tree_copy(iup));
  dh->cgp_up = iup;

  /* make "i$up = min(i$+step,up)" */

  node = gen_ASSIGNMENT(AST_NIL, tree_copy(iup), up);
  list_insert_before(outer_loop, node);

  if (pt_comp_inv_name(up, "min"))    /* whether upper bound is min(...) */
  {
    /* make "i$size = i$up - i$ + 1" */

    node = pt_gen_sub(tree_copy(iup), tree_copy(lo));
    node = pt_gen_add(node, pt_gen_int(1));
    sprintf(buf, "%ssize", gen_get_text(lo));
    isize = pt_gen_ident(buf);
    node = gen_ASSIGNMENT(AST_NIL, tree_copy(isize), node);
    list_insert_before(outer_loop, node);

    dh->cgp_size = isize;
  }

  /* if inner two loops were permuted by fgp, return to orig order */

  for (i = 0; i < dh->fgp_idx; i++)
  {
    if (loop == dh->fgp[i])
    {
      pt_switch_loops(dh->ped,outer_loop,RECT_LOOP,NULL);
      return outer_loop;
    }
  }

  return outer_loop;
}

static loop_cost(AST_INDEX loop)
{
  int cost;
  int ivar, node;

  
}


/*--------------------------------------------------------------------

  dc_cgp_clear()    Clear global structures marking cgp

*/

void
dc_cgp_clear(Dist_Globals *dh)
{
  dh->cgp_grain = 0;
  dh->cgp_size = AST_NIL;
  dh->cgp_up = AST_NIL;
  dh->cgp_ivar = AST_NIL;
  dh->cgp_ivar2 = AST_NIL;
}

/******************/
/* Local Routines */
/******************/

/*---------------------------------------------------------------------

 dc_divide_distrib_loop()

 called from pt_do_user_distrib
 look at all the statements inside the doloop; they should
 be in the adjacency list.
 get the iteration set from the side array
 check each statement's iteration set. If different set of
 processors computing, calculate groups.

-------------------------------------------------------------------------*/

static void
dc_divide_distrib_loop(PedInfo ped, Adj_List *adj_list, int level, 
                       AST_INDEX do_node1, Dist_Globals *dh)
{
/* contains information about the statements in the loop nest */

  int section_number, node_number, group_number, first_stmt, g_num_stmt2;
  Iter_set *iter_set, *iter_set2;
  Adj_Node *node_array;
  Region_Type *region_array;
  Boolean done;
  AST_INDEX do_node;

  section_number = -1;
  node_number = 0;
  node_array = adj_list->node_array;
  region_array = adj_list->region_array;

  section_number++;
  do_node = node_array[node_number].do_node;
  group_number = node_array[node_number].region;
  first_stmt = region_array[group_number].first_stmt;
  region_array[group_number].visited = true;
  node_array[node_number].loop = section_number;
  ++node_number;

  while (node_number <= adj_list->last_used_node)
  {

    /* everything in a strong connected region must be included */
    g_num_stmt2 = node_array[node_number].region;
    if (group_number == g_num_stmt2)
    {
      node_array[node_number].loop = section_number;
      ++node_number;
    }

    /* include everything nested at the same level */
    else if ((node_array[node_number].do_node != AST_NIL) &&
             (node_array[node_number].do_node == do_node))
    {
      node_array[node_number].loop = section_number;
      ++node_number;
    }

    else
    {
     /* Check to see if this statement should be included. If it 
          has the same iteration set as all statements with 
          node_array[node_number].loop = section_number then include.
        If not increment section_number,
          set first_statement = node_number;
     */
      done = false;
      while ((first_stmt != node_number) && (!done))
      {
        iter_set = (Iter_set *)
          get_info(ped, node_array[first_stmt].statement, type_dc);

        iter_set2 = (Iter_set *)
          get_info(ped, node_array[node_number].statement, type_dc);

        if (iter_set2 == (Iter_set *) NO_DC_INFO)
        {
          node_array[node_number].loop = section_number;
          done = true;
          ++node_number;
        }

        else if (iter_set == (Iter_set *) NO_DC_INFO)
          ++first_stmt;

        else          /* both itersets are present */
        { 
          if (is_diff_isets(iter_set, iter_set2, -1))
          {
            node_array[node_number].loop = section_number + 1;
            ++section_number;
            done = true;
            ++node_number;
          }

          /* if not differing iteration sets, check next stmt belonging to
               * the group */

          else
            ++first_stmt;
        }
      }
      if (!done)
      {
        node_array[node_number].loop = section_number;
        ++node_number;
      }
    }
    do_node = node_array[node_number - 1].do_node;
    group_number = node_array[node_number - 1].region;
    first_stmt = region_array[group_number].first_stmt;
  }
  adj_list->max_loop = section_number;
}


/*-----------------------------------------------------------------

 void dc_patch_distrib_loop()

 the iter_set info needs to be updated to store the new upper, step
 and lower bound ast numbers

 walk the statements, update loopref information, followed by
 an update to iter set information

------------------------------------------------------------------*/
static void
dc_patch_distrib_loop(AST_INDEX return_node, Adj_List *adj_list, Dist_Globals *dh)
{
  int i;

  for (i = 0; i <= adj_list->max_loop; i++)
  {
    dt_update_loopref(PED_DT_INFO(dh->ped), PED_INFO(dh->ped), return_node);
    walk_statements(return_node, LEVEL1, (WK_STMT_CLBACK)dc_patch_iter_set, NULL, 
                    (Generic)dh);
    return_node = list_next(return_node);
  }
}

/*-----------------------------------------------------------------

 void dc_patch_iter_set()

 the iter_set info needs to be updated to store the new upper, step
 and lower bound ast numbers

------------------------------------------------------------------*/
static int
dc_patch_iter_set(AST_INDEX stmt, int level, Dist_Globals *dh)
{
  Iter_set *iter_set1;
  Subs_list *sinfo;
  Loop_list *linfo;
  int i, ivar;

  if (is_assignment(stmt))
  {
    if ((iter_set1 = (Iter_set *) get_info(dh->ped, stmt, type_dc)) != ((Iter_set*)NO_DC_INFO))
    {
      sinfo = (Subs_list *) get_info(dh->ped, iter_set1->lhs, type_ref);
      linfo = sinfo->loop_nest;

      /* store the new Subs_list */
      iter_set1->sinfo = sinfo;
      for (i = 0; i < sinfo->dims; i++)
      {
        ivar = sinfo->subs[i].stype;
        /*-------------------------------------------------------------------*/
        /* store information on the subscript in the dimension corresponding */
        /* to the loop index variable                                        */

        if ((sp_is_part(iter_set1->lhs_sp, i) != FD_DIST_LOCAL))
        {
          iter_set1->set.loops[ivar].lo.ast = linfo->loops[ivar].lo.ast;
          iter_set1->set.loops[ivar].up.ast = linfo->loops[ivar].up.ast;
          iter_set1->set.loops[ivar].step.ast = linfo->loops[ivar].step.ast;
        }
      }
    }
  }
  return (WALK_CONTINUE);
}


/*-----------------------------------------------------------------------

  cross_proc_loop()  Determine whether loop is a cross-processor loop

  Cross-processor loops cause computation wavefronts to sweep 
  across processor boundaries.  They can be calculated using the
  formula in:

    "Compiler Optimizations for Fortran D on MIMD Distributed-Memory 
     Machines," Hiranandani, Kennedy, Tseng, Proceedings of 
     Supercomputing '91, Albuquerque, NM, Nov 1991.

  For the Fortran D compiler we implement a more efficient algorithm
  that only checks true dependences carried on a given loop, rather
  than all true loop-carried dependences.

  We ignor the following pathological case:

    DISTRIBUTE A(BLOCK,BLOCK)
    do j
      do i
        A(i,j) = A(i-1,j-1) 
      enddo
    enddo

  Both the i & j loops are cross-processor, but only the j loop will be
  marked as such, since the dependence is carried on the j loop.  The
  implemented algorithm will perform loop interchange unnecessarily,
  but does not otherwise affect the quality of the generated code,
  since the order does not affect parallelism.
  
*/

static Boolean
cross_proc_loop(Dist_Globals *dh, AST_INDEX loop)
{
  Carried_deps *deps;
  AST_INDEX src, sink, node;
  Subs_list *src_subs;
  Subs_list *sink_subs;
  SNODE *sp;
  Subs_data *sub1;
  Subs_data *sub2;
  int i, j, level;

  deps = dg_carried_deps( PED_DG(dh->ped), PED_INFO(dh->ped), loop);

  for (j = 0; j < deps->true_num; j++)
  {
    src = deps->true_deps[j]->src;
    sink = deps->true_deps[j]->sink;
    level = deps->true_deps[j]->level - 1;    /* sub->coeffs is 0 based */

    if ((node = dt_ast_sub(src)) == src)      /* skip if not an array */
      continue;

    sp = findadd2(node, 0, 0, dh);  

    if (!sp || !sp->decomp)   /* skip if lhs is not distributed */
      continue;

    src_subs = (Subs_list *) get_info(dh->ped, node, type_ref);
    sink_subs = (Subs_list *) get_info(dh->ped, dt_ast_sub(sink), type_ref);

    /* find subscript in distributed dimension */

    for (i = 0; i < sp->numdim; i++)
    {
      if (sp_is_part(sp, i) != FD_DIST_LOCAL)
      {
        sub1 = src_subs->subs+i;
        sub2 = sink_subs->subs+i;

        if (bcmp(sub1, sub2, sizeof(Subs_data)) || /* subs not identical   */
            (sub1->stype > SUBS_SIV))              /* OR not SIV subscript */
        {
          /* loop is cross-processor if its index var can be found in sub */

          if (sub1->coeffs[level] || sub2->coeffs[level]) 
          {
            free_mem(deps);
            return true;
          }
        }
      }
    }
  }

  free_mem(deps);
  return false;
}

/*--------------------------------------------------------------------

  dc_cgp_granularity()    Select granularity for coarse-grain pipelining

  Should utilize results of performance estimation & training sets
  to calculate an efficient degree of pipelining.  For now just rely
  on system default.


*/

static int
dc_cgp_granularity(Dist_Globals *dh, AST_INDEX loop)
{
  return dh->opts.pipe_size;   /* use default granularity for now */
}


/*--------------------------------------------------------------------

  is_cgp_safe()    Determine whether coarse-grain pipelining is safe

  Coarse-grain pipelining is not safe if local results to be
  communicated will be overwritten on suceeding iterations of the
  strip-mined loop.  We check the source of each true dependence
  for each RSD to be communicated.  If any source has an output
  dependence carried on the strip-mined loop, cgp is unsafe.

*/

static Boolean
is_cgp_safe(Dist_Globals *dh, AST_INDEX loop, int pipe_idx, Rsd_set **pipe, 
            int strip_level)
{
  int i, j, ref, ref2;
  EDGE_INDEX edge;
  DG_Edge *Earray;      /* array of all DG edges    */
  Rsd_set *rset;
  AST_INDEX sink, src;
  AST_INDEX defs[MAXREF];
  int def_idx;
  Boolean found;

  def_idx = 0;
  Earray = dg_get_edge_structure(PED_DG(dh->ped));

  /* collect all nonlocal refs */

  for (i = 0; i < pipe_idx; i++)
  {
    for (rset = *pipe++; rset; rset = rset->rsd_merge)
    {
      sink = gen_SUBSCRIPT_get_name(rset->subs[0]);

      if ((ref = get_info(dh->ped, sink, type_levelv)) == NO_LEVELV)
        continue;

      /* look at all dependences edges with id as sink */

      for (edge = dg_first_sink_ref(PED_DG(dh->ped), ref);
           edge >= 0;
           edge = dg_next_sink_ref(PED_DG(dh->ped), edge))
      {
        if (sink != Earray[edge].sink)
          die_with_message("rsd_get_level(): DG error, ref_list invalid");

        /* collect their corresponding defs */

        src = Earray[edge].src;
        if (Earray[edge].type == dg_true)
        {
          /* check whether defs checked previously */

          found = false;
          for (j = 0; j < def_idx; j++)
          {
            if (src == defs[j])
              found = true;
          }

          /* check defs for output dependences carried on target loop */

          if (found == false)
          {
            defs[def_idx++] = src;
            ref2 = get_info(dh->ped, src, type_levelv);
            
            /* look at all dependences edges with id as source */

            for (edge = dg_first_src_ref(PED_DG(dh->ped), ref2);
                 edge >= 0;
                 edge = dg_next_src_ref(PED_DG(dh->ped), edge))
            {
              if ((Earray[edge].type == dg_output) && 
                  (Earray[edge].level == strip_level))
              {
                return false; 
              }
            }
          }
        }
      }
    }
  }

  return true;   
}



/* eof */

