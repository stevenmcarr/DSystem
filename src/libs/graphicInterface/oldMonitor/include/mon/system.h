/* $Id: system.h,v 1.6 1999/03/31 21:54:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 		        system.h			*/
		/*		Replacements to system calls.		*/
		/* 							*/
		/********************************************************/

#ifndef system_h
#define system_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifdef SOLARIS
   /* forward declare to satisfy Solaris compiler gripe.  This will */
   /* need to be changed to comply with SVR4 standards.             */
union wait;

EXTERN(void, status_wait, (int pid, union wait *status_ptr));
#else
EXTERN(void, status_wait, (int pid, int *status_ptr));
#endif
/* replaces wait3 C call                                                                */
/* Takes two parameters (int pid) the process id to wait on and (union wait *sptr) the	*/
/* pointer to the union wait which will record the status.  The call waits for a change	*/
/* in the status of the child process and returns it in *sptr.  The call handles other	*/
/* incoming child signals properly.  Note:  it is strongly advisable to register the	*/
/* process pid *before* the process could possibly change status.			*/

#if (!(defined(__cplusplus)) && !(defined(c_plusplus)))

/* Overridden UNIX functions */

/* extern int system(); */                /* replaces 'system (3)' C call		*/
/* Takes one parameter (char *s) the string sent to a shell.  The return value is the	*/
/* return value of the shell.  This call is used in place of UNIX 'system' so that other*/
/* child signals will be handled properly.						*/

/* extern unsigned sleep(); */            /* replaces 'sleep (3)' C call		*/
/* Takes one parameter (unsigned sec) the number of seconds to sleep.  Returns the	*/
/* number of second left unslept.  This call is used in place of UNIX 'sleep' so that	*/
/* compute-bound event flushing works properly.						*/

#endif

EXTERN(void, beginComputeBound, (void));
/* Takes no parameters.  Signals the beginning of a compute-bound region.  No screen	*/
/* updating nor event interpretation may be done during a compute bound region.  The	*/
/* compute-bound region of code must be followed by a call to endComputeBound().	*/
/* Note:  compute bound regions may be nested.						*/

EXTERN(void, endComputeBound, (void));
/* Takes no parameters.  Signals the end of a compute-bound region.  See 		*/
/* beginComputeBound(), above.								*/

#endif
