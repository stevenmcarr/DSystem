/* $Id: cp_mgr.h,v 1.5 1997/06/25 14:52:22 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*                                cp_mgr.h                              */
/*                 command processor manager include file               */
/*                                                                      */
/************************************************************************/

#ifndef cp_mgr_h
#define cp_mgr_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#include <libs/graphicInterface/oldMonitor/include/mon/cp.h>

extern aManager cp_manager;             /* the cp manager               */

EXTERN(Generic, cp_mgr_use, (Generic));
/* set the cp manager to use  */
/* Takes one parameter:  (Generic mgr) the manager specific information */
/* field.  All cp manipulation subsequent to this call use the mentioned*/
/* cp manager.  Returns the old handle.                                 */

EXTERN(void, cp_mgr_initialize, (aCpMgr *m, short num_porcessors,
         aProcessor **processors));
/* initialize the cp manager  */
/* Takes three parameters:  (Generic mgr) the manager instance to init- */
/* ialize, (short num) the number of command processors in the following*/
/* array, and (aProcessor **pro) the list of available command process- */
/* ors.  The manager should only be initialized once per instance.      */

EXTERN(void, cp_mgr_run, (aCpMgr *m, short startup));
/* run the cp manager          */
/* Takes two parameters:  (Generic mgr) the manager instance to start   */
/* and (short start) the index of the command processor to start or     */
/* no processors will be started if start==UNUSED.  The manager should  */
/* only be started once per instance.                                   */

#endif
