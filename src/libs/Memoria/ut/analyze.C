/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
#include <mh.h>
#include <analyze.h>
#include <gi.h>
#include <mem_util.h>

static int build_pre(AST_INDEX       stmt,
		     int             level,
		     build_info_type *build_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX upb,step,lval,lwb;
   int       loop_num,upb_v,step_v,lwb_v;

     if (is_do(stmt))
       {
	loop_num = get_stmt_info_ptr(stmt)->loop_num;
	build_info->loop_data[loop_num].inner_loop = -1;
	build_info->loop_data[loop_num].next_loop = -1;
	build_info->loop_data[loop_num].tri_loop = -1;
	build_info->loop_data[loop_num].trap_loop = -1;
	if (build_info->last_loop == 0)
	  if (build_info->parent != -1)
            build_info->loop_data[build_info->parent].inner_loop = loop_num;
	  else;
	else 
          build_info->loop_data[build_info->last_loop].next_loop = loop_num;
	build_info->loop_data[loop_num].parent = build_info->parent;  
	build_info->loop_data[loop_num].node = stmt;
	build_info->loop_data[loop_num].level = level;
	build_info->loop_data[loop_num].transform = true;
	build_info->loop_data[loop_num].reduction = false;
	build_info->loop_data[loop_num].interchange = true;
	build_info->loop_data[loop_num].distribute = true;
	build_info->loop_data[loop_num].rho = 0;
	build_info->loop_data[loop_num].stride = 0;
	build_info->loop_data[loop_num].scalar_array_refs = 0;
	build_info->loop_data[loop_num].ibalance= 0;
	build_info->loop_data[loop_num].fbalance= 0;
	build_info->loop_data[loop_num].registers = 0;
	build_info->loop_data[loop_num].type = RECT;
	build_info->loop_data[loop_num].inner_stmts = MAXLOOP;
	build_info->loop_data[loop_num].outer_stmts = -1;
	step = gen_INDUCTIVE_get_rvalue3(gen_DO_get_control(stmt));
	if (!pt_eval(step,&step_v) || step == AST_NIL)
	  {
	   upb = gen_INDUCTIVE_get_rvalue2(gen_DO_get_control(stmt));
	   lwb = gen_INDUCTIVE_get_rvalue1(gen_DO_get_control(stmt));
	   if (!pt_eval(upb,&upb_v) && !pt_eval(lwb,&lwb_v))
	     {
	      if (step == AST_NIL)
	        step_v = 1;
	      else
	        if (step_v < 0)
		  step_v = -step_v;
	      upb_v = (upb_v - lwb_v)/step_v;
	      if (upb_v < 0)
	        build_info->loop_data[loop_num].max = -upb_v;
	      else
	        build_info->loop_data[loop_num].max = upb_v;
	     }
	   else
	     build_info->loop_data[loop_num].max = 
	         ((config_type *)PED_MH_CONFIG(build_info->ped))->max_regs;
	  }
	else
	  build_info->loop_data[loop_num].max = 0;
	build_info->loop_data[loop_num].split_list = util_list_alloc(NULL,
							    "split-list");
	build_info->last_stack[++build_info->stack_top] = loop_num;
	build_info->last_loop = 0;
	build_info->parent = loop_num;
       }
     else
       {
	build_info->loop_data[build_info->last_stack[build_info->stack_top]].
                                                                stmts = true;
	if (is_assignment(stmt))
	  {
	   lval = gen_ASSIGNMENT_get_lvalue(stmt);
	   if (is_identifier(lval) && 
	       (gen_get_converted_type(lval) == TYPE_DOUBLE_PRECISION ||
	        gen_get_converted_type(lval) == TYPE_REAL))
	     build_info->loop_data[build_info->
			   last_stack[build_info->stack_top]].reduction
			   = pt_find_var(gen_ASSIGNMENT_get_rvalue(stmt),
					 gen_get_text(lval));
	  }
       }
     return(WALK_CONTINUE);
  }

static int build_post(AST_INDEX       stmt,
		      int             level,
		      build_info_type *build_info)
 
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
 
  {
   if (is_do(stmt))
     {
      build_info->last_loop = build_info->last_stack[build_info->stack_top--];
      build_info->parent = build_info->loop_data[build_info->last_loop].parent;
     }
   return(WALK_CONTINUE);
  }

static void check_uj_preventing(model_loop *loop_data,
				int        loop,
				DG_Edge    edge)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   Boolean less = false;
   int     dist,
           i;
   int     do1,do2;

     if (edge.dt_type == DT_UNKNOWN)
       {
	loop_data[loop].max = 0;
	loop_data[loop].interchange = false;
       }
     else if (edge.type != dg_input)
       {
	i = edge.level + 1;
	while(i <= gen_get_dt_LVL(&edge) && !less)
	  {
	   dist = gen_get_dt_DIS(&edge,i);
	   switch(dist) {
	     case DDATA_GE:
	     case DDATA_GT: break;

	     case DDATA_LT:
	     case DDATA_LE:
	     case DDATA_NE:  less = true;
	                     break;

	     case DDATA_ANY: if (edge.consistent != consistent_SIV || 
				 edge.symbolic)
	                       less = true;
	                     break;

	     case DDATA_ERROR: break;

	     default:
	       if (dist < 0)
	         less = true;
	    }
	   i++;
	  }
	if (less)
	  {
	   loop_data[loop].interchange = false;
	   dist = gen_get_dt_DIS(&edge,edge.level);
	   if (dist < DDATA_BASE)
             loop_data[loop].max = 0;
	   else
	     loop_data[loop].max = dist - 1;
	  }
       }
  }

static void check_backwards_dep(model_loop *loop,
				DG_Edge    *dg,
				int        edge,
				PedInfo    ped)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   stmt_info_type *sptr1,*sptr2;

      sptr1 = get_stmt_info_ptr(ut_get_stmt(dg[edge].src));
      sptr2 = get_stmt_info_ptr(ut_get_stmt(dg[edge].sink));
      if (sptr1 != NULL && sptr2 != NULL)
        if (sptr1->surrounding_do != sptr2->surrounding_do &&
	    sptr1->stmt_num > sptr2->stmt_num)
	  {
	   loop->max = 0;
	   loop->distribute = false;
	  }
	else;
      else
        dg_delete_free_edge( PED_DG(ped),edge);
  }

static int crossing_loop(model_loop *loop_data,
			 int        loop1,
			 int        loop2)

  {
     do
       {
	if (loop_data[loop1].level < loop_data[loop2].level)
	  loop2 = loop_data[loop2].parent;
	else if (loop_data[loop2].level < loop_data[loop1].level)
	  loop1 = loop_data[loop1].parent;
	else
	  {
	   loop1 = loop_data[loop1].parent;
	   loop2 = loop_data[loop2].parent;
	  }
       } while (loop1 != loop2);
     return(loop1);
  }


static void check_crossing_dep(model_loop *loop_data,
			       int        *stack,
			       DG_Edge    *dg,
			       int        edge,
			       PedInfo    ped,
			       SymDescriptor symtab)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   stmt_info_type *sptr1,*sptr2;
   int loop;

      sptr1 = get_stmt_info_ptr(ut_get_stmt(dg[edge].src));
      sptr2 = get_stmt_info_ptr(ut_get_stmt(dg[edge].sink));
      if (sptr1 != NULL && sptr2 != NULL)
        if (sptr1->surrounding_do != sptr2->surrounding_do)
	  {
	   if (dg[edge].type == dg_true && 
	       fst_GetField(symtab,gen_get_text(dg[edge].src),SYMTAB_NUM_DIMS)
	       == 0)
	     {
	      loop = crossing_loop(loop_data,sptr1->surrounding_do,
				   sptr2->surrounding_do);
	      if (fst_GetField(symtab,gen_get_text(dg[edge].src),EXPAND_LVL) <
		  loop_data[loop].level)
	        fst_PutField(symtab,gen_get_text(dg[edge].src),EXPAND_LVL,
			     loop_data[loop].level);
	      loop_data[loop].distribute = false;
	     }
	   dg_delete_free_edge( PED_DG(ped),edge);
	  }
	else;
      else
        dg_delete_free_edge( PED_DG(ped),edge);
  }

static int build_edge_pre(AST_INDEX       stmt,
			  int             level,
			  build_info_type *build_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
 
  {
   DG_Edge *dg;
   EDGE_INDEX edge,
              next_edge;
   int        vector,
              rec_num,
              thresh,
              max_thresh,
              lvl;
   float      rho_R;
   AST_INDEX  node;

   dg = dg_get_edge_structure( PED_DG(build_info->ped));
   vector = get_info(build_info->ped,stmt,type_levelv);
   for (lvl = build_info->level;
	lvl < level;
	lvl++)
     {
      rec_num = 0;
      max_thresh = 0;
      for (edge = dg_first_src_stmt( PED_DG(build_info->ped),vector,lvl);
	   edge != END_OF_LIST;
	   edge = next_edge)
	{
	 next_edge = dg_next_src_stmt( PED_DG(build_info->ped),edge);
	 if ((dg[edge].type == dg_true || dg[edge].type == dg_anti ||
	      dg[edge].type == dg_output) && 
	     fst_GetField(build_info->symtab,gen_get_text(dg[edge].src),
			  SYMTAB_NUM_DIMS) > 0)
	   {
	    check_uj_preventing(build_info->loop_data,
				build_info->last_stack[lvl],dg[edge]);
	    if (stmt == ut_get_stmt(dg[edge].sink) && dg[edge].type == dg_true)
	      {

	      /* this estimates the length and threshold of a single statement
		 recurrance.  Only single statement recurrances are used in
		 the computation of rho(L) */
	      
	       if (gen_is_dt_DIS(&dg[edge]))
	         thresh = gen_get_dt_DIS(&dg[edge],lvl);
	       else
	         thresh = 1;
	       rec_num = 1;
	       node = tree_out(tree_out(dg[edge].sink));
	       while(!stmt_containing_expr(tree_out(node)))
		 {
		  rec_num++;
		  node = tree_out(node);
		 }
	       rho_R = ((float) rec_num) / ((float) thresh);
	       if (build_info->loop_data[build_info->last_stack[lvl]].rho <
		   rho_R)
	         build_info->loop_data[build_info->last_stack[lvl]].rho =rho_R;
	      }
	    else if (dg[edge].src == dg[edge].sink && 
		     dg[edge].consistent == consistent_SIV &&
		     !dg[edge].symbolic && dg[edge].type == dg_output)
	      {
	       build_info->loop_data[build_info->last_stack[lvl]].
	                                        scalar_array_refs++;
	       build_info->loop_data[build_info->last_stack[lvl]].reduction =
	                                        true;
	      }
	    check_backwards_dep(&build_info->loop_data[build_info->
	                        last_stack[lvl]],dg,edge,build_info->ped);
	   }
	 else if (dg[edge].type == dg_inductive)
	   {
	    build_info->loop_data[build_info->last_stack[lvl]].max = 0;
	    build_info->loop_data[build_info->last_stack[lvl]].transform=false;
	   }
	}
     }
   for (edge = dg_first_src_stmt( PED_DG(build_info->ped),vector,
				 LOOP_INDEPENDENT);
	edge != END_OF_LIST;
	edge = next_edge)
     {
      next_edge = dg_next_src_stmt( PED_DG(build_info->ped),edge);
      if (dg[edge].type == dg_true || dg[edge].type == dg_anti ||
	  dg[edge].type == dg_input || dg[edge].type == dg_output)
        check_crossing_dep(build_info->loop_data,build_info->last_stack,
			   dg,edge,build_info->ped,build_info->symtab);
     }
   for (edge = dg_first_sink_stmt( PED_DG(build_info->ped),vector,
				  LOOP_INDEPENDENT);
	edge != END_OF_LIST;
	edge = next_edge)
     {
      next_edge = dg_next_sink_stmt( PED_DG(build_info->ped),edge);
      if (dg[edge].type == dg_true || dg[edge].type == dg_anti ||
	  dg[edge].type == dg_input || dg[edge].type == dg_output)
        check_crossing_dep(build_info->loop_data,build_info->last_stack,
			   dg,edge,build_info->ped,build_info->symtab);
     }
   if (is_do(stmt))
     {
      build_info->parent = get_stmt_info_ptr(stmt)->loop_num;
      build_info->last_stack[build_info->loop_data[build_info->parent].level] 
        = build_info->parent;
     }
   return(WALK_CONTINUE);
  }

static int build_edge_post(AST_INDEX       stmt,
			   int             level,
			   build_info_type *build_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
 
  {
   if (is_do(stmt))
     build_info->parent = build_info->loop_data[build_info->parent].parent;
   return(WALK_CONTINUE);
  }

static Boolean propogate_transform(model_loop *loop_data,
				   int        loop)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   Boolean can_transform;
   int     next;

     if (loop_data[loop].inner_loop != -1)
       loop_data[loop].transform = BOOL(loop_data[loop].transform &&
					propogate_transform(loop_data,
						  loop_data[loop].inner_loop));
     else
       loop_data[loop].max = 0;
     if (!loop_data[loop].transform)
       loop_data[loop].max = 0;
     can_transform = loop_data[loop].transform;
     for (next = loop_data[loop].next_loop;
	  next != -1;
	  next = loop_data[next].next_loop)
       {
       loop_data[loop].transform = BOOL(loop_data[loop].transform &&
				   propogate_transform(loop_data,next));
	if (!loop_data[loop].transform)
	  loop_data[loop].max = 0;
	can_transform = BOOL(can_transform && loop_data[next].transform);
       }
     return(can_transform);
  }

static void walk_loop(model_loop *loop_data,
		      int        loop,
		      int        outer_stmts,
		      int        *inner_stmts)

  {
   int next;

     loop_data[loop].outer_stmts = outer_stmts;
     if (loop_data[loop].inner_loop != -1)
       {
	if (loop_data[loop].stmts)
          outer_stmts = loop_data[loop].level;
	walk_loop(loop_data,loop_data[loop].inner_loop,outer_stmts,
		  inner_stmts);
       }
     else
       *inner_stmts = MAXLOOP;
     for (next = loop_data[loop].next_loop;
	  next != -1;
	  next = loop_data[next].next_loop)
       walk_loop(loop_data,next,outer_stmts,inner_stmts);
     loop_data[loop].inner_stmts = *inner_stmts;
     if (loop_data[loop].stmts && loop_data[loop].level < *inner_stmts &&
	 loop_data[loop].inner_loop != -1);
       *inner_stmts = loop_data[loop].level;
  }


void ut_analyze_loop(AST_INDEX  root,
		     model_loop *loop_data,
		     int        level,
		     PedInfo    ped,
		     SymDescriptor symtab)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   build_info_type build_info;
   AST_INDEX       upb;
   int             inner_stmt;

     build_info.parent = -1;
     build_info.last_loop = 0;
     build_info.stack_top = level-1;
     build_info.level = level;
     build_info.loop_data = loop_data;
     build_info.ped = ped;
     build_info.symtab = symtab;
     build_info.last_stack[0] = 0;
     loop_data[0].parent = -1;
     walk_statements(root,level,build_pre,build_post,(Generic)&build_info);
     walk_statements(root,level,build_edge_pre,build_edge_post,
		     (Generic)&build_info);
     if (loop_data[0].inner_loop != -1)
       loop_data[0].transform = propogate_transform(loop_data,
						    loop_data[0].inner_loop);
     else
       loop_data[0].transform = false;
     walk_loop(loop_data,0,-1,&inner_stmt);
  }
