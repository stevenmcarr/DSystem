/* $Id: table.C,v 1.8 1994/07/20 11:32:55 carr Exp $ */
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
#include <stdlib.h>
#include <general.h>
#include <sr.h>
#include <mh_ast.h>
#include <fort/walk.h>
#include <table.h>

#ifndef Arena_h
#include <misc/Arena.h>
#endif

#ifndef scalar_h
#include <scalar.h>
#endif

#include <mem_util.h>
#include <pt_util.h>

static int get_value(AST_INDEX     node,
		     int           *index)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   if (is_identifier(node))
     *index = *index + atoi(gen_get_text(node));
   else if (is_constant(node))
     *index = *index + atoi(gen_get_text(node));
   else if (is_binary_plus(node))
     *index = *index + GEN_BINARY_PLUS;
   else if (is_binary_minus(node))
     *index = *index + GEN_BINARY_MINUS;
   else if (is_binary_times(node))
     *index = *index + GEN_BINARY_TIMES;
   else if (is_binary_divide(node))
     *index = *index + GEN_BINARY_DIVIDE;
   return(WALK_CONTINUE);
  }
     

static int array_hash(array_table_type *array_table,
		      AST_INDEX        node,
		      int              size)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/


  {
   int index;

     index = atoi(gen_get_text(gen_SUBSCRIPT_get_name(node)));
     walk_expression(gen_SUBSCRIPT_get_rvalue_LIST(node),
		     (WK_EXPR_CLBACK)get_value,NOFUNC,(Generic)&index);
     index = index % size;
     while (!is_null_node(array_table[index].node) && 
            !pt_expr_equal(node,array_table[index].node))
       index = (index + 1) % size;
     return(index);
  }


static int add_elements(AST_INDEX         node,
			prelim_info_type  *prelim_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   scalar_info_type *scalar_info;

     if (is_subscript(node))
       {
	scalar_info = get_scalar_info_ptr(gen_SUBSCRIPT_get_name(node));
	scalar_info->table_index = array_hash(prelim_info->array_table,node,
					      prelim_info->array_refs);
	if (prelim_info->array_table[scalar_info->table_index].node == 
	    (Generic)NULL)
	  {
	   prelim_info->array_table[scalar_info->table_index].node = 
	                                                    tree_copy_with_type(node);
	   prelim_info->array_table[scalar_info->table_index].def =
	                           prelim_info->def_num++;
	   prelim_info->array_table[scalar_info->table_index].profit = 0.0;
	  }
	scalar_info->def = prelim_info->array_table[scalar_info->
						    table_index].def;
       }
     return(WALK_CONTINUE);
  }
    
int sr_build_table(AST_INDEX         stmt,
		   int               level,
		   Generic           prelim_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   if (is_assignment(stmt))
     {
      walk_expression(gen_ASSIGNMENT_get_lvalue(stmt),NOFUNC,
		      (WK_EXPR_CLBACK)add_elements,
		      prelim_info);
      walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),NOFUNC,
		      (WK_EXPR_CLBACK)add_elements,
		      prelim_info);
     }
   else if (is_guard(stmt))
     walk_expression(gen_GUARD_get_rvalue(stmt),NOFUNC,
		     (WK_EXPR_CLBACK)add_elements,
		     prelim_info);
   else if (is_logical_if(stmt))
     walk_expression(gen_LOGICAL_IF_get_rvalue(stmt),NOFUNC,
		     (WK_EXPR_CLBACK)add_elements,
		     prelim_info);
   else if (is_write(stmt))
     walk_expression(gen_WRITE_get_data_vars_LIST(stmt),NOFUNC,
		     (WK_EXPR_CLBACK)add_elements,
		     prelim_info);
   else if (is_read_short(stmt))
     walk_expression(gen_READ_SHORT_get_data_vars_LIST(stmt),NOFUNC,
		     (WK_EXPR_CLBACK)add_elements,prelim_info);
   else if (is_arithmetic_if(stmt))
     walk_expression(gen_ARITHMETIC_IF_get_rvalue(stmt),NOFUNC,
		     (WK_EXPR_CLBACK)add_elements,
		     prelim_info);
   else if (is_call(stmt))
     walk_expression(gen_CALL_get_invocation(stmt),NOFUNC,
		     (WK_EXPR_CLBACK)add_elements,
		     prelim_info);
   return(WALK_CONTINUE);
  }

void debug_print_table(prelim_info_type *prelim_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   int i;

     for (i = 0;
	  i < prelim_info->array_refs;
	  i++)
       {
	printf("entry %d ",i);
	if (is_null_node(prelim_info->array_table[i].node))
	  printf("IS NULL\n\n");
	else
	  {
	   tree_print(prelim_info->array_table[i].node);
	   printf("\n");
	  }
       }
  }



     
