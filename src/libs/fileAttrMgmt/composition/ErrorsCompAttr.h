/* $Id: ErrorsCompAttr.h,v 1.1 1997/03/11 14:27:49 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// ErrorsCompAttr.h
//
// Author: John Mellor-Crummey                                October 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#ifndef ErrorsCompAttr_h
#define ErrorsCompAttr_h

#ifndef Attribute_h
#include <libs/fileAttrMgmt/attributedFile/Attribute.h>
#endif


class ErrorsCompAttr : public Attribute {
public:
  OrderedSetOfStrings errors;
  OrderedSetOfStrings warnings;

  ErrorsCompAttr();
  ~ErrorsCompAttr();

  virtual int Create();
  virtual void Destroy();

  virtual int ComputeUpCall();

  virtual int ReadUpCall(File *file);
  virtual int WriteUpCall(File *file);
};

CLASS_NAME_EDEF(ErrorsCompAttr);

#endif /* ErrorsCompAttr_h */
