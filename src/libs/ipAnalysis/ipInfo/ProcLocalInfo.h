/* $Id: ProcLocalInfo.h,v 1.1 1997/03/11 14:34:46 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// ProcLocalInfo.h
//
// Author: John Mellor-Crummey                                December 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#ifndef ProcLocalInfo_h
#define ProcLocalInfo_h


#ifndef ClassName_h
#include <include/ClassName.h>
#endif

#ifndef NamedObject_h
#include <libs/support/tables/namedObject/NamedObject.h>
#endif

#ifndef CallSitesLocalInfo_h
#include <libs/ipAnalysis/ipInfo/CallSitesLocalInfo.h>
#endif


//***************************************************************************
// class ProcLocalInfo
//***************************************************************************
class ProcLocalInfo : public NamedObjectIO, public CallSitesLocalInfo {
public:
  ProcLocalInfo(const char *name);
  virtual ~ProcLocalInfo();
  
  CLASS_NAME_FDEF(ProcLocalInfo);

private:
  virtual int ReadUpCall(FormattedFile *file) = 0;
  virtual int WriteUpCall(FormattedFile *file) = 0;

  int NamedObjectReadUpCall(FormattedFile *file);
  int NamedObjectWriteUpCall(FormattedFile *file);
};


CLASS_NAME_EDEF(ProcLocalInfo);

#endif
