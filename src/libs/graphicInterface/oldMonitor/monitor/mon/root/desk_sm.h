/* $Id: desk_sm.h,v 1.7 1997/06/25 14:52:22 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*			       desk_sm.h				*/
/*			desk manager abstraction			*/
/*									*/
/************************************************************************/

#ifndef desk_sm_h
#define desk_sm_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef sm_h
#include <libs/graphicInterface/oldMonitor/include/mon/sm.h>
#endif

#include <libs/graphicInterface/oldMonitor/include/mon/manager.h>

struct amgrinst;
typedef struct amgrinst aMgrInst;

typedef FUNCTION_POINTER(void, RunAgainFunc,(Generic));
typedef FUNCTION_POINTER(void,AsyncRegFunc,(short,Generic,Boolean,Boolean));
typedef FUNCTION_POINTER(void,AsyncUnregFunc,(short,Generic));
typedef FUNCTION_POINTER(void,ChildRegFunc,(int,Generic,Boolean));
typedef FUNCTION_POINTER(void,ChildUnregFunc,(int,Generic));

/**************************** EVENT CALLBACKS ***************************/

EXTERN(void, sm_desk_register_fd,(short fd, aMgrInst* mi, Boolean buffer,
                                  Boolean urgent)); 
                                       /* register a file descriptor	*/
/* Takes four parameters:  (int fd) the file descriptor to register,	*/
/* (Generic manager_id) the manager id of who wants the event, 		*/
/* (Boolean should_read) true if input should be read, and (Boolean	*/
/* urgent) true if input should not be held.				*/

EXTERN(void, sm_desk_unregister_fd,(short fd, aMgrInst *mi));
                                     /* unregister a file descriptor    */
/* Takes two parameters:  (int fd) the file descriptor to unregister,	*/
/* and (Generic manager_id) the manager id of who wants the event.	*/

EXTERN(void, sm_desk_register_process,(int pid, aMgrInst *mi, Boolean ungent));
                                            /* register a process	*/
/* Takes three parameters:  (int pid) the process id to be registered,	*/
/* (Generic manager_id) the manager id of who wants the event, and	*/
/* (Boolean urgent) true if input should not be held.			*/

EXTERN(void, sm_desk_unregister_process,(int pid, aMgrInst *mi));
                                           /* unregister a process	*/
/* Takes two parameters:  (int pid) the process id to be unregistered	*/
/* and (Generic manager_id) the manager id of who wants the event.	*/

EXTERN(void, sm_desk_register_option,(Generic id, char *name, 
                                      char *help, aMgrInst *mi));
                                           /* register an option	*/
/* Takes four parameters:  (Generic id) the id associated with the	*/
/* event, (char *name) the name of the option, (char *help) the help	*/
/* associated with the option, and (Generic manager_id) the manager id	*/
/* of who wants the event.						*/

EXTERN(void, sm_desk_unregister_option,(Generic id, aMgrInst *mi));
                                         /* unregister an option	*/
/* Takes two parameters:  (Generic id) the id associated with the event	*/
/* (Generic manager_id) the manager id of who wants the event.		*/

EXTERN(void, sm_desk_give_event,(aMgrInst *mi));/* give an event to a mgr*/
/* Takes one parameter:  (Generic manager_id) the id of the manager who	*/
/* wants an event.  If no "real" event is possible, a null event will	*/
/* be sent.								*/

/************************** WINDOW CALLBACKS ****************************/

EXTERN(Window*, sm_desk_win_create,(aMgrInst *mi, Generic info, short
                                    title_front, Boolean resizeable));  
                                           /* create a window		*/
/* Takes four parameters:  (Generic manager_id) the id of the manager	*/
/* creating the window, (Generic info) the information parameter passed	*/
/* to the manager's window tiling call, (short font) the font to use for*/
/* the title bar, and (Boolean resizable) DSM_WIN_RESIZE if the window	*/
/* is resizable and DSM_WIN_NO_RESIZE otherwise.			*/
#define		DSM_WIN_RESIZE	  true	/* the window is resizable	*/
#define		DSM_WIN_NO_RESIZE false	/* the window is not resizable	*/

EXTERN(void, sm_desk_win_modify,(Window *w, Generic info));  
                                    /* modify an existing window	*/
/* Takes two parameters:  (Window *w) the window to retile and (Generic	*/
/* info) the tiling information to be passed to the manager's window	*/
/* tiling call.  The window is retiled accordingly.			*/

EXTERN(short, sm_desk_title_width,(char *title, short font)); 
                                        /* get the width of a title	*/
/* Takes two parameters:  (char *title) the proposed title and		*/
/* (short font) the font to assume.  Returns the pixel width necessary	*/
/* to completely display the title.					*/

EXTERN(void, sm_desk_win_title,(Window *w, char *title)); 
                                   /* (re)title a window		*/
/* Takes two parameters:  (Window *w) the window to retitle and		*/
/* (char *title) the new title to use.  The title font is given by the	*/
/* sm_desk_win_create() call.						*/

EXTERN(void, sm_desk_win_destroy,(Window *w));/* destroy a window	*/
/* Takes one parameter:	 (Window *w) the window to destroy.  The window */
/* is destroyed and removed from display.  This call will hide a showing*/
/* window.								*/

EXTERN(void, sm_desk_win_show,(Window *w));   /* make a window visible	*/
/* Takes one parameter:	 (Window *w) the window to show.  Will show a	*/
/* hidden window in its current position and depth in the window list.	*/
/* If the window is showing, nothing is done.				*/

EXTERN(void, sm_desk_win_hide,(Window *w));  /* make a window invisible	*/
/* Takes one parameter:  (Window *w) the window to hide.  Will hide a	*/
/* showing window and leave it in its current position and depth in the	*/
/* window list.  If the window is hidden, nothing is done.		*/

EXTERN(void, sm_desk_win_top,(Window *w)); /* bring a window to the top	*/
/* Takes one parameter:  (Window *w) the window of interest.  The window*/
/* is shown and brought to the top of the window list and the display is*/
/* updated accordingly.							*/

EXTERN(void, sm_desk_circulate,(Pane *p));/* circulate the desk		*/
/* Takes one parameter: (Pane *p) the desk pane and circulates the	*/
/* windows on that Pane.						*/

EXTERN(Boolean,	sm_desk_can_circulate,(Pane *p));   /* can the desk be	*/
/* circulated? Takes one parameter: (Pane *p) and returns true if so.	*/

EXTERN(Generic,	sm_desk_inst_id,(Pane *p));/* return the current active	*/
/* manager. Takes one parameter: (Pane *p) and returns the aMgrInst *	*/
/* of the currently active manager for the pane.			*/

EXTERN(void, sm_desk_tickle_cp,(int id, Generic owner));	
                                   /* fool a cp into thinking it	*/
/* received a SELECT from the root menu. Takes two parameters:		*/
/* (int id), the cp_index from the processors list of the cp to fool,	*/
/* (aMgrInst *owner), the manager for cp_mgr from sm_desk_inst_id...	*/

EXTERN(void, sm_desk_win_force,(Window *w, MouseCursor cur));	
                                     /* run a window until release	*/
/* Takes two parameters:  (Window *w) the window which grabs control of	*/
/* all mouse and character input and (MouseCursor c) the mouse cursor	*/
/* in the pane until the window is released.  Mouse and character input	*/
/* is released upon the destruction of the window or a call to		*/
/* sm_desk_win_release().  As a result of this call, the window will be	*/
/* made showing and will become the top window in the window list.	*/

EXTERN(void, sm_desk_win_release,(Window *w));/* release a forced window*/
/* Takes one parameter:  (Window *w) the window to be released.  The	*/
/* window is not hidden, destroyed, moved, or shuffled as a result of	*/
/* this call.								*/

EXTERN(Boolean,	sm_desk_owns_pane,(Pane *p));/* true if does desk own a pane */
/* Takes one parameter:  (Pane *p) a pane within a window in the desk	*/
/* pane.  Returns true if the pane is owned by the desk pane.		*/


/************************ SCREEN MODULE CALLBACKS ***********************/

EXTERN(short, sm_desk_get_index,(void));/* get the index of the desk sm */
/* Takes no parameters.  Returns the installed screen module index	*/
/* number for the desk screen module.					*/

EXTERN(void,sm_desk_initialize,(Pane*, Generic, RunAgainFunc,AsyncRegFunc,
				AsyncUnregFunc,ChildRegFunc,ChildUnregFunc));	/* initialize the desk manager	*/
/* Takes seven parameters:  (Pane *p) the desk pane, (Generic id) the 	*/
/* id to use in the following calls, (PFV run_me) a one parameter call	*/
/* which runs the desk again, (PFV register_fd) a three parameter call	*/
/* which registers a file descriptor, (PFV unregister_fd) a two		*/
/* parameter call which unregisters a file descriptor, (PFV		*/
/* register_child, unregister_child) two parameter calls which register	*/
/* and unregister child processes.					*/

EXTERN(Generic,	sm_desk_use_manager,(Pane *,aManager*));	/* install a new manager	*/
/* Takes two parameters:  (Pane *p) the desk manager pane and (aManager	*/
/* *mgr) a pointer to the manager structure routines to install.	*/
/* Returns a handle to the manager instance which may be used in manager*/
/* callbacks.								*/

typedef FUNCTION_POINTER(void,RedrawFunc, (void));
typedef FUNCTION_POINTER(void,QuitFunc, (void));

typedef FUNCTION_POINTER(int,OptsProcessFunc,(int,char**));
typedef FUNCTION_POINTER(int,RootStartupFunc,(int,char**));

EXTERN(void, resixeRoot, (Point size));

EXTERN(void, quit_environment, (void));

EXTERN(void, redrawRottRectList, (RectList rl));

EXTERN(int, runRoot, (int argc, char **argv, OptsProcessFunc opts_process, 
		      RootStartupFunc startup_func));

EXTERN(void, root_load_funcs, (RedrawFunc *redrawptr, QuitFunc *shutdownptr));

EXTERN(Boolean, root_can_circulate, (void));

EXTERN(void, root_circulate, (void));

EXTERN(void, root_tickle_tool_cp, (int id));

#endif
