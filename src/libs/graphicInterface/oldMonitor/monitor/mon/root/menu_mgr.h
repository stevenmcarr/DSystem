/* $Id: menu_mgr.h,v 1.4 1997/03/11 14:33:48 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*                               menu_mgr.h                             */
/*                        menu manager include file                     */
/*                                                                      */
/************************************************************************/

#ifndef menu_mgr_h
#define menu_mgr_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef manager_h
#include <libs/graphicInterface/oldMonitor/include/mon/manager.h>
#endif

struct menu_mgr;
typedef struct menu_mgr aMenuMgr;

extern aManager menu_manager;           /* the menu manager             */

EXTERN(Generic, menu_mgr_use, (aMenuMgr* mm));
/* set the menu manager to use  */
/* Takes one parameter:  (Generic mgr) the manager specific information */
/* field.  All menu creations subsequent to this call use the mentioned */
/* menu manager.  Returns the old handle.                               */

EXTERN(void, menu_mgr_resize_notify, (aMenuMgr* mm));
/* notify of parent pane resize*/
/* Takes one parameter:  (Generic mgr) the system manager instance      */
/* handle.  This is to be called whenever the manager's desk pane is    */
/* resized.                                                             */

#endif
