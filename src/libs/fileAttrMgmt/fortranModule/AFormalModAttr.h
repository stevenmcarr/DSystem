/* $Id: AFormalModAttr.h,v 1.1 1997/03/11 14:27:52 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// AFormalModAttr.h
//
//   initial information about procedures in a module that describes
//   whether any of the parameters to entry points in the procedures are
//   declared as arrays 
//
// Author: John Mellor-Crummey                                October 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#ifndef AFormalModAttr_h
#define AFormalModAttr_h


#ifndef ModuleLocalInfo_h
#include <libs/ipAnalysis/ipInfo/ModuleLocalInfo.h>
#endif

#ifndef ProcLocalInfo_h
#include <libs/ipAnalysis/ipInfo/ProcLocalInfo.h>
#endif

#ifndef ClassName_h
#include <include/ClassName.h>
#endif

#ifndef StringSet_h
#include <libs/support/strings/StringSet.h>
#endif


class FormattedFile; // minimal external declaration

//-----------------------------------------------------------------------
// class AFormalModAttr:  array formal info for a module
//-----------------------------------------------------------------------

class AFormalModAttr : public ModuleLocalInfo {
public:
  AFormalModAttr();
  ~AFormalModAttr();

  CLASS_NAME_FDEF(AFormalModAttr);

  ProcLocalInfo *NewProcEntry();
  int ComputeUpCall();
  void DetachUpCall();
};

CLASS_NAME_EDEF(AFormalModAttr);


//-----------------------------------------------------------------------
// class ProcAFormalInfo:  array formal info for a procedure
//-----------------------------------------------------------------------
class ProcAFormalInfo : public ProcLocalInfo, public StringSet {
public:
  ProcAFormalInfo(const char *name = 0);
  ~ProcAFormalInfo();
  CLASS_NAME_FDEF(ProcAFormalInfo);

  int WriteUpCall(FormattedFile *port);
  int ReadUpCall(FormattedFile *port);
};

CLASS_NAME_EDEF(ProcAFormalInfo);


#endif /* AFormalModAttr_h */
