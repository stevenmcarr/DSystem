/* $Id: NeedProvModAttr.h,v 1.1 1997/03/11 14:28:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// NeedProvModAttr.h
//
// Author: John Mellor-Crummey                                August 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#ifndef NeedProvModAttr_h
#define NeedProvModAttr_h

#ifndef Attribute_h
#include <libs/fileAttrMgmt/attributedFile/Attribute.h>
#endif

class NeedProvSet; // minimal external declaration

class NeedProvModAttr : public Attribute {
private:
public:
  
  NeedProvSet *needs;
  NeedProvSet *provs;

  NeedProvModAttr();
  virtual ~NeedProvModAttr();

  virtual int Create();
  virtual void Destroy();

  virtual int ComputeUpCall() = 0;
  virtual void DetachUpCall();

  virtual int ReadUpCall(File *file);
  virtual int WriteUpCall(File *file);
};

CLASS_NAME_EDEF(NeedProvModAttr);

#endif /* NeedProvModAttr_h */

