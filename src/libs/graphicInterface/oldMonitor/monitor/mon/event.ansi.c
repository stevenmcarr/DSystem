/* $Id: event.ansi.c,v 1.17 1999/06/23 13:40:01 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 			event.c				*/
		/* 							*/
		/* Handles window events, extra events, asynchronous io,*/
		/* and child processes.  Contains the main program.	*/
		/* 							*/
		/********************************************************/

#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#ifdef SOLARIS
#include <vfork.h>
#endif
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#ifdef LINUX
#include <sys/select.h>
#endif

#include <libs/support/misc/general.h>

#include <libs/graphicInterface/support/graphics/point.h>
#include <libs/support/memMgmt/mem.h>
#include <libs/graphicInterface/oldMonitor/monitor/keyboard/kb.h>
#include <libs/graphicInterface/oldMonitor/monitor/keyboard/keymap.h>

#include <libs/graphicInterface/oldMonitor/include/mon/event_codes.h>
#include <libs/graphicInterface/oldMonitor/include/mon/event.h>
#include <libs/graphicInterface/oldMonitor/include/mon/standalone.h>
#include <libs/graphicInterface/oldMonitor/include/mon/system.h>
#include <libs/graphicInterface/oldMonitor/include/mon/gfx.h>

#include <libs/graphicInterface/oldMonitor/monitor/mon/mach.h>

	/* LOCAL SPECIAL VARIABLES */

static	short		window_only = 0;	/* non-win. events should be suppressed	*/
static	short		urgent_only = 0;	/* supress non-urgent events		*/
static	Boolean		redo_event = false;	/* redo last event flag (ungetEvent)	*/
static	Boolean		give_null_event = false;/* true if a null event should be given	*/
static	anEvent		saved_event;		/* the saved mon_event (redo)		*/
static	Boolean		last_was_mouse = true;	/* true if last event was a mouse event	*/




	/* CHILD PROCESS INFORMAION */

struct	cr_node	{				/* CHILD REGISTRATION NODE (for 1-list)	*/
	int		pid;			/* the registered process id		*/
	Generic		owner;			/* the registered owner of the process	*/
	Boolean		urgent;			/* input should not be held		*/
#ifdef SOLARIS
	union	wait	status;			/* the most recient status of process	*/
#else
       int status;
#endif
	struct	cr_node	*next;			/* the next entry in the list		*/
	Boolean		ready;			/* there is a waiting child here	*/
		};
static	struct	cr_node	*cr_list;		/* the child registration list head	*/
static	short		boring_childs;		/* the number of non-urgent child procs	*/
static	short		urgent_childs;		/* the number of urgent child proc.	*/

typedef FUNCTION_POINTER(void,SignalHandler,(int));


	/* ASYNCHRONOUS IO INFORMATION */

#ifndef LINUX
typedef	struct	fd_set	SelectMask;		/* the select mask type			*/
#else
typedef	fd_set	SelectMask;		/* the select mask type			*/
#endif
struct	file		{			/* FILE descriptor			*/
	Generic		owner;			/* the owner of the file		*/
	Boolean		buffer;			/* true if the file is buffered		*/
	Boolean		urgent;			/* do not hold input			*/
			};
static	struct	file	*file;			/* file array (allocated)		*/
static	int		num_files;		/* the number of allowed files		*/
static	SelectMask	file_mask;		/* input mask for registered files.	*/
static	SelectMask	urgent_mask;		/* input mask for registered urg. files	*/


	/* SLEEP/ALARM INFORMATION */

static	int		ignore_count = 0;	/* count of ignore nesting		*/
static	struct	timeval	granularity = { 1, 0 };	/* how often we check the timer		*/
static	struct	timeval	sleep_left = { 0, 0 };	/* the time left to sleep		*/
static	MouseCursor	save_cursor;		/* saved cursor during compute		*/


	/* KEYBOARD INFORMATION */

#define	SELF_INSERT	-2			/* binding to self inserting sequences	*/
static	Keymap		*keyboard_km = NULL_KEYMAP;/* the keyboard keymapping		*/


	/* EVENT INFORMATION */

	anEvent		mon_event;		/* the current event			*/
static	SelectMask	select_mask;		/* the select file mask			*/



/* Initialize for child process events.							*/
void
startChildProcessEvents(void)
{
	cr_list       = (struct cr_node *) 0;
	boring_childs = 0;
	urgent_childs = 0;
}


/* Return true if there is a waiting process event.					*/
static Boolean
readyChildProcessEvent(void)
{
#ifdef SOLARIS
	status_wait(0, (union wait *) 0);
#else
	status_wait(0, (int *) 0);
#endif
	return BOOL(urgent_childs + ((urgent_only) ? 0 : boring_childs));
}


/* Return a waiting child process event into mon_event.					*/
static void
getChildProcessEvent(void)
{
register struct	cr_node	*current;		/* current entry in registration list	*/

	for (current = cr_list; current; current = current->next)
	{/* check each entry in the list for one that is ready */
		if (current->ready && !(urgent_only && !current->urgent))
		{/* current is a ready process */
			mon_event.type = EVENT_SIGCHLD;
			mon_event.to   = current->owner;
			mon_event.from = current->pid;
			mon_event.info = Origin;
			mon_event.msg  = (Generic) &current->status;
			/* mon_event.loc mon_event.offset do not change */
			current->ready = false;
			if (current->urgent)
				urgent_childs--;
			else
				boring_childs--;
			return;
		}
	}

	die_with_message("getChildProcessEvent() no event waiting.");
}


/* Finalize child process list.								*/
static void
stopChildProcessEvents(void)
{
	if (cr_list)
	{/* registered children remain */
		die_with_message("Child process registrations remain.");
	}
}


/* Add the child process registration of "owner" to process "pid".			*/
void
registerChildProcess(int pid, Generic owner, Boolean urgent)
{
register struct	cr_node	*New;			/* the new head of registration list	*/

	New = (struct cr_node *) get_mem(sizeof(struct cr_node), "registerChildProcess():  child process registration");
	New->pid    = pid;
	New->owner  = owner;
	New->urgent = urgent;
	New->next   = cr_list;
	New->ready  = false;
	cr_list     = New;
}


/* Remove a child process registration for "pid".					*/
void
unregisterChildProcess(int pid, Generic owner)
{
register struct	cr_node	**crlp;			/* child registration list pointer ptr	*/
register struct	cr_node	*current;		/* the list entry being deleted		*/

	for (crlp = &cr_list; *crlp; crlp = &(*crlp)->next)
	{/* walk down the child registration list */
		current = *crlp;
		if ((current->pid == pid) && (current->owner == owner))
		{/* current is the registration node--delete it */
			if (current->ready)
			{/* there was an event here--stop it */
				if (current->urgent)
					urgent_childs--;
				else
					boring_childs--;
			}
			*crlp = current->next;
			free_mem((void*) current);
			return;
		}
	}

	die_with_message("Could not unregister child process.");
}


/* Replaces "system" with something that knows about the other child pids.		*/
#ifndef LINUX
int system(const char* s)
#else
int system(char* s)
#endif
{
  register int pid;			/* the (shard) process id		*/
  register int omask;			/* old signal mask			*/
#ifdef SOLARIS
  union	wait   status;			/* the return status			*/
#else
  int status;
#endif

  pid = vfork();
  if (pid == -1)
    {/* there was a vfork error */
      return 127;
    }
  else if (pid == 0)
    {/* child process -- start the shell */
      execl("/bin/sh", "sh", "-c", s, 0);
      _exit(127);
      /*NOTREACHED*/
    }
  else
    {/* parent process -- wait for termination */
      omask = sigblock(sigmask(SIGINT) | sigmask(SIGQUIT));
#ifdef SOLARIS
      status_wait(pid, &status);
#else
      status_wait(pid, &status);
#endif
      sigsetmask(omask);
#ifdef _AIX
      return (WIFEXITED(status)) ? status.w_retcode : status.w_termsig;
#elif defined(LINUX) || defined(OSF1)
      if (WIFEXITED(status)) 
	return WEXITSTATUS(status); 
      else
	return WTERMSIG(status);
#else
      return (WIFEXITED(status.w_status)) ? status.w_retcode : status.w_termsig;
#endif
    }
}


/* Replaces "wait3" with something that knows about other child pids.			*/
void
#ifdef SOLARIS
status_wait(int pid, union wait* status_ptr)
#else
status_wait(int pid, int* status_ptr)
#endif
{
register struct	cr_node	*current;		/* current entry in registration list	*/
register int		new_pid;		/* the new ready child process id	*/
#ifdef SOLARIS
union wait		status;			/* the return status			*/
#else
int		status;			/* the return status			*/
#endif

	if (pid)
	{/* check the existing registration list & arrive list for this pid */
		for (current = cr_list; current; current = current->next)
		{/* walk down the child registration list to see if the child isn't already ready */
			if (current->pid == pid)
			{/* there is a child registration */
				if (current->ready)
				{/* remove this ready event and give this status back */
					current->ready = false;
					if (current->urgent)
						urgent_childs--;
					else
						boring_childs--;
					#ifdef SOLARIS
					*status_ptr = current->status;
					#else
					*status_ptr = current->status;
					#endif
					return;
				}
			}
		}
	}

#ifndef LINUX
	while ((new_pid = wait3(&status, (pid) ? WUNTRACED : WUNTRACED|WNOHANG, (struct rusage *) 0)) > 0 && new_pid != pid)
#else
	while ((new_pid = wait3(&status, (pid) ? WUNTRACED : WUNTRACED|WNOHANG, (struct rusage *) 0)) > 0 && new_pid != pid)
#endif
	{/* process a child that we are not interested in */
		for (current = cr_list; current; current = current->next)
		{/* check each entry in the list for new_pid match */
			if (current->pid == new_pid)
			{/* current has a new child event */
				current->status = status;
				if (NOT(current->ready))
				{/* a new child status has arrived */
					current->ready = true;
					if (current->urgent)
						urgent_childs++;
					else
						boring_childs++;
				}
				break;
			}
		}
	}

	if (new_pid > 0)
	{/* return the status of the interesting process */
		#ifdef SOLARIS
		*status_ptr = status;
		#else
		*status_ptr = status;
		#endif
	}
}


/* Initialize for asynchronous io events.						*/
static void
startAsyncIOEvents(void)
{
register short		i;			/* index into the file registration tbl.*/

	num_files = getdtablesize();
	file = (struct file *) get_mem(sizeof(struct file) * num_files, "event.c: file table");
	for (i = 0; i < num_files; i++)
	{/* clean out the owner table */
		file[i].owner = UNUSED;
	}
	FD_ZERO(&file_mask);
	FD_ZERO(&urgent_mask);
}


/* Return true if there is an asynchronous io event waiting.				*/
static Boolean
readyAsyncIOEvent(void)
{
static	struct	timeval	no_time = { 0, 0 };	/* timeout val for select		*/
SelectMask		inmask;			/* the file input mask			*/

	inmask = (urgent_only) ? urgent_mask : file_mask;
	return (BOOL(select(num_files, &inmask, (SelectMask *) 0, (SelectMask *) 0, &no_time) > 0));
}


/* Get a random waiting asynchronous io event into mon_event.				*/
static void
getAsyncIOEvent(void)
{
#define BUFF_SIZE	128			/* maximum characters per message	*/
static	char		io_buf[BUFF_SIZE];	/* the character buffer			*/
SelectMask		inmask;			/* the current available file mask	*/
static	short		fd = 0;			/* current file descriptor of interest	*/

	inmask = (urgent_only) ? urgent_mask : file_mask;
	(void) select(num_files, &inmask, (SelectMask *) 0, (SelectMask *) 0, (struct timeval *) 0);
	do
	{/* rotate fd at least once until a waiting file input is found */
		fd = (fd + 1) % num_files;
	} while (!FD_ISSET(fd, &inmask));

	mon_event.type   = EVENT_IO;
	mon_event.to     = file[fd].owner;
	mon_event.from   = fd;
	if (file[fd].buffer)
	{/* buffered--do the read */
		mon_event.info.x = read(fd, io_buf, BUFF_SIZE);
		mon_event.info.y = UNUSED;
		mon_event.msg    = (Generic) io_buf;
		if (mon_event.info.x <= 0)
		{/* a closed or dead file -- remove from polling list */
			if (file[fd].urgent)
				FD_CLR(fd, &urgent_mask);
			FD_CLR(fd, &file_mask);
			FD_CLR(fd, &select_mask);
			mon_event.info.x = 0;
		}
	}
	else
	{/* unbuffered--just notify */
		mon_event.info.x = UNUSED;
		mon_event.info.y = UNUSED;
		mon_event.msg    = (Generic) UNUSED;
	}
}


/* Finalize asynchronous io events.							*/
static void
stopAsyncIOEvents(void)
{
register short		i;			/* index into the file registration tbl.*/

	for (i = 0; i < num_files; i++)
	{/* check each entry for a registration */
		if (file[i].owner != UNUSED)
		{/* a registration was not canceled */
			die_with_message("Asynchronous file registrations remain.");
		}
	}
	free_mem((void*) file);
}


/* Add the file descriptor to the list of those being polled in the name of the owner.	*/
void
registerAsyncIO(short fd, Generic owner, Boolean buffer, Boolean urgent)
{
	if ((fcntl(fd, F_GETFD, 0) == -1) || (file[fd].owner != UNUSED))
	{/* this is a mistaken request */
		die_with_message("Could not register asynchronous file.");
	}
	else
	{/* the request is valid */
		file[fd].owner   = owner;
		file[fd].buffer  = buffer;
		file[fd].urgent  = urgent;
		if (urgent)
			FD_SET(fd, &urgent_mask);
		FD_SET(fd, &file_mask);
		FD_SET(fd, &select_mask);
	}
}


/* Delete the file descriptor from the list of those being polled.			*/
void
unregisterAsyncIO(short fd, Generic owner)
{
	if ((fd < 0) || (fd > num_files) || (file[fd].owner != owner))
	{/* this is a mistaken request */
		die_with_message("Could not unregister asynchronous file.");
	}
	else
	{/* the request is valid */
		file[fd].owner   = UNUSED;
		if (file[fd].urgent)
			FD_CLR(fd, &urgent_mask);
		FD_CLR(fd, &file_mask);
		FD_CLR(fd, &select_mask);
	}
}


/* Turn on the alarm to go off every "granularity".					*/
static void
setAlarm(void)
{
struct	itimerval	time;			/* the time to put on the alarm		*/

	time.it_interval = granularity;
	time.it_value    = granularity;
	(void) setitimer(ITIMER_REAL, &time, (struct itimerval *)0);
}


/* Turn off the alarm.									*/
static void
clearAlarm(void)
{
struct	itimerval	time;			/* the time to put on the alarm		*/

	timerclear(&time.it_interval);
	timerclear(&time.it_value);
	(void) setitimer(ITIMER_REAL, &time, (struct itimerval *)0);
}


/* Handle an alarm signal going off.							*/
static int
handleAlarm(int dummy)
{
	if (timerisset(&sleep_left))
	{/* decrement the sleep time */
		if (timercmp(&sleep_left, &granularity, <))
		{/* wake up! */
			timerclear(&sleep_left);
		}
		else
		{/* decrement the alarm by the granularity */
			sleep_left.tv_usec -= granularity.tv_usec;
			sleep_left.tv_sec  -= granularity.tv_sec;
			if (sleep_left.tv_usec < 0)
			{/* handle the carry */
				sleep_left.tv_usec += 1000000;
				sleep_left.tv_sec  -= 1;
			}
		}
	}
	if (ignore_count)
	{/* flush the input event queue */
		flushEvents();
	}

	if ((ignore_count == 0) && !timerisset(&sleep_left))
	{/* shut off the alarm--not being used */
		clearAlarm();
	}

	return 0;
}


/* Sleep for a specified number of seconds.  Handle the case of other installed alarm.	*/
unsigned int sleep(unsigned int sec)
{
int			savemask;		/* the saved signal mask		*/

	if (sec == 0)
		return 0;

	savemask = sigblock(sigmask(SIGALRM));
	sleep_left.tv_sec  = sec;
	sleep_left.tv_usec = 0;
	setAlarm();
	while (timerisset(&sleep_left))
	{/* wait for another alarm */
		(void) sigpause(savemask &~ sigmask(SIGALRM));
	}
	(void) sigsetmask(savemask);
	return 0;  /* we'll always sleep the full amount */
}


/* Begin a compute bound region.							*/
void
beginComputeBound(void)
{
	if (ignore_count++ == 0)
	{/* set up to ignore events */
		save_cursor = CURSOR(wait_cursor);
		flushEvents();
		if (!timerisset(&sleep_left))
		{/* turn on the alarm */
			setAlarm();
		}
	}
}


/* End a compute bound region.								*/
void
endComputeBound(void)
{
	if (--ignore_count == 0)
	{/* set up to quit ignoring events */
		if (!timerisset(&sleep_left))
		{/* turn off the alarm */
			clearAlarm();
		}
		flushEvents();
		(void) CURSOR(save_cursor);
	}
}


/* Start the events from the keyboard.							*/
static void
startKeyboardEvents(void)
{
char			*ksq;			/* the current key sequence		*/
short			i;			/* the current binding index		*/

	keyboard_km = keymap_create((Generic) UNUSED);
	keymap_set_push(keyboard_km, true);
	keymap_bind_range(
		keyboard_km,
		SELF_INSERT,
		makeKbString("", "startKeyboardEvents()"),
		KB_first_ascii,
		KB_last_ascii
	);
	for (i = 0; i < KB_num_right; i++)
	{/* bind the right keypad */
		ksq = inputFromKbChar(KB_right(i));
		if (ksq[0])
		{/* bind a valid input sequence */
			keymap_bind_KbString(
				keyboard_km,
				(Generic) KB_right(i),
				makeKbString(ksq, "startKeyboardEvents()")
			);
		}
	}
	for (i = 0; i < KB_num_left; i++)
	{/* bind the left keypad */
		ksq = inputFromKbChar(KB_left(i));
		if (ksq[0])
		{/* bind a valid input sequence */
			keymap_bind_KbString(
				keyboard_km,
				(Generic) KB_left(i),
				makeKbString(ksq, "startKeyboardEvents()")
			);
		}
	}
	for (i = 0; i < KB_num_top; i++)
	{/* bind the top keypad */
		ksq = inputFromKbChar(KB_top(i));
		if (ksq[0])
		{/* bind a valid input sequence */
			keymap_bind_KbString(
				keyboard_km,
				(Generic) KB_top(i),
				makeKbString(ksq, "startKeyboardEvents()")
			);
		}
	}
	for (i = 0; i < KB_num_arrow; i++)
	{/* bind the arrow keys */
		ksq = inputFromKbChar(KB_arrow(i));
		if (ksq[0])
		{/* bind a valid input sequence */
			keymap_bind_KbString(
				keyboard_km,
				(Generic) KB_arrow(i),
				makeKbString(ksq, "startKeyboardEvents()")
			);
		}
	}
}


/* Check to see if a keyboard event is ready.						*/
static Boolean
readyKeyboardEvent(void)
{
	return (keymap_mapping_complete(keyboard_km));
}


/* Get a ready keyboard event.								*/
static void
getKeyboardEvent(void)
{
static	char		mapped_buffer[10];	/* the buffer of mapped input chars	*/
Keymap_report		rep;			/* the keymap report structure		*/

	rep = keymap_report_all(keyboard_km);
	convertKbString(rep.seq, mapped_buffer);
	mon_event.type    = MOUSE_KEYBOARD;					/* keyboard event          */
	mon_event.info.x  = (rep.info == SELF_INSERT) ? mapped_buffer[0] : rep.info;	/* the return KbChar	   */
	mon_event.info.y  = rep.seq.num_kc;					/* length of msg field     */
	mon_event.msg     = (Generic) mapped_buffer;				/* what got us this KbChar */
}


/* Stop the flow of keyboard events.							*/
static void
stopKeyboardEvents(void)
{
	keymap_destroy(keyboard_km);
	keyboard_km = 0;
}


/* Process a new keyboard character.							*/
static void
newKeyboardChar(char c)
{
	if (keyboard_km)
	{/* we have a mapping to use */
		keymap_enqueue_KbChar(keyboard_km, toKbChar(c));
	}
}


/* Return true if there is a real event waiting.					*/
Boolean
readyEvent(void)
{
	return (
		BOOL(
			redo_event ||
			readyScreenEvent() ||
			readyKeyboardEvent() ||
			(
				NOT(window_only) &&
				(
					readyChildProcessEvent() ||
					readyAsyncIOEvent()
				)
			)
		)
	);
}


/* Get a new input event in the global variable mon_event.				*/
void
getEvent(void)
{
static	struct timeval	time_limit = { 0, 1000};/* amount of time to wait for main sel.	*/
						/* Note: this could be indefinite if a	*/
						/* status change in a traced child broke*/
						/* us out of a select.			*/
SelectMask		inmask;			/* mask of file descriptors we want	*/
int			fd;			/* temporary file descriptor		*/
static	short		button_down = 0;	/* the button number depressed		*/
static	Boolean		out_of_window = true;	/* true if events are out of the window	*/

	if (redo_event)
	{/* let them have the old event */
		saved_event.loc    = subPoint(transPoint(saved_event.loc, saved_event.offset), mon_event.offset);
		saved_event.offset = mon_event.offset;
		mon_event = saved_event;
		redo_event = false;
		return;
	}
	else if (window_only)
	{/* give only mouse events */
		if (readyScreenEvent())
		{/* give the current waiting screen event */
			getScreenEvent();
		}
		else if (readyKeyboardEvent())
		{/* give the current waiting keyboard event */
			getKeyboardEvent();
		}
		else if (NOT(last_was_mouse))
		{/* give a bogus mouse event */
			if (out_of_window)
			{/* the mouse is out of the window--give a bogus exit event */
				mon_event.type = MOUSE_EXIT;
				mon_event.info = Origin;
			}
			else
			{/* the mouse is in the window--give a bogus move event */
				mon_event.type = (button_down) ? MOUSE_DRAG : MOUSE_MOVE;
				mon_event.info = makePoint(button_down, 0);
			}
		}
		else
		{/* wait for a new screen event */
			while (NOT(readyScreenEvent() || readyKeyboardEvent()))
			{/* wait indefinitely for a change in the signal/io status */
				FD_ZERO(&inmask);
				for (fd = 0; fd < num_files; fd++)
				{/* create inmask so that inmask = select_mask & ~file_mask */
					if (FD_ISSET(fd, &select_mask) && !FD_ISSET(fd, &file_mask))
						FD_SET(fd, &inmask);
				}
				(void) select(num_files, &inmask, (SelectMask *) 0, (SelectMask *) 0, (struct timeval *) 0);
			}
			
			if (readyScreenEvent())
			{/* give the new screen event */
				getScreenEvent();
			}
			else
			{/* give the new keyboard event */
				getKeyboardEvent();
			}
		}
	}
	else
	{/* we need a new event */
		while (true)
		{/* slow busy-wait for an event--We cannot wait indefinitely because we */
		 /* can get traced child events without being thrown out of the select. */
			if (readyScreenEvent())
			{/* give a mouse event */
				getScreenEvent();
				break;
			}
			else if (readyKeyboardEvent())
			{/* give a keyboard event */
				getKeyboardEvent();
				break;
			}
			else if (readyChildProcessEvent())
			{/* give a child event */
				getChildProcessEvent();
				break;
			}
			else if (readyAsyncIOEvent())
			{/* give an io event */
				getAsyncIOEvent();
				break;
			}
			else if (NOT(urgent_only) && give_null_event)
			{/* give a null event */
				mon_event.type = EVENT_NULL;
				give_null_event = false;
				break;
			}
			else if (NOT(last_was_mouse))
			{/* give a filler mouse event */
				if (out_of_window)
				{/* the mouse is out of the window--give a bogus exit event */
					mon_event.type = MOUSE_EXIT;
					mon_event.info = Origin;
				}
				else
				{/* the mouse is in the window--give a bogus move event */
					mon_event.type = (button_down) ? MOUSE_DRAG : MOUSE_MOVE;
					mon_event.info = makePoint(button_down, 0);
				}
				break;
			}
			inmask = select_mask;
			if (cr_list)
			{/* there are children, do a slow busy wait as described above */
				(void) select(num_files, &inmask, (SelectMask *) 0, (SelectMask *) 0, &time_limit);
			}
			else
			{/* there are no children--block until we get a file descriptor */
				(void) select(num_files, &inmask, (SelectMask *) 0, (SelectMask *) 0, (struct timeval *) 0);
			}
		}
	}

	/* adjust the out_of_window flag */
		if (out_of_window && (mon_event.type < MOUSE_EXIT))
		{/* we are no longer out of the window */
			out_of_window = false;
		}
		else if (NOT(out_of_window) && (mon_event.type == MOUSE_EXIT))
		{/* we are leaving the window */
			out_of_window = true;
		}

	/* adjust the button status */
		if ((button_down && (mon_event.type == MOUSE_EXIT)) || (mon_event.type == MOUSE_UP))
		{/* raise the button */
			button_down = 0;
			window_only--;
		}
		else if (mon_event.type == MOUSE_DOWN)
		{/* lower the button */
			button_down = mon_event.info.x;
			window_only++;
		}

	/* adjust the last_was flag */
		last_was_mouse = BOOL((mon_event.type < MOUSE_KEYBOARD) || (mon_event.type == MOUSE_EXIT));
}


/* Force the next getEvent() to return the same value.					*/
void
ungetEvent(void)
{
	redo_event  = true;
	saved_event = mon_event;
}


/* Flush all remaining human-events.  Note:  may be called asynchronously.		*/
void
flushEvents(void)
{
	(void) flushScreenEvents();	/* this may generate keyboard events */
	last_was_mouse = false;
}


/* Hold non-urgent events depending on the flag.					*/
void
holdNonUrgentEvents(Boolean flag)
{
	urgent_only += (flag) ? 1 : -1;
}


/* Set flag to give a null event (before blocking).					*/
void
giveNullEvent(void)
{
	give_null_event = true;
}


/* Start the event queues.								*/
void
startEvents(void)
{
	(void) signal(SIGALRM, (SignalHandler) handleAlarm);
	FD_ZERO(&select_mask);
	startScreenEvents(resizeRoot, redrawRootRectList, newKeyboardChar);
	startKeyboardEvents();
	startAsyncIOEvents();
	startChildProcessEvents();
}


/* Stop the event queues.								*/
void
stopEvents(void)
{
	signal(SIGALRM, SIG_DFL);
	stopChildProcessEvents();
	stopAsyncIOEvents();
	stopKeyboardEvents();
	stopScreenEvents();
}


/* Add a file descriptor to the select mask.						*/
void
add_select_mask(int fd)
{
	FD_SET(fd, &select_mask);
}



/* Remove a file descriptor from the select mask.					*/
void
remove_select_mask(int fd)
{
	FD_CLR(fd, &select_mask);
}
