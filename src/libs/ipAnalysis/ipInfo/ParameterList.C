/* $Id: ParameterList.C,v 1.2 1997/03/27 20:40:38 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// ParameterList.C
//
// Author: John Mellor-Crummey                September 1993
//
// Copyright 1993, Rice University
//***************************************************************************


#include <libs/support/strings/rn_string.h>
#include <libs/support/strings/StringIO.h>
#include <libs/support/file/FormattedFile.h>

#include <libs/ipAnalysis/ipInfo/ParameterList.h>


//***************************************************************************
// class ParameterListEntry interface operations
//***************************************************************************

ParameterListEntry::ParameterListEntry(char *_pname, int _ptype) :
pname(_pname ? ssave(_pname) : 0), ptype(_ptype)
{ 
}


ParameterListEntry::ParameterListEntry(ParameterListEntry *rhs) :
pname(rhs->pname ? ssave(rhs->pname) : 0), ptype(rhs->ptype)
{ 
}


//-----------------------------------------------------
// free all storage for pointers used in a
// ParameterListEntry
//-----------------------------------------------------
ParameterListEntry::~ParameterListEntry()
{
  if (pname) sfree(pname);
}


int ParameterListEntry::type() 
{ 
  return ptype; 
}


char *ParameterListEntry::name() 
{ 
  return pname; 
}

//-----------------------------------------------
// read an unambiguous representation of a 
// ParameterListEntry from a database port 
//-----------------------------------------------
int ParameterListEntry::ReadUpCall(FormattedFile& port)
{
  return ReadString(&pname, &port) || port.Read(ptype);
}


//-----------------------------------------------
// write an unambiguous representation of a 
// ParameterListEntry to a database port 
//-----------------------------------------------
int ParameterListEntry::WriteUpCall(FormattedFile& port)
{
  return WriteString(pname, &port) || port.Write(ptype);
}


//***************************************************************************
// class ParameterList interface operations
//***************************************************************************

ParameterList::ParameterList() : current(0) 
{ 
}


ParameterList::ParameterList(ParameterList *rhs) : current(0) 
{ 
  SinglyLinkedListIterator entries(rhs);
  ParameterListEntry *entry;
  for (; entry = (ParameterListEntry *) entries.Current(); ++entries) {
    Append(new ParameterListEntry(entry));
  }
}


//-----------------------------------------------------
// derived function used by class 
// SinglyLinkedListEntryIO to allocate storage 
// for an entry in a ParameterList
//-----------------------------------------------------
SinglyLinkedListEntryIO *
ParameterList::NewEntry()
{
  return new ParameterListEntry;
}


ParameterList::~ParameterList()
{
}
