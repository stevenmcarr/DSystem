/* $Id: FormalProcedureBindings.C,v 1.1 1997/03/11 14:34:35 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/ipAnalysis/callGraph/FormalProcedureBindings.i>

//*************************************************************************
//                  Bindings Abstraction
//*************************************************************************

class ProcParBinding : public NamedObjectIO {
private:
  int NamedObjectReadUpCall(FormattedFile *) { return 0; };
  int NamedObjectWriteUpCall(FormattedFile *) { return 0; };
public:
  ProcParBinding(const char *pname = 0) : NamedObjectIO(pname) { }; 

  CLASS_NAME_FDEF(ProcParBinding);
};


CLASS_NAME_IMPL(ProcParBinding);


class ProcParBindingsSet : private NamedObjectTableIO {
public:
  ~ProcParBindingsSet() {
    NamedObjectTable::Destroy();
  };

  CLASS_NAME_FDEF(ProcParBindingsSet);

  // add proc name to a bindings set for a procedure variable 
  void Bind(const char *procName) { AddEntry(new ProcParBinding(procName)); };
  Boolean ContainsBinding(const char *procName) {
    return (QueryEntry(procName) ? true : false);
  };

  int Write(FormattedFile *file) { return NamedObjectTableWrite(file);};
  int Read(FormattedFile *file) { return NamedObjectTableRead(file);};
  NamedObjectIO *NewEntry() { return new ProcParBinding(); };

  void Dump() { NamedObjectTableDump(); };
friend class ProcParBindingSetIterator;
};


CLASS_NAME_IMPL(ProcParBindingsSet);


struct ProcParBindingsIteratorS {
  ProcParBindingsIteratorS(NamedObjectTable *t) : iterator(t) {};
  NamedObjectTableIterator iterator;
};


ProcParBindingsIterator::ProcParBindingsIterator(ProcParBindingsSet *s)
{
  hidden = new ProcParBindingsIteratorS((NamedObjectTable *) s);
}


ProcParBindingsIterator::~ProcParBindingsIterator()
{
  delete hidden; 
}


const char *ProcParBindingsIterator::Current()
{
  NamedObject *e = hidden->iterator.Current();
  return (e ? e->name : 0);
}

void ProcParBindingsIterator::Reset()
{
  hidden->iterator.Reset();
}

void ProcParBindingsIterator::operator++()
{
  (hidden->iterator)++;
}

class FormalParameter : public NamedObjectIO {
private:
  int NamedObjectReadUpCall(FormattedFile *file) { 
    return procParBindings.Read(file); };
  int NamedObjectWriteUpCall(FormattedFile *file) { 
    return procParBindings.Write(file); };
  void NamedObjectDumpUpCall() { procParBindings.Dump(); };	
public:
  // data
  ProcParBindingsSet procParBindings;

  // constructor
  FormalParameter(const char *_name) : NamedObjectIO(_name) { }; 
  ~FormalParameter() {
  };

  CLASS_NAME_FDEF(FormalParameter);
};


CLASS_NAME_IMPL(FormalParameter);


class FormalParameterSet : private NamedObjectTableIO {
public:
  ~FormalParameterSet() {
    NamedObjectTable::Destroy();
  };
  CLASS_NAME_FDEF(FormalParameterSet);

  Boolean IsFormal(const char *theName) { return QueryEntry(theName)?true:false; };

  const char *Name(uint position);
  int Position(const char *theName) { return GetEntryIndex(theName); };

  ProcParBindingsSet *Bindings(const char *theName);

  NamedObjectTable::AddEntry;
  FormalParameter *QueryEntry(const char *formal) {
    return (FormalParameter *) NamedObjectTable::QueryEntry(formal);
  };

  int Write(FormattedFile *file) { return NamedObjectTableWrite(file); };
  int Read(FormattedFile *file) { return NamedObjectTableRead(file); };
  void Dump() { NamedObjectTable::NamedObjectTableDump(); };
private:
  NamedObjectIO *NewEntry() { return new FormalParameter(0); };
};

CLASS_NAME_IMPL(FormalParameterSet);


const char *FormalParameterSet::Name(uint position)
{
  FormalParameter *p = (FormalParameter *) GetEntryByIndex(position);
  return p->name;
}


