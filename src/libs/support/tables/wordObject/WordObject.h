/* $Id: WordObject.h,v 1.4 1997/03/11 14:37:42 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef WordObject_h
#define WordObject_h

//*************************************************************************** 
//
// WordObject and WordObjectIO
//                                                                          
//   base class for entities identified by a unsigned int tag
//
// Author:  John Mellor-Crummey                            June 1994
//
// Copyright 1994, Rice University
//                                                                          
//***************************************************************************

#ifndef ClassName_h
#include <include/ClassName.h>
#endif

class FormattedFile;   // minimal external declaration

//-----------------------------------------------------------------
// class WordObject
//-----------------------------------------------------------------
class WordObject {
public:
  const unsigned int id;
  WordObject(const unsigned int _id);
  void Create(const unsigned int _id);
  virtual ~WordObject();

  CLASS_NAME_FDEF(WordObject);

  //--------------------------------------------------------
  // debugging support for WordObjects and derived classes 
  //--------------------------------------------------------
  void WordObjectDump();
  virtual void WordObjectDumpUpCall();
};

CLASS_NAME_EDEF(WordObject);


//-----------------------------------------------------------------
// class WordObjectIO
//-----------------------------------------------------------------
class WordObjectIO : public WordObject {
public:
  WordObjectIO(const unsigned int _id);
  virtual ~WordObjectIO();

  CLASS_NAME_FDEF(WordObjectIO);

  //--------------------------------------------------------
  // support for I/O of WordObjects and derived classes 
  //--------------------------------------------------------
  int WordObjectRead(FormattedFile *file);
  virtual int WordObjectReadUpCall(FormattedFile *file);
    
  int WordObjectWrite(FormattedFile *file);
  virtual int WordObjectWriteUpCall(FormattedFile *file);
};

CLASS_NAME_EDEF(WordObjectIO);

#endif /* WordObject_h */
