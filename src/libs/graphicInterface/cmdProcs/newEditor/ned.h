/* $Id: ned.h,v 1.5 1997/03/11 14:30:37 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/ned.h							*/
/*									*/
/*	ned.h -- Common stuff for New Fortran Editor modules		*/
/*	Last edited: September 23, 1992 at 4:17 pm			*/
/*									*/
/************************************************************************/




/* standard Unix stuff */

#include <stdio.h>




/* standard Rn stuff */

/* these were in <cp_def.h> and/or <sm_def.h> */
#include <libs/support/misc/general.h>
#include <libs/graphicInterface/oldMonitor/monitor/keyboard/kb.h>
#include <libs/graphicInterface/support/graphics/point.h>
#include <libs/graphicInterface/support/graphics/rect.h>
#include <libs/graphicInterface/support/graphics/rect_list.h>
#include <libs/support/memMgmt/mem.h>
#include <libs/graphicInterface/oldMonitor/include/mon/font.h>
#include <libs/graphicInterface/oldMonitor/include/mon/menu.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/prompt.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/message.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/yes_no.h>
#include <libs/graphicInterface/oldMonitor/include/mon/gfx.h>
#include <libs/graphicInterface/oldMonitor/include/mon/event.h>
#include <libs/graphicInterface/oldMonitor/include/mon/event_codes.h>
#include <libs/graphicInterface/oldMonitor/include/mon/system.h>

#include <include/maxdefs.h>
#include <libs/support/strings/rn_string.h>
#include <string.h>
#include <libs/support/database/context.h>
#include <libs/support/database/newdatabase.h>




/* our own standard stuff */

#include <libs/graphicInterface/cmdProcs/newEditor/Utility.h>

#ifndef nil
#define nil	((Generic) 0)
#endif

#define MAXNAME	64




/* change notification codes shared by many modules */

#define NOTIFY_SEL_CHANGED        0
#define NOTIFY_DOC_WILL_CHANGE   -1
#define NOTIFY_DOC_CHANGED        1



/* save version codes */

#define NED_SAVE_CURRENT	  8	/* current version */
#define NED_SAVE_OLDEST		  8	/* oldest readable version */
