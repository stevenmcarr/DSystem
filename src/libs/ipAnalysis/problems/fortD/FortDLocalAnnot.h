/* $Id: FortDLocalAnnot.h,v 1.1 1997/03/11 14:34:58 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/* $Id */

#ifndef FortDLocalAnnot_h
#define FortDLocalAnnot_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#include <libs/ipAnalysis/callGraph/CallGraphAnnot.h>


class IPinfoTree;             // external declaration
class FormattedFile;          // external declaration
class CallGraph;              // external declaration
class CallGraphNode;          // external declaration
class CallGraphEdge;          // external declaration
class OrderedSetOfStrings;    // external declaration

class FortDLocalAnnot : public FlowGraphAnnot {
public:
  const IPinfoTree *tree;

  // create a FortD annotation 
  FortDLocalAnnot(IPinfoTree *tree = 0);
  
  ~FortDLocalAnnot();
  
  // upcall to read an annotation
  int ReadUpCall(FormattedFile *file);
  
  // upcall to write an annotation
  int WriteUpCall(FormattedFile *file);
  
  // generate printable version of the annotation
  OrderedSetOfStrings *CreateOrderedSetOfStrings();
};

extern char *FORTD_LOCAL_ANNOT;

#endif /* FortDLocalAnnot_h */
