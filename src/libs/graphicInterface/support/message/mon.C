/* $Id: mon.C,v 1.1 1997/06/25 15:02:41 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/message.h>

extern "C" void _doprnt(char*,va_list,FILE*);

/* Print a message */
void message(char* format, ...)
{
va_list arg_list;

  va_start(arg_list, format);
    {
       _doprnt(format, arg_list, stdout);
    }
  va_end(arg_list);

  return;
}

void show_message2(char *dummy,...)
{
  return;
}

void hide_message2()
{
  return;
}

