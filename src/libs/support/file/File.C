/* $Id: File.C,v 1.8 1999/06/11 20:32:29 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//*****************************************************************
// File Stream I/O Abstraction
//
// abstract interface for buffered I/O to a file 
// 
// Author: 
//   John Mellor-Crummey                              May 1993
//
// Copyright 1993, Rice University                                
//                                                                 
// ****************************************************************

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

#include <libs/support/strings/rn_string.h>
#include <libs/support/file/File.h>

File::File() 
{ 
  hidden = new FileS;
  hidden->fp = 0;
}


File::~File() 
{ 
  (void) Close(); 
  delete hidden;
}


// EOF returned if error
int File::Open(const char *name, const char *mode) 
{ 
  hidden->fp = fopen(name, mode);
  return (hidden->fp ? 0 : EOF);
}

unsigned char File::Flags()
{
#ifdef LINUX
  return hidden->fp->_flags;
#else
  return hidden->fp->_flag;
#endif
}

// EOF returned if error
int File::Close() 
{ 
  if (hidden->fp) {
	int code = fclose(hidden->fp);
	hidden->fp = 0;
	return code;
  } else return EOF;
}

// EOF returned if error
int File::Seek(long offset, SeekRefPt seekRefPt) 
{ 
  return fseek(hidden->fp, offset, (int) seekRefPt);
}

// EOF returned if error
int File::Stat(struct stat *buf) 
{ 
  if (hidden->fp) return fstat(fileno(hidden->fp), buf);
  else return EOF;
}


// character input, EOF returned in case of error
int File::Getc() 
{ 
  return getc(hidden->fp); 
}

// one character pushback, EOF returned in case of error
int File::Ungetc(char c) 
{ 
  return (ungetc(c, hidden->fp) == EOF) ? EOF : 0; 
}

// character output, EOF returned in case of error
int File::Putc(char c) 
{ 
  return putc(c, hidden->fp); 
}

// bulk input: EOF returned in case of error, 0 otherwise
int File::Read(void *buf, unsigned int nbytes) 
{
  // has single object of requested size been correctly read? 
  return (fread((char *) buf, nbytes, 1, hidden->fp) != 1) ? EOF : 0;
}

// bulk output: EOF returned in case of error, 0 otherwise
int File::Write(const void *buf, unsigned int nbytes) 
{
  // has single object of requested size been correctly written? 
  return (fwrite((char *) buf, nbytes, 1, hidden->fp) != 1) ? EOF : 0;
}

// formatted output: EOF returned in case of error
int File::Printf(const char *format, ...)
{
  va_list args;
  va_start(args, format);

  int retcode = vfprintf(hidden->fp, format, args);
  
  va_end(args);

  return retcode;
}

// string input: NULL returned in case of error and error flags set for the file
char *File::Gets(char *buffer, int buflen)
{
  return fgets(buffer, buflen, hidden->fp);
}

// string output:  EOF in case of error, 0 otherwise
int File::Puts(const char *buffer)
{
  return (fputs(buffer, hidden->fp) == EOF ? EOF : 0);
}
