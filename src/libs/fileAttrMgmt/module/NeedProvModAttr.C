/* $Id: NeedProvModAttr.C,v 1.1 1997/03/11 14:28:06 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// NeedProvModAttr.C
//
// Author: John Mellor-Crummey                                August 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#include <libs/support/file/FormattedFile.h>

#include <libs/frontEnd/include/NeedProvSet.h>

#include <libs/fileAttrMgmt/module/Module.h>
#include <libs/fileAttrMgmt/module/NeedProvModAttr.h>


//*******************************************************************
// declarations 
//*******************************************************************

CLASS_NAME_EIMPL(NeedProvModAttr);


//*******************************************************************
// class NeedProvModAttr interface operations
//*******************************************************************

NeedProvModAttr::NeedProvModAttr()
{
}


NeedProvModAttr::~NeedProvModAttr()
{
  Destroy();
}


int NeedProvModAttr::Create()
{
  needs = 0;
  provs = 0;
  return 0;
}


void NeedProvModAttr::Destroy()
{
  delete needs;
  delete provs;
}


void NeedProvModAttr::DetachUpCall()
{
}

int NeedProvModAttr::ReadUpCall(File *file)
{
  FormattedFile ffile(file);

  needs = new NeedProvSet;
  provs = new NeedProvSet;

  int code = needs->Read(&ffile) || provs->Read(&ffile);
  if (code) { 
    // reinitialize state
    Destroy(); 
    Create();
  }

  return code;
}


int NeedProvModAttr::WriteUpCall(File *file)
{
  FormattedFile ffile(file);
  return needs->Write(&ffile) ||  provs->Write(&ffile);
}



