/* $Id: astutil.C,v 1.3 2001/09/17 13:45:17 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************
*
*        astutil.c
*  Fortran AST manipulation routines.
*
*
*
***************************************************************************/

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>

#ifdef LINUX
#include <time.h>
#else
#include <sys/time.h>
#endif

#include <libs/support/misc/general.h>
#include <include/bstring.h>
#include <libs/support/database/newdatabase.h>

#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/frontEnd/ast/astlist.h>
#include <libs/frontEnd/ast/asttree.h>
#include <include/frontEnd/astnode.h>
#include <libs/frontEnd/ast/gen.h>

#include <libs/support/memMgmt/mem.h>
#include <libs/support/strings/rn_string.h>
#include <libs/support/tables/symtable.h>
#include <libs/support/file/File.h>


/* Global variables */
AST_INDEX    ast_null_node = (AST_INDEX)0;

/*Pointer to the current ast table */
Asttab    *asttab;



/* Pointer to the old ast table */
AsttabOld       *asttabOld;



/*  static variables for use with error code storage */

static AST_INDEX  Err_Nodes[1024];
static int        Err_Codes[1024];
static int        Err_HWM = 0;
/* static AST_INDEX  the_null_node; */
/* static int        has_been_assigned = 0; */


/**************************************/
/* Static Functions                   */
/**************************************/

static void    print_error_message();
static void    print_error_list_node();

/* ast_zero_scratch_all() is not needed since we */
/* now require the root to be passed to ast_zero_scratch() */
/* static void    ast_zero_scratch_all(); */

static void check_tree_edges (AST_INDEX oldson, AST_INDEX newson);
static void ast_dump_tree(AST_INDEX root);
static Header  init_header();
static Header *read_header();

static void ast_import2_back_end(DB_FP *buff_fp, Header *header, 
				 Asttab *newtab);


static void ast_write_nodes(DB_FP *buff_fp, AST_INDEX tree);

static int FindIndexIntoSideArrayTable();


/**************************************************************/
/* The following functions used to be macros but were turned  */
/* into functions to allow for error checking                 */
/**************************************************************/


Boolean is_leaf_node(AST_INDEX node)
{
  NODE_TYPE node_type;
  node_type = ast_get_node_type(node);
  return (BOOL(node != AST_NIL
	       && node != 0
	       && ast_get_node_type_son_count(node_type) == 0
	       && node_type != AST_LIST_OF_NODES
	       && node_type != AST_FREED_NODE));
}

Boolean is_list_node(AST_INDEX node)
{
  return (BOOL(ast_get_node_type(node) == AST_LIST_OF_NODES));
}

Boolean is_error_code(AST_INDEX node)
{
  if ( node == AST_NIL )
    return (BOOL(0));
  else
    return (BOOL(N(node)->Leafnode->node_type & ERROR_BIT));
}

static void print_error_message(char *str)
{
  fprintf(stderr, "Error is %s -- trying to access non-existent field\n", str);
}


static void print_error_list_node(char *place, char *field)
{
  fprintf(stderr, "Error in %s -- Listnodes have no %s field\n", place, field);
}

NODE_TYPE ast_get_node_type(AST_INDEX node)
{
  if (node != AST_NIL)  
    return (N(node)->Leafnode->node_type & NODE_TYPE_BITS);
  else return AST_NULL_NODE;
}


STR_INDEX ast_get_symbol(AST_INDEX node)
{
  /* check to make sure that the node is a leaf node */
  if (is_leaf_node(node))
    {
      return (N(node)->Leafnode->symbol);
    }
  else
    {
      /* fprintf(stderr, "Error in ast_get_symbol() -- node is not a leaf node\n"); */
      return 0;
    }
}

void ast_put_symbol(AST_INDEX node, STR_INDEX value)
{
  /* check to make sure that the node is a leaf node */
  if (is_leaf_node(node))
    {
      N(node)->Leafnode->symbol = value;
      str_reuse_symbol(value);
    }
  else
    fprintf(stderr, "Error in ast_put_symbol() -- node is not a leaf node\n");
}
  
AST_INDEX  ast_get_father(AST_INDEX node)
{
  if (node == AST_NIL || node == 0)
    return AST_NIL;

  /* if it is a list node, return its father */
  else if (is_list_node(node))
    return (N(node)->Listnode->father);

  /* if it is part of a list, return the head of the list */
  else if ( is_list_node( N(node)->Leafnode->father ) )
    return N(N(node)->Leafnode->father)->Listnode->father;

  /* else it has nothing to do with a list */
  else
    return (N(node)->Leafnode->father);
}

void ast_put_father(AST_INDEX son, AST_INDEX father)
{

  /* If there is no son, then do nothing */
  if (son == AST_NIL || son == 0)
    return;
  else if (is_list_node(son))
    N(son)->Listnode->father = father;
  else
    N(son)->Leafnode->father = father;
}


META_TYPE ast_get_meta_type(AST_INDEX node)
{
  if (is_list_node(node))
    {
      print_error_list_node("ast_get_meta_type()", "meta_type");
      return (META_TYPE)0;
    }
  else
    return (N(node)->Leafnode->meta_type);
}

void ast_put_meta_type(AST_INDEX node, META_TYPE meta_type)
{
  if (is_list_node(node))
    {
      print_error_list_node("ast_put_meta_type()","meta_type");
    }
  else
    N(node)->Leafnode->meta_type = meta_type;
}

STATUS ast_get_status(AST_INDEX node)
{
  if (is_list_node(node))
    {
      print_error_list_node("ast_get_status()","status");
      return (STATUS)0;
    }
  else
    return (N(node)->Leafnode->status);
}

void ast_put_status(AST_INDEX node, STATUS status)
{
  if (is_list_node(node))
    {
      print_error_list_node("ast_put_status()","status");
    }
  else
    N(node)->Leafnode->status = status;
}



int ast_get_real_type(AST_INDEX node)
{
  if (is_list_node(node))
    {
      print_error_list_node("ast_get_real_type()","real_type");
      return (int)0;
    }
  else
    return (N(node)->Leafnode->type & REAL_BITS);
}


int ast_get_converted_type(AST_INDEX node)
{
  if (is_list_node(node))
    {
      print_error_list_node("ast_get_converted_type()","converted_type");
      return (int)0;
    }
  else
    return (int)((int)(N(node)->Leafnode->type & CONVERTED_BITS) >> 4);
}

Generic ast_get_error_code(AST_INDEX node)
{
  int   i;

  if (is_error_code(node))
    {
      /* look up error code in the table */
      for (i = 0; i < Err_HWM; i++)
	{
	  if (node == Err_Nodes[i])
	    break;
	}
      return Err_Codes[i];
    }
  else
    return (Generic)0;
}

void ast_put_error_code(AST_INDEX node, Generic error_code)
{
  int      i;

  if (error_code)
    {
      N(node)->Leafnode->node_type |= ERROR_BIT;

      /* find a place to store in table */
      for (i = 0; i < Err_HWM; i++)
	{
	  if (node == Err_Nodes[i]) /* see if it is already in the table */
	    break;
	}
      if (i == Err_HWM && Err_HWM < 1024)
	Err_HWM++;
      Err_Nodes[i] = node;
      Err_Codes[i] = error_code;
    }
  else if (is_error_code(node))
    {
      /* we need to reset the error bit */
      N(node)->Leafnode->node_type &= ~ERROR_BIT;
      /* free the error code that is currently in the table */
      for (i = 0; i < Err_HWM; i++)
	{
	  if (node == Err_Nodes[i])
	    break;
	}
      Err_HWM--;
      Err_Nodes[i] = Err_Nodes[Err_HWM];
      Err_Codes[i] = Err_Codes[Err_HWM];
    }
}  

short ast_get_var_info(AST_INDEX)
{
  print_error_message("ast_get_var_info()");
  return 0;
}

void ast_put_var_info(AST_INDEX , short )
{
  print_error_message("ast_put_var_info()");
}

short ast_get_entry_info(AST_INDEX )
{
  print_error_message("ast_get_entry_info()");
  return 0;
}

void ast_put_entry_info(AST_INDEX , short )
{
  print_error_message("ast_put_entry_info()");
}


AST_INDEX ast_get_link_prev(AST_INDEX )
{
  print_error_message("ast_get_link_prev()");
  return AST_NIL;
}

void ast_put_link_prev(AST_INDEX , AST_INDEX )
{
  print_error_message("ast_put_link_prev()");
}


AST_INDEX ast_get_link_next(AST_INDEX )
{
  print_error_message("ast_get_link_next()");
  return AST_NIL;
}

void ast_put_link_next(AST_INDEX , AST_INDEX )
{
  print_error_message("ast_put_link_next()");
}


AST_INDEX ast_get_decl(AST_INDEX )
{
  print_error_message("ast_get_decl()");
  return AST_NIL;
}

void ast_put_decl(AST_INDEX , AST_INDEX )
{
  print_error_message("ast_put_decl()");
}

AST_INDEX ast_get_declaration(AST_INDEX )
{
  print_error_message("ast_get_declaration()");
  return AST_NIL;
}

void ast_put_declaration(AST_INDEX , AST_INDEX )
{
  print_error_message("ast_put_declaration()");
}


SCRATCH ast_get_scratch(AST_INDEX node)
{
  if (is_list_node(node))
    {
      print_error_list_node("ast_get_scratch()","scratch");
      return 0;
    }
  else
    return (N(node)->Leafnode->scratch);
}


void ast_put_scratch(AST_INDEX node, SCRATCH scratch)
{
  if (is_list_node(node))
    {
      print_error_list_node("ast_put_scratch()","scratch");
    }
  else
    N(node)->Leafnode->scratch = scratch;
}

short ast_get_display(AST_INDEX node)
{
  if (is_list_node(node))
    {
      print_error_list_node("ast_get_display()", "display");
      return 0;
    }
  else
    return (N(node)->Leafnode->display);
}

void ast_put_display(AST_INDEX node, short display)
{
  if (is_list_node(node))
    {
      print_error_list_node("ast_put_display()", "display");
    }
  else
    N(node)->Leafnode->display = display;
}

short ast_get_is_param(AST_INDEX )
{
  print_error_message("ast_get_is_param()");
  return 0;
}

void ast_put_is_param(AST_INDEX , short )
{
  print_error_message("ast_put_is_param()");
}



short ast_get_other(AST_INDEX )
{
  print_error_message("ast_get_other()");
  return 0;
}

void ast_put_other(AST_INDEX , short )
{
  print_error_message("ast_put_other()");
}


short ast_get_parens(AST_INDEX node)
{
  if (is_list_node(node))
    {
      print_error_list_node("ast_get_parens()", "parens");
      return 0;
    }
  /* The paren bit is stored within the display field */
  if (N(node)->Leafnode->display & PAREN_BIT)
    return 1;
  else
    return 0;
}

void ast_put_parens(AST_INDEX node, short parens)
{
  if (is_list_node(node))
    print_error_list_node("ast_put_parens()", "parens");      
  else
    {
      /* The paren bit is stored as part of the display field */
      if (parens)
	N(node)->Leafnode->display |= PAREN_BIT;
      else
	N(node)->Leafnode->display &= ~PAREN_BIT;
    }
}


AST_INDEX ast_get_head(AST_INDEX node)
{
  AST_INDEX  father;

  /* if the node is part of a list, ast_get_father will jump  */
  /* up to the head of the list.  If the node returned by     */
  /* ast_get_father is not a listnode then the node is not    */
  /* part of a list, and therefore the head should be AST_NIL */

  father = ast_get_father(node);
  if (is_list_node(father))
    return father;
  else
    return AST_NIL;
}


void ast_put_head(AST_INDEX node, AST_INDEX head)
{
  if (is_list_node(node))
    N(node)->Listnode->father = head;
  else
    fprintf(stderr, "Error in ast_put_head() -- the node is not an AST_LIST_OF_NODES\n");
}

AST_INDEX ast_get_next(AST_INDEX node)
{
  /* if we are at an auxiliary node or the head of a list */
  /* return the next element */
  if (is_list_node(node))
    return (N(node)->Listnode->next);

  /* else, the node is not part of a list, and therefore there */
  /* is no next node */
  else
    return AST_NIL;
}

void ast_put_next(AST_INDEX node, AST_INDEX next)
{
  /* if node is the null node then do nothing */
  if (node == AST_NIL || node == 0)
    return;

  else if (is_list_node(node) && (is_list_node(next) || next == AST_NIL))
    N(node)->Listnode->next = next;
  else
    fprintf(stderr, "Error in ast_put_next() -- either node or next is not an AST_LIST_OF_NODES\n");
}


AST_INDEX ast_get_prev(AST_INDEX node)
{
  if (is_list_node(node))
    return (N(node)->Listnode->prev);
  else
    {
      fprintf(stderr, "Error in ast_get_prev() -- node is not an AST_LIST_OF_NODES\n");
      return AST_NIL;
    }
}

void ast_put_prev(AST_INDEX node, AST_INDEX prev)
{
  /* if node is the null node, then do nothing */
  if (node == AST_NIL || node == 0)
    return;
  
  else if (is_list_node(node) && (is_list_node(prev) || prev == AST_NIL))
    N(node)->Listnode->prev = prev;
  else
    fprintf(stderr, "Error in ast_put_prev() -- either node or prev is not an AST_LIST_OF_NODES\n");
}


short ast_get_length(AST_INDEX node)
{
  AST_INDEX  father;
  short      length=0;
  AST_INDEX  curr;

  if (is_list_node(node))
    {
      father = N(node)->Listnode->father;
      if (is_list_node(father))
	curr = father;
      else
	curr = node;
      while ((curr = ast_get_next(curr)) != AST_NIL)
	length++;
      return length;
    }
  else
    {
      fprintf(stderr, "Error in ast_get_length() -- node is not part of a list\n");
      return 0;
    }
}

void ast_put_length(AST_INDEX , short )
{
  /* This does not make sense to me since there is no length field */
  fprintf(stderr, "Error in ast_put_length() -- there is no length field\n");
}


AST_INDEX  ast_get_last(AST_INDEX node)
{
  AST_INDEX  father;
  
  if (is_list_node(node))
    {
      father = N(node)->Listnode->father;

      /* if father is a list node, then father is the head */
      /* the list.  The last element in the list is head->last */
      if (is_list_node(father))
	return (N(father)->Listnode->prev);
      
      /* since the father is not a list node, node must be */
      /* the head of the list */
      else
	return (N(node)->Listnode->prev);
    }
  else
    {
      fprintf(stderr, "Error in ast_get_last() -- node is not part of a list\n");
      return AST_NIL;
    }
}


void ast_put_last(AST_INDEX node, AST_INDEX last)
{
  /* This function only resets the "last" pointer */
  /* in the head of the list */

  /* node should be a list, and last should be a list node */
  
  if (is_list_node(node) && is_list_node(last))
    {
      N(node)->Listnode->prev = last;
    }
  else
    {
      fprintf(stderr, "Error in ast_put_last() -- either node or last is not part of a list\n");
    }
}




AST_INDEX  ast_get_first(AST_INDEX node)
{
  AST_INDEX  father;
  
  if (is_list_node(node))
    {
      father = N(node)->Listnode->father;

      /* if father is a list node, father must be the */
      /* head of the list.  The first node in the list */
      /* is head->next */
      if (is_list_node(father))
	return (N(father)->Listnode->next);

      /* since father is not a list node, node must be */
      /* the head of the list */
      else
	return (N(node)->Listnode->next);
    }
  else
    {
      fprintf(stderr, "Error in ast_get_first() -- node is not part of a list\n");
      return AST_NIL;
    }
}


void ast_put_first(AST_INDEX node, AST_INDEX first)
{
  /* This function only resets the "first" pointer */
  /* in the head of the list */
  
  /* node should be a list, and first should be an aux node */

  if (is_list_node(node) && is_list_node(first))
    {
      N(node)->Listnode->next = first;
    } else {
      fprintf(stderr, "Error in ast_put_first() -- either node or first is not part of a list\n");
    }
}


AST_INDEX ast_get_son(AST_INDEX node)
{
  if (is_list_node(node))
    return N(node)->Listnode->son;
  else
    return AST_NIL;
}


/****************************************************************

 The following functions are new versions of functions 
 that exist in the old astutil.c 

*****************************************************************/


Asttab *ast_open (Nodeinfo *nodeinfo, Generic )
{
  /* the nodes argument is never used. It is there    */
  /* since that is how ast_open() is called by others */


  /* Save the current ast table pointer */
  
  Asttab   *othertab = asttab;
  
  /* Create a new ast table */

  Asttab   *newtab = (Asttab *) get_mem (AST_TABLE_SIZE, "Asttab");




  /* Initialize ast table statistics */
  
  newtab->stats.total_allocs = 0;
  newtab->stats.total_frees  = 0;
/*  newtab->stats.side_array_list = (Ast_side_array *) 0; */
  newtab->nodeinfo = nodeinfo;

  /* eventually, newtab->stats.side_array_width will be passed */
  /* in as an argument, but for now it is set to a default value */
  newtab->stats.side_array_width = 16;
  newtab->stats.side_array_in_use =
    (Generic *) get_mem (newtab->stats.side_array_width * sizeof(Generic),
			 "side_array_in_use");

  newtab->stats.side_array_initial_values =
    (Generic *) get_mem (newtab->stats.side_array_width * sizeof(Generic),
			 "side_array_initial_values");

  bzero ((char *) newtab->stats.side_array_in_use,
	 newtab->stats.side_array_width * sizeof(Generic));
  
  newtab->stats.SideArrayTableSize = INIT_SIDE_ARRAY_TABLE_SIZE;
  newtab->stats.SideArrayTableHWM = 0;
  newtab->stats.SideArrayTable =
    (Generic **) get_mem (newtab->stats.SideArrayTableSize * sizeof(Generic*),
			 "SideArrayTable");
  bzero ((char *) newtab->stats.SideArrayTable, newtab->stats.SideArrayTableSize * sizeof(Generic));
  
  
  /* Initialize the string table pointer */
  newtab->strtab = (Strtab *) 0;

  /* Make the new ast table current for a moment */
  asttab = newtab;

  /* I have decided that AST_NIL should simply be zero -- dcs 3/4/92 */
  /* Make sure that the_null_node only gets assigned once */
/*  if (!has_been_assigned)
    {
      the_null_node = ast_alloc(AST_NULL_NODE);
      has_been_assigned++;
    } */

  /* Make sure that the null node is initialized */
  /* AST_NIL = the_null_node */
  /* ast_null_node = the_null_node; */

  AST_NIL = 0;
  
  /* Restore the old ast table */
  asttab = othertab;

  /* Return the pointer to the new ast table */

  return newtab;
}


void ast_setroot(Asttab *a, AST_INDEX root)
{
  /* ASSERT: caller has done any relevant freeing */
  a->root = root;
}
  

void ast_close(Asttab *a)
{
  /* ft and ftt work in tandem; currently, ftt_Close calls  */
  /* tree_free on ft->root; ft_Close calls ast_destroy which */
  /* reaches here. the tree nodes are already gone by this point. */
  /* THIS SHOULD BE CHANGED -- ftt should not free the AST */ 
  /* underlying its ft  -- JMC 6/93 */

  if (a->root) tree_free(a->root);
  
  /* free the side arrays -- JMC 6/93 */
  free_mem(a->stats.side_array_in_use);
  free_mem(a->stats.side_array_initial_values);
  free_mem(a->stats.SideArrayTable);
  
  /* Free the ast table */
  free_mem(a);
  
  /* If the ast table just freed is current, clobber it */
  if (a == asttab)
    asttab = (Asttab *) 0;
}


Asttab *
ast_create(Nodeinfo *nodeinfo, Generic ast_size, Generic str_size)
{
  Asttab      *a = ast_open(nodeinfo, ast_size);
  Strtab      *s = str_open(str_size);
  
  a->strtab = s;
  a->root   = AST_NIL;

  return a;
}


void ast_destroy(Asttab *a)
{
  /* Close the ast table and the associated string table */
  str_close (a->strtab);
  ast_close (a);
}


/* I am not going to write the function ast_grow_tree()
   since it has no meaning in the new implementation */


Asttab *ast_select(Asttab *newtab)
{
  Asttab   *old = asttab;

  /* If the new table is specified as 0, don't change to a */
  /* new table, just return the pointer to the old table */

  if (newtab != 0)
    {
      /* Must want to select a new table.  Do */
      /* it, getting the string table, too */

      asttab = newtab;
      (void) str_select (newtab->strtab);
    }
  
  /* Return the old value of the asttab */

  return old;
}



/*******************************************************************/
/* Utility routines.                                               */
/*******************************************************************/


Generic ast_which_son(AST_INDEX node, AST_INDEX son)
{
  int num_of_sons;
  int i;
  AST_INDEX *v = &N(node)->Int1node->son1;

  num_of_sons = ast_get_son_count(node);
  for (i = 1; i <= num_of_sons; i++)
    {
      /* if (son == ast_get_son_n(node, i)) */
      if (son == v[i - 1])
	return i;
    }
  return 0;
}

Generic ast_get_side_array(Generic s_a_id, AST_INDEX node, Generic slot)
{
  if ((s_a_id + slot) >= asttab->stats.side_array_width)
    {
      /* They are trying to access a place that is out */
      /* of the range of the side_array */
      fprintf(stderr, "Error in ast_get_side_array() -- Trying to access slot %d, the width of the side_array is %d\n", slot + s_a_id, asttab->stats.side_array_width);
      return 0;
    }
  else if ( node == 0 )
    {
      return (asttab->stats.side_array_initial_values[s_a_id + slot]);
    }
  else if (N(node)->Leafnode->side_array_ptr == 0)
    return (asttab->stats.side_array_initial_values[s_a_id + slot]);
  else
    return (N(node)->Leafnode->side_array_ptr[s_a_id + slot]);
}


void ast_put_side_array(Generic s_a_id, AST_INDEX node, Generic slot, Generic value)
{
  int    i;
  int    index;

  if ( node == 0 )
    return;

  else if ((s_a_id + slot) >= asttab->stats.side_array_width)
    {
      /* They are trying to access a place that is out */
      /* of the range of the side_array */
      fprintf(stderr, "Error in ast_put_side_array() -- Trying to access slot %d, the width of the side_array is %d\n", slot + s_a_id, asttab->stats.side_array_width);
      return;
    }

  else if (N(node)->Leafnode->side_array_ptr == 0)
    {
      /* We need to allocate a new side array */
      N(node)->Leafnode->side_array_ptr =
	(Generic *) get_mem(asttab->stats.side_array_width * sizeof(Generic),
			    "side array");

      for (i = 0; i < asttab->stats.side_array_width; i++)
	{
	  /* place the initial values into the array */
	  N(node)->Leafnode->side_array_ptr[i] =
	    asttab->stats.side_array_initial_values[i];
	}

      /* do not forward substitute this */
      index = FindIndexIntoSideArrayTable();
      asttab->stats.SideArrayTable[ index ] = N(node)->Leafnode->side_array_ptr;
    }
  N(node)->Leafnode->side_array_ptr[s_a_id + slot] = value;
}



AST_INDEX  ast_get_son_n(AST_INDEX node, Generic son_num)
{
  int         num_of_sons;
  AST_INDEX  *v;

  num_of_sons = ast_get_son_count(node);
  
  if (son_num > num_of_sons || son_num < 1)
    {
      fprintf(stderr, "Error in ast_get_son_n() -- tried to access son number %d, only %d sons available\n", son_num, num_of_sons);
      return AST_NIL;
    }
  else
    {
      v = &N(node)->Int1node->son1;
      return v[son_num - 1];
    }
}


void ast_put_son_n(AST_INDEX node, Generic num, AST_INDEX son)
{
  int         num_of_sons;
  AST_INDEX  *v;

  num_of_sons = ast_get_son_count(node);
  if (num > num_of_sons || num < 1)
    {
      fprintf(stderr, "Error in ast_put_son_n() -- tried to access son number %d, only %d sons available\n", num, num_of_sons);
    }
  else
    {
      v = &N(node)->Int1node->son1;
      /* Check to make sure we are not violating any rules */
      check_tree_edges(v[num-1], son);
      
      /* Update the father's son pointer */
      v[num - 1] = son;
      
      /* Update the son's father pointer */
      if (son != AST_NIL && son != 0 && !(is_list_node(son)))
	N(son)->Leafnode->father = node;
      else if (son != AST_NIL && son != 0 && (is_list_node(son)))
	N(son)->Listnode->father = node;
    }
}


AST_INDEX ast_alloc(NODE_TYPE node_type)
{
  /* This is a temporary function.  It will soon be */
  /* replaced by a much better allocator to be written by Keith */
  
  AST_INDEX  node;
  int        num_of_sons;

  node = (AST_INDEX) calloc(1,sizeof(NODE));
  if (node_type == AST_NULL_NODE)
    {
      N(node)->Leafnode = (LeafNode *) calloc(1,sizeof(LeafNode));
      N(node)->Leafnode->father = AST_NIL;
    }
  else if (node_type == AST_LIST_OF_NODES)
    {
      N(node)->Listnode = (ListNode *) calloc(1,sizeof(ListNode));
      N(node)->Listnode->father = AST_NIL;
      N(node)->Listnode->next = AST_NIL;
      N(node)->Listnode->prev = AST_NIL;
      N(node)->Listnode->son = AST_NIL;
    }
  else
    {
      num_of_sons = ast_get_node_type_son_count(node_type);
      switch(num_of_sons)
	{
	case 0:
	  N(node)->Leafnode = (LeafNode *) calloc(1,sizeof(LeafNode));
	  break;
	case 1:
	  N(node)->Int1node = (Int_1_Node *) calloc(1,sizeof(Int_1_Node));
	  N(node)->Int1node->son1 = AST_NIL;
	  break;
	case 2:
	  N(node)->Int2node = (Int_2_Node *) calloc(1,sizeof(Int_2_Node));
	  N(node)->Int2node->son1 = AST_NIL;
	  N(node)->Int2node->son2 = AST_NIL;
	  break;
	case 3:
	  N(node)->Int3node = (Int_3_Node *) calloc(1,sizeof(Int_3_Node));
	  N(node)->Int3node->son1 = AST_NIL;
	  N(node)->Int3node->son2 = AST_NIL;
	  N(node)->Int3node->son3 = AST_NIL;
	  break;
	case 4:
	  N(node)->Int4node = (Int_4_Node *) calloc(1,sizeof(Int_4_Node));
	  N(node)->Int4node->son1 = AST_NIL;
	  N(node)->Int4node->son2 = AST_NIL;
	  N(node)->Int4node->son3 = AST_NIL;
	  N(node)->Int4node->son4 = AST_NIL;
	  break;
	case 5:
	  N(node)->Int5node = (Int_5_Node *) calloc(1,sizeof(Int_5_Node));
	  N(node)->Int5node->son1 = AST_NIL;
	  N(node)->Int5node->son2 = AST_NIL;
	  N(node)->Int5node->son3 = AST_NIL;
	  N(node)->Int5node->son4 = AST_NIL;
	  N(node)->Int5node->son5 = AST_NIL;
	  break;
	case 6:
	  N(node)->Int6node = (Int_6_Node *) calloc(1,sizeof(Int_6_Node));
	  N(node)->Int6node->son1 = AST_NIL;
	  N(node)->Int6node->son2 = AST_NIL;
	  N(node)->Int6node->son3 = AST_NIL;
	  N(node)->Int6node->son4 = AST_NIL;
	  N(node)->Int6node->son5 = AST_NIL;
	  N(node)->Int6node->son6 = AST_NIL;
	  break;
	default:
	  fprintf(stderr, "Error in ast_alloc() -- unknown node_type, %d\n",
		  node_type);
	  free((char *) node);
	  return (AST_INDEX) 0;
	}
      N(node)->Leafnode->father = AST_NIL;
    }
  asttab->stats.total_allocs++;
  N(node)->Leafnode->node_type = node_type;
  N(node)->Leafnode->side_array_ptr = 0;
  return node;
}

void ast_free(AST_INDEX node)
{
  int i;
  /* This is a temporary function.  It will soon be replaced */
  /* by a better function, to be written by Keith  */

  /* if it is the null node, don't free it.  We have to make sure */
  /* that there is only one copy of the null node floating around */
  if (node == AST_NIL)
    return;
  else if (node != 0)
    {
      if (N(node)->Leafnode->side_array_ptr != 0)
	{
	  for (i = 0; i < asttab->stats.SideArrayTableHWM; i++)
	    {
	      if (asttab->stats.SideArrayTable[i] == N(node)->Leafnode->side_array_ptr)
		{
		  /* found the side array in the Table */
		  int hwm = asttab->stats.SideArrayTableHWM;
		  asttab->stats.SideArrayTable[i] =
		    asttab->stats.SideArrayTable[(hwm - 1)];
		  asttab->stats.SideArrayTableHWM--;
		  break;
		}
	    }
	  free_mem(N(node)->Leafnode->side_array_ptr);
	}

      free((char *) N(node)->Leafnode);
      free((char *) node);
      asttab->stats.total_frees++;
    }
}


AST_INDEX ast_copy(AST_INDEX node)
{
  AST_INDEX        newnode;
  Generic          width,
                   i;
  NODE_TYPE        node_type;
  int              index;
  
  /* Check the validity of the node to be copied */
  if (node == AST_NIL || node == 0)
    return AST_NIL;

  node_type = ast_get_node_type(node);
  if (node_type == AST_FREED_NODE)
    {
      /* Trying to copy a node which is not allocated */
      (void) printf("cannot copy a non-allocated node\n");
    }
  
  newnode = ast_alloc(node_type);
  
  if (is_leaf_node(node))
    {
      if (N(node)->Leafnode->symbol != 0)
	str_reuse_symbol(N(node)->Leafnode->symbol);
      N(newnode)->Leafnode->symbol = N(node)->Leafnode->symbol;
    }

  if (!is_list_node(node))
    {
      N(newnode)->Leafnode->scratch = N(node)->Leafnode->scratch;
/*      N(newnode)->Leafnode->father  = N(node)->Leafnode->father; */
      N(newnode)->Leafnode->father  = AST_NIL;    
/*      N(newnode)->Leafnode->display  = N(node)->Leafnode->display;
      N(newnode)->Leafnode->type  = N(node)->Leafnode->type;
      N(newnode)->Leafnode->status  = N(node)->Leafnode->status;
      N(newnode)->Leafnode->meta_type  = N(node)->Leafnode->meta_type;*/

    }

  /* If there is a side array that we need to allocate a */
  /* a new one and copy over the contents    */
  if (N(node)->Leafnode->side_array_ptr != 0)
    {
      width = asttab->stats.side_array_width;
      

      N(newnode)->Leafnode->side_array_ptr =
	(Generic *) get_mem(width * sizeof(Generic), "side array");
      for (i = 0; i < width; i++)
	{
	  N(newnode)->Leafnode->side_array_ptr[i] =
	    N(node)->Leafnode->side_array_ptr[i];
	}

      /* stash the side array back into the SideArrayTable -- Alan & Nat */
      index = FindIndexIntoSideArrayTable();
      asttab->stats.SideArrayTable[ index ] = N(newnode)->Leafnode->side_array_ptr;
    }
  return newnode;
}


AST_INDEX ast_copy_with_type(AST_INDEX node)
{
  AST_INDEX        newnode;
  Generic          width,
                   i;
  NODE_TYPE        node_type;
  int              index;
  
  /* Check the validity of the node to be copied */
  if (node == AST_NIL || node == 0)
    return AST_NIL;

  node_type = ast_get_node_type(node);
  if (node_type == AST_FREED_NODE)
    {
      /* Trying to copy a node which is not allocated */
      (void) printf("cannot copy a non-allocated node\n");
    }
  
  newnode = ast_alloc(node_type);
  
  if (is_leaf_node(node))
    {
      if (N(node)->Leafnode->symbol != 0)
	str_reuse_symbol(N(node)->Leafnode->symbol);
      N(newnode)->Leafnode->symbol = N(node)->Leafnode->symbol;
    }

  if (!is_list_node(node))
    {
      N(newnode)->Leafnode->scratch = N(node)->Leafnode->scratch;
/*      N(newnode)->Leafnode->father  = N(node)->Leafnode->father; */
      N(newnode)->Leafnode->father  = AST_NIL;    
/*      N(newnode)->Leafnode->display  = N(node)->Leafnode->display; */
      N(newnode)->Leafnode->type  = N(node)->Leafnode->type;
/*      N(newnode)->Leafnode->status  = N(node)->Leafnode->status;
      N(newnode)->Leafnode->meta_type  = N(node)->Leafnode->meta_type;*/

    }

  /* If there is a side array that we need to allocate a */
  /* a new one and copy over the contents    */
  if (N(node)->Leafnode->side_array_ptr != 0)
    {
      width = asttab->stats.side_array_width;
      N(newnode)->Leafnode->side_array_ptr =
	(Generic *) get_mem(width * sizeof(Generic), "side array");
      for (i = 0; i < width; i++)
	{
	  N(newnode)->Leafnode->side_array_ptr[i] =
	    N(node)->Leafnode->side_array_ptr[i];
	}

      /* stash the side array back into the SideArrayTable -- Alan & Nat */
      index = FindIndexIntoSideArrayTable();
      asttab->stats.SideArrayTable[ index ] = 
	N(newnode)->Leafnode->side_array_ptr;
    }
  return newnode;
}

void ast_put_real_type(AST_INDEX node, Generic value)
{
  if (is_list_node(node))
    print_error_list_node("ast_put_real_type()","real_type");
  else
    {
      /* zero out the bottom four bits */
      N(node)->Leafnode->type &= ~REAL_BITS;

      N(node)->Leafnode->type |= (value & REAL_BITS);
    }
}
  
void ast_put_converted_type(AST_INDEX node, Generic value)
{
  if ( node == AST_NIL )
    return;

  if (is_list_node(node))
    print_error_list_node("ast_put_converted_type()","converted_type");
  else
    {
      /* zero out the top four bits */
      N(node)->Leafnode->type &= ~CONVERTED_BITS;

      N(node)->Leafnode->type |= ((value << 4) & CONVERTED_BITS);
    }
}



/* I don't think we will need ast_zero_display_top() */
void ast_zero_display_top()
{
  fprintf(stderr, "Error in ast_zero_display_top() - function not written\n");
}



void ast_zero_scratch(AST_INDEX node)
{
  AST_INDEX  next;
  int        num_of_sons, i;

  
  if (node == AST_NIL || node == 0)
    return;
  else if (is_list_node(node))
    {
      next = list_first(node);
      while (next != AST_NIL)
	{
	  ast_zero_scratch(next);
	  next = list_next(next);
	}
    }
  else
    {
      num_of_sons = ast_get_son_count(node);
      
      N(node)->Leafnode->scratch = 0;
      for (i = 1; i <= num_of_sons; i++)
	ast_zero_scratch(ast_get_son_n(node, i));
    }
}

static void check_tree_edges (AST_INDEX oldson, AST_INDEX newson)
{
    /************************************************************/
    /* We are changing the son pointer to a different value:    */
    /* */
    /* 1) Break the old son's father pointer.                   */
    /* 2) Check the new son's father pointer.                   */
    /* */
    /* We are trying to prevent any structure other than a      */
    /* strict tree from being manufactured. This could happen   */
    /* by adding a son which is not the root of a subtree. The  */
    /* old son will now become the root of a subtree.           */
    /************************************************************/

  AST_INDEX  newfather;

  /* Remove the father pointer of the old son */
  if (oldson == AST_NIL || oldson == 0)
    ;
  else if (is_list_node(oldson))
    N(oldson)->Listnode->father = AST_NIL;
  else
    N(oldson)->Leafnode->father = AST_NIL;


  if (newson == AST_NIL || newson == 0)
    newfather = AST_NIL;
  else if (is_list_node(newson))
    newfather = N(newson)->Listnode->father;
  else 
    newfather = N(newson)->Leafnode->father;
  
  

  
  /* Make sure that the new son is a root */
  
  if (newfather != AST_NIL)
    {
      /* Might want to dump the ast here... */
      (void) printf ("node not root of subtree\n");
    }
}



/*****************************************************************/
/* Higher Level ast access routines ast_dump, ast_statistics     */
/*****************************************************************/


void ast_dump_all()
{
  (void) ast_dump_tree(asttab->root);
}

static void ast_dump_tree(AST_INDEX root)
{
  int        num_of_sons;
  AST_INDEX  node;
  int        i;

  
  /* If it is a null node or if it has been freed do nothing */
  if (root == AST_NIL || root == 0 || ast_get_node_type(root) == AST_FREED_NODE)
    return;

  /* Print current node */
  ast_dump(root);

  /* If in a list, print each node in the list */
  if (is_list_node(root))
    {
      node = list_first(root);
      (void) printf("  list_first = %u\n", node);
      while (node != AST_NIL)
	{
          (void) printf("list node = %u\n", node);
	  AST_INDEX prev = list_prev(node);
      	  (void) printf("  list_prev = %u\n", prev);
	  node = list_next(node);
      	  (void) printf("  list_next = %u\n", node);
	}

      node = list_first(root);
      while (node != AST_NIL)
	{
	  ast_dump_tree(node);
	  AST_INDEX prev = list_prev(node);
	  node = list_next(node);
	}
    }

  /* Else, it is either an internal or leaf node */
  else
    {
      num_of_sons = ast_get_son_count(root);
      for (i = 1; i <= num_of_sons; i++)
	ast_dump_tree(ast_get_son_n(root, i));
    }
}


void ast_dump(AST_INDEX node)
{
  int   num_of_sons;
  int   i;
  
  /* If the node is AST_NIL, print nothing */
  if (node == AST_NIL || node == 0)
    return;

  (void) printf("node = %u\n", node);
  if (is_list_node(node))
    {
      (void) printf("  node_type = %u\n", ast_get_node_type(node));
      (void) printf("  next = %u\n", N(node)->Listnode->next);
      (void) printf("  prev = %u\n", N(node)->Listnode->prev);
      (void) printf("  father = %u\n", N(node)->Listnode->father);
      (void) printf("  son = %u\n", N(node)->Listnode->son);
    }
  else
    {
      num_of_sons = ast_get_son_count(node);
      (void) printf("  node_type = %u\n", ast_get_node_type(node));
      (void) printf("  type = %u\n", N(node)->Leafnode->type);      
      (void) printf("  status = %u\n", N(node)->Leafnode->status);      
      (void) printf("  scratch = %u\n", N(node)->Leafnode->scratch);
      (void) printf("  father = %u\n", N(node)->Leafnode->father);      
      (void) printf("  display = %u\n", N(node)->Leafnode->display);
      (void) printf("  meta_type = %u\n", N(node)->Leafnode->meta_type);
      (void) printf("  real type = %u\n", ast_get_real_type(node));      
      (void) printf("  converted type = %u\n", ast_get_converted_type(node));      
      (void) printf("  has parens = %u\n", ast_get_parens(node));      
      char *text = gen_get_text(node);
      if (text != NULL && text != "")
        (void) printf("  text = %s\n", text);

      if (num_of_sons == 0)
	      (void) printf("  symbol = %u\n", N(node)->Leafnode->symbol);
      else
	{
	  for (i = 1; i <= num_of_sons; i++)
	    (void) printf("  son%d = %u\n", i, ast_get_son_n(node, i));
	}
    }
}


Ast_stats ast_statistics()
{
  return asttab->stats;   /* Return the ast statistics block */
}


static Header init_header(Generic type, Generic flags, Generic str_size, 
                          Generic str_used, Generic str_bytes, AST_INDEX tree)
{
  Header    header;
  struct timeval ts;

  header.type = type;
  header.flags = flags;
  header.time = (int) time((time_t *) 0);
  header.uid = (int) getuid ();
  header.gid = (int) getgid ();
  header.str_size = str_size;
  header.str_used = str_used;
  header.str_bytes = str_bytes;
/*   header.asttab = 0;   -- dcs 5/18/92 */
  if (tree == AST_NIL)
    header.tree = 0;
  else
    header.tree = tree;
  header.buff_fp = 0;
  header.checksum = 0;
  return header;
}


static Header *read_header(DB_FP *buff_fp)
{
  Header          *header;

  /* It is ok to read in the rest of the header, so allocate a header */

  header = (Header *) get_mem(HDR_HEADER_SIZE, "header");

  /* Try to read the header */
  if (!db_buffered_read (buff_fp, (char *) header, sizeof(Header)))
    {
      free_mem(header);
      return 0;
    }

  /* Got a header.  Fill in the rest and return it */
  header->buff_fp = buff_fp;
  return header;
}


/* I don't feel that a garbage collect routine or */
/* a node marking routine will be necessary  */

AST_INDEX  ast_gc(AST_INDEX root)
{
  fprintf(stderr, "Error in ast_gc() - function not written\n");
  return root;
}



void ast_export2(DB_FP *buff_fp, AST_INDEX tree)
{
  Str_table_entry    *entry;
  Generic             string_loc,
                      i;
  Header              header;
  System_version       system_version;

  /* Write some system_version stuff */
  system_version.system = SYSTEM_SUN;
  /* The following lines were added to distinguish between different versions of ASTs */
  system_version.hdr_version = 0;
  system_version.ast_version = AST_VERSION;
  system_version.sym_version = 0;

  
  db_buffered_write(buff_fp, (char *) &system_version, sizeof(System_version));

  /* Call init_header with the correct values */
  header = init_header (DBTYPE_FORTRAN_CODE, 1,
			asttab->strtab->stats.size,
			asttab->strtab->stats.high_water_mark,
			asttab->strtab->stats.bytes_used,
			tree);
  
  /* Write out the header */
  db_buffered_write(buff_fp, (char *) &header, sizeof(Header));


  /* Write out asttab */
  db_buffered_write(buff_fp, (char *) asttab, sizeof(Asttab));  


  /* Write out the nodes */
  ast_write_nodes(buff_fp, tree);

  /* Write out some more stuff, like strings */
  /* STRING STATS */
  db_buffered_write(buff_fp, (char *) &asttab->strtab->stats,
		    sizeof(Str_stats));

  entry = (Str_table_entry *)
    get_mem(STR_ENT_SIZE * asttab->strtab->stats.size, "export strings");

  /* STRING BYTES */
  bcopy ((char *) asttab->strtab->table_entry,
	 (char *) entry, STR_ENT_SIZE * asttab->strtab->stats.size);

  string_loc = 0;
  for (i = 0; i < asttab->strtab->stats.high_water_mark; i++)
    {
      if (asttab->strtab->table_entry[i].in_use)
	{
	  char  *text;

	  text = string_table_get_text((STR_INDEX) i);
	  db_buffered_write(buff_fp, text, strlen(text));
	  db_buffered_write(buff_fp, "", 1);
	  entry[i].text = (char*) string_loc;
	  string_loc += strlen(text) + 1;
	}
    }

  /* TABLE ENTRIES */
  db_buffered_write (buff_fp, (char *) entry,
		     STR_ENT_SIZE * asttab->strtab->stats.size);

  /* STRING FREE LIST */
  db_buffered_write (buff_fp, (char *) &asttab->strtab->str_free_list,
		     sizeof(STR_INDEX));
  
  free_mem(entry);
}

static void ast_write_nodes(DB_FP *buff_fp, AST_INDEX tree)
{
  int         length;
  int         num_of_sons;
  AST_INDEX   node;
  int         i;
  NODE_TYPE   node_type;
  

  if (tree == AST_NIL || tree == 0)
    {
      node_type = AST_NULL_NODE;
      db_buffered_write(buff_fp, (char *) &node_type, sizeof(NODE_TYPE));
    }

  /* if it is the head of a list */
  else if (is_list_node(tree))
    {
      length = list_length(tree);

      db_buffered_write(buff_fp, (char *) &N(tree)->Listnode->node_type,
			sizeof(NODE_TYPE));
      db_buffered_write(buff_fp, (char *) &length, sizeof(int));
      node = list_first(tree);
      while (node != AST_NIL)
	{
	  ast_write_nodes(buff_fp, node);
	  node = list_next(node);
	}
    }
  else
    {
      num_of_sons = ast_get_son_count(tree);


      db_buffered_write(buff_fp, (char *) &N(tree)->Leafnode->node_type,
			sizeof(NODE_TYPE));

      if (N(tree)->Leafnode->node_type | ERROR_BIT)
      {
	Generic error_code = ast_get_error_code(tree);
	db_buffered_write(buff_fp, (char *) &error_code, sizeof(Generic));
      }

      db_buffered_write(buff_fp, (char *) &N(tree)->Leafnode->type,
			sizeof(TYPE));
      db_buffered_write(buff_fp, (char *) &N(tree)->Leafnode->status,
			sizeof(STATUS));
      db_buffered_write(buff_fp, (char *) &N(tree)->Leafnode->scratch,
			sizeof(SCRATCH));
      db_buffered_write(buff_fp, (char *) &N(tree)->Leafnode->display,
			sizeof(short));
      db_buffered_write(buff_fp, (char *) &N(tree)->Leafnode->meta_type,
			sizeof(META_TYPE));


      if (num_of_sons == 0)
	db_buffered_write(buff_fp, (char *) &N(tree)->Leafnode->symbol,
			  sizeof(STR_INDEX));
      else
	{
	  for(i = 1; i <= num_of_sons; i++)
	    ast_write_nodes(buff_fp, ast_get_son_n(tree, i));
	}
    }
}


AST_INDEX ast_import2(Nodeinfo *nodeinfo, DB_FP *buff_fp, Asttab **tab)
{
  Header        *header;
  Asttab        *newtab;
  System_version system_version;
  
  if (!db_buffered_read(buff_fp, (char *) &system_version,
			sizeof(System_version)))
    return 0;
  
  assert(system_version. ast_version == AST_VERSION);
  header = read_header(buff_fp);
  if (header == 0)
    return AST_NIL;
  
  /* Asttab */
  newtab = ast_create(nodeinfo, 0, header->str_size);
  ast_import2_back_end(buff_fp, header, newtab);
  
  *tab = newtab;
  
  /* save tree for return value and deallocate header */
  AST_INDEX tree = header->tree;
  free_mem(header);
  
  return tree;
}


static void ast_import2_back_end(DB_FP *buff_fp, Header *header, Asttab *newtab)
{
  Generic        i,
                 offset;
  Asttab         temptab;
  char          *temp;
  Asttab        *othertab;

  /* Make the newtab current during this function */
  othertab = ast_select(newtab);

  
  db_buffered_read(buff_fp, (char *) &temptab, sizeof(Asttab));

  newtab->stats.total_allocs = temptab.stats.total_allocs;
  newtab->stats.total_frees = temptab.stats.total_frees;

  /* The function ast_read_nodes will read in all of the nodes */
  /* and will return the root of the tree.  */
  newtab->root = ast_read_nodes(buff_fp);
  header->tree = newtab->root;
  
  /* STRING STATS */
  db_buffered_read(buff_fp, (char *) &newtab->strtab->stats,
		   sizeof(Str_stats));

  /* STRING BYTES */
  temp = (char *) get_mem(header->str_bytes, "import strings");

  db_buffered_read(buff_fp, (char *) temp, header->str_bytes);

  /* TABLE ENTRIES */
  {
    /* free initial valid entry before overwriting it -- JMC 8/93 */
    sfree(newtab->strtab->table_entry[0].text); 

    db_buffered_read (buff_fp, (char *) newtab->strtab->table_entry,
		      header->str_size * STR_ENT_SIZE);
  }

  
  offset = (Generic) temp;

  for (i = 0; i < newtab->strtab->stats.high_water_mark; i++)
    {
      if (newtab->strtab->table_entry[i].in_use)
	{
	  newtab->strtab->table_entry[i].text =
	    ssave(newtab->strtab->table_entry[i].text + offset);
	}
      else
	newtab->strtab->table_entry[i].text = ssave("");
    }

  /* STRING FREE LIST */
  db_buffered_read(buff_fp, (char *) &newtab->strtab->str_free_list,
		   sizeof(STR_INDEX));

  
  free_mem(temp); /* now flush all the strings */

  /* Make the old table current again */
  ast_select(othertab);
}    

     

AST_INDEX ast_read_nodes(DB_FP *buff_fp)
{
  NODE_TYPE   node_type;
  int         length,
              i;
  AST_INDEX   node;
  AST_INDEX   aux;
  AST_INDEX   prev;
  AST_INDEX   next_read;
  int         num_of_sons;
  
  db_buffered_read(buff_fp, (char *) &node_type, sizeof(NODE_TYPE));

  if (node_type == AST_NULL_NODE)
    {
      return AST_NIL;
    }
  else if ((node_type & NODE_TYPE_BITS) == AST_LIST_OF_NODES)
    {
      db_buffered_read(buff_fp, (char *) &length, sizeof(int));
      node = ast_alloc(AST_LIST_OF_NODES);

      /* Since this is recursive, if the node actually has */
      /* a father, it will be set correctly upon return    */
      N(node)->Listnode->father = AST_NIL;
      
      if (length == 0)
	{
	  /* ast_put_first(node, AST_NIL); */
	  /* ast_put_last(node, AST_NIL); */
	  N(node)->Listnode->next = AST_NIL;
	  N(node)->Listnode->prev = AST_NIL;
	}
      else
	{
	  prev = AST_NIL;
	  aux = ast_alloc(AST_LIST_OF_NODES);
	  ast_put_first(node, aux);

	  /* This for loop will read in all but the last one */
	  for (i = 1; i < length; i++)
	    {
	      
	      ast_put_head(aux, node);
	      next_read = ast_read_nodes(buff_fp);
	      N(aux)->Listnode->son = next_read;
	      ast_put_father(next_read, aux);

	      ast_put_prev(aux, prev);
	      ast_put_next(prev, aux);
	      prev = aux;
	      aux = ast_alloc(AST_LIST_OF_NODES);
	    }

	  /* Read in the last node */
	  ast_put_head(aux, node);
	  next_read = ast_read_nodes(buff_fp);
	  N(aux)->Listnode->son = next_read;
	  ast_put_father(next_read, aux);

	  ast_put_prev(aux, prev);
	  ast_put_next(prev, aux);
	  ast_put_next(aux, AST_NIL);
	  ast_put_last(node, aux);
	}
    }
  else 
    {
      num_of_sons = ast_get_node_type_son_count(node_type & NODE_TYPE_BITS);
      node = ast_alloc(node_type & NODE_TYPE_BITS);
      
      /* Since this is recursive, if the node actually does have */
      /* a father, it will be set correctly upon returning from  */
      /* being read.     */
      N(node)->Leafnode->father = AST_NIL;

      
      /* Since the node_type field also contains the error bit    */
      /* we have to be sure to write out exactly what was read in */
      N(node)->Leafnode->node_type = node_type;

      if (N(node)->Leafnode->node_type | ERROR_BIT)
      {
	Generic error_code;
	db_buffered_read(buff_fp, (char *) &error_code, sizeof(Generic));
	ast_put_error_code(node, error_code);
      }

      db_buffered_read(buff_fp, (char *) &N(node)->Leafnode->type,
		       sizeof(TYPE));
      db_buffered_read(buff_fp, (char *) &N(node)->Leafnode->status,
			sizeof(STATUS));
      db_buffered_read(buff_fp, (char *) &N(node)->Leafnode->scratch,
			sizeof(SCRATCH));
      db_buffered_read(buff_fp, (char *) &N(node)->Leafnode->display,
			sizeof(short));
      db_buffered_read(buff_fp, (char *) &N(node)->Leafnode->meta_type,
			sizeof(META_TYPE));

      if (num_of_sons == 0)
	db_buffered_read(buff_fp, (char *) &N(node)->Leafnode->symbol,
			  sizeof(STR_INDEX));

      else
	{
	  for (i = 1; i <= num_of_sons; i++)
	    {
	      next_read = ast_read_nodes(buff_fp);
	      ast_put_son_n(node, i, next_read);
	      ast_put_father(next_read, node);
	    }
	}
    }

  return node;
}


void ast_merge (Nodeinfo , DB_FP *, DB_FP *, Asttab **, 
                AST_INDEX *, AST_INDEX *)
{
  fprintf(stderr, "Error in ast_merge() - function not written\n");
}

Generic ast_attach_side_array(Asttab *tab, Generic width, Generic *initial_values)
{
  Generic     i, j;
  Generic     start;       
  Generic     s_a_id = -1;
  
  if (width > tab->stats.side_array_width)
    {
      fprintf(stderr, "Error in ast_attach_side_array() -- the width requested, %d, is greater that the maximum width, %d\n", width, tab->stats.side_array_width);
      return 0;
    }
  for(i = 0; i <= (tab->stats.side_array_width - width); i++)
    {
      /* Check to see if i through i+width are unoccupied */
      for (start = i; (i < (start + width)) &&
	   (tab->stats.side_array_in_use[i] ==0); i++)
	;
      if (i == (start + width))
	{
	  /* Since i == start + width, it means that there */
	  /* are width number of slots that are unoccupied */
	  s_a_id = start;
	  for (i = 0; i < width; i++)
	    {
	      tab->stats.side_array_in_use[s_a_id + i] = width;
	      
	      tab->stats.side_array_initial_values[s_a_id + i] =
		initial_values[i];
	    }
	  for (j = 0; j < tab->stats.SideArrayTableHWM; j++)
	    {
	      Generic *a_side_array = tab->stats.SideArrayTable[j];
	      if (a_side_array != 0)
		{
		  for (i = 0; i < width; i++)
		    a_side_array[s_a_id + i] = initial_values[i];
		}
	    }
	  return s_a_id;
	}
    }
  /* if we are here, it means that we were unable to find */
  /* a contiguous block that is width wide */
  fprintf(stderr, "Error in ast_attach_side_array() -- Unable to find a block that is %d wide\n", width);
  return 0;
}


void ast_detach_side_array(Asttab *tab, Generic s_a_id)
{
  Generic      i;
  Generic      width = tab->stats.side_array_in_use[s_a_id];

  for (i = s_a_id; i < (s_a_id + width); i++)
    tab->stats.side_array_in_use[i] = 0;
}


static int FindIndexIntoSideArrayTable()
{
  int return_val = asttab->stats.SideArrayTableHWM;
  asttab->stats.SideArrayTableHWM++;

  if (return_val == asttab->stats.SideArrayTableSize)
    {
      /* need to grow the SideArrayTable */
      asttab->stats.SideArrayTableSize *= 2;
      asttab->stats.SideArrayTable = 
	(Generic**)reget_mem( asttab->stats.SideArrayTable,
			     (Generic) asttab->stats.SideArrayTableSize * 
			     sizeof(Generic*), "growing");
    }
  return return_val;
} 

