/* $Id: FileTimeStamp.C,v 1.2 1997/03/27 20:31:09 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//****************************************************************
// File Time Stamp Abstraction
//
// Author: John Mellor-Crummey                           May 1993   
//                                                                
// Copyright 1993, Rice University
//                                
//****************************************************************

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <libs/support/strings/rn_string.h>
#include <libs/support/file/File.h>
#include <libs/support/memMgmt/mem.h>
#include <libs/support/time/times.h>

#include <libs/fileAttrMgmt/attributedFile/FileTimeStamp.i>

#define STAMP_FORMAT "%10ld %10ld"
#define STAMP_SIZE   21



FileTimeStamp TIMESTAMP_NULL;   

const time_t TIME_NULL = (time_t) 0;
const ino_t INODE_NULL = (ino_t) 0;

void FileTimeStamp::InitNullStamp()
{
  hidden->lmtime = TIME_NULL;
  hidden->inode  = INODE_NULL;
}

FileTimeStamp::FileTimeStamp()
{
  hidden = new FileTimeStampS;
  InitNullStamp();
}


FileTimeStamp::~FileTimeStamp()
{
  delete hidden;
}

time_t FileTimeStamp::GetTime()
{
  return hidden->lmtime;
}

int FileTimeStamp::Init(char *fileName)
{
  struct stat buf;
  
  int code = stat(fileName, &buf);
  if (code == -1) return -1; // failure
  
  hidden->lmtime = buf.st_mtime;
  hidden->inode  = buf.st_ino;

  return 0; // success
}

void FileTimeStamp::SetTime(time_t lmtime)
{
  assert(hidden->inode != 0);
  hidden->lmtime = lmtime;
}

int FileTimeStamp::SetTime()
{
  assert(hidden->inode != 0);
  time_t temp = time(0);
  if (temp == (time_t) -1) return -1;
  hidden->lmtime = temp;
  return 0;
}

int FileTimeStamp::operator>=(FileTimeStamp &rhs)
{
  assert(hidden->inode != 0);
  return ((hidden->lmtime >= rhs.hidden->lmtime) && 
	  (hidden->inode  == rhs.hidden->inode));
}

int FileTimeStamp::operator==(FileTimeStamp &rhs)
{
  assert(hidden->inode != 0);
  return ((hidden->lmtime == rhs.hidden->lmtime) && 
	  (hidden->inode  == rhs.hidden->inode));
}


FileTimeStamp &FileTimeStamp::operator=(FileTimeStamp &rhs)
{
  hidden->lmtime = rhs.hidden->lmtime;
  hidden->inode  = rhs.hidden->inode;
  return *this;
}


int FileTimeStamp::Write(File &fp)
{
  char buf[STAMP_SIZE + 1];
  sprintf(buf, STAMP_FORMAT, hidden->lmtime, hidden->inode);
  return fp.Write(buf, STAMP_SIZE);
}

int FileTimeStamp::Read(File &fp)
{
  time_t lmtime;
  ino_t  inode;
  char   str[STAMP_SIZE + 1];

  int code = fp.Read(str, STAMP_SIZE);
  if (code) return EOF;
  str[STAMP_SIZE]  = '\0';
  
  if (sscanf(str, STAMP_FORMAT, &lmtime, &inode) != 2) {
    InitNullStamp();
    return EOF;
  }
  else {
    hidden->lmtime = lmtime;
    hidden->inode = inode;
    return 0;
  }
}

