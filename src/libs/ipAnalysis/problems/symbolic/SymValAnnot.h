/* $Id: SymValAnnot.h,v 1.3 1997/03/11 14:35:23 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *  Backwards propagation of symbolic information -- that is, symbolic 
 *  expressions/relations for variables *returned* at a call site.
 *						paco 18 May 93
 */

#ifndef SymValAnnot_h
#define SymValAnnot_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef CallGraphAnnot_h
#include <libs/ipAnalysis/callGraph/CallGraphAnnot.h>
#endif

class CallGraph;              // external declaration
class CallGraphNode;          // external declaration
class OrderedSetOfStrings;    // external declaration
class IpVarList;              // external declaration

typedef int ValNumber;

class SymValAnnot : public CallGraphAnnot {
    struct SymValAnnotS *hidden;
public:
    // create an SymVal annotation 
    SymValAnnot(SymValAnnot &rhs);
    SymValAnnot(CallGraphNode *node);
  
    ~SymValAnnot();
  
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
    struct ValIP *valIp();
    ValNumber getGlobal(ValNumber vn, ValNumber nameId, int varOffset);
    void addGlobal(ValNumber vn, ValNumber nameId, int varOffset, ValNumber nv);
    ValNumber &fwd(ValNumber ov);

    void encache(struct ValTable *newTab, ValNumber ov, ValNumber nv);
    ValNumber cached(struct ValTable *newTab, ValNumber ov);

    IpVarList *refScalars();

    void Dump();
};

extern char *SYMVAL_ANNOT;

#endif /* SymValAnnot_h */
