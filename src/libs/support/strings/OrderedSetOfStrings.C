/* $Id: OrderedSetOfStrings.C,v 1.4 1997/03/11 14:37:29 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//******************************************************************
// OrderedSetOfStrings.C: 
//
//  a vector of strings that can be read and written to a file
//
// Author: 
//   John Mellor-Crummey                              
//
// Copyright 1993, Rice University
//******************************************************************


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <libs/support/strings/rn_string.h>
#include <libs/support/strings/OrderedSetOfStrings.h>
#include <libs/support/file/FormattedFile.h>
#include <libs/support/msgHandlers/DumpMsgHandler.h>

#define OSS_MAX_EXTERNAL_STRING_LEN 1024


OrderedSetOfStrings::OrderedSetOfStrings(Boolean destructorDeallocateStrings)
{
  deallocateStringsAtDestruction = destructorDeallocateStrings;
}

OrderedSetOfStrings::~OrderedSetOfStrings()
{
  if (deallocateStringsAtDestruction) DeallocateStrings();
}

void OrderedSetOfStrings::Append(char *string)
{
  PointerVector::Append(string);
}

char *OrderedSetOfStrings::operator [] (int sindex) const
{
  return (char *) Entry((unsigned int) sindex);
}

unsigned int OrderedSetOfStrings::Size() const
{
  return PointerVector::NumberOfEntries();
}

void OrderedSetOfStrings::Print() const
{
  int nstrings = Size();
  for(unsigned int i = 0; i < nstrings; i++)
    fprintf(stderr,"%s\n", Entry(i));
}


void OrderedSetOfStrings::Dump() const
{
  int nstrings = Size();
  for(unsigned int i = 0; i < nstrings; i++)
    dumpHandler.Dump("%s\n", Entry(i));
}

int OrderedSetOfStrings::Write(FormattedFile *file) const
{
  int nstrings = Size();
  int code = file->Write(nstrings);
  if (code) return code;
  for(unsigned int i = 0; i < nstrings; i++) {
    code = file->Write((char *) Entry(i), OSS_MAX_EXTERNAL_STRING_LEN);
    if (code) return code;
  }
  return 0;
}


int OrderedSetOfStrings::Read(FormattedFile *file)
{
  char buffer[OSS_MAX_EXTERNAL_STRING_LEN];
  unsigned int nstrings;

  int code = file->Read(nstrings);
  if (code) return code;
  for(; nstrings-- > 0;) {
    code = file->Read(buffer, OSS_MAX_EXTERNAL_STRING_LEN);
    if (code) return code;
    Append(ssave(buffer));
  }	
  return 0;
}

void OrderedSetOfStrings::DeallocateStrings()
{
  int nstrings = Size();
  for(unsigned int i = 0; i < nstrings; i++)
    sfree((char *) Entry(i));
  ReInitialize();
}

void OrderedSetOfStrings::DestructorDeallocateStrings(Boolean doit)
{
  deallocateStringsAtDestruction = doit;
}





