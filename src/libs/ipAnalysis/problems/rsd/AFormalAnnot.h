/* $Id: AFormalAnnot.h,v 1.3 1997/03/11 14:35:14 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef AFormalAnnot_h
#define AFormalAnnot_h

//*****************************************************************************
// AFormalAnnot.h
//
//      node annotations that indicate whether each formal parameter is used
//      as an array by the procedure or its descendants.
//
// Author: John Mellor-Crummey                                 September 1994
//
// Copyright 1994, Rice University. All rights reserved.
//*****************************************************************************

#ifndef DataFlowSet_h
#include <libs/support/sets/DataFlowSet.h>
#endif

#ifndef StringSet_h
#include <libs/support/strings/StringSet.h>
#endif

class FormattedFile;          // external declaration
class OrderedSetOfStrings;    // external declaration

class AFormalAnnot : public DataFlowSet, public StringSet {
public:
  // create an AFormal annotation 
  AFormalAnnot();
  AFormalAnnot(AFormalAnnot &rhs);
  
  ~AFormalAnnot();

  // copy a derived annotation
  DataFlowSet *Clone() const;

  void operator |=(const DataFlowSet &rhs);
  void operator |=(StringSet &rhs);

  int operator ==(const DataFlowSet &rhs) const;

  // upcall to read an annotation
  int ReadUpCall(FormattedFile *file);
  
  // upcall to write an annotation
  int WriteUpCall(FormattedFile *file);
  
  // generate printable version of the annotation
  OrderedSetOfStrings *CreateOrderedSetOfStrings();
};

extern char *AFORMAL_ANNOT;

#endif /* AFormalAnnot_h */
