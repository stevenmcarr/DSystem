/* $Id: Module.h,v 1.1 1997/03/11 14:28:05 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// Module.h
//
// Author: John Mellor-Crummey                                November 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#ifndef Module_h
#define Module_h


#ifndef AttributedFile_h
#include <libs/fileAttrMgmt/attributedFile/AttributedFile.h>
#endif


//--------------------------------------------------------------------
// class Module
//--------------------------------------------------------------------
class Module : public AttributedFile {
public:
  Module();
  ~Module();
};


#endif
