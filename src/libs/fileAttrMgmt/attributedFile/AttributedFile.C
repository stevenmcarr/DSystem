/* $Id: AttributedFile.C,v 1.3 1997/06/24 17:37:32 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//****************************************************************************
// AttributedFile
//
//   basis for demand driven computation of attributes for a reference file
//
// Author: John Mellor-Crummey                                     June 1993
//
// Copyright 1993, Rice University
//
//****************************************************************************



#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/param.h>

#ifdef OSF1
#include <include/realpath.h>
#endif

#include <libs/support/msgHandlers/Changes.h>

#include <libs/support/strings/rn_string.h>
#include <libs/support/file/File.h>

#include <libs/fileAttrMgmt/attributedFile/AttributeConstructorRegistry.h>

#include <libs/fileAttrMgmt/attributedFile/AttributedFile.h>
#include <libs/fileAttrMgmt/attributedFile/AttributeTable.h>
#include <libs/fileAttrMgmt/attributedFile/Attribute.h>
#include <libs/fileAttrMgmt/attributedFile/RepositoryContext.i>



//****************************************************************************
// declarations
//****************************************************************************

// prefix for invisible timestamp files
#define CTL_PREFIX      ".:"     

// prefix for attribute files to avoid name conflicts with timestamp files
#define ATTR_PREFIX    ":"     


//****************************************************************************
// implementation of class AttributedFile
//****************************************************************************

AttributedFile::AttributedFile()
{
  attributedFileRepr = 0;
}


AttributedFile::~AttributedFile()
{
  if (attributedFileRepr) {
    Close();
  }
}


time_t AttributedFile::GetLastModificationTime()
{
  return 0;
}


void AttributedFile::ResetTimeStamp()
{
  time_t oldTime = attributedFileRepr->timeStamp.GetTime();

  attributedFileRepr->timeStamp.Init
    (attributedFileRepr->referenceFilePathName);

  time_t newTime = GetLastModificationTime();
  if (newTime != 0) attributedFileRepr->timeStamp.SetTime(newTime);

  if (oldTime != attributedFileRepr->timeStamp.GetTime())
    AttributesNeedSaving();
}


int AttributedFile::Open(const char *filePathName, AttributedFile *parent)
{
  assert(attributedFileRepr == 0);

  char canonicalAbsolutePath[MAXPATHLEN];

  //----------------------------------------------------------------------
  // the cast below is safe since realpath does not modify its first
  // argument -- JMC 10/93
  //----------------------------------------------------------------------
  if (realpath((char *) filePathName, canonicalAbsolutePath) == 0) {
    perror("AttributedFile::Open can't compute reference path");
    return -1;
  }

  attributedFileRepr = new AttributedFileS;
  attributedFileRepr->flushOnDetachIfPositive = 0; 
  attributedFileRepr->revision = 0; 
  attributedFileRepr->closeInProgress = 0;   // true only in Close operation
  attributedFileRepr->parent = parent;
  attributedFileRepr->attributeCachingEnabledIfZero = 0; 

  attributedFileRepr->referenceFilePathName = ssave(canonicalAbsolutePath);

  int code = 
    attributedFileRepr->reposContext.Open
    (attributedFileRepr->referenceFilePathName, 
     (parent ? &parent->attributedFileRepr->reposContext : 0));

  if (code) { 
    delete attributedFileRepr;
    attributedFileRepr = 0;
  } else {
    attributedFileRepr->attrTable = new AttributeTable;
    ResetTimeStamp();
  }

  return code;
}


int AttributedFile::Close()
{
  int code = 0;
  
  if (attributedFileRepr) {
    // signal to flush detached attributes immediately
    attributedFileRepr->closeInProgress = 1;      
    
    delete attributedFileRepr->attrTable;
    
    code = attributedFileRepr->reposContext.Close();
    sfree(attributedFileRepr->referenceFilePathName);

    delete attributedFileRepr;
    attributedFileRepr = 0;
  }

  return code;
}


const char *const AttributedFile::ReferenceFilePathName()
{
  if (attributedFileRepr) 
    return (const char *const) attributedFileRepr->referenceFilePathName;
  else return 0;
}


Attribute *AttributedFile::AttachAttribute(const char *attrName)
{
  return AttachAttribute(attrName, 1);
}


Attribute *AttributedFile::AttachAttribute(const char *attrName, 
					   int checkTimeStamps)
{
  if (!attributedFileRepr) {
    fprintf(stderr, 
	    "AttributedFile: unable to attach attribute\n");
    fprintf(stderr, 
	    "since ARO internal representation not initialized.\n");
    return 0;
  }

  Attribute *attribute = attributedFileRepr->attrTable->QueryEntry(attrName);
  
  if (attribute == 0) {
#if 0
    // old style code; replaced by code below
    attribute = GetAttributeConstructorRegistry()->New(this, attrName);
#endif
    attribute = (Attribute *) 
      CONSTRUCT_BY_NAME_FROM_REGISTRY(GetAttributeConstructorRegistry(), 
				      attrName);
    attribute->Construct(attrName, this);
    attribute->Attach();
    attributedFileRepr->attrTable->AddEntry(attribute);
    
    char *ctlFileName = nssave(2, CTL_PREFIX, attrName);
    File *ctlFile = attributedFileRepr->reposContext.GetFile(ctlFileName);
    
    if (ctlFile) {
      int failure = 0;
      int versionId = 0;

      //------------------------------------------------------------------
      // never use attributes from the disk cache if the memory-resident copy
      // of the AttributedFile differs from the disk copy (revision > 0)
      //------------------------------------------------------------------
      if (attributedFileRepr->revision > 0) failure = 1;
      
      //------------------------------------------------------------------
      // check that software version id recorded in control file
      // is identical to the current software version id
      //------------------------------------------------------------------
      if (ctlFile->Read(&versionId, sizeof(versionId)) || 
	  versionId != attribute->SoftwareVersionId()) 
	failure = 1;
      
      if (!failure && checkTimeStamps) {
	//------------------------------------------------------------------
	// check the attribute timestamps for validity against the timestamps
	// of the current AttributedFile and all ancestors
	//------------------------------------------------------------------
	FileTimeStamp myTimeStamp;
	for(AttributedFile *src=this; src; 
	    src = src->attributedFileRepr->parent) {
	  if (myTimeStamp.Read(*ctlFile)) {
#ifdef DEBUG
	    fprintf(stderr, "%s:\terror reading timestamps for attribute `%s'\n",
		    "RepositoryContext::GetAttribute", attrName);
	    fprintf(stderr, 
		    "\tremoving timestamp file and synthesizing attribute\n");
#endif
	    
	    failure = 1; 
	    break;
	  }
	  
	  if (!(myTimeStamp == src->attributedFileRepr->timeStamp)) 	{
	    failure = 1; 
	    break;
	  }
	}
      }
      
      delete ctlFile;
      
      if (failure) {
	// delete file
	attributedFileRepr->reposContext.DestroyFile(ctlFileName);
	goto compute_attribute;
      } 
      
      char *attrFileName = nssave(2, ATTR_PREFIX, attrName);
      File *attrFile = attributedFileRepr->reposContext.GetFile(attrFileName);
      
      if (attrFile) {
	int code = attribute->Read(attrFile);
	delete attrFile;
	if (code) {
	  // delete file
	  attributedFileRepr->reposContext.DestroyFile(attrFileName); 
	  failure = 1;
	}
      } else failure = 1;
      
      sfree(attrFileName);
      if (failure) goto compute_attribute;
      
    } else {
    compute_attribute:
      if (attribute->Compute()) { 
	// compute operation failed, delete attribute and return NULL 
	attribute->Detach();
	attributedFileRepr->attrTable->DeleteEntry(attribute->name);
	attribute = 0;
      } else attribute->SetNeedsSaving();
    }
    if (ctlFileName) sfree(ctlFileName); 
  } else {
    if (attribute->GetRevision() != attributedFileRepr->revision)
      attribute->ReCompute();
    attribute->Attach();
  }

  return attribute;
}


int AttributedFile::SaveAttribute(Attribute *attr)
{
  if (AttributeCachingEnabled()  == 0) return 1; // non-erroneous failure

  if (attr->GetRevision() != GetRevision()) attr->ReCompute();

  if (attr->NeedsSaving()) {
    char *ctlFileName = nssave(2, CTL_PREFIX, attr->name);
    File *ctlFile = attributedFileRepr->reposContext.CreateFile(ctlFileName);
    
    assert(ctlFile != NULL);
    
    //------------------------------------------------------------------
    // record software version id in control file
    //------------------------------------------------------------------
    int versionId = attr->SoftwareVersionId();
    if (ctlFile->Write(&versionId, sizeof(versionId))) {
#ifdef DEBUG
      fprintf(stderr, 
	      "%s: error writing software version id for attribute `%s'\n",
	      "AttributedFile::SaveAttribute", attr->name);
#endif
      delete ctlFile;
      attributedFileRepr->reposContext.DestroyFile(ctlFileName); // delete file
      sfree(ctlFileName);
      return -1;
    }
    
    
    //------------------------------------------------------------------
    // record the timestamp of the current AttributedFile and all its
    // ancestors in the control file
    //------------------------------------------------------------------
    for(AttributedFile *src = this; src; 
	src = src->attributedFileRepr->parent) {
      if (src->attributedFileRepr->timeStamp.Write(*ctlFile)) {
#ifdef DEBUG
	fprintf(stderr, "%s: error writing timestamps for attribute `%s'\n",
		"AttributedFile::SaveAttribute", attr->name);
#endif
	delete ctlFile;
	attributedFileRepr->reposContext.DestroyFile(ctlFileName); // delete file
	sfree(ctlFileName);
	return -1;
      }
    }
    
    delete ctlFile;
    
    char *attrFileName = nssave(2, ATTR_PREFIX, attr->name);
    File *attrFile = attributedFileRepr->reposContext.CreateFile(attrFileName);
    
    assert(attrFile != NULL);
    
    int code;
    if (code = attr->Write(attrFile)) {
      //------------------------------------------------------------------
      // if write is unsuccessful, delete the attribute file and its 
      // associated control file 
      //------------------------------------------------------------------
      attributedFileRepr->reposContext.DestroyFile(attrFileName); // delete file
      attributedFileRepr->reposContext.DestroyFile(ctlFileName); // delete file
    }
    
    delete attrFile;
    
    sfree(attrFileName);
    sfree(ctlFileName);
    
    return code;
  } else return 0;
}


int AttributedFile::SaveAllNewAttributes()
{
  if (AttributeCachingEnabled() == 0) return 1;

  int code = 0;
  AttributeTableIterator attributes(attributedFileRepr->attrTable);
  Attribute *attr;
  for(; attr = attributes.Current(); ++attributes) {
    // save only up-to-date attributes
    if (attr->GetRevision() == GetRevision()) {
      if (SaveAttribute(attr) < 0) {
	fprintf(stderr, "%s: error saving attribute `%s'\n",
		"AttributedFile::SaveAllNewAttributes", attr->name);
	code = -1;
      }
    }
  }
  return code;
}


void AttributedFile::AttributesNeedSaving()
{
  AttributeTableIterator attributes(attributedFileRepr->attrTable);
  Attribute *attr;
  for(; attr = attributes.Current(); ++attributes) attr->SetNeedsSaving();
}


//=========================================================================  
// void AttributedFile::DetachAttribute(Attribute *attr, CacheFlushSpec cfs)
// 
//   detach an attribute from an AttributedFile, destroying the attribute 
//   if its reference count is zero and a condition is satisfied that 
//   warrants its immediate destruction.
//
//   NOTE: the only way we could see a DetachAttribute while closeInProgress 
//   is set is if this is the last remaining AttributedFile object open for
//   this file, and thus (1) the entire attribute table for this 
//   AttributedFile is being emptied, (2) attr was linked to some other 
//   attribute that is now being deleted. the deletion of this other 
//   attribute must have been what invoked the DetachAttribute -- JMC 8/93
//=========================================================================  
int AttributedFile::DetachAttribute(Attribute *attr, CacheFlushSpec cfs)
{
  if (attributedFileRepr == 0 || attr == 0) return -1;

  if (cfs ==CACHE_FLUSH_IMMEDIATE) 
    attributedFileRepr->flushOnDetachIfPositive++;

  //----------------------------------------------------------------------
  // upcall to an attribute requesting that any attributes used to compute
  // attr are deleted if currently unreferenced. this call must be outside
  // the if statement below since attr may be referenced only from some 
  // other attribute that is no longer referenced
  //----------------------------------------------------------------------
  attr->DetachUpCall();

  if (attr->RefCount() == 1) {
    if (attributedFileRepr->closeInProgress && 
	(attributedFileRepr->attrTable->QueryEntry(attr->name) == 0)) {
      //------------------------------------------------------------------
      // attribute no longer in table thus it must be freed explicitly
      //------------------------------------------------------------------
      delete attr;
    } else if (attributedFileRepr->flushOnDetachIfPositive > 0) {
      SaveAttribute(attr);
      attr->Detach();
      attributedFileRepr->attrTable->DeleteEntry(attr->name);
    } else attr->Detach();
  } else attr->Detach();

  if (cfs ==CACHE_FLUSH_IMMEDIATE) 
    attributedFileRepr->flushOnDetachIfPositive--;

  return 0;
}


int AttributedFile::DetachAttributeIfUnreferenced(const char *attrName)
{  
  Attribute *attribute = attributedFileRepr->attrTable->QueryEntry(attrName);

  if (attribute && attribute->RefCount() == 0) {  
    attribute->Attach(); // increase reference count to 1
    return DetachAttribute(attribute, CACHE_FLUSH_IMMEDIATE);
  }
  else return 0;
}


void AttributedFile::DisableAttributeCaching()
{
  if (attributedFileRepr->attributeCachingEnabledIfZero-- == 0) 
    Changed(AttrCachingDisabled, 0);
}


void AttributedFile::EnableAttributeCaching()
{
  if (++(attributedFileRepr->attributeCachingEnabledIfZero) == 0) {
    SaveAllNewAttributes();
    Changed(AttrCachingEnabled, 0);
  }
}


int AttributedFile::AttributeCachingEnabled()
{
  return (attributedFileRepr->attributeCachingEnabledIfZero == 0);
}


unsigned int AttributedFile::GetRevision()
{
  return attributedFileRepr->revision;
}


void AttributedFile::Changed(int kind, void *ob)
{
  if (kind == SrcLineChange) {
    attributedFileRepr->revision++;
    if (AttributeCachingEnabled()) DisableAttributeCaching();
  }
  Object::Changed(kind, ob);
}


void AttributedFile::NoteChange(Object *, int kind, void *)
{
  if (kind == SrcLineChange) {
    Changed(SrcLineChange, 0);
  } else if (kind == AttrCachingEnabled) {
    EnableAttributeCaching();
  } else if (kind == AttrCachingDisabled) {
    DisableAttributeCaching();
  }
}


//******************************************************************


File *AttributedFile::GetExternalAttributeFile(const char *attrName, 
					       int checkTimeStamps)
{
  assert(attributedFileRepr != NULL); 
  
  File *attrFile = 0;
  char *ctlFileName = nssave(2, CTL_PREFIX, attrName);
  File *ctlFile = attributedFileRepr->reposContext.GetFile(ctlFileName);
  
  if (ctlFile) {
    int failure = 0;
    
    if (checkTimeStamps) {
      //------------------------------------------------------------------
      // check the attribute timestamps for validity against the timestamps
      // of the current AttributedFile and all ancestors
      //------------------------------------------------------------------
      FileTimeStamp myTimeStamp;
      for(AttributedFile *src=this; src; 
	  src = src->attributedFileRepr->parent) {
	if (myTimeStamp.Read(*ctlFile)) {
#ifdef DEBUG
	  fprintf(stderr, "%s:\terror reading timestamps for attribute `%s'\n",
		  "RepositoryContext::GetAttribute", attrName);
	  fprintf(stderr, 
		  "\tremoving timestamp file and synthesizing attribute\n");
#endif
	  
	  failure = 1; 
	  break;
	}
	
	if (!(myTimeStamp == src->attributedFileRepr->timeStamp)) 	{
	  failure = 1; 
	  break;
	}
      }
    }
    
    delete ctlFile;
    
    if (failure) {
      // delete file
      attributedFileRepr->reposContext.DestroyFile(ctlFileName);
    } else { 
      char *attrFileName = nssave(2, ATTR_PREFIX, attrName);
      attrFile = attributedFileRepr->reposContext.GetFile(attrFileName);
      sfree(attrFileName);
    }
  }

  sfree(ctlFileName);
  return attrFile;
}



void AttributedFile::CloseExternalAttributeFile(File *attrFile)
{
  delete attrFile;
}



File *AttributedFile::CreateExternalAttributeFile(const char *attrName) 
{
  char *ctlFileName = nssave(2, CTL_PREFIX, attrName);
  File *ctlFile = attributedFileRepr->reposContext.CreateFile(ctlFileName);
  
  assert(ctlFile != NULL);
  
  //------------------------------------------------------------------
  // record the timestamp of the current AttributedFile and all its
  // ancestors in the control file
  //------------------------------------------------------------------
  for(AttributedFile *src = this; src; 
      src = src->attributedFileRepr->parent) {
    if (src->attributedFileRepr->timeStamp.Write(*ctlFile)) {
#ifdef DEBUG
      fprintf(stderr, "%s: error writing timestamps for attribute `%s'\n",
	      "AttributedFile::SaveAttribute", attr->name);
#endif
      delete ctlFile;
      attributedFileRepr->reposContext.DestroyFile(ctlFileName); // delete file
      sfree(ctlFileName);
      return 0;
    }
  }
  
  delete ctlFile;
  sfree(ctlFileName);
  
  char *attrFileName = nssave(2, ATTR_PREFIX, attrName);
  File *attrFile = attributedFileRepr->reposContext.CreateFile(attrFileName);
  sfree(attrFileName);
  
  return attrFile;
}


int AttributedFile::Rename(const char *filePathName)
{
  char canonicalAbsolutePath[MAXPATHLEN];
  AttributedFile *parent = attributedFileRepr->parent;
  
  //----------------------------------------------------------------------
  // the cast below is safe since realpath does not modify its first
  // argument -- JMC 10/93
  //----------------------------------------------------------------------
  if (realpath((char *) filePathName, canonicalAbsolutePath) == 0) {
    perror("AttributedFile::Open can't compute reference path");
    return -1;
  }
  
  char *newRefPathName = ssave(canonicalAbsolutePath);
  
  attributedFileRepr->reposContext.Close();
  int code = attributedFileRepr->reposContext.Open
    (newRefPathName, (parent ? &parent->attributedFileRepr->reposContext : 0));
  
  if (code) { 
    attributedFileRepr->reposContext.Close();
    attributedFileRepr->reposContext.Open
      (attributedFileRepr->referenceFilePathName, 
       (parent ? &parent->attributedFileRepr->reposContext : 0));
    sfree(newRefPathName);
  } else {
    sfree(attributedFileRepr->referenceFilePathName);
    attributedFileRepr->referenceFilePathName = newRefPathName;

    // upon a sucessful rename, assume no attributes already saved
    AttributesNeedSaving();
  }
  
  return code;
}
