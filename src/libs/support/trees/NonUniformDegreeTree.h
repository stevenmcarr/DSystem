/* $Id: NonUniformDegreeTree.h,v 1.7 1997/06/25 15:21:23 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef nudtree_h
#define nudtree_h

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
// class NonUniformDegreeTreeNode
//    a non-uniform degree tree abstraction
//-----------------------------------------------
class NonUniformDegreeTreeNode {
	NonUniformDegreeTreeNode *parent, *children, *next_sibling, *prev_sibling;
	int child_count;
public:

	NonUniformDegreeTreeNode(NonUniformDegreeTreeNode *parent = 0);

	// destructor for destroying derived instances
	virtual ~NonUniformDegreeTreeNode();

	// link/unlink a node to a parent and siblings 
	void link(NonUniformDegreeTreeNode *parent);
	void unlink();

	// functions for inspecting links to other nodes
	unsigned int ChildCount() { return child_count; };
	NonUniformDegreeTreeNode *NextSibling() { return next_sibling; };
	NonUniformDegreeTreeNode *PrevSibling() { return prev_sibling; };
	NonUniformDegreeTreeNode *FirstChild() { return children; };
	NonUniformDegreeTreeNode *Parent() { return parent; };

};


// type of iterator enumeration requested
typedef enum {PREORDER, POSTORDER} NonUniformDegreeTreeIteratorType;

//-----------------------------------------------
// class NonUniformDegreeTreeIterator
//    an abstraction of an iterator that 
//    enumerates all nodes in a non-uniform 
//     degree tree 
//-----------------------------------------------
class NonUniformDegreeTreeIterator {
	NonUniformDegreeTreeNode *root, *current;
	void (NonUniformDegreeTreeIterator::*next)(); 

	void post_order_next();
	void pre_order_next();
public:
	// initialize an iterator for a non-uniform degree tree
	// and an enumeration order 
	void reset(NonUniformDegreeTreeNode *tree_root,
		NonUniformDegreeTreeIteratorType type = PREORDER);  

	// constructor that defines an iterator for a non-uniform degree 
	// tree and an enumeration order
    NonUniformDegreeTreeIterator(NonUniformDegreeTreeNode *tree_root, 
			NonUniformDegreeTreeIteratorType type = PREORDER) {
		reset(tree_root, type);
	}; 

	// advance the enumeration to the next node
    void operator ++() { if (current != 0) (this->*next)(); };

	// inspect the current node in the enumeration 
	NonUniformDegreeTreeNode *Current() {
        return current;
	};
};

#endif 
