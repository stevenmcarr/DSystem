/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
#include <mh.h>
#include <mem_util.h>
#include <compute_uj.h>
#include <balance.h>
#include <gi.h>

static int determine_uj_prof(AST_INDEX      stmt,
			     int            level,
			     comp_info_type *comp_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

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
	     {
	      inner = lvl;
	      l = lvl+1;
	      while (l < gen_get_dt_LVL(&dg[edge]) && inner != -1)
		{
		 if (gen_get_dt_DIS(&dg[edge],l) > 0 || 
		     gen_get_dt_DIS(&dg[edge],l) == DDATA_ANY)
		   if (inner == lvl)
		     inner = l;
		   else
		     inner = -1;
		 l++;
		}
	      if (inner != -1)
	        if (lvl == inner && comp_info->num_loops == 2)
		  {
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
	    
static void pick_loops_to_unroll(model_loop         *loop_data,
				 int                loop,
				 comp_info_type     comp_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

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

static void pick_loops(model_loop *loop_data,
		       int        size,
		       int        num_loops,
		       PedInfo    ped,
		       SymDescriptor symtab,
		       arena_type *ar)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

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
   walk_statements(loop_data[0].node,loop_data[0].level,determine_uj_prof,
		   NOFUNC,(Generic)&comp_info);
   pick_loops_to_unroll(loop_data,0,comp_info);
  }

static Boolean edge_creates_pressure(DG_Edge       *edge,
				     dep_info_type *dep_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

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

static void do_partition(AST_INDEX name,
			 UtilList  *nlist,
			 DG_Edge   *dg,
			 dep_info_type *dinfo)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
 

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

static int partition_names(AST_INDEX      node,
			   dep_info_type *dinfo)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

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

static void check_incoming_edges(AST_INDEX     node,
				 dep_info_type *dep_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

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
	     case dg_anti: if (dg[edge].consistent == consistent_SIV || 
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



static int survey_edges(AST_INDEX     node,
			dep_info_type *dep_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

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
	       dep_info->flops += ((config_type *)PED_MH_CONFIG(dep_info->ped))
	                          ->mul_cycles * ops;
	     else if (is_binary_plus(node) || is_binary_minus(node))
	       dep_info->flops += ((config_type *)PED_MH_CONFIG(dep_info->ped))
	                          ->add_cycles * ops;
	     else if (is_binary_divide(node))
	       dep_info->flops += ((config_type *)PED_MH_CONFIG(dep_info->ped))
	                          ->div_cycles * ops;
	     else
               dep_info->flops += ops; 
	    }
	  else;
	else;
     else if (is_identifier(node) && 
	      fst_GetField(dep_info->symtab,gen_get_text(node),
			   SYMTAB_NUM_DIMS) == 0)
       if (fst_GetField(dep_info->symtab,gen_get_text(node),FIRST))
	 {
	  fst_PutField(dep_info->symtab,gen_get_text(node),FIRST,false);
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


static int count_regs(AST_INDEX     stmt,
		      int           level,
		      reg_info_type *reg_info)

  {
   int stmt_regs = 0;

     stmt_regs = reg_info->expr_regs;
     if (is_assignment(stmt))
       walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),NOFUNC,get_expr_regs,
		       (Generic)reg_info);
     else if (is_guard(stmt))
       walk_expression(gen_GUARD_get_rvalue(stmt),NOFUNC,get_expr_regs,
		       (Generic)reg_info);
     else if (is_arithmetic_if(stmt))
       walk_expression(gen_ARITHMETIC_IF_get_rvalue(stmt),NOFUNC,get_expr_regs,
		       (Generic)reg_info);
     else if (is_logical_if(stmt))
       walk_expression(gen_LOGICAL_IF_get_rvalue(stmt),NOFUNC,get_expr_regs,
		       (Generic)reg_info);
     else if (is_read_short(stmt))
       walk_expression(gen_READ_SHORT_get_data_vars_LIST(stmt),NOFUNC,
		       get_expr_regs,(Generic)reg_info);
     else if (is_write(stmt))
       walk_expression(gen_WRITE_get_data_vars_LIST(stmt),NOFUNC,get_expr_regs,
		       (Generic)reg_info);
     if (stmt_regs > reg_info->expr_regs)
       reg_info->expr_regs = stmt_regs;
     return(WALK_CONTINUE);
  }

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

	       /* try to handle scalar array refs correctly so that we can 
		  find the oldest value */
	     (!pt_expr_equal(tree_out(dg[edge].src),tree_out(dg[edge].sink)) ||
	      (ut_get_stmt(dg[edge].src) != ut_get_stmt(dg[edge].sink) &&
	       dg[edge].level == LOOP_INDEPENDENT)) &&

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

static void summarize_distance_vector(int *dvec,
				      int *num_edges,
				      AST_INDEX node,
				      PedInfo ped,
				      UtilList *nlist,
				      dep_info_type *dinfo)
  { 
   DG_Edge *dg;
   EDGE_INDEX edge;
   int refl,i,dist1;
   Boolean first = true;

     dg = dg_get_edge_structure( PED_DG(ped));
     refl = get_info(ped,node,type_levelv);
     for (edge = dg_first_src_ref( PED_DG(ped),refl);
	  edge != END_OF_LIST;
	  edge = dg_next_src_ref( PED_DG(ped),edge))
       if (dg[edge].consistent == consistent_SIV && !dg[edge].symbolic &&
	   dg[edge].type == dg_true || dg[edge].type == dg_input &&
	   get_subscript_ptr(dg[edge].src)->surrounding_do ==
	   get_subscript_ptr(dg[edge].sink)->surrounding_do)
         if (edge_creates_pressure(&dg[edge],dinfo) &&
	     util_in_list(get_subscript_ptr(dg[edge].sink)->lnode,nlist))
	   { 
	    (*num_edges)++;
	    for (i = 1; i < gen_get_dt_LVL(&dg[edge]); i++)
	      {
	       if ((dist1 = gen_get_dt_DIS(&dg[edge],i)) < DDATA_BASE)
	         if (i == dinfo->level1 || i == dinfo->level2 ||
		     i == dinfo->inner_level)
	           dist1 = 1;
		 else
		   dist1 = 0;
	       else if (dist1 < 0)
	         dist1 = -dist1;
	       if (get_vec_DIS(dvec,i) > dist1 || first)
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
	    if (get_vec_DIS(dvec,i) < dist1 || first)
	      put_vec_DIS(dvec,i,dist1);
	    first = false;
	   } 
  } 


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


static void compute_registers(dep_info_type *dep_info, 
			      int *dvect,
			      AST_INDEX node,
			      int num_edges) 

  {
   subscript_info_type *subp;
   int refs, regs, dist1, dist2, distn; 

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
	      if (!subp->is_scalar[2] && !subp->is_scalar[1])
		{ 
		 dep_info->mem_coeff[2] += refs * num_edges; 
		 if (subp->store)
		   dep_info->mem_coeff[2] += refs; 
		 if (index_in_outer_subscript(node,dep_info->index[1]))
		   dep_info->addr_coeff[1] += num_edges;
		}
	     }
	   else if (!subp->is_scalar[2])
	     dep_info->scalar_coeff[1] += regs;
	   if (subp->is_scalar[1])
	     {
	      if (dep_info->x2 == 1)
	        dep_info->x2 = 2;
	      if (!subp->is_scalar[2] && !subp->is_scalar[0])
		{
		 dep_info->mem_coeff[1] += refs * num_edges;
		 if (subp->store)
		   dep_info->mem_coeff[1] += refs; 
		 if (index_in_outer_subscript(node,dep_info->index[0]))
		   dep_info->addr_coeff[2] += num_edges;
		}
	     }
	   else if (!subp->is_scalar[2])
	     dep_info->scalar_coeff[2] += regs;
	  }
       }
     else
       {
	get_distances(dep_info,dvect,&dist1,&dist2,&distn);
	dep_info->reg_coeff[0] += (distn + 1) * regs;
	dep_info->reg_coeff[1] -= dist2 * (distn + 1) * regs;
	dep_info->reg_coeff[2] -= dist1 * (distn + 1) * regs;
	dep_info->reg_coeff[3] += dist1 * dist2 * (distn + 1) * regs;
	dep_info->mem_coeff[1] += dist2 * refs * num_edges;
	dep_info->mem_coeff[2] += dist1 * refs * num_edges;
	dep_info->mem_coeff[3] -= dist1 * dist2 * refs * num_edges;
	if (index_in_outer_subscript(node,dep_info->index[0]))
	  if (index_in_outer_subscript(node,dep_info->index[1]))
	    {
	     dep_info->addr_coeff[1] += dist2 * num_edges;
	     dep_info->addr_coeff[2] += dist1 * num_edges;
	     dep_info->addr_coeff[3] -= dist1 * dist2 * num_edges;
	    }
	  else
	    {
	     dep_info->addr_coeff[1] += num_edges;
	     dep_info->addr_coeff[3] -= dist1 * num_edges;
	    }
	else if (index_in_outer_subscript(node,dep_info->index[1]))
	  {
	   dep_info->addr_coeff[2] += num_edges;
	   dep_info->addr_coeff[3] -= dist2 * num_edges;
	  }
	if (dist1 > dep_info->x1)
	  dep_info->x1 = dist1;
	if (dist2 > dep_info->x2)
	  dep_info->x2 = dist2;
       }
  }


static void compute_extra_regs(dep_info_type *dinfo,
			       int           *dvect,
			       AST_INDEX     node,
			       AST_INDEX     prevnode,
			       int           num_edges)

  {
   DG_Edge *dg;
   EDGE_INDEX edge;
   int     vector;
   int     dist1,sdist1,
           dist2,sdist2,
           distn,sdistn,regs,refs;

     dg = dg_get_edge_structure( PED_DG(dinfo->ped));
     vector = get_info(dinfo->ped,node,type_levelv);
     for (edge = dg_first_sink_ref( PED_DG(dinfo->ped),vector);
	  dg[edge].src != prevnode && edge != END_OF_LIST;
	  edge = dg_next_sink_ref( PED_DG(dinfo->ped),edge));
     get_machine_parms(node,&regs,&refs,dinfo);
     if (edge == END_OF_LIST)
       {
	/* we have found an intrastatement loop independent antidependence
	   add memory references */
	dinfo->mem_coeff[3] += refs;
       }
     else
       {
	get_distances(dinfo,dg[edge].dt_data,&dist1,&dist2,&distn);
	get_distances(dinfo,dvect,&sdist1,&sdist2,&sdistn);
	if (dist1 != 0)
	  if (dist2 != 0)
	    dinfo->reg_coeff[3] += dist1 * dist2 * (sdistn + 1) * regs;
	  else
	    {
	     dinfo->reg_coeff[2] += dist1 * (sdistn + 1) * regs;
	     dinfo->reg_coeff[3] -= dist1 * sdist2 * (sdistn + 1) * regs;
	    }
	else
	  {
	   dinfo->reg_coeff[1] += dist2 * (sdistn + 1) * regs;
	   dinfo->reg_coeff[3] -= sdist1 * dist2 * (sdistn + 1) * regs;
	  }
       }
  }

static void compute_mem_addr_coeffs(dep_info_type *dep_info,
				    AST_INDEX     node)

  {
   if (gen_get_converted_type(tree_out(node)) == TYPE_COMPLEX ||
       gen_get_converted_type(tree_out(node)) == TYPE_DOUBLE_PRECISION)
     dep_info->mem_coeff[0] += 
	     ((config_type *)PED_MH_CONFIG(dep_info->ped))->double_fetches;
   else
     dep_info->mem_coeff[0]++;
   if (NOT(missing_out_LI_anti_dep(tree_out(node)) && 
	   NOT(get_subscript_ptr(node)->store)))
     if (index_in_outer_subscript(node,dep_info->index[0]))
       if (index_in_outer_subscript(node,dep_info->index[1]))
         dep_info->addr_coeff[0]++;
       else
         dep_info->addr_coeff[1]++;
     else
       if (index_in_outer_subscript(node,dep_info->index[1]))
         dep_info->addr_coeff[2]++;
  }

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
		dinfo->reg_coeff[0] += (dist * coeff0 * dinfo->step[0]) /
	                               (coeff2 * dinfo->step[2]);
	       }
	     else
	       dinfo->reg_coeff[0] += (coeff0 * dinfo->step[0]) /
	                              (coeff2 * dinfo->step[2]);
	    }
	  else 
	    dinfo->reg_coeff[3] += 1;
	if (dinfo->index[1] != NULL)
	  if (pt_find_var(sub1,dinfo->index[1]))
	    {
	     pt_get_coeff(sub1,dinfo->index[1],&lin,&coeff1);
	     pt_get_coeff(sub1,dinfo->index[2],&lin,&coeff2);
	     if ((dist = gen_get_dt_DIS(&dg[edge],dinfo->inner_level)) != 0)
	       {
		if (dist < 0) 
	          dist = -dist;
		dinfo->reg_coeff[0] += (dist * coeff1 * dinfo->step[1]) /
	                               (coeff2 * dinfo->step[2]);
	       }
	     else
	       dinfo->reg_coeff[0] += (coeff0 * dinfo->step[0]) /
	                              (coeff2 * dinfo->step[2]);
	    }
	  else 
	    dinfo->reg_coeff[3] += 1;
	dinfo->mem_coeff[3] += 1;
	dinfo->addr_coeff[3] += 1;
       }
     else 
       if (dinfo->index[0] != NULL)
         if (!pt_find_var(sub1,dinfo->index[0]))
           if (dinfo->index[1] != NULL)
             if (!pt_find_var(sub1,dinfo->index[1]))
               dinfo->reg_coeff[3] += 1;
	     else
               dinfo->reg_coeff[2] += 1;
	   else
	     dinfo->reg_coeff[2] += 1;
	 else
	   if (dinfo->index[1] != NULL)
             if (!pt_find_var(sub1,dinfo->index[1]))
	       dinfo->reg_coeff[1] += 1;
	     else
               dinfo->reg_coeff[0] += 1;
	   else
	     dinfo->reg_coeff[0] += 1;
       else 
         if (dinfo->index[1] != NULL)
           if (!pt_find_var(sub1,dinfo->index[1]))
	     dinfo->reg_coeff[1] += 1;
	   else
             dinfo->reg_coeff[0] += 1;
	 else
	   dinfo->reg_coeff[0] += 1;
  }
				     
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

static void compute_coefficients(dep_info_type *dep_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   UtilNode            *lnode;
   UtilList            *nlist;
   AST_INDEX           node,
                       prevnode;
   int                 dvect[MAXLOOP];
   subscript_info_type *sptr;
   int                 num_edges;

     for (lnode = UTIL_HEAD(dep_info->partition);
	  lnode != NULL;
	  lnode = UTIL_NEXT(lnode))
       {
	nlist = (UtilList *)UTIL_NODE_ATOM(lnode);
	sptr = get_subscript_ptr((AST_INDEX)UTIL_NODE_ATOM(UTIL_HEAD(nlist)));
	if (UTIL_HEAD(nlist) != UTIL_TAIL(nlist) ||
	    sptr->is_scalar[0] || sptr->is_scalar[1] | sptr->is_scalar[2])
	  {
	   num_edges = 0;
	   node = find_oldest_value(nlist,dep_info);
	   compute_mem_addr_coeffs(dep_info,node);
	   summarize_distance_vector(dvect,&num_edges,node,dep_info->ped,
				     nlist,dep_info);
	   compute_registers(dep_info,dvect,node,num_edges);
	   remove_nodes(node,dep_info->ped,dep_info->inner_level,nlist);
	   prevnode = node;
	   while (UTIL_HEAD(nlist) != UTIL_TAIL(nlist))
	     {
	      num_edges = 0;
	      node = find_oldest_value(nlist,dep_info);
	      summarize_distance_vector(dvect,&num_edges,node,dep_info->ped,
					nlist,dep_info);
	      compute_extra_regs(dep_info,dvect,node,prevnode,num_edges);
	      remove_nodes(node,dep_info->ped,dep_info->inner_level,nlist);
	      prevnode = node;
	     }
	  }
	else
	  {
	   node = (AST_INDEX)UTIL_NODE_ATOM(UTIL_HEAD(nlist));
	   if (node_is_consistent_MIV(node,dep_info))
	     {
	      get_subscript_ptr(node)->MIV = true;
	      compute_MIV_coefficients(node,dep_info);
	     }
	   else
	     compute_mem_addr_coeffs(dep_info,node);
	  }
	util_list_free(nlist);
       }
     util_list_free(dep_info->partition);
  }

int mh_increase_unroll(int   max,
		       int   denom,
		       float rhoL_lp)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   int x;

     x = (int)(rhoL_lp / denom);
     if (x * denom != rhoL_lp)
       x++;
     if (x <= max+1)
       return(x);
     else
       return(max+1);
  }

static void compute_two_loops(model_loop    *loop_data,
			      int           *unroll_vector,
			      int           *unroll_loops,
			      dep_info_type *dep_info,
			      float         rhoL_lp)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   int   x1,x2,max_x2,min_x1,temp;
   float min_obj,new_obj,abs_obj,regs;
   
     if ((loop_data[unroll_loops[0]].count == 0 &&
	  dep_info->scalar_coeff[2] == 0 && dep_info->x2 == 1 && 
	  loop_data[unroll_loops[1]].count == 0 &&
	  dep_info->scalar_coeff[1] == 0 && dep_info->x1 == 1) ||
	 (dep_info->reg_coeff[0] == 0 && dep_info->reg_coeff[1] == 0 &&
	  dep_info->reg_coeff[2] == 0 && dep_info->scalar_coeff[1] == 0 && 
	  dep_info->scalar_coeff[2] == 0))
       {
	dep_info->x1 = 1;
	dep_info->x2 = 1;
       }
     else
       {
	regs = mh_register_pressure(dep_info->reg_coeff,dep_info->scalar_coeff,
				    dep_info->x1,dep_info->x2) + 
				    dep_info->scalar_regs;
	if (regs < ((config_type *)PED_MH_CONFIG(dep_info->ped))->max_regs)
	  {	
	   x1 = mh_compute_x(((config_type *)PED_MH_CONFIG(dep_info->ped))
			     ->max_regs-dep_info->scalar_regs,
			     ((config_type *)PED_MH_CONFIG(dep_info->ped))
			     ->int_regs,
			     dep_info->reg_coeff,dep_info->scalar_coeff,
			     dep_info->addr_coeff,dep_info->x2,2);
	   max_x2 = mh_compute_x(((config_type *)PED_MH_CONFIG(dep_info->ped))
				 ->max_regs - dep_info->scalar_regs,
				 ((config_type *)PED_MH_CONFIG(dep_info->ped))
				 ->int_regs,
				 dep_info->reg_coeff,dep_info->scalar_coeff,
				 dep_info->addr_coeff,dep_info->x1,1);
	   if (x1 < dep_info->x1 || max_x2 < dep_info->x2)
	     {
	      dep_info->x1 = 1;
	      dep_info->x2 = 1;
	     }
	   else
	     {
	      min_x1 = dep_info->x1;
	      x2 = dep_info->x2;
	      min_obj = (float)MAXINT;
	      while(min_obj > 0.01 && x1 >= min_x1 && x2 <= max_x2)
		{
		 new_obj = mh_loop_balance(dep_info->mem_coeff,dep_info->flops,
					   x1,x2)
	               - ((config_type *)PED_MH_CONFIG(dep_info->ped))->beta_m;
		 if (new_obj < 0.0)
		   abs_obj = -new_obj;
		 else
	          abs_obj = new_obj;
		 if (abs_obj < min_obj)
		   {
		    min_obj = abs_obj;
		    dep_info->x1 = x1;
		    dep_info->x2 = x2;
		   }
		 if (new_obj > 0.01)
		   {
		    x2++;
		    temp = mh_compute_x(((config_type *)PED_MH_CONFIG(dep_info
								      ->ped))
					->max_regs - dep_info->scalar_regs,
					((config_type *)PED_MH_CONFIG(dep_info
								      ->ped))
					->int_regs,
					dep_info->reg_coeff,
					dep_info->scalar_coeff,
					dep_info->addr_coeff,x2,2);
		    if (temp < x1)
		      x1 = temp;
		   }
		 else if (new_obj < -0.01)
		 x1--;
		}
	     }
	  }
	else
	  {
	   dep_info->x1 = 1;
	   dep_info->x2 = 1;
	  }
       }

      /* sometimes loops that do not carry dependences will be considered
	 because the contain multiple inner loops at the same level, where
	 one  loop body has dependence carried at that level and another does
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
					       rhoL_lp);
	     if (rhoL_lp > dep_info->x1 * dep_info->x2 * dep_info->flops)
	       dep_info->x2 =mh_increase_unroll(loop_data[unroll_loops[1]].max,
						dep_info->x1 * dep_info->flops,
						rhoL_lp);
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
	    dep_info->x1 = mh_increase_unroll(loop_data[unroll_loops[0]].max,
					      dep_info->x2 * dep_info->flops,
					      rhoL_lp);
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
	    dep_info->x2 = mh_increase_unroll(loop_data[unroll_loops[1]].max,
					      dep_info->x1 * dep_info->flops,
					      rhoL_lp);
	  unroll_vector[loop_data[unroll_loops[1]].level-1] = dep_info->x2 - 1;
	 }
       else
	 {
	  unroll_vector[loop_data[unroll_loops[0]].level-1] = 
	                loop_data[unroll_loops[0]].max;
	  unroll_vector[loop_data[unroll_loops[1]].level-1] = 
	                loop_data[unroll_loops[1]].max;
	 }
  }
  
static void compute_one_loop(model_loop    *loop_data,
			     int           *unroll_vector,
			     int           unroll_loop,
			     dep_info_type *dep_info,
			     float           rhoL_lp)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   float min_obj,new_obj,abs_obj;
   int   x,min_x,max_x,regs;

     if ((loop_data[unroll_loop].count == 0  &&
	  dep_info->scalar_coeff[2] == 0 && dep_info->x2 == 1) ||
	 (dep_info->reg_coeff[0] == 0 && dep_info->reg_coeff[1] == 0 &&
	  dep_info->reg_coeff[2] == 0 && dep_info->scalar_coeff[2] == 0))
       dep_info->x1 = 1;
     else
       {
	regs = mh_register_pressure(dep_info->reg_coeff,dep_info->scalar_coeff,
				    dep_info->x1,dep_info->x2) + 
				    dep_info->scalar_regs;
	if (regs < ((config_type *)PED_MH_CONFIG(dep_info->ped))->max_regs)
	  {	
	   min_obj = (float)MAXINT;
	   min_x = dep_info->x1;
	   max_x = mh_compute_x(((config_type *)PED_MH_CONFIG(dep_info->ped))
				->max_regs - dep_info->scalar_regs,
				((config_type *)PED_MH_CONFIG(dep_info->ped))
				->int_regs,
				dep_info->reg_coeff,dep_info->scalar_coeff,
				dep_info->addr_coeff,dep_info->x2,2);
	   if (max_x < min_x)
	     dep_info->x1 = 1;
	   else
	     while(min_x <= max_x && min_obj > 0.01)
	       {
		x = (max_x + min_x) >> 1;
		new_obj = mh_loop_balance(dep_info->mem_coeff,dep_info->flops,
					  x,dep_info->x2) -
			 ((config_type *)PED_MH_CONFIG(dep_info->ped))->beta_m;
		if (new_obj < 0.0)
	          abs_obj = -new_obj;
		else
	          abs_obj = new_obj;
		if (abs_obj < min_obj)
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
	  dep_info->x1 = mh_increase_unroll(loop_data[unroll_loop].max,
					    dep_info->x2 * dep_info->flops,
					    rhoL_lp);
	unroll_vector[loop_data[unroll_loop].level-1]= dep_info->x1 - 1;
       }
     else
       unroll_vector[loop_data[unroll_loop].level-1] = 
                  loop_data[unroll_loop].max;
  }   
  
static void do_computation(model_loop    *loop_data,
			   int           loop,
			   int           *unroll_vector,
			   int           *unroll_loops,
			   int           count,
			   PedInfo       ped,
			   SymDescriptor symtab,
			   arena_type    *ar)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   int           i,regs;
   float         rhoL_lp,bal;
   dep_info_type dep_info;
   reg_info_type reg_info;
   AST_INDEX     step;

     dep_info.ar = ar;
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
	dep_info.reg_coeff[i] = 0;
	dep_info.mem_coeff[i] = 0;
	dep_info.addr_coeff[i] = 0;
	if (i < 3)
	  {
	   dep_info.scalar_coeff[i] = 0;
	  }
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
     walk_statements(loop_data[loop].node,loop_data[loop].level,count_regs,
		     NOFUNC,(Generic)&reg_info);
     if (((config_type *)PED_MH_CONFIG(ped))->chow_alloc && 
	 reg_info.expr_regs < 4)

	/* reserve at least 4 register for a Chow-style register allocator
	     for expressions because of high interference */

       dep_info.scalar_regs += 4;
     else
       dep_info.scalar_regs += reg_info.expr_regs;
     fst_KillField(symtab,FIRST);
     walk_expression(loop_data[loop].node,survey_edges,NOFUNC,
		     (Generic)&dep_info);
     dep_info.partition = util_list_alloc((Generic)NULL,(char *)NULL);
     walk_expression(loop_data[loop].node,partition_names,NOFUNC,
		     (Generic)&dep_info);
     compute_coefficients(&dep_info);
     if ((bal = mh_loop_balance(dep_info.mem_coeff,dep_info.flops,dep_info.x1,
			 dep_info.x2) -
	  ((config_type *)PED_MH_CONFIG(ped))->beta_m) <= 0.0)
       {
        if (bal < 0.0)
	  {
	   unroll_vector[loop_data[unroll_loops[0]].level-1] = 0;
	   if (count == 2)
	     unroll_vector[loop_data[unroll_loops[1]].level-1] = 0;
	  }
	else
	  {
	   unroll_vector[loop_data[unroll_loops[0]].level-1] = dep_info.x1-1;
	   if (count == 2)
	     unroll_vector[loop_data[unroll_loops[1]].level-1] = dep_info.x2-1;
	  }
       }
     else
       {
	rhoL_lp = loop_data[loop].rho * 
	     ((config_type *)PED_MH_CONFIG(ped))->pipe_length;
	if (count == 2)
	  compute_two_loops(loop_data,unroll_vector,unroll_loops,&dep_info,
			    rhoL_lp);
	else if (count == 1)
	  compute_one_loop(loop_data,unroll_vector,unroll_loops[0],&dep_info,
			   rhoL_lp);
	else if ((regs = dep_info.reg_coeff[0] + dep_info.scalar_coeff[0]) > 0)
	  loop_data[unroll_loops[0]].max = (((config_type *)PED_MH_CONFIG(ped))
	                                  ->max_regs - dep_info.scalar_regs) /
					  regs - 1;
       }
  }
      
static void compute_values(model_loop    *loop_data,
			   int           loop,
			   int           *unroll_vector,
			   int           *unroll_loops,
			   int           count,
			   PedInfo       ped,
			   SymDescriptor symtab,
			   arena_type    *ar)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

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
       else;
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
	      loop_data[i].unroll_vector= ar->arena_alloc_mem_clear(LOOP_ARENA,
								    MAXLOOP * 
								  sizeof(int));
	      unroll_vector = loop_data[i].unroll_vector;
	     }
	   
	  }
       }
  }
   

void mh_compute_unroll_amounts(model_loop    *loop_data,
			       int           size,
			       int           num_loops,
			       PedInfo       ped,
			       SymDescriptor symtab,
			       arena_type    *ar)
			       
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   int unroll_loops[2];

     pick_loops(loop_data,size,num_loops,ped,symtab,ar);
     loop_data[0].unroll_vector = ar->arena_alloc_mem_clear(LOOP_ARENA,
							    MAXLOOP * 
							    sizeof(int));
     compute_values(loop_data,0,loop_data[0].unroll_vector,unroll_loops,0,ped,
		    symtab,ar);
  }
