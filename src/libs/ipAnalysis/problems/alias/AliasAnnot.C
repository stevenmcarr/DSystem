/* $Id: AliasAnnot.C,v 1.2 1997/03/27 20:40:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//*****************************************************************************
// AliasAnnot.C
//
// representation of flow insensitive alias pairs for a call site or procedure
//
// Author: John Mellor-Crummey                                March 1994
//
// Copyright 1994, Rice University
//*****************************************************************************

#include <stdio.h>
#include <assert.h>

#include <libs/support/strings/OrderedSetOfStrings.h>
#include <libs/support/strings/StringBuffer.h>

#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/callGraph/CallGraphAnnotReg.h>

#include <libs/ipAnalysis/problems/alias/AliasAnnot.h>
#include <libs/ipAnalysis/problems/alias/AliasDFProblem.i>


//*****************************************************************************
// class AliasAnnot interface operations
//*****************************************************************************

CLASS_NAME_IMPL(AliasAnnot);

//----------------------------------------------------------------------------
// constructors
//----------------------------------------------------------------------------


// constructor used to register dummy class instance in the
// CallGraph annotation registry
AliasAnnot::AliasAnnot() : DataFlowSet(ALIAS_ANNOT) 
{
}

// constructor used to register dummy class instance in the
// CallGraph annotation registry
AliasAnnot::AliasAnnot(const AliasAnnot *rhs) : DataFlowSet(ALIAS_ANNOT),
formalAliases(((AliasAnnot *)rhs)->formalAliases), 
globalAliases(((AliasAnnot *)rhs)->globalAliases)
{
}

AliasAnnot::~AliasAnnot() 
{
}

// copy a derived annotation
DataFlowSet *AliasAnnot::Clone() const
{
  return new AliasAnnot(this);
}


int AliasAnnot::ReadUpCall(FormattedFile *file)
{
  return formalAliases.Read(file) || globalAliases.Read(file);
}

int AliasAnnot::WriteUpCall(FormattedFile *file)
{
  return formalAliases.Write(file) || globalAliases.Write(file);
}


const char *CONT_FORMAT =  ", %s";
const char *INIT_FORMAT = "%s";
const char *FIRST_PAIR = "%d..%d";
const char *NEXT_PAIR =  ",%d..%d";

// generate printable version of the annotation
OrderedSetOfStrings *AliasAnnot::CreateOrderedSetOfStrings()
{
  OrderedSetOfStrings *oss = new OrderedSetOfStrings(true);

  FormalAliasesSetIterator fi(&formalAliases);
  FormalAliases *fa;
  for (; fa = fi.Current(); ++fi) {
    StringBuffer string(80); // large enough initial length so few reallocs
    string.Append("%s: ",fa->name);
    StringSetIterator si(fa);
    const char *s;
    const char *format = INIT_FORMAT;
    for (; s = si.Current(); ++si) {
      string.Append(format, s);
      format = CONT_FORMAT;
    }
    EqClassPairSetIterator eqs(fa);
    EqClassPairs *eqp;
    if (format == CONT_FORMAT && eqs.Current())
      string.Append(", ");
    for (; eqp = eqs.Current(); ++eqs) {
      string.Append("%s[", eqp->name);
      format = FIRST_PAIR;
      unsigned int npairs = eqp->NumberOfEntries();
      for (int i=0; i < npairs; i++) {
	OffsetLengthPair *olp = eqp->GetPair(i);
	string.Append(format, olp->offset, olp->offset + olp->length);
	format = NEXT_PAIR;
      }
      string.Append("]");
    }
    oss->Append(string.Finalize());
  }

  if (oss->NumberOfEntries() == 0) {
    oss->Append(ssave("no aliases present"));
  }

  return oss;
}


void AliasAnnot::Add(const char *formal1, const char *formal2)
{
  FormalAliases *fa;

  fa = formalAliases.GetEntry(formal1);
  fa->Add(formal2);

  fa = formalAliases.GetEntry(formal2);
  fa->Add(formal1);
}


void AliasAnnot::operator |=(const DataFlowSet &rhs)
{
  AliasAnnot *arhs = (AliasAnnot *) &rhs;
  formalAliases |= arhs->formalAliases;
  globalAliases |= arhs->globalAliases;
}


int AliasAnnot::operator ==(const DataFlowSet &rhs) const
{
  AliasAnnot *arhs = (AliasAnnot *) &rhs;
  return ((formalAliases == arhs->formalAliases) && 
	  (globalAliases == arhs->globalAliases));
}


void AliasAnnot::Add(const char *formal, const char *global, 
		     int offset, int length)
{
  //----------------------------------
  // add global/formal alias pair
  //----------------------------------
  GlobalAliases *ga = globalAliases.GetEntry(global, offset, length);
  ga->Add(formal);
  
  //----------------------------------
  // add formal/global alias pair
  //----------------------------------
  FormalAliases *fa = formalAliases.GetEntry(formal);
  fa->AddGlobalAlias(global, offset, length);
}


GlobalAliases *AliasAnnot::FindAliasesGlobal
(const char *_name, int offset, int length) 
{
  return globalAliases.QueryEntry(_name, offset, length);
}


FormalAliases *AliasAnnot::FindAliasesFormal(const char *_name) 
{
  return formalAliases.QueryEntry(_name);
}


Boolean AliasAnnot::AliasedGlobal(const char *_name, int offset, int length) 
{
  return (globalAliases.QueryEntry(_name, offset, length) ? true : false);
}


Boolean AliasAnnot::AliasedFormal(const char *_name) 
{
  return (formalAliases.QueryEntry(_name) ? true : false);
}

//*****************************************************************************
// class FormalAliasesIterator interface operations
//*****************************************************************************

FormalAliasesIterator::FormalAliasesIterator(AliasAnnot *annot) :
FormalAliasesSetIterator(&annot->formalAliases)
{
}


FormalAliasesIterator::~FormalAliasesIterator()
{
}


//*****************************************************************************
// class GlobalAliasesIterator interface operations
//*****************************************************************************

GlobalAliasesIterator::GlobalAliasesIterator(AliasAnnot *annot) :
GlobalAliasesSetIterator(&annot->globalAliases)
{
}


GlobalAliasesIterator::~GlobalAliasesIterator()
{
}
