/* $Id: AnnotationMgr.h,v 1.1 1997/03/11 14:36:27 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef AnnotationMgr_h
#define AnnotationMgr_h

//***************************************************************************
//
// AnnotationMgr.h
//
// Author: John Mellor-Crummey                                   July 1994
//
// Copyright 1994, Rice University
// 
//***************************************************************************


#ifndef RegistryObject_h
#include <libs/support/registry/RegistryObject.h>
#endif


class Annotation;        // minimal external declaration


//***************************************************************************
// class AnnotationMgr
//***************************************************************************
class AnnotationMgr : public RegistryObject { 
public:
  // create an instance of the managed annotation 
  virtual Annotation *New() = 0;
};


#endif /* AnnotationMgr_h */

