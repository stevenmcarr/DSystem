/* $Id: ErrorMsgHandler.C,v 1.1 1997/03/11 14:37:00 carr Exp $ */
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

#include <stdio.h>

#include <libs/support/msgHandlers/ErrorMsgHandler.h>



//*******************************************************************
// declarations 
//*******************************************************************


ErrorMsgHandler errorMsgHandler;

MsgHandlerStack *ErrorMsgHandler::errorMsgHandlerStack = 0;



//**********************
// forward declarations
//**********************

static int DefaultErrorMsgHandler(void *, const char *format, va_list args);



//***************************************************************************
// class FortranComposition interface operations
//***************************************************************************


MsgHandlerStack *ErrorMsgHandler::GetStack()
{
  return errorMsgHandlerStack;
}


void ErrorMsgHandler::SetStack(MsgHandlerStack *stack)
{
  errorMsgHandlerStack = stack;
  PushHandler(DefaultErrorMsgHandler, 0);
}



//***************************************************************************
// private operations 
//***************************************************************************


static int DefaultErrorMsgHandler(void *, const char *format, va_list args)
{
  return vfprintf(stderr, format, args);
}



