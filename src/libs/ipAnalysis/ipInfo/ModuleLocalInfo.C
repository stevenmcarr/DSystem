/* $Id: ModuleLocalInfo.C,v 1.1 1997/03/11 14:34:43 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// ModuleLocalInfo.C
//
// Author: John Mellor-Crummey                                November 1993
//
// Copyright 1993, Rice University
//***************************************************************************


#include <libs/support/file/FormattedFile.h>

#include <libs/ipAnalysis/ipInfo/ProcLocalInfo.h>
#include <libs/ipAnalysis/ipInfo/ModuleLocalInfo.h>
#include <libs/fileAttrMgmt/module/Module.h>



//***************************************************************************
// class ModuleLocalInfo interface operations
//***************************************************************************

ModuleLocalInfo::ModuleLocalInfo()
{
}


ModuleLocalInfo::~ModuleLocalInfo() 
{
  Destroy();
}


void ModuleLocalInfo::Destroy() 
{
  NamedObjectTable::Destroy();
}


int ModuleLocalInfo::Create() 
{
  NamedObjectTable::Create();
  return 0;
}


void ModuleLocalInfo::AddProcEntry(ProcLocalInfo *pli) 
{
  AddEntry(pli);
}


ProcLocalInfo *ModuleLocalInfo::GetProcEntry(const char *procName) 
{
  return (ProcLocalInfo *) QueryEntry(procName);
}


//--------------------------------------------------------------------------
//  I/O write support
//--------------------------------------------------------------------------
int ModuleLocalInfo::WriteUpCall(File *file) 
{
  FormattedFile ffile(file);
  return NamedObjectTableWrite(&ffile) || ModuleLocalInfoWriteUpCall(file);
}


int ModuleLocalInfo::ModuleLocalInfoWriteUpCall(File *) 
{
  return 0;
}


//--------------------------------------------------------------------------
//  I/O read support
//--------------------------------------------------------------------------
int ModuleLocalInfo::ReadUpCall(File *file) 
{
  FormattedFile ffile(file);
  return NamedObjectTableRead(&ffile) || ModuleLocalInfoReadUpCall(file);
}


NamedObjectIO *ModuleLocalInfo::NewEntry() 
{
  return NewProcEntry();
}


int ModuleLocalInfo::ModuleLocalInfoReadUpCall(File *) 
{
  return 0;
}



//***************************************************************************
// class ModuleLocalInfo interface operations
//***************************************************************************


ModuleLocalInfoIterator::ModuleLocalInfoIterator(const ModuleLocalInfo *mli) : 
NamedObjectTableIterator((NamedObjectTable *) mli)
{
}

ModuleLocalInfoIterator::~ModuleLocalInfoIterator() 
{ 
}

ProcLocalInfo *ModuleLocalInfoIterator::Current() const
{
  return (ProcLocalInfo *) NamedObjectTableIterator::Current();
}


