/* $Id: astlist.C,v 1.1 1997/06/24 17:41:50 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/frontEnd/ast/astlist.h>
#include <stdio.h>




/*************************************/
/*    Creating and appending lists   */
/*************************************/


AST_INDEX list_create(AST_INDEX node)
{
  AST_INDEX  list;
  AST_INDEX  aux;

  list = ast_alloc(AST_LIST_OF_NODES);
  if (node != AST_NIL)
    {
      aux = ast_alloc(AST_LIST_OF_NODES);

      N(list)->Listnode->prev = aux;
      N(list)->Listnode->next = aux;
      
      N(aux)->Listnode->son = node;
      ast_put_father(node, aux);
      
      ast_put_head(aux, list);

      ast_put_prev(aux, AST_NIL);
      ast_put_next(aux, AST_NIL);
    }
  else
    {
      N(list)->Listnode->next = AST_NIL;
      N(list)->Listnode->prev = AST_NIL;
    }
  return list;
}

AST_INDEX  list_append(AST_INDEX list1, AST_INDEX list2)
{
  AST_INDEX  last1, first2, last2;

  if (list1 == AST_NIL)
    return list2;
  if (list2 == AST_NIL)
    return list1;

  if (list_empty(list2))
  {
    ast_free(list2);
    return list1;
  }

  /* if list1 is empty, then it contains a list header which may
     already be connected into a tree, so don't just return list2,
     go ahead and insert the items in list2 into list1 */

  first2 = N(list2)->Listnode->next;
  last2  = N(list2)->Listnode->prev;

  for (last1 = first2; last1 != AST_NIL; last1 = N(last1)->Listnode->next)
    ast_put_head(last1, list1);

  /* Free the header for list2 */
  N(list2)->Listnode->prev = AST_NIL;
  N(list2)->Listnode->next = AST_NIL;
  ast_free(list2);

  /* Find the last node of list1 */

  last1 = N(list1)->Listnode->prev;

  if (last1 != AST_NIL) {  /* list1 empty ? */
    /* Append the nodes for list1 and list2 */
    ast_put_prev(first2, last1);
    ast_put_next(last1, first2);
  } else {
    /* Make first node of list2 the first node of list1 as well */
    N(list1)->Listnode->next = first2;
  }

  /* Update the end of list pointer */
  N(list1)->Listnode->prev = last2;
  

  return list1;
}



/****************************************/
/* Information about lists              */
/****************************************/

Boolean is_list(AST_INDEX node)
{
  return BOOL((node != AST_NIL) &&
	        (ast_get_node_type(node) == AST_LIST_OF_NODES) &&
	        (ast_get_node_type(ast_get_father(node)) != AST_LIST_OF_NODES));
}


Boolean in_list(AST_INDEX node)
{
  if (node == AST_NIL)
    return false;

  return (BOOL(ast_get_node_type(node) != AST_LIST_OF_NODES
	       && ast_get_node_type(ast_get_father(node)) == AST_LIST_OF_NODES));
}


Boolean is_aux(AST_INDEX node)
{
  return (BOOL(node != AST_NIL
	       && ast_get_node_type(node) == AST_LIST_OF_NODES
	       && ast_get_node_type(ast_get_father(node)) == AST_LIST_OF_NODES));
}


Boolean list_empty(AST_INDEX list)
{
  /* if there is no first element, the list must be empty */
  return (BOOL(ast_get_first(list) == AST_NIL));
}


Generic list_length(AST_INDEX list)
{
  return ast_get_length(list);
}


AST_INDEX list_head(AST_INDEX node)
{
  if (is_list(node))
    return node;

  /* since ast_get_father skips over auxiliary nodes, it can */
  /* be called if node is an aux node or if it simply a node */
  /* the list */
  else if (is_aux(node)  || in_list(node))
    return ast_get_father(node);

  /* otherwise, the node has nothing to do with a list */
  else
    return AST_NIL;
}


/**************************************/
/* Examine lists                      */
/**************************************/


AST_INDEX list_first(AST_INDEX list)
{
  if (is_list(list))
    return (ast_get_son(N(list)->Listnode->next));
  else
    return AST_NIL;
}

AST_INDEX list_last(AST_INDEX list)
{
  if (is_list(list))
    return (ast_get_son(N(list)->Listnode->prev));
  else
    return AST_NIL;
}


AST_INDEX list_next(AST_INDEX node)
{
  AST_INDEX   aux;
  
  if (in_list(node))
    {
      aux = N(node)->Leafnode->father;

      /* if the next node is null node, ast_get_son will handle it correctly */
      return ast_get_son(ast_get_next(aux));
    }
  else if (is_aux(node))
    return ast_get_son(N(node)->Listnode->next);
  else
    return AST_NIL;
}


AST_INDEX list_prev(AST_INDEX node)
{
  AST_INDEX  aux;
  
  if (in_list(node))
    {
      aux = N(node)->Leafnode->father;

      /* if prev node is null node, ast_get_son() will handle correctly */
      return ast_get_son(ast_get_prev(aux));
    }
  else if (is_aux(node))
    return ast_get_son(N(node)->Listnode->prev);
  else
    return AST_NIL;
}


AST_INDEX  list_retrieve(AST_INDEX list, Generic element_number)
{
  AST_INDEX  element;
  Generic    length, i;


  length = list_length(list);
  if (element_number < 1 || element_number > length)
    return AST_NIL;

  element = list_first(list);
  for (i = 1;;i++)
    {
      if (i == element_number)
	return element;
      element = list_next(element);
    }

  /* Should never get here */
}


int list_element(AST_INDEX element)
{
  AST_INDEX  node;
  int        i;

  if (!in_list(element))
    return 0;

  node = list_first(list_head(element));
  for(i = 1; node != AST_NIL;i++)
    {
      if (node == element)
	return i;
      node = list_next(node);
    }

  /* Should never get here */
  /* If we get here, it is because the list is totally messed up */
  /* Print error message and return 0 */
  fprintf(stderr, "Error in list_element() - list not built correctly\n");
  return 0;
}



AST_INDEX  list_get_aux(AST_INDEX node)
{
  if (in_list(node))
    return N(node)->Leafnode->father;
  else
    return AST_NIL;
}

/***************************************************/
/* Inserting elements into lists                   */
/***************************************************/


AST_INDEX list_insert_before(AST_INDEX node, AST_INDEX element)
{
  AST_INDEX   prev, list, result, next;
  AST_INDEX   aux;
  
  if (!in_list(node) || element == AST_NIL)
    return AST_NIL;

  if (is_list(element))
  {
    result = list_first(element);
    aux = result;
    while (aux != AST_NIL)
    {
      next = list_next(aux);
      list_insert_before(node, aux);
      aux = next;
    }
    return result;
  }

  list = list_head(node);
  prev = list_prev(node);

  if (prev == AST_NIL)  /* Must be at first node in list */
    {
      (void) list_insert_first(list, element);
    }
  else
    {
      aux = ast_alloc(AST_LIST_OF_NODES);

      N(aux)->Listnode->son = element;
      ast_put_father(element, aux);

      ast_put_head(aux, list);

      ast_put_prev(aux, N(prev)->Leafnode->father);
      ast_put_prev(N(node)->Leafnode->father, aux);

      ast_put_next(N(prev)->Leafnode->father, aux);
      ast_put_next(aux, N(node)->Leafnode->father);
    }
  return element;
}


AST_INDEX list_insert_after(AST_INDEX node, AST_INDEX element)
{
  AST_INDEX   next, list, result, prev;
  AST_INDEX   aux;
  
  if (!in_list(node) || element == AST_NIL)
    return AST_NIL;

  if (is_list(element))
  {
    result = list_first(element);
    aux = list_last(element);
    while (aux != AST_NIL)
    {
      prev = list_prev(aux);
      list_insert_after(node, aux);
      aux = prev;
    }
    return result;
  }

  list = list_head(node);
  next = list_next(node);

  if (next == AST_NIL)  /* Must be at last node in list */
    {
      (void) list_insert_last(list, element);
    }
  else
    {
      aux = ast_alloc(AST_LIST_OF_NODES);

      N(aux)->Listnode->son = element;
      ast_put_father(element, aux);

      ast_put_head(aux, list);

      ast_put_prev(aux, N(node)->Leafnode->father);
      ast_put_prev(N(next)->Leafnode->father, aux);

      ast_put_next(N(node)->Leafnode->father, aux);
      ast_put_next(aux, N(next)->Leafnode->father);
    }
  return element;
}


AST_INDEX  list_insert_first(AST_INDEX list, AST_INDEX element)
{
  AST_INDEX   first, prev;
  AST_INDEX   aux;

  
  if (!is_list(list))
    return AST_NIL;
  if (element == AST_NIL)
    return list;

  if (is_list(element))
  {
    aux = list_last(element);
    while (aux != AST_NIL)
    {
      prev = list_prev(aux);
      list_insert_first(list, aux);
      aux = prev;
    }
    return list;
  }

  /* Insert at beginning of list */

  first = list_first(list);

  aux = ast_alloc(AST_LIST_OF_NODES);
  N(aux)->Listnode->son = element;
  N(element)->Leafnode->father = aux;
  ast_put_head(aux, list);

  /* Since ast_put_first() doesn't work quite right */
  /* I'm using the more complicated version */
  /* ast_put_first(list, aux); */
  N(list)->Listnode->next = aux;
  
  /* If an empty list, must set the end of list pointer */
  if (first == AST_NIL)
    {
      /* next line commented out because of ast_put_last() */
      /* ast_put_last(list, aux); */
      N(list)->Listnode->prev = aux;
      ast_put_next(aux, AST_NIL);
    }
  else
    {
      ast_put_next(aux, N(first)->Leafnode->father);
      ast_put_prev(N(first)->Leafnode->father, aux);
    }

  ast_put_prev(aux, AST_NIL);  /* added by dcs */
  return list;
}


AST_INDEX  list_insert_last(AST_INDEX list, AST_INDEX element)
{
  AST_INDEX  last;
  AST_INDEX  aux;

  if (!is_list(list))
    return AST_NIL;

  if (element == AST_NIL)
    return list;

  if (is_list(element))
    return list_append(list, element);

  /* Insert at end of list */

  aux = ast_alloc(AST_LIST_OF_NODES);
  last = list_last(list);

  N(aux)->Listnode->son = element;
  N(element)->Leafnode->father = aux;

  if (last == AST_NIL)   /* Empty List */
    {
      /* I commented out the next line since */
      /* ast_put_first() doesn't work right */
      /* ast_put_first(list, aux); */
      N(list)->Listnode->next = aux;
      ast_put_prev(aux, AST_NIL);
    }
  else
    {
      ast_put_next(N(last)->Leafnode->father, aux);
      ast_put_prev(aux, N(last)->Leafnode->father);
    }

  ast_put_head(aux, list);
  /* next line commented out because of ast_put_last() */
  /* ast_put_last(list, aux); */
  N(list)->Listnode->prev = aux;
  
  ast_put_next(aux, AST_NIL);      /* added by dcs */
  
  return list;
}



/*******************************************/
/* Removing elements from lists            */
/*******************************************/


AST_INDEX list_remove_first(AST_INDEX list)
{
  AST_INDEX   node_aux, next_aux;
  AST_INDEX   node;
  AST_INDEX   son;

  if (!is_list(list))
    return AST_NIL;

  /* Get the first auxiliary node */
  node_aux = N(list)->Listnode->next;

  /* Return null if there was no node in the list */
  if (node_aux == AST_NIL)
    return AST_NIL;

  next_aux = N(node_aux)->Listnode->next;

  if (next_aux != AST_NIL)
    ast_put_prev(next_aux, AST_NIL);
  else
    {
      /* Since next_aux is AST_NIL, that means that there */
      /* was only one element in the list.  We need to    */
      /* set the last pointer to be AST_NIL since we are  */
      /* about to remove the only element                 */
      N(list)->Listnode->prev = AST_NIL;
    }

  /* Remove node from the list */

  son = ast_get_son(node_aux);
  ast_put_head(node_aux, AST_NIL);
  ast_put_next(node_aux, AST_NIL);
  ast_put_prev(node_aux, AST_NIL);
  N(node_aux)->Listnode->son = AST_NIL;
  
  ast_put_father(son, AST_NIL);
  
  /* Since we have removed the first element, we */
  /* need to reset the first pointer             */
  N(list)->Listnode->next = next_aux;


  /* free the auxiliary node */
  ast_free(node_aux);


  return son;
}


AST_INDEX  list_remove_last(AST_INDEX list)
{
  AST_INDEX   node_aux;
  AST_INDEX   prev_aux;
  AST_INDEX   node;

  if (!is_list(list))
    return AST_NIL;

  /* Get the last auxiliary node in the list */
  node_aux = N(list)->Listnode->prev;
  if (node_aux == AST_NIL)
    return AST_NIL;

  prev_aux = N(node_aux)->Listnode->prev;

  /* If not the only element, clobber prev's next field */
  /* If it was the only element, reset the first pointer */
  if (prev_aux != AST_NIL)
    ast_put_next(prev_aux, AST_NIL);
  else
    {
      /* Since we are about to remove the only node */
      /* in the list, we must also reset the first pointer */
      N(list)->Listnode->next = AST_NIL;
    }

  node = ast_get_son(node_aux);

  ast_put_head(node_aux, AST_NIL);
  ast_put_next(node_aux, AST_NIL);
  ast_put_prev(node_aux, AST_NIL);
  N(node_aux)->Listnode->son = AST_NIL;
  
  ast_put_father(node, AST_NIL);

  /* Since we have just removed the last node, */
  /* we must reset the last pointer            */
  N(list)->Listnode->prev = prev_aux;
  
  /* free the auxiliary node */
  ast_free(node_aux);

  
  return node;
}    


AST_INDEX  list_remove_node(AST_INDEX node)
{
  AST_INDEX   prev, next, list;
  AST_INDEX   node_aux;
  
  /* if the node is not in a list, just return it */
  if (!in_list(node))
    return node;

  prev = list_prev(node);
  next = list_next(node);

  list = list_head(node);

  if (prev == AST_NIL)
    (void) list_remove_first(list);
  else if (next == AST_NIL)
    (void) list_remove_last(list);
  else
    {
      node_aux = list_get_aux(node);
      ast_put_head(node_aux, AST_NIL);
      ast_put_next(node_aux, AST_NIL);
      ast_put_prev(node_aux, AST_NIL);
      N(node_aux)->Listnode->son = AST_NIL;
      
      ast_put_father(node, AST_NIL);

      ast_put_next(list_get_aux(prev), list_get_aux(next));
      ast_put_prev(list_get_aux(next), list_get_aux(prev));
      /* free the auxiliary node */
      ast_free(node_aux);
    }

  return node;
}


AST_INDEX  list_remove(AST_INDEX list, Generic element_number)
{
  AST_INDEX   element;
  element = list_retrieve(list, element_number);

  if (element != AST_NIL)
    return list_remove_node(element);
  else
    return AST_NIL;
}

