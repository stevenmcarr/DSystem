/* $Id: Attribute.C,v 1.1 1997/03/11 14:27:42 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// Attribute.C
//
// Author: John Mellor-Crummey                                October 1993
//
// Copyright 1993, Rice University
//***************************************************************************


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <libs/fileAttrMgmt/attributedFile/Attribute.h>
#include <libs/fileAttrMgmt/attributedFile/AttributedFile.h>


//***********************************************************************
// interface operations for class Attribute
//***********************************************************************

//--------------------------------------------------------------------
// constructor/destructor
//--------------------------------------------------------------------

#if 0
Attribute::Attribute(const char *const name, AttributedFile *uplink) :
NamedObject(name), referenceCount(0), uplinkToFile(uplink)
{
  ClearNeedsSaving();

  // request notification of changes to the AttributedFile
  uplinkToFile->Notify(this, true);
}
#endif

Attribute::Attribute() : NamedObject(0), referenceCount(0), uplinkToFile(0)
{
  ClearNeedsSaving();
}

void Attribute::Construct(const char *const _name, AttributedFile *uplink)
{
  NamedObject::Create(_name);
  uplinkToFile = uplink;

  // request notification of changes to the AttributedFile
  uplinkToFile->Notify(this, true);
  Create();
}


Attribute::~Attribute()
{
  // stop change notification from the AttributedFile
  uplinkToFile->Notify(this, false);
}


//--------------------------------------------------------------------
// attribute synthesis
//--------------------------------------------------------------------

int Attribute::Compute()
{
  MarkRevisionCurrent();

  int code = ComputeUpCall();
  if (code == 0) SetNeedsSaving();
  else ClearNeedsSaving();

  if (code < 0) {
    fprintf(stderr,"Attribute::Compute: failed to synthesize attribute \"%s\"\
for \"%s\"\n",this->name, uplinkToFile->ReferenceFilePathName());
  }
  return code;
}


int Attribute::ReCompute()
{
  MarkRevisionCurrent();

  int code = ReComputeUpCall();
  if (code == 0) SetNeedsSaving();
  else ClearNeedsSaving();

  if (code < 0) {
    fprintf(stderr,"Attribute::ReCompute: failed to synthesize attribute \"%s\"\
for \"%s\"\n",this->name, uplinkToFile->ReferenceFilePathName());
  }
  return code;
}


int Attribute::ReComputeUpCall()
{
  Destroy();
  return Create() || ComputeUpCall();
}

//--------------------------------------------------------------------
// save status management
//--------------------------------------------------------------------

int Attribute::NeedsSaving()
{
  return needsSaving;
}


void Attribute::SetNeedsSaving()
{
  needsSaving = 1;
}


void Attribute::ClearNeedsSaving()
{
  needsSaving = 0;
}


//--------------------------------------------------------------------
// revision status management
//--------------------------------------------------------------------

void Attribute::MarkRevisionCurrent()
{
  revision = uplinkToFile->GetRevision();
}


unsigned int Attribute::GetRevision()
{
  return revision;
}


// supply a default version so no extra effort is required when developing 
// clients that do not use software versioning 
int Attribute::SoftwareVersionId()
{
  return 0;
}

//--------------------------------------------------------------------
// change notification
//--------------------------------------------------------------------

void Attribute::NoteChange(Object *, int, void *) 
{
  // no action to be taken for changes in general
}


 
//--------------------------------------------------------------------
// reference count management
//--------------------------------------------------------------------

void Attribute::Attach()
{
  referenceCount++;
}


// reference count may drop to -1
int Attribute::Detach()
{
  //return ((referenceCount > 0) ? --referenceCount : 0);
  return --referenceCount;
}


int Attribute::RefCount()
{
  return referenceCount;
}


void Attribute::DetachUpCall()
{
}


//--------------------------------------------------------------------
// I/O
//--------------------------------------------------------------------

int Attribute::Write(File *file)
{
  ClearNeedsSaving();
  return WriteUpCall(file);
}


int Attribute::WriteUpCall(File *)
{
  return 1;
}


int Attribute::Read(File *file)
{
  MarkRevisionCurrent();

  int code = ReadUpCall(file);
  if (code == 0) ClearNeedsSaving();
  return code;
}


int Attribute::ReadUpCall(File *)
{
  return 1;
}


//--------------------------------------------------------------------
// debugging support
//--------------------------------------------------------------------

void Attribute::Dump()
{
  fprintf(stderr, "Attribute: %s rc=%d, ul=%x\n", name, referenceCount, 
	  uplinkToFile);
  DumpUpCall();
}


void Attribute::DumpUpCall()
{
}

