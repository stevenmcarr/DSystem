/* $Id: AttributeTable.h,v 1.1 1997/03/11 14:27:43 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// AttributeTable.h
//
// Author: John Mellor-Crummey                                February 1994
//
// Copyright 1994, Rice University
//***************************************************************************

#ifndef  AttributeTable_h
#define  AttributeTable_h

#include <libs/support/tables/namedObject/NamedObjectTable.h>

class Attribute;


//-------------------------------------------------------------
// class AttributeTable
//-------------------------------------------------------------
class AttributeTable : private NamedObjectTable {
  uint referenceCount;
public:
  AttributeTable();
  ~AttributeTable();

  NamedObjectTable::AddEntry;       // used to add Attribute* to table
  NamedObjectTable::DeleteEntry;    // used to delete Attribute* from table

  void EntryCleanup(void *entry);   // used to delete Attribute* from table

  Attribute *QueryEntry(const char *attrName);

  void Attach();     // increment reference count
  uint Detach();     // decrement and return reference count

  // debugging support
  void Dump();
  
friend class AttributeTableIterator;
};


//-------------------------------------------------------------
// class AttributeTableIterator
//-------------------------------------------------------------
class AttributeTableIterator : private NamedObjectTableIterator {
public:
  AttributeTableIterator(AttributeTable *theTable);
  Attribute *Current();
  NamedObjectTableIterator::operator++;
  NamedObjectTableIterator::Reset;
};

#endif
