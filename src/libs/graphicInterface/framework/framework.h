/* $Id: framework.h,v 1.4 1997/06/25 14:43:41 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/framework.h						*/
/*									*/
/*	Generally useful definitions for the CP Framework modules	*/
/*      Last edited: August 26, 1993 at 5:42 pm				*/
/*									*/
/************************************************************************/




#ifndef framework_h
#define framework_h




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

#include <stdio.h>

#include <libs/graphicInterface/cmdProcs/newEditor/Utility.h>

#include <libs/graphicInterface/framework/CObject.h>
#include <libs/graphicInterface/framework/Change.h>







#ifndef nil
#define nil    0
#endif

#define INFINITY  9999		/* must fit in a 'short' */




#define SUBCLASS_RESPONSIBILITY(what)					\
									\
	die_with_message("%s is a subclass responsibility", what)




#define NOT_CALLED(what)						\
									\
	die_with_message("%s should not be called", what)




#define NOT_IMPLEMENTED(what)						\
									\
	die_with_message("%s is not implemented", what)




#define ASSERT(predicate)						\
									\
	if( ! (predicate) )						\
	  die_with_message("Assertion failure")






/**************************************/
/* Stuff from Utility.h -- TEMPORARY? */
/**************************************/




/* alerts */


EXTERN (void, notImplemented, (char * what));


EXTERN (void, CHECK, (int what));




/* rectangles */


EXTERN (void, setRect, (Rectangle *r, int left, int top, int right, 
			int bottom));


EXTERN (void, setRectSize, (Rectangle *r, int left, int top, int width, 
			    int height));




/* mouse */


EXTERN (Boolean, stillDown, (Point *mousePt));







#endif /* framework_h */
