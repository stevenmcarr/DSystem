/* $Id: NeedProvFortModAttr.C,v 1.1 1997/03/11 14:27:59 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// NeedProvFortModAttr.C
//
// Author: John Mellor-Crummey                                December 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#include <libs/fileAttrMgmt/attributedFile/AttributedFile.h>

#include <libs/frontEnd/include/NeedProvSet.h>

#include <libs/fileAttrMgmt/fortranModule/FortTreeModAttr.h>
#include <libs/fileAttrMgmt/fortranModule/NeedProvFortModAttr.h>


//*******************************************************************
// declarations
//*******************************************************************

CLASS_NAME_EIMPL_DERIVED(NeedProvFortModAttr, NeedProvModAttr); 



//*******************************************************************
// class NeedProvFortModAttr interface operations
//*******************************************************************

NeedProvFortModAttr::NeedProvFortModAttr()
{
}


NeedProvFortModAttr::~NeedProvFortModAttr()
{
}

void NeedProvFortModAttr::DetachUpCall()
{
  uplinkToFile->DetachAttributeIfUnreferenced(CLASS_NAME(FortTreeModAttr));
}


int NeedProvFortModAttr::ComputeUpCall()
{
  int retcode = 0;

  FortTreeModAttr *ftAttr = (FortTreeModAttr *) 
    uplinkToFile->AttachAttribute(CLASS_NAME(FortTreeModAttr));

  NeedProvSet *tmpNeeds = ftAttr->GetNeeds();
  NeedProvSet *tmpProvs = ftAttr->GetProvs();

  if (tmpNeeds == 0 || tmpProvs == 0) retcode = -1;
  else {
    needs = new NeedProvSet(*tmpNeeds);
    provs = new NeedProvSet(*tmpProvs);
  }

  uplinkToFile->DetachAttribute(ftAttr, CACHE_FLUSH_IMMEDIATE);
  return retcode; 
}
