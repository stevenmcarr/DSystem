/* $Id: AFormalAnnot.C,v 1.2 1997/03/27 20:41:49 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//*****************************************************************************
// AFormalAnnot.C
//
//      node annotations that indicate whether each formal parameter is used
//      as an array by the procedure or its descendants.
//
// Author: John Mellor-Crummey                                 September 1994
//
// Copyright 1994, Rice University. All rights reserved.
//*****************************************************************************


#include <libs/ipAnalysis/problems/rsd/AFormalAnnot.h>

#include <libs/support/strings/OrderedSetOfStrings.h>
#include <libs/support/strings/StringBuffer.h>

char *AFORMAL_ANNOT = "AFormal";



//*****************************************************************************
// class  AFormalAnnot interface operations 
//*****************************************************************************

AFormalAnnot::AFormalAnnot() : DataFlowSet(AFORMAL_ANNOT)
{
}


AFormalAnnot::AFormalAnnot(AFormalAnnot &rhs) : DataFlowSet(AFORMAL_ANNOT),
StringSet(rhs)
{
}


AFormalAnnot::~AFormalAnnot()
{
}


DataFlowSet *AFormalAnnot::Clone() const
{
  return new AFormalAnnot(*(AFormalAnnot *) this);
}

int AFormalAnnot::operator ==(const DataFlowSet &rhs) const 
{
  StringSet *srhs = (AFormalAnnot *) &rhs;
  return *((StringSet *)this) == *srhs;
}

void AFormalAnnot::operator |=(const DataFlowSet &rhs) 
{
  StringSet *srhs = (AFormalAnnot *) &rhs;
  *this |= *srhs;
}


void AFormalAnnot::operator |=(StringSet &rhs) 
{
  *this |= rhs;
}


int AFormalAnnot::ReadUpCall(FormattedFile *file)
{
  return StringSet::Read(file);
}


int AFormalAnnot::WriteUpCall(FormattedFile *file)
{
  return StringSet::Write(file);
}


OrderedSetOfStrings *
AFormalAnnot::CreateOrderedSetOfStrings()
{
  OrderedSetOfStrings *oss = new OrderedSetOfStrings;
  StringBuffer string(80);
  const char *name;
  
  string.Append("%s:", this->name);
  
  for (StringSetIterator it(this); name = it.Current(); ++it) {
    string.Append(" %s", name);
  }
  oss->Append(string.Finalize());
  return oss;
}


