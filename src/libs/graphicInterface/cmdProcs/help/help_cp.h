/* $Id: help_cp.h,v 1.5 1997/03/11 14:30:30 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/****************************************************************/
		/* 			   helpcp.h				*/
		/* 								*/
		/*	 Help command processor users' include file.		*/
		/* 								*/
		/****************************************************************/

/* Additional information is available through Rn newsletter 12 and the author.		*/

#ifndef help_cp_h
#define help_cp_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

EXTERN(void, help_cp_give_help, (Generic owner, char *file_name, short num_valid,
 short *arg_list, short position_number));
/* give/refresh help session		*/
/* Takes five parameters (Generic owner) the id of the owner (either a cp id or a dialog*/
/* instance), (char *file_name) the name of the help session file, (short num_valid) the*/
/* number of valid arguments, (short *arg_list) the startup arguments, and (short       */
/* position) the startup positioning parameter.  An existing help session is refreshed  */
/* or a new session is created.								*/

EXTERN(void, help_cp_terminate_help, (Generic owner));
/* remove any dependent help session	*/
/* Takes one parameter (Generic owner) the id of the owner (either a cp id or a dialog	*/
/* instance).  Any help session attached to this cp or dialog is terminated.		*/

#endif
