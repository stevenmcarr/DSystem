/* $Id: asttree.C,v 1.1 1997/06/24 17:41:50 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <stdlib.h>
#include <libs/support/misc/general.h>
#include <libs/support/memMgmt/mem.h>
#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/frontEnd/ast/astlist.h>
#include <libs/frontEnd/ast/asttree.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/frontEnd/include/gi.h>

STATIC (void, indent, (Generic i, FILE *fp));
STATIC (void, tree_print_node, (AST_INDEX node, Generic level, Boolean sides_p,
                                FILE *fp));
STATIC (void, tree_print_internal, (AST_INDEX node, Generic level, Boolean sides_p,
                                    FILE *fp));
STATIC (void, tree_free_internal, (AST_INDEX node));


/****************************************************************************/
/* Copy                                                                     */
/****************************************************************************/

AST_INDEX tree_copy(AST_INDEX node)
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
			(void) list_insert_last(result, tree_copy (temp));
			temp = list_next(temp);
			}
		}
	else
		{
		result = ast_copy(node);
		num_of_sons = ast_get_son_count(node);
		for(i=1; i <= num_of_sons; i++)
			{
			from = ast_get_son_n(node,i);
 			if (from != AST_NIL)
		 	    ast_put_son_n(result,i,tree_copy(from));
			}
		}

	return result;
	}

AST_INDEX tree_copy_with_type(AST_INDEX node)
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
						tree_copy_with_type (temp));
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
		 	    ast_put_son_n(result,i,
					  tree_copy_with_type(from));
			}
		}

	return result;
	}

/****************************************************************************/
/* Free                                                                     */
/****************************************************************************/

void tree_free(AST_INDEX node)
{
   if (in_list(node)) {
		(void) printf("error occured in tree_free...node is in list...\n");
		tree_print(node);
		abort();
		}
   tree_free_internal(node);
}

static void tree_free_internal(AST_INDEX node)
//AST_INDEX node;
{
   AST_INDEX temp,next;
   Generic   i, num_of_sons;

   if (node == AST_NIL) return;

   if (is_list(node)) {
	temp = list_first(node);
	while (temp!=AST_NIL) {
		next = list_next(temp);
		(void) list_remove_node(temp);
		tree_free_internal(temp);
		temp = next;
		}
	ast_free(node);
	}
   else {
	num_of_sons = ast_get_son_count(node);
	for (i = 1; i <= num_of_sons; i++)
	  tree_free_internal(ast_get_son_n(node,i));
	ast_free(node);
   }
}

/****************************************************************************/
/* Replace - put newnode where oldnode was. Returns nothing. Oldnode must   */
/*           still be freed.                                                */
/****************************************************************************/

void tree_replace(AST_INDEX oldnode, AST_INDEX newnode)
	{
	AST_INDEX f;
	Generic   i;

	if (in_list(oldnode))
		{
		/* replace in a list */
		(void) list_insert_before(oldnode, newnode);
		(void) list_remove_node(oldnode);
		}
	else if ((oldnode != AST_NIL) && (ast_get_father(oldnode) != AST_NIL))
		{
		/* replace of a son */
		f = ast_get_father(oldnode);
		i = ast_which_son(f, oldnode);
		ast_put_son_n(f, i, newnode);
		}
	}

/*************************************************************
 * tree_replace_free(old_node,new_node)
 *
 *   Do improved tree_replace.  This routine frees whatever
 *   was at old_node.  Also, if old_node is the root, the
 *   routine still works, whereas tree_replace does not.
 *
 * Inputs:  old_node = node of tree to replace
 *          new_node = what to replace old_node with
 *************************************************************
 */
void
tree_replace_free(AST_INDEX old_node, AST_INDEX new_node)
{
  AST_INDEX father;
  Generic idx;

  if (old_node == new_node)
    return;
  if (old_node == AST_NIL)
    old_node = gen_PLACE_HOLDER();
  if (in_list(old_node))
    tree_replace(old_node,new_node);
  else {
    father = ast_get_father(old_node);
    if (father != AST_NIL)
      idx = ast_which_son(father,old_node);
    tree_free(old_node);
    old_node = tree_copy(new_node);
    tree_free(new_node);
    if (father != AST_NIL)
      ast_put_son_n(father,idx,old_node);
  }
} /* end_tree_replace_free */


static void indent(Generic i, FILE *fp) 
	{
	while(i-- > 0) fprintf(fp, "  ");
	}

static void tree_print_node(AST_INDEX node, Generic level, Boolean sides_p, 
                            FILE *fp)
	{
	STR_TEXT  text;
	STR_INDEX sym;
	Generic   type = ast_get_node_type(node);
	indent(level, fp);

	if (is_list(node))
	{
		(void) fprintf(fp, "%s [%d] ", ast_get_node_type_name(type), node);
		(void) fprintf(fp, "\n");
		return;
	}

	(void) fprintf(fp, "%s [%d,%d] ", ast_get_node_type_name(type),
  		   	      node, ast_get_meta_type(node));

	if (ast_get_real_type(node) != TYPE_UNKNOWN) 
	   (void) fprintf(fp, "R%s ",gen_type_get_text_short(ast_get_real_type(node)));
	if (ast_get_converted_type(node) != TYPE_UNKNOWN) 
	   (void) fprintf(fp, "C%s ",gen_type_get_text_short(ast_get_converted_type(node)));

        sym = ast_get_symbol(node);
        text = string_table_get_text(sym);

	if (text[0] != '\0') 
	    (void) fprintf(fp, "%d:\"%s\" ",sym,text);

	if (sides_p && (N(node)->Leafnode->side_array_ptr))
	{
	    int i;
	    (void) fprintf(fp, "sides: ");
	    for (i = 0; i < asttab->stats.side_array_width; i++)
		if (asttab->stats.side_array_in_use[i])
		    (void) fprintf(fp, "%d ", N(node)->Leafnode->side_array_ptr[i]);
	}

	(void) fprintf(fp, "\n");
	}

static void tree_print_internal(AST_INDEX node, Generic level, Boolean sides_p, 
                                FILE *fp)
	{
	Generic i,n;

	if (node == AST_NIL) return;

        n = ast_get_node_type_son_count(ast_get_node_type(node));

	/* Print the node contents */
	tree_print_node(node, level, sides_p, fp);
	/* Print the sons */
	if (is_list(node))
		{
		node = list_first(node);
		while(node != ast_null_node)
			{
			tree_print_internal(node, level+1, sides_p, fp);
			node = list_next(node);
			}
		}
	else
		{
		for(i = 1; i <= n; i++)		
			{
			tree_print_internal(ast_get_son_n(node, i),
					    level+1, sides_p, fp);
			}
		}
	}

void tree_print(AST_INDEX node)
	{
	tree_print_internal(node, 0, false, stdout);
	}

void tree_print_to_file(AST_INDEX node, FILE *fp)
	{
	tree_print_internal(node, 0, false, fp);
	}

void tree_print_w_sides(AST_INDEX node)
	{
	tree_print_internal(node, 0, true, stdout);
	}

AST_INDEX tree_out(AST_INDEX node)
	{
	if (in_list(node)) return ast_get_father(list_head(node));
	else return ast_get_father(node);
	}
