/* $Id: MsgHandler.h,v 1.1 1997/03/11 14:37:01 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// 
// MsgHandler.h: 
// 
// Author:  John Mellor-Crummey                               October 1993
//
// Copyright 1993, Rice University
//                                                                          
//***************************************************************************

#ifndef MsgHandler_h
#define MsgHandler_h

#include <stdarg.h>

class MsgHandlerStack; // minimal external declaration

typedef int (*MsgHandlerCallBack)(void *handle, const char *format, 
				  va_list args);

class MsgHandler {
public:
  void PushHandler(MsgHandlerCallBack fn, void *handle);
  void *PopHandler(); // returns handle
  int HandleMsg(const char *format, ...);
protected:
  int HandleMsgInternal(const char *format, va_list args);
  virtual MsgHandlerStack *GetStack() = 0;
  virtual void SetStack(MsgHandlerStack *) = 0;
};


#endif
