/* $Id: ProcScalarModRefInfo.h,v 1.1 1997/03/11 14:34:46 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef ProcScalarModRefInfo_h
#define ProcScalarModRefInfo_h

//******************************************************************
//  ProcScalarModRefInfo.h: 
//
//  a representation of scalar mod and ref information for variables
//  in procedure
//  
// Author: 
//   John Mellor-Crummey                              December 1993
//
// Copyright 1993, Rice University
//******************************************************************


#include <include/ClassName.h>
#include <libs/support/tables/namedObject/NamedObject.h>
#include <libs/support/tables/namedObject/NamedObjectTable.h>

#include <libs/ipAnalysis/ipInfo/OffsetLength.h>
#include <libs/ipAnalysis/ipInfo/ProcLocalInfo.h>
#include <libs/ipAnalysis/ipInfo/iptypes.h>


class FormattedFile; // minimal external declaration
class ProcScalarModRefInfoIterator; // forward declaration

//-----------------------------------------------------------------------
// class EqClassScalarModRefInfo
//-----------------------------------------------------------------------
class EqClassScalarModRefInfo: public NamedObjectIO {
public:
  unsigned type;
  OffsetLengthPairVector pairs[2]; // index with ModRefType

  EqClassScalarModRefInfo(const char *name = 0, unsigned vtype = 0);
  ~EqClassScalarModRefInfo();

  void Dump();

private:
  int NamedObjectWriteUpCall(FormattedFile *file);
  int NamedObjectReadUpCall(FormattedFile *file);
  void NamedObjectDumpUpCall();

  CLASS_NAME_FDEF(EqClassScalarModRefInfo);
};


//-----------------------------------------------------------------------
// class ProcScalarModRefInfo
//-----------------------------------------------------------------------
class ProcScalarModRefInfo : public ProcLocalInfo, private NamedObjectTableIO {
private:
  NamedObjectIO *NewEntry();
public:
  ProcScalarModRefInfo(const char *name = 0);
  ~ProcScalarModRefInfo();
  CLASS_NAME_FDEF(ProcScalarModRefInfo);

  
  EqClassScalarModRefInfo *GetEntry(const char *name);
  void AddEntry(EqClassScalarModRefInfo *entry);

  int WriteUpCall(FormattedFile *port);
  int ReadUpCall(FormattedFile *port);
friend class ProcScalarModRefInfoIterator;
};

CLASS_NAME_EDEF(ProcScalarModRefInfo);


//-----------------------------------------------------------------------
// class ProcScalarModRefInfoIterator
//-----------------------------------------------------------------------
class ProcScalarModRefInfoIterator : private NamedObjectTableIterator {
public:
  ProcScalarModRefInfoIterator(ProcScalarModRefInfo *smri);
  ~ProcScalarModRefInfoIterator();
  
  EqClassScalarModRefInfo *Current();
  NamedObjectTableIterator::operator++;
  NamedObjectTableIterator::Reset;
};


#endif
