/* $Id: FormattedFile.C,v 1.5 1997/03/11 14:36:37 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//*****************************************************************
//                                                           
// Formatted File I/O Abstraction               May 1993
// Author: John Mellor-Crummey                               
//                                                           
// The FormattedFile abstraction supports reading and writing
// of various types to a File in an unambiguous way.
//
// (default formatting in printable ASCII) 
//
// Copyright 1993, Rice University
//*****************************************************************

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include <libs/support/file/File.h>
#include <libs/support/file/FormattedFile.h>


FormattedFile::FormattedFile()
{
  fp = new File;
  constructedFromPointer = 0;
}

FormattedFile::FormattedFile(File *fptr)
{
  assert(fptr != 0);
  fp = fptr;
  constructedFromPointer = 1;
}

FormattedFile::~FormattedFile()
{
  if (!constructedFromPointer) delete fp;
}

int FormattedFile::Open(const char *name, char *mode)
{
  return fp->Open(name, mode);
}

int FormattedFile::Close()
{
  return fp->Close();
}

//*****************************************************************
// implementation of default formatting in printable ASCII 
//*****************************************************************

// forward declarations
static int write_decimal_digits(File *fp, long n);
static int read_decimal_digits(File *fp, long& n);


//---------------------------------------------------
// long I/O
//---------------------------------------------------

int FormattedFile::Write(long n) 
{
  if (write_decimal_digits(fp, n) == EOF) return EOF;
  else if(fp->Putc('.') == EOF) return EOF;
  else return 0; // success
}


int FormattedFile::Read(long& n) 
{
  if (read_decimal_digits(fp, n) == EOF) return EOF;
  else if (fp->Getc() != '.') return EOF;
  else return 0; // success
}


//---------------------------------------------------
// unsigned long I/O
//---------------------------------------------------

int FormattedFile::Write(unsigned long n) 
{
  return Write((long) n);
}


int FormattedFile::Read(unsigned long& n) 
{
  return Read(*(long *) &n);
}


//---------------------------------------------------
// void * I/O
//---------------------------------------------------

int FormattedFile::Write(void *ptr) 
{ 
  return Write((long) ptr);
}


int FormattedFile::Read(void *&ptr) 
{ 
  long temp;
  int code = Read(temp);
  ptr = (void *) temp;
  return code;
}


//---------------------------------------------------
// int I/O
//---------------------------------------------------

int FormattedFile::Write(int n) 
{ 
  return Write((long) n);
}


int FormattedFile::Read(int &n)
{ 
  long temp;
  int code = Read(temp);
  n = (int) temp;
  return code;
}


//---------------------------------------------------
// unsigned int I/O
//---------------------------------------------------

int FormattedFile::Write(unsigned int n) 
{
  return Write((long) n);
}


int FormattedFile::Read(unsigned int& n) 
{
  long temp;
  int code = Read(temp);
  n = (unsigned int) temp;
  return code;
}


//----------------------------------------------------
// float I/O
//----------------------------------------------------

int FormattedFile::Write(float f) 
{
  return Write((double) f);
}


int FormattedFile::Read(float& f) 
{
  double dd;

  if (Read(dd) == EOF) return EOF;
  f = dd;
  return 0;
}

// Notes on floating point read/write routines:
// 
// Floating point is, of course, tremendously implementation
// dependent. As a result, SOME LOSS OF PRECISION may result
// when you write out a float or double with these routines
// and then read them back in later on. If you can't deal
// with a loss of precision, then please don't use these routines.
//
// These routines rely on the routine 'gcvt', which could be 
// a source of portability problems (Sun has labelled it an 
// "obsolete" routine and designated some new routines to replace
// it, whereas it is the only routine of its type available on
// the RS-6000.
//

//-----------------------------------------------
// double I/O
//-----------------------------------------------

int FormattedFile::Write(double d) 
{
  // Doubles are uncommon enough that we'll just treat them as strings.
  char buf[300];

  if (fp->Putc('D') == EOF) return EOF;
  else if (Write(gcvt(d, 255, buf)) == EOF) return EOF;
  else return 0; // success
}


int FormattedFile::Read(double &d) 
{
  char buf[256];

  if (fp->Getc() != 'D') return EOF;
  else if (Read(buf, 255) == EOF) return EOF;
  sscanf(buf, "%lf", &d);
  return 0; // success
}

//-----------------------------------------------
// character string I/O
//-----------------------------------------------

int FormattedFile::Write(const char *s, unsigned int maxlen) 
{
  if (fp->Putc('`') == EOF) return EOF;
  while (maxlen-- > 0 && *s != 0) {
    if (*s == '\'') 
      // escape end of string char by doubling 
      if(fp->Putc('\'') == EOF) return EOF; 
    if (fp->Putc(*s++) == EOF) return EOF;
  }
  if (fp->Putc('\'') == EOF) return EOF;
  else return 0; // success
}


int FormattedFile::Read(char *s, unsigned int maxlen) 
{ 
  int c;
  unsigned int len = 0;
  assert (maxlen >= 0);
  if ((c = fp->Getc()) != '`') return EOF; // not start of string
  for(;;) {
    if ((c = fp->Getc()) == EOF) return EOF;
    if (c == '\'') {  // end of string?
      if ((c = fp->Getc()) == EOF) break; // EOF allowed here
      else if (c != '\'') {  
				// not an escaped end of string
				// push back first char after string
	if (fp->Ungetc(c) == EOF) return EOF; 
	break; 
      }
    }
    if (++len < maxlen) *s++ = c; 
  }
  *s = 0; // terminate the string
  return 0; // success
}

// ***********************************************************
// internal functions
// ***********************************************************


//-----------------------------------------------
// write an long in decimal to a file port in printable ASCII
//-----------------------------------------------
static int write_unsigned_decimal_digits(File *fp, unsigned long n)
{
  if (n >= 10) 
    if (write_unsigned_decimal_digits(fp, n/10) == EOF) return EOF;
  return fp->Putc((char)((n % 10) + '0'));
}

static int write_decimal_digits(File *fp, long n)
{
  if (n < 0) { 
    if (fp->Putc('-') == EOF) return EOF;
    n = -n;
  }
  return write_unsigned_decimal_digits(fp, n);
}


//-----------------------------------------------
// read an long in decimal from a file port in printable ASCII
//-----------------------------------------------
static int read_decimal_digits(File *fp, long& n)
{
  n = 0;
  int one_trip = 0;
  
  int sign = 1;
  
  // compute sign
  int csign = fp->Getc();
  if (csign == '-') sign = -1;
  else if (csign == '+') sign = 1;
  else if (csign == EOF) return EOF;
  else if (fp->Ungetc((char) csign) == EOF) return EOF;
  
  for(;;) {
    int digit = fp->Getc();
    if (digit >= '0' && digit <= '9') {
      n = n * 10 + digit - '0';
      one_trip = 1;
    } else {
      if (digit == EOF) return EOF;
      if (fp->Ungetc((char) digit) == EOF) return EOF;
      break;
    }
  }
  n = n * sign;
  return (one_trip == 0) ? EOF : 0; // success ?
}

