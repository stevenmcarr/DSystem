/* $Id: TraceMsgHandler.h,v 1.1 1997/03/11 14:37:02 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// 
// TraceMsgHandler.h: 
// 
// Author:  John Mellor-Crummey                               October 1993
//
// Copyright 1993, Rice University
//                                                                          
//***************************************************************************

#ifndef TraceMsgHandler_h
#define TraceMsgHandler_h

#ifndef MsgHandler_h
#include <libs/support/msgHandlers/MsgHandler.h>
#endif

class TraceMsgHandler : private MsgHandler {
public: 
  MsgHandler::PushHandler;
  MsgHandler::PopHandler;
  void BeginScope();
  void EndScope();
  int HandleMsg(unsigned int level, const char *format, ...); 
  void SetTraceLevel(unsigned int level);
private:
  virtual MsgHandlerStack *GetStack();
  virtual void SetStack(MsgHandlerStack *stack);
  static MsgHandlerStack *TraceMsgHandlerStack;
  static unsigned int traceLevel;
  static unsigned int nestingDepth;
};

extern TraceMsgHandler traceMsgHandler;

#endif
