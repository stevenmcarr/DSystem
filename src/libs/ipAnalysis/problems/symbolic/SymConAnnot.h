/* $Id: SymConAnnot.h,v 1.1 1997/03/11 14:35:19 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *  Forward propagation of symbolic information -- that is, symbolic 
 *  constraints among for variables *passed* at a call site.
 *						paco Aug 1993
 */

#ifndef SymConAnnot_h
#define SymConAnnot_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef CallGraphAnnot_h
#include <libs/ipAnalysis/callGraph/CallGraphAnnot.h>
#endif

#include <libs/ipAnalysis/problems/symbolic/SymConstraints.h>

class CallGraph;              // external declaration
class CallGraphNode;          // external declaration
class OrderedSetOfStrings;    // external declaration
class ValIP;		      // external declaration

//  Item in constraint tree (the tree is a vector of these)
//
class ConstraintItem {
  public:
    ConstraintItem() : parent(UNUSED), myCoeff(0), pCoeff(0), addend(0) {};
    ConstraintItem(int par, int my, int pc, int add) : 
	parent(par), myCoeff(my), pCoeff(pc), addend(add) {};
    int parent;
    int myCoeff;
    int pCoeff;
    int addend;
};

class SymConAnnot : public CallGraphAnnot {
    struct ConstraintItem *hidden;
    friend SymConAnnot *SymFwdMerge(CallGraphNode *node);
    friend void SymFwdEdge(CallGraphNode *node);
    friend ValIP *SymGetEntryVals(CallGraphNode *node);
public:
    // create an SymCon annotation 
    // SymConAnnot(SymConAnnot &rhs);
    SymConAnnot(CallGraphNode *node);
    SymConAnnot(CallGraphEdge *edge);
    SymConAnnot();
  
    ~SymConAnnot();
  
    // copy a derived annotation
    CallGraphAnnot *Clone();
  
    // create new annotations
    CallGraphAnnot *New(CallGraphNode *node);
  
    CallGraphAnnot *DemandAnnotation(CallGraphNode *node);
  
    // generate printable version of the annotation
    OrderedSetOfStrings *CreateOrderedSetOfStrings(unsigned int width);
  
    // upcall to read an annotation
    int ReadUpCall(FormattedFile &port);

    // upcall to write an annotation
    int WriteUpCall(FormattedFile &port);

    // update function
  
    // query function

    // functions to access internal info
    //
    ConstraintItem *getConstraints();

    void Dump(CallGraphNode *node);
};

extern char *SYMCON_ANNOT;

#endif /* SymConAnnot_h */
