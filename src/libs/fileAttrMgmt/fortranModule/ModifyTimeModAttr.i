/* $Id: ModifyTimeModAttr.i,v 1.2 1997/03/27 20:31:44 carr Exp $ */
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
#include <libs/support/file/FileUtilities.h>
#include <libs/support/file/FormattedFile.h>

class ModifyTimeSet; // minimal external declaration

//***************************************************************************
// class ModTimeEntry interface operations 
//***************************************************************************


class ModTimeEntry : public NamedObjectIO {
public:
  time_t modTime;
  ModTimeEntry(const char *name, time_t modTime);
  ModTimeEntry();
  ~ModTimeEntry();
  int NamedObjectReadUpCall(FormattedFile *ffile);
  int NamedObjectWriteUpCall(FormattedFile *ffile);
};


class ModInfoTable : public NamedObjectTableIO {
  NamedObjectIO *NewEntry() {return new ModTimeEntry; }
};


class ModifyTimeModAttrS {
public:
  ModInfoTable ht;
  ModifyTimeModAttrS() { };
  ~ModifyTimeModAttrS() { ht.Destroy(); };
};

class ModifyTimeModAttr : public Attribute {
private:
  ModifyTimeModAttrS *hidden;
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

