/* $Id: compute_uj.C,v 1.22 1995/08/18 10:27:02 trsuchyt Exp $ */

/****************************************************************************/
/*                                                                          */
/*    File:    compute_uj.C                                                 */
/*                                                                          */
/*    Description:  This file contains the code to compute the unroll       */
/*                  amounts for one or two loops based upon balance.        */
/*                  This comes from Carr's thesis.                          */
/*                                                                          */
/****************************************************************************/


#include <general.h>
#include <mh.h>
#include <mh_ast.h>
#include <fort/walk.h>
#include <compute_uj.h>
#include <balance.h>

#ifndef gi_h
#include <fort/gi.h>
#endif 

#ifndef dg_h
#include <dg.h>
#endif 

#ifndef dt_h
#include <dt.h>
#endif 

#ifndef pt_util_h
#include <pt_util.h>
#endif 

#include <mem_util.h>

static Boolean mc_unroll_cache = false;
dep_info_type *machine_info;

/****************************************************************************/
/*                                                                          */
/*    Function:     determine_uj_prof                                       */
/*                                                                          */
/*    Input:        stmt - AST index of a statement                         */
/*                  level - nesting level of stmt                           */
/*                  comp_info - various information                         */
/*                                                                          */
/*    Description:  Determine how many edges incident upon a statement can  */
/*                  be moved to the innermost loop by uj.  Increment a      */
/*                  counter for the loops that need to be unrolled for this */
/*                  to occur.                                               */
/*                                                                          */
/****************************************************************************/


static int determine_uj_prof(AST_INDEX      stmt,
			     int            level,
			     comp_info_type *comp_info)

  {
   DG_Edge    *dg;
   EDGE_INDEX edge;
   int        vector,
              inner,
              l,lvl;

     if (is_do(stmt))
       comp_info->loop_stack[level] = get_stmt_info_ptr(stmt)->loop_num;
     dg = dg_get_edge_structure( PED_DG(comp_info->ped));
     vector = get_info(comp_info->ped,stmt,type_levelv);
     for (lvl = comp_info->level; lvl < level-1;lvl++)
       for (edge = dg_first_src_stmt( PED_DG(comp_info->ped),vector,lvl);
	    edge != END_OF_LIST;
	    edge = dg_next_src_stmt( PED_DG(comp_info->ped),edge))
         if ((dg[edge].type == dg_true || dg[edge].type == dg_anti ||
	      dg[edge].type == dg_input || dg[edge].type == dg_output) &&
	     fst_GetField(comp_info->symtab,gen_get_text(dg[edge].src),
			  SYMTAB_NUM_DIMS) > 0)
           if (get_subscript_ptr(dg[edge].src)->surrounding_do ==
	       get_subscript_ptr(dg[edge].sink)->surrounding_do &&
	       dg[edge].consistent == consistent_SIV && !dg[edge].symbolic)

	      /* we have a consistent edge, now check whether unrolling one or
		 two loops can make the edge innermost */

	     {
	      inner = lvl;
	      l = lvl+1;

	          /* make sure at most one non-zero distance vector enty besides
		     the carrier */

	      while (l < gen_get_dt_LVL(&dg[edge]) && inner != -1)
		{
		 if (gen_get_dt_DIS(&dg[edge],l) > 0 || 
		     (gen_get_dt_DIS(&dg[edge],l) != 0 && dg[edge].type == dg_input) || 
		     gen_get_dt_DIS(&dg[edge],l) == DDATA_ANY)
		   if (inner == lvl)
		     inner = l;
		   else
		     inner = -1;
		 l++;
		}
	      if (inner != -1)

	             /* increment the count for the two loops that must be unrolled
			to make the edge innermost */

	        if (lvl == inner && comp_info->num_loops == 2)
		  {

		       /* unrolling any with the loop at lvl can make the edge 
			  innermost */

		   for (l = comp_info->level; l < lvl; l++)
		     comp_info->count[comp_info->loop_stack[l]]
		                     [comp_info->loop_stack[lvl]]++;
		   for (l = lvl; l < level-1; l++)
		     comp_info->count[comp_info->loop_stack[lvl]]
		                     [comp_info->loop_stack[l]]++;
		  }
		else 
		  comp_info->count[comp_info->loop_stack[lvl]]
	                          [comp_info->loop_stack[inner]]++;
	     }
     return(WALK_CONTINUE);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:      pick_max_loops                                         */
/*                                                                          */
/*    Input:         loop_data - loop structure                             */
/*                   count - array of dependence counts                     */
/*                   loop_stack - stack of loops surrounding a loop body    */
/*                   num_loops - number of loops to be unrolled             */
/*                   outer_level - outermost level of loop nest             */
/*                   inner_level - innermost level of loop nest             */
/*                                                                          */
/*    Description:   Determine which loops have the greatest potential      */
/*                   for removing memory references by unrolling them.      */
/*                                                                          */
/****************************************************************************/


static void pick_max_loops(model_loop *loop_data,
			   int        **count,
			   int        *loop_stack,
			   int        num_loops,
			   int        outer_level,
			   int        inner_level)

  {
   int   max_count = MININT,
         i,j,upperb,
         i1 = -1,
	 j1 = -1;

     upperb = inner_level;
     for (i = outer_level; i <= inner_level; i++)
       {
	if (num_loops == 1)
          upperb = i;

	    /* if we are unrolling two loops, look for the best pair of loops,
	       otherwise look for the best loop */

	for (j = i; j <= upperb; j++)
	  if (count[loop_stack[i]][loop_stack[j]] > max_count &&
	      loop_data[loop_stack[i]].max > 0 &&
	      loop_data[loop_stack[j]].max > 0 &&
	      !loop_data[loop_stack[i]].unroll &&
	      !loop_data[loop_stack[j]].unroll &&
	      (i == j || loop_data[loop_stack[i]].type == RECT ||
	       loop_data[loop_stack[j]].type == RECT))
	    {
	     max_count = count[loop_stack[i]][loop_stack[j]];
	     i1 = i;
	     j1 = j;
	    }
       }
     if (i1 != -1)
       if (i1 != j1)
	 {
	  loop_data[loop_stack[i1]].unroll = true;
	  loop_data[loop_stack[j1]].unroll = true;
	  loop_data[loop_stack[i1]].count = count[loop_stack[i1]]
	                                         [loop_stack[j1]];
	  loop_data[loop_stack[j1]].count = count[loop_stack[i1]]
	                                         [loop_stack[j1]];
	 }
       else
	 {
	  loop_data[loop_stack[i1]].unroll = true;
	  loop_data[loop_stack[i1]].count = count[loop_stack[i1]]
	                                         [loop_stack[i1]];
	 }
  }
	    

/****************************************************************************/
/*                                                                          */
/*    Function:     pick_loops_to_unroll                                    */
/*                                                                          */
/*    Input:        loop_data - loop structure                              */
/*                  loop - index of a loop                                  */
/*                  comp_info - various info                                */
/*                                                                          */
/*    Description:  Deterine the best loops to unroll for each possible     */
/*                  perfect loop nest.                                      */
/*                                                                          */
/****************************************************************************/


static void pick_loops_to_unroll(model_loop         *loop_data,
				 int                loop,
				 comp_info_type     comp_info)

  {
   int next;

     comp_info.loop_stack[loop_data[loop].level] = loop;
     if (loop_data[loop].inner_loop == -1)
       pick_max_loops(loop_data,comp_info.count,comp_info.loop_stack,
		      comp_info.num_loops,comp_info.level,
		      loop_data[loop].level);
     else 
       pick_loops_to_unroll(loop_data,loop_data[loop].inner_loop,comp_info);
     for (next = loop_data[loop].next_loop;
	  next != -1;
	  next = loop_data[next].next_loop)
       pick_loops_to_unroll(loop_data,next,comp_info);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:     pick_loops                                              */
/*                                                                          */
/*    Input:        loop_data - loop structure                              */
/*                  size - size of loop_data                                */
/*                  num_loops - number of loops to unroll                   */
/*                  ped - dependence graph handle                           */
/*                  symtab - symbol table                                   */
/*                  ar - arena for memory allocation                        */
/*                                                                          */
/*    Description:  Deterine the best loops to unroll for each possible     */
/*                  perfect loop nest.                                      */
/*                                                                          */
/****************************************************************************/


static void pick_loops(model_loop *loop_data,
		       int        size,
		       int        num_loops,
		       PedInfo    ped,
		       SymDescriptor symtab,
		       arena_type *ar)

  {
   comp_info_type comp_info;
   int i;

   comp_info.count = (int **)ar->arena_alloc_mem(LOOP_ARENA,
						 size*sizeof(int *));
   for (i=0;i<size;i++)
     comp_info.count[i] = (int *)ar->arena_alloc_mem_clear(LOOP_ARENA,
							   size*sizeof(int));
   comp_info.ped = ped;
   comp_info.num_loops = num_loops;
   comp_info.level = loop_data[0].level;
   comp_info.symtab = symtab;
   walk_statements(loop_data[0].node,loop_data[0].level,(WK_STMT_CLBACK)determine_uj_prof,(WK_STMT_CLBACK)
		   NOFUNC,(Generic)&comp_info);
   pick_loops_to_unroll(loop_data,0,comp_info);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:     edge_creates_pressure                                   */
/*                                                                          */
/*    Input:        edge - dependence graph edge                            */
/*                  dep_info - various info                                 */
/*                                                                          */
/*    Description:  Determine if unrolling loops at level1 and level2 can   */
/*                  make edge innermost.                                    */
/*                                                                          */
/****************************************************************************/


static Boolean edge_creates_pressure(DG_Edge       *edge,
				     dep_info_type *dep_info)

  {
   int lvl;

     if (edge->consistent == consistent_SIV && !edge->symbolic &&
	 get_subscript_ptr(edge->src)->surrounding_do ==
	 get_subscript_ptr(edge->sink)->surrounding_do)
       {
	for (lvl = 1; lvl < gen_get_dt_LVL(edge); lvl++)
	  if (gen_get_dt_DIS(edge,lvl) != 0  &&
	      (gen_get_dt_DIS(edge,lvl) != DDATA_ANY  ||
	       (gen_get_dt_DIS(edge,lvl) == DDATA_ANY &&
		lvl == edge->level)) &&
	      lvl != dep_info->level1 && lvl != dep_info->level2)
	    return(false);
	return(true);
       }
     return(false);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:     do_partition                                            */
/*                                                                          */
/*    Input:        name - AST index for an array name                      */
/*                  nlist - list of elements in a partition                 */
/*                  dg - dependence graph                                   */
/*                  dinfo - various info                                    */
/*                                                                          */
/*    Description:  Put all subscript references that are connected by      */
/*                  edges that can create register pressure after uj into   */
/*                  one partition.                                          */
/*                                                                          */
/****************************************************************************/


static void do_partition(AST_INDEX name,
			 UtilList  *nlist,
			 DG_Edge   *dg,
			 dep_info_type *dinfo)


  {
   subscript_info_type *sptr;
   int              refl;
   EDGE_INDEX       edge;

     sptr = get_subscript_ptr(name);
     sptr->visited = true;
     sptr->lnode = util_node_alloc(name,NULL);
     util_append(nlist,sptr->lnode);
     refl = get_info(dinfo->ped,name,type_levelv);
     for (edge = dg_first_src_ref( PED_DG(dinfo->ped),refl);
	  edge != END_OF_LIST;
	  edge = dg_next_src_ref( PED_DG(dinfo->ped),edge))
       if ((dg[edge].type == dg_true || dg[edge].type == dg_input) &&
	   edge_creates_pressure(&dg[edge],dinfo))
	 {
	  sptr = get_subscript_ptr(dg[edge].sink);
	  if(!sptr->visited)
	    do_partition(dg[edge].sink,nlist,dg,dinfo);
	 }
     for (edge = dg_first_sink_ref( PED_DG(dinfo->ped),refl);
	  edge != END_OF_LIST;
	  edge = dg_next_sink_ref( PED_DG(dinfo->ped),edge))
       if ((dg[edge].type == dg_true || dg[edge].type == dg_input) &&
	   edge_creates_pressure(&dg[edge],dinfo))
	 {
	  sptr = get_subscript_ptr(dg[edge].src);
	  if(!sptr->visited)
	    do_partition(dg[edge].src,nlist,dg,dinfo);
	 }
  }


/****************************************************************************/
/*                                                                          */
/*    Function:     partition_names                                         */
/*                                                                          */
/*    Input:        node - AST node                                         */
/*                  dinfo - various info                                    */
/*                                                                          */
/*    Description:  Put all subscript references that are connected by      */
/*                  edges that can create register pressure after uj into   */
/*                  one partition.                                          */
/*                                                                          */
/****************************************************************************/


static int partition_names(AST_INDEX      node,
			   dep_info_type *dinfo)

  {
   subscript_info_type *sptr;
   AST_INDEX           name;
   UtilNode            *lnode;

     if (is_subscript(node))
       {
	name = gen_SUBSCRIPT_get_name(node);
	sptr = get_subscript_ptr(name);
	if (NOT(sptr->visited))
	  {
	   sptr->visited = true;
	   lnode = util_node_alloc((Generic)util_list_alloc((Generic)NULL,
							    (char *)NULL),
				   (char *)NULL);
	   util_append(dinfo->partition,lnode);
	   do_partition(name,(UtilList *)UTIL_NODE_ATOM(lnode),
			dg_get_edge_structure( PED_DG(dinfo->ped)),
			dinfo);
	  }
       }
     return(WALK_CONTINUE);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:     check_incoming_edges                                    */
/*                                                                          */
/*    Input:        node - AST index of the name of a subscript reference   */
/*                  dep_info - various info                                 */
/*                                                                          */
/*    Description:  Examine incoming dependence edges to determine if a     */
/*                  reference is scalar wrt a loop.                         */
/*                                                                          */
/****************************************************************************/


static void check_incoming_edges(AST_INDEX     node,
				 dep_info_type *dep_info)

  {
   int 	      vector,i,
              dist,
              refs,
	      index;
   Boolean    has_true_dep[4],
              store_made[4],
              incoming,
              store = false;
   DG_Edge    *dg;
   EDGE_INDEX edge;
   subscript_info_type *sptr;

     vector = get_info(dep_info->ped,node,type_levelv);
     dg = dg_get_edge_structure( PED_DG(dep_info->ped));
     for (edge = dg_first_sink_ref( PED_DG(dep_info->ped),vector);
	  edge != END_OF_LIST;
	  edge = dg_next_sink_ref( PED_DG(dep_info->ped),edge))
       {
	if (get_subscript_ptr(dg[edge].src)->surrounding_do !=
	    get_subscript_ptr(dg[edge].sink)->surrounding_do)
	  index = -1;
	else if (dg[edge].level == dep_info->level1)
	  index = 0;
	else if (dg[edge].level == dep_info->level2)
	  index = 1;
	else if (dg[edge].level == dep_info->inner_level)
	  index = 2;
	else if (dg[edge].level == LOOP_INDEPENDENT)
	  index = 3;
	else
	  index = -1;
	if (index != -1)
	  {
	   switch(dg[edge].type) {
	     case dg_true: if (dg[edge].consistent != consistent_SIV || 
			       dg[edge].symbolic)
			     {
			      sptr = get_subscript_ptr(dg[edge].src);
			      sptr->is_scalar[index] = false;
			      sptr->prev_sclr[index] = true;
			     }
	                   break;
	     case dg_input:  if (dg[edge].consistent == consistent_SIV && 
				 !dg[edge].symbolic)
			       if (Self_loop(dg[edge]))
				 get_subscript_ptr(dg[edge].src)->
				             is_scalar[index] = true;
			     break;

	     case dg_output: if (dg[edge].consistent == consistent_SIV && 
				 !dg[edge].symbolic)
			       {
				sptr = get_subscript_ptr(dg[edge].src);
				if (Self_loop(dg[edge]) && 
				    !sptr->prev_sclr[index])
				  {
				   store_made[index] = false;
				   sptr->is_scalar[index] = true;
				  }
			       }
	                     else
			       {
				sptr = get_subscript_ptr(dg[edge].src);
				sptr->is_scalar[index] = false;
				sptr->prev_sclr[index] = true;
				sptr = get_subscript_ptr(dg[edge].sink);
				sptr->is_scalar[index] = false;
				sptr->prev_sclr[index] = true;
			       }
			     break;  
	     case dg_anti: if (dg[edge].consistent != consistent_SIV || 
			       dg[edge].symbolic)
	                     {
			      sptr = get_subscript_ptr(dg[edge].sink);
			      sptr->is_scalar[index] = false;
			      sptr->prev_sclr[index] = true;
			      store_made[index] = true;
			     }
	     default:;
	    }
	  }
       }
  }


/****************************************************************************/
/*                                                                          */
/*    Function:     survey_edges                                            */
/*                                                                          */
/*    Input:        node - AST index of a node                              */
/*                  dep_info - various info                                 */
/*                                                                          */
/*    Description:  Check for scalar array refs and compute the number of   */
/*                  flops in a loop body.                                   */
/*                                                                          */
/****************************************************************************/


static int survey_edges(AST_INDEX     node,
			dep_info_type *dep_info)

  {
   AST_INDEX    name;
   int          vector;
   int          ops;

     if (is_subscript(node) &&
	 (gen_get_converted_type(node) == TYPE_REAL ||
	  gen_get_converted_type(node) == TYPE_DOUBLE_PRECISION ||
	  gen_get_converted_type(node) == TYPE_COMPLEX))
       {
	name = gen_SUBSCRIPT_get_name(node);
	check_incoming_edges(name,dep_info);
       }
     else if (is_binary_op(node) || is_unary_minus(node))
        if (!is_binary_times(node) || 
	    (!is_binary_plus(tree_out(node)) && 
	     !is_binary_minus(tree_out(node))) ||
	    !((config_type *)PED_MH_CONFIG(dep_info->ped))->mult_accum)
	  if (gen_get_converted_type(node) == TYPE_DOUBLE_PRECISION ||
	      gen_get_converted_type(node) == TYPE_COMPLEX ||
	      gen_get_converted_type(node) == TYPE_REAL)
	    {
	     if (gen_get_converted_type(node) == TYPE_COMPLEX)
	       ops = 2;
	     else
	       ops = 1;
	     if (is_binary_times(node))
	       dep_info->flops += 
	       (((config_type *)PED_MH_CONFIG(dep_info->ped))->mul_cycles /
		((config_type *)PED_MH_CONFIG(dep_info->ped))->min_flop_cycles)
	       * ops;
	     else if (is_binary_plus(node) || is_binary_minus(node))
	       dep_info->flops += 
	       (((config_type *)PED_MH_CONFIG(dep_info->ped))->add_cycles /
		((config_type *)PED_MH_CONFIG(dep_info->ped))->min_flop_cycles)
	       * ops;
	     else if (is_binary_divide(node))
	       dep_info->flops += 
	       (((config_type *)PED_MH_CONFIG(dep_info->ped))->div_cycles /
		((config_type *)PED_MH_CONFIG(dep_info->ped))->min_flop_cycles)
	       * ops;
	     else
               dep_info->flops += ops; 
	    }
	  else;
	else;

           /* determine the number of scalars that take up registers */

     else if (is_identifier(node) && 
	      fst_GetField(dep_info->symtab,gen_get_text(node),
			   SYMTAB_NUM_DIMS) == 0)
       if (fst_GetField(dep_info->symtab,gen_get_text(node),FIRST))
	 {
	  fst_PutField(dep_info->symtab,(int)gen_get_text(node),FIRST,false);
	  if (gen_get_converted_type(node) == TYPE_REAL)
	    dep_info->scalar_regs++;
	  else if (gen_get_converted_type(node) == TYPE_DOUBLE_PRECISION)
	    dep_info->scalar_regs += 
	        ((config_type *)PED_MH_CONFIG(dep_info->ped))->double_regs;
	  else if (gen_get_converted_type(node) == TYPE_COMPLEX)
	    dep_info->scalar_regs += 2;
	 }
   return(WALK_CONTINUE);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:     get_expr_regs                                           */
/*                                                                          */
/*    Input:        node - AST index                                        */
/*                  reg_info - various info                                 */
/*                                                                          */
/*    Description:  Compute the number of registers required by an          */
/*                  expression using Sethi-Ullman numbering                 */
/*                                                                          */
/****************************************************************************/


static int get_expr_regs(AST_INDEX     node,
			 reg_info_type *reg_info)

  {
   int label1,label2;
   int nregs;
   subscript_info_type *sptr;
  
     if (gen_get_converted_type(node) == TYPE_REAL)
       nregs = 1;
     else if (gen_get_converted_type(node) == TYPE_DOUBLE_PRECISION)
       nregs = reg_info->config->double_regs;
     else if (gen_get_converted_type(node) == TYPE_COMPLEX)
       nregs = 2;
     else
       nregs = 0;
     if (is_subscript(node))
       {
	sptr = get_subscript_ptr(gen_SUBSCRIPT_get_name(node));
	if (sptr->eliminated)
	  put_label(node,0);
	else	  
	  put_label(node,nregs);
	if (get_label(node) > reg_info->expr_regs)
          reg_info->expr_regs = get_label(node);
	return(WALK_CONTINUE);
       }
     if (is_identifier(node))
       {
	if (fst_GetField(reg_info->symtab,gen_get_text(node),
			 SYMTAB_NUM_DIMS) == 0)
	  put_label(node,0);
	return(WALK_CONTINUE);
       }
     if (is_unary_minus(node))
       {
        put_label(node,get_label(gen_UNARY_MINUS_get_rvalue(node)));
	if (get_label(node) > reg_info->expr_regs)
          reg_info->expr_regs = get_label(node);
	return(WALK_CONTINUE);
       }
     if (is_binary_exponent(node))
       {
        label1 = get_label(gen_BINARY_EXPONENT_get_rvalue1(node));
        label2 = get_label(gen_BINARY_EXPONENT_get_rvalue2(node));
       }
     else if (is_binary_times(node))
       {
        label1 = get_label(gen_BINARY_TIMES_get_rvalue1(node));
        label2 = get_label(gen_BINARY_TIMES_get_rvalue2(node));
       }
     else if (is_binary_divide(node))
       {
        label1 = get_label(gen_BINARY_DIVIDE_get_rvalue1(node));
        label2 = get_label(gen_BINARY_DIVIDE_get_rvalue2(node));
       }
     else if (is_binary_plus(node))
       {
        label1 = get_label(gen_BINARY_PLUS_get_rvalue1(node));
        label2 = get_label(gen_BINARY_PLUS_get_rvalue2(node));
       }
     else if (is_binary_minus(node))
       {
        label1 = get_label(gen_BINARY_MINUS_get_rvalue1(node));
        label2 = get_label(gen_BINARY_MINUS_get_rvalue2(node));
       }
     else
       return(WALK_CONTINUE);
     if (label1 == label2)
       put_label(node,label1 + nregs);
     else
       {
	if (label1 > label2)
	  put_label(node,label1);
	else
	  put_label(node,label2);
       }
     if (get_label(node) > reg_info->expr_regs)
       reg_info->expr_regs = get_label(node);
     return(WALK_CONTINUE);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:      count_regs                                             */
/*                                                                          */
/*    Input:         stmt - AST index of a statement                        */
/*                   level - nesting level of stmt                          */
/*                   reg_info - various info                                */
/*                                                                          */
/*    Description:   Call get_expr_regs to compute the number of registers  */
/*                   required for each expression, store the max.           */
/*                                                                          */
/****************************************************************************/


static int count_regs(AST_INDEX     stmt,
		      int           level,
		      reg_info_type *reg_info)

  {
   int stmt_regs = 0;

     stmt_regs = reg_info->expr_regs;
     if (is_assignment(stmt))
       walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),(WK_EXPR_CLBACK)(WK_EXPR_CLBACK)NOFUNC,(WK_EXPR_CLBACK)(WK_EXPR_CLBACK)get_expr_regs,
		       (Generic)reg_info);
     else if (is_guard(stmt))
       walk_expression(gen_GUARD_get_rvalue(stmt),(WK_EXPR_CLBACK)(WK_EXPR_CLBACK)NOFUNC,(WK_EXPR_CLBACK)(WK_EXPR_CLBACK)get_expr_regs,
		       (Generic)reg_info);
     else if (is_arithmetic_if(stmt))
       walk_expression(gen_ARITHMETIC_IF_get_rvalue(stmt),(WK_EXPR_CLBACK)(WK_EXPR_CLBACK)NOFUNC,(WK_EXPR_CLBACK)(WK_EXPR_CLBACK)get_expr_regs,
		       (Generic)reg_info);
     else if (is_logical_if(stmt))
       walk_expression(gen_LOGICAL_IF_get_rvalue(stmt),(WK_EXPR_CLBACK)(WK_EXPR_CLBACK)NOFUNC,(WK_EXPR_CLBACK)(WK_EXPR_CLBACK)get_expr_regs,
		       (Generic)reg_info);
     else if (is_read_short(stmt))
       walk_expression(gen_READ_SHORT_get_data_vars_LIST(stmt),(WK_EXPR_CLBACK)(WK_EXPR_CLBACK)NOFUNC,(WK_EXPR_CLBACK)(WK_EXPR_CLBACK)
		       get_expr_regs,(Generic)reg_info);
     else if (is_write(stmt))
       walk_expression(gen_WRITE_get_data_vars_LIST(stmt),(WK_EXPR_CLBACK)(WK_EXPR_CLBACK)NOFUNC,(WK_EXPR_CLBACK)(WK_EXPR_CLBACK)get_expr_regs,
		       (Generic)reg_info);
     if (stmt_regs > reg_info->expr_regs)
       reg_info->expr_regs = stmt_regs;
     return(WALK_CONTINUE);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:     index_in_outer_subscript                                */
/*                                                                          */
/*    Input:        node - AST index for a subscript reference name         */
/*                  index - induction variable                              */
/*                                                                          */
/*    Description:  Determine if index appears in a subscript reference in  */
/*                  a position other than the first subscript position.     */
/*                                                                          */
/****************************************************************************/


static Boolean index_in_outer_subscript(AST_INDEX node,
					char      *index)

  {
   AST_INDEX sub,sub_list;

     if (index == NULL)
       return(false);
     sub_list = gen_SUBSCRIPT_get_rvalue_LIST(tree_out(node));
     for (sub = list_next(list_first(sub_list));
	  sub != AST_NIL;
	  sub = list_next(sub))
       if (pt_find_var(sub,index))
         return(true);
     return(false);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static Boolean missing_out_LI_anti_dep(AST_INDEX node)

  {
   AST_INDEX lhs,stmt;

     stmt = ut_get_stmt(node);
     if (is_assignment(stmt))
       if (node != gen_ASSIGNMENT_get_lvalue(stmt) &&
	   pt_expr_equal(node,gen_ASSIGNMENT_get_lvalue(stmt)))
         return(true);
     return(false);
  } 


static Boolean NotInvariantEdge(DG_Edge *edge,
				dep_info_type *dinfo)

  {
   if (edge->level == dinfo->level1)
     return(pt_find_var(tree_out(edge->src),dinfo->index[0]));
   else if (edge->level == dinfo->level2)
     return(pt_find_var(tree_out(edge->src),dinfo->index[1]));
   else if (edge->level == dinfo->inner_level)
     return(pt_find_var(tree_out(edge->src),dinfo->index[2]));
   else
    return(false);
  }

/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static AST_INDEX find_oldest_value(UtilList *nlist,
				   dep_info_type *dinfo)

  { 
   DG_Edge *dg; 
   EDGE_INDEX edge; 
   UtilNode *node; 
   Boolean found; 
   int refl;

     dg = dg_get_edge_structure( PED_DG(dinfo->ped));
     for (node = UTIL_HEAD(nlist);
	  node != NULL;
	  node = UTIL_NEXT(node)) 
       {
	refl = get_info(dinfo->ped,(AST_INDEX)UTIL_NODE_ATOM(node),
			type_levelv);
 	found = true; 
	for (edge = dg_first_sink_ref( PED_DG(dinfo->ped),refl);
	     edge != END_OF_LIST;
	     edge = dg_next_sink_ref( PED_DG(dinfo->ped),edge))
	  if(dg[edge].src != dg[edge].sink &&
	     (dg[edge].level == dinfo->level1 ||
	      dg[edge].level == dinfo->level2 ||
	      dg[edge].level == dinfo->inner_level ||
	      dg[edge].level == LOOP_INDEPENDENT) &&

	       /* handle scalar array refs correctly so that we can 
		  find the oldest value */

	     NotInvariantEdge(&dg[edge],dinfo) &&

	     get_subscript_ptr(dg[edge].src)->surrounding_do ==
	     get_subscript_ptr(dg[edge].sink)->surrounding_do)
	    if (util_in_list(get_subscript_ptr(dg[edge].src)->lnode,nlist))
	      {
	       found = false;
	       break;
	      } 	
	if (found)
	  break;
       }
     return(UTIL_NODE_ATOM(node));
  }


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static void summarize_partition_vector(int *dvec,
				      AST_INDEX node,
				      UtilList *nlist,
				      dep_info_type *dinfo)
  { 
   DG_Edge *dg;
   EDGE_INDEX edge;
   int refl,i,dist1;
   Boolean first = true;

     dg = dg_get_edge_structure( PED_DG(dinfo->ped));
     refl = get_info(dinfo->ped,node,type_levelv);
     for (edge = dg_first_src_ref( PED_DG(dinfo->ped),refl);
	  edge != END_OF_LIST;
	  edge = dg_next_src_ref( PED_DG(dinfo->ped),edge))
       if (dg[edge].consistent == consistent_SIV && !dg[edge].symbolic &&
	   dg[edge].type == dg_true || dg[edge].type == dg_input &&
	   get_subscript_ptr(dg[edge].src)->surrounding_do ==
	   get_subscript_ptr(dg[edge].sink)->surrounding_do)
         if (edge_creates_pressure(&dg[edge],dinfo) &&
	     util_in_list(get_subscript_ptr(dg[edge].sink)->lnode,nlist))
	   { 
	    for (i = 1; i < gen_get_dt_LVL(&dg[edge]); i++)
	      {
	       if (dg[edge].level == LOOP_INDEPENDENT)
	          dist1 = 0;
	       else if ((dist1 = gen_get_dt_DIS(&dg[edge],i)) < DDATA_BASE)
	         if (i == dinfo->level1 || i == dinfo->level2 ||
		     i == dinfo->inner_level)
	           dist1 = 1;
		 else
		   dist1 = 0;
	       else if (dist1 < 0)
	         dist1 = -dist1;
	       if (first || get_vec_DIS(dvec,i) > dist1)
	         put_vec_DIS(dvec,i,dist1);
	      }
	    i = gen_get_dt_LVL(&dg[edge]);
	    if ((dist1 = gen_get_dt_DIS(&dg[edge],i)) < DDATA_BASE)
	      if (i == dinfo->level1 || i == dinfo->level2 ||
		  i == dinfo->inner_level)
	        dist1 = 1;
	      else
	        dist1 = 0;
	    else if (dist1 < 0) 
	      dist1 = -dist1;
	    if (first || get_vec_DIS(dvec,i) < dist1)
	      put_vec_DIS(dvec,i,dist1);
	    first = false;
	   } 
  } 


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static void summarize_node_vector(int *memory_vec,
				  int *address_vec,
				  AST_INDEX node,
				  UtilList *nlist,
				  int      *level,
				  int      *max_level,
				  dep_info_type *dinfo)
  { 
   DG_Edge *dg;
   EDGE_INDEX edge;
   int refl,i,dist1;
   UtilNode *lnode;

     dg = dg_get_edge_structure( PED_DG(dinfo->ped));
     refl = get_info(dinfo->ped,node,type_levelv);
     for (edge = dg_first_sink_ref( PED_DG(dinfo->ped),refl);
	  edge != END_OF_LIST;
	  edge = dg_next_sink_ref( PED_DG(dinfo->ped),edge))
       if (dg[edge].consistent == consistent_SIV && !dg[edge].symbolic &&
	   get_subscript_ptr(dg[edge].src)->surrounding_do ==
	   get_subscript_ptr(dg[edge].sink)->surrounding_do)
         if (edge_creates_pressure(&dg[edge],dinfo) &&
	     util_in_list(get_subscript_ptr(dg[edge].sink)->lnode,nlist))
	   { 
	    *max_level = gen_get_dt_LVL(&dg[edge]) > *max_level ?
		 	 gen_get_dt_LVL(&dg[edge]) :
			 *max_level;
	    *level = dg[edge].level < *level ?
		     dg[edge].level :
		     *level;
	    for (i = 1; i <= gen_get_dt_LVL(&dg[edge]); i++)
	      {
	       if (dg[edge].level == LOOP_INDEPENDENT)
	          dist1 = 0;
	       else 
	         if ((dist1 = gen_get_dt_DIS(&dg[edge],i)) < DDATA_BASE)
	           if (i == dinfo->level1 || i == dinfo->level2 ||
		       i == dinfo->inner_level)
		     dist1 = 1;
		   else
		     dist1 = 0;
		 else if (dist1 < 0)
	           dist1 = -dist1;
	       if (dg[edge].type != dg_anti)
	         if (get_vec_DIS(memory_vec,i) > dist1)
	           put_vec_DIS(memory_vec,i,dist1);
	       if (get_vec_DIS(address_vec,i) > dist1)
	         put_vec_DIS(address_vec,i,dist1);
	      }
	   } 
  } 


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static void remove_nodes(AST_INDEX node,
			 PedInfo ped,
			 int level,
			 UtilList *nlist) 

  {
   DG_Edge *dg;
   EDGE_INDEX edge;
   int refl; 	

     util_pluck(get_subscript_ptr(node)->lnode);
     dg = dg_get_edge_structure( PED_DG(ped));
     refl = get_info(ped,node,type_levelv);
     for (edge = dg_first_src_ref( PED_DG(ped),refl);
	  edge != END_OF_LIST;
	  edge = dg_next_src_ref( PED_DG(ped),edge))
       if (get_subscript_ptr(dg[edge].src)->surrounding_do ==
	   get_subscript_ptr(dg[edge].sink)->surrounding_do)
         if ((dg[edge].level == LOOP_INDEPENDENT || dg[edge].level == level) &&
	     util_in_list(get_subscript_ptr(dg[edge].sink)->lnode,nlist))
           util_pluck(get_subscript_ptr(dg[edge].sink)->lnode);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static void get_machine_parms(AST_INDEX node,
			      int *regs,
			      int *refs,
			      dep_info_type *dep_info)

  {
   if (gen_get_converted_type(tree_out(node)) == TYPE_REAL)
     {
      *regs = 1;
      *refs = 1;
     } 
   else if (gen_get_converted_type(tree_out(node)) == TYPE_DOUBLE_PRECISION)
     { 
      *regs = ((config_type *)PED_MH_CONFIG(dep_info->ped))->double_regs;
      *refs = ((config_type *)PED_MH_CONFIG(dep_info->ped))->double_fetches;
     }	
   else if (gen_get_converted_type(tree_out(node)) == TYPE_COMPLEX)
     { 
      *regs = 2;
      *refs = ((config_type *)PED_MH_CONFIG(dep_info->ped))->double_fetches;
     }	
   else	
     {
      *regs = 0;
      *refs = 0; 
     } 
  } 


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static void get_distances(dep_info_type *dep_info, 
			  int *dvect,
			  int *dist1, 
			  int *dist2,
			  int *distn) 

  { 
     if (dep_info->level1 == 0) 
       *dist1 = 0;
     else if ((*dist1 = get_vec_DIS(dvect,dep_info->level1)) < DDATA_BASE)
       *dist1 = 1;
     else if (*dist1 < 0)
       *dist1 = -(*dist1);
     if (dep_info->level2 == 0)
       *dist2 = 0;
     else if ((*dist2 = get_vec_DIS(dvect,dep_info->level2)) <	DDATA_BASE)
       *dist2 = 1;
     else if (*dist1 < 0)
       *dist2 = -(*dist2);
     if (*dist1 > dep_info->x1) 
       dep_info->x1 = *dist1;
     if (*dist2 > dep_info->x2) 
       dep_info->x2 = *dist2; 
     if ((*distn = get_vec_DIS(dvect,dep_info->inner_level)) < DDATA_BASE)
       *distn = 1;
     else if (*distn < 0)
       *distn = -(*distn);
  } 


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static int get_coeff_index(int dist)

  {
     switch (dist)
       {
	case 0: 
	  return 0;
	case 1: 
          return 1;
	default:
	  return 2;
       }
  }


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static void update_gcoeff(int coeff[3][3],
			  int index1,
			  int index2,
			  int val)

  {
   int i,j;
   
     for (i = 2; i >= index1; i--)
       for (j = 2; j >= index2; j--)
         coeff[i][j] += val;
  }


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static void update_lcoeff(int coeff[3][3],
			  int index1,
			  int index2,
			  int val)

  {
   int i,j;
   
     for (i = index1-1; i >= 0; i--)
       for (j = 2; j >= 0; j--)
         coeff[i][j] += val;
     for (i = 2; i >= index1; i--)
       for (j = index2-1; j >= 0; j--)
         coeff[i][j] += val;
  }


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static void update_GGCoeff(float coeff[3][3],
			   int index1,
			   int index2,
			   float val,
			   PrefetchCoeffComponentType Comp[3][3],
			   ComponentType Type)

  {
   int i,j;
   
     for (i = 2; i >= index1; i--)
       for (j = 2; j >= index2; j--)
	 {
	  coeff[i][j] += val;
	  switch(Type)
	    {
	     case UNIT:          Comp[i][j].unit++; break;
	     case CEIL_FRACTION: Comp[i][j].ceil_fraction++; break;
	     case FRACTION:      Comp[i][j].fraction++; break;
	    }
	 }
  }


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static void update_LLCoeff(float coeff[3][3],
			   int index1,
			   int index2,
			   float val,
			   PrefetchCoeffComponentType Comp[3][3],
			   ComponentType Type)

  {
   int i,j;
   
     for (i = index1-1; i >= 0; i--)
       for (j = index2-1; j >= 0; j--)
	 {
	  coeff[i][j] += val;
	  switch(Type)
	    {
	     case UNIT:          Comp[i][j].unit++; break;
	     case CEIL_FRACTION: Comp[i][j].ceil_fraction++; break;
	     case FRACTION:      Comp[i][j].fraction++; break;
	    }
	 }
  }


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static void update_LGCoeff(float coeff[3][3],
			   int index1,
			   int index2,
			   float val,
			   PrefetchCoeffComponentType Comp[3][3],
			   ComponentType Type)

  {
   int i,j;
   
     for (i = index1-1; i >= 0; i--)
       for (j = 2; j >= index2; j--)
	 {
	  coeff[i][j] += val;
	  switch(Type)
	    {
	     case UNIT:          Comp[i][j].unit++; break;
	     case CEIL_FRACTION: Comp[i][j].ceil_fraction++; break;
	     case FRACTION:      Comp[i][j].fraction++; break;
	    }
	 }
  }


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static void update_GLCoeff(float coeff[3][3],
			   int index1,
			   int index2,
			   float val,
			   PrefetchCoeffComponentType Comp[3][3],
			   ComponentType Type)

  {
   int i,j;
   
     for (i = 2; i >= index1; i--)
       for (j = index2-1; j >= 0; j--)
	 {
	  coeff[i][j] += val;
	  switch(Type)
	    {
	     case UNIT:          Comp[i][j].unit++; break;
	     case CEIL_FRACTION: Comp[i][j].ceil_fraction++; break;
	     case FRACTION:      Comp[i][j].fraction++; break;
	    }
	 }
  }

/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static void compute_registers(dep_info_type *dep_info, 
			      AST_INDEX     node,
			      UtilList      *nlist) 

  {
   subscript_info_type *subp;
   int refs, regs, dist1, dist2, distn, cindex1, cindex2; 
   int dvect[MAXLOOP];

     summarize_partition_vector(dvect,node,nlist,dep_info);
     get_machine_parms(node,&regs,&refs,dep_info); 
     subp = get_subscript_ptr(node);
     if (subp->is_scalar[0] || subp->is_scalar[1] || subp->is_scalar[2])
       { 
	if (subp->is_scalar[2] && !subp->is_scalar[0] && !subp->is_scalar[1])
	  dep_info->scalar_coeff[0] += regs;
	else 
	  {
	   if (subp->is_scalar[0])
	     { 
	      if (dep_info->x1 == 1)
	        dep_info->x1 = 2;
	     }
	   else if (!subp->is_scalar[2])
	     dep_info->scalar_coeff[1] += regs;
	   if (subp->is_scalar[1])
	     {
	      if (dep_info->x2 == 1)
	        dep_info->x2 = 2;
	     }
	   else if (!subp->is_scalar[2])
	     dep_info->scalar_coeff[2] += regs;
	  }
       }
     else
       {
	get_distances(dep_info,dvect,&dist1,&dist2,&distn);
	cindex1 = get_coeff_index(dist1);
	cindex2 = get_coeff_index(dist2);
	update_gcoeff(dep_info->reg_coeff[0],cindex1,cindex2,
		      (distn + 1) * regs);
	update_gcoeff(dep_info->reg_coeff[1],cindex1,cindex2,
		      -(dist2*(distn+1)*regs));
	update_gcoeff(dep_info->reg_coeff[2],cindex1,cindex2,
		      -(dist1*(distn+1)*regs));
	update_gcoeff(dep_info->reg_coeff[3],cindex1,cindex2,
		      dist1*dist2*(distn+1)*regs);
       }
  }


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static void compute_extra_regs(dep_info_type *dinfo,
			       AST_INDEX     node,
			       AST_INDEX     prevnode,
			       UtilList      *nlist)

  {
   DG_Edge *dg;
   EDGE_INDEX edge;
   int     vector;
   int     dist1,sdist1,
           dist2,sdist2,
           distn,sdistn,regs,refs,cindex1,cindex2;
   int                 dvect[MAXLOOP];

     dg = dg_get_edge_structure( PED_DG(dinfo->ped));
     vector = get_info(dinfo->ped,node,type_levelv);
     for (edge = dg_first_sink_ref( PED_DG(dinfo->ped),vector);
	  dg[edge].src != prevnode && edge != END_OF_LIST;
	  edge = dg_next_sink_ref( PED_DG(dinfo->ped),edge));
     summarize_partition_vector(dvect,node,nlist,dinfo);
     get_machine_parms(node,&regs,&refs,dinfo);
     get_distances(dinfo,dg[edge].dt_data,&dist1,&dist2,&distn);
     cindex1 = get_coeff_index(dist1);
     cindex2 = get_coeff_index(dist2);
     get_distances(dinfo,dvect,&sdist1,&sdist2,&sdistn);
     if (dist1 != 0)
       if (dist2 != 0)
         update_gcoeff(dinfo->reg_coeff[3],cindex1,cindex2,
		       dist1*dist2*(sdistn+1)*regs);
       else
	 {
	  update_gcoeff(dinfo->reg_coeff[2],cindex1,cindex2,
			dist1*(sdistn+1)*regs);
	  update_gcoeff(dinfo->reg_coeff[3],cindex1,cindex2,
			-(dist1*sdist2*(sdistn+1)*regs));
	 }
     else
       {
	update_gcoeff(dinfo->reg_coeff[1],cindex1,cindex2,
		      dist2*(sdistn+1)*regs);
	update_gcoeff(dinfo->reg_coeff[3],cindex1,cindex2,
		      -(sdist1*dist2*(sdistn+1)*regs));
       }
  }


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


Boolean vector_all_zeros(int *dvect,
			 int level)

  {
   int i;

     for (i = 0; i < level-1; i++)
       if (dvect[i] != 0)
         return false;
     return true;
  }


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static Boolean NotInOtherPositions(AST_INDEX node,
				   char      *var)

  {
     for (node = list_next(list_first(node));
	  node != AST_NIL;
	  node = list_next(node))
       if (pt_find_var(node,var))
         return(false);
     return(true);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static Boolean CanMoveToInnermost(int *vector,
				  int level,
				  int max_level)

  {
   int i;
   
     if (level == LOOP_INDEPENDENT)
       return(true);
     for (i = level+1; i < max_level;i++)
       if (vector[i] != 0)
         return(false);
     return(true);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static char *FindInductionVar(model_loop *loop_data,
			      AST_INDEX  node,
			      int        level)

  {
   int i;

     i = get_subscript_ptr(gen_SUBSCRIPT_get_name(node))->surrounding_do;
     while(loop_data[i].level != level)
       i = loop_data[i].parent;
     return(gen_get_text(gen_INDUCTIVE_get_name(gen_DO_get_control(
			 loop_data[i].node))));
  }


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static Boolean OnlyInInnermostPosition(model_loop *loop_data,
				       AST_INDEX  node,
				       int        level)
  {
   AST_INDEX sub_list,sub;
   char *var;
   int coeff,words;
   Boolean lin;
   
     sub_list = gen_SUBSCRIPT_get_rvalue_LIST(node);
     sub = list_first(sub_list);
     var = FindInductionVar(loop_data,node,level);
     if (pt_find_var(sub,var) && NotInOtherPositions(sub_list,var))
       return(true);
     return(false);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static Boolean HasGroupSpatial(AST_INDEX  node,
			       int        *vector,
			       model_loop *loop_data,
			       int        words,
			       int        level,
			       int        max_level)
  {
   if (level > 1 && CanMoveToInnermost(vector,level,max_level))
     if (OnlyInInnermostPosition(loop_data,node,level))
       if (vector[level] < words)
         return(true);
   return(false);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static Boolean IsSpatial(AST_INDEX  node,
			 char       *ivar,
			 int        words)

  {
   AST_INDEX sub_list,sub;
   char      *var;
   int       coeff;
   Boolean   lin;

     sub_list = gen_SUBSCRIPT_get_rvalue_LIST(node);
     if (pt_find_var(sub_list,ivar))
       {
	sub = list_first(sub_list);
	if (pt_find_var(sub,ivar) && NotInOtherPositions(sub_list,ivar))
	  {
	   pt_get_coeff(sub,ivar,&lin,&coeff);
	   if (coeff < words && lin)
	     return(true);
	  }
       }
     return(false);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


SpatialLocalityType GetSpatialType(AST_INDEX  node,
				   dep_info_type *dinfo,
				   int        *vector,
				   int        level,
				   int        max_level)

  {
   int words;
  
     if (gen_get_converted_type(node) == TYPE_DOUBLE_PRECISION ||
	 gen_get_converted_type(node) == TYPE_COMPLEX)
       words = (((config_type *)PED_MH_CONFIG(dinfo->ped))->line) >> 3; 
     else
       words = (((config_type *)PED_MH_CONFIG(dinfo->ped))->line) >> 2; 
     if (vector != NULL)
       if (HasGroupSpatial(node,vector,dinfo->loop_data,words,level,max_level))
         return(GROUP);
     if (IsSpatial(node,dinfo->index[2],words))
       return(SELF);
     else if (IsSpatial(node,dinfo->index[0],words))
       return(SELF1);
     else if (IsSpatial(node,dinfo->index[1],words))
       return(SELF2);
     else
       return(S_NONE);
  }

static int GetStride(AST_INDEX node,
		     char      *ivar,
		     int       step)

  {
   AST_INDEX sub_list,sub;
   char      *var;
   int       coeff;
   Boolean   lin;

     sub_list = gen_SUBSCRIPT_get_rvalue_LIST(node);
     sub = list_first(sub_list);
     pt_get_coeff(sub,ivar,&lin,&coeff);
     return(coeff*step);
  }

/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static void UpdatePrefetchCoeffFor_V0(AST_INDEX node,
				      dep_info_type *dinfo,
				      int refs)
  {
   int stride,CacheLineSize;
   float denom,f_refs;

   node = tree_out(node);
   f_refs = (float)refs;
   if (gen_get_converted_type(node) == TYPE_REAL)
     CacheLineSize = ((config_type *)PED_MH_CONFIG(dinfo->ped))->line >> 2;
   else
     CacheLineSize = ((config_type *)PED_MH_CONFIG(dinfo->ped))->line >> 3;
   switch (GetSpatialType(node,dinfo,NULL,0,0))
     {
      case SELF:
        stride = GetStride(node,dinfo->index[2],dinfo->step[2]);
        denom = (float)floor_ab(CacheLineSize,stride);
        update_GGCoeff(dinfo->PrefetchCoeff[0],0,0,f_refs/denom,
		       dinfo->PrefetchComponent[0],FRACTION);
        break;
      case SELF1:
        stride = GetStride(node,dinfo->index[0],dinfo->step[0]);
        denom = (float)floor_ab(CacheLineSize,stride);
        update_GGCoeff(dinfo->PrefetchCoeff[1],0,0,f_refs/denom,
		       dinfo->PrefetchComponent[1],CEIL_FRACTION);
        break;
      case SELF2:
        stride = GetStride(node,dinfo->index[1],dinfo->step[1]);
        denom = (float)floor_ab(CacheLineSize,stride);
        update_GGCoeff(dinfo->PrefetchCoeff[2],0,0,f_refs/denom,
		       dinfo->PrefetchComponent[2],CEIL_FRACTION);
        break;
      case S_NONE:
        update_GGCoeff(dinfo->PrefetchCoeff[3],0,0,f_refs,
		       dinfo->PrefetchComponent[3],UNIT);
        break;
      default:
        break;
     }
  }


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static void UpdatePrefetchCoeffFor_VC(AST_INDEX node,
				      int *vector,
				      dep_info_type *dinfo,
				      int level,
				      int max_level,
				      int refs)
  {
   int dist1,dist2,distn,cindex1,cindex2;
   int stride;
   float denom,f_refs;
   int CacheLineSize;

     node = tree_out(node);
     get_distances(dinfo,vector,&dist1,&dist2,&distn);
     cindex1 = get_coeff_index(dist1);
     cindex2 = get_coeff_index(dist2);
     if (gen_get_converted_type(node) == TYPE_REAL)
       CacheLineSize = ((config_type *)PED_MH_CONFIG(dinfo->ped))->line >> 2;
     else
       CacheLineSize = ((config_type *)PED_MH_CONFIG(dinfo->ped))->line >> 3;
     f_refs = (float)refs;
     switch (GetSpatialType(node,dinfo,vector,level,max_level))
       {
        case SELF:
          stride = GetStride(node,dinfo->index[2],dinfo->step[2]);
          denom=(float)floor_ab(CacheLineSize,stride);
	  update_LLCoeff(dinfo->PrefetchCoeff[0],cindex1,cindex2,
			 f_refs/denom,dinfo->PrefetchComponent[0],FRACTION);
	  update_LGCoeff(dinfo->PrefetchCoeff[0],cindex1,cindex2,
			 f_refs/denom,dinfo->PrefetchComponent[0],FRACTION);
	  update_GLCoeff(dinfo->PrefetchCoeff[0],cindex1,cindex2,
			 f_refs/denom,dinfo->PrefetchComponent[0],FRACTION);
	  update_GGCoeff(dinfo->PrefetchCoeff[1],cindex1,cindex2,
			 (dist2 * f_refs)/denom,dinfo->PrefetchComponent[1],FRACTION);
	  update_GGCoeff(dinfo->PrefetchCoeff[2],cindex1,cindex2,
			 (dist1 * f_refs)/denom,dinfo->PrefetchComponent[2],FRACTION);
	  update_GGCoeff(dinfo->PrefetchCoeff[3],cindex1,cindex2,
			 -(dist1 * dist2 * f_refs)/denom,dinfo->PrefetchComponent[3],
			 FRACTION);
          break;
        case SELF1:
	  stride = GetStride(node,dinfo->index[0],dinfo->step[0]);
          denom=(float)floor_ab(CacheLineSize,stride);
	  if (distn == 0)
	    {
	     update_LLCoeff(dinfo->PrefetchCoeff[0],cindex1,cindex2,f_refs/denom,
			    dinfo->PrefetchComponent[0],CEIL_FRACTION);
	     update_LGCoeff(dinfo->PrefetchCoeff[2],cindex1,cindex2,
			    (dist1-CacheLineSize+1)*f_refs/denom,
			    dinfo->PrefetchComponent[2],CEIL_FRACTION);
	     update_GLCoeff(dinfo->PrefetchCoeff[1],cindex1,cindex2,
			    dist2*f_refs/denom,
			    dinfo->PrefetchComponent[1],CEIL_FRACTION);
	     update_GGCoeff(dinfo->PrefetchCoeff[3],cindex1,cindex2,
			    (dist2*(dist1-CacheLineSize+1))*f_refs/denom,
			    dinfo->PrefetchComponent[3],CEIL_FRACTION);
	    }
	  else
	    {
	     update_LLCoeff(dinfo->PrefetchCoeff[0],cindex1,cindex2,f_refs/denom,
			    dinfo->PrefetchComponent[0],CEIL_FRACTION);
	     update_LGCoeff(dinfo->PrefetchCoeff[2],cindex1,cindex2,
			    dist1*f_refs/denom,
			    dinfo->PrefetchComponent[2],CEIL_FRACTION);
	     update_GLCoeff(dinfo->PrefetchCoeff[1],cindex1,cindex2,
			    dist2*f_refs/denom,
			    dinfo->PrefetchComponent[1],CEIL_FRACTION);
	     update_GGCoeff(dinfo->PrefetchCoeff[3],cindex1,cindex2,
			    dist2*dist1*f_refs/denom,
			    dinfo->PrefetchComponent[3],CEIL_FRACTION);
	    }
	  break;
        case SELF2:
          stride = GetStride(node,dinfo->index[1],dinfo->step[1]);
          denom=(float)floor_ab(CacheLineSize,stride);
	  if (distn == 0)
	    {
	     update_LLCoeff(dinfo->PrefetchCoeff[0],cindex1,cindex2,f_refs/denom,
			    dinfo->PrefetchComponent[0],CEIL_FRACTION);
	     update_LGCoeff(dinfo->PrefetchCoeff[1],cindex1,cindex2,
			    (dist2-CacheLineSize+1)*f_refs/denom,
			    dinfo->PrefetchComponent[1],CEIL_FRACTION);
	     update_GLCoeff(dinfo->PrefetchCoeff[2],cindex1,cindex2,
			    dist1*f_refs/denom,
			    dinfo->PrefetchComponent[2],CEIL_FRACTION);
	     update_GGCoeff(dinfo->PrefetchCoeff[3],cindex1,cindex2,
			    (dist1*(dist2-CacheLineSize+1))*f_refs/denom,
			    dinfo->PrefetchComponent[3],CEIL_FRACTION);
	    }
	  else
	    {
	     update_LLCoeff(dinfo->PrefetchCoeff[0],cindex1,cindex2,f_refs/denom,
			    dinfo->PrefetchComponent[0],CEIL_FRACTION);
	     update_LGCoeff(dinfo->PrefetchCoeff[1],cindex1,cindex2,
			    dist2*f_refs/denom,
			    dinfo->PrefetchComponent[1],CEIL_FRACTION);
	     update_GLCoeff(dinfo->PrefetchCoeff[2],cindex1,cindex2,
			    dist1*f_refs/denom,
			    dinfo->PrefetchComponent[2],CEIL_FRACTION);
	     update_GGCoeff(dinfo->PrefetchCoeff[3],cindex1,cindex2,
			    dist1*dist2*f_refs/denom,
			    dinfo->PrefetchComponent[3],CEIL_FRACTION);
	    }
          break;
        case S_NONE:
	  update_LLCoeff(dinfo->PrefetchCoeff[0],cindex1,cindex2,
			 f_refs,dinfo->PrefetchComponent[0],UNIT);
	  update_LGCoeff(dinfo->PrefetchCoeff[0],cindex1,cindex2,
			 f_refs,dinfo->PrefetchComponent[0],UNIT);
	  update_GLCoeff(dinfo->PrefetchCoeff[0],cindex1,cindex2,
			 f_refs,dinfo->PrefetchComponent[0],UNIT);
	  update_GGCoeff(dinfo->PrefetchCoeff[1],cindex1,cindex2,
			 (dist2 * f_refs),dinfo->PrefetchComponent[1],UNIT);
	  update_GGCoeff(dinfo->PrefetchCoeff[2],cindex1,cindex2,
			 (dist1 * f_refs),dinfo->PrefetchComponent[2],UNIT);
	  update_GGCoeff(dinfo->PrefetchCoeff[3],cindex1,cindex2,
			 -(dist1 * dist2 * f_refs),dinfo->PrefetchComponent[3],UNIT);
          break;
          break;
        default:
          break;
       }
  }


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static void UpdatePrefetchCoeffFor_VI(AST_INDEX node,
				      int *vector,
				      dep_info_type *dinfo,
				      int level,
				      int max_level,
				      int refs)
  {
   subscript_info_type *sptr;
   int dist1,dist2,distn,cindex1,cindex2;
   int stride,CacheLineSize;
   float denom,f_refs;

     node = tree_out(node);
     if (gen_get_converted_type(node) == TYPE_REAL)
       CacheLineSize = ((config_type *)PED_MH_CONFIG(dinfo->ped))->line >> 2;
     else
       CacheLineSize = ((config_type *)PED_MH_CONFIG(dinfo->ped))->line >> 3;
     f_refs = (float)refs;
     get_distances(dinfo,vector,&dist1,&dist2,&distn);
     sptr = get_subscript_ptr(gen_SUBSCRIPT_get_name(node));
     switch (GetSpatialType(node,dinfo,vector,level,max_level))
       {
	case SELF:
          stride = GetStride(node,dinfo->index[2],dinfo->step[2]);
          denom=(float)floor_ab(CacheLineSize,stride);
	  if (sptr->is_scalar[1])
	    if (sptr->is_scalar[0])
	      update_GGCoeff(dinfo->PrefetchCoeff[3],0,0,f_refs/denom,
			     dinfo->PrefetchComponent[3],FRACTION);
	    else
	      update_GGCoeff(dinfo->PrefetchCoeff[1],0,0,f_refs/denom,
			     dinfo->PrefetchComponent[1],FRACTION);
	  else
	    if (sptr->is_scalar[0])
	      update_GGCoeff(dinfo->PrefetchCoeff[2],0,0,f_refs/denom,
			     dinfo->PrefetchComponent[2],FRACTION);
          break;
        case SELF1:
          stride = GetStride(node,dinfo->index[0],dinfo->step[0]);
          denom=(float)floor_ab(CacheLineSize,stride);
	  update_GGCoeff(dinfo->PrefetchCoeff[1],0,0,f_refs/denom,
			     dinfo->PrefetchComponent[1],CEIL_FRACTION);
          break;
        case SELF2:
          stride = GetStride(node,dinfo->index[1],dinfo->step[1]);
          denom=(float)floor_ab(CacheLineSize,stride);
	  update_GGCoeff(dinfo->PrefetchCoeff[2],0,0,f_refs/denom,
			     dinfo->PrefetchComponent[2],CEIL_FRACTION);
          break;
        case S_NONE:
	  if (sptr->is_scalar[1])
	    if (sptr->is_scalar[0])
	      update_GGCoeff(dinfo->PrefetchCoeff[3],0,0,f_refs,
			     dinfo->PrefetchComponent[3],FRACTION);
	    else
	      update_GGCoeff(dinfo->PrefetchCoeff[1],0,0,f_refs,
			     dinfo->PrefetchComponent[1],FRACTION);
	  else
	    if (sptr->is_scalar[0])
	      update_GGCoeff(dinfo->PrefetchCoeff[2],0,0,f_refs,
			     dinfo->PrefetchComponent[2],FRACTION);
          break;
        default:
          break;
	 }
  }


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static void update_coeff_for_invariants(AST_INDEX node,
					subscript_info_type *sptr,
					dep_info_type *dep_info,
					int       *dvect,
					int       refs)
  {
     if (sptr->is_scalar[1])
       if (sptr->is_scalar[0])
	 {
	  if (NOT(vector_all_zeros(dvect,dep_info->inner_level)))
	    {
	     update_gcoeff(dep_info->mem_coeff[3],0,0,refs);
	     if (index_in_outer_subscript(node,dep_info->index[2]))
	       update_gcoeff(dep_info->addr_coeff[3],0,0,1);
	    }
	 }
       else
	 {
	  if (NOT(vector_all_zeros(dvect,dep_info->inner_level)))
	    {
	     update_gcoeff(dep_info->mem_coeff[1],0,0,refs);
	     if (index_in_outer_subscript(node,dep_info->index[0]))
  	       update_gcoeff(dep_info->addr_coeff[1],0,0,1);
	     else
	       update_gcoeff(dep_info->addr_coeff[3],0,0,1);
	    }
	 }
     else
       if (sptr->is_scalar[0])
	 {
	  if (NOT(vector_all_zeros(dvect,dep_info->inner_level)))
	    {
	     update_gcoeff(dep_info->mem_coeff[2],0,0,refs);
	     if (index_in_outer_subscript(node,dep_info->index[1]))
	       update_gcoeff(dep_info->addr_coeff[2],0,0,1);
	     else
	       update_gcoeff(dep_info->addr_coeff[3],0,0,1);
	    }
	 }
  }


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static void update_mem_coeff_with_vector(AST_INDEX node,
					 dep_info_type *dep_info,
					 int       *dvect,
					 int       refs)

  {
   int dist1,dist2,distn,cindex1,cindex2;

     get_distances(dep_info,dvect,&dist1,&dist2,&distn);
     cindex1 = get_coeff_index(dist1);
     cindex2 = get_coeff_index(dist2);
     update_gcoeff(dep_info->mem_coeff[1],cindex1,cindex2,dist2 * refs);
     update_gcoeff(dep_info->mem_coeff[2],cindex1,cindex2,dist1 * refs);
     update_gcoeff(dep_info->mem_coeff[3],cindex1,cindex2,
		   -(dist1 * dist2 * refs));
     update_lcoeff(dep_info->mem_coeff[0],cindex1,cindex2,1);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static void update_addr_coeff_with_no_dep(AST_INDEX node,
					  dep_info_type *dep_info)

  {
   int cindex;

     if (index_in_outer_subscript(node,dep_info->index[0]))
       if (index_in_outer_subscript(node,dep_info->index[1]))
         update_gcoeff(dep_info->addr_coeff[0],0,0,1);
       else
         update_gcoeff(dep_info->addr_coeff[1],0,0,1);
     else
       if (index_in_outer_subscript(node,dep_info->index[1]))
         update_gcoeff(dep_info->addr_coeff[2],0,0,1);
       else
         update_gcoeff(dep_info->addr_coeff[3],0,0,1);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static void update_addr_coeff_with_vector(AST_INDEX node,
					  dep_info_type *dep_info,
					  int       *dvect)
  {
   int dist1,dist2,distn;
   int cindex1,cindex2;

     get_distances(dep_info,dvect,&dist1,&dist2,&distn);
     if (index_in_outer_subscript(node,dep_info->index[0]))
       if (index_in_outer_subscript(node,dep_info->index[1]))
	 {
	  cindex1 = get_coeff_index(dist1);
	  cindex2 = get_coeff_index(dist2);
	  update_gcoeff(dep_info->addr_coeff[1],cindex1,cindex2,dist2);
	  update_gcoeff(dep_info->addr_coeff[2],cindex1,cindex2,dist1);
	  update_gcoeff(dep_info->addr_coeff[3],cindex1,cindex2,
			-(dist1 * dist2));
          update_lcoeff(dep_info->addr_coeff[0],cindex1,cindex2,1);
	 }
       else
	 {
	  cindex1 = get_coeff_index(dist1);
	  update_gcoeff(dep_info->addr_coeff[3],cindex1,0,dist1);
          update_lcoeff(dep_info->addr_coeff[1],cindex1,0,1);
	 }
     else if (index_in_outer_subscript(node,dep_info->index[1]))
       {
	cindex2 = get_coeff_index(dist2);
	update_gcoeff(dep_info->addr_coeff[3],0,cindex2,dist2);
	update_lcoeff(dep_info->addr_coeff[2],0,cindex2,1);
       }
     else
       update_gcoeff(dep_info->addr_coeff[3],0,0,1);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static void compute_mem_addr_coeffs(dep_info_type *dep_info,
				    UtilList      *nlist)

  {
   subscript_info_type *sptr;
   int refs, regs, dist1, dist2, distn,i, level = MAXINT, max_level = 0; 
   int memory_vec[MAXLOOP], address_vec[MAXLOOP];
   AST_INDEX node;
   UtilNode  *lnode;

     for (lnode = UTIL_HEAD(nlist);
	  lnode != NULL;
	  lnode = UTIL_NEXT(lnode))
       {
	node = (AST_INDEX)UTIL_NODE_ATOM(lnode);
	get_machine_parms(node,&regs,&refs,dep_info); 
	for (i = 1; i <= MAXLOOP; i++)
	  {
	   put_vec_DIS(memory_vec,i,MAXINT);
	   put_vec_DIS(address_vec,i,MAXINT);
	  }
	summarize_node_vector(memory_vec,address_vec,node,nlist,&level,
			      &max_level,dep_info);
	if (get_vec_DIS(memory_vec,1) == MAXINT)/* if no incoming dependence */
	  {
	   update_gcoeff(dep_info->mem_coeff[0],0,0,refs);
	   if (mc_unroll_cache)
	     UpdatePrefetchCoeffFor_V0(node,dep_info,refs);
	   if (get_vec_DIS(address_vec,1) == MAXINT) 
                     /* if no incoming dependence */
	     update_addr_coeff_with_no_dep(node,dep_info);
	   else
	     update_addr_coeff_with_vector(node,dep_info,address_vec);
	  }
	else
	  {
	   sptr = get_subscript_ptr(node);
	   if (!sptr->is_scalar[2])
	     if (sptr->is_scalar[0] || sptr->is_scalar[1])
	       {
		update_coeff_for_invariants(node,sptr,dep_info,memory_vec,
					    refs);
		if (mc_unroll_cache)
		  UpdatePrefetchCoeffFor_VI(node,memory_vec,dep_info,level,max_level,refs);
	       }
	     else
	       {
		if (mc_unroll_cache)
		  UpdatePrefetchCoeffFor_VC(node,memory_vec,dep_info,level,max_level,refs);
		update_mem_coeff_with_vector(node,dep_info,memory_vec,refs);
		update_addr_coeff_with_vector(node,dep_info,address_vec);
	       }
	  }
       }
  }


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static void compute_MIV_coefficients(AST_INDEX     node,
				     dep_info_type *dinfo)
  {
   AST_INDEX sub1;
   DG_Edge   *dg;
   int       vector,coeff0,coeff1,coeff2,dist;
   Boolean   lin;
   EDGE_INDEX edge;

     dg = dg_get_edge_structure(PED_DG(dinfo->ped));
     vector = get_info(dinfo->ped,node,type_levelv);
     for (edge = dg_first_sink_ref(PED_DG(dinfo->ped),vector);
	  dg[edge].consistent != consistent_MIV;
	  edge = dg_next_sink_ref(PED_DG(dinfo->ped),edge));
     sub1 = tree_out(dg[edge].src);
     for (sub1 = list_first(gen_SUBSCRIPT_get_rvalue_LIST(sub1));
	  !pt_find_var(sub1,dinfo->index[2]);
	  sub1 = list_next(sub1));
     if (sub1 != AST_NIL)
       {
	if (dinfo->index[0] != NULL)
	  if (pt_find_var(sub1,dinfo->index[0]))
	    {
	     pt_get_coeff(sub1,dinfo->index[0],&lin,&coeff0);
	     pt_get_coeff(sub1,dinfo->index[2],&lin,&coeff2);
	     if ((dist = gen_get_dt_DIS(&dg[edge],dinfo->inner_level)) != 0)
	       {
		if (dist < 0) 
	          dist = -dist;
		update_gcoeff(dinfo->reg_coeff[0],0,0,
			      (dist * coeff0 * dinfo->step[0]) /
			      (coeff2 * dinfo->step[2]));
	       }
	     else
	       update_gcoeff(dinfo->reg_coeff[0],0,0,
			     (coeff0 * dinfo->step[0]) /
			     (coeff2 * dinfo->step[2]));
	    }
	  else 
	    update_gcoeff(dinfo->reg_coeff[3],0,0,1);
	if (dinfo->index[1] != NULL)
	  if (pt_find_var(sub1,dinfo->index[1]))
	    {
	     pt_get_coeff(sub1,dinfo->index[1],&lin,&coeff1);
	     pt_get_coeff(sub1,dinfo->index[2],&lin,&coeff2);
	     if ((dist = gen_get_dt_DIS(&dg[edge],dinfo->inner_level)) != 0)
	       {
		if (dist < 0) 
	          dist = -dist;
		update_gcoeff(dinfo->reg_coeff[0],0,0,
			      (dist * coeff1 * dinfo->step[1]) /
			      (coeff2 * dinfo->step[2]));
	       }
	     else
	       update_gcoeff(dinfo->reg_coeff[0],0,0,
			     (coeff0 * dinfo->step[0]) /
			     (coeff2 * dinfo->step[2]));
	    }
	  else 
	    update_gcoeff(dinfo->reg_coeff[3],0,0,1);
	update_gcoeff(dinfo->mem_coeff[3],0,0,1);
	update_gcoeff(dinfo->addr_coeff[3],0,0,1);
       }
     else 
       if (dinfo->index[0] != NULL)
         if (!pt_find_var(sub1,dinfo->index[0]))
           if (dinfo->index[1] != NULL)
             if (!pt_find_var(sub1,dinfo->index[1]))
               update_gcoeff(dinfo->reg_coeff[3],0,0,1);
	     else
               update_gcoeff(dinfo->reg_coeff[2],0,0,1);
	   else
	     update_gcoeff(dinfo->reg_coeff[2],0,0,1);
	 else
	   if (dinfo->index[1] != NULL)
             if (!pt_find_var(sub1,dinfo->index[1]))
	       update_gcoeff(dinfo->reg_coeff[1],0,0,1);
	     else
               update_gcoeff(dinfo->reg_coeff[0],0,0,1);
	   else
	     update_gcoeff(dinfo->reg_coeff[0],0,0,1);
       else 
         if (dinfo->index[1] != NULL)
           if (!pt_find_var(sub1,dinfo->index[1]))
	     update_gcoeff(dinfo->reg_coeff[1],0,0,1);
	   else
             update_gcoeff(dinfo->reg_coeff[0],0,0,1);
	 else
	   update_gcoeff(dinfo->reg_coeff[0],0,0,1);
  }
				     

/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static Boolean node_is_consistent_MIV(AST_INDEX     node,
				      dep_info_type *dinfo)
  
  {
   AST_INDEX sub1;
   DG_Edge   *dg;
   int       vector,count2 = 0;
   EDGE_INDEX edge;

     dg = dg_get_edge_structure(PED_DG(dinfo->ped));
     vector = get_info(dinfo->ped,node,type_levelv);
     for (edge = dg_first_sink_ref(PED_DG(dinfo->ped),vector);
	  edge != END_OF_LIST && 
	  (dg[edge].consistent != consistent_MIV ||
	   dg[edge].symbolic);
	  edge = dg_next_sink_ref(PED_DG(dinfo->ped),edge));
     if (edge == END_OF_LIST)
       return(false);
     sub1 = tree_out(dg[edge].src);
     for (sub1 = list_first(gen_SUBSCRIPT_get_rvalue_LIST(sub1));
	  sub1 != AST_NIL;
	  sub1 = list_next(sub1))
       {
	if (pt_find_var(sub1,dinfo->index[2]))
	  count2++;
	else
	  if (pt_find_var(sub1,dinfo->index[0]) ||
	      pt_find_var(sub1,dinfo->index[1]))
	    return(false);
       }
     if (count2 <= 1)
       return(true);
     else
       return(false);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static void change_MIV_to_inconsistent(AST_INDEX     node,
			 	       dep_info_type *dinfo)
  
  {
   DG_Edge   *dg;
   int       vector;
   EDGE_INDEX edge;

     dg = dg_get_edge_structure(PED_DG(dinfo->ped));
     vector = get_info(dinfo->ped,node,type_levelv);
     for (edge = dg_first_sink_ref(PED_DG(dinfo->ped),vector);
	  edge != END_OF_LIST;
	  edge = dg_next_sink_ref(PED_DG(dinfo->ped),edge))
       if (dg[edge].consistent == consistent_MIV)
         dg[edge].consistent = inconsistent;
  }


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static void compute_coefficients(dep_info_type *dep_info)

  {
   UtilNode            *lnode;
   UtilList            *nlist;
   AST_INDEX           node,
                       prevnode;
   subscript_info_type *sptr;

     for (lnode = UTIL_HEAD(dep_info->partition);
	  lnode != NULL;
	  lnode = UTIL_NEXT(lnode))
       {
	nlist = (UtilList *)UTIL_NODE_ATOM(lnode);
	sptr = get_subscript_ptr((AST_INDEX)UTIL_NODE_ATOM(UTIL_HEAD(nlist)));
	compute_mem_addr_coeffs(dep_info,nlist);
	if (UTIL_HEAD(nlist) != UTIL_TAIL(nlist) ||
	    sptr->is_scalar[0] || sptr->is_scalar[1] | sptr->is_scalar[2])
	  {
	   node = find_oldest_value(nlist,dep_info);
	   compute_registers(dep_info,node,nlist);
	   remove_nodes(node,dep_info->ped,dep_info->inner_level,nlist);
	   prevnode = node;
	   while (UTIL_HEAD(nlist) != UTIL_TAIL(nlist))
	     {
	      node = find_oldest_value(nlist,dep_info);
	      compute_extra_regs(dep_info,node,prevnode,nlist);
	      remove_nodes(node,dep_info->ped,dep_info->inner_level,nlist);
	      prevnode = node;
	     }
	  }
	else
	  {
	   node = (AST_INDEX)UTIL_NODE_ATOM(UTIL_HEAD(nlist));
	   if (node_is_consistent_MIV(node,dep_info))
	     compute_MIV_coefficients(node,dep_info);
	   else
	     change_MIV_to_inconsistent(node,dep_info);
	  }
	util_list_free(nlist);
       }
     util_list_free(dep_info->partition);
  }


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


int mh_increase_unroll(int   max,
		       int   denom,
		       float rhoL_lp,
		       dep_info_type *dep_info)

  {
   float v;
   int x;
   int fp_regs,a_regs,mach_fp,mach_a;



     mach_fp = ((config_type *)PED_MH_CONFIG(dep_info->ped))->max_regs;
     mach_a = ((config_type *)PED_MH_CONFIG(dep_info->ped)) ->int_regs;

     x = (int)(rhoL_lp / denom);
     v = rhoL_lp / denom;
     if (v > (float) x)
       x++;
     if (x > max+1)
	x= max+1;

	fp_regs = mh_fp_register_pressure(dep_info->reg_coeff,
					  dep_info->scalar_coeff,x,1) +
					  dep_info->scalar_regs;

	a_regs = mh_addr_register_pressure(dep_info->addr_coeff,x,1);

	while(fp_regs >mach_fp || a_regs >mach_a) {
		x = x-1;
		fp_regs = mh_fp_register_pressure(dep_info->reg_coeff,
                                        dep_info->scalar_coeff,x,1) +
                                        dep_info->scalar_regs;
		a_regs = mh_addr_register_pressure(dep_info->addr_coeff,x,1);
	}

	if(x>=1)
		return(x);
	else
		return(1);

  }


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static void ComputeTwoCache(model_loop    *loop_data,
			    int           *unroll_vector,
			    int           *unroll_loops,
			    int           inner_loop,
			    dep_info_type *dep_info,
			    float         rhoL_lp)

  {
   int   x1,x2,max_x2,min_x1,temp,fp_regs,a_regs,mach_fp,mach_a,LineSize;
   float min_obj,new_obj,abs_obj,MissCost;
   
     mach_fp = ((config_type *)PED_MH_CONFIG(dep_info->ped))->max_regs;
     mach_a = ((config_type *)PED_MH_CONFIG(dep_info->ped))->int_regs;
     LineSize = ((config_type *)PED_MH_CONFIG(dep_info->ped))->line >> 3;
     MissCost =
         (float)((config_type *)PED_MH_CONFIG(dep_info->ped))->miss_cycles /
	 (float)((config_type *)PED_MH_CONFIG(dep_info->ped))->hit_cycles;
     fp_regs = mh_fp_register_pressure(dep_info->reg_coeff,
				       dep_info->scalar_coeff,1,1) + 
				       dep_info->scalar_regs;
     a_regs = mh_addr_register_pressure(dep_info->addr_coeff,1,1);
     min_obj = 100000000.0;
     x1 = 1;
     do
       {	
	x2 = 1;
	do
	  {
	   new_obj = mh_CacheBalance(dep_info->mem_coeff,
				     dep_info->PrefetchCoeff,
				     dep_info->flops,x1,x2,MissCost,
				     dep_info->PrefetchComponent,LineSize)
	               - ((config_type *)PED_MH_CONFIG(dep_info->ped))->beta_m;
	   if (new_obj < 0.0)
	     abs_obj = -new_obj;
	   else
	     abs_obj = new_obj;
	   fp_regs = mh_fp_register_pressure(dep_info->reg_coeff,
					     dep_info->scalar_coeff,x1,x2) +
					     dep_info->scalar_regs;
	   a_regs = mh_addr_register_pressure(dep_info->addr_coeff,x1,x2);
	   if (abs_obj < min_obj && fp_regs <= mach_fp && a_regs <= mach_a)
	     {
	      min_obj = abs_obj;
	      dep_info->x1 = x1;
	      dep_info->x2 = x2;
	     }
	   x2++;
	  } while (fp_regs <= mach_fp && a_regs <= mach_a && x2 <= mach_fp);
	x1++;
       } while (x1 <= mach_fp);

     if (dep_info->x1 <= loop_data[unroll_loops[0]].max)
       if (dep_info->x2 <= loop_data[unroll_loops[1]].max)
	 {
	  if (rhoL_lp > (float)(dep_info->x1*dep_info->x2*dep_info->flops) &&
	      dep_info->flops > 0)
	    {
	     dep_info->x1 = mh_increase_unroll(loop_data[unroll_loops[0]].max,
					       dep_info->x2 * dep_info->flops,
					       rhoL_lp,dep_info);
	     if (rhoL_lp > dep_info->x1 * dep_info->x2 * dep_info->flops)
	       dep_info->x2 =mh_increase_unroll(loop_data[unroll_loops[1]].max,
						dep_info->x1 * dep_info->flops,
						rhoL_lp,dep_info);
	     loop_data[inner_loop].InterlockCausedUnroll = true;
	    }
	  unroll_vector[loop_data[unroll_loops[0]].level-1] = dep_info->x1 - 1;
	  unroll_vector[loop_data[unroll_loops[1]].level-1] = dep_info->x2 - 1;
	 }
       else
	 {
	  unroll_vector[loop_data[unroll_loops[1]].level-1] = 
                          loop_data[unroll_loops[1]].max;
	  dep_info->x2 = unroll_vector[loop_data[unroll_loops[1]].level-1] + 1;
	  if (rhoL_lp > (float)(dep_info->x1*dep_info->x2*dep_info->flops) &&
	      dep_info->flops > 0)
	    {
	     dep_info->x1 = mh_increase_unroll(loop_data[unroll_loops[0]].max,
					       dep_info->x2 * dep_info->flops,
					       rhoL_lp,dep_info);
	     loop_data[inner_loop].InterlockCausedUnroll = true;
	    }
	  unroll_vector[loop_data[unroll_loops[0]].level-1] = dep_info->x1 - 1;
	 }
     else 
       if (dep_info->x2 <= loop_data[unroll_loops[1]].max)
	 {
	  unroll_vector[loop_data[unroll_loops[0]].level-1] = 
	             loop_data[unroll_loops[0]].max;
	  dep_info->x1 = unroll_vector[loop_data[unroll_loops[0]].level-1] + 1;
	  if (rhoL_lp > (float)(dep_info->x1*dep_info->x2*dep_info->flops) &&
	      dep_info->flops > 0)
	    {
	     dep_info->x2 = mh_increase_unroll(loop_data[unroll_loops[1]].max,
					       dep_info->x1 * dep_info->flops,
					       rhoL_lp,dep_info);
	     loop_data[inner_loop].InterlockCausedUnroll = true;
	    }
	  unroll_vector[loop_data[unroll_loops[1]].level-1] = dep_info->x2 - 1;
	 }
       else
	 {
	  unroll_vector[loop_data[unroll_loops[0]].level-1] = 
	                loop_data[unroll_loops[0]].max;
	  unroll_vector[loop_data[unroll_loops[1]].level-1] = 
	                loop_data[unroll_loops[1]].max;
	  if (loop_data[unroll_loops[0]].max == 0 &&
	      loop_data[unroll_loops[1]].max == 0)
	    {
	     if (NOT(loop_data[unroll_loops[0]].distribute) ||
		 NOT(loop_data[unroll_loops[1]].distribute))
	       loop_data[inner_loop].Distribute = true;
	     if (NOT(loop_data[unroll_loops[0]].interchange) ||
		 NOT(loop_data[unroll_loops[0]].interchange))
	       loop_data[inner_loop].Interchange = true;
	    }
	 }
  }
  

/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static void ComputeOneCache(model_loop    *loop_data,
			    int           *unroll_vector,
			    int           unroll_loop,
			    int           inner_loop,
			    dep_info_type *dep_info,
			    float           rhoL_lp)

  {
   float min_obj,new_obj,abs_obj,MissCost;
   int   x,min_x,max_x,fp_regs,a_regs,mach_fp,mach_a,LineSize;

     mach_fp = ((config_type *)PED_MH_CONFIG(dep_info->ped))->max_regs;
     mach_a = ((config_type *)PED_MH_CONFIG(dep_info->ped))->int_regs;
     LineSize = ((config_type *)PED_MH_CONFIG(dep_info->ped))->line >> 3;
     MissCost =
          (float)((config_type *)PED_MH_CONFIG(dep_info->ped))->miss_cycles /
	  (float)((config_type *)PED_MH_CONFIG(dep_info->ped))->hit_cycles;
     min_obj = 100000000.0;
     x = 1;
     do
       {
	new_obj = mh_CacheBalance(dep_info->mem_coeff,
				  dep_info->PrefetchCoeff,
				  dep_info->flops,x,1,MissCost,
				  dep_info->PrefetchComponent,LineSize) -
			 ((config_type *)PED_MH_CONFIG(dep_info->ped))->beta_m;
	if (new_obj < 0.0)
	  abs_obj = -new_obj;
	else
	  abs_obj = new_obj;
	fp_regs = mh_fp_register_pressure(dep_info->reg_coeff,
					  dep_info->scalar_coeff,x,1) + 
					  dep_info->scalar_regs;
	a_regs = mh_addr_register_pressure(dep_info->addr_coeff,x,1);
	if (abs_obj < min_obj && a_regs <= mach_a && fp_regs <= mach_fp)
	  {
	   min_obj = abs_obj;
	   dep_info->x1 = x;
	  }
	x++;
       } while (x <= mach_fp && a_regs <= mach_a && fp_regs <= mach_fp);
     if (dep_info->x1 <= loop_data[unroll_loop].max)
       {
	if (rhoL_lp > (float)(dep_info->x1*dep_info->x2*dep_info->flops) &&
	    dep_info->flops > 0)
	  {
	   dep_info->x1 = mh_increase_unroll(loop_data[unroll_loop].max,
					     dep_info->x2 * dep_info->flops,
					     rhoL_lp,dep_info);
	   loop_data[inner_loop].InterlockCausedUnroll = true;
	  }
	unroll_vector[loop_data[unroll_loop].level-1]= dep_info->x1 - 1;
       }
     else
       {
	unroll_vector[loop_data[unroll_loop].level-1] = 
                  loop_data[unroll_loop].max;
	if (loop_data[unroll_loop].max == 0)
	  {
	   if (NOT(loop_data[unroll_loop].distribute))
	     loop_data[inner_loop].Distribute = true;
	   if (NOT(loop_data[unroll_loop].interchange))
	     loop_data[inner_loop].Interchange = true;
	  }
       }
  }   
  
/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static void compute_two_loops(model_loop    *loop_data,
			      int           *unroll_vector,
			      int           *unroll_loops,
			      int           inner_loop,
			      dep_info_type *dep_info,
			      float         rhoL_lp)

  {
   int   x1,x2,max_x2,min_x1,temp,fp_regs,a_regs,mach_fp,mach_a,LineSize;
   float min_obj,new_obj,abs_obj,MissCost;
   
     if ((loop_data[unroll_loops[0]].count == 0 &&
	  dep_info->scalar_coeff[2] == 0 && dep_info->x2 == 1 && 
	  loop_data[unroll_loops[1]].count == 0 &&
	  dep_info->scalar_coeff[1] == 0 && dep_info->x1 == 1) ||
	 (dep_info->reg_coeff[0][2][2] == 0 && 
	  dep_info->reg_coeff[1][2][2] == 0 &&
	  dep_info->reg_coeff[2][2][2] == 0 && 
	  dep_info->scalar_coeff[1] == 0 && 
	  dep_info->scalar_coeff[2] == 0))
       {
	dep_info->x1 = 1;
	dep_info->x2 = 1;
	loop_data[inner_loop].NoImprovement = true;
       }
     else
       {
	mach_fp = ((config_type *)PED_MH_CONFIG(dep_info->ped))->max_regs;
	mach_a = ((config_type *)PED_MH_CONFIG(dep_info->ped))->int_regs;
	LineSize = ((config_type *)PED_MH_CONFIG(dep_info->ped))->line >> 3;
	MissCost=(float)((config_type *)PED_MH_CONFIG(dep_info->ped))->miss_cycles /
	         (float)((config_type *)PED_MH_CONFIG(dep_info->ped))->hit_cycles;
	fp_regs = mh_fp_register_pressure(dep_info->reg_coeff,
					  dep_info->scalar_coeff,1,1) + 
					  dep_info->scalar_regs;
	a_regs = mh_addr_register_pressure(dep_info->addr_coeff,1,1);
	if (fp_regs <= mach_fp && a_regs <= mach_a)
	  {	
	   x1 = mach_fp;
	   x2 = 1;
	   min_obj = (float)MAXINT;
	   while(min_obj > 0.01 && x1 >= 1 && x2 <= mach_fp)
	     {
	      new_obj = mh_loop_balance(dep_info->mem_coeff,dep_info->flops,
					x1,x2)
	                  - ((config_type *)PED_MH_CONFIG(dep_info->ped))->beta_m;
	      if (new_obj < 0.0)
	        abs_obj = -new_obj;
	      else
	        abs_obj = new_obj;
	      fp_regs = mh_fp_register_pressure(dep_info->reg_coeff,
						dep_info->scalar_coeff,x1,x2) +
						dep_info->scalar_regs;
	      a_regs = mh_addr_register_pressure(dep_info->addr_coeff,x1,x2);
	      if (abs_obj < min_obj && fp_regs <= mach_fp && a_regs <= mach_a)
		{
		 min_obj = abs_obj;
		 dep_info->x1 = x1;
		 dep_info->x2 = x2;
		}
	      if (new_obj > 0.01 && fp_regs <= mach_fp && a_regs <= mach_a)
		 x2++;
	      else 
		 x1--;
	     }
	  }
	else
	  {
	   dep_info->x1 = 1;
	   dep_info->x2 = 1;
	  }
       }

      /* sometimes loops that do not carry dependences will be considered
	 because they contain multiple inner loops at the same level, where
	 one loop body has dependence carried at that level and another does
	 not.  This situation will not be caught earlier by checking the
	 dependence count of a loop.  Therefore, this is a hack to check if
	 unrolling one loop does nothing. */

      if (dep_info->x1 > 1)
        {
	 new_obj = mh_loop_balance(dep_info->mem_coeff,dep_info->flops,
				   1,x2)
	               - ((config_type *)PED_MH_CONFIG(dep_info->ped))->beta_m;
	 if (new_obj < 0.0)
	   abs_obj = -new_obj;
	 else
	   abs_obj = new_obj;
	 if (abs_obj == min_obj)
	   dep_info->x1 = 1;
	}

     if (dep_info->x1 <= loop_data[unroll_loops[0]].max)
       if (dep_info->x2 <= loop_data[unroll_loops[1]].max)
	 {
	  if (rhoL_lp > (float)(dep_info->x1*dep_info->x2*dep_info->flops) &&
	      dep_info->flops > 0)
	    {
	     dep_info->x1 = mh_increase_unroll(loop_data[unroll_loops[0]].max,
					       dep_info->x2 * dep_info->flops,
					       rhoL_lp,dep_info);
	     if (rhoL_lp > dep_info->x1 * dep_info->x2 * dep_info->flops)
	       dep_info->x2 =mh_increase_unroll(loop_data[unroll_loops[1]].max,
						dep_info->x1 * dep_info->flops,
						rhoL_lp,dep_info);
	     loop_data[inner_loop].InterlockCausedUnroll = true;
	    }
	  unroll_vector[loop_data[unroll_loops[0]].level-1] = dep_info->x1 - 1;
	  unroll_vector[loop_data[unroll_loops[1]].level-1] = dep_info->x2 - 1;
	 }
       else
	 {
	  unroll_vector[loop_data[unroll_loops[1]].level-1] = 
                          loop_data[unroll_loops[1]].max;
	  dep_info->x2 = unroll_vector[loop_data[unroll_loops[1]].level-1] + 1;
	  if (rhoL_lp > (float)(dep_info->x1*dep_info->x2*dep_info->flops) &&
	      dep_info->flops > 0)
	    {
	     dep_info->x1 = mh_increase_unroll(loop_data[unroll_loops[0]].max,
					       dep_info->x2 * dep_info->flops,
					       rhoL_lp,dep_info);
	     loop_data[inner_loop].InterlockCausedUnroll = true;
	    }
	  unroll_vector[loop_data[unroll_loops[0]].level-1] = dep_info->x1 - 1;
	 }
     else 
       if (dep_info->x2 <= loop_data[unroll_loops[1]].max)
	 {
	  unroll_vector[loop_data[unroll_loops[0]].level-1] = 
	             loop_data[unroll_loops[0]].max;
	  dep_info->x1 = unroll_vector[loop_data[unroll_loops[0]].level-1] + 1;
	  if (rhoL_lp > (float)(dep_info->x1*dep_info->x2*dep_info->flops) &&
	      dep_info->flops > 0)
	    {
	     dep_info->x2 = mh_increase_unroll(loop_data[unroll_loops[1]].max,
					       dep_info->x1 * dep_info->flops,
					       rhoL_lp,dep_info);
	     loop_data[inner_loop].InterlockCausedUnroll = true;
	    }
	  unroll_vector[loop_data[unroll_loops[1]].level-1] = dep_info->x2 - 1;
	 }
       else
	 {
	  unroll_vector[loop_data[unroll_loops[0]].level-1] = 
	                loop_data[unroll_loops[0]].max;
	  unroll_vector[loop_data[unroll_loops[1]].level-1] = 
	                loop_data[unroll_loops[1]].max;
	  if (loop_data[unroll_loops[0]].max == 0 &&
	      loop_data[unroll_loops[1]].max == 0)
	    {
	     if (NOT(loop_data[unroll_loops[0]].distribute) ||
		 NOT(loop_data[unroll_loops[1]].distribute))
	       loop_data[inner_loop].Distribute = true;
	     if (NOT(loop_data[unroll_loops[0]].interchange) ||
		 NOT(loop_data[unroll_loops[0]].interchange))
	       loop_data[inner_loop].Interchange = true;
	    }
	 }
  }
  

/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static void compute_one_loop(model_loop    *loop_data,
			     int           *unroll_vector,
			     int           unroll_loop,
			     int           inner_loop,
			     dep_info_type *dep_info,
			     float           rhoL_lp)

  {
   float min_obj,new_obj,abs_obj,MissCost;
   int   x,min_x,max_x,fp_regs,a_regs,mach_fp,mach_a,LineSize;

     if ((loop_data[unroll_loop].count == 0  &&
	  dep_info->scalar_coeff[2] == 0 && dep_info->x2 == 1) ||
	 (dep_info->reg_coeff[0][2][2] == 0 && 
	  dep_info->reg_coeff[1][2][2] == 0 &&
	  dep_info->reg_coeff[2][2][2] == 0 && 
	  dep_info->scalar_coeff[2] == 0))
       {
	dep_info->x1 = 1;
	loop_data[inner_loop].NoImprovement = true;
       }
     else
       {
	mach_fp = ((config_type *)PED_MH_CONFIG(dep_info->ped))->max_regs;
	mach_a = ((config_type *)PED_MH_CONFIG(dep_info->ped))->int_regs;
	LineSize = ((config_type *)PED_MH_CONFIG(dep_info->ped))->line >> 3;
	MissCost=(float)((config_type *)PED_MH_CONFIG(dep_info->ped))->miss_cycles /
	         (float)((config_type *)PED_MH_CONFIG(dep_info->ped))->hit_cycles;
	fp_regs = mh_fp_register_pressure(dep_info->reg_coeff,
					  dep_info->scalar_coeff,1,1) + 
					  dep_info->scalar_regs;
	a_regs = mh_addr_register_pressure(dep_info->addr_coeff,1,1);
	if (fp_regs <= mach_fp && a_regs <= mach_a)
	  {	
	   min_obj = (float)MAXINT;
	   min_x = 1;
	   max_x = mach_fp;
	   while(min_x <= max_x && min_obj > 0.01)
	     {
	      x = (max_x + min_x) >> 1;
	      new_obj = mh_loop_balance(dep_info->mem_coeff,dep_info->flops,
					x,1) -
			 ((config_type *)PED_MH_CONFIG(dep_info->ped))->beta_m;
	      if (new_obj < 0.0)
	        abs_obj = -new_obj;
	      else
	        abs_obj = new_obj;
	      fp_regs = mh_fp_register_pressure(dep_info->reg_coeff,
						dep_info->scalar_coeff,x,1) + 
						dep_info->scalar_regs;
	      a_regs = mh_addr_register_pressure(dep_info->addr_coeff,x,1);
	      if (abs_obj < min_obj && a_regs <= mach_a && fp_regs <= mach_fp)
		{
		 min_obj = abs_obj;
		 dep_info->x1 = x;
		 if (new_obj > 0.0)
		   min_x = x + 1;
		 else
		   max_x = x - 1;
		}
	      else
	        max_x = x - 1;
	     }
	  }
	else
	  dep_info->x1 = 1;
       }
     if (dep_info->x1 <= loop_data[unroll_loop].max)
       {
	if (rhoL_lp > (float)(dep_info->x1*dep_info->x2*dep_info->flops) &&
	    dep_info->flops > 0)
	  {
	   dep_info->x1 = mh_increase_unroll(loop_data[unroll_loop].max,
					     dep_info->x2 * dep_info->flops,
					     rhoL_lp,dep_info);
	   loop_data[inner_loop].InterlockCausedUnroll = true;
	  }
	unroll_vector[loop_data[unroll_loop].level-1]= dep_info->x1 - 1;
       }
     else
       {
	unroll_vector[loop_data[unroll_loop].level-1] = 
                  loop_data[unroll_loop].max;
	if (loop_data[unroll_loop].max == 0)
	  {
	   if (NOT(loop_data[unroll_loop].distribute))
	     loop_data[inner_loop].Distribute = true;
	   if (NOT(loop_data[unroll_loop].interchange))
	     loop_data[inner_loop].Interchange = true;
	  }
       }
  }   
  
/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static void do_computation(model_loop    *loop_data,
			   int           loop,
			   int           *unroll_vector,
			   int           *unroll_loops,
			   int           count,
			   PedInfo       ped,
			   SymDescriptor symtab,
			   arena_type    *ar)

  {
   int           i,j,k,regs,LineSize; float         rhoL_lp,bal,MissCost;
   dep_info_type dep_info;
   reg_info_type reg_info;
   AST_INDEX     step;

     dep_info.ar = ar;
     dep_info.loop_data = loop_data;
     if (count == 2)
       {
	dep_info.level1 = loop_data[unroll_loops[0]].level;
	dep_info.level2 = loop_data[unroll_loops[1]].level;
	dep_info.index[0] = gen_get_text(gen_INDUCTIVE_get_name(
                        gen_DO_get_control(loop_data[unroll_loops[0]].node)));
	dep_info.index[1] = gen_get_text(gen_INDUCTIVE_get_name(
			gen_DO_get_control(loop_data[unroll_loops[1]].node)));
	step = gen_INDUCTIVE_get_rvalue3(gen_DO_get_control(
                                          loop_data[unroll_loops[0]].node));
	if (step == AST_NIL)
	  dep_info.step[0] = 1;
	else if (pt_eval(step,&dep_info.step[0]));
	step = gen_INDUCTIVE_get_rvalue3(gen_DO_get_control(
                                          loop_data[unroll_loops[1]].node));
	if (step == AST_NIL)
	  dep_info.step[1] = 1;
	else if (pt_eval(step,&dep_info.step[1]));
       }
     else if (count == 1)
       {
	dep_info.level1 = loop_data[unroll_loops[0]].level;
	dep_info.level2 = 0;
	dep_info.index[0] = gen_get_text(gen_INDUCTIVE_get_name(
                        gen_DO_get_control(loop_data[unroll_loops[0]].node)));
	dep_info.index[1] = NULL;
	step = gen_INDUCTIVE_get_rvalue3(gen_DO_get_control(
                                          loop_data[unroll_loops[0]].node));
	if (step == AST_NIL)
	  dep_info.step[0] = 1;
	else if (pt_eval(step,&dep_info.step[0]));
       }
     else
       {
	dep_info.level1 = 0;
	dep_info.level2 = 0;
	dep_info.index[0] = NULL;
	dep_info.index[1] = NULL;
       }
     dep_info.inner_level = loop_data[loop].level;
     dep_info.index[2] = gen_get_text(gen_INDUCTIVE_get_name(
                        gen_DO_get_control(loop_data[loop].node)));
     step = gen_INDUCTIVE_get_rvalue3(gen_DO_get_control(
                                         loop_data[loop].node));
     if (step == AST_NIL)
       dep_info.step[2] = 1;
     else if (pt_eval(step,&dep_info.step[2]));
     for (i = 0; i < 4; i++)
       {
	for (j = 0; j < 3; j++)
	  for (k = 0; k < 3; k++)
	    {
	     dep_info.reg_coeff[i][j][k] = 0;
	     dep_info.mem_coeff[i][j][k] = 0;
	     dep_info.addr_coeff[i][j][k] = 0;
	     dep_info.PrefetchCoeff[i][j][k] = 0.0;
	     dep_info.PrefetchComponent[i][j][k].fraction = 0;
	     dep_info.PrefetchComponent[i][j][k].ceil_fraction = 0;
	     dep_info.PrefetchComponent[i][j][k].unit = 0;
	    }
	if (i < 3)
	  dep_info.scalar_coeff[i] = 0;
       }
     dep_info.scalar_regs = 0;
     dep_info.flops = 0;
     dep_info.x1 = 1;
     dep_info.x2 = 1;
     dep_info.ped = ped;
     fst_InitField(symtab,FIRST,true,0);
     dep_info.symtab = symtab;
     reg_info.expr_regs = 0;
     reg_info.config = (config_type *)PED_MH_CONFIG(ped);
     reg_info.symtab = symtab;
     walk_statements(loop_data[loop].node,loop_data[loop].level,(WK_STMT_CLBACK)count_regs,(WK_STMT_CLBACK)
		     NOFUNC,(Generic)&reg_info);
     if (((config_type *)PED_MH_CONFIG(ped))->chow_alloc && 
	 reg_info.expr_regs < 4)

	/* reserve at least 4 register for a Chow-style register allocator
	     for expressions because of high interference */

       dep_info.scalar_regs += 4;
     else
       dep_info.scalar_regs += reg_info.expr_regs;
     fst_KillField(symtab,FIRST);
     walk_expression(loop_data[loop].node,(WK_EXPR_CLBACK)survey_edges,(WK_EXPR_CLBACK)NOFUNC,
		     (Generic)&dep_info);
     dep_info.partition = util_list_alloc((Generic)NULL,(char *)NULL);
     walk_expression(loop_data[loop].node,(WK_EXPR_CLBACK)partition_names,(WK_EXPR_CLBACK)NOFUNC,
		     (Generic)&dep_info);
     compute_coefficients(&dep_info);
     MissCost =
        (float)((config_type *)PED_MH_CONFIG(dep_info.ped))->miss_cycles /
	(float)((config_type *)PED_MH_CONFIG(dep_info.ped))->hit_cycles;
     LineSize = ((config_type *)PED_MH_CONFIG(dep_info.ped))->line >> 3;
     if (mc_unroll_cache)
       loop_data[loop].ibalance = mh_CacheBalance(dep_info.mem_coeff,
						  dep_info.PrefetchCoeff,
						  dep_info.flops,1,1,
						  MissCost,
						  dep_info.PrefetchComponent,
						  LineSize);
     else
       loop_data[loop].ibalance = mh_loop_balance(dep_info.mem_coeff,
						  dep_info.flops,1,1);
     rhoL_lp = loop_data[loop].rho * 
	          ((config_type *)PED_MH_CONFIG(ped))->pipe_length;
     if ((loop_data[loop].ibalance -
	  ((config_type *)PED_MH_CONFIG(ped))->beta_m) <= 0.0)
       {
	if (count == 2)
	  unroll_vector[loop_data[unroll_loops[1]].level-1] = 0;
	if (rhoL_lp > (float)(dep_info.flops) && dep_info.flops > 0)
	  {
	   unroll_vector[loop_data[unroll_loops[0]].level-1] = 
	      mh_increase_unroll(loop_data[unroll_loops[0]].max,dep_info.flops,rhoL_lp,&dep_info); 
	   if (mc_unroll_cache)
	     {
	      loop_data[loop].fbalance = mh_CacheBalance(dep_info.mem_coeff,
							 dep_info.PrefetchCoeff,
							 dep_info.flops,
				    unroll_vector[loop_data[unroll_loops[0]].level-1]+1,
							 1,MissCost,
							 dep_info.PrefetchComponent,
							 LineSize);
	      loop_data[loop].P_L = mh_PrefetchRequirements(dep_info.PrefetchCoeff,
							    dep_info.flops,
				    unroll_vector[loop_data[unroll_loops[0]].level-1]+1,
							    1,
							    dep_info.PrefetchComponent,
							    LineSize);
	     }
	   else
	     loop_data[loop].fbalance= mh_loop_balance(dep_info.mem_coeff,dep_info.flops,
				    unroll_vector[loop_data[unroll_loops[0]].level-1]+1,
						      1);
	   loop_data[loop].registers = mh_fp_register_pressure(dep_info.reg_coeff,
							       dep_info.scalar_coeff,
				     unroll_vector[loop_data[unroll_loops[0]].level-1]+1,
							       1) + dep_info.scalar_regs;
	   loop_data[loop].InterlockCausedUnroll = true;
	  }
	else
	  {
	   unroll_vector[loop_data[unroll_loops[0]].level-1] = 0;
	   loop_data[loop].fbalance = loop_data[loop].ibalance;
	   loop_data[loop].registers = mh_fp_register_pressure(dep_info.reg_coeff,
							       dep_info.scalar_coeff,1,
							       1) + dep_info.scalar_regs;
	  }
       }
     else
       {
	if (count == 2)
	  if (mc_unroll_cache)
	    ComputeTwoCache(loop_data,unroll_vector,unroll_loops,loop,
			    &dep_info,rhoL_lp);
	  else
	    compute_two_loops(loop_data,unroll_vector,unroll_loops,loop,
			      &dep_info, rhoL_lp);
	else if (count == 1)
	  if (mc_unroll_cache)
	    ComputeOneCache(loop_data,unroll_vector,unroll_loops[0],loop,
			    &dep_info,rhoL_lp);
	  else
	    compute_one_loop(loop_data,unroll_vector,unroll_loops[0],loop,
			     &dep_info,rhoL_lp);
	else if ((regs = dep_info.reg_coeff[0][2][2] + 
		  dep_info.scalar_coeff[0]) > 0)
	  loop_data[unroll_loops[0]].max = (((config_type *)PED_MH_CONFIG(ped))
	                                  ->max_regs - dep_info.scalar_regs) /
					  regs - 1;
	if (mc_unroll_cache)
	  {
	   loop_data[loop].fbalance = mh_CacheBalance(dep_info.mem_coeff,
						      dep_info.PrefetchCoeff,
						      dep_info.flops,dep_info.x1,
						      dep_info.x2,MissCost,
						      dep_info.PrefetchComponent,
						      LineSize);
	   loop_data[loop].P_L = mh_PrefetchRequirements(dep_info.PrefetchCoeff,
							 dep_info.flops,dep_info.x1,
							 dep_info.x2,
							 dep_info.PrefetchComponent,
							 LineSize);
	  }
	else
	  loop_data[loop].fbalance = mh_loop_balance(dep_info.mem_coeff,
						   dep_info.flops,dep_info.x1,
						   dep_info.x2);
	loop_data[loop].registers = 
	      mh_fp_register_pressure(dep_info.reg_coeff,dep_info.scalar_coeff,
				      dep_info.x1,dep_info.x2) +
				      dep_info.scalar_regs;
       }

       machine_info = &dep_info;
  }
      

/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


static void compute_values(model_loop    *loop_data,
			   int           loop,
			   int           *unroll_vector,
			   int           *unroll_loops,
			   int           count,
			   PedInfo       ped,
			   SymDescriptor symtab,
			   arena_type    *ar)

  {
   int i;

     if (loop_data[loop].inner_loop == -1)
       if (count > 0)
         do_computation(loop_data,loop,unroll_vector,unroll_loops,count,ped,
			symtab,ar);
       else if (loop_data[loop].reduction)
	 {
	  unroll_loops[count] = loop;
	  do_computation(loop_data,loop,unroll_vector,unroll_loops,count,ped,
			 symtab,ar);
	 }
       else
         loop_data[loop].NoImprovement = true;
     else
       {
	if (loop_data[loop].unroll)
	  unroll_loops[count++] = loop;
	i = loop_data[loop].inner_loop;
	while(i != -1)
	  {
	   compute_values(loop_data,i,unroll_vector,unroll_loops,count,ped,
			  symtab,ar);
	   i = loop_data[i].next_loop;
	   if (i != -1)
	     {
	      loop_data[i].unroll_vector= (int *)ar->arena_alloc_mem_clear(LOOP_ARENA,
									   MAXLOOP * 
									   sizeof(int));
	      unroll_vector = loop_data[i].unroll_vector;
	     }
	   
	  }
       }
  }


/****************************************************************************/
/*                                                                          */
/*    Function:
/*                                                                          */
/*    Input:
/*                                                                          */
/*    Description:
/*                                                                          */
/****************************************************************************/


void mh_compute_unroll_amounts(model_loop    *loop_data,
			       int           size,
			       int           num_loops,
			       PedInfo       ped,
			       SymDescriptor symtab,
			       arena_type    *ar)
			       
  {
   int unroll_loops[2];

     pick_loops(loop_data,size,num_loops,ped,symtab,ar);
     loop_data[0].unroll_vector = (int *)ar->arena_alloc_mem_clear(LOOP_ARENA,
								   MAXLOOP * 
								   sizeof(int));
     compute_values(loop_data,0,loop_data[0].unroll_vector,unroll_loops,0,ped,
		    symtab,ar);
  }
