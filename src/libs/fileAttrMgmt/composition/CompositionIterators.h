/* $Id: CompositionIterators.h,v 1.1 1997/03/11 14:27:48 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef CompositionIterators_h
#define CompositionIterators_h

//******************************************************************
// CompositionIterators.h: 
//
//  iterators to enumerate the components or modules of a Composition
//
// Author: 
//   John Mellor-Crummey                              December 1993
//
// Copyright 1993, Rice University
//******************************************************************


#ifndef IteratorStack_h
#include <libs/support/iterators/IteratorStack.h>
#endif

#ifndef AttributedFileSet_h
#include <libs/fileAttrMgmt/attributedFile/AttributedFileSet.h>
#endif

class Composition;               // minimal external declaration
class Module;                    // minimal external declaration
class AttributedFile;            // minimal external declaration


//--------------------------------------------------------------------
// class CompComponentsIterator
//--------------------------------------------------------------------
class CompComponentsIterator : public AttributedFileSetIterator {
public:
  CompComponentsIterator(const Composition *comp);
  ~CompComponentsIterator();
}; 
#if 0
public StackableIterator {
private:
  struct CompComponentsIteratorS *hidden;
public:
  CompComponentsIterator(const Composition *pgm);
  ~CompComponentsIterator();
  
  void operator++();
  void Reset();
  AttributedFile *Current() const;
  void *CurrentUpCall() const;
};
#endif


//--------------------------------------------------------------------
// class CompositionModulesIterator
//--------------------------------------------------------------------
class CompModulesIterator : public IteratorStack {
public:
  CompModulesIterator(const Composition *pgm);
  ~CompModulesIterator();

  Module *Current() const;
  void *CurrentUpCall() const;
  StackableIterator *IteratorToPushIfAny(void *current);
};


#endif
