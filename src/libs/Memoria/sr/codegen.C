/* $Id: codegen.C,v 1.4 1992/10/03 15:48:37 rn Exp $ */
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
#include <sr.h>
#include <Arena.h>
#include <scalar.h>
#include <name.h>
#include <codegen.h>
#include <label.h>
#include <bound.h>
	
static int null_scratch(AST_INDEX node,
			int dummy)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   set_scratch_to_NULL(node);
   return(WALK_CONTINUE);
  }

static void insert_load(block_type *block,
			array_table_type table_entry,
			SymDescriptor symtab,
			arena_type *ar,
			Boolean    top)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   char       reg[20];
   AST_INDEX  array_ref,
              new_stmt,
              stmt,olist,glist,lnode,next_node,guard;

     sprintf(reg,"%s$%d$0",gen_get_text(gen_SUBSCRIPT_get_name(
				          table_entry.node)),table_entry.def);
     array_ref = tree_copy_with_type(table_entry.node);
     new_stmt = gen_ASSIGNMENT(AST_NIL,ut_gen_ident(symtab,reg,
			       gen_get_converted_type(table_entry.node)),
			       array_ref);
     fst_PutField(symtab,reg,NUM_REGS,table_entry.regs-1);
     if (NOT(top))
       if (block->last != (Generic)NULL)
         if (is_guard(block->last))
           if (list_prev(block->last) == AST_NIL)
	     list_insert_before(tree_out(block->last),new_stmt);
	   else
	     {
	      olist = list_head(block->last);
	      glist = list_create(AST_NIL);
	      for (lnode = block->last;
		   lnode != AST_NIL;
		   lnode = next_node)
		{
		 next_node = list_next(lnode);
		 (void) list_remove_node(lnode);
		 glist = list_insert_last(glist,lnode);
		}
	      stmt = gen_IF(AST_NIL,AST_NIL,glist);
	      guard = gen_GUARD(AST_NIL,AST_NIL,list_create(stmt));
	      list_insert_before(stmt,new_stmt);
	      olist = list_insert_last(olist,guard);
	     }
	 else
	   {
	    /* if (is_logical_if((stmt = tree_out(block->last))))
	      stmt = sr_change_logical_to_block_if(stmt); */
	    if (is_goto(block->last) || is_computed_goto(block->last) || 
	       is_arithmetic_if(block->last) || is_logical_if(block->last))
	      list_insert_before(block->last,new_stmt);
	    else
	      list_insert_after(block->last,new_stmt);
	   }
       else

         /* new block has been created on an edge */

         if (is_guard(block->first))
           gen_GUARD_put_stmt_LIST(block->first,list_create(new_stmt));
	 else if (is_continue(block->first))
           list_insert_after(block->first,new_stmt);
         else; /* code for other cases should be written when needed */
     else
       {
	/* if (is_logical_if((stmt = tree_out(block->first))))
	  stmt = sr_change_logical_to_block_if(stmt); */
	list_insert_before(block->first,new_stmt);
       }
     block->last = new_stmt;
     create_stmt_info_ptr(new_stmt,ar);
     get_stmt_info_ptr(new_stmt)->block = block;
     get_stmt_info_ptr(new_stmt)->generated = true;
  }

static void insert_statements(arena_type *ar,
			      block_type *entry,
			      array_table_type *array_table,
			      SymDescriptor symtab,
			      int *loads)
  
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   block_type *block,
              *new_block;
   int        gen,
              egen;
   edge_type  *edge,
              *next_edge;
   Boolean    top;

     for (block = entry;
	  block != NULL;
          block = block->next)
       {
	if (!ut_set_is_empty(block->Insert))
	  ut_forall(gen,block->Insert)
	    {
	     (*loads)++;
	     insert_load(block,array_table[gen],symtab,ar,false);
	    }
	edge = block->pred;
	while(edge != NULL)
	  {
	   next_edge = edge->next_pred;
	   if (!ut_set_is_empty(edge->Insert))
	     {
	      if (edge->to->pred->next_pred == NULL)

	         /* if to-block has only one incoming edge put load at the
		    top of the block */

		{
		 new_block = block;
		 top = true;
		}
	      else
		{
		 new_block = sr_insert_block_on_edge(ar,edge,symtab);
		 top = false;
		}
	      ut_forall(egen,edge->Insert)
		{
		 (*loads)++;
		 insert_load(new_block,array_table[egen],symtab,ar,top);
		}
	      /* free(edge); */
	     }
	   edge = next_edge;
	  }
       }
  }

static int calc_peel_amt(UtilList *glist)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   UtilNode         *node;
   scalar_info_type *gptr;
   int              peel_amt = 0;

     for (node = UTIL_HEAD(glist);
	  node != NULLNODE;
	  node = UTIL_NEXT(node))
       {
	gptr = get_scalar_info_ptr(((name_node_type *)UTIL_NODE_ATOM(node))
				   ->gen);
	if (!gptr->recurrence && !gptr->scalar)
	  if (gptr->num_regs-1 > peel_amt)
	    peel_amt = gptr->num_regs - 1;
	  else;
	else
	  if (gptr->num_regs > peel_amt)
	    peel_amt = gptr->num_regs;
       }
     return(peel_amt);
  }
	
static int check_def(AST_INDEX node,
		     code_info_type *code_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX        name,
                    new_stmt,
                    stmt,
                    array_ref;
   scalar_info_type *scalar_info;
   char             reg_name[20],
                    *s;
   int              dist;

     if (is_subscript(node))
       {
	name = gen_SUBSCRIPT_get_name(node);
        scalar_info = get_scalar_info_ptr(name);
	if (scalar_info->is_generator)
	  {
	   sprintf(reg_name,"%s$%d$%d",gen_get_text(name),scalar_info->def,
		   mod(-code_info->copy, scalar_info->num_regs));
	   if (!scalar_info->no_store)
	     if (!scalar_info->scalar || code_info->do_stmt != AST_NIL)
	       {
		array_ref = tree_copy_with_type(node);
		/* set_scratch_to_NULL(gen_SUBSCRIPT_get_name(array_ref)); */
		new_stmt = gen_ASSIGNMENT(AST_NIL,array_ref,
					  ut_gen_ident(code_info->symtab,
						       reg_name,
					    gen_get_converted_type(node)));
		if (scalar_info->scalar)
	          list_insert_after(code_info->do_stmt,new_stmt);
		else
		  {
		   stmt = tree_out(code_info->stmt);
		   if (is_logical_if(stmt))
		     {
		      /* stmt = sr_change_logical_to_block_if(stmt); */
		      code_info->stmt = list_first(gen_GUARD_get_stmt_LIST(
                                   list_first(gen_IF_get_guard_LIST(stmt))));
		     }
	          list_insert_after(code_info->stmt,new_stmt);
		  }
	       }
	     else if (scalar_info->scalar)
	       code_info->post_stores = true;
	   pt_tree_replace(node,ut_gen_ident(code_info->symtab,reg_name,
					     gen_get_converted_type(node)));
	  }
       }
     else if (is_identifier(node))
       if ((dist = fst_GetField(code_info->symtab,gen_get_text(node),
				NUM_REGS)) > 0)
         {
	  strcpy(reg_name,gen_get_text(node));
	  s = rindex(reg_name,'$');
	  s[1] = '\0';
	  sprintf(reg_name,"%s%d",reg_name,mod(-code_info->copy,dist+1));
	  pt_tree_replace(node,ut_gen_ident(code_info->symtab,reg_name,
					    gen_get_converted_type(node)));
	 }
     return(WALK_CONTINUE);
  }

static int check_use(AST_INDEX node,
		     code_info_type *code_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX        name,
                    array_ref,
                    olist,glist,lnode,guard,next_node,
                    stmt,
                    new_stmt;
   scalar_info_type *scalar_info;
   char             reg_name[20];
   int              reg_num;

     if (is_subscript(node))
       {
	name = gen_SUBSCRIPT_get_name(node);
        scalar_info = get_scalar_info_ptr(name);
	if (scalar_info->is_generator)
	  {
	   sprintf(reg_name,"%s$%d$%d",gen_get_text(name),scalar_info->def,
		   mod(-code_info->copy, scalar_info->num_regs));
	   tree_replace(node,ut_gen_ident(code_info->symtab,reg_name,
					  gen_get_converted_type(node)));
	   if (!scalar_info->scalar || code_info->load_scalar)
	    {
	     array_ref = tree_copy_with_type(node);
	     /* set_scratch_to_NULL(gen_SUBSCRIPT_get_name(array_ref)); */
	     new_stmt = gen_ASSIGNMENT(AST_NIL,
				       ut_gen_ident(code_info->symtab,reg_name,
						 gen_get_converted_type(node)),
				       array_ref);
	     if (is_guard(code_info->stmt))
	       if (list_prev(code_info->stmt) == AST_NIL)
	         code_info->stmt = tree_out(code_info->stmt);
	       else
		 {
		  olist = list_head(code_info->stmt);
		  glist = list_create(AST_NIL);
		  for (lnode = code_info->stmt;
		       lnode != AST_NIL;
		       lnode = next_node)
		    {
		     next_node = list_next(lnode);
		     (void) list_remove_node(lnode);
		     glist = list_insert_last(glist,lnode);
		    }
		  code_info->stmt = gen_IF(AST_NIL,AST_NIL,glist);
		  guard = gen_GUARD(AST_NIL,AST_NIL,
				    list_create(code_info->stmt));
		  olist = list_insert_last(olist,guard);
		 }
	     else
	       {
		if (is_logical_if((stmt=tree_out(code_info->stmt))))
		  {
		   /* stmt = sr_change_logical_to_block_if(stmt); */
		   code_info->stmt = list_first(gen_GUARD_get_stmt_LIST(
                                     list_first(gen_IF_get_guard_LIST(stmt))));
		  }
		if (gen_get_label(code_info->stmt) != AST_NIL)
		  {
		   gen_ASSIGNMENT_put_lbl_def(new_stmt,tree_copy_with_type(gen_get_label(
                                              code_info->stmt)));
		   pt_tree_replace(gen_get_label(code_info->stmt),AST_NIL);
		  }
	       }
	     list_insert_before(code_info->stmt,new_stmt);
	    }
	   tree_free(node);
	  }
	else if (scalar_info->generator != -1 && 
	    scalar_info->gen_distance <= code_info->iteration)
	  {
	   if ((scalar_info->recurrence || scalar_info->scalar) &&
	       scalar_info->gen_type != LIAV)
	     reg_num = 0;
	   else
	     reg_num = mod(scalar_info->gen_distance - code_info->copy,
			   scalar_info->num_regs); 
	   sprintf(reg_name,"%s$%d$%d",gen_get_text(name),scalar_info->def,
		   reg_num);
	   pt_tree_replace(node,ut_gen_ident(code_info->symtab,reg_name,
					     gen_get_converted_type(node)));
	  }
       }
     return(WALK_CONTINUE);
  }

static int replace_references(AST_INDEX stmt,
			      int level,
			      code_info_type *code_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   code_info->stmt = stmt;
   if (is_assignment(stmt))
     { 
      if (!get_stmt_info_ptr(stmt)->generated)
        walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),NOFUNC,check_use,
			(Generic)code_info);
      walk_expression(gen_ASSIGNMENT_get_lvalue(stmt),NOFUNC,check_def,
		      (Generic)code_info);
      return(WALK_FROM_OLD_NEXT);
     }
   else if (is_guard(stmt))
     walk_expression(gen_GUARD_get_rvalue(stmt),NOFUNC,check_use,
		     (Generic)code_info);
   else if (is_logical_if(stmt))
     walk_expression(gen_LOGICAL_IF_get_rvalue(stmt),NOFUNC,check_use,
		     (Generic)code_info);
   else if (is_arithmetic_if(stmt))
     walk_expression(gen_ARITHMETIC_IF_get_rvalue(stmt),NOFUNC,check_use,
		     (Generic)code_info);
   else if (is_write(stmt))
     walk_expression(gen_WRITE_get_data_vars_LIST(stmt),NOFUNC,check_use,
		     (Generic)code_info);
   else if (is_read_short(stmt))
     {
      walk_expression(gen_READ_SHORT_get_data_vars_LIST(stmt),NOFUNC,check_def,
		      (Generic)code_info);
      return(WALK_FROM_OLD_NEXT);
     }
   return(WALK_CONTINUE);
  }

static void insert_transfers(AST_INDEX stmt_list,
			     UtilList *glist,
			     int iteration,
			     SymDescriptor symtab)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX        tlist,newa;
   scalar_info_type *scalar_info;
   int              max,i;
   char             lval[20],rval[20];
   UtilNode         *lnode;
   name_node_type   *name_node;

     tlist = list_create(AST_NIL);
     for (lnode = UTIL_HEAD(glist);
	  lnode != NULLNODE;
	  lnode = UTIL_NEXT(lnode))
       {
	name_node = (name_node_type *)UTIL_NODE_ATOM(lnode);
	scalar_info = get_scalar_info_ptr(name_node->gen);
	if (iteration >= scalar_info->num_regs)
	  max = scalar_info->num_regs - 1;
	else
	  max = iteration;
	for (i = max; i > 0; i--)
	  {
	   sprintf(lval,"%s$%d$%d",gen_get_text(name_node->gen),
		   scalar_info->def,i);
	   sprintf(rval,"%s$%d$%d",gen_get_text(name_node->gen),
		   scalar_info->def,i-1);
	   newa = gen_ASSIGNMENT(AST_NIL,
				ut_gen_ident(symtab,lval,
			            gen_get_converted_type(name_node->gen)),
				ut_gen_ident(symtab,rval,
			            gen_get_converted_type(name_node->gen)));
	   tlist = list_insert_last(tlist,newa);
	  }
       }
     stmt_list = list_append(stmt_list,tlist);
  }

static void peel_iterations(AST_INDEX root,
			    code_info_type *code_info,
			    int level,
			    int peel_amt)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX node,cont,condition,upb,step,next_node,
             stmts,control,lwb,go_to,log_if,newa,iteration;
   char      *ivar,*target;
   int       i,step_v;

     stmts = gen_DO_get_stmt_LIST(root);
     control = gen_DO_get_control(root);
     ivar = gen_get_text(gen_INDUCTIVE_get_name(control));
     lwb = gen_INDUCTIVE_get_rvalue1(control);
     upb = gen_INDUCTIVE_get_rvalue2(control);
     step = gen_INDUCTIVE_get_rvalue3(control);
     if (step == AST_NIL)
       step = pt_gen_int(1);
     target = ut_symtab_get_label_str(code_info->symtab);
     cont = gen_CONTINUE(pt_gen_label_def(target));
     (void) list_insert_after(root,cont);
     code_info->do_stmt = AST_NIL;
     for (i = 0; i < peel_amt; i++)
       {
	newa = tree_copy_with_type(stmts);
	code_info->iteration = i;
	walk_statements(newa,level,replace_references,NOFUNC,
			(Generic)code_info);
	iteration = pt_simplify_expr(pt_gen_add(tree_copy_with_type(lwb),
						pt_gen_mul(pt_gen_int(i),
							   tree_copy_with_type(step))));
	pt_var_replace(newa,ivar,iteration);
	insert_transfers(newa,code_info->glist,i+1,code_info->symtab);
	ut_update_labels(newa,code_info->symtab);
	if (pt_eval(step,&step_v))
	  condition = gen_BINARY_GT(pt_gen_mul(pt_gen_sub(tree_copy_with_type(upb),
							 tree_copy_with_type(iteration)),
					       pt_gen_func1("sign",step)),
				    pt_gen_int(0));
	else
	  if (step_v > 0)
	    condition = gen_BINARY_GT(tree_copy_with_type(iteration),tree_copy_with_type(upb));
	  else
	    condition = gen_BINARY_LT(tree_copy_with_type(iteration),tree_copy_with_type(upb));
	go_to = gen_GOTO(AST_NIL,pt_gen_label_ref(target));
	log_if = gen_LOGICAL_IF(AST_NIL,condition,go_to);
	(void) list_insert_before(list_first(newa),log_if);
	/* walk_expression(newa,null_scratch,NOFUNC,NULL); */
	for (node = list_first(newa);
	     node != AST_NIL;
	     node = next_node)
	  {
	   next_node = list_next(node);
	   (void) list_insert_before(root,node);
	  }
	if (code_info->post_stores && i == 0 && peel_amt > 1)
	  {
	   code_info->target = ut_symtab_get_label_str(code_info->symtab); 
	   target = code_info->target;
	  }
	code_info->load_scalar = false;
       }
      pt_tree_replace(lwb,pt_simplify_expr(pt_gen_add(tree_copy_with_type(lwb),
					   pt_gen_mul(pt_gen_int(peel_amt),
						      tree_copy_with_type(step)))));
  }

static int create_pre_loop(AST_INDEX  root,
			   code_info_type *code_info,
			   int level)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   int            val,lwb_v,upb_v,step_v;
   AST_INDEX      control,lwb,upb,step,new_loop;
   Boolean        need_pre_loop = false;
   UtilNode       *lnode;
   name_node_type *name_node;

     lnode = UTIL_HEAD(code_info->glist);
     name_node = (name_node_type *)UTIL_NODE_ATOM(lnode);
     val = get_scalar_info_ptr(name_node->gen)->num_regs;
     for (lnode = UTIL_NEXT(lnode);
	  lnode != NULLNODE;
	  lnode = UTIL_NEXT(lnode))
       {
	name_node = (name_node_type *)UTIL_NODE_ATOM(lnode);
	val = lcm(val,get_scalar_info_ptr(name_node->gen)->num_regs);
       }
     if (--val == 0)
       return(0);
     control = gen_DO_get_control(root);
     lwb = gen_INDUCTIVE_get_rvalue1(control);
     if (pt_eval(lwb,&lwb_v))
       need_pre_loop = true;
     else
       {
	upb = gen_INDUCTIVE_get_rvalue2(control);
	if (pt_eval(upb,&upb_v))
	  need_pre_loop = true;
	else
	  {
	   step = gen_INDUCTIVE_get_rvalue3(control);
	   if (step == AST_NIL)
	     step_v = 1;
	   else if (pt_eval(step,&step_v))
	     need_pre_loop = true;
	   if (!need_pre_loop)
	     if (mod((upb_v - lwb_v + 1)/step_v,val + 1)
		 != 0)
	       need_pre_loop = true;
	  }
       }
     if (need_pre_loop)
       {
	new_loop = tree_copy_with_type(root);
	walk_statements(new_loop,level,replace_references,NOFUNC,
			(Generic)code_info);
	/* walk_expression(new_loop,null_scratch,NOFUNC,NULL); */
	insert_transfers(gen_DO_get_stmt_LIST(new_loop),code_info->glist,
			 code_info->iteration,code_info->symtab);
	ut_update_bounds(root,new_loop,val);
	ut_update_labels(new_loop,code_info->symtab);
	(void) list_insert_before(root,new_loop);
       }
     return(val);
  }

void sr_generate_code(AST_INDEX        root,
		      PedInfo          ped,
		      int              level,
		      flow_graph_type  flow_graph,
		      array_table_type *array_table,
		      SymDescriptor    symtab,
		      UtilList         *glist,
		      FILE             *logfile,
		      arena_type       *ar)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   code_info_type code_info;
   AST_INDEX      stmt_list,
                  new_list,
                  step,
                  new_body;
   int            i,unroll_amt,peel_amt,step_v;
   char           *ivar;
   int            loads = 0;

     fst_InitField(symtab,NUM_REGS,0,0);
     insert_statements(ar,flow_graph.entry,array_table,symtab,&loads);
     if (logfile != NULL)
       fprintf(logfile,"loads inserted = %d\n",loads);
     code_info.symtab = symtab;
     code_info.glist = glist;
     code_info.copy = 0;
     code_info.target = NULL;
     code_info.post_stores = false;
     code_info.load_scalar = true;
     if ((peel_amt = calc_peel_amt(glist)) > 0)
       peel_iterations(root,&code_info,level,peel_amt);
     new_list = list_create(AST_NIL);
     stmt_list = gen_DO_get_stmt_LIST(root);
     code_info.iteration = peel_amt; 
     code_info.load_scalar = false;
     step = gen_INDUCTIVE_get_rvalue3(gen_DO_get_control(root));
     if (step == AST_NIL)
       step_v = 1;
     else
       (void)pt_eval(step,&step_v);
     if ((unroll_amt = create_pre_loop(root,&code_info,level)) > 0)
       {
	ivar = gen_get_text(gen_INDUCTIVE_get_name(gen_DO_get_control(root)));
	for (i = 1; i < unroll_amt; i++)
	  {
	   new_body = tree_copy_with_type(stmt_list);
	   code_info.copy = i;
	   walk_statements(new_body,level,replace_references,NOFUNC,
			   (Generic)&code_info);
	   pt_var_add(new_body,ivar,i*step_v);
	   /* walk_expression(new_body,null_scratch,NOFUNC,NULL); */
	   ut_update_labels(new_body,symtab);
	   new_list = list_append(new_list,new_body);
	  }
	new_body = tree_copy_with_type(stmt_list);
	code_info.copy = unroll_amt;
	walk_statements(new_body,level,replace_references,NOFUNC,
			(Generic)&code_info);
	pt_var_add(new_body,ivar,unroll_amt*step_v);
	/* walk_expression(new_body,null_scratch,NOFUNC,NULL); */
	new_list = list_append(new_list,new_body);
	ut_update_labels(stmt_list,symtab);
       }
     code_info.copy = 0;
     code_info.do_stmt = root;
     walk_statements(stmt_list,level,replace_references,NOFUNC,
		     (Generic)&code_info);
     stmt_list = list_append(stmt_list,new_list);
     if (code_info.target != NULL)
      list_insert_after(root,gen_CONTINUE(pt_gen_label_def(code_info.target)));
     fst_KillField(symtab,NUM_REGS);
  }
