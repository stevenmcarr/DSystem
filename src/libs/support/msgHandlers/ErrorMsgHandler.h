/* $Id: ErrorMsgHandler.h,v 1.1 1997/03/11 14:37:00 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// 
// ErrorMsgHandler.h: 
// 
// Author:  John Mellor-Crummey                               October 1993
//
// Copyright 1993, Rice University
//                                                                          
//***************************************************************************

#ifndef ErrorMsgHandler_h
#define ErrorMsgHandler_h

#ifndef MsgHandler_h
#include <libs/support/msgHandlers/MsgHandler.h>
#endif

class ErrorMsgHandler : public MsgHandler {
private:
  virtual MsgHandlerStack *GetStack();
  virtual void SetStack(MsgHandlerStack *stack);
  static MsgHandlerStack *errorMsgHandlerStack;
public:  
  MsgHandler::PushHandler;
  MsgHandler::PopHandler;
  MsgHandler::HandleMsg;
};

extern ErrorMsgHandler errorMsgHandler;

#endif
