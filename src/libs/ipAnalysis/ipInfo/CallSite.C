/* $Id: CallSite.C,v 1.9 1997/03/11 14:34:38 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/******************************************************************
 * CallSite Abstraction                        September 1991     *
 * Author: John Mellor-Crummey                                    *
 *                                                                *
 * this file contains code that supports an external              *
 * representation of callsite summary information used for        *
 * interprocedural dataflow analysis                              *
 *                                                                *
 * Copyright 1991, Rice University, as part of the ParaScope      *
 * Programming Environment Project                                *
 *                                                                *
 ******************************************************************/

#include <libs/support/misc/general.h>
#include <libs/support/strings/rn_string.h>
#include <libs/support/strings/StringIO.h>
#include <libs/support/file/FormattedFile.h>

#include <libs/ipAnalysis/ipInfo/CallSite.h>

//-----------------------------------------------------
// free all storage for pointers used in a
// ActualListEntry
//-----------------------------------------------------
ActualListEntry::~ActualListEntry()
{
	if (pname) sfree(pname);
}


//-----------------------------------------------
// read an unambiguous representation of a 
// ActualListEntry from a database port 
//-----------------------------------------------
int ActualListEntry::ReadUpCall(FormattedFile& port)
{
  return ReadString(&pname, &port) ||
    port.Read(ptype) || port.Read(field[0]) ||
    port.Read(field[1]);
}


//-----------------------------------------------
// write an unambiguous representation of a 
// ActualListEntry to a database port 
//-----------------------------------------------
int ActualListEntry::WriteUpCall(FormattedFile& port)
{
  return WriteString(pname, &port) ||
    port.Write(ptype) || port.Write(field[0]) ||
    port.Write(field[1]);
}

//-----------------------------------------------------
// derived function used by class 
// SinglyLinkedListEntryIO to allocate storage 
// for an entry in an ActualList
//-----------------------------------------------------
SinglyLinkedListEntryIO *
ActualList::NewEntry()
{
	return new ActualListEntry;
}


//-----------------------------------------------------
// free all storage for pointers used in a CallSite
//-----------------------------------------------------
CallSite::~CallSite()
{
	if (entry_name) sfree(entry_name);
	delete alist;
}


//-----------------------------------------------
// write an unambiguous representation of a 
// CallSite to a database port 
//-----------------------------------------------
int CallSite::WriteUpCall(FormattedFile& port)
{
  return WriteString(entry_name, &port) ||
    port.Write(callsite_id) || 
    port.Write((int)entry_is_proc_param) ||
    alist->Write(port);
}


//-----------------------------------------------
// read an unambiguous representation of a 
// CallSite from a database port 
//-----------------------------------------------
int CallSite::ReadUpCall(FormattedFile& port)
{
  return ReadString(&entry_name, &port) ||
    port.Read(callsite_id) || 
    port.Read(*(int *) &entry_is_proc_param) ||
    alist->Read(port);
}	


void CallSites::Add(CallSite *cs)
{
  SinglyLinkedListIO::Append(cs);
}

//-----------------------------------------------------
// derived function used by class 
// SinglyLinkedListEntryIO to allocate storage 
// for an entry in a CallSiteList
//-----------------------------------------------------
SinglyLinkedListEntryIO *CallSites::NewEntry()
{
	return new CallSite;
}


CallSitesIterator::CallSitesIterator(CallSites *callSites) : 
SinglyLinkedListIterator((SinglyLinkedList *) callSites)
{
}

CallSitesIterator::~CallSitesIterator()
{
}

CallSite *CallSitesIterator::Current()
{
  return (CallSite *) SinglyLinkedListIterator::Current();
}



