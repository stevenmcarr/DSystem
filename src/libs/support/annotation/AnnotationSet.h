/* $Id: AnnotationSet.h,v 1.1 1997/03/11 14:36:28 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef AnnotationSet_h
#define AnnotationSet_h

//*************************************************************************** 
// AnnotationSet
//                                                                          
//   can be used to store (by reference) anything derived from the 
//   Annotation base class.
//                                                                          
// Author:  John Mellor-Crummey                            July 1994
//
// Copyright 1994, Rice University
//                                                                          
//***************************************************************************


#ifndef NamedObjectTable_h
#include <libs/support/tables/namedObject/NamedObjectTable.h>
#endif


class Annotation;             // minimal external declaration
class FormattedFile;          // minimal external declaration
class ClassInstanceRegistry;  // minimal external declaration



//***************************************************************************
// class AnnotationSet
//***************************************************************************
class AnnotationSet : private NamedObjectTable {
public:
  AnnotationSet();                        
  ~AnnotationSet();                         // free all annotations 

  CLASS_NAME_FDEF(AnnotationSet);
  
  NamedObjectTable::AddEntry;                 // add an item to the table
  NamedObjectTable::DeleteEntry;              // delete matching item 
  Annotation *QueryEntry(const char *name);   // get item matching name, if any

  NamedObjectTable::NumberOfEntries;

  // I/O
  int Write(FormattedFile *file, ClassInstanceRegistry *registry);
  int Read(FormattedFile *file, ClassInstanceRegistry *registry);

  void Dump();
friend class AnnotationSetIterator;
};



//***************************************************************************
// class AnnotationSetIterator
//***************************************************************************
class AnnotationSetIterator : private NamedObjectTableIterator {
public:
  AnnotationSetIterator(AnnotationSet *theTable);
  Annotation *Current();
  NamedObjectTableIterator::operator++;
  NamedObjectTableIterator::Reset;
};


#endif /* AnnotationSet_h */
