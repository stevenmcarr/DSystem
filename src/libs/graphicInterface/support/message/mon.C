/* $Id: mon.C,v 1.2 1999/06/11 21:21:47 carr Exp $ */
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
#ifdef LINUX
       vprintf(format, arg_list);
#else
       _doprnt(format, arg_list, stdout);
#endif
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

