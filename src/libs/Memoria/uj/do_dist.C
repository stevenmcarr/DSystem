/* $Id: do_dist.C,v 1.4 1992/12/11 11:23:20 carr Exp $ */
#include <general.h>
#include <mh.h>
#include <mh_ast.h>
#include <fort/walk.h>
#include <do_dist.h>

#ifndef dt_h
#include <dt.h>
#endif 

static Boolean need_distribution(int *vec1,
				 int *vec2,
				 int init_level,
				 int max_level,
				 int *level)
  {
   int i;

     for (i = init_level-1; i < max_level; i++)
       if (vec1[i] != vec2[i])
         {
	  *level = i+1;
	  return(true);
	 }
     return(false);
  }

static void distribute_loop(model_loop    *loop_data,
			    int           loop,
			    int           *num_loops)
  {
   AST_INDEX stmt,
             stmt_list[MAXLOOP],
             next_stmt,
             new_do,prev_do,do_template;
   int       i,j,prev,temp,next_inner;

     i = 0;
     stmt = list_first(gen_DO_get_stmt_LIST(loop_data[loop].node));
     while (stmt != AST_NIL)
       {
	stmt_list[i] = list_create(AST_NIL);
	while (!is_do(stmt) && stmt != AST_NIL)
	  {
	   next_stmt = list_next(stmt);
	   list_remove_node(stmt);
	   stmt_list[i] = list_insert_last(stmt_list[i],stmt);
	   stmt = next_stmt;
	  } 
	if (is_do(stmt))
	  {
	   if (!list_empty(stmt_list[i]))
	     {
	      i++;
	      stmt_list[i] = list_create(AST_NIL);
	     }
	   next_stmt = list_next(stmt);
	   list_remove_node(stmt);
	   stmt_list[i] = list_insert_last(stmt_list[i],stmt);
	   stmt = next_stmt;
	  }
	i++;
       }
     tree_free(gen_DO_get_stmt_LIST(loop_data[loop].node));
     do_template = tree_copy_with_type(loop_data[loop].node);
     prev = loop;
     prev_do = loop_data[loop].node;
     gen_DO_put_stmt_LIST(loop_data[loop].node,stmt_list[0]);
     next_inner = loop_data[loop_data[loop].inner_loop].next_loop;
     loop_data[loop_data[loop].inner_loop].next_loop = -1;
     for (j = 1; j < i; j++)
       {
	new_do = tree_copy_with_type(do_template);
	gen_DO_put_stmt_LIST(new_do,stmt_list[j]);
	list_insert_after(prev_do,new_do);
        loop_data[*num_loops] = loop_data[loop];
	loop_data[*num_loops].next_loop = -1;
	loop_data[prev].next_loop = *num_loops;
	loop_data[*num_loops].node = new_do;
	loop_data[*num_loops].inner_loop = next_inner;
	loop_data[*num_loops].unroll_vector = 
	                       loop_data[next_inner].unroll_vector;
	loop_data[next_inner].unroll_vector = NULL;
	temp = loop_data[next_inner].next_loop;
	loop_data[next_inner].next_loop = -1;
	next_inner = temp;
	prev = (*num_loops)++;
	prev_do = new_do;
       }
    tree_free(do_template);
  }

static void do_distribution(model_loop *loop_data,
			    int        loop,
			    int        max_loop,
			    int        *num_loops)
  {
   int i;

     for (i = loop_data[loop].parent;
	  i != loop_data[max_loop].parent;
	  i = loop_data[i].parent)
       distribute_loop(loop_data,i,num_loops);
  }

static void assign_unroll_amounts(model_loop *loop_data,
				  int        init_level,
				  int        level,
				  int        *loop_stack,
				  int        *unroll_vector)

  {
   int i;
   
     for (i = init_level-1; i < level; i++)
       loop_data[loop_stack[i]].val = unroll_vector[i];
  }

static void walk_loops(model_loop *loop_data,
		       int        loop,
		       int        *unroll_vector,
		       int        *num_loops,
		       int        *loop_stack,
		       int        init_level)
  {
   int next,level;

   loop_stack[loop_data[loop].level-1] = loop;
   if (loop_data[loop].next_loop != -1)
     if (need_distribution(unroll_vector,loop_data[loop_data[loop].next_loop].
			                 unroll_vector,init_level,
			   loop_data[loop].level,&level))
       do_distribution(loop_data,loop,loop_stack[level-1],num_loops);
   if (loop_data[loop].inner_loop != -1)
     walk_loops(loop_data,loop_data[loop].inner_loop,unroll_vector,num_loops,
		loop_stack,init_level);
   else
     assign_unroll_amounts(loop_data,init_level,loop_data[loop].level,
			   loop_stack,unroll_vector);
   for (next = loop_data[loop].next_loop;
	next != -1;
	next = loop_data[next].next_loop)
     walk_loops(loop_data,next,loop_data[next].unroll_vector,num_loops,
		loop_stack,init_level);
  }

void mh_do_distribution(model_loop *loop_data,
			int        *num_loops)
  {
   int loop_stack[MAXLOOP];

     walk_loops(loop_data,0,loop_data[0].unroll_vector,num_loops,loop_stack,
		loop_data[0].level);
  }
