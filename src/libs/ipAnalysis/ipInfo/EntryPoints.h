/* $Id: EntryPoints.h,v 1.2 1997/03/27 20:40:38 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// EntryPoints.h
//
// Author: John Mellor-Crummey                                February 1994
//
// Copyright 1994, Rice University
//***************************************************************************


#ifndef EntryPoints_h
#define EntryPoints_h

#ifndef NamedObject_h
#include <libs/support/tables/namedObject/NamedObject.h>
#endif

#ifndef NamedObjectTable_h
#include <libs/support/tables/namedObject/NamedObjectTable.h>
#endif

#ifndef FormalParameters_h
#include <libs/ipAnalysis/ipInfo/FormalParameters.h>
#endif


//--------------------------------------------------------------------
// class EntryPoint
//--------------------------------------------------------------------
class EntryPoint : public NamedObjectIO {
private:
  int NamedObjectReadUpCall(FormattedFile *ffile);
  int NamedObjectWriteUpCall(FormattedFile *ffile);
public:
  int nodeId;
  FormalParameters formals; 

  EntryPoint(const char *_name = 0, int _nodeId = 0);
  EntryPoint(EntryPoint *rhs); // deep copy
  ~EntryPoint();
};


//--------------------------------------------------------------------
// class EntryPoints
//--------------------------------------------------------------------
class EntryPoints : public NamedObjectTableIO {
private:
  NamedObjectIO *NewEntry();
public:
  EntryPoints();
  ~EntryPoints();

  NamedObjectTableIO::Create;
  NamedObjectTableIO::Destroy;

  void operator =(EntryPoints &rhs);

  void AddEntry(EntryPoint *entry);
  EntryPoint *QueryEntry(const char *name) const;

  int Read(FormattedFile *ffile);
  int Write(FormattedFile *ffile);
};


//-------------------------------------------------------------
// class EntryPointsIterator
//-------------------------------------------------------------
class EntryPointsIterator : public NamedObjectTableIterator {
public:
  EntryPointsIterator(const EntryPoints *theTable);
  ~EntryPointsIterator();

  EntryPoint *Current() const;

  NamedObjectTableIterator::operator++;
  NamedObjectTableIterator::Reset;
};

#endif 
