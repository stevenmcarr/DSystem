/* $Id: CompositionSuffixRegistry.C,v 1.1 1997/03/11 14:27:49 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//******************************************************************
// CompositionSuffixRegistry.C: 
//
//  a registry for the types of files a composition understands 
//
// Author: 
//   John Mellor-Crummey                              December 1993
//
// Copyright 1993, Rice University
//******************************************************************

#include <libs/fileAttrMgmt/attributedFile/FileSuffixRegistry.h>

#include <libs/fileAttrMgmt/fortranModule/FortranModule.h>
#include <libs/fileAttrMgmt/composition/Composition.h>


//*******************************************************************
// declarations 
//*******************************************************************

FileSuffixRegistry *Composition::compFileSuffixRegistry = 0;

REGISTER_FILE_SUFFIX(FORTRAN_FILE_SUFFIX, FortranModule, Composition);

REGISTER_FILE_SUFFIX(COMPOSITION_FILE_SUFFIX, Composition, Composition);



//***************************************************************************
// class Composition file suffix registry interface operations
//***************************************************************************


FileSuffixRegistry *Composition::GetFileSuffixRegistry()
{
  return compFileSuffixRegistry;
}


void Composition::RegisterFileSuffixHandle(FileSuffixHandle *fsh)
{
  if (!compFileSuffixRegistry) compFileSuffixRegistry = new FileSuffixRegistry;
  compFileSuffixRegistry->Register(fsh);
}
