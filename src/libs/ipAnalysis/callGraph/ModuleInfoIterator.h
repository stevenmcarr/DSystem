/* $Id: ModuleInfoIterator.h,v 1.1 1997/03/11 14:34:36 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef ModuleInfoIterator_h
#define ModuleInfoIterator_h

//***************************************************************************
//    ModuleInfoIterator.h:  
//
//    Aturhor: 
//      John Mellor-Crummey                                    November 1993
//
//    Copyright 1993, Rice University
//***************************************************************************

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

class Composition; // minimal external declaration
class Module; // minimal external declaration
class ModuleLocalInfo; // minimal external declaration

//--------------------------------------------------------------------------
// class ModuleInfoIterator 
//--------------------------------------------------------------------------
class ModuleInfoIterator {
  struct ModuleInfoIteratorS *hidden;   
  void SetCurrent();
public:
  const Module *module;  
  const ModuleLocalInfo *moduleInfo;  

  //--------------------------------------------------------------
  // constructor and destructor
  //--------------------------------------------------------------
  ModuleInfoIterator(const Composition *program, 
		     const char *ModuleInfoClassName);
  ~ModuleInfoIterator();

  void Advance(Boolean detachCurrentModuleInfo);
  void operator++(); // same as Advance(true)
  void Reset();

};


#endif /* ModuleLocalInfoIterator_h */
