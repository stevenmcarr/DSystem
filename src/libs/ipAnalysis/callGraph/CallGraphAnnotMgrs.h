/* $Id: CallGraphAnnotMgrs.h,v 1.1 1997/03/11 14:34:30 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef CallGraphAnnotMgrs_h
#define CallGraphAnnotMgrs_h

//***************************************************************************
//
// CallGraphAnnotMgrs.h
//
// Author: John Mellor-Crummey                              June 1994
//
// Copyright 1994, Rice University
// 
//***************************************************************************

#ifndef AnnotationMgr_h
#include <libs/support/annotation/AnnotationMgr.h>
#endif

class CallGraph;
class CallGraphEdge;
class CallGraphNode;

//------------------------------------------------------------------------
// class CallGraphNodeAnnotMgr
//------------------------------------------------------------------------
class CallGraphNodeAnnotMgr : public AnnotationMgr { 
public:
  // realize an annotation for a particular CallGraphNode  
  virtual Annotation *Compute(CallGraphNode *node) = 0;
};


//------------------------------------------------------------------------
// class CallGraphEdgeAnnotMgr
//------------------------------------------------------------------------
class CallGraphEdgeAnnotMgr : public AnnotationMgr { 
public:
  // realize an annotation for a particular CallGraphEdge  
  virtual Annotation *Compute(CallGraphEdge *edge) = 0;
};


//------------------------------------------------------------------------
// class CallGraphGraphAnnotMgr
//------------------------------------------------------------------------
class CallGraphAnnotMgr : public AnnotationMgr { 
public:
  // realize an annotation for a particular CallGraph  
  virtual Annotation *Compute(CallGraph *graph) = 0;
};


#endif /* CallGraphAnnotMgrs_h */
