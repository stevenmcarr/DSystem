/* $Id: VariableSet.C,v 1.1 1997/03/11 14:34:48 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//******************************************************************
//  VariableSet.C: 
//
// Author: 
//   John Mellor-Crummey                              January 1994
//
// Copyright 1994, Rice University
//******************************************************************


#include <libs/ipAnalysis/ipInfo/VariableSet.h>
#include <libs/support/file/FormattedFile.h>



//***********************************************************************
// class VariableSet interface operations
//***********************************************************************

CLASS_NAME_IMPL(VariableSet);

VariableSet::VariableSet()
{
}


VariableSet::VariableSet(VariableSet &rhs) : formals(rhs.formals),
locals(rhs.locals), globals(rhs.globals) 
{
}


VariableSet::~VariableSet()
{
}


int VariableSet::operator ==(VariableSet &rhs)
{
  return formals == rhs.formals && locals == rhs.locals && 
    globals == rhs.globals;
}


void VariableSet::operator |=(VariableSet &rhs)
{
  formals |= rhs.formals; 
  locals |= rhs.locals;
  globals |= rhs.globals;
}


//========================================================================
// file I/O operations
//========================================================================


int VariableSet::Read(FormattedFile *file)
{
  return formals.Read(file) || locals.Read(file) || globals.Read(file);
}


int VariableSet::Write(FormattedFile *file)
{
  return formals.Write(file) || locals.Write(file) || globals.Write(file);
}


