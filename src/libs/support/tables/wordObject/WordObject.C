/* $Id: WordObject.C,v 1.2 1997/03/11 14:37:42 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//*************************************************************************** 
//
// WordObject
//                                                                          
//   base class for Word entities
//
// Author:  John Mellor-Crummey                            June 1993
//
// Copyright 1993, Rice University
//                                                                          
//***************************************************************************

#include <stdio.h>
#include <stdlib.h>

#include <libs/support/file/FormattedFile.h>
#include <libs/support/tables/wordObject/WordObject.h>
#include <libs/support/strings/rn_string.h>
#include <libs/support/strings/StringIO.h>


//***************************************************************************
// class WordObject interface operations
//***************************************************************************

CLASS_NAME_IMPL(WordObject);


WordObject::WordObject(const unsigned int _id) : id(_id)
{
}


void WordObject::Create(const unsigned int _id) 
{
  // sidestep const nature  
  *(int *) &id = *(int *) &_id;
}


WordObject::~WordObject()
{
}


void WordObject::WordObjectDump()
{
  fprintf(stderr, "%s(id = %d)\n", ClassName(), id);
  WordObjectDumpUpCall();
}


void WordObject::WordObjectDumpUpCall()
{
}


//***************************************************************************
// class WordObjectIO interface operations
//***************************************************************************

CLASS_NAME_IMPL(WordObjectIO);


WordObjectIO::WordObjectIO(const unsigned int _id) : WordObject(_id)
{
  
}


WordObjectIO::~WordObjectIO()
{
}


int WordObjectIO::WordObjectRead(FormattedFile *file)
{
  return file->Read(*(unsigned int *) &id) || WordObjectReadUpCall(file);
}

int WordObjectIO::WordObjectReadUpCall(FormattedFile *)
{
  return 0;
}


int WordObjectIO::WordObjectWrite(FormattedFile *file)
{
  return file->Write(id) || WordObjectWriteUpCall(file);
}

int WordObjectIO::WordObjectWriteUpCall(FormattedFile *)
{
  return 0;
}


