/* $Id: Composition.C,v 1.2 1997/03/27 20:31:26 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//******************************************************************
// Composition.C: 
//
//  representation of a composition, a (sub)set of files that forms  
//  a program
//
// Author: 
//   John Mellor-Crummey                              October 1993
//
// Copyright 1993, Rice University
//******************************************************************

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/param.h>


#include <libs/support/file/File.h>
#include <libs/support/strings/OrderedSetOfStrings.h>
#include <libs/support/strings/StringBuffer.h>
#include <libs/support/file/FileUtilities.h>
#include <libs/support/strings/rn_string.h>
#include <libs/support/msgHandlers/TraceMsgHandler.h>
#include <libs/support/msgHandlers/ErrorMsgHandler.h>

#include <libs/frontEnd/include/NeedProvSet.h>

#include <libs/fileAttrMgmt/attributedFile/FileSuffixRegistry.h>

#include <libs/fileAttrMgmt/module/Module.h>

#include <libs/fileAttrMgmt/composition/Composition.h>
#include <libs/fileAttrMgmt/composition/CompositionIterators.h>
#include <libs/fileAttrMgmt/composition/NeedProvCompAttr.h>
#include <libs/fileAttrMgmt/composition/ProcModuleMap.h>

//*******************************************************************
// declarations 
//*******************************************************************

//**********************
// forward declarations
//**********************

static Boolean ComponentsInternal(const char *compFileName, OrderedSetOfStrings *oss); 

static time_t GetLastModificationTimeStatic
(const char *compName, const OrderedSetOfStrings *fileSet); 

static time_t time_t_max(time_t t1, time_t t2);



//***************************************************************************
// class Composition interface operations
//***************************************************************************

CLASS_NAME_IMPL(Composition)

Composition::Composition() :  
componentNames(true), errors(true), warnings(true)
{ 
  attributedCompositionRepr = 0;
}


Composition::~Composition()
{
  if (attributedCompositionRepr != 0) Close();
}


Composition::Close()
{
  delete attributedCompositionRepr;
  attributedCompositionRepr = 0;
  
  return AttributedFile::Close();
}


int Composition::Open(const char *filePathName, 
				AttributedFile *parent)
{ 
  int code = COMPOSITION_OPEN_OK;

  traceMsgHandler.HandleMsg(1, "Opening composition %s ...\n", 
			    filePathName);
  traceMsgHandler.BeginScope();
  
  assert(attributedCompositionRepr == 0);
  attributedCompositionRepr = new CompositionS;
  
  if (ComponentsInternal(filePathName, 
			 (OrderedSetOfStrings *) &componentNames) == true) {

    errorMsgHandler.HandleMsg("Unable to open composition %s.\n",
			      filePathName);

    code = COMPOSITION_OPEN_MISSING_PARTS;
  } else {
    if (OpenComponents() == 0) {
      if (AttributedFile::Open(filePathName, parent) == -1) {
	delete attributedCompositionRepr;
	attributedCompositionRepr = 0;
	code = COMPOSITION_OPEN_NOCACHE;
      } else {
	attributedCompositionRepr->npcAttr = 
	  (NeedProvCompAttr *) AttachAttribute(CLASS_NAME(NeedProvCompAttr));
      }
    }
  }

  traceMsgHandler.EndScope();
  
  return code;
}


int Composition::OpenComponents()
{
  traceMsgHandler.HandleMsg
    (1, "Begin opening composition components ...\n"); 

  int code = 0;
  OrderedSetOfStrings *errs = (OrderedSetOfStrings *) &errors; // un-const 
  StringBuffer error(256); // temporary space for building error messages
  
  int i, numComponents = componentNames.Size();
  for (i=0; i < numComponents; i++) {
    AttributedFile *file = OpenFile(componentNames[i]);
    if (file) {
      // arrange for composition to be notified of changes to its components 
      file->Notify(this, true);
    } else {
      errorMsgHandler.HandleMsg("Unable to open composition component %s.", 
				 componentNames[i]);
      code = -1;
    }
  }

  traceMsgHandler.HandleMsg
    (1, "Finished opening composition components.\n", 
     this->ReferenceFilePathName());

  return code;
}


Boolean Composition::IsCompleteAndConsistent() const
{
  NeedProvCompAttr *npcAttr = attributedCompositionRepr->npcAttr; 
  return BOOL(npcAttr->programName != 0 && npcAttr->consistent == true);
}


Boolean Composition::IsComposition(AttributedFile *attrFile)
{
  return BOOL(attrFile->ClassName() == CLASS_NAME(Composition));
}


Boolean Composition::IsComposition(const char *fileName)
{
  int len = strlen(fileName);
  int suffixLen = strlen(COMPOSITION_FILE_SUFFIX);
  return BOOL(strcmp(COMPOSITION_FILE_SUFFIX, &fileName[len - suffixLen])==0);
}


OrderedSetOfStrings *Composition::Components(const char *compFileName)
{
  // oss constructor argument indicates sfree string contents on destruction
  OrderedSetOfStrings  *oss = new OrderedSetOfStrings(true); 
  ComponentsInternal(compFileName, oss);
  return oss;
}


time_t Composition::GetLastModificationTime()
{
  time_t lmtime = FileLastModificationTime(ReferenceFilePathName());
  if (lmtime == 0) return 0;
  
  CompComponentsIterator components(this); 
  AttributedFile *attrFile;
  for (; attrFile = components.Current(); ++components) {
    time_t tmp = attrFile->GetLastModificationTime();
    if (tmp == 0) return 0;
    else lmtime = time_t_max(tmp, lmtime);
  } 
  return lmtime;
}


Module *Composition::GetModule(const char *name)
{
  Module *module = (Module *) GetFile(name);

  if (module) // module is component of this composition
    return module; 
  else {
    // the hard way -- module is component of nested composition
    CompModulesIterator modules(this);
    for(; module = modules.Current(); ++modules)
      if (strcmp(module->ReferenceFilePathName(), name) == 0) return module;
  }

  assert(0);
  return 0;
}


//============================================================================
// attribute constructor registry management
//============================================================================

AttributeConstructorRegistry *Composition::compAttrConstructorRegistry = 0;


AttributeConstructorRegistry *
Composition::GetAttributeConstructorRegistry()
{
  return compAttrConstructorRegistry;
}


#if 0
void Composition::RegisterAttributeConstructor
(AttributeConstructor *attrConstructor)
{
  if (!compAttrConstructorRegistry)
    compAttrConstructorRegistry = new AttributeConstructorRegistry;  
  compAttrConstructorRegistry->Register(attrConstructor);
}
#endif

// return full paths of modules whose suffix matches the query
OrderedSetOfStrings *Composition::LookupModuleNameBySuffix(const char *name)
{
  int nameLen = strlen(name);
  OrderedSetOfStrings *matches = new OrderedSetOfStrings(true);

  CompModulesIterator modules(this);
  Module *module;
  for(; module = modules.Current(); ++modules) {
    const char *modulePath  = module->ReferenceFilePathName();
    int compareOffset = strlen(modulePath) - nameLen;
    if (compareOffset >= 0  && (strcmp(name, modulePath + compareOffset) == 0))
      matches->Append(ssave(modulePath));
  }

#if 0
  if (matches->NumberOfEntries() == 0) {
    delete matches;
    matches = 0;
  }
#endif

  return matches;
}



//***************************************************************************
// private operations 
//***************************************************************************

//===========================================================================
// static Boolean ComponentsInternal
//
// parse a composition file into an OrderedSetOfStrings containing the
// names of all of the components of the composition
//
// error handling: true is returned on error, and a call back occurs
// for each component of the composition that does not exist 
//===========================================================================

static Boolean ComponentsInternal(const char *compFileName, OrderedSetOfStrings *oss)
{
  Boolean error = false;
  
  // open the composition file
  File compFile;
  int code = compFile.Open(compFileName, "r");
  
  if (code) return true;
  
  char path[MAXPATHLEN];
  char *dir = ssave(FileDirName(compFileName));
  
  while (compFile.Gets(path, MAXPATHLEN) != NULL) {
    int lastCharIndex = strlen(path) - 1;
    
    // chop off a newline, if any 
    if (path[lastCharIndex] == '\n') path[lastCharIndex] = '\0';
    
    char *file = FullPath(dir, path);
    
    oss->Append(ssave(file));
  }
  
  sfree(dir);
  
  return error;
}


//===========================================================================
// static time_t time_t_max
//
// compute the max of a pair of time_t 
//===========================================================================
static time_t time_t_max(time_t t1, time_t t2)
{
  return (t1 > t2) ? t1 : t2;
}
