/* $Id: FortranModuleAttributes.C,v 1.1 1997/03/11 14:27:58 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// FortranModuleAttributes.C
//
// Author: John Mellor-Crummey                                December 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#include <libs/fileAttrMgmt/fortranModule/FortranModule.h>
#include <libs/fileAttrMgmt/fortranModule/NeedProvFortModAttr.h>
#include <libs/fileAttrMgmt/fortranModule/ProcSummariesFortModAttr.h>

//***************************************************************************
// declarations
//***************************************************************************


REGISTER_FORTRAN_MODULE_DERIVED_ATTRIBUTE(ProcSummariesFortModAttr, 
					  ProcSummariesModAttr);

REGISTER_FORTRAN_MODULE_DERIVED_ATTRIBUTE(NeedProvFortModAttr,
					  NeedProvModAttr);



//***************************************************************************
// class FortranModule attribute constructor registry implementation 
//***************************************************************************

AttributeConstructorRegistry 
*FortranModule::fortModuleAttrConstructorRegistry = 0;

AttributeConstructorRegistry *FortranModule::GetAttributeConstructorRegistry()
{
  return fortModuleAttrConstructorRegistry;
}

#if 0
void FortranModule::RegisterAttributeConstructor
(AttributeConstructor *attrConstructor)
{
  if (!fortModuleAttrConstructorRegistry)
    fortModuleAttrConstructorRegistry = new AttributeConstructorRegistry;  
  fortModuleAttrConstructorRegistry->Register(attrConstructor);
}
#endif
