/* $Id: CallGraphNodeSet.C,v 1.1 1997/03/11 14:34:33 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/ipAnalysis/callGraph/CallGraphNodeSet.h>

//*************************************************************************
//                  CallGraphNodeSet Abstraction
//*************************************************************************

//------------------------------------------------------------------------
// call graph node set constructor
//------------------------------------------------------------------------

CallGraphNodeSet::CallGraphNodeSet()
{
}

//------------------------------------------------------------------------
// call graph node set destructor
//------------------------------------------------------------------------

CallGraphNodeSet::~CallGraphNodeSet()
{
}

//------------------------------------------------------------------------
// call graph node set add a member
//------------------------------------------------------------------------

void CallGraphNodeSet::Add(CallGraphNode *node) 
{ 
   WordSet::Add((unsigned long) node); 
}

//------------------------------------------------------------------------
// call graph node set check for membership
//------------------------------------------------------------------------

int CallGraphNodeSet::IsMember(CallGraphNode *node) 
{
   return WordSet::IsMember((unsigned long) node);
}
