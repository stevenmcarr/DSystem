/* $Id: AliasAnnot.h,v 1.1 1997/03/11 14:34:53 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef AliasAnnot_h
#define AliasAnnot_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef DataFlowSet_h
#include <libs/support/sets/DataFlowSet.h>
#endif

#include <libs/ipAnalysis/problems/alias/AliasSets.h>

#include <libs/support/tables/namedObject/NamedObjectTable.h>


class FormattedFile;          // external declaration
class OrderedSetOfStrings;    // external declaration

class AliasAnnot : public DataFlowSet {
  FormalAliasesSet formalAliases;
  GlobalAliasesSet globalAliases;
public:
  // create an alias annotation 
  AliasAnnot(const AliasAnnot *rhs);           // used by the Clone method
  AliasAnnot();                       
  ~AliasAnnot();                       
  
  // copy a derived annotation
  DataFlowSet *Clone() const;
  
  // I/O upcalls
  int ReadUpCall(FormattedFile *file);
  int WriteUpCall(FormattedFile *file);

  int operator==(const DataFlowSet &rhs) const;
  void operator|=(const DataFlowSet &rhs);

  void Add(const char *formal1, const char *formal2);
  void Add(const char *formal, const char *global, int offset, int length);
  
  // query functions
  GlobalAliases *AliasAnnot::FindAliasesGlobal(const char *_name, int offset, 
					       int length); 
  FormalAliases *AliasAnnot::FindAliasesFormal(const char *_name); 

  Boolean AliasAnnot::AliasedGlobal(const char *_name, int offset, int length);
  Boolean AliasAnnot::AliasedFormal(const char *_name); 

  // generate printable version of the annotation
  OrderedSetOfStrings *CreateOrderedSetOfStrings();

  CLASS_NAME_FDEF(AliasAnnot);

friend class FormalAliasesIterator;
friend class GlobalAliasesIterator;
};


class FormalAliasesIterator : private FormalAliasesSetIterator {
public:
  FormalAliasesIterator(AliasAnnot *annot);
  ~FormalAliasesIterator();

  FormalAliasesSetIterator::Current;
  FormalAliasesSetIterator::operator++;
  FormalAliasesSetIterator::Reset;
};


class GlobalAliasesIterator : private GlobalAliasesSetIterator {
public:
  GlobalAliasesIterator(AliasAnnot *annot);
  ~GlobalAliasesIterator();

  GlobalAliasesSetIterator::Current;
  GlobalAliasesSetIterator::operator++;
  GlobalAliasesSetIterator::Reset;
};


extern char *ALIAS_ANNOT;

#endif /* AliasAnnot_h */
