/* $Id: Cookie.C,v 1.1 1997/03/11 14:36:34 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//******************************************************************
// Cookie.C:
//
//  read/write a file marker. presence of the marker at the end of a 
//  file section when reading indicates that the file section was 
//  previously written in its entirety
//
// Author: 
//   John Mellor-Crummey                              October 1993
//
// Copyright 1993, Rice University
//******************************************************************

#include <libs/support/file/Cookie.h>
#include <libs/support/file/File.h>

#define MAGIC_COOKIE 0x12345678 // file marker value

int ReadCookie(File *file)
{
  int magic_cookie;  
  int code = file->Read(&magic_cookie, sizeof(magic_cookie));
  if (code || (magic_cookie != MAGIC_COOKIE)) return -1;
  return 0;
}

int WriteCookie(File *file)
{
  int magic_cookie = MAGIC_COOKIE;
  return file->Write(&magic_cookie, sizeof(magic_cookie));
}
