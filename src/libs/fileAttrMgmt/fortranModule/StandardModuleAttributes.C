/* $Id: StandardModuleAttributes.C,v 1.2 1997/03/27 20:31:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// StandardModuleAttributes.C
//
// Author: John Mellor-Crummey                                January 1994
//
// Copyright 1993, Rice University
//***************************************************************************


#include <libs/support/tables/namedObject/NamedObjectTable.h>
#include <libs/fileAttrMgmt/attributedFile/AttributedFile.h>
#include <libs/fileAttrMgmt/fortranModule/StandardModuleAttributes.h>



//***********************************************************************
// declarations
//***********************************************************************

NamedObjectTable *standardModuleAttributes = 0;



//***********************************************************************
// interface operations for class StandardModuleAttribute
//***********************************************************************

StandardModuleAttribute::StandardModuleAttribute(const char *string) :
NamedObject(string)
{
  if (!standardModuleAttributes)
    standardModuleAttributes = new NamedObjectTable;
  standardModuleAttributes->AddEntry(this);
}



//***********************************************************************
// public operations
//***********************************************************************

void ComputeStandardModuleAttributes(AttributedFile *file)
{
  if (standardModuleAttributes) {
    NamedObjectTableIterator attrs(standardModuleAttributes);
    NamedObject *fileAttr;
    for(; fileAttr = attrs.Current(); ++attrs) {
      Attribute *attr = file->AttachAttribute(fileAttr->name);
      if (attr) file->DetachAttribute(attr);
    }
  }
}




