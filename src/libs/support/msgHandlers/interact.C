/* $Id: interact.C,v 1.1 1997/06/25 15:17:31 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <stdarg.h>
#include <stdio.h>

#include <libs/support/msgHandlers/interact.h>

/* Print a message. */
void message_non_windowing(char* format, ...)
{
  va_list  arg_list;               /* arument list as per varargs  */

  va_start(arg_list, format);
    {
       vfprintf(stdout, format, arg_list);
    }
  va_end(arg_list);
}


Boolean yes_no_non_windowing(char *prompt, Boolean *answer, Boolean def)
{
  char buffer[80];

  printf("%s [YyNn]?\n", prompt);
  while (gets(buffer) != NULL)
  {
    if (buffer[0] == 'Y' || buffer[0] == 'y')
    {
      *answer = true;
      return *answer;
    }
    else if (buffer[0] == 'N' || buffer[0] == 'n')
    {
      *answer = false;
      return *answer;
    }
    else
    {
      printf("[YyNn]?\n", prompt);
    }
  }

  /* must be EOF, return the default */
  *answer = def;
  return *answer;
}

