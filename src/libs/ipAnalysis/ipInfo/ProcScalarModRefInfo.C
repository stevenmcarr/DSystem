/* $Id: ProcScalarModRefInfo.C,v 1.1 1997/03/11 14:34:46 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//******************************************************************
//  ProcScalarModRefInfo.C: 
//
//  a representation of scalar mod and ref information for variables
//  
// Author: 
//   John Mellor-Crummey                              December 1993
//
// Copyright 1993, Rice University
//******************************************************************


#include <stdio.h>
#include <stdlib.h>
#include <values.h>

#include <libs/support/strings/rn_string.h>
#include <libs/ipAnalysis/ipInfo/ProcScalarModRefInfo.h>
#include <libs/support/file/FormattedFile.h>



//***********************************************************************
// class EqClassScalarModRefInfo interface operations
//***********************************************************************

CLASS_NAME_IMPL(EqClassScalarModRefInfo);

EqClassScalarModRefInfo::EqClassScalarModRefInfo(const char *_name, 
						 unsigned vtype) :
NamedObjectIO(_name), type(vtype)
{
}


EqClassScalarModRefInfo::~EqClassScalarModRefInfo()
{
}


//========================================================================
// file I/O operations
//========================================================================


int EqClassScalarModRefInfo::NamedObjectReadUpCall(FormattedFile *file)
{
  return file->Read(type) || pairs[MODREFTYPE_MOD].Read(file) || pairs[MODREFTYPE_REF].Read(file);
}


int EqClassScalarModRefInfo::NamedObjectWriteUpCall(FormattedFile *file)
{
  return file->Write(type) || pairs[MODREFTYPE_MOD].Write(file) || pairs[MODREFTYPE_REF].Write(file);
}
 

//========================================================================
// output support for debugging 
//========================================================================


void EqClassScalarModRefInfo::NamedObjectDumpUpCall()
{
  fprintf(stderr, "type = %d\n", type);
  fprintf(stderr, "MOD offset-length pairs\n");
  pairs[MODREFTYPE_MOD].Dump();

  fprintf(stderr, "REF offset-length pairs\n");
  pairs[MODREFTYPE_REF].Dump();
}


void EqClassScalarModRefInfo::Dump()
{
  NamedObjectDump();
}


//***********************************************************************
// class ProcScalarModRefInfo interface operations
//***********************************************************************

CLASS_NAME_IMPL(ProcScalarModRefInfo);

ProcScalarModRefInfo::ProcScalarModRefInfo(const char *_name) : 
ProcLocalInfo(_name)
{
}


ProcScalarModRefInfo::~ProcScalarModRefInfo()
{
  NamedObjectTable::Destroy();
}


void ProcScalarModRefInfo::AddEntry(EqClassScalarModRefInfo *entry)
{
  NamedObjectTable::AddEntry(entry);
}


EqClassScalarModRefInfo *ProcScalarModRefInfo::GetEntry(const char *name)
{
  return (EqClassScalarModRefInfo *) NamedObjectTable::QueryEntry(name);
}


//========================================================================
// file I/O operations
//========================================================================


int ProcScalarModRefInfo::ReadUpCall(FormattedFile *file)
{
  return NamedObjectTableRead(file);
}


int ProcScalarModRefInfo::WriteUpCall(FormattedFile *file)
{
  return NamedObjectTableWrite(file);
}


NamedObjectIO *ProcScalarModRefInfo::NewEntry()
{
  return new EqClassScalarModRefInfo();
}
 

//***********************************************************************
// class ProcScalarModRefInfoIterator interface operations
//***********************************************************************

ProcScalarModRefInfoIterator::ProcScalarModRefInfoIterator
(ProcScalarModRefInfo *smri) : 
NamedObjectTableIterator((NamedObjectTable *) smri)
{
}


ProcScalarModRefInfoIterator::~ProcScalarModRefInfoIterator()
{
}


EqClassScalarModRefInfo *ProcScalarModRefInfoIterator::Current()
{
  return (EqClassScalarModRefInfo *) NamedObjectTableIterator::Current();
}
