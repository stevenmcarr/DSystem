/* $Id: dialog_mgr.h,v 1.4 1997/06/25 14:52:22 carr Exp $ */
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

struct dia_info;
typedef struct	dia_info aDiaMgr;		/* MANAGER INFORMATION STRUCT.	*/

EXTERN(Generic, dialog_mgr_use,(aDiaMgr *dm));
/* set the dialog manager to use*/
/* Takes one parameter:	 (Generic mgr) the manager specific information	*/
/* field.  All dialogue creations subsequent to this call use the	*/
/* mentioned dialogue manager.  Returns the old handle.			*/
