/* $Id: list.h,v 1.8 1997/06/25 15:16:14 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef list_h
#define list_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

struct UtilList_struct;

typedef struct UtilNode_struct {
    struct UtilNode_struct *next;
    struct UtilNode_struct *prev;
    Generic atom;
    struct UtilList_struct *up;
} UtilNode;

typedef struct UtilList_struct {
    UtilNode *head;
    UtilNode *tail;
    Generic atom;
} UtilList;

EXTERN(UtilNode *, util_node_alloc, (Generic atom, char *who));
/*  Generic atom;	info for this node
 *  char *who;
 *
 * Returns a new initialized node.
 * `atom' is typically a pointer to the "owner" of the node,
 * eg. another structure containing the node.
 * `who' is used for debugging purposes.
 */

EXTERN(UtilList *, util_list_alloc, (Generic atom, char *who));
/*  Generic atom;	info for this node
 *  char *who;
 *
 * Returns a new initialized list.  `who' is used for debugging purposes.
 */

EXTERN(UtilNode *, util_node_init, (UtilNode *node, Generic atom));
/*  UtilNode *node;
 *  Generic atom;
 *
 * Initialize the fields of a node in a sensible way.
 * `atom' is typically a pointer to the "owner" of the node,
 * eg. another structure containing the node.
 * `node' is returned, for convenience.
 */

EXTERN(UtilList *, util_list_init, (UtilList *list, Generic atom));
/*  UtilList *list;
 *  Generic atom;
 *
 * Initialize the fields of a list in a sensible way.
 * `atom' is typically a pointer to the "owner" of the list,
 * eg. another structure containing the list.
 * `list' is returned, for convenience.
 */

EXTERN(UtilNode *, util_head, (UtilList *list));
/*  UtilList *list;
 *
 * Returns the head of `list'.  Returns NULLNODE if list is empty.
 */
 
EXTERN(UtilNode *, util_tail, (UtilList *list));
/*  UtilList *list;
 *
 * Returns the tail of `list'.  Returns NULLNODE if list is empty.
 */
 
EXTERN(UtilList *, util_list, (UtilNode *node));
/*  UtilNode *node;
 *
 * Returns the list of which `node' is a member.
 */

EXTERN(Generic, util_list_atom, (UtilList *list));
/*  UtilList *list;
 *
 * Returns the atom associated with `list'.
 */

EXTERN(Generic, util_node_atom, (UtilNode *node));
/*  UtilNode *node;
 *
 * Returns the atom associated with `node'.
 */

EXTERN(UtilNode *, util_next, (UtilNode *node));
/*  UtilNode *node;
 *
 * Returns the next node in the list.  Returns NULLNODE if there is no next node.
 */

EXTERN(UtilNode *, util_prev, (UtilNode *node));
/*  UtilNode *node;
 *
 * Returns the previous node in the list.  Returns NULLNODE if there is no previous node.
 */


EXTERN(void, util_push, (UtilNode *node, UtilList *list));
/*  UtilNode *node;
 *  UtilList *list;
 *
 * Pushes `node' onto the front of `list'.
 * If `node' is on another list when `util_push' is called, well,
 * the other list is trashed.  So don't do that.
 */

EXTERN(void, util_append, (UtilList *list, UtilNode *node));
/*  UtilList *list;
 *  UtilNode *node;
 *
 * Appends `node' to the end of `list'.
 * If `node' is on another list when `util_append' is called,
 * the other list is trashed.  So don't do that.
 */

EXTERN(void, util_insert_after, (UtilNode *afternode, UtilNode *node));
/*  UtilNode *afternode;
 *  UtilNode *node;
 *
 * Insert `node' into the list that `afternode' is on,
 * immediately after `afternode' in the list.
 * Preconditions:
 *  `afternode' should be on a list
 *  `node' should not be on any list
 */


EXTERN(void, util_insert_before, (UtilNode *node, UtilNode *beforenode));
/*  UtilNode *node;
 *  UtilNode *beforenode;
 *
 * Insert `node' into the list that `beforenode' is on,
 * immediately before `beforenode' in the list.
 * Preconditions:
 *  `beforenode' should be on a list
 *  `node' should not be on any list
 */

EXTERN(void, util_join, (UtilList *res, UtilList *first, UtilList *second));
/*  UtilList *res;
 *  UtilList *first;
 *  UtilList *second;
 *
 * Join two lists `first' and `second' end-to-end,
 * placing the result in `res'.
 * Preconditions:
 *  `res' need not be distinct from `first' or `second'
 * Postconditions:
 *  old contents, if any, of `res' are lost (not freed)
 *  `first' and `second' contain trash.  They should be
 *  re-initialized before any reuse.
 */

EXTERN(void, util_pluck, (UtilNode *to_pluck));
/*  UtilNode *to_pluck;
 *
 * Remove `to_pluck' from the list it is on.
 */

EXTERN(Boolean, util_list_empty, (UtilList *list));
/*  UtilList *list;
 *
 * Returns true iff `list' is an empty list.
 */

EXTERN(UtilNode *, util_pop, (UtilList *list));
/*  UtilList *list;
 *
 * Removes the first node from `list', and returns the popped node.
 * Returns NULLNODE if `list' is empty.
 */


EXTERN(void, util_list_free, (UtilList *list));
/*  UtilList *list;
 *
 * Frees list.
 */

EXTERN(void, util_free_node, (UtilNode *node));
/*  UtilNode *node;
 *
 * Frees node.
 */

EXTERN(void, util_free_node_and_atom, (UtilNode *node));
/*  UtilNode *node;
 *
 * Frees node and the atom field of the node.  `atom' is treated like
 * a pointer to a block of dynamic storage.
 */


EXTERN(void, util_free_nodes, (UtilList *list));
/*  UtilList *list;
 *
 * `util_free_node' is called for every node in `list'.
 */

EXTERN(void, util_free_nodes_and_atoms, (UtilList *list));
/*  UtilList *list;
 *
 * `util_free_node_and_atom' is called for every node in `list'.
 */

EXTERN(Boolean, util_in_list, (UtilNode *node, UtilList *list));
/*  UtilNode *node;
 *  UtilList *list;
 *
 * Returns true iff `node' is in `list'.
 */


EXTERN(Boolean, util_good_list, (UtilList *list));
/*  UtilList *list;
 *
 * util_good_list is used to verify the "goodness" of a list.
 * All next and prev links are checked, to determine if the list is
 * fully connected in both directions, and properly terminated.
 * This routine is of interest to those who either
 * doubt the correctness of the list routines in this file,
 * suspect that they may be using these routines incorrectly,
 * or suspect that their lists are being trampled by some other routine.
 * It is, or course, a fairly expensive call.
 */

typedef FUNCTION_POINTER(int, util_apply_func, (UtilNode *node));
EXTERN(void, util_apply, (UtilList *list, util_apply_func func));
/*  UtilList *list;
 *  util_apply_func func;
 *
 * Applies `func' to every node in `list'.  That is, for every node in the
 * list, `func' is called, with the node passed as an argument to `func'.
 */

typedef FUNCTION_POINTER(int, util_apply1_func,
 (UtilNode *node, Generic arg1));
EXTERN(void, util_apply_1,
 (UtilList *list, util_apply1_func func, Generic arg1));
/*  UtilList *list;
 *  util_apply1_func func;
 *  Generic arg1;
 *
 * Applies `func' to every node in `list', passing `arg1' as the second
 * argument to `func'.
 */

typedef FUNCTION_POINTER(int, util_apply2_func,
 (UtilNode *node, Generic arg1, Generic arg2));
EXTERN(void, util_apply_2,
 (UtilList *list, util_apply2_func func, Generic arg1, Generic arg2));
/*  UtilList *list;
 *  util_apply2_func func;
 *  Generic arg1;
 *  Generic arg2;
 *
 * Applies `func' to every node in `list', passing `arg1' as the second
 * argument to `func', and `arg2' as the third.
 */

typedef FUNCTION_POINTER(UtilNode*, util_match_func,
 (UtilNode *node));
EXTERN(UtilNode *, util_match,
 (UtilList *list, util_match_func func));
/*  UtilList *list;
 *  util_match_func func;	match function
 *
 * Apply `func' successively to each node of `list' (see `util_apply'),
 * until an application returns a value other than NULLNODE.  This value is
 * then returned, without applying `func' to the remaining nodes in `list'
 */

typedef FUNCTION_POINTER(UtilNode*, util_match1_func,
 (UtilNode *node, Generic arg1));
EXTERN(UtilNode *, util_match_1,
 (UtilList *list, util_match1_func func, Generic arg1));
/*  UtilList *list;
 *  util_match1_func func;	match function
 *  Generic arg1;
 *
 * `util_match_1' is `util_match' as `util_apply_1' is to `util_apply'
 *
 * Apply `func' successively to each node of `list' (see `util_apply_1'),
 * passing `arg1' as a second argument,
 * until an application returns a value other than NULLNODE.  This value is
 * then returned, without applying `func' to the remaining nodes in `list'
 */

#define NULLNODE (UtilNode *)0
#define NULLLIST (UtilList *)0


/* Macros for access described above */
#define UTIL_NODE_ATOM(n)	((n)->atom)
#define UTIL_LIST_ATOM(l)	((l)->atom)
#define UTIL_NEXT(n)		((n)->next)
#define UTIL_PREV(n)		((n)->prev)
#define UTIL_HEAD(l)		((l)->head)
#define UTIL_TAIL(l)		((l)->tail)
#define UTIL_LIST(n)		((n)->up)

#endif

