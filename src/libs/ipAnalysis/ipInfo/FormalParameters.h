/* $Id: FormalParameters.h,v 1.1 1997/03/11 14:34:42 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef FormalParameters_h
#define FormalParameters_h

//*****************************************************************
// Parameter List Abstraction                   February 1994
// Author: John Mellor-Crummey                                
//
// Copyright 1994, Rice University
//*****************************************************************

#include <libs/support/tables/namedObject/NamedObject.h>
#include <libs/support/tables/namedObject/NamedObjectTable.h>

//-----------------------------------------------------
// class FormalParameter
//    information about a formal parameter to a 
//    procedure
//
//    values of type field are VTYPE_STAR, 
//    VTYPE_PROCEDURE, or VTYPE_UNDEFINED
//-----------------------------------------------------

class FormalParameter : public NamedObjectIO {
private:
  int NamedObjectReadUpCall(FormattedFile *file);
  int NamedObjectWriteUpCall(FormattedFile *file);

public:
  NamedObjectIO::name;
  const int ptype;    // parameter type

  // used with no args when initializing one to read from a file
  FormalParameter(const char *pname = 0, int ptype = 0);
  FormalParameter(FormalParameter *rhs);
  virtual ~FormalParameter();
  
friend class FormalParameters;
};


//-----------------------------------------------------
// class FormalParameters
//    information about a procedure's formal parameters
//-----------------------------------------------------

class FormalParameters : private NamedObjectTableIO {
private:
  NamedObjectIO *NewEntry();
public:
  FormalParameters();
  FormalParameters(FormalParameters &rhs); // deep copy
  ~FormalParameters();
  
  NamedObjectTableIO::Create;
  NamedObjectTableIO::Destroy;

  void Append(FormalParameter *entry);
  FormalParameter *QueryEntry(const char *name) const;
  FormalParameter *GetMember(unsigned int canonicalIndex); // 0 --> invalid 
  int GetMemberIndex(const char *name); // -1 --> not present

  NamedObjectTableIO::NumberOfEntries;

  int Read(FormattedFile *ffile);
  int Write(FormattedFile *ffile);
friend class FormalParametersIterator;
};


//-------------------------------------------------------------
// class FormalParametersIterator
//-------------------------------------------------------------
class FormalParametersIterator : private NamedObjectTableIterator {
public:
  FormalParametersIterator(const FormalParameters *theTable);
  ~FormalParametersIterator();

  FormalParameter *Current() const;

  NamedObjectTableIterator::operator++;
  NamedObjectTableIterator::Reset;
};

#endif
