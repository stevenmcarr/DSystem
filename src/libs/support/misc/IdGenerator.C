/* $Id: IdGenerator.C,v 1.2 1997/06/25 15:16:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//*******************************************************************
// IdGenerator.C
//
// Author: John Mellor-Crummey                             July 1994
//
// Copyright 1994, Rice University
//*******************************************************************


#include <assert.h>

#include <libs/support/sets/WordSet.h>
#include <libs/support/file/FormattedFile.h>

#include <libs/support/misc/IdGenerator.h>



//*******************************************************************
// class IdGenerator interface operations
//*******************************************************************

IdGenerator::IdGenerator()
{
  freedIds = 0;
}


IdGenerator::~IdGenerator()
{
}


void IdGenerator::Create()
{
  idHighWaterMark = 0;
  freedIds = new WordSet;
}


void IdGenerator::Destroy()
{
  delete freedIds;
  freedIds = 0; // clear pointer so operation is idempotent
}


unsigned int IdGenerator::GetHighWaterMark()
{
  assert(freedIds != NULL); // active instance

  return idHighWaterMark;
}


unsigned int IdGenerator::AcquireId()
{
  unsigned int id;
  assert(freedIds != NULL);  // active instance

  unsigned long freedCount = freedIds->NumberOfEntries();
  if (freedCount > 0) {
    id = (unsigned int) 
      freedIds->GetEntryByIndex((unsigned int) (freedCount - 1));
    freedIds->Delete(id);
  } else id = idHighWaterMark++;
  return id;
}


void IdGenerator::ReleaseId(unsigned int id)
{
  assert(freedIds != NULL);  // active instance

  freedIds->Add(id);
}


int IdGenerator::Read(FormattedFile *file)
{
  assert(idHighWaterMark == 0); // cannot be in use
  return file->Read(idHighWaterMark) || freedIds->Read(file);
}


int IdGenerator::Write(FormattedFile *file)
{
  return file->Write(idHighWaterMark) || freedIds->Write(file);
}
