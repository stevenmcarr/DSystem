/* $Id: Annotation.C,v 1.1 1997/03/11 14:36:26 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//******************************************************************
//
// Annotation.C
//
// Implementation of FlowGraph Annotations. 
// A base class "Annotation" can be used to manipulate
// arbitrary types of annotations. Derived annotations are provided to
// provide basic functionality for annotations that can be attached to a 
// node, an edge, or either a node or edge
//
// Author: John Mellor-Crummey                        Feburary 1993
//
// Copyright 1993, Rice University
// 
//******************************************************************


#include <libs/support/strings/OrderedSetOfStrings.h>
#include <libs/support/annotation/Annotation.h>



//***************************************************************************
// implementation of class Annotation
//***************************************************************************

Annotation::Annotation(const char *const aname) : NamedObject(aname)
{
}


Annotation::~Annotation()
{
}


int Annotation::WriteUpCall(FormattedFile *)
{
  return 1;
}


int Annotation::ReadUpCall(FormattedFile *)
{
  return 1;
}


void Annotation::Dump()
{
  NamedObjectDump();
}


void Annotation::NamedObjectDumpUpCall()
{
  OrderedSetOfStrings *s = CreateOrderedSetOfStrings();
  s->Dump();
  delete s;
}


OrderedSetOfStrings *Annotation::CreateOrderedSetOfStrings()
{
  return new OrderedSetOfStrings(true); // return empty string set
}

