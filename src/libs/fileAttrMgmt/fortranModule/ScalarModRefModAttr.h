/* $Id: ScalarModRefModAttr.h,v 1.1 1997/03/11 14:28:03 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// ScalarModRefModAttr.h
//
// Author: John Mellor-Crummey                                October 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#ifndef ScalarModRefModAttr_h
#define ScalarModRefModAttr_h


#ifndef ModuleLocalInfo_h
#include <libs/ipAnalysis/ipInfo/ModuleLocalInfo.h>
#endif


class ScalarModRefModAttr : public ModuleLocalInfo {
public:
  ScalarModRefModAttr();
  ~ScalarModRefModAttr();

  CLASS_NAME_FDEF(ScalarModRefModAttr);

  ProcLocalInfo *NewProcEntry();
  int ComputeUpCall();
  void DetachUpCall();
};

CLASS_NAME_EDEF(ScalarModRefModAttr);

#endif /* ScalarModRefModAttr_h */
