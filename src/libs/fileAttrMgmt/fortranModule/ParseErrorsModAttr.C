/* $Id: ParseErrorsModAttr.C,v 1.1 1997/03/11 14:28:00 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// ParseErrorsModAttr.C
//
// Author: John Mellor-Crummey                                August 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#include <stdarg.h>

#include <libs/support/file/FormattedFile.h>
#include <libs/support/msgHandlers/ErrorMsgHandler.h>

#include <libs/fileAttrMgmt/fortranModule/FortranModule.h>
#include <libs/fileAttrMgmt/fortranModule/ParseErrorsModAttr.h>
#include <libs/fileAttrMgmt/fortranModule/FortTextTreeModAttr.h>
#include <libs/fileAttrMgmt/fortranModule/FortTreeModAttr.h>


//*******************************************************************
// declarations 
//*******************************************************************

REGISTER_FORTRAN_MODULE_ATTRIBUTE(ParseErrorsModAttr);


//**********************
// forward declarations
//**********************

static int ListLine(int lineNumber, char *lineText, va_list args);
static int ErrorLine(int lineNumber, char *lineText, va_list args);
static void ReportParseErrors(const char *, OrderedSetOfStrings *oss);



//*******************************************************************
// class ParseErrorsModAttr interface operations
//*******************************************************************

ParseErrorsModAttr::ParseErrorsModAttr() : oss(true)
{
}


ParseErrorsModAttr::~ParseErrorsModAttr()
{
}


int ParseErrorsModAttr::Create()
{
  return 0;
}


void ParseErrorsModAttr::Destroy()
{
  oss.DeallocateStrings();
}


void ParseErrorsModAttr::DetachUpCall()
{
  uplinkToFile->DetachAttributeIfUnreferenced(CLASS_NAME(FortTextTreeModAttr));
  uplinkToFile->DetachAttributeIfUnreferenced(CLASS_NAME(FortTreeModAttr));
}


int ParseErrorsModAttr::ComputeUpCall()
{
  FortTreeModAttr *ftAttr = (FortTreeModAttr *)
    uplinkToFile->AttachAttribute(CLASS_NAME(FortTreeModAttr));
  
  ft_States state = ftAttr->GetCheckedState();

  if (state == ft_ERRONEOUS) {
    
    FortTextTreeModAttr *fttAttr = (FortTextTreeModAttr *)
      uplinkToFile->AttachAttribute(CLASS_NAME(FortTextTreeModAttr));

    ftt_Listing(fttAttr->ftt, NULL, ListLine, ErrorLine, &oss);

    uplinkToFile->DetachAttribute(fttAttr);
    ReportParseErrors(uplinkToFile->ReferenceFilePathName(), &oss);
  }

  uplinkToFile->DetachAttribute(ftAttr);
  
  return 0; // success ?!
}


int ParseErrorsModAttr::ReadUpCall(File *file)
{
  FormattedFile f(file);
  return oss.Read(&f);
}


int ParseErrorsModAttr::WriteUpCall(File *file)
{
  FormattedFile f(file);
  return oss.Write(&f);
}



//*******************************************************************
// private operations
//*******************************************************************


static int ListLine(int lineNumber, char *lineText, va_list args)
{
  OrderedSetOfStrings *oss = va_arg(args, OrderedSetOfStrings *);

  char lineno[80];
  sprintf(lineno, "%6d: ", lineNumber);

  oss->Append(nssave(2, lineno, lineText));

  return 0; // success
}


static int ErrorLine(int, char *lineText, va_list args)
{
  OrderedSetOfStrings *oss = va_arg(args, OrderedSetOfStrings *);

  char lineno[80];
  sprintf(lineno, "%6s  ", "");

  oss->Append(nssave(2, lineno, lineText));

  return 0; // success
}


static void ReportParseErrors(const char *filename, OrderedSetOfStrings *oss)
{
  int entries = oss->NumberOfEntries();
  errorMsgHandler.HandleMsg("Errors for Fortran Module %s\n", filename);
  for (int i = 0; i < entries; i++) 
    errorMsgHandler.HandleMsg("%s\n", (*oss)[i]);
}

