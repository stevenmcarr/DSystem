/* $Id: message_dia2.C,v 1.1 1997/06/25 14:47:38 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*                             message2.c                               */
/*                                                                      */
/************************************************************************/

#include <stdarg.h>
#include <stdlib.h>

#include <libs/graphicInterface/oldMonitor/include/mon/dialog_def.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/message.h>
#include <libs/graphicInterface/oldMonitor/include/items/title.h>
#include <libs/support/strings/rn_string.h>
#include <string.h>
#include <sys/time.h>
#include <stdio.h>

#define MIN_TIME	2.0		/* min seconds message must be up*/
#define BUF			1000	/* size of message buffer	 */

/* local variables */

static struct timeval startTime;	/* the copyright "up" time	*/
static Dialog *di = 0; 			/* the dialog instance          */

/*--------------------------------------------------*/
/* Handle a modification to the message2 dialog		*/

static Boolean
message2_handler(Dialog *di_inst, Generic *owner, Generic item_id)
{
    return DIALOG_QUIT;
}

/*-----------------------------------*/
/* Create a message window with text */

void
show_message2(char *format, ...)
{
	va_list         arg_list;		/* argument list as per varargs */
	char            buf[1024];		/* the message buffer           */

	if (di != 0)
	{
	  fprintf(stderr, "show_message2: message appears to still be showing.\n");
	  exit(-1);
	}

	va_start(arg_list, format);
	vsprintf(buf, format, arg_list);
	va_end(arg_list);

	di = dialog_create("message", (dialog_handler_callback)message2_handler, 
                           (dialog_helper_callback) 0, 0, item_title(DIALOG_DEFAULT_ID, buf, 
                           DEF_FONT_ID));

	dialog_modeless_show(di);
	(void) gettimeofday(&startTime, (struct timezone *) 0);
}

/*----------------------------------------*/
/* Take down & destroy the message window */

void
hide_message2()
{
	struct	timeval	currentTime;	/* the current time time		*/
	float		elapsed;			/* the elapsed time in seconds	*/

	if (di == 0)
	{
	  fprintf(stderr, "hide_message2: no message is showing.\n");
	  exit(-1);
	}

	do	/* if needed, busy-wait until message has been up long enough */
	{
		(void) gettimeofday(&currentTime, (struct timezone *) 0);

		elapsed = currentTime.tv_sec - startTime.tv_sec +
		   (float)(currentTime.tv_usec - startTime.tv_usec)/1000000.0;
	}
	while (elapsed < MIN_TIME);

	dialog_destroy(di);

	/* null it out... */
	di = 0;
}

