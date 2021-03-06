/* $Id: ModuleInfoIterator.C,v 1.2 1997/03/27 20:40:12 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
//    ModuleInfoIterator.C:  
//
//    Author: 
//      John Mellor-Crummey                                    November 1993
//
//    Copyright 1993, Rice University
//***************************************************************************

#include <libs/ipAnalysis/ipInfo/ModuleLocalInfo.h>

#include <libs/fileAttrMgmt/module/Module.h>

#include <libs/fileAttrMgmt/composition/CompositionIterators.h>

#include <libs/ipAnalysis/callGraph/ModuleInfoIterator.h>



//***************************************************************************
// class ModuleInfoIterator interface operations
//***************************************************************************


ModuleInfoIterator::ModuleInfoIterator(const Composition *program, 
				       const char *moduleInfoClassName)
: hidden(new ModuleInfoIteratorS(program, moduleInfoClassName))
{
  SetCurrent();
}


ModuleInfoIterator::~ModuleInfoIterator()
{
  delete hidden;
}


void ModuleInfoIterator::SetCurrent() 
{
  Module *tempModule = hidden->modules.Current();
  *((Module **)&module) = tempModule;
  *((ModuleLocalInfo **) &moduleInfo) = (ModuleLocalInfo *) 
    (module ? tempModule->AttachAttribute(hidden->moduleInfoClassName) : 0);
}


void ModuleInfoIterator::operator++()
{
  Advance(true);
}

void ModuleInfoIterator::Advance(Boolean detachCurrentModuleInfo)
{
  if (detachCurrentModuleInfo && moduleInfo) 
    (*(Module **) &module)->DetachAttribute(*((ModuleLocalInfo **)&moduleInfo));

  ++hidden->modules;
  SetCurrent();
}


void ModuleInfoIterator::Reset()
{
  hidden->modules.Reset();
  SetCurrent();
}

