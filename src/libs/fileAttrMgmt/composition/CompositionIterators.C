/* $Id: CompositionIterators.C,v 1.1 1997/03/11 14:27:48 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//******************************************************************
// CompositionIterators.C: 
//
// Author: 
//   John Mellor-Crummey                              December 1993
//
// Copyright 1993, Rice University
//    
//******************************************************************

#include <stdlib.h>
#include <assert.h>

#include <libs/support/tables/HashTable.h>
#include <libs/support/tables/namedObject/NamedObject.h>
#include <libs/support/tables/namedObject/NamedObjectTable.h>

#include <libs/fileAttrMgmt/attributedFile/AttributedFileSet.h>

#include <libs/fileAttrMgmt/composition/Composition.h>
#include <libs/fileAttrMgmt/composition/CompositionIterators.h>


//***************************************************************************
// interface operations for class CompComponentsIterator
//***************************************************************************

CompComponentsIterator::CompComponentsIterator
(const Composition *comp) : 
AttributedFileSetIterator((AttributedFileSet *) comp)
{
}


CompComponentsIterator::~CompComponentsIterator()
{
}

#if 0
struct CompComponentsIteratorS {
  AttributedFileSetIterator afsi;
  CompComponentsIteratorS(const AttributedFileSet *afs) : afsi(afs) { ; };
};


CompComponentsIterator::CompComponentsIterator
(const Composition *comp) : 
hidden(new CompComponentsIteratorS(&comp->components))
{
}


CompComponentsIterator::~CompComponentsIterator()
{
  delete hidden;
}


AttributedFile *CompComponentsIterator::Current() const
{
  return hidden->afsi.Current();
}

void *CompComponentsIterator::CurrentUpCall() const
{
  return Current();
}


void CompComponentsIterator::operator++() 
{
  hidden->afsi++;
}


void CompComponentsIterator::Reset() 
{
  hidden->afsi.Reset();
}

#endif


//***************************************************************************
// interface operations for class CompModulesIterator
//***************************************************************************


CompModulesIterator::CompModulesIterator(const Composition *comp) : 
IteratorStack(PostOrder, ITER_STACK_ENUM_LEAVES_ONLY)
{
  Push(new CompComponentsIterator(comp));
}

CompModulesIterator::~CompModulesIterator()
{
}


Module *CompModulesIterator::Current() const
{
  return (Module *) IteratorStack::CurrentUpCall();
}

void *CompModulesIterator::CurrentUpCall() const
{
  return Current();
}

StackableIterator *CompModulesIterator::IteratorToPushIfAny(void *current)
{
  return 
    ((((AttributedFile *) current)->ClassName() == 
      CLASS_NAME(Composition)) ?
     new CompComponentsIterator((Composition *) current) : 0);
}
