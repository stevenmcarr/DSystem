/* $Id: iflex.C,v 1.1 1997/06/25 14:59:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <stdio.h>
#include <libs/support/misc/general.h>
#include <libs/support/memMgmt/mem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <libs/support/strings/rn_string.h>
#include <string.h>
#include <libs/graphicInterface/oldMonitor/monitor/sms/ted_sm/iflex.h>
/*#include <undo.h>*/
/*#include <misc/xstack.h>*/

/* a local global */
char *sm_ted_undo_null_string;

/*
 * All of the routines below call the corresponding flex routines.
 * The insert delete routines record the necessary information for 
 * undoing the action, and the stack structure is passed in the first
 * parameter for these routines.
 */

/*
 * iflex_delete (h, u, f, start, n) -   delete n elements beginning at start.
 */
void iflex_delete (Stack h, Undo *u, Flex *f, int start, int n)
{
	u->amount   = n;	
	u->location = start;
	u->buffer   = flex_get_buffer (f, start, n, (char *) 0);
	stack_push  (h, (Generic*)u);
	
	(void) flex_delete (f, start, n);
}

/*
 * iflex_delete_buffer (h, u, f, start, n, buf) delete n elements at start to buf
 *  	if buf == 0, alloc buf. Return it regardless. (Is this flex_delete?)
 */
char *iflex_delete_buffer (Stack h, Undo *u, Flex *f, int start, int n, char *buf)
{
	u->amount   = n;	
	u->location = start;
	u->buffer   = flex_get_buffer (f, start, n, (char *) 0);
	stack_push  (h, (Generic*)u);
	
	return (flex_delete_buffer (f, start, n, buf));
}

/*
 * iflex_delete_one (h, u, f, start, *buf) - delete one element at start into buf
 *	If buf = 0 allocate it.
 */
char *iflex_delete_one (Stack h, Undo *u, Flex *f, int start, char *buf)
{
	u->amount   = 1;	
	u->location = start;
	u->buffer   = flex_get_one (f, start);
	stack_push  (h, (Generic*)u);
	
	return (flex_delete_one (f, start, buf));
}

/*
 * iflex_insert (h, u, f, start, n) -insert n undefined elements at start
 */
/*ARGSUSED*/
void iflex_insert (Stack h, Undo *u, Flex *f, int start, int n)
{
	flex_insert (f, start, n);
}

/*
 * iflex_insert_buffer (h, u, f, start, n, buf) - insert n elements at start from
 * 	buf
 */
void iflex_insert_buffer (Stack h, Undo *u, Flex *f, int start, int n, char *buf)
{
	u->amount   = -n;	
	u->location =  start;
	u->buffer   =  sm_ted_undo_null_string;
	stack_push (h, (Generic*)u);

	flex_insert_buffer (f, start, n, buf);
}

/*
 * iflex_insert_one (h, u, f, start, elem) - insert elem at start
 */
void iflex_insert_one (Stack h, Undo *u, Flex *f, int start, char *elem)
{
	u->amount   = -1;	
	u->location =  start;
	u->buffer   =  sm_ted_undo_null_string;
	stack_push (h, (Generic*)u);
	
	flex_insert_one (f, start, elem);
} 

/*
 * iflex_insert_file (h, u, f, start, fname) -  insert file, fname at start.
 */
int iflex_insert_file (Stack h, Undo *u, Flex *f, int start, char *fname)
{
	int fsize;

	fsize = flex_insert_file (f, start, fname);
	if (fsize != 0)
	{
		if (u->adot = HIDE)
			u->adot = u->dot + fsize;
		u->amount   = -fsize;	
		u->location =  start;
		u->buffer   =  sm_ted_undo_null_string;
		stack_push (h, (Generic*)u);
	}
	return (fsize);
}

/*
 * iflex_set_buffer (h, u, f, start, n, buf) - set n elements at start from buf
 */
void iflex_set_buffer (Stack h, Undo *u, Flex *f, int start, int n, char *buf)
{
	/* Here we remember the old stuff (what is currently being changed) */
	u->amount   = n;	
	u->location = start;
	u->buffer   = flex_get_buffer (f, start, n, (char *) 0);
	stack_push  (h, (Generic*)u);
	
	/* Here we delete the new stuff (what is currently being written) */
	u->dot	   = HIDE;
	u->amount   = -n;	
	u->location = start + n;
	u->buffer   = sm_ted_undo_null_string;
	stack_push  (h, (Generic*)u);

	flex_set_buffer (f, start, n, buf);
}

/*
 * iflex_set_one (h, u, f, offset, elem) - set element at offset to elem
 */
void iflex_set_one (Stack h, Undo *u, Flex *f, int offset, char *elem)
{

	/* Here we remember the old char */
	u->amount   =  1;	
	u->location =  offset;
	u->buffer   =  flex_get_buffer (f, offset, 1, (char *) 0);
	stack_push (h, (Generic*)u);
	
	/* Here we delete the new char */
	u->dot	   = HIDE;
	u->amount   = -1;	
	u->location =  offset;
	u->buffer   =  sm_ted_undo_null_string;
	stack_push (h, (Generic*)u);

	flex_set_one (f, offset, elem);
}

