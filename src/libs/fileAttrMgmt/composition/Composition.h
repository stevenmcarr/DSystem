/* $Id: Composition.h,v 1.2 1997/03/27 20:31:26 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef Composition_h
#define Composition_h

#ifdef __cplusplus

// ******************************************************************
// Composition.h: 
//
//  representation of a composition, a (sub)set of files that forms  
//  a program
//
// Author: 
//   John Mellor-Crummey                              October 1993
//
// Copyright 1993, Rice University
// ******************************************************************

#include <sys/types.h>

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef AttributedFile_h
#include <libs/fileAttrMgmt/attributedFile/AttributedFile.h>
#endif 

#ifndef AttributeConstructorRegistry_h
#include <libs/fileAttrMgmt/attributedFile/AttributeConstructorRegistry.h>
#endif


#ifndef AttributedFileSet_h
#include <libs/fileAttrMgmt/attributedFile/AttributedFileSet.h>
#endif

#ifndef OrderedSetOfStrings_h
#include <libs/support/strings/OrderedSetOfStrings.h>
#endif

#ifndef NeedProvCompAttr_h
#include <libs/fileAttrMgmt/composition/NeedProvCompAttr.h>
#endif

#define COMPOSITION_FILE_SUFFIX ".comp"

class AttributeConstructor;
class AttributeConstructorRegistry;
class FileSuffixHandle;
class FileSuffixRegistry;
class Module;


#define COMPOSITION_OPEN_OK 0
#define COMPOSITION_OPEN_NOCACHE  -1
#define COMPOSITION_OPEN_MISSING_PARTS  -2


class CompositionS {
public:
  NeedProvCompAttr *npcAttr;
  CompositionS() { npcAttr = 0; };
  ~CompositionS() { 
    if (npcAttr) npcAttr->uplinkToFile->DetachAttribute(npcAttr);
  };
};


// ***************************************************************************
// class Composition
// ***************************************************************************
class Composition : public AttributedFile, private AttributedFileSet {
private:
  CompositionS *attributedCompositionRepr;


  //--------------------------------------------------------------------
  // AttributedFile uses this to get access to constructors for 
  // Composition Attributes
  //--------------------------------------------------------------------
  AttributeConstructorRegistry *GetAttributeConstructorRegistry();

  //--------------------------------------------------------------------
  // AttributedFileSet uses this to get access to constructors for files
  //--------------------------------------------------------------------
  FileSuffixRegistry *GetFileSuffixRegistry();

  int OpenComponents();

public:

  static AttributeConstructorRegistry *compAttrConstructorRegistry;
  static FileSuffixRegistry *compFileSuffixRegistry;

  //--------------------------------------------------------------------
  // interface to register a constructor to synthesize an AttributedFile 
  // from a file suffix
  //--------------------------------------------------------------------
  static void RegisterFileSuffixHandle(FileSuffixHandle *fsh);

  //--------------------------------------------------------------------
  // components listed in the composition file
  //--------------------------------------------------------------------
  const OrderedSetOfStrings componentNames;

  // const AttributedFileSet components;

  //--------------------------------------------------------------------
  // error messages and warnings collected when building composition
  //--------------------------------------------------------------------
  const OrderedSetOfStrings errors;
  const OrderedSetOfStrings warnings;

  Composition();
  ~Composition();

  CLASS_NAME_FDEF(Composition);

  virtual int Open(const char *filePathName, AttributedFile *parent = 0);
  virtual int Close();

  virtual time_t GetLastModificationTime();

  Boolean IsCompleteAndConsistent() const;

#if 0
  //--------------------------------------------------------------------
  // interface to register a constructor for each Attribute available
  // for a Composition
  //--------------------------------------------------------------------
  static void RegisterAttributeConstructor(AttributeConstructor *attrConstructor);
#endif

  //-------------------------------------------------------------------------------
  // Components: 
  //  on success, components returns an OrderedSetOfStrings containing
  //  the file name of each of the components of the composition
  //     
  // NOTES: 
  //  (1) NULL is returned if a non-existent file encountered. the name of the
  //      non existent file (the composition itself or one of its components) 
  //      will have been reported by a call to bfcb. 
  //  (2) it is the responsibility of the client of this interface to deallocate
  //      the returned OrderedSetOfStrings with the delete operator
  //  (3) file names passed to bfcb are overwritten after the call; if the client
  //      wants them to persist beyond returning from bfcb, they must be saved in bfcb
  //-------------------------------------------------------------------------------
  static OrderedSetOfStrings *Components(const char *compFileName); 

  static Boolean IsComposition(const char *fileName);
  static Boolean IsComposition(AttributedFile *attrFile);

  // return full paths of modules whose suffix matches the query
  OrderedSetOfStrings *LookupModuleNameBySuffix(const char *name);
  Module *GetModule(const char *name);

friend class CompComponentsIterator;
};

CLASS_NAME_EDEF(Composition);

#define REGISTER_COMPOSITION_ATTRIBUTE(classname) \
CLASS_NAME_EIMPL(classname); \
REGISTER_ATTRIBUTE_CONSTRUCTOR(CLASS_NAME(classname), classname,  \
			       Composition::compAttrConstructorRegistry);

#else

typedef void *Composition;

#endif

#endif

