/* $Id: StandardModuleAttributes.h,v 1.1 1997/03/11 14:28:04 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef StandardModuleAttributes_h
#define StandardModuleAttributes_h

//***************************************************************************
// StandardModuleAttributes.h
//
// Author: John Mellor-Crummey                                January 1994
//
// Copyright 1993, Rice University
//***************************************************************************

#ifndef NamedObject_h
#include <libs/support/tables/namedObject/NamedObject.h>
#endif

class StandardModuleAttribute : public NamedObject {
public:
  StandardModuleAttribute(const char *string);
};

class AttributedFile; // minimal external declaration

extern void ComputeStandardModuleAttributes(AttributedFile *file);

#endif

