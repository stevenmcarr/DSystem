/* $Id: ScalarModRefAnnot.h,v 1.4 1997/03/11 14:35:08 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef ScalarModRefAnnot_h
#define ScalarModRefAnnot_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef CallGraphAnnot_h
#include <libs/ipAnalysis/callGraph/CallGraphAnnot.h>
#endif

#ifndef VariableSet_h
#include <libs/ipAnalysis/ipInfo/VariableSet.h>
#endif

#ifndef iptypes_h
#include <libs/ipAnalysis/ipInfo/iptypes.h>
#endif

class FormattedFile;          // external declaration
class CallGraphNode;          // external declaration
class CallGraphEdge;          // external declaration
class OrderedSetOfStrings;    // external declaration
class AliasAnnot;             // external declaration

class ScalarModRefAnnot : public FlowGraphDFAnnot, public VariableSet {
public:
  // create an ScalarModRef annotation 
  ScalarModRefAnnot(const char *const aname);  
  ScalarModRefAnnot(ScalarModRefAnnot& rhs, const char *const newName = 0);

  ~ScalarModRefAnnot(); // destructor
  
  int operator != (const DataFlowSet &) const;
  int operator == (const DataFlowSet &) const;

  ScalarModRefAnnot &operator = (ScalarModRefAnnot &);
  void operator |= (const DataFlowSet &);
  
  // copy a derived annotation
  FlowGraphDFAnnot *Clone() const;
  
  // upcall to read an annotation
  int ReadUpCall(FormattedFile *file);

  // upcall to write an annotation
  int WriteUpCall(FormattedFile *file);
  
  // generate printable version of the annotation
  OrderedSetOfStrings *CreateOrderedSetOfStrings();

#if 0
  void Insert(char *name, int offset = 0, int length = 
	      INFINITE_INTERVAL_LENGTH);
#endif

  void AugmentWithAliases(AliasAnnot *alias_annot);
  void AugmentCallerAnnotFromCalleeAnnot(CallGraphEdge *edge, 
					 ScalarModRefAnnot *calleeAnnot);

  // query functions
  Boolean ScalarModRefAnnot::IsMember(char *name, int offset = 0, 
				      int length = INFINITE_INTERVAL_LENGTH); 
friend class ScalarModRefAnnotIterator;
};

class ScalarModRefAnnotIterator {
  struct ScalarModRefAnnotIteratorS *hidden;
public:
  ScalarModRefAnnotIterator(ScalarModRefAnnot *annot);
  ~ScalarModRefAnnotIterator();
  char *Current(VarScope &scope, int& offset, int& length);
  void operator ++();
  void Reset();
};


// node annotations
extern char *SCALAR_GMOD_ANNOT;
extern char *SCALAR_GREF_ANNOT;

// edge annotations
extern char *SCALAR_MOD_ANNOT;
extern char *SCALAR_REF_ANNOT;

    
extern ScalarModRefAnnot *MapAcrossEdge(ScalarModRefAnnot *calleeAnnot, 
					CallGraphEdge *edge);

#endif /* ScalarModRefAnnot_h */
