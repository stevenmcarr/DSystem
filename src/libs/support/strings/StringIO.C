/* $Id: StringIO.C,v 1.1 1997/03/11 14:37:30 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//*****************************************************************
//                                                           
// StringIO 
//
//   simple interface to read/write ascii strings 
//
// Author: John Mellor-Crummey                         Sept 1993
//                                                           
// Copyright 1993, Rice University
//                                                           
//*****************************************************************

#include <stdlib.h>
#include <libs/support/file/FormattedFile.h>
#include <libs/support/strings/rn_string.h>
#include <libs/support/strings/StringIO.h>

int ReadString(char **string, FormattedFile *file, uint maxlen)
{
  char *buffer = (char *) malloc(maxlen);

  int code = file->Read(buffer, maxlen);
  if (code == 0) {
    *string = ssave(buffer);
  }

  free(buffer);
  return code;
}


int WriteString(const char *name, FormattedFile *file, uint maxlen)
{
  return file->Write(name, maxlen);
}
