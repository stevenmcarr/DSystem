/* $Id: regerror.C,v 1.1 1997/06/25 15:18:28 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/support/misc/general.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/message.h>

EXTERN(void, regerror, (char* s));

void regerror(char* s)
{
  message("regexp: %s",s);

  return;
}
