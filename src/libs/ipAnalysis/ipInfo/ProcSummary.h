/* $Id: ProcSummary.h,v 1.1 1997/03/11 14:34:47 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// ProcSummary.h
//
// summary information about a procedure including type, parameters and
// call sites. this summary is used to construct a call graph for a
// program.
//
// Author: John Mellor-Crummey                                December 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#ifndef ProcSummary_h
#define ProcSummary_h


#ifndef ProcLocalInfo_h
#include <libs/ipAnalysis/ipInfo/ProcLocalInfo.h>
#endif

#ifndef EntryPoints_h
#include <libs/ipAnalysis/ipInfo/EntryPoints.h>
#endif


typedef enum {ProcType_SUB, ProcType_PGM, ProcType_FUN, ProcType_BDATA, 
	      ProcType_ILLEGAL} ProcType;


class FormattedFile; // minimal external declaration
class ParameterList; // minimal external declaration
class CallSites;     // minimal external declaration


//--------------------------------------------------------------------
// class ProcSummary
//--------------------------------------------------------------------
class ProcSummary: public ProcLocalInfo {
public:
  const ProcType procType;       
  EntryPoints entryPoints;

  CallSites *calls;  // descriptors for call sites in the procedure

  ProcSummary(const char *name, ProcType type);
  ProcSummary();

  ~ProcSummary();

  int WriteUpCall(FormattedFile *port);
  int ReadUpCall(FormattedFile *port);
};


#endif
