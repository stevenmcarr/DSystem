/* $Id: NeedProvCompAttr.h,v 1.1 1997/03/11 14:27:50 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// NeedProvCompAttr.h
//
// Author: John Mellor-Crummey                                October 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#ifndef NeedProvCompAttr_h
#define NeedProvCompAttr_h

#ifndef Attribute_h
#include <libs/fileAttrMgmt/attributedFile/Attribute.h>
#endif

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

class ProcModuleMap; // minimal external declaration

class NeedProvCompAttr : public Attribute {
private:
public:
  ProcModuleMap *needs;
  ProcModuleMap *provides;

  char *programName;
  Boolean consistent;

  NeedProvCompAttr();
  ~NeedProvCompAttr();

  virtual void Destroy();
  virtual int Create();

  virtual int ComputeUpCall();

  virtual int ReadUpCall(File *file);
  virtual int WriteUpCall(File *file);
};

CLASS_NAME_EDEF(NeedProvCompAttr);

#endif /* NeedProvCompAttr_h */
