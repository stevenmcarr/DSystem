/* $Id: iptree.C,v 1.12 1997/03/11 14:34:49 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/ipAnalysis/ipInfo/iptree.h>

/******************************************************************
 * Procedure IP Information Abstraction        September 1991     *
 * Author: John Mellor-Crummey                                    *
 *                                                                *
 * this file contains definitions that support an external        *
 * representation of summary interprocedural information for an   *
 * entry. this information includes a parameter mapping and       *
 * mod/ref and callsite information at the loop level.            *
 *                                                                *
 * Copyright 1991, Rice University, as part of the ParaScope      *
 * Programming Environment Project                                *
 *                                                                *
 ******************************************************************/

#define IPINFOTREE_STRING "IPinfoTree: "
#define IPINFOTREENODE_STRING "IPinfoTreeNode: "




//-----------------------------------------------
// constructor -- allocate heap storage for
// pointers
//-----------------------------------------------
IPinfoTreeNode::IPinfoTreeNode (IPinfoTreeNode *parent,
				int code_block_id, CodeBlockType t) : 
				NonUniformDegreeTreeNodeWithDBIO(parent) 
{
  calls = new CallSites;
#if 0
  refs = new ModRefInfo;
#endif
  type = t;
  block_id = code_block_id;
#if 0
  annot_tab = NULL;
#endif
}
 


//-----------------------------------------------
// destructor that deletes heap allocated storage
// for a node
//-----------------------------------------------
IPinfoTreeNode::~IPinfoTreeNode()
{ 
#if 0
	delete refs;
#endif
	delete calls;
#if 0
        if (annot_tab != NULL) delete annot_tab;
#endif
}	


//-----------------------------------------------
// write an unambiguous representation of a 
// IPinfoTreeNode to a database port 
//
// Note: the return value of this routine is always 0 (i.e. success)
//       since we don't check the returns of the various component
//       writes Someone should probably change it so that it checks
//       for EOF when writing its subparts and returns EOF in this case.
//-----------------------------------------------
int IPinfoTreeNode::WriteUpCall(FormattedFile& port) 
{ 
  port.Write((int)type);
#if 0
  refs->Write(port);
#endif
  calls->Write(port);
  
  /* annotation table -- assume data-flow  */
  /*    problems take care of this.        */
  
  return 0; /* indicates that the write was successful */
}


//-----------------------------------------------
// read an unambiguous representation of a 
// IPinfoTreeNode from a database port 
//
// Note: the return value of this routine is always 0 (i.e. success)
//       since we don't check the returns of the various component
//       reads. Someone should probably change it so that it checks
//       for EOF when reading its subparts and returns EOF in this case.
//-----------------------------------------------
int IPinfoTreeNode::ReadUpCall(FormattedFile& port) 
{ 
  int temp;
  port.Read(temp);
  type = (CodeBlockType) temp;
#if 0
  refs->Read(port);
#endif
  calls->Read(port);
  
  /* annotation table -- assume data-flow  */
  /*    problems take care of this.        */
  
  return 0; /* indicates that the read was successful */
}


//-----------------------------------------------------
// derived function used by class 
// NonUniformDegreeTreeNodeWithDBIO to allocate storage 
// for an IPinfoTreeNode
//-----------------------------------------------------
NonUniformDegreeTreeNodeWithDBIO *
IPinfoTreeNode::New(NonUniformDegreeTreeNodeWithDBIO *parent)
{ 
	return new IPinfoTreeNode((IPinfoTreeNode *) parent); 
}



//-----------------------------------------------
// write an unambiguous representation of an
// IPinfoTree to a database port 
//-----------------------------------------------
int IPinfoTree::Write(FormattedFile& port) 
{ 
  port.Write(name, IP_NAME_STRING_LENGTH);
  port.Write(procedureIsProgram);
  plist->Write(port);
  tree->Write(port);
  
  return 0; // success!?!
}


//-----------------------------------------------
// read an unambiguous representation of an
// IPinfoTree from a database port 
//-----------------------------------------------
int IPinfoTree::Read(FormattedFile& port) 
{ 
  char buffer[IP_NAME_STRING_LENGTH];
  port.Read(buffer, IP_NAME_STRING_LENGTH);
  port.Read(procedureIsProgram);
  name = ssave(buffer);
  plist->Read(port);
  tree->Read(port);
  
  return 0; // success!?!
}


struct IPinfoTreeCallSiteIteratorS {
  IPinfoTreeNode *root;
  NonUniformDegreeTreeIterator *tree_iter;
  SinglyLinkedListIterator *call_list_iter;
};


#if 0
//*************************************************************************
//  IPinfoTreeCallSiteIterator: iterator for all callsites recorded in the
//      local information about a procedure
//
//  John Mellor-Crummey                                     November 1992
//
//*************************************************************************

IPinfoTreeCallSiteIterator::IPinfoTreeCallSiteIterator(IPinfoTree *t)
{
  Init(t);
}

IPinfoTreeCallSiteIterator::IPinfoTreeCallSiteIterator(IPinfoTree& t)
{
  Init(&t);
}

IPinfoTreeCallSiteIterator::~IPinfoTreeCallSiteIterator()
{
  delete repr->tree_iter;
  delete repr;
}



void IPinfoTreeCallSiteIterator::operator ++()
{
  if (repr->call_list_iter) {
    // currently iteratoring through a list of calls
    (*(repr->call_list_iter))++;
    if (repr->call_list_iter->Current() != 0) return; // found another callsite
    else { 
      // no more calls in this list, move to next node in the tree
      DeleteList();
      (*(repr->tree_iter))++;
    }
  } 
  // search through a tree for a node that has a list of calls
  for(;;) {
    IPinfoTreeNode *node = (IPinfoTreeNode *) repr->tree_iter->Current();
    if (node == 0) return; // no more tree nodes 
    if (node->calls && node->calls->Count() > 0) { 
      // found a list of callsites
      repr->call_list_iter = new SinglyLinkedListIterator(node->calls);
      return;
    }
    (*(repr->tree_iter))++;
  }
}

CallSite *IPinfoTreeCallSiteIterator::Current()
{
  return (repr->call_list_iter) ? 
    (CallSite *) repr->call_list_iter->Current() : 0;
}

void IPinfoTreeCallSiteIterator::Reset()
{
  repr->tree_iter->reset(repr->root);
  if (repr->call_list_iter) DeleteList();
}

void IPinfoTreeCallSiteIterator::DeleteList()
{
  delete repr->call_list_iter;
  repr->call_list_iter = 0; 
}

void IPinfoTreeCallSiteIterator::Init(IPinfoTree *t)
{
  repr = new struct IPinfoTreeCallSiteIteratorS;
  repr->root = t->tree;
  repr->tree_iter = new NonUniformDegreeTreeIterator(repr->root);
  repr->call_list_iter = 0;
  IPinfoTreeCallSiteIterator::operator++();
}
#endif



