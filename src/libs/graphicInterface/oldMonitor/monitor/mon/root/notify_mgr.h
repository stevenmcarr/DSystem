/* $Id: notify_mgr.h,v 1.4 1997/03/11 14:33:48 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*                              notify_mgr.h                            */
/*                      notify manager include file                     */
/*                                                                      */
/************************************************************************/

extern
aManager        notify_manager;         /* the notify manager           */

EXTERN(Generic, notify_mgr_use,(Generic num)); /* set the notify manager to use*/
/* Takes one parameter:  (Generic mgr) the manager specific information */
/* field.  All notify manipulation subsequent to this call use the      */
/* mentioned notify manager.  Returns the old handle.                   */
