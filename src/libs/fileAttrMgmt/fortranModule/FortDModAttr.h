/* $Id: FortDModAttr.h,v 1.1 1997/03/11 14:27:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// FortDModAttr.h
//
// Author: John Mellor-Crummey                                April 1994
//
// Copyright 1994, Rice University
//***************************************************************************

#ifndef FortDModAttr_h
#define FortDModAttr_h


#ifndef ModuleLocalInfo_h
#include <libs/ipAnalysis/ipInfo/ModuleLocalInfo.h>
#endif


class FortDModAttr : public ModuleLocalInfo {
public:
  FortDModAttr();
  ~FortDModAttr();

  CLASS_NAME_FDEF(FortDModAttr);

  ProcLocalInfo *NewProcEntry();
  int ComputeUpCall();
  void DetachUpCall();
};

CLASS_NAME_EDEF(FortDModAttr);

#endif /* FortDModAttr_h */
