/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
#include <mh.h>
#include <gi.h>
#include <mem_util.h>
#include <Arena.h>

AST_INDEX ut_get_stmt(AST_INDEX node)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   while(!stmt_containing_expr(node) && node != AST_NIL)
     node = tree_out(node);
   if (node == AST_NIL)
     die_with_message("ut_get_stmt: Invalid Statement AST_NIL");
   else
     return(node);
  }

int ut_init_copies(AST_INDEX node,
		   Generic   copy_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX           name;
   subscript_info_type *sptr;
   
     if (is_identifier(node))
       if ((sptr = get_subscript_ptr(node)) != NULL &&
	   fst_GetField(((copy_info_type *)copy_info)->symtab,
			gen_get_text(node),SYMTAB_NUM_DIMS) > 0)
	 sptr->copies = (AST_INDEX *)((copy_info_type *)copy_info)->ar->
	      arena_alloc_mem(LOOP_ARENA,
			 ((copy_info_type *)copy_info)->val*sizeof(AST_INDEX));
     return(WALK_CONTINUE);
  }

AST_INDEX ut_tree_copy_with_type(AST_INDEX node,
		       int       index,
		       arena_type *ar)
	{
	AST_INDEX from;
	AST_INDEX result;
	Generic   i, num_of_sons;
	AST_INDEX temp;

	if (node == ast_null_node) return ast_null_node;

	if (is_list(node))
		{
		result = list_create(ast_null_node);

		/* Since result is an AST_LIST_OF_NODES, it will not */
		/* have a meta_type or status field */
		/* ast_put_meta_type(result, ast_get_meta_type(node)); */
		/* ast_put_status(result, ast_get_status(node)); */

		temp = list_first(node);
		while(temp != ast_null_node)
			{
			 (void) list_insert_last(result, 
						 ut_tree_copy_with_type(temp,index,ar));
			temp = list_next(temp);
			}
		}
	else
		{
		result = ast_copy_with_type(node);
		num_of_sons = ast_get_son_count(node);
		for(i=1; i <= num_of_sons; i++)
			{
			from = ast_get_son_n(node,i);
 			if (from != AST_NIL)
			  ast_put_son_n(result,i,ut_tree_copy_with_type(from,index,ar));
			}
		if (is_identifier(node))
		  if (get_subscript_ptr(node) != NULL &&
		      is_subscript(tree_out(node)))
		    {
		     get_subscript_ptr(node)->copies[index] = result;
		     create_subscript_ptr(result,ar);
		     get_subscript_ptr(result)->original = node;
		    }
		}

	return result;
	}

void ut_new_tail(listnode   *list,
		 EDGE_INDEX edge,
		 arena_type *ar)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   if (list->head != NULL)
     {
      list->tail->next = (listp)ar->arena_alloc_mem(LOOP_ARENA,
						    sizeof(listnode));
      list->tail->next->prev = list->tail;
      list->tail = list->tail->next;
      list->tail->edge = edge;
      list->tail->next = NULL;
     }
   else
     {
      list->head = (listp)ar->arena_alloc_mem(LOOP_ARENA,sizeof(listnode));
      list->head->prev = NULL;
      list->tail = list->head;
      list->tail->edge = edge;
      list->tail->next = NULL;
     }
  }

void ut_empty_new(listnode *list)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   listp p,q;

     if (list->ptail == NULL)
       {
	p = list->head;
	list->head = NULL;
       }
     else
       p = list->ptail->next;
     while (p != NULL)
       {
	q = p->next;
	/* free((char *)p); */
	p = q;
       }
     list->tail = list->ptail;
  }

listp ut_get_start(listnode list)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
  
  {
   listp p;

     if (list.ptail == NULL)
       p = list.head;
     else
       p = list.ptail->next;
     return(p);
  }

listp ut_remove(listp     p,
		listnode  *list)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   listp  q;

     if (p->prev != NULL)
       p->prev->next = p->next;
     if (p->next != NULL)
       p->next->prev = p->prev;
     q = p;
     p = p->next;
     if (list->head == q) 
       list->head = p;
     if (list->tail == q)
       if (p == NULL)
         list->tail = q->prev;
       else
         list->tail = p;
     if (list->ptail == q)
       if (p == NULL)
         list->ptail = q->prev;
       else
         list->ptail = p;
     /* free((char *)q); */
     return(p);
  }

void ut_free_list(listnode list)

  {
   listp  ptr,nptr;

     for (ptr = list.head;
	  ptr != NULL;
	  ptr = nptr)
       {
	nptr = ptr->next;
	free((char *)ptr);
       }
  }

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

int floor_ab(int a,
	     int b)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   int flr_ab;

     flr_ab = a / b;
     if (((a < 0 && b > 0) || (a > 0 && b < 0)) && (flr_ab * b != a))
       flr_ab--;
     return(flr_ab);
  }

/*

  % in C is the remainder function, not mod.  Below is the mod function 

  mod(a,b) = a mod b = a - floor(a/b) * b

*/

int mod(int a,
	int b)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
  
  {
   return(a - floor_ab(a,b) * b);
  }

int gcd(int a,
	int b)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   int x,y,t,r;

     x = a;
     y = b;
     if (x == 0)
       return(y);
     else if (y == 0)
       return(x);
     else
       {
	if (x < y)
	  {
	   t = x;
	   x = y;
	   y = t;
	  }
	while (y != 0)
	  {
	   r = mod(x,y);
	   x = y;
	   y = r;
	  }
       }
     return(x);
  }

int lcm(int a,
	int b)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   return((a * b) / (gcd(a,b)));
  }

AST_INDEX ut_gen_ident(SymDescriptor symtab,
		       char          *name,
		       int           asttype)

  {
   AST_INDEX   node;
   fst_index_t index;
   
     node = pt_gen_ident(name);
     gen_put_converted_type(node,asttype);
     gen_put_real_type(node,asttype);
     index = fst_Index(symtab,name);
     fst_PutFieldByIndex(symtab,index,NEW_VAR,true);
     fst_PutFieldByIndex(symtab,index,SYMTAB_TYPE,asttype);
     return(node);
  }

int ut_check_div(AST_INDEX node,
		 Generic   contains_div)

  {
   if (is_binary_divide(node))
     if (gen_get_converted_type(node) == TYPE_REAL ||
	 gen_get_converted_type(node) == TYPE_DOUBLE_PRECISION)
       {
	*(Boolean*)contains_div = true;
	return(WALK_ABORT);
       }
   return(WALK_CONTINUE);
  }
   
     
