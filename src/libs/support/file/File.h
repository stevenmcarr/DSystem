/* $Id: File.h,v 1.6 1997/06/25 15:14:22 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef File_h
#define File_h

//****************************************************************
// File Stream I/O Abstraction                            
//
// Author: John Mellor-Crummey                           May 1993   
//                                                                
// Copyright 1993, Rice University
//                                
//****************************************************************

#include <stdio.h>
#include <sys/file.h>

// SeekRefPt: reference point from which to seek
//   SEEK_ORIGIN  = start of file
//   SEEK_CURRENT = current position
//   SEEK_END     = end of file
typedef enum 
#ifdef  SEEK_SET
// Solaris
{SEEK_ORIGIN=SEEK_SET, SEEK_CURRENT=SEEK_CUR, SEEK_LAST=SEEK_END}
#else
// BSD or Sun O/S
{SEEK_ORIGIN=L_SET, SEEK_CURRENT=L_INCR, SEEK_LAST=L_XTND}
#endif
SeekRefPt;

// definition of File internal representation
struct FileS {
  FILE *fp;
};

struct stat;

class File {
  struct FileS *hidden;
public:
  File();
  virtual ~File();
  
  // open/close, EOF returned in case of error
  virtual int Open(const char *name, const char *mode);
  virtual int Close();
  
  int Seek(long offset, SeekRefPt seekRefPt);
  
  int Stat(struct stat *buf);
  unsigned char Flags();
  
  // character input: EOF returned in case of error
  int Getc();
  
  // one character pushback: EOF returned in case of error
  int Ungetc(char c);
  
  // character output: EOF returned in case of error
  int Putc(char c);

  // string input: on success, string returned; in case of error or
  // end of input, NULL returned and error flags set for the file
  char *Gets(char *buffer, int buflen);

  // string output: EOF in case of error, 0 otherwise
  int Puts(const char *buffer);
  
  // bulk input: EOF returned in case of error, 0 otherwise
  int Read(void *buf, unsigned int nbytes);
  
  // bulk output: EOF returned in case of error, 0 otherwise
  int Write(const void *buf, unsigned int nbytes);

  // formatted output: EOF returned in case of error
  int Printf(const char *format, ...);
};

#endif /* File_h */
