/* $Id: dialog_mgr.h,v 1.3 1997/03/11 14:33:46 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*				dialog_mgr.h				*/
/*			dialogue manager include file			*/
/*									*/
/************************************************************************/

extern
aManager	dialog_manager;		/* the dialogue manager		*/

extern	Generic	dialog_mgr_use();	/* set the dialog manager to use*/
/* Takes one parameter:	 (Generic mgr) the manager specific information	*/
/* field.  All dialogue creations subsequent to this call use the	*/
/* mentioned dialogue manager.  Returns the old handle.			*/
