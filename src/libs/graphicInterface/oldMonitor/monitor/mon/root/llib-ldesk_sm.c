/* $Id: llib-ldesk_sm.c,v 1.4 1997/03/11 14:33:46 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/* LINTLIBRARY */
/* Lint library for managers */

#include <libs/graphicInterface/oldMonitor/include/mon/sm_def.h>
#include <libs/graphicInterface/oldMonitor/include/mon/manager.h>
#include <libs/graphicInterface/oldMonitor/monitor/mon/root/desk_sm.h>


/************************ SCREEN MODULE CALLBACKS ***********************/

short		sm_desk_get_index()				{return 0;}
void		sm_desk_initialize(p, id, run, rf, urf, rc, urc) Pane *p; Generic id; PFV run, rf, urf, rc, urc; {}
Generic		sm_desk_use_manager(p, mgr)			Pane *p; aManager *mgr; {return (Generic) 0;}
