/* $Id: MsgHandler.C,v 1.1 1997/03/11 14:37:01 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// 
// MsgHandler.C: 
// 
// Author:  John Mellor-Crummey                               October 1993
//
// Copyright 1993, Rice University
//                                                                          
//***************************************************************************

#include <libs/support/msgHandlers/MsgHandler.h>
#include <libs/support/stacks/PointerStack.h>

class MsgHandlerStack : public PointerStack {
};

struct MsgHandlerPair {
  MsgHandlerCallBack fn;
  void *handle;
  MsgHandlerPair(MsgHandlerCallBack _fn, void *_handle) {
    fn = _fn;
    handle = _handle; 
  };
};


MsgHandlerStack *MsgHandler::GetStack()
{
  MsgHandlerStack *stack = GetStack();

  if (stack == 0) {
    stack = new MsgHandlerStack;
    SetStack(stack);
  }

  return stack;
}


void MsgHandler::PushHandler(MsgHandlerCallBack fn, void *handle)
{
  MsgHandler::GetStack()->Push(new MsgHandlerPair(fn, handle));
}


void *MsgHandler::PopHandler()
{
  MsgHandlerPair *p = (MsgHandlerPair *) MsgHandler::GetStack()->Pop();
  void *handle = p->handle;
  delete p;
  return handle;
}


int MsgHandler::HandleMsg(const char *format, ...)
{
  va_list args;
  va_start(args, format);

  int retval = HandleMsgInternal(format, args);

  va_end(args);
  return retval;
}


int MsgHandler::HandleMsgInternal(const char *format, va_list args)
{
  MsgHandlerPair *p = (MsgHandlerPair *) MsgHandler::GetStack()->Top();
  return (*p->fn)(p->handle, format, args);
}

