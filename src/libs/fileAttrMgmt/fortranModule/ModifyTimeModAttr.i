/* $Id: ModifyTimeModAttr.i,v 1.1 1997/03/11 14:27:59 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// ModifyTimeModAttr.h
//
// Author: John Mellor-Crummey                                August 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#ifndef ModifyTimeModAttr_h
#define ModifyTimeModAttr_h

#ifndef Attribute_h
#include <libs/fileAttrMgmt/attributedFile/Attribute.h>
#endif

class ModifyTimeSet; // minimal external declaration

class ModifyTimeModAttr : public Attribute {
private:
  struct ModifyTimeModAttrS *hidden;
public:

  ModifyTimeModAttr();
  ~ModifyTimeModAttr();

  CLASS_NAME_FDEF(ModifyTimeModAttr);

  time_t GetLastModificationTime();

  virtual int ComputeUpCall();
  virtual int Create();
  virtual void Destroy();

  virtual int ReadUpCall(File *file);
  virtual int WriteUpCall(File *file);

  void NoteChange(Object *ob, int kind, void *change);
};

CLASS_NAME_EDEF(ModifyTimeModAttr);

#endif /* ModifyTimeModAttr_h */

