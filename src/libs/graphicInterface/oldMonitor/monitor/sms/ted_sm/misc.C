/* $Id: misc.C,v 1.1 1997/06/25 14:59:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/graphicInterface/oldMonitor/monitor/sms/ted_sm/tedprivate.h>

void sm_ted_unbound_key(Pane *p)
{
	sm_ted_bitch( p, "Unbound key" );
}

void sm_ted_bell(Pane *p)
{
	flashPane(p);
}

void sm_ted_transpose_chars(Pane *p)
{
	char c1, c2;
	int  curvature;
	int  offset = DOT(p);
	
	if ( offset - 2 < 0 )
		return;
	/*
	 * Set c1 and c2, swap the contents of the two buffer positions
	 * they represent (but not c1 and c2)
	 */
	c1 = sm_ted_buf_get_char(p, offset - 1);
	c2 = sm_ted_buf_get_char(p, offset - 2);

	if (c1 == c2)	/* An important test when c1 == c2 == '\n' */
		return;
	
	curvature = UNDO(p).curvature;
	UNDO(p).curvature = CONTINUOUS;
	
	sm_ted_buf_set_char(p, offset - 1, c2);
	sm_ted_buf_set_char(p, offset - 2, c1);
	
	if ((UNDO(p).curvature = curvature) == DISJOINT)
		sm_ted_dot_record(p);

	if (c1 == '\n' || c2 == '\n')
		sm_ted_damaged_prefer (p);

	sm_ted_damaged_line_to_end (p, DOT(p)-2);
	sm_ted_damaged_line_to_end (p, DOT(p));
	sm_ted_damaged_buffer (p);
}

/* sets a multiplier for a function call to a user defined value */

void sm_ted_set_multiplier (Pane *p)
{
	char *buffer;
	int   number;

	buffer = prompted_string_edit ("How many times? ", makePoint (10, 1), "");
	number = atoi (buffer);
	if ((number == 0) && (strlen (buffer) == 0))
		MULTIPLIER(p) = 1;
	else
		MULTIPLIER(p) = number;
		
}
int sm_ted_count_lines (char *str)
{
	int count = 0;
	int i;
	
	if (str == "")
		return count;
	
	for (i = 0; str[i] != '\0'; i++)
	{
		if (str[i] == '\n')
			count++;
	}
	return count;
}

void sm_ted_case_word_upper (Pane *p)
{
	char c;
	int i;
	int curvature;
	int boword = sm_ted_whereis_beginning_of_word (p, DOT(p));
	
	if (boword == -1)
		return;
	
	curvature = UNDO(p).curvature;
	UNDO(p).curvature = CONTINUOUS;
	
	for (i = boword; ;i++)
	{
		c = sm_ted_buf_get_char (p, i);
		if (IS_ALPHA(c))
		{
			c = UPPERCASE(c);
			sm_ted_buf_set_char (p, i, c);
		}
		else if (!IS_DIGIT(c))
			break;
	}
	if ((UNDO(p).curvature = curvature) == DISJOINT)
		sm_ted_dot_record(p);
	
	sm_ted_damaged_line_to_end (p, boword);
}

void sm_ted_case_word_lower (Pane *p)
{
	char c;
	int i;
	int curvature;
	int boword = sm_ted_whereis_beginning_of_word (p, DOT(p));
	
	if (boword == -1)
		return ;

	curvature = UNDO(p).curvature;
	UNDO(p).curvature = CONTINUOUS;
	
	for (i = boword; ;i++)
	{
		c = sm_ted_buf_get_char(p, i);
		if (IS_ALPHA(c))
		{
			c = LOWERCASE(c);
			sm_ted_buf_set_char(p, i, c);
		}
		else if (!IS_DIGIT(c))
			break;
	}
	if ((UNDO(p).curvature = curvature) == DISJOINT)
		sm_ted_dot_record(p);

	sm_ted_damaged_line_to_end (p, boword);	
}

void sm_ted_case_word_capitilize (Pane *p)
{
	char c;
	int i;
	int curvature;
	int boword = sm_ted_whereis_beginning_of_word (p, DOT(p));
	
	if (boword == -1)
		return ;
		
	curvature = UNDO(p).curvature;
	UNDO(p).curvature = CONTINUOUS;
	
	c = sm_ted_buf_get_char(p, boword);
	if (IS_ALPHA(c))
	{
		c = UPPERCASE(c);
		sm_ted_buf_set_char(p, boword, c);
	}
	for (i = boword + 1; ;i++)
	{
		c = sm_ted_buf_get_char(p, i);
		if (IS_ALPHA(c))
		{
			c = LOWERCASE(c);
			sm_ted_buf_set_char(p, i, c);
		}
		else
			break;
	}
	if ((UNDO(p).curvature = curvature) == DISJOINT)
		sm_ted_dot_record(p);

	sm_ted_damaged_line_to_end (p, boword);
}
