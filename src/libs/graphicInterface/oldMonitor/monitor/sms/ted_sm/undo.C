/* $Id: undo.C,v 1.1 1997/06/25 14:59:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/graphicInterface/oldMonitor/monitor/sms/ted_sm/tedprivate.h>
#include <sys/types.h>
#include <sys/file.h>

#define FIRST 1

/* forward declarations */
/*Boolean sm_ted_continue();*/

/* a local global */
char *sm_ted_undo_null_string = "";

void sm_ted_dot_record (Pane *p)
{
	UNDO(p).dot      =  DOT(p);
	UNDO(p).adot	 =  HIDE;
	UNDO(p).location = -1;
	UNDO(p).amount   =  0;
	UNDO(p).buffer   =  sm_ted_undo_null_string;
	
	stack_push (HISTORY(p), (Generic*)&UNDO(p));
}

void sm_ted_event_continuous (Pane *p)
{
	UNDO(p).curvature = CONTINUOUS;
	sm_ted_dot_record (p);
}

void sm_ted_event_disjoint (Pane *p)
{
	UNDO(p).curvature = DISJOINT;
	sm_ted_dot_record (p);
}

/* Performs discrete undo actions upon the continued prompting from the user 
 */
void sm_ted_undo (Pane *p)
{
	Undo event;

	int stack_index;	/* top of stack at some point */
	int tos;		/* current top of stack */
	int depth;		/* depth to look into stack */
	int old_dot;		
	int curvature = DISJOINT;		
	int action = FIRST;
	
	/* top of stack when we start undoing ... of course every action has
	 * a reaction and when we execute an event we have to put its contra
	 * on top of the stack, so the top of stack, and the stack depth
	 * get deeper during an undo session.
	 * Where we want to look in the stack:  
	 * 	depth = tos - stack_index + depth + 1
	 *	stack_index = tos (before tos changes from our action)
	 * Loop while the user says 'yes' using a dialog, and while
	 * there is stuff on the stack. The first two things on the stack
	 * are the clear and put of the file, so we ignore them
	 */

	MODS(p)++;

	stack_index = stack_depth (HISTORY(p)); 
	
	for (depth = 1; (depth < (stack_index - 1)); )
	{
		tos   = stack_depth (HISTORY(p));
		depth = tos - stack_index + depth;
		stack_index = tos;
		if (!stack_get (HISTORY(p), (Generic*)&event, depth++))
			return;
		
		/* If we are changing from a group of actions to single actions
		 * we must ask to continue undoing.  If we are going from a
		 * single action to a group we must record the dot location
		 * so that two group actions wont be put together as one
		 */
		if ((curvature == CONTINUOUS) && (event.curvature == DISJOINT))
		{
			sm_ted_set_dot(p,event.dot);
			if (sm_ted_continue (p, event.curvature, depth, tos) == false)
				return;
		}		
		else if ((curvature == DISJOINT) && (event.curvature == CONTINUOUS) &&
			 (action == FIRST))
		{
			sm_ted_dot_record (p);
		}
		
		/* redundant dot movement is ignored */
		while ((event.amount == 0) && (event.dot == DOT(p)))
		{
			if (!stack_get (HISTORY(p), (Generic*)&event, depth++))
				return;
		}
		curvature = event.curvature;
		old_dot = DOT(p);
		if (DOT(p) != event.dot)
		{
			/* Deletion of characters is executed as a delete next
			 * n chars, but if it was not originally done that way
			 * we hide the dot positioning.  Also in continuous 
			 * groups we hide dot positioning, otherwise we position
			 * and query
			 */
			if ((event.curvature == CONTINUOUS) || 
			    ((event.amount < 0) &&
			    ((event.dot == (DOT(p) + event.amount)) ||
			    (event.dot == (DOT(p) - event.amount)))))
			{
				sm_ted_set_dot(p,event.dot);
			}
			else
			{
				sm_ted_damaged_prefer (p);
				sm_ted_damaged_dot_row (p);
				sm_ted_set_dot(p,event.dot);
				if (sm_ted_continue (p, event.curvature, depth, (stack_index - 1)) == false)
					return;
			}
		}
		if (event.amount < 0)
		{
			
			sm_ted_set_dot(p,event.location);
			sfree (KILLBUF(p));
			KILLBUF(p) = sm_ted_buf_delete_n_chars (p, -event.amount);
			
			UNDO(p).adot = old_dot;
			UNDO(p).dot  = event.adot;
			UNDO(p).curvature = event.curvature;
			(void) stack_set (HISTORY(p), (Generic*)&UNDO(p), 1);

			sm_ted_set_dot(p,event.adot);
			sm_ted_undo_damage (p, event, old_dot);
			
			if (sm_ted_continue (p, event.curvature, depth, (stack_index - 1)) == false)
				return;
		}
		else if (event.amount > 0)
		{
			sm_ted_set_dot(p,event.location);
			sm_ted_buf_insert_n_chars (p, event.buffer, event.amount);
			
			UNDO(p).adot = old_dot;
			UNDO(p).dot  = event.dot;
			UNDO(p).curvature = event.curvature;
			(void) stack_set (HISTORY(p), (Generic*)&UNDO(p), 1);

			sm_ted_set_dot(p,event.adot);
			sm_ted_undo_damage (p, event, old_dot);

			if (sm_ted_continue (p, event.curvature, depth, (stack_index - 1)) == false)
				return;
		}
		action++;
	}
	if (depth < (stack_index - 1))
		MODS(p) = 0;
}
/* Performs a single undo of the last editing operation.
 */

void sm_ted_undo_one (Pane *p)
{
	Undo event;

	int stack_index;	/* top of stack at some point */
	int tos;		/* current top of stack */
	int old_dot;		
	int curvature = DISJOINT;	
	int depth = 1;		/* depth to look into stack */	
	int action = FIRST;

	MODS(p)++;

	stack_index = stack_depth (HISTORY(p)); 
	
	while (depth < (stack_index - 1))
	{
		tos   = stack_depth (HISTORY(p));
		depth = tos - stack_index + depth;
		stack_index = tos;
		if (!stack_get (HISTORY(p), (Generic*)&event, depth++))
			return;

		if ((curvature == CONTINUOUS) && (event.curvature == DISJOINT))
		{
			sm_ted_set_dot(p,event.dot);
			return;
		}

		/* For continuous events the dot must be recorded in the undo stack */
		else if ((event.curvature == CONTINUOUS) && (action == FIRST))
		{
			sm_ted_dot_record (p);
		}
		
		/* redundant dot movement is ignored */
		while ((event.amount == 0) && (event.dot == DOT(p)))
		{
			if (!stack_get (HISTORY(p), (Generic*)&event, depth++))
				return;
		}
		curvature = event.curvature;
		old_dot = DOT(p);
		if (DOT(p) != event.dot)
		{
			/* Deletion of characters is executed as a delete next
			 * n chars, but if it was not originally done that way
			 * we hide the dot positioning.  Also in continuous 
			 * groups we hide dot positioning, otherwise we position
			 * the dot and quit.
			 */
			if ((event.curvature == CONTINUOUS) || 
			    ((event.amount < 0) &&
			    ((event.dot == (DOT(p) + event.amount)) ||
			    (event.dot == (DOT(p) - event.amount)))))
			{
				sm_ted_set_dot(p,event.dot);
			}
			else
			{
				sm_ted_damaged_prefer (p);
				sm_ted_damaged_dot_row (p);
				sm_ted_set_dot(p,event.dot);
				if (event.curvature == DISJOINT)
					return;
			}
		}
		if (event.amount < 0)
		{
			
			sm_ted_set_dot(p,event.location);
			sfree (KILLBUF(p));
			KILLBUF(p) = sm_ted_buf_delete_n_chars (p, -event.amount);
			
			UNDO(p).adot = old_dot;
			UNDO(p).dot  = event.adot;
			UNDO(p).curvature = event.curvature;
			(void) stack_set (HISTORY(p), (Generic*)&UNDO(p), 1);

			sm_ted_set_dot(p,event.adot);
			sm_ted_undo_damage (p, event, old_dot);
			
			if (event.curvature == DISJOINT)
				return;
		}
		else if (event.amount > 0)
		{
			sm_ted_set_dot(p,event.location);
			sm_ted_buf_insert_n_chars (p, event.buffer, event.amount);
			
			UNDO(p).adot = old_dot;
			UNDO(p).dot  = event.dot;
			UNDO(p).curvature = event.curvature;
			(void) stack_set (HISTORY(p), (Generic*)&UNDO(p), 1);

			sm_ted_set_dot(p,event.adot);
			sm_ted_undo_damage (p, event, old_dot);

			if (event.curvature == DISJOINT)
				return;
		}
		action++;
	}
	if (depth < (stack_index - 1))
		MODS(p) = 0;
}

Boolean sm_ted_continue (Pane *p, short curvature, int depth, int tos)
{
	Boolean continu;	
	
	if (curvature == DISJOINT)
	{ /* the actions are divisable */
	
		sm_ted_repair (p);
		if (depth < tos)
		{ /* there is stuff on the stack */
			continu = undo_confirm (UND(p));
			if (continu == false)
			{
				undo_dialog_hide(UND(p));
				return false;
			}
		}
		else
		{
			MODS(p) = 0;
			undo_dialog_hide (UND(p));
		 	return false;
		}
	}
	return true;
}

void sm_ted_undo_damage (Pane *p, Undo event, int old_dot)
{
	if ((old_dot == event.adot) && (event.amount < 0))
		sm_ted_damaged_prefer (p);
	else if (old_dot != event.adot)
		sm_ted_damaged_prefer (p);

	if ((sm_ted_count_lines (event.buffer) == 0) &&
	    (old_dot == event.adot))
		sm_ted_damaged_line_to_end (p, event.adot);
	else
	{
		sm_ted_damaged_dot_row (p);
		sm_ted_damaged_win (p);
	}
	sm_ted_damaged_buffer (p);
}

void sm_ted_undo_destroy (Pane *p)
{
	Undo event;
	int tos;
	int depth;
	
	tos = stack_depth (HISTORY(p));
	
	/* delete all the ssaved strings in the undo stack */
	
	for (depth = 1; depth < (tos - 1); depth++)
	{
		if (!stack_get (HISTORY(p), (Generic*)&event, depth))
			return;

		if (event.buffer != sm_ted_undo_null_string)
			sfree (event.buffer);
	}
	
	/* destroy the entire stack */
	stack_destroy( HISTORY(p));
}
