/* $Id: ionudtree.C,v 1.8 1997/03/11 14:34:49 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/support/misc/general.h>

//-----------------------------------------------------
// include of stdio must follow include of general.h
// so that the ParaScope c_varieties file is used
//-----------------------------------------------------
#include <stdio.h>
#include <assert.h>

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

#include <libs/support/file/FormattedFile.h>
#include <libs/ipAnalysis/ipInfo/ionudtree.h>


//-----------------------------------------------
// constructor
//-----------------------------------------------
NonUniformDegreeTreeNodeWithDBIO::NonUniformDegreeTreeNodeWithDBIO
(NonUniformDegreeTreeNodeWithDBIO* new_parent) : 
NonUniformDegreeTreeNode(new_parent) 
{
}

//-----------------------------------------------
// virtual function to allocate a tree node of
// the same type as the node performing the
// allocation. this function should never
// be invoked since it serves only as a 
// placeholder for a virtual function in a 
// derived class
//-----------------------------------------------
NonUniformDegreeTreeNodeWithDBIO *
NonUniformDegreeTreeNodeWithDBIO::New(NonUniformDegreeTreeNodeWithDBIO *parent)
{
	assert(0);
	parent = parent; // bogus code to avoid unused argument warning
	return 0;
}


//-----------------------------------------------
// virtual function to write the subtree rooted at
// this node to a database port in an unambiguous
// format. a derived class should have its own 
// virtual function to write information unique to
// the derived node, then invoke the virtual write
// function for its base class
//-----------------------------------------------

int NonUniformDegreeTreeNodeWithDBIO::Write(FormattedFile& port)
{
  int count = ChildCount();
  if (port.Write(count) == EOF) return EOF;

  if (WriteUpCall(port)) return EOF;

  if (count > 0) {
    NonUniformDegreeTreeNodeWithDBIO *child =
      (NonUniformDegreeTreeNodeWithDBIO *) FirstChild();
    while (count-- != 0) {
      if (child->Write(port) == EOF) return EOF;
      child = (NonUniformDegreeTreeNodeWithDBIO *) child->NextSibling();
    }
  }
  return 0; // success
}

//-----------------------------------------------
// virtual function to read an unambiguous 
// representation of a tree from a database port 
// and reconstruct it. a derived class should have 
// its own virtual function to read information 
// unique to the derived node, then invoke the virtual 
// read  function for its base class
//-----------------------------------------------
int NonUniformDegreeTreeNodeWithDBIO::Read(FormattedFile& port)
{
  int i;
  if (port.Read(i) == EOF) return EOF;

  if (ReadUpCall(port)) return EOF;

  while (i-- > 0) {
    NonUniformDegreeTreeNodeWithDBIO *child = this->New(this);
    if (child->Read(port) == EOF) return EOF;
  }
  return 0; // success
}
