/* $Id: Attribute.h,v 1.1 1997/03/11 14:27:42 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// Attribute.h
//
// Author: John Mellor-Crummey                                October 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#ifndef Attribute_h
#define Attribute_h

#include <sys/types.h>

#ifndef ClassName_h
#include <include/ClassName.h>
#endif

#ifndef NamedObject_h
#include <libs/support/tables/namedObject/NamedObject.h>
#endif

#ifndef Object_h
#include <libs/graphicInterface/framework/CObject.h>
#endif

#ifndef RegistryObject_h
#include <libs/support/registry/RegistryObject.h>
#endif

class File;
class AttributedFile;


//********************************************************************
// class Attribute
//********************************************************************
class Attribute : public Object, public NamedObject, public RegistryObject {
private:
  time_t creationTime;
  uint referenceCount;
  int needsSaving;
protected:
  int revision;
public:
  AttributedFile *uplinkToFile;

  //--------------------------------------------------------------------
  // constructor/destructor
  //--------------------------------------------------------------------
  Attribute();
  void Construct(const char *const _name, AttributedFile *uplink);

  virtual ~Attribute();
  
  //--------------------------------------------------------------------
  // when an attribute is to be computed, the compute method is the 
  // method of choice. this invokes ComputeUpCall and upon success
  // sets the needsSaving flag which will indicates the attribute is
  // newer than the copy in the disk cache
  //--------------------------------------------------------------------
  int Compute();
  int ReCompute();

  //--------------------------------------------------------------------
  // each attribute must define a ComputeUpCall method that
  // computes the attribute value; a return value of zero indicates
  // success. 
  //--------------------------------------------------------------------
  virtual int ComputeUpCall() = 0;
  virtual int ReComputeUpCall();

  virtual int Create() = 0;
  virtual void Destroy() = 0;
  
  virtual void DetachUpCall();

  //--------------------------------------------------------------------
  // support for identifying the software version of an attribute
  // by changing the software version id of an attribute, all old
  // copies of the attribute are painlessly ignored and removed
  // when encountered
  //--------------------------------------------------------------------
  virtual int SoftwareVersionId();
  
  //--------------------------------------------------------------------
  // access creation time
  //--------------------------------------------------------------------
  time_t GetCreationTime();
  
  //--------------------------------------------------------------------
  // reference count management
  //--------------------------------------------------------------------
  void Attach();
  int Detach();
  int RefCount();
  
  //--------------------------------------------------------------------
  // save status management
  //--------------------------------------------------------------------
  void SetNeedsSaving();
  void ClearNeedsSaving();
  int NeedsSaving();
  
  //--------------------------------------------------------------------
  // revision management
  //--------------------------------------------------------------------
  void MarkRevisionCurrent();    // track revision of *uplinkToFile
  unsigned int GetRevision();

  //--------------------------------------------------------------------
  // change notification
  //--------------------------------------------------------------------
  void NoteChange(Object *obj, int kind, void *change);
  
  //--------------------------------------------------------------------
  // I/O
  //   success:       return code = 0 
  //   unimplemented: return code > 0
  //   failure:       return code < 0
  //--------------------------------------------------------------------
  int Read(File *file);
  int Write(File *file); 
  
  virtual int ReadUpCall(File *file);
  virtual int WriteUpCall(File *file); 
  
  //--------------------------------------------------------------------
  // debugging support
  //--------------------------------------------------------------------
  void Dump();
  virtual void DumpUpCall();
};

#endif
