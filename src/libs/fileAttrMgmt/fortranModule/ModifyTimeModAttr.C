/* $Id: ModifyTimeModAttr.C,v 1.1 1997/03/11 14:27:59 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// ModifyTimeModAttr.C
//
// Author: John Mellor-Crummey                                September 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#include <sys/types.h>

#include <libs/support/msgHandlers/Changes.h>

#include <libs/support/file/FileUtilities.h>
#include <libs/support/file/FormattedFile.h>
#include <libs/support/strings/StringIO.h>

#include <libs/frontEnd/fortTextTree/FortTextTree.h>

#include <libs/fileAttrMgmt/fortranModule/FortranModule.h>
#include <libs/fileAttrMgmt/fortranModule/ModifyTimeModAttr.i>
#include <libs/fileAttrMgmt/fortranModule/ScanFile.h>


//*******************************************************************
// declarations 
//*******************************************************************

REGISTER_FORTRAN_MODULE_ATTRIBUTE(ModifyTimeModAttr);


//**********************
// forward declarations 
//**********************

static int FillTable(NamedObjectTable *ht, const char *srcFileName);

static time_t ComputeModificationTime(NamedObjectTable *ht);



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


ModTimeEntry::ModTimeEntry() : NamedObjectIO(0), modTime(0) 
{
}

ModTimeEntry::ModTimeEntry(const char *_name, time_t _modTime) : 
NamedObjectIO(_name), modTime(_modTime)
{
}


ModTimeEntry::~ModTimeEntry()
{
}


int ModTimeEntry::NamedObjectReadUpCall(FormattedFile *file)
{
  uint time;
  int code = file->Read(time);
  modTime = (time_t) time;

  return code;
}

int ModTimeEntry::NamedObjectWriteUpCall(FormattedFile *file)
{
  return file->Write((uint) modTime);
}





//***************************************************************************
// class ModifyTimeModAttr interface operations 
//***************************************************************************

class ModInfoTable : public NamedObjectTableIO {
  NamedObjectIO *NewEntry() {return new ModTimeEntry; }
};


struct ModifyTimeModAttrS {
  ModInfoTable ht;
  ModifyTimeModAttrS() { };
  ~ModifyTimeModAttrS() { ht.Destroy(); };
};


ModifyTimeModAttr::ModifyTimeModAttr()
{
}


ModifyTimeModAttr::~ModifyTimeModAttr()
{
  Destroy();
}


int ModifyTimeModAttr::ComputeUpCall()
{
  int code = FillTable(&hidden->ht, uplinkToFile->ReferenceFilePathName());

  // save in cache 
  if (code == 0) {
    this->SetNeedsSaving();	
    this->uplinkToFile->SaveAttribute(this);
  }

  return code;
}

int ModifyTimeModAttr::Create()
{
  hidden = new ModifyTimeModAttrS;
  return hidden ? 0 : -1;
}


void ModifyTimeModAttr::Destroy()
{
  delete hidden;
}


time_t ModifyTimeModAttr::GetLastModificationTime()
{
  time_t time = ComputeModificationTime(&hidden->ht);

  if (time == 0) { // error, try one more time
    ReComputeUpCall();
    time = ComputeModificationTime(&hidden->ht);
  }

  return time;
}


int ModifyTimeModAttr::ReadUpCall(File *file)
{
  FormattedFile ffile(file);
  return hidden->ht.NamedObjectTableRead(&ffile);
}


int ModifyTimeModAttr::WriteUpCall(File *file)
{
  FormattedFile ffile(file);
  return hidden->ht.NamedObjectTableWrite(&ffile);
}


void ModifyTimeModAttr::NoteChange(Object *ob, int kind, void *change)
{
  if (kind == SrcLineChange) {
    // harmless change that emanates editors since we do not handle
    // include files
    MarkRevisionCurrent();
  } else {
    // for changes not handled at this level -- pass them down
    Attribute::NoteChange(ob, kind, change);
  }
}


//*****************************************************************************
// private operations
//*****************************************************************************

static time_t ComputeModificationTime(NamedObjectTable *ht)
{
  time_t stamp = 0;

  NamedObjectTableIterator noti(ht);
  ModTimeEntry *mte;
  for (; mte = (ModTimeEntry *) noti.Current(); noti++) {
    time_t lmt = FileLastModificationTime(mte->name);
    if (lmt == 0) return 0;
    else if (mte->modTime != lmt) return 0;
    else if (stamp < lmt) stamp = lmt;
  }

  return stamp;
}

static void AddFileEntry(const char *srcFileName, va_list args)
{
  char *dir = va_arg(args, char *);
  NamedObjectTable *ht = va_arg(args, NamedObjectTable *);

  char *path = ssave(FullPath(dir, srcFileName));

  ht->AddEntry(new ModTimeEntry(path, FileLastModificationTime(path)));

  sfree(path);
}


static int FillTable(NamedObjectTable *ht, const char *srcFileName)
{
  char *dir = ssave(FileDirName(srcFileName));
  int code = ScanFile(srcFileName, AddFileEntry, NULL, NULL, dir, ht);
  sfree(dir);
  return code;
}

