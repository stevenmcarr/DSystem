/* $Id: AttributedFile.h,v 1.1 1997/03/11 14:27:43 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// AttributedFile.h
//
// Author: John Mellor-Crummey                                February 1994
//
// Copyright 1994, Rice University
//***************************************************************************

#ifndef AttributedFile_h
#define AttributedFile_h

#ifndef Object_h
#include <libs/graphicInterface/framework/CObject.h>
#endif

#ifndef ClassName_h
#include <include/ClassName.h>
#endif


class AttributeTables;
class AttributeConstructorRegistry;
class Attribute;

//--------------------------------------------------------------------
// enum CacheFlushSpec
//--------------------------------------------------------------------
enum CacheFlushSpec {CACHE_FLUSH_LRU, CACHE_FLUSH_IMMEDIATE};


//--------------------------------------------------------------------
// class AttributedFile
//--------------------------------------------------------------------
class AttributedFile : public Object {
private:
  struct AttributedFileS *attributedFileRepr;

protected:
  AttributedFile();

  //--------------------------------------------------------------------
  // AttributedFile uses this to get access to constructors for 
  // Attributes of each derived class
  //--------------------------------------------------------------------
  virtual AttributeConstructorRegistry *GetAttributeConstructorRegistry() = 0;

  //--------------------------------------------------------------------
  // attach Attribute without timestamp checking if checkTimeStamps == 0
  //--------------------------------------------------------------------
  Attribute *AttachAttribute(const char *attrName, int checkTimeStamps);

  //--------------------------------------------------------------------
  // when the timestamp of an AttributedFile is changed via 
  // ResetTimeStamp, all attributes are marked to need saving since
  // any existing copy in the disk cache is invalidated by the timestamp
  // change
  //--------------------------------------------------------------------
  void AttributesNeedSaving();

public:
  virtual ~AttributedFile();

  const char *const ReferenceFilePathName();

  CLASS_NAME_FDEF(AttributedFile) = 0;

  // last modification time is 0 if file does not exist
  virtual time_t GetLastModificationTime();
  void ResetTimeStamp();

  virtual int Open(const char *filePathName, AttributedFile *parent = 0);
  int Rename(const char *filePathName);
  virtual int Close();

  Attribute *AttachAttribute(const char *attrName);

  int DetachAttribute(Attribute *attr, CacheFlushSpec cfs = CACHE_FLUSH_LRU);
  int DetachAttributeIfUnreferenced(const char *attrName);

  int SaveAttribute(Attribute *attr); 
  //   zero: success, negative: error, positive: saving disabled

  int SaveAllNewAttributes();
  //   zero: success, negative: error, positive: saving disabled

  void DisableAttributeCaching();
  void EnableAttributeCaching();
  int AttributeCachingEnabled(); // non-zero if true

  unsigned int GetRevision();

  void Changed(int kind, void *change);
  void NoteChange(Object *ob, int kind, void *change);

  //--------------------------------------------------------------------
  // interface for externally managed attributes 
  //--------------------------------------------------------------------
  File *CreateExternalAttributeFile(const char *attrName); 
  File *GetExternalAttributeFile(const char *attrName, int checkTimeStamps);
  void CloseExternalAttributeFile(File *attrFile);
};


//--------------------------------------------------------------------
// class RepositoryObjectComponentsIterator
//
//   enumerate components of a repository object
//--------------------------------------------------------------------
class RepositoryObjectComponentsIterator {
public:
  RepositoryObjectComponentsIterator(AttributedFile *);
  ~RepositoryObjectComponentsIterator();

  char *operator++();
  char *Current();
  void Reset();
};

#define ATTACH_ATTRIBUTE(attrFilePtr, attrClassName) \
((attrClassName *) ((attrFilePtr)->AttachAttribute(CLASS_NAME(attrClassName))))

#endif
