/* $Id: notify.h,v 1.6 1997/03/11 14:33:18 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/* 									*/
/* 		     		  notify.h				*/
/* 		         process & i/o notification			*/
/* 									*/
/* Using the calls below will allow a non-command processer to receive	*/
/* notification of child process events and asynchonous i/o events.	*/
/* Command processors should use the corresponding calls in cp.h.	*/
/* 									*/
/************************************************************************/

#ifndef notify_h
#define notify_h

typedef FUNCTION_POINTER(void, notify_async_callback,
  (Generic owner, short fd, char *buf, short len));
/* The asynchronous io notifier takes four arguments:  (Generic owner)	*/
/* the owner handle, (short fd) the file descriptor that has input,	*/
/* (char *buf) the buffer of data, and (short len) the length of the	*/
/* buffer.  If read is set to false, above, buf and len are undefined	*/
/* on notification.							*/

EXTERN(void, notify_register_async_fd, (short fd, Generic owner,
 PFV handler, Boolean read, Boolean urgent));
/* register async fd		*/
/* Takes five parameters (short fd) the file descriptor to begin	*/
/* polling for asynchronous input, (Generic owner) the owner handle to	*/
/* give notification about that file descriptor,			*/
/* (notify_async_callback notifier) the notification function to call	*/
/* for notification, (Boolean read) which is true if the input is to be	*/
/* read and false if just notification is to be given, and		*/
/* (Boolean urgent) true if notification should not be held for the	*/
/* user interface.  All registered file descriptors must be		*/
/* unregistered.							*/

EXTERN(void, notify_unregister_async_fd, (short fd));
/* unregister async fd	*/
/* Takes one parameter (short fd) the file descriptor which is no	*/
/* longer of interest.  No more notification will be given based on the	*/
/* earlier registration.						*/


typedef FUNCTION_POINTER(void, notify_process_callback,
  (Generic owner, int pid, union wait *status));
/* The process notifier takes three arguments:  (Generic owner) the	*/
/* owner handle, (int pid) the process id of the notification, and	*/
/* (union wait *status) the status of the process.			*/

EXTERN(void, notify_register_process, (int pid, Generic owner,
 notify_process_callback notifier, Boolean urgent));
/* register a process		*/
/* Takes four parameters (int pid) the process id of interest, (Generic	*/
/* owner) the owner handle to give notification about a change in the	*/
/* status of the pid, (notify_process_callback notifier) the		*/
/* notification function to call for notification, and (Boolean urgent)	*/
/* true if notification should not be held for the user interface.  All	*/
/* registered processes must be unregistered.				*/

EXTERN(void, notify_unregister_process, (int pid));
/* unregister a process	*/
/* Takes one parameter (int pid) the process id which is no longer of	*/
/* interest.  Not more notifications will be given based on the earlier	*/
/* registration.							*/

#endif
