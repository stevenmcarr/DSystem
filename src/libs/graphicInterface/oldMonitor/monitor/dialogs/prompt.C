/* $Id: prompt.C,v 1.1 1997/06/25 14:47:38 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*                              prompt.c                                */
/*                           prompt dialog                              */
/*                                                                      */
/************************************************************************/

#include <stdlib.h>
#include <string.h>

#include <libs/graphicInterface/oldMonitor/include/mon/dialog_def.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/prompt.h>
#include <libs/graphicInterface/oldMonitor/include/items/text.h>

#define         TEXT_ITEM       1       /* the text item                */

STATIC(Boolean, prompt_handler, (Dialog* di, Generic owner, Generic item_id));

/* Handle a modification to a prompt dialog.                            */
static Boolean
prompt_handler(Dialog* di, Generic owner, Generic item_id)
   /* di - the dialog instance          */
   /* owner - the owner of the dialog   */
   /* item_id - the id of the item      */
{
    return (item_id == TEXT_ITEM) ? DIALOG_NOMINAL : DIALOG_QUIT;
}


/* Create a prompt dialogue from the specified information.             */
char*
prompted_string_edit(char* prompt, Point size, char* buffer)
   /* prompt - the prompt of the edit       */
   /* size - the size of the editor box   */
   /* buffer - the buffer to start with     */
{
Dialog          *di;                    /* the dialog instance          */
char            *buf;                   /* the buffer to edit           */

    buf = strcpy((char*)get_mem(strlen(buffer)+1, "prompted_string_edit() buffer copy"),
                  buffer);

    di = dialog_create("prompt",
                       prompt_handler,
	               (dialog_helper_callback)0,
#include <stdlib.h>

#include <stdlib.h>

#include <stdlib.h>

                       0,
                       item_text(TEXT_ITEM, prompt, DEF_FONT_ID, &buf, size.x));

    dialog_modal_run(di);
    dialog_destroy(di);
    return (buf);
}
