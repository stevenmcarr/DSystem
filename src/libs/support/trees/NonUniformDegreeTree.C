/* $Id: NonUniformDegreeTree.C,v 1.6 1997/03/11 14:37:48 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <libs/support/trees/NonUniformDegreeTree.h>

/******************************************************************
 * Non-Uniform Degree Tree Abstraction         September 1991     *
 * Author: John Mellor-Crummey                                    *
 *                                                                *
 * this file contains a general purpose non-uniform degree tree   *
 * abstraction and an iterator abstraction that supports preorder *
 * and postorder traversal of the nodes in a non-uniform degree   *
 * tree. this abstraction is useless in its own right             *
 * since the tree contains only structural information. to make   *
 * use of this abstraction, derive a tree node class that         *
 * contains some useful data. all of the structural manipulation  *
 * can be performed using the functions provided in the base      *
 * class defined herein.                                          *
 *                                                                *
 * Copyright 1991, Rice University, as part of the ParaScope      *
 * Programming Environment Project                                *
 *                                                                *
 ******************************************************************/

//-----------------------------------------------
// constructor initializes empty node then links
// it to its parent and siblings (if any)
//-----------------------------------------------
NonUniformDegreeTreeNode::NonUniformDegreeTreeNode(
	NonUniformDegreeTreeNode *parent)
{
	// no parent
	this->parent = 0;

	// no children
	child_count = 0; children = 0;

	// initial circular list of siblings includes only self
	next_sibling = this; prev_sibling = this;

	link(parent); // link to parent and siblings if any
}


//-----------------------------------------------
// links a node to a parent and at the end of the 
// circular doubly-linked list of its siblings 
// (if any)
//-----------------------------------------------
void NonUniformDegreeTreeNode::link(NonUniformDegreeTreeNode *parent)
{
	assert(this->parent == 0); // can only have one parent
	this->parent = parent;
	if (parent != 0) {
		parent->child_count++;

		// children maintained as a doubly linked ring. 
		// a new node is linked at the end of the ring (as a predecessor 
		// of "parent->children") which points to first child in the ring

		NonUniformDegreeTreeNode *first_sibling = parent->children;
		if (first_sibling) {
			// link forward chain
			next_sibling = first_sibling;
			first_sibling->prev_sibling->next_sibling = this;

			// link backward chain
			prev_sibling = first_sibling->prev_sibling;
			first_sibling->prev_sibling = this;
		} else 
			// solitary child
			parent->children = this;
	}
}

//-----------------------------------------------
// unlinks a node from a parent and siblings
//-----------------------------------------------
void NonUniformDegreeTreeNode::unlink()
{
	if (parent != 0) {
		// children maintained as a doubly linked ring. 
		// excise this node from from the ring 
		if (--(parent->child_count) == 0) {
			// current node is removed as only child of parent
			// leaving it linked in a circularly linked list with
			// itself
			parent->children = 0;
		} else {
			// unlink forward chain
			prev_sibling->next_sibling = next_sibling;
			next_sibling = this;

			// unlink backward chain
			next_sibling->prev_sibling = prev_sibling;
			prev_sibling = this;

			// fix link from parent to the ring if necessary
			if (parent->children == this)
				parent->children = next_sibling;
		}
	}
	this->parent = 0;
}


//-----------------------------------------------
// virtual destructor that frees all of its 
// children before freeing itself 
//-----------------------------------------------
NonUniformDegreeTreeNode::~NonUniformDegreeTreeNode()
{
	if (child_count > 0) {
		NonUniformDegreeTreeNode *next, *start = children;
		for (int i = child_count; i-- > 0; ) {
			next = start->next_sibling;
			delete start;
			start = next;
		}
	}
}

// ************ class NonUniformDegreeTreeIterator ************************

//-----------------------------------------------
// forward declarations for functions used by the 
// iterator class to enumerate nodes in a tree
//-----------------------------------------------
static NonUniformDegreeTreeNode *
	leftmost_deepest_child(NonUniformDegreeTreeNode *node);
static NonUniformDegreeTreeNode *
	next_unenumerated_sibling_of_ancestor(NonUniformDegreeTreeNode *node);

//-----------------------------------------------
// initialize an iterator for a non-uniform degree 
// tree. arguments are the root of the tree,
// and a type that selects a preorder or 
// postorder enumeration of nodes 
//-----------------------------------------------
void NonUniformDegreeTreeIterator::reset(
	NonUniformDegreeTreeNode *tree_root, 
	NonUniformDegreeTreeIteratorType type)
{
	root = tree_root;

	switch(type) {
	case PREORDER:
		current = root; 
		next = &NonUniformDegreeTreeIterator::pre_order_next;
		break;
	case POSTORDER:
		// current starts as leftmost, bottommost node
		current = leftmost_deepest_child(root);
		next = &NonUniformDegreeTreeIterator::post_order_next;
		break;
	}
} 

//-----------------------------------------------
// return the lowest leftmost child of a subtree 
// rooted at node
//-----------------------------------------------
NonUniformDegreeTreeNode * 
leftmost_deepest_child(NonUniformDegreeTreeNode *node)
{
	NonUniformDegreeTreeNode *child;
	while ((child = node->FirstChild()) != 0) node = child;
	return node;
}


//-----------------------------------------------
// return the next unenumerated sibling of node 
// in an ancestor in the tree rooted at "root"
// this routine is only used for preorder 
// traversal
//-----------------------------------------------
NonUniformDegreeTreeNode *
next_unenumerated_sibling_of_ancestor(NonUniformDegreeTreeNode *node)
{
	NonUniformDegreeTreeNode *parent;
	while ((parent = node->Parent()) && 
			(node->NextSibling() == parent->FirstChild())) 
		node = parent;
	return (parent ? node->NextSibling() : 0);
}


//-----------------------------------------------
// set current to the next node in the preorder
// enumeration of the tree with root "root" 
//-----------------------------------------------
void NonUniformDegreeTreeIterator::pre_order_next()
{
	if (current->ChildCount() != 0) current = current->FirstChild();
	else {
		NonUniformDegreeTreeNode *parent = current->Parent();
		if (parent == 0) current = 0; // can't have siblings
		else {
			// ---- look for siblings in circular list -----
			NonUniformDegreeTreeNode *next_sibling = current->NextSibling();
			if (next_sibling != parent->FirstChild()) 
				current = next_sibling;
			else current = next_unenumerated_sibling_of_ancestor(parent);
		}
	}
}


//-----------------------------------------------
// set current to the next node in the postorder
// enumeration of the tree with root "root" 
//-----------------------------------------------
void NonUniformDegreeTreeIterator::post_order_next()
{
	if (current->ChildCount() != 0) current = current->FirstChild();
	else {
		NonUniformDegreeTreeNode *parent = current->Parent();
		if (parent == 0) current = 0; // can't have siblings
		else {
			// ---- look for siblings in circular list -----
			NonUniformDegreeTreeNode *next_sibling = current->NextSibling();
			if (next_sibling != parent->FirstChild()) 
				current = leftmost_deepest_child(next_sibling);
			else current = parent;
		}
	}
}
