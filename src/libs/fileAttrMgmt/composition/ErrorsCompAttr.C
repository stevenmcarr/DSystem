/* $Id: ErrorsCompAttr.C,v 1.2 1997/06/24 17:36:26 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// ErrorsCompAttr.C
//
// Author: John Mellor-Crummey                                October 1993
//
// Copyright 1993, Rice University
//***************************************************************************


#include <stdarg.h>

#include <libs/support/file/FormattedFile.h>

#include <libs/fileAttrMgmt/composition/Composition.h>
#include <libs/fileAttrMgmt/composition/ErrorsCompAttr.h>


//*******************************************************************
// declarations 
//*******************************************************************

REGISTER_COMPOSITION_ATTRIBUTE(ErrorsCompAttr)



//*******************************************************************
// class ErrorsCompAttr interface operations
//*******************************************************************

ErrorsCompAttr::ErrorsCompAttr() : 
errors(true), warnings(true)
{
}


ErrorsCompAttr::~ErrorsCompAttr()
{
}


int ErrorsCompAttr::Create()
{
  return 0;
}


void ErrorsCompAttr::Destroy()
{
  errors.DeallocateStrings();
  warnings.DeallocateStrings();
}


int ErrorsCompAttr::ComputeUpCall()
{
  return 0; // success ?!
}


int ErrorsCompAttr::ReadUpCall(File *file)
{
  FormattedFile f(file);
  return errors.Read(&f) || warnings.Read(&f);
}


int ErrorsCompAttr::WriteUpCall(File *file)
{
  FormattedFile f(file);
  return errors.Write(&f) || warnings.Write(&f);
}

