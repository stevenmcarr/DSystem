/* $Id: DataFlowSet.h,v 1.1 1997/03/11 14:37:18 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef DataFlowSet_h
#define DataFlowSet_h

//***************************************************************************
//
// DataFlowSet.h
//
// Base class for arbitrary Data Flow Sets
//
// Author: John Mellor-Crummey                                  July 1994 
//
// Copyright 1994, Rice University
// 
//***************************************************************************


#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef Annotation_h
#include <libs/support/annotation/Annotation.h>
#endif


//***************************************************************************
// class DataFlowSet
//***************************************************************************
class DataFlowSet : public Annotation {
protected:
  DataFlowSet(const char *const name);
public:
  virtual ~DataFlowSet(); 
  virtual int operator ==(const DataFlowSet &rhs) const = 0;
  virtual void operator |=(const DataFlowSet &rhs);
  virtual DataFlowSet *Clone() const;

  int operator !=(const DataFlowSet &rhs) const;

  Annotation::Dump;
  Annotation::CreateOrderedSetOfStrings;

  Annotation::WriteUpCall;
  Annotation::ReadUpCall;
};


#endif /* DataFlowSet_h */

