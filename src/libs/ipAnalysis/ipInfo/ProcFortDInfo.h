/* $Id: ProcFortDInfo.h,v 1.1 1997/03/11 14:34:45 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// ProcFortDInfo.h
//
// Fortran D information about a procedure including decomposition,
// alignment, overlap, common block declaration, and callsite information
//
// Author: Gil Hansen                                         April 1994
//
// Copyright 1994, Rice University
//***************************************************************************

#ifndef ProcFortDInfo_h
#define ProcFortDInfo_h


#ifndef ProcLocalInfo_h
#include <libs/ipAnalysis/ipInfo/ProcLocalInfo.h>
#endif

#ifndef iptree_h
#include <libs/ipAnalysis/ipInfo/iptree.h>
#endif



//--------------------------------------------------------------------
// class ProcFortDInfo
//--------------------------------------------------------------------
class ProcFortDInfo: public ProcLocalInfo {
public:
  /* The iptree is needed when call graph built and contains the local
     Fortran D analysis information */
  IPinfoTree *tree;

  ProcFortDInfo(const char *name = 0);

  ~ProcFortDInfo();

  int WriteUpCall(FormattedFile *port);
  int ReadUpCall(FormattedFile *port);
};


#endif
