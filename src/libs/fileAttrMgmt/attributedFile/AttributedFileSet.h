/* $Id: AttributedFileSet.h,v 1.2 1997/03/27 20:31:09 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef AttributedFileSet_h
#define AttributedFileSet_h

//******************************************************************
// AttributedFileSet.h: 
//
// Author: 
//   John Mellor-Crummey                              October 1993
//
// Copyright 1993, Rice University
//    
//******************************************************************


#ifndef StackableIterator_h
#include <libs/support/iterators/StackableIterator.h>
#endif
#include <libs/support/tables/namedObject/NamedObject.h>
#include <libs/support/tables/namedObject/NamedObjectTable.h>

class AttributedFile;            // minimal external declaration
class FileSuffixRegistry;        // minimal external declaration

//***************************************************************************
// interface operations for class AttributedFileSet
//***************************************************************************

struct AttributedFileSetS {
  NamedObjectTable ht;
};

//--------------------------------------------------------------------
// class AttributedFileSet
//--------------------------------------------------------------------
class AttributedFileSet {
private:
  struct AttributedFileSetS *attrFileSetRepr;
public:
  virtual FileSuffixRegistry *GetFileSuffixRegistry() = 0;

  AttributedFileSet();
  virtual ~AttributedFileSet();

  AttributedFile *GetFile(const char *name);

  AttributedFile *OpenFile(const char *name);
  void CloseFile(AttributedFile *mod);
friend class AttributedFileSetIterator;
};


//***************************************************************************
// interface operations for class AttributedFileSetIterator
//***************************************************************************


class AttributedFileSetIteratorS {
public:
  NamedObjectTableIterator noti;
  AttributedFileSetIteratorS(NamedObjectTable *ht) : noti(ht) { ; };
};



//--------------------------------------------------------------------
// class AttributedFileSetIterator
//--------------------------------------------------------------------
class AttributedFileSetIterator : public StackableIterator {
private:
  struct AttributedFileSetIteratorS *attrFileSetIterRepr;
public:
  AttributedFileSetIterator(const AttributedFileSet *attrFileSet);
  ~AttributedFileSetIterator();
  
  void operator++();
  void Reset();
  AttributedFile *Current() const;
  void *CurrentUpCall() const;
};


#endif /* AttributedFileSet_h */

