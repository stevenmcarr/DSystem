/* $Id: IdGenerator.h,v 1.1 1997/03/11 14:36:54 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef IdGenerator_h
#define IdGenerator_h

//*******************************************************************
// IdGenerator.h
//
// Author: John Mellor-Crummey                             July 1994
//
// Copyright 1994, Rice University
//*******************************************************************


class WordSet; // minimal external declaration
class FormattedFile; // minimal external declaration


//*******************************************************************
// class IdGenerator 
//*******************************************************************
class IdGenerator {
public: 
  IdGenerator();
  ~IdGenerator();

  void Create();
  void Destroy();

  unsigned int AcquireId();
  void ReleaseId(unsigned int id);

  unsigned int GetHighWaterMark();

  int Read(FormattedFile *file);
  int Write(FormattedFile *file);
private:
  unsigned int idHighWaterMark;
  WordSet *freedIds;
};

#endif
