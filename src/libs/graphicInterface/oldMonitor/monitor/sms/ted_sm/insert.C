/* $Id: insert.C,v 1.1 1997/06/25 14:59:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/graphicInterface/oldMonitor/monitor/sms/ted_sm/tedprivate.h>

/*
 *  insert_char inserts a normal (not newline) character into the current buffer.
 */
void sm_ted_insert_char(Pane *p)
{
	KbString ks;
	char     ch;
	short    width;

	/* get the last ch mapped */
	ks = keymap_seq_KbString(KEY_MAP(p));
	ch = (char) ks.kc_ptr[ks.num_kc - 1];
	freeKbString(ks);

	/* put the char on the screen and in the buffer, fixing everything */
	switch ( char_tab[ ch ] )
	{
		case LF:
			sm_ted_insert_line(p);
			return;

		case TAB:
			width = sm_ted_tab_size(p,DOT_LOC(p).x);
			break;

		case CTRL:
		case META:
			width = sm_ted_ctrl_size(p,DOT_LOC(p).x);
			break;

		default:
			width = 1;
			break;
	}
	sm_ted_damaged_line_to_end( p, DOT(p) );
	if (PREFER(p) != EOL)
		PREFER(p) += width;
	sm_ted_damaged_dot_col(p);

	sm_ted_buf_insert_char(p, ch);
	sm_ted_damaged_buffer (p);

	MODS(p)++;
}


/* bound to return */
void sm_ted_newline(Pane *p)
{
	sm_ted_buf_insert_char(p,'\n');

	MODS(p)++;
	PREFER(p) = 0;

	if (DOT_LOC(p).y == SIZE(p).y - 1)
	{ /* the newline is at the bottom of the window, damage the whole window */
		sm_ted_damaged_win (p);
	}
	else
	{ /* just damage to the bottom */
		DOT_LOC(p).y++;
		sm_ted_damaged_win_to_end(p, DOT(p));
	}
	sm_ted_damaged_dot_col(p);
	sm_ted_damaged_buffer (p);
}


/* bound to CTRL-O */
void sm_ted_newline_and_backup(Pane *p)
{
	sm_ted_buf_insert_char(p,'\n');

	sm_ted_set_dot_relative(p,-1);
	PREFER(p) = EOL;
	MODS(p)++;

	sm_ted_damaged_win_to_end(p,DOT(p));
	sm_ted_damaged_buffer (p);
}


/*
 *  Insert_line inserts a new line below the current line, padding it with initial white space equal to the
 *  initial white space found on the current line.  The new position is at the end of the new line.
 *  Currently, the padding is done only with blanks.  This function is bound to linefeed (CTRL-J).
 */
void sm_ted_insert_line(Pane *p)
{
	short nb;				/* number of bytes inserted into the buffer */
	int bol, eol;				/* indices of beginning/end of line */
	short width = 0;			/* width of the leading white space */
	char *buf;				/* buffer used for inserting chars in buffer */
	int i, offset;

	/* count blanks at start off current line */
	bol = sm_ted_whereis_bol(p,DOT(p));
	eol = sm_ted_whereis_eol(p,DOT(p));
	
	offset = bol;
	for ( nb = 0; 
	      IS_WHITE(sm_ted_buf_get_char(p,offset)) && nb < eol - bol; 
	      offset++ )
		nb++;

	width = sm_ted_width_of_text( p, bol, 0, nb );

	/* this would not be the case if we were using tabs for the new 
	 * inital white space
	 */
	nb = width + 1;

	/* We are going to insert a '\n' and [0..?] blanks one byte before the end of the current line.	*/

	sm_ted_goto_eol(p);

	buf = (char *)get_mem(nb,"sm_ted_insert_line");

	buf[0] = '\n';
	for (i=1; i < nb; i++)
		buf[i] = ' ';
	sm_ted_buf_insert_n_chars (p, buf, nb);

	free_mem((void*)buf);

	PREFER(p) = width;
	sm_ted_damaged_dot_col(p);

	if (DOT_LOC(p).y == SIZE(p).y - 1)
	{ /* the newline is at the bottom of the window, damage the whole window */
		sm_ted_damaged_win (p);
	}
	else
	{ /* just damage to the bottom */
		DOT_LOC(p).y++;
		sm_ted_damaged_win_to_end(p,eol);
	}
	sm_ted_damaged_buffer (p);
	MODS(p)++;
}

void sm_ted_insert_copy_buffer (Pane *p)
{
	if (strcmp (COPYBUF(p), "") == 0)
		return;
	sm_ted_buf_insert_n_chars (p, COPYBUF(p), strlen (COPYBUF(p)));
	sm_ted_damaged_dot_row (p);
	sm_ted_damaged_win (p);
	sm_ted_damaged_prefer (p);
	sm_ted_damaged_buffer (p);

	MODS(p)++;
}
void sm_ted_insert_kill_buffer (Pane *p)
{
	if (strcmp (KILLBUF(p), "") == 0)
		return;
	sm_ted_buf_insert_n_chars (p, KILLBUF(p), strlen (KILLBUF(p)));
	sm_ted_damaged_dot_row (p);
	sm_ted_damaged_win (p);
	sm_ted_damaged_prefer (p);
	sm_ted_damaged_buffer (p);

	MODS(p)++;
}

