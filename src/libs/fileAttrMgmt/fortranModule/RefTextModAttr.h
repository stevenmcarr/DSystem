/* $Id: RefTextModAttr.h,v 1.1 1997/03/11 14:28:02 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// RefTextModAttr.h
//
// Author: John Mellor-Crummey                                August 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#ifndef RefTextModAttr_h
#define RefTextModAttr_h

#ifndef Attribute_h
#include <libs/fileAttrMgmt/attributedFile/Attribute.h>
#endif

#ifndef interact_h
#include <libs/support/msgHandlers/interact.h>
#endif

#ifndef general_h
#include <libs/support/misc/general.h>
#endif


class FortTextTreeModAttr; // minimal external declaration


class RefTextModAttr : public Attribute {
  FortTextTreeModAttr *fttAttr;
public:

  RefTextModAttr();
  ~RefTextModAttr();

  CLASS_NAME_FDEF(RefTextModAttr);

  virtual int Create();
  virtual void Destroy();

  virtual int ComputeUpCall();

  Boolean Export(MessageFunction message_func, YesNoFunction yes_no_func);
};

CLASS_NAME_EDEF(RefTextModAttr);

#endif /* RefTextModAttr_h */
