/* $Id: ProcSummariesModAttr.h,v 1.1 1997/03/11 14:28:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// ProcSummariesModAttr.h
//
// Author: John Mellor-Crummey                                October 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#ifndef ProcSummariesModAttr_h
#define ProcSummariesModAttr_h

#ifndef ModuleLocalInfo_h
#include <libs/ipAnalysis/ipInfo/ModuleLocalInfo.h>
#endif

class Module;

//--------------------------------------------------------------------
// class ProcSummariesModAttr
//--------------------------------------------------------------------
class ProcSummariesModAttr : public ModuleLocalInfo {
public:
  ProcSummariesModAttr();
  virtual ~ProcSummariesModAttr();

  ProcLocalInfo *NewProcEntry();

  virtual int ComputeUpCall() = 0;
  virtual void DetachUpCall();
};

CLASS_NAME_EDEF(ProcSummariesModAttr);

#endif /* ProcSummariesModAttr_h */

