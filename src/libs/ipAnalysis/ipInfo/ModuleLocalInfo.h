/* $Id: ModuleLocalInfo.h,v 1.1 1997/03/11 14:34:43 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef ModuleLocalInfo_h
#define ModuleLocalInfo_h

//***************************************************************************
// ModuleLocalInfo.h
//
// Author: John Mellor-Crummey                                December 1993
//
// Copyright 1993, Rice University
//***************************************************************************


#ifndef Attribute_h
#include <libs/fileAttrMgmt/attributedFile/Attribute.h>
#endif

#ifndef NamedObjectTable_h
#include <libs/support/tables/namedObject/NamedObjectTable.h>
#endif


class Module;                  // minimal external declaration
class ProcLocalInfo;           // minimal external declaration
class NamedObjectIO;           // minimal external declaration


//--------------------------------------------------------------------
// class ModuleLocalInfo
//--------------------------------------------------------------------
class ModuleLocalInfo : public Attribute, private NamedObjectTableIO {
  NamedObjectIO *NewEntry();
  int ReadUpCall(File *file);
  int WriteUpCall(File *file);
public:
  ModuleLocalInfo();
  virtual ~ModuleLocalInfo();

  virtual int Create();
  virtual void Destroy();

  void AddProcEntry(ProcLocalInfo *pli);
  ProcLocalInfo *GetProcEntry(const char *procName);

  virtual ProcLocalInfo *NewProcEntry() = 0;

  int ModuleLocalInfoReadUpCall(File *file);
  int ModuleLocalInfoWriteUpCall(File *file);

friend class ModuleLocalInfoIterator;
};


//--------------------------------------------------------------------
// class ModuleLocalInfoIterator
//--------------------------------------------------------------------
class  ModuleLocalInfoIterator : private NamedObjectTableIterator {
public:
  ModuleLocalInfoIterator(const ModuleLocalInfo *mli);
  ~ModuleLocalInfoIterator();

  ProcLocalInfo *Current() const;
  NamedObjectTableIterator::operator++;
  NamedObjectTableIterator::Reset;
};


#endif

