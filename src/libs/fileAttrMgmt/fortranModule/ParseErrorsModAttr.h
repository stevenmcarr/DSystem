/* $Id: ParseErrorsModAttr.h,v 1.1 1997/03/11 14:28:00 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// ParseErrorsModAttr.h
//
// Author: John Mellor-Crummey                                August 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#ifndef ParseErrorsModAttr_h
#define ParseErrorsModAttr_h

#ifndef Attribute_h
#include <libs/fileAttrMgmt/attributedFile/Attribute.h>
#endif

#ifndef OrderedSetOfStrings_h
#include <libs/support/strings/OrderedSetOfStrings.h>
#endif


class ParseErrorsModAttr : public Attribute {
public:
  OrderedSetOfStrings oss;

  ParseErrorsModAttr();
  ~ParseErrorsModAttr();

  CLASS_NAME_FDEF(ParseErrorsModAttr);

  virtual int Create();
  virtual void Destroy();

  virtual int ComputeUpCall();
  virtual void DetachUpCall();
  
  virtual int ReadUpCall(File *file);
  virtual int WriteUpCall(File *file);
};

CLASS_NAME_EDEF(ParseErrorsModAttr);

#endif /* ParseErrorsModAttr_h */
