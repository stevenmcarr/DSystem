/* $Id: log_msg.C,v 1.1 1997/06/25 15:17:31 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *  log message facility
 *
 *    includes:
 *	--routine that conditionally prints logging messages
 *  
 *	R. Hood,	Aug. 1988
 */

#include <stdio.h>
#include <stdarg.h>

#include <libs/support/msgHandlers/log_msg.h>

char*  log_msg_file_name;
int    log_msg_line_number;

/*
 *  if type is true then print out a log message to stderr
 *	(with a copy to logfile if that is enabled)
 */
void log_msg_internal (Boolean do_it, char* format, ...)
{
  va_list  arg_list;

  va_start(arg_list, format);
    {
      if (do_it) 
        {
          fprintf(stderr, "line %4d of %-16s: ", log_msg_line_number, log_msg_file_name);
          vfprintf(stderr, format, arg_list);
        }
    }
  va_end(arg_list);

  return;
}
