/* $Id: StringBuffer.C,v 1.6 1997/03/11 14:37:30 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//************************************************************************
// StringBuffer.C -- a simple utility for building strings of arbitrary
//                   length through a sequence of Append operations.
//
// Author:
//   John Mellor-Crummey                                 February 1993
//
// Copyright 1993 as part of the ParaScope Programming Environment 
// project, Rice University.
//************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <libs/support/strings/StringBuffer.h>

static char *carefulRealloc(char *buffer, unsigned int newSize);

StringBuffer::StringBuffer(const unsigned int init_len) 
{ 
  buffer = 0;
  Reset(init_len);
}

StringBuffer::~StringBuffer()
{ 
  if (buffer) free(buffer);
}


void StringBuffer::Reset(const unsigned int init_len) 
{
  buflen = init_len;
  slen = 0; 
  if (buffer) free(buffer);
  buffer = (char *) malloc(init_len);
}

void StringBuffer::GetSpace(const int howmuch)
{
  int needed = howmuch + slen;
  if (needed >= buflen) {
    int buflen2 = buflen << 1;
    buflen = (needed < buflen2) ? buflen2 : needed + buflen;
    buffer = carefulRealloc(buffer, buflen);
  }
}

void StringBuffer::Append(const char *format, ...)
{
  char local_buffer[1024];
  va_list va_args;
  va_start(va_args, format);

  vsprintf(local_buffer, format, va_args);

  unsigned int newlen = strlen(local_buffer);
  GetSpace(newlen);
  strcpy(&(buffer[slen]), local_buffer);
  slen += newlen;
}

void StringBuffer::Append(const char c)
{
  GetSpace(1);
  buffer[slen++] = c;
  buffer[slen] = '\0';
}

void StringBuffer::Append(const StringBuffer *buf)
{
 Append("%s", buf->buffer);
}

char *StringBuffer::Flush()
{
  char *string;

  // ensure that after finalization string occupies no more space than
  // needed --JMC 1/93
  if (buflen > slen) {
    // allocate new string buffer of smallest possible size
    string = (char*)malloc(slen + 1);
    if (slen > 0) strcpy(string, buffer);
    else string[0] = 0;
    free(buffer); // free oversize buffer
    buffer = string;
  } else string = buffer;

  buffer = 0; // ensure buffer will not be freed by destructor
  Reset();
  return string;
}

char *StringBuffer::Finalize()
{
  return Flush();
}


static char *carefulRealloc(char *buffer, unsigned int newSize)
{
	assert(buffer);
	char *newbuf = (char *) realloc(buffer, newSize);
	assert(newbuf);

	return newbuf;
}
