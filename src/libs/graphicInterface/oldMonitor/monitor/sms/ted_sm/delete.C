/* $Id: delete.C,v 1.1 1997/06/25 14:59:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/graphicInterface/oldMonitor/monitor/sms/ted_sm/tedprivate.h>

int sm_ted_delete_prev_char(Pane *p)
{
	if ( sm_ted_at_bob(p) )
	{
		sm_ted_inform(p,"Beginning of file");
		return 0;
	}

	sm_ted_damaged_prefer(p);
	sfree (KILLBUF(p));
	KILLBUF(p) = sm_ted_buf_delete_nprev_chars(p, 1);

	if ( *KILLBUF(p) == '\n' )
	{
		if (DOT_LOC(p).y != 0)
			DOT_LOC(p).y--;

		sm_ted_damaged_win_to_end( p, DOT(p) );
	}
	else
	{
		sm_ted_damaged_line_to_end( p, DOT(p) );
	}
	sm_ted_damaged_buffer (p);
	MODS(p)++;

  return 0;
}


/* bound to CTRL-D, CTRL-H */
int sm_ted_delete_next_char(Pane *p)
{
	if ( sm_ted_at_eob(p) )		/* at end of buffer? */
	{
		sm_ted_bitch(p, "Bottom of file");
		return 0;
	}
	sfree (KILLBUF(p));
	KILLBUF(p) = sm_ted_buf_delete_char(p);

	if ( *KILLBUF(p) == '\n' )
	{
		sm_ted_damaged_win_to_end( p, DOT(p) );
		sm_ted_damaged_prefer (p);
	}
	else
	{
		sm_ted_damaged_line_to_end( p, DOT(p) );
	}

	sm_ted_damaged_buffer (p);
	MODS(p)++;

  return 0;
}

int sm_ted_delete_next_word (Pane *p)
{
	int mark;
	
	if ((mark = sm_ted_whereis_end_of_word (p)) == -1)
		return 0;

	sfree (KILLBUF(p));
	KILLBUF(p) = sm_ted_buf_delete_n_chars (p, mark - DOT(p) + 1);

	if (sm_ted_count_lines (KILLBUF(p)) == 0)
		sm_ted_damaged_line_to_end (p, DOT(p));
	else
		sm_ted_damaged_win_to_end (p, DOT(p));
	sm_ted_damaged_buffer (p);
	
	MODS(p)++;

  return 0;
}

int sm_ted_delete_previous_word (Pane *p)
{
	int mark;
	int l;

	if ((mark = sm_ted_whereis_beginning_of_word(p, DOT(p) - 1)) == -1)
		return 0;
		
	if (DOT(p) != mark)
	{
		sfree (KILLBUF(p));
		KILLBUF(p) = sm_ted_buf_delete_nprev_chars (p, DOT(p) - mark);
	}
	sm_ted_damaged_prefer (p);
	l = sm_ted_count_lines (KILLBUF(p));
	if (l == 0)
		sm_ted_damaged_line_to_end (p, DOT(p));
	else
	{
		DOT_LOC(p).y -= l;
		sm_ted_damaged_win_to_end (p, DOT(p));
	}
	sm_ted_damaged_buffer (p);

	MODS(p)++;

  return 0;
}

int sm_ted_kill_to_end_of_line(Pane *p)
{
	int eol = sm_ted_whereis_eol (p, DOT(p));
	int nb = eol - DOT(p);

	if ( sm_ted_at_eol(p) )
	{
		sm_ted_delete_next_char (p);
		return 0;
	}
	sfree (KILLBUF(p));
	KILLBUF(p) = sm_ted_buf_delete_n_chars (p, nb);

	sm_ted_damaged_line_to_end (p, DOT(p) );
	sm_ted_damaged_buffer (p);
	MODS(p)++;

  return 0;
}

int sm_ted_kill_to_beginning_of_line(Pane *p)
{
	int bol = sm_ted_whereis_bol (p, DOT(p));
	int nb = DOT(p) - bol;
	
	if (nb == 0)
		return 0;
	sm_ted_goto_bol(p);
	
	sfree (KILLBUF(p));
	KILLBUF(p) = sm_ted_buf_delete_n_chars (p, nb);
	
	sm_ted_damaged_prefer (p);
	sm_ted_damaged_line_to_end (p, DOT(p));
	sm_ted_damaged_buffer (p);

	MODS(p)++;

  return 0;
}

int sm_ted_kill_line(Pane *p)
{
	int bol = sm_ted_whereis_bol (p, DOT(p));
	int eol = sm_ted_whereis_eol (p, DOT(p));
	int nb;
	char ch = sm_ted_buf_get_char (p, eol);
	
	if (ch == '\n')
		nb = eol - bol + 1;
	else	
		nb = eol - bol;
	sm_ted_goto_bol(p);
	
	sfree (KILLBUF(p));
	KILLBUF(p) = sm_ted_buf_delete_n_chars (p, nb);

	sm_ted_damaged_prefer (p);	
	sm_ted_damaged_win_to_end (p, DOT(p));
	sm_ted_damaged_buffer (p);

	MODS(p)++;

  return 0;
}

int sm_ted_delete_from_dot_to_mark (Pane *p)
{
	int nb;
	
	if (MARK(p) == NOT_SET)
	{
		sm_ted_bitch (p, "mark not set");
		return 0;
	}
	if (DOT(p) > MARK(p))
	{
		nb = DOT(p) - MARK(p);
		sm_ted_set_dot(p,MARK(p));
		sm_ted_damaged_dot_row (p);
		sm_ted_damaged_prefer (p);
	}
	else
	{
		nb = MARK(p) - DOT(p);
	}
	sfree (KILLBUF(p));
	KILLBUF(p) = sm_ted_buf_delete_n_chars (p, nb);
	
	sm_ted_damaged_win (p);
	sm_ted_damaged_buffer (p);
	
	MODS(p)++;

  return 0;
}

int sm_ted_copy_from_dot_to_mark (Pane *p)
{
	int nb;
	
	if (MARK(p) == NOT_SET)
	{
		sm_ted_bitch (p, "mark not set");
		return 0;
	}
	if (DOT(p) > MARK(p))
	{
		nb = DOT(p) - MARK(p);
		sm_ted_set_dot(p,MARK(p));
		sm_ted_damaged_dot_row (p);
		sm_ted_damaged_prefer (p);
	}
	else
	{
		nb = MARK(p) - DOT(p);
	}
	sfree (COPYBUF(p));
	COPYBUF(p) = sm_ted_buf_delete_n_chars (p, nb);
	
	sm_ted_damaged_win (p);	
	sm_ted_damaged_buffer (p);

	MODS(p)++;

  return 0;
}
