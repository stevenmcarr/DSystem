/* $Id: TraceMsgHandler.C,v 1.1 1997/03/11 14:37:01 carr Exp $ */
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

#include <stdio.h>

#include <libs/support/msgHandlers/TraceMsgHandler.h>



//*******************************************************************
// declarations 
//*******************************************************************


TraceMsgHandler traceMsgHandler;

MsgHandlerStack *TraceMsgHandler::TraceMsgHandlerStack = 0;
unsigned int TraceMsgHandler::traceLevel = 1;
unsigned int TraceMsgHandler::nestingDepth = 0;



//**********************
// forward declarations
//**********************

static int DefaultTraceMsgHandler(void *, const char *format, va_list args);



//***************************************************************************
// class FortranComposition interface operations
//***************************************************************************


MsgHandlerStack *TraceMsgHandler::GetStack()
{
  return TraceMsgHandlerStack;
}


void TraceMsgHandler::SetStack(MsgHandlerStack *stack)
{
  TraceMsgHandlerStack = stack;
  PushHandler(DefaultTraceMsgHandler, 0);
}

void TraceMsgHandler::SetTraceLevel(unsigned int level)
{
  traceLevel = level;
}


void TraceMsgHandler::BeginScope()
{
  nestingDepth++;
}


void TraceMsgHandler::EndScope()
{
  nestingDepth--;
}


int TraceMsgHandler::HandleMsg(unsigned int level, const char *format, ...)
{
  int retval = 0;

  va_list args;
  va_start(args, format);

  if (level <= traceLevel) 
    retval = MsgHandler::HandleMsg(format, nestingDepth, args);

  va_end(args);
  return retval;
}


//***************************************************************************
// private operations 
//***************************************************************************


static int DefaultTraceMsgHandler(void *, const char *format, va_list args)
{
  unsigned int nestingDepth = va_arg(args, unsigned int);
  va_list userArgs = va_arg(args, va_list);
  
  int code =  fprintf(stderr, "%*s", 2 * nestingDepth, "");
  if (code < 0) return code;
  return  vfprintf(stderr, format, userArgs);
}
