/* $Id: CallSiteParamBindings.C,v 1.4 1997/03/11 14:34:34 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <stdio.h>
#include <assert.h>

#include <libs/support/strings/rn_string.h>
#include <libs/support/file/FormattedFile.h>
#include <libs/support/lists/SinglyLinkedList.h>
#include <libs/support/misc/dict.h>
#include <libs/support/msgHandlers/DumpMsgHandler.h>

#include <libs/ipAnalysis/callGraph/CallSiteParamBindings.h>

class ParamBindingListEntry : public SinglyLinkedListEntry {
public:
  ParamBindingListEntry(const char *actual, int a_offset, int a_length, 
			    ActualParamClass a_class, /* VarScope a_scope, */
			    const char *formal);
  ~ParamBindingListEntry();
  ParamBinding binding;
};


ParamBindingListEntry::ParamBindingListEntry(const char *actual, int a_offset, 
					     int a_length, 
					     ActualParamClass a_class, 
					     /* VarScope a_scope, */ 
					     const char *formal)
{
  *((char **) &binding.actual)   = ssave(actual);
  binding.a_offset = a_offset;
  binding.a_length = a_length;
#if 0
  binding.a_scope  = a_scope;
#endif
  binding.a_class  = a_class;
  *((char **) &binding.formal)   = ssave(formal);
}

ParamBindingListEntry::~ParamBindingListEntry()
{
  sfree((char *) binding.actual);
  sfree((char *) binding.formal);
}

struct CallSiteParamBindingsS {
  CallSiteParamBindingsS() : forward(cmpstr, hashstr, 0), 
  reverse(cmpstr, hashstr, 0) {};
  Dict forward;
  Dict reverse;
};

CallSiteParamBindings::CallSiteParamBindings()
{
  hidden = new struct CallSiteParamBindingsS(); 
}

CallSiteParamBindings::~CallSiteParamBindings()
{
  for (DictI i(&(hidden->forward)); i.k; i++) {
    delete (SinglyLinkedList *) i.v;
  }
  delete hidden;
}

void CallSiteParamBindings::Bind(const char *actual, int a_offset, int a_length, 
			    ActualParamClass a_class, /* VarScope a_scope, */
			    const char *formal)
{
  // create binding entry
  ParamBindingListEntry *b = 
    new ParamBindingListEntry(actual, a_offset, a_length, a_class, /* a_scope, */
			      formal);

  // add reverse binding to map
  hidden->reverse.Insert(b->binding.formal, &b->binding);
  
  // add forward binding to map
  SinglyLinkedList *fset = (SinglyLinkedList *) (hidden->forward)[actual];
  if (fset == NULL) {
    fset = new SinglyLinkedList;
    hidden->forward.Insert(b->binding.actual, fset);
  }
  fset->Append(b);
}

ParamBindingsSet *CallSiteParamBindings::GetForwardBindings(const char *actual)
{
  return (ParamBindingsSet *) (hidden->forward)[actual];
}


ParamBinding *CallSiteParamBindings::GetReverseBinding(const char *formal)
{
  return (ParamBinding *) (hidden->reverse)[formal];
}

int CallSiteParamBindings::Read(FormattedFile *file) 
{
  int nbindings;
  int a_offset, a_length, /* a_scope, */ a_class;
  char actual[100], formal[100];

  file->Read(nbindings);           // dictionary size
  for(int i = 0; i < nbindings; i++ ) {

    int code = file->Read(actual, 100) ||
      file->Read(a_offset) || file->Read(a_length) || /* file->Read(a_scope) || */
      file->Read(a_class) || file->Read(formal, 100);
    if (code) return code;

    this->Bind(actual, a_offset, a_length, (ActualParamClass) a_class, 
	       /* (VarScope) a_scope, */ formal);
  }
  return 0; // success!
}


int CallSiteParamBindings::Write(FormattedFile *file) 
{
  // number of binding pairs is same as number of reverse
  // bindings
  file->Write((unsigned int) hidden->reverse.Size()); 
  for(DictI i(&hidden->reverse); i.k; i++) {
    ParamBinding *binding = (ParamBinding *) i.v;
    int code = file->Write(binding->actual, strlen(binding->actual)) ||
      file->Write(binding->a_offset) || file->Write(binding->a_length) ||
#if 0
      file->Write((int) binding->a_scope) ||
#endif
      file->Write((int) binding->a_class) ||
      file->Write(binding->formal, strlen(binding->formal));
    if (code) return code;
  }
  return 0; // success!
}

void CallSiteParamBindings::Dump() 
{
  dumpHandler.Dump("CallSiteParamBindings\n");
  dumpHandler.BeginScope();
  // number of binding pairs is same as number of reverse
  // bindings
  for(DictI i(&hidden->reverse); i.k; i++) {
    ParamBinding *binding = (ParamBinding *) i.v;
    dumpHandler.Dump("(%s[off=%d,len=%d,class=%d],%s)\n",
		     binding->actual, binding->a_offset, binding->a_length,
		     binding->a_class, binding->formal);
  }
  dumpHandler.EndScope();
}

struct ParamNameIteratorS {
  ParamNameIteratorS(Dict *d) : iterator(d) {};
  DictI iterator;
};


ParamNameIterator::ParamNameIterator(CallSiteParamBindings &b, ParamNameSet s)
{
  hidden = new struct ParamNameIteratorS(s == ActualNameSet ? 
					 &(b.hidden->forward) : 
					 &(b.hidden->reverse));
}


ParamNameIterator::~ParamNameIterator()
{
  delete hidden;
}


void ParamNameIterator::operator ++()
{
  (hidden->iterator)++;
}


void ParamNameIterator::Reset()
{
  hidden->iterator.reset();
}


const char *ParamNameIterator::Current()
{
  return (const char *) hidden->iterator.k;
}


struct ParamBindingsSetIteratorS {
  ParamBindingsSetIteratorS(ParamBindingsSet *s) : 
  iterator((SinglyLinkedList *) s) {};
  SinglyLinkedListIterator iterator;
};

ParamBindingsSetIterator::ParamBindingsSetIterator(ParamBindingsSet *s)
{
  hidden = new struct ParamBindingsSetIteratorS(s);
}

ParamBindingsSetIterator::~ParamBindingsSetIterator()
{
  delete hidden;
}


void ParamBindingsSetIterator::operator ++()
{
  (hidden->iterator)++;
}

void ParamBindingsSetIterator::Reset()
{
  hidden->iterator.Reset();
}

ParamBinding *ParamBindingsSetIterator::Current()
{
  ParamBindingListEntry *e = 
    (ParamBindingListEntry *) hidden->iterator.Current();
  return (e ? &e->binding : 0);
}

