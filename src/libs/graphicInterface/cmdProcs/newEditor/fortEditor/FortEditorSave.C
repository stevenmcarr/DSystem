/* $Id: FortEditorSave.C,v 1.1 1997/03/11 14:30:50 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// FortEditorSave.C
//
// Author: John Mellor-Crummey                                February 1994
//
// Copyright 1994, Rice University
//***************************************************************************


#include <libs/fileAttrMgmt/fortranModule/FortranModule.h>
#include <libs/fileAttrMgmt/fortranModule/RefTextModAttr.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortEditor/FortEditorSave.h>


//*******************************************************************
// interface operations
//*******************************************************************


Boolean ed_RefSrcSave(FortranModule *module, MessageFunction message_func, 
		      YesNoFunction yes_no_func)
{
  RefTextModAttr *refTextAttr = (RefTextModAttr *)
    module->AttachAttribute(CLASS_NAME(RefTextModAttr));

  Boolean success = refTextAttr->Export(message_func, yes_no_func);

  module->DetachAttribute(refTextAttr, CACHE_FLUSH_IMMEDIATE);

  return success;
}


