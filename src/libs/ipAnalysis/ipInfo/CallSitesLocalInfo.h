/* $Id: CallSitesLocalInfo.h,v 1.1 1997/03/11 14:34:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef CallSitesLocalInfo_h
#define CallSitesLocalInfo_h

//***************************************************************************
// CallSitesLocalInfo.h
//
// Author: John Mellor-Crummey                                July 1994
//
// Copyright 1994, Rice University
//***************************************************************************


#ifndef WordObjectTable_h
#include <libs/support/tables/wordObject/WordObjectTable.h>
#endif


class CallSites;              // minimal external declaration
class ProcLocalInfo;          // minimal external declaration
class CallSiteLocalInfo;      // minimal external declaration
class WordObjectIO;           // minimal external declaration


//--------------------------------------------------------------------
// class CallSitesLocalInfo
//--------------------------------------------------------------------
class CallSitesLocalInfo : public WordObjectTableIO {
public:
  CallSitesLocalInfo();
  virtual ~CallSitesLocalInfo();
  
  void AddCallSiteEntry(CallSiteLocalInfo *cli);
  CallSiteLocalInfo *GetCallSiteEntry(unsigned int callSiteId);
  
private:
  virtual int Create();
  virtual void Destroy();
  virtual CallSiteLocalInfo *NewCallSiteEntry();
  
  WordObjectIO *NewWordObjectIO();
  
friend class CallSitesLocalInfoIterator;
};


//--------------------------------------------------------------------
// class CallSitesLocalInfoIterator
//--------------------------------------------------------------------
class  CallSitesLocalInfoIterator : private WordObjectTableIterator {
public:
  CallSitesLocalInfoIterator(const CallSitesLocalInfo *cli);
  ~CallSitesLocalInfoIterator();

  CallSiteLocalInfo *Current() const;
  WordObjectTableIterator::operator++;
  WordObjectTableIterator::Reset;
};


#endif

