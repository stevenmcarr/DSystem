/* $Id: FortranModule.C,v 1.1 1997/03/11 14:27:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// FortranModule.C
//
// Author: John Mellor-Crummey                                August 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#include <sys/stat.h>

#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/support/file/FileUtilities.h>

#include <libs/fileAttrMgmt/attributedFile/FileSuffixRegistry.h>

#include <libs/fileAttrMgmt/fortranModule/FortranModule.h>
#include <libs/fileAttrMgmt/fortranModule/ModifyTimeModAttr.i>

//****************************
// forward declarations
//****************************
static void push_lmt(const char *const filePathName, time_t *lmtime);


//***************************************************************************
// implementation of class FortranModule
//***************************************************************************

CLASS_NAME_IMPL(FortranModule)

FortranModule::~FortranModule()
{
}


time_t FortranModule::GetLastModificationTime()
{
  ModifyTimeModAttr *modAttr = 
    (ModifyTimeModAttr *) AttachAttribute(CLASS_NAME(ModifyTimeModAttr), 0);
  time_t lmt = modAttr->GetLastModificationTime();
  DetachAttribute(modAttr);
  return lmt;
}

time_t FortranModule::GetLastModificationTime
(const char *const srcFileName)
{
  // initialize time stamp with last modification time of file
  // (0 if file does not exist)
  time_t lmtime = FileLastModificationTime(srcFileName);
  
  // if file exists
  if (lmtime != (time_t) 0) {
    // update the last modification time for included files, if necessary
    ftt_TraverseText((char *) srcFileName, (PushFunc) push_lmt, 
		     (PopFunc) NULL, (EnterFunc) NULL, (Generic) &lmtime);
  }
  
  return lmtime;
}

static void push_lmt(const char *const filePathName, time_t *lmtime)
{
  time_t newlmtime =  FileLastModificationTime(filePathName);
  if (newlmtime > *lmtime) *lmtime = newlmtime;
}

