/* $Id: ProcSummariesFortModAttr.h,v 1.1 1997/03/11 14:28:01 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// ProcSummariesFortModAttr.h
//
// Author: John Mellor-Crummey                                October 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#ifndef ProcSummariesFortModAttr_h
#define ProcSummariesFortModAttr_h

#ifndef ProcSummariesModAttr_h
#include <libs/fileAttrMgmt/module/ProcSummariesModAttr.h>
#endif


class ProcSummariesFortModAttr : public ProcSummariesModAttr {
public:
  ProcSummariesFortModAttr();
  virtual ~ProcSummariesFortModAttr();
  
  virtual int ComputeUpCall();
  virtual void DetachUpCall();
};

CLASS_NAME_EDEF(ProcSummariesFortModAttr);

#endif /* ProcSummariesFortModAttr_h */
