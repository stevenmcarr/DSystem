/* $Id: AttributeTable.C,v 1.2 1997/03/27 20:31:09 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//*************************************************************************** 
// AttributeTable
//                                                                          
//   can be used to store (by reference) anything derived from the 
//   Attribute base class.
//                                                                          
// Author:  John Mellor-Crummey                            June 1993
//
// Copyright 1993, Rice University
//                                                                          
//***************************************************************************


#include <stdio.h>

#include <libs/fileAttrMgmt/attributedFile/AttributeTable.h>
#include <libs/fileAttrMgmt/attributedFile/AttributedFile.h>
#include <libs/fileAttrMgmt/attributedFile/Attribute.h>


//**********************************************************************
// implementation of class AttributeTable
//**********************************************************************

AttributeTable::AttributeTable()
{
  referenceCount = 0;
}


AttributeTable::~AttributeTable()
{
  AttributeTableIterator attributes(this);
  Attribute *attr;
  for (; attr = attributes.Current(); ++attributes) 
    attr->uplinkToFile->SaveAttribute(attr);

  this->Destroy();
}


Attribute *AttributeTable::QueryEntry(const char *name)
{
  return (Attribute *) NamedObjectTable::QueryEntry(name);
}


void AttributeTable::EntryCleanup(void *entry)
{
  Attribute *attr = (Attribute *) *(NamedObject **)entry;
  if (attr->RefCount() == 0) delete attr;
}
  

void AttributeTable::Attach()
{
  referenceCount++;
}


uint AttributeTable::Detach()
{
  return --referenceCount;
}


void AttributeTable::Dump()
{
   NamedObjectTable::NamedObjectTableDump();
}


//**********************************************************************
// implementation of class AttributeTableIterator
//**********************************************************************

AttributeTableIterator::AttributeTableIterator
(AttributeTable *theTable) : NamedObjectTableIterator(theTable)
{
}


Attribute *AttributeTableIterator::Current()
{
  return (Attribute *) NamedObjectTableIterator::Current();
}


