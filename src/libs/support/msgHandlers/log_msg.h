/* $Id: log_msg.h,v 1.5 1997/03/11 14:37:03 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *  log_msg.h : a facility for controlling debugging output from Rn
 */

#ifndef log_msg_h
#define log_msg_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

/*
 *  run a dialog that allows us to set Boolean variables arranged in
 *	a 2 dimensional display (with titles on the top and left)
 *
 *	(for argument description, see example below)
 */
EXTERN(void, run_2D_checkbox_dialog, (char *title, int columns, int rows, ...));


#define log_msg \
    log_msg_file_name   = __FILE__, \
    log_msg_line_number = __LINE__, \
    log_msg_internal


/*
 *  a printf command with a Boolean variable prepended (before the format)
 *	if the Boolean is true, then do the printf to stderr
 */
EXTERN(void, log_msg_internal, (Boolean do_it, char *format, ...));


/*
 * Example use:
 *  
 * the Booleans could be globals or instance variables
 *
 *	Boolean	ExTrace   = false,
 *		ExWarning = false,
 *		ExError   = true;
 *
 *	Boolean	RemTrace   = false,
 *		RemWarning = true,
 *		RemError   = true;
 *
 *	bar () {
 *	    ...
 *				   title of dialog		#cols	#rows
 *	    run_2D_checkbox_dialog("Set Exmon log levels", 	3,	2,
 *				"Traces",  "Warnings",  "Errors",
 *		"general     ", &ExTrace,  &ExWarning,  &ExError,
 *		"remote stuff", &RemTrace, &RemWarning, &RemError
 *	    );
 *	    ...
 *	}
 *
 *	foo () {
 *	    ...
 *	    log_msg(ExTrace, "<printf-style format>", arg1, arg2, ... );
 *	    ...
 *	}
 */


extern	int	 errno,			/* system error number		*/
		 sys_nerr;		/* # of system errors in table	*/
extern	char	*sys_errlist[];		/* system error string table	*/


extern	char	*log_msg_file_name;
extern	int	 log_msg_line_number;

/*
 *  perror_msg is the string that would be printed by perror
 *	this can be used (with a %s format) in a printf (or log_msg)
 */
#define	perror_msg (errno < sys_nerr ? sys_errlist[errno] : "Unknown system error--errno out of range")

#endif
