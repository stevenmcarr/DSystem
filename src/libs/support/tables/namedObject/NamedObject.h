/* $Id: NamedObject.h,v 1.4 1997/03/11 14:37:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef NamedObject_h
#define NamedObject_h

//*************************************************************************** 
//
// NamedObject and NamedObjectNoIO
//                                                                          
//   base class for named entities
//
// Author:  John Mellor-Crummey                            June 1993
//
// Copyright 1993, Rice University
//                                                                          
//***************************************************************************

#ifndef ClassName_h
#include <include/ClassName.h>
#endif

class FormattedFile;   // minimal external declaration

//-----------------------------------------------------------------
// class NamedObject
//-----------------------------------------------------------------
class NamedObject {
public:
  const char *const name;
  NamedObject(const char *const objectName);
  void Create(const char *const objectName);
  virtual ~NamedObject();

  CLASS_NAME_FDEF(NamedObject);

  //--------------------------------------------------------
  // debugging support for NamedObjects and derived classes 
  //--------------------------------------------------------
  void NamedObjectDump();
  virtual void NamedObjectDumpUpCall();
};

CLASS_NAME_EDEF(NamedObject);


//-----------------------------------------------------------------
// class NamedObjectIO
//-----------------------------------------------------------------
class NamedObjectIO : public NamedObject {
public:
  NamedObjectIO(const char *const objectName);
  virtual ~NamedObjectIO();

  CLASS_NAME_FDEF(NamedObjectIO);

  //--------------------------------------------------------
  // support for I/O of NamedObjects and derived classes 
  //--------------------------------------------------------
  int NamedObjectRead(FormattedFile *file);
  virtual int NamedObjectReadUpCall(FormattedFile *file);
    
  int NamedObjectWrite(FormattedFile *file);
  virtual int NamedObjectWriteUpCall(FormattedFile *file);
};

CLASS_NAME_EDEF(NamedObjectIO);

#endif /* NamedObject_h */
