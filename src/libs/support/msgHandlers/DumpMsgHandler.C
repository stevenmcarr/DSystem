/* $Id: DumpMsgHandler.C,v 1.1 1997/03/11 14:36:59 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// 
// DumpMsgHandler.h: 
// 
// Author:  John Mellor-Crummey                               October 1993
//
// Copyright 1993, Rice University
//                                                                          
//***************************************************************************

#include <stdio.h>

#include <libs/support/msgHandlers/DumpMsgHandler.h>



//*******************************************************************
// declarations 
//*******************************************************************


DumpMsgHandler dumpHandler;

unsigned int DumpMsgHandler::nestingDepth = 0;


//**********************
// forward declarations
//**********************


//***************************************************************************
// class DumpMsgHandler interface operations
//***************************************************************************



void DumpMsgHandler::BeginScope()
{
  nestingDepth++;
}


void DumpMsgHandler::EndScope()
{
  nestingDepth--;
}


int DumpMsgHandler::Dump(const char *format, ...)
{
  int retval = 0;

  va_list args;
  va_start(args, format);

  retval = fprintf(stderr, "%*s", 2 * nestingDepth, "");
  if (retval < 0) return retval;
  retval = vfprintf(stderr, format, args);

  va_end(args);
  return retval;
}


