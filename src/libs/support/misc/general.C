/* $Id: general.C,v 1.1 1997/06/25 15:16:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 			general.c			*/
		/* 							*/
		/*	 General utilities for the Rn environment.	*/
		/* 							*/
		/********************************************************/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <libs/support/misc/general.h>

   /* Find the maximum of two integers. */
int max(int a, int b)
{
  return ((a > b) ? a : b);
}


   /* Find the minimum of two integers. */
int min(int a, int b)
{
  return ((a < b) ? a : b);
}


   /* Print error message and die. */
void die_with_message(char* format, ...)
{
  va_list  arg_list;		/* the argument list as per varargs	*/

  va_start(arg_list, format);
    {
       fprintf(stderr, "Abnormal termination:  ");
       vfprintf(stderr, format, arg_list);
       fprintf(stderr, "\n");
    }
  va_end(arg_list);

  abort();

  return;
}
