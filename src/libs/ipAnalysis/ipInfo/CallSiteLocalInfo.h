/* $Id: CallSiteLocalInfo.h,v 1.1 1997/03/11 14:34:39 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef CallSiteLocalInfo_h
#define CallSiteLocalInfo_h

//***************************************************************************
// CallSiteLocalInfo.h
//
// Author: John Mellor-Crummey                                July 1994
//
// Copyright 1994, Rice University
//***************************************************************************


#ifndef WordObject_h
#include <libs/support/tables/wordObject/WordObject.h>
#endif



//***************************************************************************
// class CallSiteLocalInfo
//***************************************************************************
class CallSiteLocalInfo : public WordObjectIO {
  int WordObjectReadUpCall(FormattedFile *file);
  int WordObjectWriteUpCall(FormattedFile *file);
public:
  CallSiteLocalInfo(unsigned int _id);
  virtual ~CallSiteLocalInfo();

  virtual int ReadUpCall(FormattedFile *file) = 0;
  virtual int WriteUpCall(FormattedFile *file) = 0;
};


#endif
