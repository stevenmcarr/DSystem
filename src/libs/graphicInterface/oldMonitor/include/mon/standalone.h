/* $Id: standalone.h,v 1.9 1997/06/25 14:46:17 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef standalone_h
#define standalone_h

/* 
 * external definitions for stand alone support routines 
 * provided by the Rn/ParaScope monitor (for now)
 *
 * John Mellor-Crummey                    July 1992
 */

#ifndef general_h
#include <libs/support/misc/general.h>
#endif 

#ifndef point_h
#include <libs/graphicInterface/support/graphics/point.h>
#endif 

#ifndef rect_h
#include <libs/graphicInterface/support/graphics/rect.h>
#endif 

#ifndef rect_list_h
#include <libs/graphicInterface/support/graphics/rect_list.h>
#endif

typedef FUNCTION_POINTER(int,OptsProcessFunc,(int,char**));
typedef FUNCTION_POINTER(int,RootStartupFunc,(int,char**));

EXTERN(void, sm_desk_register_fd, ( 
	short fd        /* file descriptor being registered */,
	Generic mi      /* manager instance of the event */,
	Boolean	buffer  /* input should be buffered	*/,
	Boolean	urgent  /* true if input is urgent	*/));

EXTERN(void, sm_desk_unregister_fd, (
	short fd        /* file descriptor being registered */,
	Generic mi      /* manager instance of the event */));

EXTERN(void, sm_desk_register_process, (
	int     pid     /* the process id */,
	Generic mi      /* manager instance of the event*/,
	Boolean urgent  /* true if input is urgent	*/));

EXTERN(void, sm_desk_unregister_process, (
	int     pid     /* the process id */,
	Generic mi      /* manager instance of the event*/));

EXTERN(void, redrawRootRectList, (
	RectList rl     /* the regions to redraw */));

EXTERN(void, resizeRoot, (
	Point size      /* the new root size */));


/* Create, run, & destroy the root	*/
typedef FUNCTION_POINTER(int, opts_process_callback, (int argc, char **argv));
typedef FUNCTION_POINTER(int, root_func_callback,    (int argc, char **argv));

EXTERN(int, runRoot, (
        int argc,
	char **argv,
	opts_process_callback opts_process,
        root_func_callback root_func));

EXTERN(void, dbInit, (void));
EXTERN(void, dbFini, (void));
EXTERN(void, monInit, (void));
EXTERN(void, monFini, (void));

EXTERN(void, databaseCheckNotification, (void));

EXTERN(void, databaseWaitNotification, (void));

#endif /* standalone_h */
