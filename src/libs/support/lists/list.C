/* $Id: list.C,v 1.1 1997/06/25 15:16:14 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <stdlib.h>

#include <libs/support/memMgmt/mem.h>
#include <libs/support/lists/list.h>

#ifndef COREDUMP
#define COREDUMP()      abort()
#endif

#ifdef notdef
#define ASSERT(bool)    {if ( !(bool) ) {COREDUMP();}}
#else
#define ASSERT(bool)	{}
#endif

UtilNode *
util_node_alloc(Generic atom, char *who)
{
        return util_node_init( (UtilNode *)get_mem(sizeof(UtilNode), who ), atom );
}

UtilList *
util_list_alloc(Generic atom, char *who)
{
        return util_list_init( (UtilList *)get_mem(sizeof(UtilList), who ), atom );
}

UtilNode *
util_node_init(UtilNode *node, Generic atom)
{
        node->up = NULLLIST;
        node->next = NULLNODE;
        node->prev = NULLNODE;
        node->atom = atom;
        return node;
}

UtilList *
util_list_init(UtilList *list, Generic atom)
{
        list->head = NULLNODE;
        list->tail = NULLNODE;
	list->atom = atom;

	ASSERT( util_good_list(list) );
        return list;
}

UtilNode *
util_head(UtilList *list)
{
	ASSERT( util_good_list(list) );
        return list->head;
}

UtilNode *
util_tail(UtilList *list)
{
	ASSERT( util_good_list(list) );
        return list->tail;
}

Generic
util_list_atom(UtilList *list)
{
        return list->atom;
}


Generic
util_node_atom(UtilNode *node)
{
        return node->atom;
}


UtilNode *
util_next(UtilNode *node)
{
	return node->next;
}

UtilNode *
util_prev(UtilNode *node)
{
	return node->prev;
}

UtilList *
util_list(UtilNode *node)
{
        return node->up;
}

void
util_push(UtilNode *node, UtilList *list)
{
        /* check precondition */
	ASSERT( util_good_list(list) );
        ASSERT( ! util_in_list(node, list) );

        if ( list->tail == NULLNODE || list->head == NULLNODE )
        {
                /* consistency */
                ASSERT( list->tail == NULLNODE && list->head == NULLNODE );

                /* list was empty before */
                node->prev = NULLNODE;
                node->next = NULLNODE;
                node->up = list;

                list->tail = node;
                list->head = node;
        }
        else
        {
                node->next = list->head;
                node->prev = NULLNODE;
                node->up = list;
                list->head->prev = node;

                /* new node is head of list now */
                list->head = node;
        }
	ASSERT( util_good_list(list) );
}

void
util_append(UtilList *list, UtilNode *node)
{
        /* check precondition */
	ASSERT( util_good_list(list) );
        ASSERT( !util_in_list(node, list) );

        if ( list->tail == NULLNODE || list->head == NULLNODE )
        {
                /* consistency */
                ASSERT( list->tail == NULLNODE && list->head == NULLNODE );

                /* list was empty before */
                node->next = NULLNODE;
                node->prev = NULLNODE;
                node->up = list;

                list->tail = node;
                list->head = node;
        }
        else
        {
                node->prev = list->tail;
                node->next = NULLNODE;
                node->up = list;
                list->tail->next = node;

                /* new node is tail of list now */
                list->tail = node;
        }
	ASSERT( util_good_list(list) );
}

void
util_insert_after(UtilNode *afternode, UtilNode *node)
{
        UtilNode *temp;
        UtilList *list = util_list(afternode);

	ASSERT( util_good_list(list) );
        ASSERT( node != NULLNODE );
        ASSERT( afternode != NULLNODE );
        ASSERT( !util_in_list(node, list) );

        temp = afternode->next;                 /* fix afternode's links */
        afternode->next = node;

        node->prev = afternode;                 /* fix node's links */
        node->next = temp;
        node->up = list;

        if (temp == NULLNODE)                   /* fix the links of the node following node now */
                list->tail = node;
        else
                node->next->prev = node;
	ASSERT( util_good_list(list) );
}

void
util_insert_before(UtilNode *node, UtilNode *beforenode)
{
        UtilNode *temp;
        UtilList *list = util_list(beforenode);

	ASSERT( util_good_list(list) );
        ASSERT(beforenode != NULLNODE);
        ASSERT(node != NULLNODE);
        
        ASSERT(!util_in_list(node, list) );

        temp = beforenode->prev;                /* fix beforenode's links */
        beforenode->prev = node;

        node->next = beforenode;                /* fix node's links */
        node->prev = temp;
        node->up = list;

        if (temp == NULLNODE)                   /* fix the links of the node preceeding node now */
                list->head = node;
        else
                node->prev->next = node;
	ASSERT( util_good_list(list) );
}

void
util_join(UtilList *res, UtilList *first, UtilList *second)
{
        UtilNode *node;

	ASSERT( util_good_list(first) );
	ASSERT( util_good_list(second) );

        if ( first->head != NULLNODE )
        {
                if ( second->head != NULLNODE )
                {
                        /* want this to be safe even if "res" is actually the same as "first or "second" */ 
                        UtilList temp;

                        temp.head = first->head;
                        first->tail->next = second->head;
                        second->head->prev =  first->tail;
                        temp.tail = second->tail;
                        
                        res->head = temp.head;
                        res->tail = temp.tail;
                }
                else
                {
                        res->head = second->head;
                        res->tail = second->tail;
                }
        }
        else
        {
                if ( second->head != NULLNODE )
                {
                        res->head = first->head;
                        res->tail = first->tail;
                }
                else
                {
                        res->head = NULLNODE;
                        res->tail = NULLNODE;
                }
        }

        node = res->head;
        while (node != NULLNODE)
        {
                node->up = res;
                node = node->next;
        }
	ASSERT( util_good_list(res) );
}


void
util_pluck(UtilNode *to_pluck)
{
        UtilList *list = util_list(to_pluck);

	ASSERT( util_good_list(list) );
        if ( to_pluck->next == NULLNODE )
        {
                ASSERT( to_pluck == list->tail );
                if ( to_pluck->prev == NULLNODE )
                {
                        /* to_pluck is only node on list */
                        ASSERT( to_pluck == list->head );
                        list->head = NULLNODE;
                        list->tail = NULLNODE;
                }
                else
                {
                        /* to_pluck is at end of list */
                        to_pluck->prev->next = NULLNODE;
                        list->tail = to_pluck->prev;
                        list->tail->next = NULLNODE;
                }
        }
        else
        {
                if ( to_pluck->prev == NULLNODE)
                {
                        /* to_pluck is at head of list */
                        ASSERT( to_pluck == list->head );
                        list->head = to_pluck->next;
                        list->head->prev = NULLNODE;
                }
                else                            /* to_pluck is imbedded in list */
                {
                        to_pluck->prev->next = to_pluck->next;
                        to_pluck->next->prev = to_pluck->prev;
                }
        }

        to_pluck->next = NULLNODE;
        to_pluck->prev = NULLNODE;
        to_pluck->up = NULLLIST;

	ASSERT( util_good_list(list) );
        return;
}

Boolean
util_list_empty(UtilList *list)
{       
        return (Boolean) (list->head == NULLNODE);
}

UtilNode *
util_pop(UtilList *list)
{
        UtilNode *temp;

	ASSERT( util_good_list(list) );

        /* bad to pop from a null or mangled list */
        if ( list->head == NULLNODE )
        {
                ASSERT( list->tail == NULLNODE );
                return NULLNODE;
        }

        temp = list->head;
        list->head = list->head->next;

        temp->next = NULLNODE;
        temp->up = NULLLIST;

        if ( list->head == NULLNODE )
        {
                ASSERT( list->tail == temp );

                list->tail = NULLNODE;
        }
        else
        {
                list->head->prev = NULLNODE;
        }

	ASSERT( util_good_list(list) );
	return temp;
}

void
util_free_node(UtilNode *node)
{
        free_mem((void*)node );
}

void
util_free_node_and_atom(UtilNode *node)
{
        free_mem((void*)(node->atom) );
        free_mem((void*) node );
}

void
util_free_nodes(UtilList *list)
{
        UtilNode *head;

        while( (head = util_pop(list)) != NULLNODE )
        {
                util_free_node(head);
        }
}

void
util_free_nodes_and_atoms(UtilList *list)
{
        UtilNode *head;

        while( (head = util_pop(list)) != NULLNODE )
        {
                util_free_node_and_atom(head);
        }
}


void util_list_free(UtilList *list)
{
	free_mem((void*)list );
}


Boolean
util_in_list(UtilNode *node, UtilList *list)
{
        return (Boolean) (node->up == list);
}

Boolean
util_good_list(UtilList *list)
{
        UtilNode *current = list->head;
        UtilNode *last;

        if (current == NULLNODE)
        {
                if (list->tail == NULLNODE)
                        return true;
                else
                        COREDUMP();
        }

        if ( current->prev != NULLNODE )
                COREDUMP();

        last = current;
        current = current->next;

        while ( current != NULLNODE )
        {
                if ( current->prev != last )
                        COREDUMP();

                if ( last->up != list )
                        COREDUMP();

                last = current;
                current = current->next;
        }

        if ( last->up != list )
                COREDUMP();

        if ( last != list->tail )
                COREDUMP();

        return true;
}

void
util_apply(UtilList *list, util_apply_func func)
{
        UtilNode *temp = list->head;

	ASSERT( util_good_list(list) );
        while (temp != NULLNODE)
        {
                func(temp);
                temp = temp->next;
        }
}

void
util_apply_1(UtilList *list, util_apply1_func func, Generic arg1)
{
        UtilNode *temp = list->head;

	ASSERT( util_good_list(list) );
        while (temp != NULLNODE)
        {
                func(temp, arg1);
                temp = temp->next;
        }
}


void
util_apply_2(UtilList *list, util_apply2_func func, int arg1, int arg2)
{
        UtilNode *temp = list->head;

	ASSERT( util_good_list(list) );
        while (temp != NULLNODE)
        {
                func(temp, arg1, arg2);
                temp = temp->next;
        }
}

UtilNode *
util_match(UtilList *list, util_match_func func)
{
        UtilNode *temp = list->head;
        UtilNode *ret;

	ASSERT( util_good_list(list) );
        while (temp != NULLNODE)
        {
                ret = func(temp);
                if (ret)
                        return ret;
                temp = temp->next;
        }
        return NULLNODE;
}

UtilNode *
util_match_1(UtilList *list, util_match1_func func, int arg1)
{
        UtilNode *temp = list->head;
        UtilNode *ret;

	ASSERT( util_good_list(list) );
        while (temp != NULLNODE)
        {
                ret = func(temp, arg1);
                if (ret)
                        return ret;
                temp = temp->next;
        }
        return NULLNODE;
}


