/* $Id: module.C,v 1.6 1997/03/11 14:34:51 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/******************************************************************
 * Module IP Information Abstraction           September 1991     *
 * Author: John Mellor-Crummey                                    *
 *                                                                *
 * this file contains code that supports an external              *
 * representation of summary interprocedural information for      *
 * all of the entry points in a module.                           *
 *                                                                *
 * Copyright 1991, Rice University, as part of the ParaScope      *
 * Programming Environment Project                                *
 *                                                                *
 ******************************************************************/

#include <libs/support/file/FormattedFile.h>
#include <libs/ipAnalysis/ipInfo/module.h>


//-----------------------------------------------------
// free all storage for pointers used in a
// ModuleIPinfoListEntry
//-----------------------------------------------------
ModuleIPinfoListEntry::~ModuleIPinfoListEntry()
{
	delete info; 
}


//-----------------------------------------------------
// read an unambiguous representation of a 
// ModuleIPinfoListEntry from a database port 
//-----------------------------------------------------
int ModuleIPinfoListEntry::ReadUpCall(FormattedFile& port)
{
  return info->Read(port);
}


//-----------------------------------------------------
// write an unambiguous representation of a 
// ModuleIPinfoListEntry to a database port 
//-----------------------------------------------------
int ModuleIPinfoListEntry::WriteUpCall(FormattedFile& port)
{
  return info->Write(port);
}


//-----------------------------------------------------
// derived function used by class 
// SinglyLinkedListEntryWithDBIO to allocate storage 
// for an entry in a ModuleIPinfoList
//-----------------------------------------------------
SinglyLinkedListEntryWithDBIO *
ModuleIPinfoList::NewEntry()
{
  return new ModuleIPinfoListEntry;
}
