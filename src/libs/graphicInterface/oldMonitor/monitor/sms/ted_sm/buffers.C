/* $Id: buffers.C,v 1.2 2001/09/17 00:48:55 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <include/bstring.h>

#include <libs/graphicInterface/oldMonitor/monitor/sms/ted_sm/tedprivate.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/errno.h>

static
struct datadoc_package {
    Pane *p;
    char *filename;
} package ;


static char ml_default[] =
"Buffer: %b\nFile: %f\nChanges: %m\nLocation: %p";

/* forward declarations */
EXTERN(char*, getwd,(char*));				/* man 3 getwd */

void sm_ted_buf_create(Pane *p)
{
	TedMark mark;
	TedBuf *buf = (TedBuf *)get_mem( sizeof(TedBuf), "ted_buf_create" );

	bzero( (char *)buf, sizeof(TedBuf) );
	BUF(p) = buf;
	
	buf->my_wins = util_list_alloc( (Generic)buf, "ted_buf_create" );

	BUFNAME(p)	= ssave("");
	FILENAME(p)	= ssave("");
	PATHNAME(p)	= ssave("");
	FORMAT(p)	= ssave(ml_default);
	MODS(p)		= 0;
	MARK(p)		= NOT_SET;
	NUM_MARKS(p)   	= 1;
	COPYBUF(p)	= ssave("");
	KILLBUF(p)	= ssave("");
	STORE(p)	= flex_create (sizeof(char));
	MARKLIST(p)	= flex_create (sizeof (TedMark));
	HISTORY(p)	= stack_create (sizeof (Undo));
	SAVE_BEHAV(p)	= BEHAV_SAVEIT;

	UNDO(p).curvature = DISJOINT;
	UND(p)		  = undo_dialog_create ("Undoing");
	mark.markname     = ssave("<default>");
	mark.location     = NOT_SET;
	flex_insert_one (MARKLIST(p), 0, (char *) &mark);
	
}

Boolean sm_ted_buf_destroy(Pane *p)
{
	int i;
	TedMark *marks;
	
	if (! util_list_empty(WIN_LIST(p)) )
		return false;

	util_list_free(WIN_LIST(p));

	flex_destroy (STORE(p));

	sfree( BUFNAME(p) );
	sfree( FILENAME(p) );
	sfree( PATHNAME(p) );
	sfree( FORMAT(p) );
	sfree( COPYBUF(p) );
	sfree( KILLBUF(p) );
	(void) sm_ted_undo_destroy (p);
	
	marks = (TedMark *) (Generic) flex_get_buffer (MARKLIST(p), 0, NUM_MARKS(p), (char *) 0);
	for (i = 0; i < NUM_MARKS(p); i++)
	{
		sfree (marks[i].markname);
	}
	flex_destroy( MARKLIST(p) );
	undo_dialog_destroy(UND(p));
	free_mem ((void*) BUF(p));
	free_mem(marks);
	return true;
}

Generic sm_ted_get_buf(Pane *p)
{
	return (Generic) BUF(p);
}

void sm_ted_set_buf(Pane *p, Generic buf)
{
	BUF(p) = (TedBuf *)buf;
}

Generic sm_ted_buf_get_owner(Pane *p)
{
	if ( BUF(p) == (TedBuf *)NULL)
		return NULL;
	else
		return BUF_OWNER(p);
}


void sm_ted_buf_set_owner(Pane *p, Generic owner)
{
	BUF_OWNER(p) = owner;
}


void sm_ted_buf_set_bname(Pane *p, char *bname)
{
	smove(&(BUFNAME(p)),bname);
}

void sm_ted_buf_set_fname(Pane *p, char *fname)
{
	smove(&(FILENAME(p)),fname);
}

void sm_ted_buf_set_pname(Pane *p, char *pname)
{
	char *tempname = PATHNAME(p);
	char *temppath = pname;
	int i;
	
	for (i = 0; temppath[i] != '\0'; i++);   /* goto the end of the string */
	for (i--; temppath[i] != '/'; i--)       /* strip off the file name to */
		temppath[i] = '\0';		 /* get the path name */
	
	PATHNAME(p) = ssave(temppath);
	sfree(tempname);
}

char *sm_ted_buf_get_bname(Pane *p)
{
	return BUFNAME(p);
}

char *sm_ted_buf_get_fname(Pane *p)
{
	return FILENAME(p);
}

char *sm_ted_buf_get_pname(Pane *p)
{
	return PATHNAME(p);
}

void sm_ted_buf_set_type(Pane *p, sm_ted_buftype type)
{
	TYPE(p) = type;
}

sm_ted_buftype sm_ted_buf_get_type(Pane *p)
{
	return (TYPE(p));
}

/*
 * sm_ted_buf_get_text - get (a copy of) the current buffer text.
 */
char *sm_ted_buf_get_text(Pane *p)
{
	return flex_get_buffer (STORE(p), 0, flex_size (STORE(p)), (char *) 0);
}

static int newlines_in_text(char *cp, int n)
{
	int i;
	int newlines = 0;

	for (i=0; i < n; i++)
		if (cp[i] == '\n')
			newlines++;
	return newlines;
}

/*
 * sm_ted_buf_set_text - set the current window to contain text.
 */
void sm_ted_buf_set_text(Pane *p, char *text, int len)
{
	int size;
	UtilNode *wnode;
	TedWin *save_win;

	size = flex_size(STORE(p));
	if (len > size)
	{

		UNDO(p).dot = HIDE;
		UNDO(p).adot = 0;
		iflex_insert (HISTORY(p), &(UNDO(p)), STORE(p), size, len-size);
	}
	else if (len < size)
	{

		UNDO(p).dot = HIDE;
		UNDO(p).adot = 0;
		iflex_delete (HISTORY(p), &(UNDO(p)), STORE(p), len, size-len);
	}
	UNDO(p).dot = 0;
	UNDO(p).adot = 0;
	iflex_set_buffer(HISTORY(p), &(UNDO(p)), STORE(p), 0, len, text );
	LINES(p) = newlines_in_text(text,len);
	sm_ted_win_adjust(p, size, len-size);

	/* It's better to be safe */
	save_win = WIN(p);
	wnode = util_head(WIN_LIST(p));
	while (wnode != NULL)
	{
		WIN(p) = (TedWin *)util_node_atom(wnode);
		DOTLINE(p) = flex1_count_occurrences(STORE(p),0,DOT(p),'\n');
		wnode = util_next(wnode);
	}
	WIN(p) = save_win;

	MODS(p) = 0;
	sm_ted_damaged_win(p);
	sm_ted_damaged_dot_col(p);
	sm_ted_damaged_dot_row(p);
}

char sm_ted_buf_get_char(register Pane *p, register int offset)
{
	return flex1_get_one (STORE(p), offset);
}

void sm_ted_buf_set_char(register Pane *p, register int offset, char ch)
{
	MODS(p)++;
	UNDO(p).dot = DOT(p);
	UNDO(p).adot = DOT(p);
	iflex_set_one (HISTORY(p), &(UNDO(p)), STORE(p), offset, &ch);
}


/*
 * sm_ted_buf_insert_char - insert a char before the dot (ie. advance the dot)
 */
void sm_ted_buf_insert_char(Pane *p, char ch)
{
	int lines;
	int dot = DOT(p);

	UNDO(p).dot = dot;
	UNDO(p).adot = dot;
	iflex_insert_one (HISTORY(p), &(UNDO(p)), STORE(p), dot, &ch);

	sm_ted_win_adjust (p, dot, 1);

	lines = (ch == '\n') ? 1 : 0;
	LINES(p) += lines;
	sm_ted_set_dot_relative(p,1);
}

/*
 * sm_ted_buf_insert_n_chars - insert n chars after the dot
 */
void sm_ted_buf_insert_n_chars (Pane *p, char *str, int len)
{
	int lines;
	int dot = DOT(p);

	UNDO(p).dot = dot;
	UNDO(p).adot = dot;
	iflex_insert_buffer (HISTORY(p), &(UNDO(p)), STORE(p), dot, len, str);

	sm_ted_win_adjust(p,dot,len);

	lines = newlines_in_text(str,len);
	LINES(p) += lines;
	sm_ted_set_dot_relative(p,len);
}
	
/*
 * sm_ted_buf_delete_char - delete the character following the dot
 */
char *sm_ted_buf_delete_char(Pane *p)
{
	char *kill;
	int lines;
	int dot = DOT(p);
	
	sm_ted_win_adjust(p, dot + 1, -1);

	UNDO(p).dot = dot;
	UNDO(p).adot = dot;
	kill = iflex_delete_one (HISTORY(p), &(UNDO(p)), STORE(p), dot, (char *)0);

	lines = (*kill == '\n') ? -1 : 0;
	LINES(p) += lines;
	return kill;
}
/*
 * sm_ted_buf_delete_nprev_chars - delete the n characters before the dot
 */
char *sm_ted_buf_delete_nprev_chars(Pane *p, int amount)
{
	char *kill;
	int lines;
	int dot = DOT(p);
	
	sm_ted_win_adjust(p, dot, -amount);

	UNDO(p).dot = dot - amount;
	UNDO(p).adot = dot;
	kill = iflex_delete_buffer (HISTORY(p), &(UNDO(p)), STORE(p), dot - amount, amount, (char *)0);

	lines = -newlines_in_text(kill,amount);
	LINES(p) += lines;
	return kill;
}

/*
 * sm_ted_buf_delete_n_chars - delete the n characters following the dot
 */
char *sm_ted_buf_delete_n_chars(Pane *p, int n)
{
	char *buf;
	int lines;
	int dot = DOT(p);

	sm_ted_win_adjust (p, dot + n, -n);

	UNDO(p).dot = dot;
	UNDO(p).adot = dot;
	buf = iflex_delete_buffer (HISTORY(p), &(UNDO(p)), STORE(p), dot, n, (char *)0);

	lines = -newlines_in_text(buf,n);
	LINES(p) += lines;
	return buf;
}


/*
 *  sm_ted_buf_use_file is for reading files into buffers.
 *	Currently, it returns nothing.
 */
void
sm_ted_buf_use_file(Pane *p, char *fname)
{
	int size;
	int lines;

	(void) sm_ted_buf_erase(p);

	UNDO(p).dot = 0;
	UNDO(p).adot = 0;
	size = iflex_insert_file (HISTORY(p), &(UNDO(p)), STORE(p), 0, fname);

	sm_ted_win_adjust(p, 0, size);
	lines = flex1_count_occurrences(STORE(p), 0, size, '\n');
	LINES(p) += lines;

	MODS(p) = 0;
	sm_ted_damaged_win(p);
	sm_ted_damaged_dot_col(p);
	sm_ted_damaged_dot_row(p);
}

/*
 * sm_ted_buf_erase - Erase the contents of a buffer.  
 */
void sm_ted_buf_erase(Pane *p)
{
	int lines;
	int textsize = flex_size (STORE(p));
	
	sm_ted_win_adjust(p,textsize,-textsize);

	UNDO(p).dot = 0;
	UNDO(p).adot = 0;
	lines = LINES(p);
	iflex_delete (HISTORY(p), &(UNDO(p)), STORE(p), 0, textsize);
	MODS(p)++;

	LINES(p) = 0;
	sm_ted_damaged_dot_col(p);
	sm_ted_damaged_win(p);
}

Boolean sm_ted_buf_is_empty(Pane *p)
{
	return BOOL((flex_size(STORE(p))) == 0);
}

void sm_ted_print_info(Pane *p)
{
	int i;
	int j;
	char *format = FORMAT(p);
	char *bn = BUFNAME(p);
	char *fn = FILENAME(p);
	char info[256];
	char ms[32];
	char ps[32];
	int bufsize = flex_size (STORE(p));
	int percent;

	(void) sprintf(ms,"%d",MODS(p));
	if (bufsize == 0 || DOT(p) == 0)
		(void) sprintf(ps,"%s","Top");
	else
	{
		percent = (100*DOT(p))/bufsize;
		(void) sprintf(ps,"%d %%",percent);
	}

	for (i=0,j=0;format[i] != '\0';i++)
	{
		if (format[i] == '%')
		{
			info[j] = '\0';		/* Eliminate trash in the string */

			switch(format[++i])
			{
			case 'b':
				(void) strcat(info,bn);
				j += strlen(bn);
				break;
			case 'f':
				(void) strcat(info,fn);
				j += strlen(fn);
				break;
			case 'p':
				(void) strcat(info,ps);
				j += strlen(ps);
				break;
			case 'm':
				(void) strcat(info,ms);
				j += strlen(ms);
				break;
			case '\0':
				i--;
				info[j++] = format[i];
				break;
			default:
				info[j++] = format[i-1];
				info[j++] = format[i];
				break;
			}
		}
		else
			info[j++] = format[i];
	}

	info[j] = '\0';
	message(info);
}
int sm_ted_buf_written(Pane *p)
{
	return (MODS(p) == 0);
}

void sm_ted_buf_set_mark(Pane *p, int num_mark)
{
	TedMark *mark;

	mark = (TedMark *) (Generic) flex_get_one (MARKLIST(p), num_mark);
	mark->location = DOT(p);
	flex_set_one (MARKLIST(p), num_mark, (char *)mark);

	if (num_mark == 0)
		MARK(p) = DOT(p);
	free_mem((void*) mark);
}

void sm_ted_buf_mark_modify(Pane *p)
{
	typedef enum marknamesenum {
		goto_mark, 
		name_mark, 
		delete_mark, 
		rename_mark, 
		max_marks
	} marknames;

	static char *select[] = {"goto mark ...", "name mark ...", "delete mark ...", "rename mark ..."};
	
	char **menu;
	char  *buffer;
	marknames selection1;
	int selection2;
	int i;
	TedMark *marks;
	TedMark newmark;
	FILE *fopen();

	selection1 = (marknames) menu_select ("", (short) max_marks, select);
	switch (selection1)
	{
		case goto_mark:	
			menu = (char **) get_mem (NUM_MARKS(p) * sizeof (char *), "sm_ted_buf_mark_modify");
			marks = (TedMark *) (Generic) flex_get_buffer (MARKLIST(p), 0, NUM_MARKS(p), (char *) 0);
			for (i = 0; i < NUM_MARKS(p); i++)
			{
				menu[i] = marks[i].markname;
			}
			selection2 = menu_select ("marks", NUM_MARKS(p), menu);
			if (selection2 == UNUSED)
				break;
			sm_ted_win_goto_mark (p, selection2);
			free_mem((void*) marks);
			break;

		case name_mark:
			buffer = prompted_string_edit ("name of mark?", makePoint (10, 1), "");
			if (strcmp (buffer, "") == 0)
				break;
			if (sm_ted_buf_repeat_mark_name (p, buffer))
				break;
			newmark.markname = buffer;	
			newmark.location = DOT(p);
			flex_insert_one (MARKLIST(p), NUM_MARKS(p), (char *) &newmark);
			++NUM_MARKS(p);
			break;

		case delete_mark:
			menu = (char **) get_mem (NUM_MARKS(p) * sizeof (char *), "sm_ted_buf_mark_modify");
			marks = (TedMark *) (Generic) flex_get_buffer (MARKLIST(p), 0, NUM_MARKS(p), (char *) 0);
			for (i = 0; i < NUM_MARKS(p); i++)
			{
				menu[i] = marks[i].markname;
			}
			selection2 = menu_select ("marks", NUM_MARKS(p), menu);
			if (selection2 == UNUSED)
				break;
			if (selection2 == 0)
			{
				sm_ted_bitch (p, "can't delete default mark");
				break;
			}
			sfree (marks[selection2].markname);
			buffer = flex_delete_one (MARKLIST(p), selection2, (char *) 0); 
			--NUM_MARKS(p);
			sfree (buffer);
			free_mem((void*) marks);
			break;

		case rename_mark:
			menu = (char **) get_mem (NUM_MARKS(p) * sizeof (char *), "sm_ted_buf_mark_modify");
			marks = (TedMark *) (Generic) flex_get_buffer (MARKLIST(p), 0, NUM_MARKS(p), (char *) 0);
			for (i = 0; i < NUM_MARKS(p); i++)
			{
				menu[i] = marks[i].markname;
			}
			selection2 = menu_select ("marks", NUM_MARKS(p), menu);
			if (selection2 == UNUSED)
				break;
			buffer = prompted_string_edit ("new name of mark?",
							       makePoint (10, 1), 
							       marks[selection2].markname);
			if (strcmp (buffer, "") == 0)
				break;
			if (sm_ted_buf_repeat_mark_name (p, buffer))
				break;
			sfree (marks[selection2].markname);
			marks[selection2].markname = buffer;
			flex_set_one (MARKLIST(p), selection2, (char *) &marks[selection2]);
			free_mem((void*) marks);
			break;
		
		default:
			break;
	}
}

Boolean sm_ted_buf_repeat_mark_name (Pane *p, char *name)
{
	int i;
	TedMark *marks;
	
	marks = (TedMark *) (Generic) flex_get_buffer (MARKLIST(p), 0,NUM_MARKS(p), (char *) 0);

	for (i = 0; i < NUM_MARKS(p); i++)
	{
		if (strcmp (name, marks[i].markname) == 0)
		{
			sm_ted_bitch (p, "Mark names must be unique");
			return (true);
		}
	}
	free_mem((void*) marks);
	return (false);
}			

int sm_ted_buf_name_unique_mark (Pane *p, char *title)
{
	char name [100];
	char *buffer;
	TedMark newmark;

	(void) sprintf (name, "_%s_%d", title, NUM_MARKS(p));

	if (sm_ted_buf_repeat_mark_name (p, name))
		return -1;

	buffer = ssave (name);
	newmark.markname = buffer;	
	newmark.location = DOT(p);
	flex_insert_one (MARKLIST(p), NUM_MARKS(p), (char *) &newmark);
	++NUM_MARKS(p);
	return (NUM_MARKS(p) -1);
}
void sm_ted_buf_delete_mark_number (Pane *p, int num)
{
	TedMark *mark;

	if (num == 0)
	{
		sm_ted_bitch (p, "Can't delete default mark.");
	}
	mark = (TedMark *) (Generic) flex_delete_one (MARKLIST(p), num, (char *) 0);
	-- NUM_MARKS(p);
	sfree (mark->markname);
	free_mem ((void*) mark);
}
/* sm_ted_buf_complete_file_name -- makes a complete file (path) name for a 
 *	name passed in using the directory from which ted was invoked for the
 *	path.
 */
char *sm_ted_buf_complete_file_name (Pane *p, char *name)
{
	char *complete = name;
	char *path;
	
	if (name[0] == '/')
	{
		complete = (char *)get_mem (strlen(name) + 1, "sm_ted_buf_complete_file_name");	
		(void) strcpy (complete, name);
	}
	else
	{
		path = sm_ted_buf_get_pname (p);
		complete = (char *)get_mem (strlen (path) + strlen(name) + 1, "sm_ted_buf_complete_file_name");
		(void) strcpy (complete, path);
		(void) strcat (complete, name);
	}
	return (complete);
}


/* manipulate the save mode */
void sm_ted_buf_set_save_mode(Pane *p, int mode)
{
  SAVE_BEHAV(p) = mode;
}

