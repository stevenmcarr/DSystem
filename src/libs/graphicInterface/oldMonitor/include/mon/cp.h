/* $Id: cp.h,v 1.6 1997/03/11 14:33:13 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/* 									*/
/* 		     		    cp.h				*/
/* 		       Command processor abstraction			*/
/* 									*/
/* Command processors provide a generalized, high-level window		*/
/* abstraction for running tools and utilities within Rn.  Command	*/
/* processors may manage several windows.  Panes within those windows	*/
/* are placed using a tiling description.  Command processors may	*/
/* create other command processors and communicate to through an	*/
/* asynchronous message.						*/
/* 									*/
/************************************************************************/

#ifndef cp_h
#define cp_h

#include <libs/graphicInterface/oldMonitor/include/mon/sm.h>

typedef	enum	{			/* COMMAND PROCESSOR STATUS	*/
		CP_UNSTARTED,		/* CP has not been started	*/
		CP_CANNOT_START,	/* CP cannot be started		*/
		CP_NOMINAL		/* CP has been started		*/
		} cpStatus;

struct inst;
typedef struct inst anInstance;
struct cp_info;
typedef struct cp_info aCpMgr;
struct w_tile;
typedef struct w_tile aTilingDesc;

/********************* COMMAND PROCESSOR DEFINITION *********************/

typedef FUNCTION_POINTER(void, cp_root_starter_func,
 (Generic creator, short cp_index));
/* Takes two parameters (Generic creator) the instance number of the	*/
/* root creator, and (short cp_index) the index of the cp.  This routine*/
/* is called if the processor is selected from the root menu.  Many cp's*/
/* will want to set this to be cp_standard_root_starter() which will	*/
/* start the command processor with the root_startup_arg for its	*/
/* argument (above).  Note:  this routine will be called for the 0th cp	*/
/* in the processors structure on environment startup.			*/

typedef FUNCTION_POINTER(Boolean, cp_start_cp_func,
 (Generic mgr));
/* Takes one paramter, (Generic mgr) the manager id to use in the 	*/
/* cp_assert_need() call (and nowhere else).  Returns true if the	*/
/* command processor can start an instance.  Sets up storage and	*/
/* information global to all instances.					*/

typedef FUNCTION_POINTER(Generic, cp_create_instance_func,
 (Generic creator, Generic me, Generic arg));
/* Takes three parameters (Generic creator) the instance number of the	*/
/* creator, (Generic me) the instance number of the new cp and		*/
/* (Generic arg) the argument to be passed to the CP.  Returns an	*/
/* instance number or UNUSED if the instance could not start.		*/

typedef FUNCTION_POINTER(Boolean, cp_handle_input_func,
 (Generic instance, Generic generator, short event_type, Point coord,
 Generic msg));
/* Takes five parameters, (Generic instance) the instance number of the	*/
/* event, (Generic generator) who the event is from, (short event_type)	*/
/* the type of the event, (Point coord) and (Generic msg) event specific*/
/* information.  Returns true if the command processor should die.	*/

typedef FUNCTION_POINTER(void, cp_destroy_instance_func,
 (Generic instance, Boolean panicked));
/* Takes two parameters, (Generic instance) the instance to be killed	*/
/* and (Boolean panicked) which is true if the destroy was not requested*/
/* by the cp.  The command processor must delete its windows.		*/

typedef FUNCTION_POINTER(void, cp_finish_cp_func, (void));
/* Takes no parameters and returns nothing.  Allows the cp to free	*/
/* storage global to all instances.  All cp's of this type will have	*/
/* been killed at the time of this call.				*/

typedef
struct	cp	{			/* COMMAND PROCESSOR DEFINITION	*/

char		*name;			/* the name of the cp		*/
/* This string is also used as the root menu choice if root_startup is	*/
/* true.								*/

Boolean		root_startup;		/* put this on the root menu	*/

Generic		root_startup_arg;	/* provided startup argument	*/
/* Will be used by cp_standard_root_starter() as the argument.		*/

cp_root_starter_func
		root_starter;		/* start cp from root menu	*/

cp_start_cp_func
		start_cp;		/* start the use of the cp	*/

cp_create_instance_func
		create_instance;	/* create an instance		*/

cp_handle_input_func
		handle_input;		/* handle event input		*/

cp_destroy_instance_func
		destroy_instance;	/* kill an instance		*/

cp_finish_cp_func
		finish_cp;		/* finish with the cp		*/

cpStatus	status;			/* cp status code (private)	*/
/* This variable should be initially set to CP_UNSTARTED if it is a	*/
/* normal command processor and CP_CANNOT_START if a command processor	*/
/* instance is never to be started.  The variable SHOULD NOT be modified*/
/* during any command processor function.				*/

		} aProcessor;

EXTERN(void, cp_standard_root_starter, (anInstance *creator,
 short cp_index));
/* This call is meant to be installed as the root_starter field (above)	*/
/* to start the cp from the root menu in the most typical way.		*/


/*************** COMMAND PROCESSOR INSTANCE MANIPULATION ****************/

EXTERN(Boolean, cp_assert_need, (aCpMgr *m, short cp_index));
/* Takes two parameters (Generic m) the manager parameter passed in	*/
/* from the command processor start call and (short cp_index) the index	*/
/* of the command processor of interest and if the command processor has*/
/* been started it returns true; if the command processor hasn't, it is	*/
/* started if possible.  Returns false if asserted command processor	*/
/* could not be started.						*/

EXTERN(Generic, cp_new, (anInstance *creator, short cp_index,
 Generic arg));
/* Takes three parameters (Generic creator) the command processor id	*/
/* who is creating the child, (short cp_index) the command processor    */
/* type index and (Generic arg) the argument to the command processor.	*/
/* Returns the command processor instance descriptor or 0 if the	*/
/* instance could not be started.					*/

EXTERN(Generic, cp_root_cp_id, (void));
/* Takes no parameters.  Returns the id of the root command processor.	*/
/* This is for cp_new() calls when a parent command processor is not	*/
/* available.								*/

EXTERN(Boolean, cp_message, (anInstance *sender, anInstance *receiver,
 Point info, Generic msg));
/* Takes four parameters (Generic sender, receiver) the sender and	*/
/* receiver of the message, (Point info) the coodinate of the message	*/
/* and (Generic msg) message itself.  Returns true if the message will	*/
/* be delivered and false otherwise.  Note:  messages command processors*/
/* send to themselves during the destroy_instance will not be received	*/
/* regardless of return code.						*/

EXTERN(Boolean, cp_assert_death, (anInstance *dier));
/* Takes one parameter (Generic dier) the command processor instance in	*/
/* question.  Returns true if there are no 'interesting' messages	*/
/* pending for the command processor instance.				*/


/******************* CP EXPECTED EVENT REGISTRATION *********************/

EXTERN(void, cp_register_async_fd, (short fd, anInstance *inst,
 Boolean buffer));
/* Takes three parameters (short fd) the file descriptor to begin	*/
/* polling for asynchronous input, (Generic inst) the owner cp of	*/
/* that file descriptor, and (Boolean buffer) true if the input should  */
/* be read and buffered and false if only notification should be given.	*/
/* All registered file desciptors must be unregistered.			*/

EXTERN(void, cp_unregister_async_fd, (short fd, anInstance *inst));
/* Takes two parameters (short fd) the file descriptor which is no	*/
/* longer of interest and (Generic inst) the owner cp of that file	*/
/* descriptor.								*/

EXTERN(void, cp_register_process, (int pid, anInstance *inst));
/* Takes two parameters (int pid) the process id of interest and	*/
/* (Generic inst) the cp to which to send an EVENT_SIGCHILD.  All	*/
/* registered processes must be unregistered.				*/

EXTERN(void, cp_unregister_process, (int pid, anInstance *inst));
/* Takes two parameters (int pid) the process id which is no longer of	*/
/* interest and (Generic inst) the owner cp of that file descriptor.	*/


/****************** TILING DESCRIPTION MANIPULATION *********************/

typedef	enum	{			/* TILING DIRECTION		*/
		TILE_LEFT,		/* first tiling LEFT of second	*/
		TILE_RIGHT,		/* first tiling RIGHT of second	*/
		TILE_UP,		/* first tiling ABOVE second	*/
		TILE_DOWN		/* first tiling BELOW second	*/
		} aTilingDir;
#define		NULL_TD    ((Generic) 0)/* the null tiling descriptor	*/

EXTERN(Generic, cp_td_pane, (Pane **ppr, Point size));
/* Takes two parameters (Generic *pptr) pointer to the identifier	*/
/* of the pane and (Point size) the desired size of the pane.  Returns	*/
/* the resulting tiling description.  Note:  the pane id should be	*/
/* initially set to the screen module number for which this pane should	*/
/* become.								*/

EXTERN(Generic, cp_td_join, (aTilingDir dir, aTilingDesc *td1,
 aTilingDesc *td2));
/* Takes three parameters (aTilingDir dir) the direction of the join,	*/
/* (Generic td1, td2) the tiling descriptions of the two joinees.	*/
/* Returns the resulting tiling description.				*/

EXTERN(Point, cp_td_size, (aTilingDesc *td));
/* Takes one parameter (Generic td) the tiling description.		*/
/* Returns the size of the description.					*/

/*************** COMMAND PROCESSOR WINDOW MANIPULATION ******************/

EXTERN(void, cp_window_tile, (Window **wptr, anInstance *owner,
                              aTilingDesc *td));
/* Takes three parameters (Window **wptr) the pointer to a window id	*/
/* (for a new window, the id should be set to one of the values below to*/
/* specify resizability), (Genric owner) the cp id owning the window	*/
/* and (Generic td) the tiling description for the window.  The window	*/
/* is created if necessary and the window id is put into *wptr.		*/

EXTERN(void, cp_window_tile_hidden, (Window **wptr, anInstance *owner, 
                                     aTilingDesc *td));
/* Takes three parameters (Generic *wptr) the pointer to a window id	*/
/* (for a new window, the id should be set to one of the values below to*/
/* specify resizability), (Generic owner) the cp id owning the window	*/
/* and (Generic td) the tiling description for the window.  The		*/
/* window is created if necessary and the window id is put into *wptr.	*/
/* The window is not made visible.					*/

#define		CP_WIN_NOT_RESIZABLE  0/* new window cannot be resized	*/
#define		CP_WIN_RESIZABLE      1/* new window can be resized	*/

EXTERN(void, cp_window_set_title, (Window* w, char *format, ...));
/* Takes a variable number of arguments: (Window* w) the window id,	*/
/* (char *format) the printf style of format string and any other	*/
/* arguments as necessary by format.  The new title is set.		*/

EXTERN(void, cp_window_to_top, (Window *w));
/* Takes one parameter (Generic w) the window id.  The window is	*/
/* brought to the top.							*/

EXTERN(void, cp_window_hide, (Window *w));
/* Takes one parameter (Generic w) the window id.  The window is	*/
/* hidden.   								*/

EXTERN(void, cp_window_show, (Window *w));
/* Takes one parameter (Genericw) the window id.  The window is shown,	*/
/* without bringing it to the top.					*/

EXTERN(void, cp_window_destroy, (Window *w, anInstance *owner));
/* Takes two parameters (Generic w) the window to be destroyed and	*/
/* (Generic owner) the cp id owning the window.				*/

#endif
