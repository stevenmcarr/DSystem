/* $Id: NeedProvFortModAttr.h,v 1.1 1997/03/11 14:28:00 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// NeedProvFortModAttr.h
//
// Author: John Mellor-Crummey                                August 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#ifndef NeedProvFortModAttr_h
#define NeedProvFortModAttr_h

#ifndef NeedProvFortAttr_h
#include <libs/fileAttrMgmt/module/NeedProvModAttr.h>
#endif


class NeedProvFortModAttr : public NeedProvModAttr {
private:
public:
  
  NeedProvFortModAttr();
  virtual ~NeedProvFortModAttr();
  
  virtual int ComputeUpCall();
  virtual void DetachUpCall();
};

CLASS_NAME_EDEF(NeedProvFortModAttr);

#endif /* NeedProvFortModAttr_h */

