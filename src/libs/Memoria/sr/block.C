/* $Id: block.C,v 1.5 1994/06/13 16:11:58 carr Exp $ */
/****************************************************************************/
/*  block.c                                                                 */
/*                                                                          */
/*  Description:  builds and manipulates the flow graph structure for an    */
/*                innermost loop body.  Currently handles block-if only.    */
/*                                                                          */
/****************************************************************************/

#include <general.h>

#include <sr.h>
#include <mh_ast.h>
#include <fort/walk.h>
#include <pt_util.h>

#ifndef block_h
#include <block.h>
#endif

#ifndef codegen_h
#include <codegen.h>
#endif

#ifndef label_h
#include <label.h>
#endif


static block_type *build_basic_blocks(block_type *block,
				      AST_INDEX stmt_list,
               			      AST_INDEX join_stmt,
				      int *block_num,
  				      arena_type *ar)	

/****************************************************************************/
/*                                                                          */
/*  Divides the AST into basic blocks.  Handles only block-if's             */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX  stmt,
              prev;
   Boolean    need_new_block = false;
   
     for (stmt = list_first(stmt_list);
	  stmt != AST_NIL;
	  stmt = list_next(stmt))
       {
	if (block->first == AST_NIL)
	  {
	   block->first = stmt;
	   block->join = join_stmt;
	   block->block_num = (*block_num)++;
	   block->num_preds = 0;
	   block->num_succs = 0;
	  }
	if (!is_comment(stmt) && gen_get_label(stmt) != AST_NIL)
	  {
	   prev = list_prev(stmt);
	   if (prev != AST_NIL)
	     {
	      block->last = prev;
	      block->join = stmt;
	      block->next = (block_type *)ar->arena_alloc_mem_clear(LOOP_ARENA,
							  sizeof(block_type));
	      block = block->next;
	      block->first = stmt;
	      block->join = join_stmt;
	      block->block_num = (*block_num)++;
	      block->num_preds = 0;
	      block->num_succs = 0;
	     }
	  }
	get_stmt_info_ptr(stmt)->block = block;
        if (is_guard(stmt))  /* guards are really stmts in AST */
	  {
	   if (gen_GUARD_get_rvalue(stmt) != AST_NIL)
	     {
	      block->last = stmt;
	      block->next = (block_type *)ar->arena_alloc_mem_clear(LOOP_ARENA,
							  sizeof(block_type));
	      block = build_basic_blocks(block->next,
					 gen_GUARD_get_stmt_LIST(stmt),
					 join_stmt,block_num,ar);
	     }
	   else
	    block = build_basic_blocks(block,gen_GUARD_get_stmt_LIST(stmt),
				       join_stmt,block_num,ar);
	   need_new_block = true;
	  }
	else if (is_if(stmt))  /* block ifs are not really stmts in the AST */
	  {
	   if (list_next(stmt) != AST_NIL)
	     {
	      block->join = list_next(stmt);
	      block = build_basic_blocks(block,gen_IF_get_guard_LIST(stmt),
					 list_next(stmt),block_num,ar);
	     }
	   else
	     block = build_basic_blocks(block,gen_IF_get_guard_LIST(stmt),
					join_stmt,block_num,ar);
	   need_new_block = true;
	  }
	else if (is_logical_if(stmt))
	  {
	   block->last = stmt;
	   block->next = (block_type *)ar->arena_alloc_mem_clear(LOOP_ARENA,
							   sizeof(block_type));
	   if (list_next(stmt) != AST_NIL)
	     {
	      block->join = list_next(stmt);
	      block = build_basic_blocks(block->next,
					 gen_LOGICAL_IF_get_stmt_LIST(stmt),
					 block->join,block_num,ar);
	     }
	   else
	     block = build_basic_blocks(block->next,
					gen_LOGICAL_IF_get_stmt_LIST(stmt),
					join_stmt,block_num,ar);
	   need_new_block = true;
	  }
	else if (is_goto(stmt) || is_arithmetic_if(stmt) ||
		 is_computed_goto(stmt))
	  {
	   block->last = stmt;
	   need_new_block = true;
	  } 
	else if (list_next(stmt) == AST_NIL)
	  block->last = stmt;
	if (need_new_block)
	  {
	   if (list_next(stmt) != AST_NIL)
	     {
	      block->next = (block_type *)ar->arena_alloc_mem_clear(LOOP_ARENA,
							   sizeof(block_type));
	      block = block->next;
	     }
	   need_new_block = false;
	  }
       }
     return(block);
  }

static void make_block_edge(block_type *block1,
			    block_type *block2,
			    AST_INDEX label,
			    arena_type *ar)

/****************************************************************************/
/*                                                                          */
/*  Create a data flow edge from block1 to block2.                          */
/*                                                                          */
/****************************************************************************/

  {
   edge_type *new_edge;

     new_edge = (edge_type *)ar->arena_alloc_mem(LOOP_ARENA,sizeof(edge_type));
     new_edge->Insert = NULL;
     new_edge->probability = 0.0;
     new_edge->lbl_ref = label;
     new_edge->from = block1;
     new_edge->to = block2;
     new_edge->next_pred = block2->pred;
     if (new_edge->next_pred != NULL)
       new_edge->next_pred->prev_pred = new_edge;
     new_edge->prev_pred = NULL;
     block2->pred = new_edge;
     block2->num_preds++;
     new_edge->next_succ = block1->succ;
     if (new_edge->next_succ != NULL)
       new_edge->next_succ->prev_succ = new_edge;
     new_edge->prev_succ = NULL;
     block1->succ = new_edge;
     block1->num_succs++;
  }

static void link_basic_blocks(flow_graph_type *flow_graph,
			      SymDescriptor symtab,
  			      arena_type *ar)

/****************************************************************************/
/*                                                                          */
/*  Construct the data flow graph from the list of basic blocks.            */
/*                                                                          */
/****************************************************************************/

  {
   block_type *block;
   AST_INDEX  guard,
              stmt,
              label;

     for (block = flow_graph->entry;
	  block != NULL;
	  block = block->next)
       if (is_guard(block->last))
	 {
          stmt = gen_GUARD_get_stmt_LIST(block->last);
	  make_block_edge(block,get_stmt_info_ptr(list_first(stmt))->block,
			  AST_NIL,ar);
          guard = list_next(block->last);
	  if (guard != AST_NIL)
	    make_block_edge(block,get_stmt_info_ptr(guard)->block,AST_NIL,ar);
	  else if (block->join != AST_NIL)
	    make_block_edge(block,get_stmt_info_ptr(block->join)->block,
			    AST_NIL,ar);
	  else
	    make_block_edge(block,flow_graph->exit,AST_NIL,ar);
	 }
       else if (is_logical_if(block->last))
	 {
          make_block_edge(block,get_stmt_info_ptr(list_first(
                          gen_LOGICAL_IF_get_stmt_LIST(block->last)))->block,
			  AST_NIL,ar);
	  if ((stmt = list_next(block->last)) != AST_NIL)
	    make_block_edge(block,get_stmt_info_ptr(stmt)->block,AST_NIL,ar);
	  else if (block->join != AST_NIL)
	    make_block_edge(block,get_stmt_info_ptr(block->join)->block,
			    AST_NIL,ar);
	  else
	    make_block_edge(block,flow_graph->exit,AST_NIL,ar);
	 }
       else if (is_goto(block->last))
         make_block_edge(block,get_stmt_info_ptr((AST_INDEX)
                         fst_GetFieldByIndex(symtab,get_label_sym_index(
                                             gen_GOTO_get_lbl_ref(block->last))
					     ,LBL_STMT))->block,AST_NIL,ar);
       else if (is_arithmetic_if(block->last))
	 {
	  make_block_edge(block,get_stmt_info_ptr((AST_INDEX)
                          fst_GetFieldByIndex(symtab,get_label_sym_index(
                                             gen_ARITHMETIC_IF_get_lbl_ref1(
                                             block->last)),LBL_STMT))->block,
			  AST_NIL,ar);
	  make_block_edge(block,get_stmt_info_ptr((AST_INDEX)
                          fst_GetFieldByIndex(symtab,get_label_sym_index(
                                             gen_ARITHMETIC_IF_get_lbl_ref2(
                                             block->last)),LBL_STMT))->block,
			  AST_NIL,ar);
          make_block_edge(block,get_stmt_info_ptr((AST_INDEX)
                          fst_GetFieldByIndex(symtab,get_label_sym_index(
                                             gen_ARITHMETIC_IF_get_lbl_ref2(
                                             block->last)),LBL_STMT))->block,
			  AST_NIL,ar);
	 }
       else if (is_computed_goto(block->last))
	 {
          for (label = list_first(gen_COMPUTED_GOTO_get_lbl_ref_LIST(
                                  block->last));
	       label != AST_NIL;
	       label = list_next(label))
	    make_block_edge(block,get_stmt_info_ptr((AST_INDEX)
                            fst_GetFieldByIndex(symtab,get_label_sym_index(
						label),LBL_STMT))->block,
			    label,ar);
	  if (list_next(stmt) != AST_NIL)
	    make_block_edge(block,get_stmt_info_ptr(list_next(stmt))->block,
			    AST_NIL,ar);
	  else
	    if (block->join != AST_NIL)
	      make_block_edge(block,get_stmt_info_ptr(block->join)->block,
			      AST_NIL,ar);
	    else
	      make_block_edge(block,flow_graph->exit,AST_NIL,ar);
	 }
       else
         if (block->join != AST_NIL)
	   make_block_edge(block,get_stmt_info_ptr(block->join)->block,
			   AST_NIL,ar);
         else
	   make_block_edge(block,flow_graph->exit,AST_NIL,ar);
  }


void sr_build_flow_graph(flow_graph_type *flow_graph,
			 AST_INDEX stmt_list,
  			 SymDescriptor symtab,
			 arena_type *ar)

/****************************************************************************/
/*                                                                          */
/*  Build the data flow graph for stmt_list.                                */
/*                                                                          */
/****************************************************************************/

  {
   block_type  *block;
   int         block_num = 1;
   UtilList    *ulist;
   
     flow_graph->entry = (block_type *)ar->arena_alloc_mem_clear(LOOP_ARENA,
							   sizeof(block_type));
     flow_graph->entry->num_preds = 0;
     flow_graph->entry->num_succs = 0;
     flow_graph->exit = (block_type *)ar->arena_alloc_mem_clear(LOOP_ARENA,
							   sizeof(block_type));
     flow_graph->exit->num_preds = 0;
     flow_graph->exit->num_succs = 0;
     block = build_basic_blocks(flow_graph->entry,stmt_list,AST_NIL,
				&block_num,ar);

   /* # BASIC BLOCKS IS CONTAINED IN block_num */

     link_basic_blocks(flow_graph,symtab,ar);
     block->next = flow_graph->exit;
     make_block_edge(flow_graph->exit,flow_graph->entry,AST_NIL,ar);
  }


static void remove_edge_links(edge_type *edge)

  {
   if (edge->prev_pred != NULL)
     edge->prev_pred->next_pred = edge->next_pred;
   if (edge->prev_succ != NULL)
     edge->prev_succ->next_succ = edge->next_succ;
   if (edge->next_pred != NULL)
     edge->next_pred->prev_pred = edge->prev_pred;
   if (edge->next_succ != NULL)
     edge->next_succ->prev_succ = edge->prev_succ;
   if (edge->from->succ == edge)
     edge->from->succ = edge->next_succ;
   if (edge->from->pred == edge)
     edge->from->pred = edge->next_pred;
   if (edge->to->succ == edge)
     edge->to->succ = edge->next_succ;
   if (edge->to->pred == edge)
     edge->to->pred = edge->next_pred;
   edge->from->num_succs--;
   edge->to->num_preds--;
   edge->prev_pred = NULL;
   edge->prev_succ = NULL;
   edge->next_pred = NULL;
   edge->next_succ = NULL;
  }

block_type *sr_insert_block_on_edge(arena_type *ar,
				    edge_type *edge,
  				    SymDescriptor symtab)

/****************************************************************************/
/*                                                                          */
/*  Insert a new basic block on a flow graph edge.                          */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX  guard,
              cont,
              label,
              prev;
   int        index;
   block_type *new_block;
   char       *lstr;

     remove_edge_links(edge);
     new_block = (block_type *)ar->arena_alloc_mem_clear(LOOP_ARENA,
						     sizeof(block_type));
     make_block_edge(edge->from,new_block,AST_NIL,ar);
     make_block_edge(new_block,edge->to,AST_NIL,ar);
     if (is_guard(edge->from->last))
       if (list_next(edge->from->last) == AST_NIL)
	 {
	  guard = gen_GUARD(AST_NIL,AST_NIL,AST_NIL);
	  list_insert_after(edge->from->last,guard);
	  new_block->first = guard;
	 }
       else
	 {
	  cont = gen_CONTINUE(AST_NIL);
	  list_insert_before(edge->to->first,cont);
	  new_block->first = cont;
	  if ((label = gen_get_label(edge->to->first)) != AST_NIL)
	    if ((int)fst_GetFieldByIndex(symtab,get_label_sym_index(label),
					 REFS) > 1)
	      {
	       list_insert_after(cont,gen_GOTO(AST_NIL,gen_get_text(label)));
	       new_block->last = list_next(cont);
	      }
	 }
     else if (is_logical_if(edge->from->last))
       {
	/* sr_change_logical_to_block_if(edge->from->last); */
	list_insert_after(edge->from->last,gen_GUARD(AST_NIL,AST_NIL,AST_NIL));
	new_block->first = list_next(edge->from->last);
       }
     else if (is_arithmetic_if(edge->from->last) || 
	      is_computed_goto(edge->from->last))
       {
	lstr = ut_symtab_get_label_str(symtab);
	label = pt_gen_label_def(lstr);
	cont = gen_CONTINUE(label);
	new_block->first = cont;
	if (is_arithmetic_if(edge->from->last))
	  {
	   index = get_label_sym_index(gen_get_label(edge->to->first));
	   if (index == get_label_sym_index(gen_ARITHMETIC_IF_get_lbl_ref1(
                                            edge->from->last)))
	     pt_tree_replace(gen_ARITHMETIC_IF_get_lbl_ref1(edge->from->last),
			     pt_gen_label_ref(lstr));
	   else if (index==get_label_sym_index(gen_ARITHMETIC_IF_get_lbl_ref2(
                                               edge->from->last)))
	     pt_tree_replace(gen_ARITHMETIC_IF_get_lbl_ref2(edge->from->last),
			     pt_gen_label_ref(lstr));
	   else
	     pt_tree_replace(gen_ARITHMETIC_IF_get_lbl_ref3(edge->from->last),
			     pt_gen_label_ref(lstr));
	   label = gen_get_label(edge->to->first);
	  }
	else if (edge->lbl_ref != AST_NIL)
	  {
	   pt_tree_replace(edge->lbl_ref,pt_gen_label_ref(lstr));
	   label = gen_get_label(edge->to->first);
	  }
	else
	  label = AST_NIL;
	if ((prev = list_prev(edge->to->first)) != edge->from->last)
	  if (!is_goto(prev))
	    list_insert_after(prev,gen_GOTO(AST_NIL,
				pt_gen_label_ref(gen_get_text(label))));
	list_insert_before(edge->to->first,cont);
	if (label != AST_NIL)
	  if ((int)fst_GetFieldByIndex(symtab,get_label_sym_index(label),REFS)
	      > 1)
	    {
	     list_insert_after(cont,gen_GOTO(AST_NIL,
				 pt_gen_label_ref(gen_get_text(label))));
	     new_block->last = list_next(cont);
	    }
       }
     else;
     return(new_block);
  }

   
/* void sr_free_flow_graph(flow_graph_type flow_graph) */
/****************************************************************************/
/*                                                                          */
/*  Free up the memory used for the flow graph.                             */
/*                                                                          */
/****************************************************************************/

/*  {
   block_type *block,
              *blockt;
   edge_type  *edge,
              *edget;
   
     for (block = flow_graph.entry;
          block != NULL;
	  block = blockt)
       {
        free(block->gen);
        free(block->LC_gen);
        free(block->kill);
	free(block->LI_avail_in);
	free(block->LI_avail_out);
	free(block->LI_rgen_in);
	free(block->LI_rgen_out);
	free(block->LI_pavail_in);
	free(block->LI_pavail_out);
	free(block->LI_antic_in);
	free(block->LI_antic_out);
	free(block->LC_avail_in);
	free(block->LC_avail_out);
	free(block->LC_rgen_in);
	free(block->LC_rgen_out);
	free(block->LC_pavail_in);
	free(block->LC_pavail_out);
	free(block->LC_avail_in_if_1);
	free(block->LC_avail_out_if_1);
	free(block->LC_rgen_in_if_1);
	free(block->LC_rgen_out_if_1);
	free(block->LC_pavail_in_if_1);
	free(block->LC_pavail_out_if_1);
	free(block->LC_antic_in);
	free(block->LC_antic_out);
	free(block->Insert);
	for (edge = block->pred;
	     edge != NULL;
	     edge = edget)
	  {
	   edget = edge->next_pred;
	   free(edge->Insert);
	   free(edge);
	  }
	blockt = block->next;
	free(block);
       }
  }
*/
	  

void debug_print_graph(flow_graph_type flow_graph)
/****************************************************************************/
/*                                                                          */
/*  Print out the flow graph.                                               */
/*                                                                          */
/****************************************************************************/

  {
   block_type *block;
   AST_INDEX  stmt;
   edge_type  *edge;

     for (block = flow_graph.entry;
	  block != NULL;
	  block = block->next)
       {
        printf("block # = %d\n",block->block_num);
	printf("stmts = ");
	stmt = block->first;
	while (stmt != block->last)
	  {
	   printf("%d ",get_stmt_info_ptr(stmt)->stmt_num);
	   if is_if(stmt)
	     stmt = list_first(gen_IF_get_guard_LIST(stmt));
	   else if is_guard(stmt)
	     stmt = list_first(gen_GUARD_get_stmt_LIST(stmt));
	   else
	     stmt = list_next(stmt);
	  }
        if (block->last != (Generic)NULL)
	  printf("%d\n",get_stmt_info_ptr(block->last)->stmt_num);
	printf("preds = ");
        for (edge = block->pred;
	     edge != NULL;
	     edge = edge->next_pred)
	  printf("%d ",edge->from->block_num);
        printf("\n");
	printf("succs = ");
        for (edge = block->succ;
	     edge != NULL;
	     edge = edge->next_succ)
	  printf("%d ",edge->to->block_num);
        printf("\n\n");
       }
  }
