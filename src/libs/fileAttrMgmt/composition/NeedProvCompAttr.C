/* $Id: NeedProvCompAttr.C,v 1.1 1997/03/11 14:27:50 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// NeedProvCompAttr.C
//
// Author: John Mellor-Crummey                                October 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#include <stdio.h>

#include <libs/support/file/FormattedFile.h>
#include <libs/support/strings/StringBuffer.h>
#include <libs/support/msgHandlers/TraceMsgHandler.h>
#include <libs/support/strings/StringIO.h>

#include <libs/frontEnd/include/NeedProvSet.h>
#include <libs/frontEnd/ast/gen.h>

#include <libs/fileAttrMgmt/module/Module.h>
#include <libs/fileAttrMgmt/module/NeedProvModAttr.h>

#include <libs/fileAttrMgmt/composition/Composition.h>
#include <libs/fileAttrMgmt/composition/ProcModuleMap.h>
#include <libs/fileAttrMgmt/composition/CompositionIterators.h>
#include <libs/fileAttrMgmt/composition/NeedProvCompAttr.h>


//*******************************************************************
// declarations 
//*******************************************************************

REGISTER_COMPOSITION_ATTRIBUTE(NeedProvCompAttr);


typedef FUNCTION_POINTER(void, NPModFn, (Composition *comp, 
					 Module *module, 
					 NeedProvCompAttr *npcAttr));

typedef FUNCTION_POINTER(void, NPCompFn, (Composition *outerComp, 
					  Composition *innerComp,
					  NeedProvCompAttr *npcAttr));


//**********************
// forward declarations
//**********************

static void CheckModuleNeeds(Composition *comp,
			     Module *module, 
			     NeedProvCompAttr *npcAttr);

static void AddModuleProvides(Composition *comp,
			      Module *module, 
			      NeedProvCompAttr *npcAttr);

static void CheckCompositionNeeds(Composition *outerComp,
				  Composition *innerComp,
				  NeedProvCompAttr *npcAttr);

static void AddCompositionProvides(Composition *outerComp,
				   Composition *innerComp, 
				   NeedProvCompAttr *npcAttr);



static void ApplyToComponents(NeedProvCompAttr *npcAttr, 
			      Composition *comp, 
			      NPModFn modFn, NPCompFn compFn);


static CompTypeMatch CheckInterfaces(OrderedSetOfStrings *errors, 
				     OrderedSetOfStrings *warnings,
				     ProcInterface *use, 
				     ProcModuleMapEntry *def);



//*******************************************************************
// class NeedProvCompAttr interface operations
//*******************************************************************

NeedProvCompAttr::NeedProvCompAttr()
{
}


NeedProvCompAttr::~NeedProvCompAttr()
{
  Destroy();
}


int NeedProvCompAttr::Create()
{
  needs = new ProcModuleMap;
  provides = new ProcModuleMap;
  programName = 0;
  consistent = true;
  return ((needs && provides) ? 0 : -1); // fail if new ops fail
}


void NeedProvCompAttr::Destroy()
{
  delete needs;
  delete provides;
  if (programName) sfree(programName);
}


int NeedProvCompAttr::ReadUpCall(File *file)
{
  FormattedFile ffile(file);

  int temp;
  int code = needs->Read(&ffile) || provides->Read(&ffile) || 
    ReadString(&programName, &ffile) || ffile.Read(temp);
  
  if (code) {
    delete needs; 
    needs = new ProcModuleMap;

    delete provides;
    provides = new ProcModuleMap;

    if (programName) sfree(programName);
    programName = 0;

    consistent = true;
  } 
  else {
    if (programName[0] == 0) {
      sfree(programName); programName = 0;
    }
    consistent = (Boolean) temp;
  }

  return code;
}


int NeedProvCompAttr::WriteUpCall(File *file)
{
  FormattedFile ffile(file);
  return needs->Write(&ffile) ||  provides->Write(&ffile) || 
    (programName ? WriteString(programName, &ffile) :  WriteString("", &ffile)) ||
    ffile.Write((int) consistent);
}


int NeedProvCompAttr::ComputeUpCall()
{
  Composition *comp = (Composition *) uplinkToFile;

  traceMsgHandler.HandleMsg
    (1, "Begin composing %s ...\n", comp->ReferenceFilePathName());

#if 0
  ErrorsCompAttr *errorsAttr = (ErrorsCompAttr *) 
    uplinkToFile->AttachAttribute(CLASS_NAME(ErrorsCompAttr));
#endif

  ApplyToComponents(this, comp, AddModuleProvides, AddCompositionProvides);
  ApplyToComponents(this, comp, CheckModuleNeeds, CheckCompositionNeeds);

  traceMsgHandler.HandleMsg
    (1, "Finish composing %s ...\n", comp->ReferenceFilePathName());

  return 0; // success ?!
}


//**************************************************************************
// private operations
//**************************************************************************


static void ApplyToComponents(NeedProvCompAttr *npcAttr, 
			      Composition *comp, 
			      NPModFn modFn, NPCompFn compFn)
{
  //-----------------------------------------
  // for each component in the composition ...
  //-----------------------------------------
  CompComponentsIterator ci(comp);
  AttributedFile *component;
  for (; component = ci.Current(); ci++) {
    if (component->ClassName() == CLASS_NAME(Composition)) 
      compFn(comp, (Composition *) component, npcAttr);
    else modFn(comp, (Module *) component, npcAttr);
  }
}



static void AddModuleProvides(Composition *comp, 
			      Module *module, 
			      NeedProvCompAttr *npcAttr)
{
  OrderedSetOfStrings *errs = (OrderedSetOfStrings *) &comp->errors;

  const char *thisModuleName = module->ReferenceFilePathName();
  NeedProvModAttr *npAttr = 
    (NeedProvModAttr *) module->AttachAttribute(CLASS_NAME(NeedProvModAttr));

  if (npAttr == 0) {
    StringBuffer error(256); // temporary space for building error messages
    error.Append("Unable to compute Needs/Provides information for module %s\n.", thisModuleName);
    errs->Append(error.Finalize());
    return;
  }
  
  //---------------------------------------------------
  // for each entry in the provide set of a module ...
  //---------------------------------------------------
  NeedProvSetIterator npsi(npAttr->provs);
  ProcInterface *pi;
  for (; pi = npsi.Current(); npsi++) {
    StringBuffer error(256); // temporary space for building error messages
    
    //-----------------------------------------------------
    // if the entry is a PROGRAM, error if it is not unique 
    //-----------------------------------------------------
    if (pi->procType == PI_PROGRAM) {
      if (npcAttr->programName == 0) {
	// record name of program unit
	npcAttr->programName = ssave(pi->name); 
      } else {
	ProcModuleMapEntry *pgmModuleMapEntry = 
	  npcAttr->provides->QueryEntry(npcAttr->programName);
	
	error.Append("%s: %s in %s and %s in %s\n",
		     "Multiple program units encountered",
		     npcAttr->programName, 
		     pgmModuleMapEntry->moduleName,
		     pi->name, thisModuleName);
	errs->Append(error.Finalize());
	
	error.Append("Ignoring definition of %s\n", pi->name);
	errs->Append(error.Finalize());
	
	npcAttr->consistent = false;
	continue;
      }
    }
    
    //------------------------------------------------------------------
    // error if the entry name was previously defined in the composition
    //------------------------------------------------------------------
    ProcModuleMapEntry *prevDefinition = 
      npcAttr->provides->QueryEntry(pi->name);
    if (prevDefinition) {
      
      error.Append("Procedure %s: defined in %s, redefined in %s.\n", 
		   pi->name, prevDefinition->moduleName, 
		   thisModuleName);
      errs->Append(error.Finalize());
      
      npcAttr->consistent = false;
      continue;
    }
    
    //--------------------------------------------------------------
    // add the new provides entry to the provides map
    //--------------------------------------------------------------
    npcAttr->provides->AddEntry(new ProcModuleMapEntry(thisModuleName, pi));
  }
  module->DetachAttribute(npAttr);
  module->SaveAllNewAttributes();
}


static void AddCompositionProvides(Composition *outerComp,
				   Composition *innerComp, 
				   NeedProvCompAttr *npcAttr)
{
  OrderedSetOfStrings *errs = (OrderedSetOfStrings *) &outerComp->errors;

  const char *innerCompName = innerComp->ReferenceFilePathName();
  NeedProvCompAttr *npAttr = (NeedProvCompAttr *) 
    innerComp->AttachAttribute(CLASS_NAME(NeedProvCompAttr));

  if (npAttr == 0) {
    StringBuffer error(256); // temporary space for building error messages
    error.Append("Unable to compute Needs/Provides information for composition %s\n.", innerCompName);
    errs->Append(error.Finalize());
    return;
  }
  
  //---------------------------------------------------
  // for each entry in the provide set of a composition ...
  //---------------------------------------------------
  ProcModuleMapIterator pmmi(npAttr->provides);
  ProcModuleMapEntry *pmme;
  for (; pmme = pmmi.Current(); pmmi++) {
    StringBuffer error(256); // temporary space for building error messages
    
    //-----------------------------------------------------
    // if the entry is a PROGRAM, error if it is not unique 
    //-----------------------------------------------------
    if (pmme->procType == PI_PROGRAM) {
      if (npcAttr->programName == 0) {
	// record name of program unit
	npcAttr->programName = ssave(pmme->name); 
      } else {
	ProcModuleMapEntry *pgmModuleMapEntry = 
	  npcAttr->provides->QueryEntry(npcAttr->programName);
	
	error.Append("%s: %s in %s and %s in %s\n",
		     "Multiple program units encountered",
		     npcAttr->programName, 
		     pgmModuleMapEntry->moduleName,
		     pmme->name, pmme->moduleName);
	errs->Append(error.Finalize());
	
	error.Append("Ignoring definition of %s\n", pmme->name);
	errs->Append(error.Finalize());
	
	npcAttr->consistent = false;
	continue;
      }
    }
    
    //------------------------------------------------------------------
    // error if the entry name was previously defined in the composition
    //------------------------------------------------------------------
    ProcModuleMapEntry *prevDefinition = 
      npcAttr->provides->QueryEntry(pmme->name);
    if (prevDefinition) {
      
      error.Append("Procedure %s: defined in %s, redefined in %s.\n", 
		   pmme->name, prevDefinition->moduleName, 
		   pmme->moduleName);
      errs->Append(error.Finalize());
      
      npcAttr->consistent = false;
      continue;
    }
    
    //--------------------------------------------------------------
    // add the new provides entry to the provides map
    //--------------------------------------------------------------
    npcAttr->provides->AddEntry
      (new ProcModuleMapEntry(pmme->moduleName, pmme));
  }
  innerComp->DetachAttribute(npAttr);
  innerComp->SaveAllNewAttributes();
}


static void CheckModuleNeeds(Composition *comp,
			     Module *module, 
			     NeedProvCompAttr *npcAttr)
{
  OrderedSetOfStrings *errs = (OrderedSetOfStrings *) &comp->errors;
  
  const char *thisModuleName = module->ReferenceFilePathName();
  NeedProvModAttr *npAttr = 
    (NeedProvModAttr *) module->AttachAttribute(CLASS_NAME(NeedProvModAttr));

  if (npAttr == 0) 
    return; // error message already echoed when processing provides
  
  //---------------------------------------------------
  // for each entry in the needs set of a module ...
  //---------------------------------------------------
  NeedProvSetIterator npsi(npAttr->needs);
  ProcInterface *pi;
  for (; pi = npsi.Current(); npsi++) {
    
    //---------------------------------------------------------------------
    // error if the entry is not provided by some module in the composition
    //---------------------------------------------------------------------
    ProcModuleMapEntry *definition = 
      npcAttr->provides->QueryEntry(pi->name);
    if (definition == 0) {
      StringBuffer error(256); // space for building an error message
      
      error.Append("Procedure %s undefined: ", pi->name);
      error.Append("required by procedure %s in module %s.\n",
		   pi->enclosingScopeName, thisModuleName); 
      errs->Append(error.Finalize());
      
      npcAttr->consistent = false;
    } else {
      CheckInterfaces(errs, (OrderedSetOfStrings *) &comp->warnings, 
		      pi, definition);
    }
  }
  module->DetachAttribute(npAttr, CACHE_FLUSH_IMMEDIATE);
}



static void CheckCompositionNeeds(Composition *outerComp,
				  Composition *innerComp, NeedProvCompAttr *npcAttr)
{
  OrderedSetOfStrings *errs = (OrderedSetOfStrings *) &outerComp->errors;

  const char *innerCompName = innerComp->ReferenceFilePathName();
  NeedProvCompAttr *npAttr = (NeedProvCompAttr *) 
    innerComp->AttachAttribute(CLASS_NAME(NeedProvCompAttr));

  if (npAttr == 0) 
    return; // error message already echoed when processing provides
  
  //---------------------------------------------------
  // for each entry in the provide set of a composition ...
  //---------------------------------------------------
  ProcModuleMapIterator pmmi(npAttr->needs);
  ProcModuleMapEntry *pmme;
  for (; pmme = pmmi.Current(); pmmi++) {
    StringBuffer error(256); // temporary space for building error messages
  
    //---------------------------------------------------------------------
    // error if the entry is not provided by some module in the composition
    //---------------------------------------------------------------------
    ProcModuleMapEntry *definition = 
      npcAttr->provides->QueryEntry(pmme->name);
    if (definition == 0) {
      StringBuffer error(256); // space for building an error message
      
      error.Append("Procedure %s undefined: ", pmme->name);
      error.Append("required by procedure %s in module %s.\n",
		   pmme->enclosingScopeName, pmme->moduleName); 
      errs->Append(error.Finalize());
      
      npcAttr->consistent = false;
    } else {
      CheckInterfaces(errs, (OrderedSetOfStrings *) &outerComp->warnings, 
		      pmme, definition);
    }
  }
  innerComp->DetachAttribute(npAttr);
}


#define INIT_ERR_LEN 250
static CompTypeMatch CheckInterfaces
(OrderedSetOfStrings *errors, OrderedSetOfStrings *warnings, 
 ProcInterface *use, ProcModuleMapEntry *def)
{
  int warn = 0;
  StringBuffer msg(INIT_ERR_LEN); // length is not a hard limit
  
  if (use->ReturnTypesConsistent(def) == TYPE_MATCH_ERROR) {
    msg.Append("Procedure %s: return type `%s' expected of call from \
procedure %s does not match return type `%s' of definition in module %s.\n", 
	       use->name, gen_type_get_text(use->returnType),
	       use->enclosingScopeName, gen_type_get_text(def->returnType),
	       def->moduleName);

    errors->Append(msg.Finalize());
    return TYPE_MATCH_ERROR;
  }
  
  if (use->ArgCountConsistent(def) == TYPE_MATCH_ERROR) {
    msg.Append("Procedure %s: call from procedure %s passes %d args, \
definition in module %s expects %d args.\n", 
		 use->name, use->enclosingScopeName, use->maxArgs,
		 def->moduleName, def->maxArgs);

    errors->Append(msg.Finalize());
    return TYPE_MATCH_ERROR;
  }
  
  uint nargs = use->NumberArgsToTestForConsistency(def);
  for (uint i = 0; i < nargs; i++) {
    switch(use->ArgPairConsistent(def, i)) {
    case TYPE_MATCH_ERROR: 
      msg.Append("Procedure %s:  call from procedure %s passes %s for arg %d, \
definition in module %s expects %s.\n",  use->name, use->enclosingScopeName, 
		   gen_type_get_text(use->Arg(i)->type), def->moduleName,
		   gen_type_get_text(def->Arg(i)->type));
      errors->Append(msg.Finalize());
      return TYPE_MATCH_ERROR;
    case TYPE_MATCH_WARNING: 
      msg.Append("Procedure %s:  call from procedure %s passes %s for arg %d, \
definition in module %s expects %s.\n",  use->name, use->enclosingScopeName, 
		 gen_type_get_text(use->Arg(i)->type), i, def->moduleName,
		 gen_type_get_text(def->Arg(i)->type));
      warnings->Append(msg.Finalize());
      msg.Reset(INIT_ERR_LEN);
      warn++;
    case TYPE_MATCH_OK: 
      break;
    }
  }
  return (warn > 0) ? TYPE_MATCH_WARNING : TYPE_MATCH_OK;
}
  
