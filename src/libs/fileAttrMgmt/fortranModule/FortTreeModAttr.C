/* $Id: FortTreeModAttr.C,v 1.1 1997/03/11 14:27:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// FortTreeModAttr.C
//
// Author: John Mellor-Crummey                                August 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#include <assert.h>
#include <libs/support/msgHandlers/Changes.h>

#include <libs/fileAttrMgmt/fortranModule/FortranModule.h>
#include <libs/fileAttrMgmt/fortranModule/StandardModuleAttributes.h>
#include <libs/fileAttrMgmt/fortranModule/FortTreeModAttr.h>
#include <libs/fileAttrMgmt/fortranModule/FortTextTreeModAttr.h>
#include <libs/fileAttrMgmt/fortranModule/ParseErrorsModAttr.h>



//*******************************************************************
// declarations 
//*******************************************************************

REGISTER_FORTRAN_MODULE_ATTRIBUTE(FortTreeModAttr);


//**********************
// forward declarations
//**********************

static void ReportErrors(AttributedFile *file);



//*******************************************************************
// class FortTreeModAttr interface operations
//*******************************************************************

FortTreeModAttr::FortTreeModAttr()
{
  ft_Init();
}


FortTreeModAttr::~FortTreeModAttr()
{
  Destroy();
  ft_Fini();
}

int FortTreeModAttr::Create()
{
  ft = ft_Create();
  return 0;
}


void FortTreeModAttr::Destroy()
{
  ft_Close(ft);
}


void FortTreeModAttr::DetachUpCall()
{
  uplinkToFile->DetachAttributeIfUnreferenced(CLASS_NAME(FortTextTreeModAttr));
}


int FortTreeModAttr::ComputeUpCall()
{
  FortTextTreeModAttr *attr = (FortTextTreeModAttr *)
    uplinkToFile->AttachAttribute(CLASS_NAME(FortTextTreeModAttr));

  uplinkToFile->DetachAttribute(attr);

  ReportErrors(uplinkToFile);

  return 0; // success ?!
}


AST_INDEX FortTreeModAttr::Root()
{ 
  return ft_Root(ft);
}


ft_States FortTreeModAttr::GetState()
{ 
  return ft_GetState(ft);
}


ft_States FortTreeModAttr::GetCheckedState()
{ 
  ft_States state = ft_GetState(ft);
  assert(state != ft_UNINITIALIZED);

  if (state == ft_INITIALIZED) state = ft_Check(ft);
  return state;
}


void FortTreeModAttr::ResetStateToInitialized()
{ 
  ft_ResetStateToInitialized(ft);
}


NeedProvSet *FortTreeModAttr::GetNeeds()
{ 
  return ft_GetNeeds(ft);
}


NeedProvSet *FortTreeModAttr::GetProvs()
{ 
  return ft_GetProvs(ft);
}


int FortTreeModAttr::ReadUpCall(File *file)
{
  int errorCode = ft_Read(ft, file);
  ReportErrors(uplinkToFile);
  return errorCode;
}


int FortTreeModAttr::WriteUpCall(File *file)
{
  ComputeStandardModuleAttributes(uplinkToFile);
  int errorCode = ft_Write(ft, file);
  return errorCode;
}


void FortTreeModAttr::NoteChange(Object *ob, int kind, void *change)
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


//*******************************************************************
// private operations
//*******************************************************************

static void ReportErrors(AttributedFile *file)
{
  // check for and report errors 
  ParseErrorsModAttr *peModAttr = (ParseErrorsModAttr *) 
    file->AttachAttribute(CLASS_NAME(ParseErrorsModAttr));
  file->DetachAttribute(peModAttr);
}




