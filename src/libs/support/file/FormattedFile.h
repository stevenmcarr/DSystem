/* $Id: FormattedFile.h,v 1.4 1997/03/11 14:36:38 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef FormattedFile_h
#define FormattedFile_h

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
//                                                           
//*****************************************************************

class File; // minimal external declaration


// default upper bound on the length of a string read or written
// to a file port. if a larger upper bound is specified
// then this constant plays no role 

#define DEFAULT_MAX_STRING_LENGTH 256

class FormattedFile {
protected:
  File *fp;
  int constructedFromPointer;
public:

  FormattedFile();
  FormattedFile(File *fptr);

  virtual ~FormattedFile();
  
  virtual int Open(const char *name, char *mode);
  virtual int Close();
  
  // I/O for void* 
  virtual int Write(void *ptr);
  virtual int Read(void *&ptr);
  
  // I/O for longs 
  virtual int Write(long n);
  virtual int Read(long& n); 
  virtual int Write(unsigned long n);
  virtual int Read(unsigned long& n); 

  // I/O for integers 
  virtual int Write(int n);
  virtual int Read(int& n); 
  virtual int Write(unsigned int n);
  virtual int Read(unsigned int& n); 
  
  // I/O for single and double precision fp.
  virtual int Write(float f);
  virtual int Read(float &f);
  virtual int Write(double d);
  virtual int Read(double &d);
  
  // I/O for character strings 
  virtual int Write(const char *s, 
		    unsigned int maxlen = DEFAULT_MAX_STRING_LENGTH);
  virtual int Read(char *s, unsigned int maxlen = DEFAULT_MAX_STRING_LENGTH);
};

#endif /* FormattedFile_h */
