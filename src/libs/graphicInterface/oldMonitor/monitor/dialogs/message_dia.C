/* $Id: message_dia.C,v 1.1 1997/06/25 14:47:38 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*                             message.c                                */
/*                          message dialog                              */
/*                                                                      */
/************************************************************************/

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include <libs/graphicInterface/oldMonitor/include/mon/dialog_def.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/message.h>
#include <libs/graphicInterface/oldMonitor/include/items/title.h>

STATIC(Boolean, message_handler, (Dialog* di, Generic owner, Generic item_id));
STATIC(void, message_helper, (Dialog* di, Generic owner, Generic item_id));

   /* Handle a modification to a message dialog.                           */
static Boolean message_handler(Dialog *di, Generic owner, Generic item_id)
{
    return DIALOG_QUIT;
}


   /* Handle a help event to a message dialog.                             */
static void message_helper(Dialog *di, Generic owner, Generic item_id)
{
    if ( item_id == DIALOG_DEFAULT_ID ) {
#ifndef lint
    	message("This is the message that was to be given:\nselect quit box or hit return to exit message dialog.");
#endif
    }
}


   /* Create a message window with text and return when clicked on.        */
void message(char* format, ...)
{
  va_list  arg_list;   /* argument list as per varargs */
  char     buf[1000];  /* the message buffer           */
  Dialog*  di;         /* the dialog instance          */

  va_start(arg_list,format);
    {
       vsprintf(buf, format, arg_list);
    }
  va_end(arg_list);

  di = dialog_create("message",
                     message_handler,
                     message_helper,
                     0,
                     item_title(DIALOG_DEFAULT_ID, buf, DEF_FONT_ID));

  dialog_modal_run(di);
  dialog_destroy(di);

  return;
}
