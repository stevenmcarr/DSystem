/* $Id: ionudtree.h,v 1.10 1997/06/25 15:03:27 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef ionudtree_h
#define ionudtree_h

#include <libs/support/trees/NonUniformDegreeTree.h>

/******************************************************************
 * I/O of a Non-Uniform Degree Tree Abstraction  September 1991   *
 * Author: John Mellor-Crummey                                    *
 *                                                                *
 * this file contains support for reading and writing a general   *
 * purpose non-uniform degree tree abstraction. this abstraction  *
 * useless in its own right since tree nodes contain no           *
 * information other than what is needed to describe              *
 * the structure. to make use of this abstraction, derive a       *
 * tree node class that contains some useful data.  input and     *
 * output of structural information is performed using the        *
 * functions provided here. a client merely has to read/write the *
 * data contained in a tree node.                                 *
 *                                                                *
 * Copyright 1991, Rice University, as part of the ParaScope      *
 * Programming Environment Project                                *
 *                                                                *
 ******************************************************************/

class FormattedFile; // minimal external declaration

//-----------------------------------------------------
// class NonUniformDegreeTreeNodeWithDBIO
//    a derived class of NonUniformDegreeTreeNode 
//    that knows how to read and write itself from/to
//    database ports. this data structure is useful
//    only as a base class for structures that have 
//    useful information stored in the tree nodes. 
//-----------------------------------------------------

class NonUniformDegreeTreeNodeWithDBIO : 
public NonUniformDegreeTreeNode {
public:
  NonUniformDegreeTreeNodeWithDBIO
    (NonUniformDegreeTreeNodeWithDBIO* new_parent);
  
  // create new derived node
  virtual NonUniformDegreeTreeNodeWithDBIO *New
    (NonUniformDegreeTreeNodeWithDBIO *parent);
  
  // I/O of the entire structure
  int Read(FormattedFile& port);
  int Write(FormattedFile& port);

  // I/O of derived class data
  virtual int ReadUpCall(FormattedFile& port) = 0;
  virtual int WriteUpCall(FormattedFile& port) = 0;
};

#endif


