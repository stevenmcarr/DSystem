/* $Id: VariableSet.h,v 1.1 1997/03/11 14:34:48 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef VariableSet_h
#define VariableSet_h

//******************************************************************
//  VariableSet.h: 
//
// Author: 
//   John Mellor-Crummey                              January 1994
//
// Copyright 1994, Rice University
//******************************************************************


#include <include/ClassName.h>
#include <libs/ipAnalysis/ipInfo/EqClassPairs.h>
#include <libs/support/strings/StringSet.h>

class FormattedFile; // minimal external declaration

//-----------------------------------------------------------------------
// class VariableSet
//-----------------------------------------------------------------------
class VariableSet {
public:
  StringSet formals;
  EqClassPairSet locals;
  EqClassPairSet globals;

  VariableSet();
  VariableSet(VariableSet &rhs);
  virtual ~VariableSet();

  int operator ==(VariableSet &rhs);
  void operator |=(VariableSet &rhs);

  int Write(FormattedFile *file);
  int Read(FormattedFile *file);

  CLASS_NAME_FDEF(VariableSet);
};


#endif
