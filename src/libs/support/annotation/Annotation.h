/* $Id: Annotation.h,v 1.1 1997/03/11 14:36:27 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef Annotation_h
#define Annotation_h

//***************************************************************************
//
// Annotation.h
//
// Base class for arbitrary annotations. 
//
// Author: John Mellor-Crummey                                  July 1994 
//
// Copyright 1994, Rice University
// 
//***************************************************************************


#include <libs/support/misc/general.h>
#include <libs/support/tables/namedObject/NamedObject.h>


class FormattedFile;          // external declaration
class OrderedSetOfStrings;    // external declaration



//***************************************************************************
// class Annotation
//***************************************************************************
class Annotation : public NamedObject {
protected:
  Annotation(const char *const aname);
public:

  //-------------------------------------------------------------------
  // virtual destructor to ensure that destruction works correctly
  // for derived classes
  //-------------------------------------------------------------------
  virtual ~Annotation(); 
  
  //-------------------------------------------------------------------
  // virtual functions that should read/write the contents of an
  // annotation derived from this base class
  //
  // return values:
  //   = 0: success
  //   < 0: unexpected failure
  //   > 0: graceful failure
  //-------------------------------------------------------------------
  virtual int WriteUpCall(FormattedFile *file);
  virtual int ReadUpCall(FormattedFile *file);
  
  //-------------------------------------------------------------------
  // dump a printable representation of the annotation to stderr
  //-------------------------------------------------------------------
  void Dump();
  
  //-------------------------------------------------------------------
  // generate printable version of the annotation (if feasible)
  //-------------------------------------------------------------------
  virtual OrderedSetOfStrings *CreateOrderedSetOfStrings();

private:
  void NamedObjectDumpUpCall();
};


#endif /* Annotation_h */

