/* $Id: FileTimeStamp.i,v 1.1 1997/03/11 14:27:45 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef FileTimeStamp_h
#define FileTimeStamp_h

//****************************************************************
// File Time Stamp Abstraction
//
// Author: John Mellor-Crummey                           May 1993   
//                                                                
// Copyright 1993, Rice University
//                                
//****************************************************************

class File; // external definition

class FileTimeStamp {
  struct FileTimeStampS *hidden;
  void InitNullStamp();
public:
  // create a NULL timestamp
  FileTimeStamp();

  ~FileTimeStamp();

  // set the timestamp to the last modification time of the file
  // non-zero return code indicates failure
  int Init(char *fileName);

  // set the timestamp to the specified time
  void SetTime(time_t lmtime);

  // set the timestamp to the the current time 
  // non-zero return code indicates failure
  int SetTime();

  // zero return code indicates predicate not satisfied, or 
  // mismatch in file identifier portion of timestamp
  int operator>=(FileTimeStamp &rhs);
  int operator==(FileTimeStamp &rhs);

  FileTimeStamp &operator=(FileTimeStamp &rhs);

  // read/write a timestamp from/to an open File at the current 
  // position in the file
  int Read(File &fp);
  int Write(File &fp);

  time_t GetTime();
};

#endif /* FileTimeStamp_h */
