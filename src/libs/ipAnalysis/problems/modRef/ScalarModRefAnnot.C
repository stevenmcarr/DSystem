/* $Id: ScalarModRefAnnot.C,v 1.7 1997/03/27 20:41:27 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <stdlib.h>
#include <assert.h>

#include <libs/support/strings/OrderedSetOfStrings.h>
#include <libs/support/strings/StringBuffer.h>

#include <libs/ipAnalysis/ipInfo/VariableSet.h>

#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/callGraph/CallGraphAnnotReg.h>

#include <libs/ipAnalysis/problems/alias/AliasAnnot.h>

#include <libs/ipAnalysis/problems/modRef/ScalarModRefDFProblem.i>
#include <libs/ipAnalysis/problems/modRef/ScalarModRefAnnot.h>


//****************************************************************************
// class ScalarModRefAnnot
//****************************************************************************


ScalarModRefAnnot::ScalarModRefAnnot(const char *const aname) : 
FlowGraphDFAnnot(aname)
{
}


ScalarModRefAnnot::ScalarModRefAnnot(ScalarModRefAnnot &rhs,
				     const char *const newName) :
FlowGraphDFAnnot(newName ? newName : rhs.name), VariableSet(rhs)
{
}


ScalarModRefAnnot::~ScalarModRefAnnot()
{
}


void ScalarModRefAnnot::operator |= (const DataFlowSet &rhs)
{
  ScalarModRefAnnot *smr = (ScalarModRefAnnot *) &rhs;
  *((VariableSet *) this) |= *((VariableSet *) smr);
}

int ScalarModRefAnnot::operator != (const DataFlowSet &rhs) const
{
  return !(*this == rhs);
}

int ScalarModRefAnnot::operator == (const DataFlowSet &rhs) const
{
  ScalarModRefAnnot *smr = (ScalarModRefAnnot *) &rhs;
  return *((VariableSet *) this) == *((VariableSet *) smr);
}

FlowGraphDFAnnot *ScalarModRefAnnot::Clone() const
{
  ScalarModRefAnnot *copy = new ScalarModRefAnnot(*(ScalarModRefAnnot *)this);
  return copy;
}


static void PrintEqClassSet(StringBuffer *string, unsigned int width,
			    EqClassPairSet *set, const char *header, 
			    OrderedSetOfStrings *oss)
{
  string->Append(header);

  int nonempty = 0;

  EqClassPairSetIterator eqclasses(set);
  EqClassPairs *eqpairs;
  for (; eqpairs = eqclasses.Current(); ++eqclasses) {
    unsigned int npairs = eqpairs->NumberOfEntries();
    if (npairs > 0) {
      nonempty = 1;
      char *format = "%d..%d";
      string->Append(" %s[", eqpairs->name);
      for (unsigned int i = 0; i < npairs; i++) {
	OffsetLengthPair *pair = eqpairs->GetPair(i);
	string->Append(format, pair->offset, pair->offset + pair->length); 
	format = ",%d..%d";
      }
      string->Append("]");
    }
  }
  
  if (nonempty) oss->Append(string->Finalize());
  string->Reset(width);
}


OrderedSetOfStrings *ScalarModRefAnnot::CreateOrderedSetOfStrings()
{
  OrderedSetOfStrings *oss = new OrderedSetOfStrings(true);
  StringBuffer string(80); // initial width sizable to prevent many reallocs
  
  if (formals.NumberOfEntries() > 0) {
    string.Append("formals:");

    StringSetIterator strings(&formals);
    const char *formal;
    for (; formal = strings.Current(); ++strings) {
      string.Append(" %s", formal);
    }

    oss->Append(string.Finalize());
    string.Reset(80);
  }

  PrintEqClassSet(&string, 80, &globals, "globals:", oss);
  PrintEqClassSet(&string, 80, &locals, "locals:", oss);

  if (oss->NumberOfEntries() == 0) {
    oss->Append(ssave("no side effects"));
  }

  return oss;
}


int ScalarModRefAnnot::ReadUpCall(FormattedFile *file)
{
  return VariableSet::Read(file);
}


int ScalarModRefAnnot::WriteUpCall(FormattedFile *file)
{
  return VariableSet::Write(file);
}

    
void ScalarModRefAnnot::AugmentCallerAnnotFromCalleeAnnot
(CallGraphEdge *edge, ScalarModRefAnnot *calleeAnnot)
{
  ParamNameIterator fnames(edge->paramBindings, FormalNameSet);
  for(const char *formal; formal = fnames.Current(); ++fnames) {
    if(calleeAnnot->formals.IsMember(formal)) {  // formal is in callee GMOD/GREF
      ParamBinding *bind = edge->paramBindings.GetReverseBinding(formal);
      EqClassPairs *entry;
      switch(bind->a_class) {
      case APC_DataFormal:
	this->formals.Add(bind->actual);
	break;
      case APC_DataGlobal: 
	entry = this->globals.GetEntry(bind->actual);
	entry->AddPair(new OffsetLengthPair(bind->a_offset, bind->a_length));
	break;
      case APC_DataLocal:
	entry = this->locals.GetEntry(bind->actual);
	entry->AddPair(new OffsetLengthPair(bind->a_offset, bind->a_length));
	break;
      default:
	assert(0);
      }
    }
  } 
  this->globals |= calleeAnnot->globals; // meet global sets
}


void ScalarModRefAnnot::AugmentWithAliases(AliasAnnot *aliasAnnot)
{
  // create a copy of the set to iterate over so that we avoid
  // adding new elements to a set while iterating over its contents.
  ScalarModRefAnnot temp(*this);

  // integrate aliases into MOD/REF annotation
  StringSetIterator allFormals(&temp.formals);
  for (const char *formal; formal = allFormals.Current(); ++allFormals) {
    FormalAliases *faliases = aliasAnnot->FindAliasesFormal(name);
    if (faliases) {
      formals |= *faliases; // note all formals aliased to "formal"
      globals |= *faliases; // note all globals aliased to "formal"
    }
  }

  GlobalAliasesIterator gai(aliasAnnot);
  for (GlobalAliases *galiases; galiases = gai.Current(); ++gai) {
    // if global with aliases is in the MOD/REF set, add the aliases too
    EqClassPairs *mrpairs = globals.QueryEntry(galiases->name);
    if (mrpairs && mrpairs->Overlaps(galiases->globalInfo.offset,
				    galiases->globalInfo.length)) {
      formals |= *galiases;
    }
  }
}


