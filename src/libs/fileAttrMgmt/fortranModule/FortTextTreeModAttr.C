/* $Id: FortTextTreeModAttr.C,v 1.1 1997/03/11 14:27:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// FortTextTreeModAttr.C
//
// Author: John Mellor-Crummey                                August 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#include <assert.h>
#include <libs/support/msgHandlers/Changes.h>

#include <libs/support/msgHandlers/TraceMsgHandler.h>

#include <libs/fileAttrMgmt/fortranModule/FortranModule.h>
#include <libs/fileAttrMgmt/fortranModule/FortTreeModAttr.h>
#include <libs/fileAttrMgmt/fortranModule/FortTextTreeModAttr.h>



//*******************************************************************
// declarations 
//*******************************************************************

REGISTER_FORTRAN_MODULE_ATTRIBUTE(FortTextTreeModAttr);



//*******************************************************************
// class FortTextTreeModAttr interface operations
//*******************************************************************

FortTextTreeModAttr::FortTextTreeModAttr()
{
  ftt_Init();
}

FortTextTreeModAttr::~FortTextTreeModAttr()
{
  Destroy();
  ftt_Fini();
}


int FortTextTreeModAttr::Create()
{
  ftAttr = (FortTreeModAttr *) 
    uplinkToFile->AttachAttribute(CLASS_NAME(FortTreeModAttr));
  ftt = ftt_Create(ftAttr->ft);
  return 0;
}


void FortTextTreeModAttr::Destroy()
{
  ftt_Close(ftt);
  uplinkToFile->DetachAttribute(ftAttr);
}


int FortTextTreeModAttr::ReComputeUpCall()
{
  AST_INDEX root = ftAttr->Root();
  ftt_TreeWillChange(ftt, root);
  ftt_TreeChanged(ftt, root);
  return 0;
}


int FortTextTreeModAttr::ComputeUpCall()
{
  switch (ftAttr->GetState()) {
  case ft_UNINITIALIZED: {
    const char *file = uplinkToFile->ReferenceFilePathName();

    traceMsgHandler.HandleMsg(1, "Begin parsing %s ...\n", file);
    ftt_ImportFromTextFile((char *) file, ftt);
    traceMsgHandler.HandleMsg(1, "Finished parsing %s.\n", file); 

    ftAttr->ResetStateToInitialized();
    break;
  }
  case ft_INITIALIZED:
  case ft_CORRECT:
  case ft_ERRONEOUS:
  case ft_WARNINGS_ONLY:
    break;
  }

  return 0; // success ?!
}


int FortTextTreeModAttr::ReadUpCall(File *file)
{
  // if tree is uninitialized, then we need to parse instead of
  // reading ft supplemental ftt state from a file
  if (ftAttr->GetState() == ft_UNINITIALIZED) 
    return 1; // graceful failure

  int errorCode = ftt_Read(ftt, file);
  return errorCode;
}


int FortTextTreeModAttr::WriteUpCall(File *file)
{
  int errorCode = ftt_Write(ftt, file);
  return errorCode;
}


void FortTextTreeModAttr::NoteChange(Object *ob, int kind, void *change)
{
  if (kind == SrcLineChange) {
    // this change emanates from editors that keeps the FortTextTree
    // up to date
    MarkRevisionCurrent();
  } else {
    // for changes not handled at this level -- pass them down
    Attribute::NoteChange(ob, kind, change);
  }
}


