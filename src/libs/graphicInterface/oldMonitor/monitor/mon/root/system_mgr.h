/* $Id: system_mgr.h,v 1.5 1997/03/11 14:33:50 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*                              system_mgr.h                            */
/*                       system manager include file                    */
/*                                                                      */
/************************************************************************/

struct sys_info;
typedef struct sys_info aSysMgr;

extern
aManager        sys_manager;            /* the system manager           */

EXTERN(void, sys_mgr_initialize,(aSysMgr *m, PFV redrawer, PFV quitter));   
                                         /* initialize the system mgr   */
/* Takes three parameters:  (Generic m) the system manager instance     */
/* handle, (PFV redrawer) the zero argument call to redraw the screen,  */
/* and (PFV quitter) the zero argument call to quit.                    */

