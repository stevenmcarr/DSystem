/* $Id: DataFlowSet.C,v 1.1 1997/03/11 14:37:17 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
//
// DataFlowSet.C
//
// Base class for arbitrary Data Flow Sets
//
// Author: John Mellor-Crummey                                  July 1994 
//
// Copyright 1994, Rice University
// 
//***************************************************************************


#include <assert.h>

#include <libs/support/sets/DataFlowSet.h>


//***************************************************************************
// class DataFlowSet
//***************************************************************************
DataFlowSet::DataFlowSet(const char *const _name) : Annotation(_name) 
{
}


DataFlowSet::~DataFlowSet()
{
}


// placeholder
void DataFlowSet::operator|=(const DataFlowSet &)
{
  assert(0);
}

int DataFlowSet::operator!=(const DataFlowSet &rhs) const
{
  return !(*this == rhs);
}


// placeholder
DataFlowSet *DataFlowSet::Clone() const
{
  assert(0);
  return 0;
}


