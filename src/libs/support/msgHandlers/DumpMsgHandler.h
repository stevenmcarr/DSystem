/* $Id: DumpMsgHandler.h,v 1.1 1997/03/11 14:36:59 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef DumpMsgHandler_h
#define DumpMsgHandler_h

//***************************************************************************
// 
// DumpMsgHandler.h: 
// 
// Author:  John Mellor-Crummey                               July 1994
//
// Copyright 1994, Rice University
//                                                                          
//***************************************************************************


#include <stdarg.h>


//***************************************************************************
// class DumpMsgHandler
//***************************************************************************
class DumpMsgHandler {
public: 
  void BeginScope();
  void EndScope();
  int Dump(const char *format, ...); 
private:
  static unsigned int nestingDepth;
};


//***************************************************************************
// external declarations
//***************************************************************************
extern DumpMsgHandler dumpHandler;

#endif
