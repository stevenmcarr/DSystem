/* $Id: FileUtilities.C,v 1.5 1997/03/11 14:36:37 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//*****************************************************************
// fileutils
//
//   a collection of miscellaneous functions for manipulating
//   Unix files, filenames, etc.
//
// Author: 
//   John Mellor-Crummey                              May 1993
//
// Copyright 1993, Rice University                                
//
//*****************************************************************

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>

#include <libs/support/file/FileUtilities.h>


char *FileDirName(const char *const pathName)
{
  static char dirpath[MAXPATHLEN];
  strcpy(dirpath, pathName);
  
  // condition the path by discarding any trailing slash
  int len = strlen(dirpath);
  if (dirpath[--len] == '/') dirpath[len] = 0;
  
  // find the last slash in the conditioned path
  char *lastSlash = strrchr(dirpath, '/');
  
  if (lastSlash) *lastSlash = 0; // clip the string at the last slash
  else strcpy(dirpath, ".");     // no slash, so "." is the dirname
  
  return dirpath;
}


char *FileBaseName(const char *const pathName)
{
  static char dirpath[MAXPATHLEN];
  strcpy(dirpath, pathName);

  // condition the path by discarding any trailing slash
  int len = strlen(dirpath);
  if (dirpath[--len] == '/') dirpath[len] = 0;

  // find the last slash in the conditioned path
  char *lastSlash = strrchr(dirpath, '/');

  if (lastSlash) return (lastSlash + 1); // return path fragment after the slash
  else return dirpath;     // no slash entire path is the basename
}


int FileExists(const char *const pathName)
{
  struct stat buf;
  return stat(pathName, &buf);
}

// return last modification time of a file or (time_t) 0 
// if the file does not exist
time_t FileLastModificationTime(const char *const pathName)
{
  struct stat buf;
  
  // file does not exist 
  if (stat(pathName, &buf) == -1) return (time_t) 0;
  
  return buf.st_mtime;
}


char *FullPath(const char *dir, const char *file)
{
  static char buffer[MAXPATHLEN];

  switch(*file) {
  case '/': case '~': 
    strcpy(buffer, file);
    break;
  default: 
    sprintf(buffer,"%s/%s", dir, file);
    break;
  }
  return buffer;
}
