/* $Id: tree.C,v 1.1 1997/06/24 17:56:30 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/****************************************************************/
		/*			tree.c					*/
		/* This file is a module which does file manipulations on the	*/
		/* external information file for the calling cp and builds the	*/
		/* outline tree based on the file.				*/
		/*								*/
		/****************************************************************/

#include <stdio.h>
#include <string.h>
#include <include/bstring.h>
#include <ctype.h>
#include <libs/graphicInterface/cmdProcs/help/help.h>
#include <libs/graphicInterface/cmdProcs/help/help_file.h>

#define	INDENT			3		/* formated text paragraph indentation	*/
#define BETWEEN_PARAGRAPHS	1		/* blank lines between paragraphs	*/

STATIC(void,		remove_tree,(NODE *root));    /* recursively remove outline tree	*/
STATIC(void,		delete_list,(LIST *head));    /* delete a transfer list		*/
STATIC(LIST,		*get_transfer_list,(LIST *test_list, NODE *subtree, 
                                            short *cptr, LIST *xfer_listt)); /* get a list of possible xfer nodes	*/
STATIC(void,		add_to_list,(Generic item, LIST **current));    /* add an entry to a list (ordered)	*/
STATIC(short,		compare_lists,(LIST *list1, LIST *list2));	/* return 1 if two lists intersect	*/
/*void			helpcp_free_tree(void);	*//* free the tree information		*/


/* Handle a new node at a given level.							*/
/*ARGSUSED*/
static
void
tree_node(STATE *instance, char *text, short length, short level)
{
NODE			*New;			/* the next node to be added to the tree*/
NODE			*parent;		/* the parent of the node 'new'		*/
char			*s;			/* pointer into heading text		*/

	/* make a new node */
		New = (NODE *) get_mem (sizeof (NODE), "Helpcp: (tree.c) new node");
		New->heading_level		= level;
		New->parent_heading		= NULL;
		New->heading_text[0]		= '\0';
		New->next_heading		= NULL;
		New->prev_heading		= NULL;
		New->number_of_subheadings	= 0;
		New->first_subheading		= NULL;
		New->last_subheading		= NULL;
		New->identification_list	= NULL;
		New->transfer_list		= NULL;
		New->text_block			= NULL;
		New->text_block_length		= 0;
		New->text_formated_width	= UNUSED;
		New->text_formated_length	= 0;
		New->first_text_entry		= NULL;
		New->last_text_entry		= NULL;

	/* attach the node to the tree & figure heading text */
		s = New->heading_text;
		if (instance->last)
		{/* this is a new node--attach it to the tree */
			/* set parent = the correct parent node */
				parent = instance->last;
				while ((parent != NULL) && (parent->heading_level >= New->heading_level))
					parent = parent->parent_heading;

			/* attach this node as the last child of the parent */
				New->parent_heading = parent;
				if (parent->number_of_subheadings == 0)
				{/* the only child of the parent */
					parent->first_subheading = New;
					parent->last_subheading =  New;
				}
				else
				{/* the last child */
					New->prev_heading = parent->last_subheading;
					parent->last_subheading->next_heading = New;
					parent->last_subheading = New;
				}
				parent->number_of_subheadings++;

			/* figure the heading text (section, blank, title) */
				for (parent = instance->root; parent != New; parent = parent->last_subheading)
				{/* add a section number for each level based on the current count of its parent */
					(void) sprintf(s, "%d.", parent->number_of_subheadings);
					s = strchr(s, '.') + 1;
				}
				*s++ = ' ';
		}
		else
		{/* root node-- */
			instance->root = New;
		}

	/* copy the new node text (dropping control characters) */
		while (*text)
		{/* transfer this text character */
			if (*text == QUOTE_CHAR)
			{/* quote the next character */
				*s++ = *++text;
			}
			else if (!iscntrl(*text))
			{/* copy regular characters */
				*s++ = *text;
			}
			text++;
		}
		*s = '\0';

	/* set up for the next iteration */
		instance->last = New;
}


/* Handle a positioning number at the current node.					*/
static
void
tree_positioning(STATE *instance, short value)
{
	if (instance->position_number == value)
	{/* this is a correct positioning--set the current node */
		instance->current = instance->last;
	}
}


/* Handle a positioning number at the current node.					*/
static
void
tree_identification(STATE *instance, short value)
{
	add_to_list(value, &instance->last->identification_list);
}


/* Handle a transfer number at the current node.					*/
static
void
tree_transfer(STATE *instance, short value)
{
	add_to_list(value, &instance->last->transfer_list);
}


/* Handle the block of text at the current node.					*/
static
void
tree_text(STATE *instance, char *buffer, short length)
{
char			*s;			/* transfer pointer			*/

	s = instance->last->text_block = (char *) get_mem(length + 1, "Helpcp: tree.c: saved text for node");
	while (*buffer)
	{/* transfer this text character */
		if (*buffer == QUOTE_CHAR)
		{/* quote the next character */
			*s++ = *++buffer;
		}
		else if (!iscntrl(*buffer))
		{/* copy regular characters */
			*s++ = *buffer;
		}
		buffer++;
	}
	*s = '\0';
	instance->last->text_block_length = s - instance->last->text_block;
}


/* Extract the pertinant outline base on arg_list from the file and put into a tree at	*/
/* root and current node current.  Return 0 if file is OK.				*/
char *
helpcp_read_tree(STATE *instance)
{
char			*error;			/* the error message			*/
error = helpcp_file_read(instance->file_name, instance->num_args, instance->arg_list, true, (Generic) instance, (PFV)tree_node, (PFV)tree_transfer, (PFV)tree_identification, (PFV)tree_positioning, (PFV)tree_text);
	if (error)
	{/* we have an error--undo our work */
		helpcp_free_tree(instance);
	}
	else
	{/* no errors--tidy things up a bit */
		if (instance->current == NULL)
		{/* choose default position */
			instance->current = instance->root;
		}
	}
	return error;
}



/* Free the tree and other info corresponding to an instance.				*/
void
helpcp_free_tree(STATE *instance)
{
	remove_tree(instance->root);
	instance->root    = NULL;
	instance->current = NULL;
	instance->last    = NULL;
	delete_list(instance->return_list);
	instance->return_list = NULL;
}


/* Change the notion of the current node based on val.					*/
void
helpcp_walk_tree(STATE *instance, short val)
{
register NODE		*node;			/* a pointer to a heading		*/
register LIST		*head;			/* the head of the list			*/
register char		**names;		/* pointers to possible headings	*/
short			count;			/* the number of possible xfers		*/
register LIST		*xfer_list;		/* the current head of posible xfers	*/
register LIST		*list;			/* the current entry in xfer_list	*/
register short		i;			/* the current node number		*/

	switch(val)
	{/* decide what to do */
		case NODE_PARENT:	/* the parent of the current node */
			helpcp_select(instance, instance->current->parent_heading);
			break;

		case NODE_ROOT:		/* the root of the outline */
			helpcp_select(instance, instance->root);
			break;

		case NODE_NEXT:		/* the next heading in the outline */
			if (instance->current->first_subheading)
			{/* the first child is next */
				helpcp_select(instance, instance->current->first_subheading);
			}
			else
			{/* move up until you can move over--that brother is next */
				for (node = instance->current; (!node->next_heading) && (node->parent_heading); node = node->parent_heading)
					;
				helpcp_select(instance, node->next_heading);
			}
			break;

		case NODE_PREVIOUS:	/* the previous heading in the outline */
			if (instance->current->prev_heading)
			{/* move over one and move down to the last node of the subtree */
				node = instance->current->prev_heading;
				for (node = instance->current->prev_heading; node->last_subheading; node = node->last_subheading)
					;
				helpcp_select(instance, node);
			}
			else
			{/* move up one level */
				helpcp_select(instance, instance->current->parent_heading);
			}
			break;
		
		case NODE_TRANSFER:	/* handle a transfer */
			if (instance->current->transfer_list)
			{/* a transfer is possible */
				count = 0;
				xfer_list = get_transfer_list(instance->current->transfer_list, instance->root, &count, (LIST *) 0);
				if (xfer_list)
				{/* there is a possible node to go to */
					names = (char **) get_mem(count * sizeof(char *), "Helpcp: (tree.c) name list");
					list = xfer_list;
					for (i = 0; i < count; i++)
					{/* transfer the i'th node name */
						names[i] = ((NODE *) list->item)->heading_text;
						list = list->next;
					}
					i = menu_select("Transfer to:", count, names);
					if (i != UNUSED)
					{/* go to node i */
						/* push the current heading onto the return stack */
							head = (LIST *) get_mem(sizeof(LIST), "Helpcp: (tree.c) return list");
							head->item = (Generic) instance->current;
							head->next = instance->return_list;
							instance->return_list = head;
						/* jump to the proper node */
							list = xfer_list;
							while (i--)
							{/* walk down the list */
								list = list->next;
							}
							helpcp_select(instance, (NODE *) list->item);
					}
					free_mem((void*) names);
				}
				else
				{/* give an excuse */
					message("No transfers possible from this heading.");
				}
				delete_list(xfer_list);
			}
			break;

		case NODE_RETURN:	/* handle a return */
			if (instance->return_list)
			{/* there is a node to return to */
				head = instance->return_list;
				node = (NODE *) head->item;
				instance->return_list = head->next;
				free_mem((void*) head);
				helpcp_select(instance, node);
			}
			break;

		default:		/* go to val'th child of current */
			node = instance->current->first_subheading;
			for (i = 0; i < val && node; i++)
			{/* walk to the next sibling */
				node = node->next_heading;
			}
			helpcp_select(instance, node);
			break;
	}

}


/* Return the storage of the tree rooted at 'root'.					*/
static
void
remove_tree(NODE *root)
{
NODE			*current_node;		/* the current subtree of root		*/
NODE			*next_node;		/* the left right sibling of current	*/
TEXT_ENTRY		*current_text;		/* the current text list entry		*/
TEXT_ENTRY		*next_text;		/* the entry after the current one	*/

	if (root)
	{/* there is a tree--remove it */
		current_node = root->first_subheading;
		while (root->number_of_subheadings)
		{/* remove each of the children trees */
			next_node = current_node->next_heading;
			remove_tree(current_node);
			current_node = next_node;
			root->number_of_subheadings--;
		}
		if (root->text_block)
			free_mem((void*) root->text_block);
		delete_list(root->identification_list);
		delete_list(root->transfer_list);
		for (current_text = root->first_text_entry; current_text; current_text = next_text)
		{/* delete a line from the end of the list */
			free_mem((void*) current_text->formated_text);
			next_text = current_text->next_text_entry;
			free_mem((void*) current_text);
		}
		free_mem((void*) root);
	}
}


/* format a block of text and put them in the current line list for printing		*/
void
helpcp_format_text(NODE *node, short width)
{
register char		*line_start;		/* the start of the current line	*/
register char		*line_end;		/* the end of the current line		*/
register char		*current_line;		/* the current line being formed	*/
register short		current_width;		/* formatting width of the current line	*/
register short		i;			/* dummy index				*/
register char		*para_start;		/* the start of the current paragraph	*/
register char		*para_end;		/* the end of the current paragraph	*/
register char		*block_start;		/* start of the block to be formated	*/
register char		*block_end;		/* the end of the block to be formated	*/
register TEXT_ENTRY	*current_entry;		/* the current line entry		*/
register TEXT_ENTRY	*next_entry;		/* the entry after the current one	*/

	if (width == node->text_formated_width || !node->text_block)
	{/* the text is formated--we are done */
		return;
	}
	for (current_entry = node->first_text_entry; current_entry; current_entry = next_entry)
	{/* delete a line from the end of the list */
		free_mem((void*) current_entry->formated_text);
		next_entry = current_entry->next_text_entry;
		free_mem((void*) current_entry);
	}
	node->first_text_entry     = NULL;
	node->last_text_entry      = NULL;
	node->text_formated_width  = UNUSED;
	node->text_formated_length = 0;
	/* find the beginning and end of the block */
		block_start = node->text_block;
		block_end   = block_start + node->text_block_length - 1;

	for (para_start = block_start; para_start <= block_end; para_start = para_end + 2)
	{/* format the paragraph */
		if (para_start != block_start)
		{/* not the first paragraph--skip the appropriate number of lines */
			for (i = 0; i < BETWEEN_PARAGRAPHS; i++)
			{/* insert a blank line */
				current_entry = (TEXT_ENTRY *) get_mem( sizeof(TEXT_ENTRY), "Helpcp: (tree.c) text line list append (empty line)");
				current_entry->formated_text = (char *) get_mem(1, "Helpcp: (tree.c) formatted text (empty line)");
				current_entry->enclosing_heading = node;
				current_entry->next_text_entry = NULL;
				current_entry->prev_text_entry = NULL;
				current_entry->formated_text[0] = '\0';
				if (node->first_text_entry)
				{/* this is an ordinary entry */
					node->last_text_entry->next_text_entry = current_entry;
					current_entry->prev_text_entry = node->last_text_entry;
				}
				else
				{/* this will be the first entry */
					node->first_text_entry = current_entry;
				}
				node->last_text_entry = current_entry;
				node->text_formated_length++;
			}
		}

		if (para_end = strchr(para_start, PARAGRAPH))
		{/* the end of the paragraph is just before the PARAGRAPH */
			para_end--;
		}
		else
		{/* there is no PARAGRAPH--the end of the paragraph is the end of the block */
			para_end = block_end;
		}

		for (line_start = para_start; line_start <= para_end; line_start = line_end)
		{/* format a line */
			/* make room for the line  */
				current_entry = (TEXT_ENTRY *) get_mem( sizeof(TEXT_ENTRY), "Helpcp: (tree.c) text line list append (full line)");
				current_entry->formated_text = (char *) get_mem(width + 1, "Helpcp: (tree.c) formatted text (full line)");
				current_entry->enclosing_heading = node;
				current_entry->next_text_entry = NULL;
				current_entry->prev_text_entry = NULL;
				current_line = current_entry->formated_text;
				if (node->first_text_entry)
				{/* this is an ordinary entry */
					node->last_text_entry->next_text_entry = current_entry;
					current_entry->prev_text_entry = node->last_text_entry;
				}
				else
				{/* this will be the first entry */
					node->first_text_entry = current_entry;
				}
				node->last_text_entry = current_entry;
				node->text_formated_length++;
			/* figure target size & set up for indentation */
				current_width = width;
				if (line_start == para_start)
				{/* indent this line */
					for (i = 0; i < INDENT; i++)
						*current_line++ = ' ';
					current_width -= INDENT;
				}
			/* figure the end of the line, format & set up for next iteration */
				line_end = line_start + current_width;
				if (line_end > para_end)
				{/* the end of a paragraph--do not format the line */
					current_width = para_end - line_start + 1;
					(void) strncpy(current_line, line_start, current_width);
					current_line[current_width] = '\0';
					line_end = para_end + 1;
				}
				else
				{/* an average line in the paragraph */
					while (!isspace(*line_end))
						line_end--;
					while (isspace(*line_end))
						line_end--;
					*++line_end = '\0';	/* temporarily end the string */
					(void) strcpy(current_line, line_start);
					*line_end++ = ' ';	/* restore the string again   */
					while (isspace(*line_end))
						line_end++;
				}
		}
	}
	node->text_formated_width = width;
}


/* Augment the transfer list for the current node to include those at subtree or below.	*/
static
LIST *
get_transfer_list(LIST *test_list, NODE *subtree, short *cptr, LIST *xfer_list)
{
register LIST		*New;			/* the new list entry			*/
register NODE		*child;			/* the children of this node		*/

	/* check its chidren's subtrees for possible additions */
		for (child = subtree->last_subheading; child; child = child->prev_heading)
			xfer_list = get_transfer_list(test_list, child, cptr, xfer_list);

	/* check the node itself */
		if (compare_lists(test_list, subtree->identification_list))
		{/* add subtree to the list of nodes */
			New = (LIST *) get_mem (sizeof(LIST), "Helpcp: (tree.c) get_xfer_list addition");
			New->item = (Generic) subtree;
			New->next = xfer_list;
			xfer_list = New;
			(*cptr)++;
		}
	return (xfer_list);
}


/* Add an entry to the linked list.  Keep in unique ascending order.			*/
static
void
add_to_list(Generic item, LIST **current)
{
register LIST		*New;			/* the new list entry			*/

	if (item > 0)
	{/* OK to add this entry */
		while (*current && ((*current)->item < item))
			current = &(*current)->next;
		if ((*current == NULL) || ((*current)->item != item))
		{/* add the entry to the list here */
			New = (LIST *) get_mem (sizeof(LIST), "Helpcp: (tree.c) xfer/id list addition");
			New->item = item;
			New->next = *current;
			*current = New;
		}
	}
}


/* Return 1 if list1 and list2 have a common entry.  Otherwize return 0.		*/
static
short
compare_lists(LIST *list1, LIST *list2)
{
	if ((list1 == NULL) || (list2 == NULL))
		return (0);
	else if (list1->item < list2->item)
		return (compare_lists(list1->next, list2));
	else if (list1->item > list2->item)
		return (compare_lists(list1, list2->next));
	else
		return (1);
}


/* Delete a list that starts at 'head' and free the associated space.			*/
static
void
delete_list(LIST *head)
{
	if (head)
	{/* there is a portion of the list remaining */
		delete_list(head->next);
		free_mem((void*) head);
	}
}
