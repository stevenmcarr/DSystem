/* $Id: search.C,v 1.1 1997/06/25 14:59:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/graphicInterface/oldMonitor/monitor/sms/ted_sm/tedprivate.h>

#define BM_FORWARD 1
#define BM_BACKWARD -1

/* The searching call back for the find and replace dialogs.
 */
Boolean sm_ted_search(Pane *p)
{
	int new_dot, size;
	char *buf;
	char *newpat;
	char *replace;
	Boolean	dir;
	Boolean case_fold;

	BMS_private *bm = BMS(p);

	if (FRD(p) == NULL)
		return false;

	find_dialog_get_values (FRD(p), &newpat, &replace, &dir, &case_fold);

	bm_newpattern (bm, newpat, strlen (newpat));
	if (dir != bm_is_forward(BMS(p)))
		(void) bm_toggle_dir (BMS(p));
	if (case_fold != bm_is_casefolded (BMS(p)))
		(void) bm_toggle_case_fold (BMS(p));
	sfree (RPATTERN(p));
	RPATTERN(p) = ssave (replace);
	RLENGTH(p) = strlen (RPATTERN(p));
	

	/* So very sorry. */	
	size = flex_size (STORE(p));
	buf = flex_get_buffer (STORE(p), 0, size, (char*)0);

	new_dot = bm_search(bm, buf, size, DOT(p));

	sfree (buf);
	if (new_dot == UNUSED)
	{
		return (false);
	}
	sm_ted_set_dot(p, new_dot );

	sm_ted_damaged_prefer(p);
	sm_ted_damaged_dot_row(p);

	return (true);
}
/* The short cut call for searching from 
 *	search_forward and search_backward 
 */
static void sm_ted_search_alone(Pane *p, Boolean direction)
	/*Boolean direction; 	true -> forward */
{
	int new_dot, size;
	char *buf;
	char *newpat;
	char *replace;
	Boolean	dir;
	Boolean case_fold;

	BMS_private *bm = BMS(p);

	if (FRD(p) == NULL)
	{
		sm_ted_bitch (p, "Null search string.  Use find button to set pattern");
		return;
	}

	find_dialog_get_values (FRD(p), &newpat, &replace, &dir, &case_fold);
	
	if (strcmp (newpat, "") == 0)
	{
		sm_ted_bitch (p, "Null search string.  Use find button to set pattern");
		return;
	}
	if (strcmp (newpat, bm->pattern) != 0)
		bm_newpattern (bm, newpat, strlen (newpat));
	if (strcmp (replace, RPATTERN(p)) != 0)
	{
		sfree (RPATTERN(p));
		RPATTERN(p) = ssave (replace);
		RLENGTH(p) = strlen (RPATTERN(p));
	}
	if (direction != bm_is_forward(bm))
		(void) bm_toggle_dir (bm);
	if (case_fold != bm_is_casefolded (bm))
		(void) bm_toggle_case_fold (bm);

	/* So very sorry. */	
	size = flex_size (STORE(p));
	buf = flex_get_buffer (STORE(p), 0, size, (char*)0);

	new_dot = bm_search(bm, buf, size, DOT(p));

	sfree (buf);
	if (new_dot == UNUSED)
	{
		sm_ted_bitch (p, "Not Found");
		return;
	}
	sm_ted_set_dot(p, new_dot );

	sm_ted_damaged_prefer(p);
	sm_ted_damaged_dot_row(p);

	/* and reset the bm_search direction if we changed it */
	if (dir != bm_is_forward(bm))
		(void) bm_toggle_dir(bm);

	return;
}
int sm_ted_search_forward (Pane *p)
{
	sm_ted_search_alone (p, true /* search forward */);

  return 0;
}

int sm_ted_search_backward (Pane *p)
{
	sm_ted_search_alone (p, false /* search backward */);

  return 0;
}

int sm_ted_forward_replace (Pane *p)
{	
	BMS_private *bm = BMS(p);
	int mark_dot;
	int curvature = DISJOINT;
	int replacements = 0;
	
	if ((mark_dot = sm_ted_buf_name_unique_mark (p, "_sm_ted_forward_replace")) == -1)
	{
		sm_ted_bitch (p, "Can't name temporary mark.");
		return 0;
	}
	/* record current nature of buffer changes */
	sm_ted_dot_record (p);
	if (UNDO(p).curvature == CONTINUOUS)
	{
		curvature = CONTINUOUS;
	}
	else
		UNDO(p).curvature = CONTINUOUS;
	
	while (sm_ted_search (p))
	{
		replacements++;
		MODS(p)++;
		sfree (KILLBUF(p));
		KILLBUF(p) = sm_ted_buf_delete_nprev_chars (p, bm->patlen);
		sm_ted_buf_insert_n_chars (p, RPATTERN(p), RLENGTH(p));
		sm_ted_damaged_win_to_end (p, DOT(p));
	}
	if (curvature == DISJOINT)	/* leave buffer nature as it was */
		UNDO(p).curvature = DISJOINT;

	sm_ted_win_goto_mark (p, mark_dot);
	sm_ted_buf_delete_mark_number (p, mark_dot);

	sm_ted_dot_record (p);

	sm_ted_damaged_buffer(p);
	return replacements;
}

int sm_ted_backward_replace (Pane *p)
{
	BMS_private *bm = BMS(p);
	int mark_dot;
	int curvature = DISJOINT;
	int replacements = 0;

	if ((mark_dot = sm_ted_buf_name_unique_mark (p, "sm_ted_backward_replace")) == -1)
	{
		sm_ted_bitch (p, "Can't name temporary mark.");
		return 0; 
	}
	/* record current nature of buffer changes */
	sm_ted_dot_record (p);
	if (UNDO(p).curvature == CONTINUOUS)
	{
		curvature = CONTINUOUS;
	}
	else
		UNDO(p).curvature = CONTINUOUS;

	while (sm_ted_search (p))
	{
		replacements++;
		MODS(p)++;
		sfree (KILLBUF(p));
		KILLBUF(p) = sm_ted_buf_delete_n_chars (p, bm->patlen);
		sm_ted_buf_insert_n_chars (p, RPATTERN(p), RLENGTH(p));
		sm_ted_set_dot_relative(p, -RLENGTH(p) );
		sm_ted_damaged_win (p);
	}
	if (curvature == DISJOINT)	/* leave buffer nature as it was */
		UNDO(p).curvature = DISJOINT;

	sm_ted_win_goto_mark (p, mark_dot);
	sm_ted_buf_delete_mark_number (p, mark_dot);
	
	sm_ted_dot_record (p);

	sm_ted_damaged_buffer (p);
	return replacements;
}

void sm_ted_replace_one (Pane *p)
{
	int curvature = DISJOINT;
	BMS_private *bm = BMS(p);
	Boolean forward_replace = bm_is_forward (BMS(p));

	sm_ted_modify_callback mod_fn = BUF(p)->mod_function;

	MODS(p)++;
	
	if (UNDO(p).curvature == CONTINUOUS)
	{
		curvature = CONTINUOUS;
	}
	else
		UNDO(p).curvature = CONTINUOUS;
	
	if (forward_replace)
	{
		sfree (KILLBUF(p));
		KILLBUF(p) = sm_ted_buf_delete_nprev_chars (p, bm->patlen);
		sm_ted_buf_insert_n_chars (p, RPATTERN(p), RLENGTH(p));
		sm_ted_damaged_win_to_end (p, DOT(p) - RLENGTH(p));
		if (PREFER(p) != EOL)
			PREFER(p) += RLENGTH(p) - bm->patlen;
		sm_ted_find_dialog_dirty(p);
	}
	else
	{
		sfree (KILLBUF(p));
		KILLBUF(p) = sm_ted_buf_delete_n_chars (p, bm->patlen);
		sm_ted_buf_insert_n_chars (p, RPATTERN(p), RLENGTH(p));
		sm_ted_set_dot_relative(p, -RLENGTH(p));
		sm_ted_damaged_win_to_end (p, DOT(p));
		sm_ted_find_dialog_dirty(p);
	}
	if (curvature == DISJOINT)	/* leave buffer nature as it was */
		UNDO(p).curvature = DISJOINT;
	sm_ted_dot_record (p);

	sm_ted_damaged_dot_col(p);
	sm_ted_damaged_buffer (p);
	sm_ted_repair (p);

	if ((Generic) mod_fn != 0)
		mod_fn(OWNER_ID(p));
}

void sm_ted_find_dialog_run_find(Pane *p)
{
	find_dialog_run_find (FRD(p));	
}

void sm_ted_find_dialog_run_replace(Pane *p)
{
	find_dialog_run_replace (FRD(p));	
}

Boolean sm_ted_finder (Pane *p, aFRDia *frd, char *what, Boolean dir, Boolean case_fold)
{
	Boolean done;

	FRD(p) = frd;
	
	if (dir != bm_is_forward(BMS(p)))
		(void) bm_toggle_dir (BMS(p));
	if (case_fold != bm_is_casefolded (BMS(p)))
		(void) bm_toggle_case_fold (BMS(p));
	bm_newpattern (BMS(p), what, strlen (what) );
	
	done = sm_ted_search(p);		
	sm_ted_repair(p);
	return done;
}

void sm_ted_replacer (Pane *p, aFRDia *frd, char *what, Boolean case_fold, 
                         char *replace)
{
	FRD(p) = frd;
	
	bm_newpattern (BMS(p), what, strlen (what) );
	
	sfree (RPATTERN(p));
	RPATTERN(p) = ssave (replace);
	RLENGTH(p) = strlen (RPATTERN(p));
	
	if (case_fold != bm_is_casefolded (BMS(p)))
		(void) bm_toggle_case_fold (BMS(p));

	sm_ted_replace_one (p);
/*	sm_ted_repair(p); */ /* called as last thing in replace_one() !! */

}

int sm_ted_global_replacer(Pane *p, aFRDia *frd, char *what, Boolean global, 
                       Boolean dir, Boolean case_fold, char *replace)
{
	int i;

	FRD(p) = frd;
	
	bm_newpattern (BMS(p), what, strlen (what) );
	
	sfree (RPATTERN(p));
	RPATTERN(p) = ssave (replace);
	RLENGTH(p) = strlen (RPATTERN(p));

	if (case_fold != bm_is_casefolded (BMS(p)))
		(void) bm_toggle_case_fold (BMS(p));

	if (dir != bm_is_forward(BMS(p)))
	{
		(void) bm_toggle_dir (BMS(p));
	}
	
	if (global)
		i = sm_ted_global_replace(p);	
	else if (dir)
		i = sm_ted_forward_replace(p);
	else	i = sm_ted_backward_replace(p);
	
	find_dialog_dirty(FRD(p));
	sm_ted_repair(p);
	return i;

}

void sm_ted_find_dialog_dirty (Pane *p)
{
	find_dialog_dirty(FRD(p));
}

int sm_ted_global_replace (Pane *p)
{
	BMS_private *bm = BMS(p);
	Boolean reverse = false;
	int	curvature;
	int     mark_dot;
	int	replacements = 0;

	int	new_dot, size, growth, difference;
	char	*char_of_buffer;

	char	*buf, *newpat, *replace;
	Boolean	dir, case_fold;

	if ((mark_dot = sm_ted_buf_name_unique_mark (p, "_sm_ted_global_replace")) == -1)
	{
		sm_ted_bitch (p, "Can't name temporary mark.");
		return 0;
	}

	/* set up for the operation */
	find_dialog_get_values(FRD(p), &newpat, &replace, &dir, &case_fold);

	if (strcmp(newpat,"") == 0)
	{
		sm_ted_bitch(p, "Null search string.  Use find button to set pattern");
		return 0;
	}
	if (strcmp(newpat, bm->pattern) != 0)	/* build failure function */
		bm_newpattern(bm, newpat, strlen(newpat));
	if (strcmp(replace, RPATTERN(p)) != 0)
	{
		sfree(RPATTERN(p));
		RPATTERN(p) = ssave(replace);
		RLENGTH(p)  = strlen(RPATTERN(p));
	}
	if (case_fold != bm_is_casefolded(bm))
		(void) bm_toggle_case_fold(bm);

	/* always replace forward, from the beginning */
	if (!bm_is_forward (bm))
	{
		reverse = true;
		(void) bm_toggle_dir (bm);
	}

	size	       = flex_size(STORE(p));
	char_of_buffer = flex_get_buffer(STORE(p), 0, size, (char*)0);
	growth         = RLENGTH(p) - strlen(newpat);	/* size change */
	difference     = 0;				/* cumulative  */

	/* record current nature of buffer changes */
	sm_ted_dot_record (p);
	
	beginComputeBound();	/* set the cursor to the watch */

	curvature = UNDO(p).curvature;
	UNDO(p).curvature = CONTINUOUS;

	sm_ted_goto_bob(p);	/* start at beginning of buffer */

	new_dot = bm_search(bm, char_of_buffer, size, 0); /* find it	*/

	sm_ted_set_dot(p, new_dot);			  /* move there	*/

	while (new_dot != UNUSED)
	{
		replacements++;
		MODS(p)++;
		sfree (KILLBUF(p));
		KILLBUF(p) = sm_ted_buf_delete_nprev_chars (p, bm->patlen);
		sm_ted_buf_insert_n_chars (p, RPATTERN(p), RLENGTH(p));

		difference += growth;			/* update it	*/
		new_dot = bm_search(bm, char_of_buffer, size, new_dot);
		sm_ted_set_dot(p,new_dot+difference);
	}	
	if (reverse)			/* leave search direction as it was */
		(void) bm_toggle_dir (bm);
					/* leave buffer nature as it was */
	UNDO(p).curvature = curvature;

	sm_ted_win_goto_mark (p, mark_dot);
	sm_ted_buf_delete_mark_number (p, mark_dot);

	sm_ted_damaged_win (p);
	sm_ted_dot_record (p);

	sm_ted_damaged_buffer (p);

	sfree(char_of_buffer);

	endComputeBound();	/* and turn the cursor back */

	return replacements;
}
