/* $Id: ted_sm.h,v 1.6 1997/03/11 14:33:25 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef ted_sm_h
#define ted_sm_h
#include <libs/graphicInterface/oldMonitor/include/sms/ted_sm_type.h>

/* Place functions here as they are fixed. */

EXTERN(short, sm_ted_get_index, (void));
/*
 *
 */


EXTERN(void, sm_ted_buf_create, (Pane *p));
/* create a fairly plain buffer */
/*
 * Creates a buffer with no contents, no buffer name, and no filename.
 * The buffer is installed in the pane information structure associated
 * with "p".
 */

EXTERN(Boolean, sm_ted_buf_destroy, (Pane *p));
/* destroy the buffer for "p" */
/*
 * Destroys the buffer associated with "p".  Returns true if the buffer could
 * be destroyed.  Returns false if it could not be destroyed.  Reasons for
 * not destroying a buffer include:
 *	Windows into the buffer still exist.
 *	The buffer is modified and is of type FILEBUFFER.
 */

EXTERN(void, sm_ted_buf_set_type, (Pane *p, sm_ted_buftype type));


EXTERN(sm_ted_buftype, sm_ted_buf_get_type, (Pane *p));

EXTERN(Generic, sm_ted_buf_get_owner, (Pane *p));


EXTERN(void, sm_ted_buf_set_owner, (Pane *p, Generic owner));


EXTERN(void, sm_ted_buf_set_bname, (Pane *p, char *bname));
/*
 * Make the name associated with the buffer for p be bname
 */

EXTERN(char *, sm_ted_buf_get_bname, (Pane *p));
/*
 * Get the name of the buffer associated with p.  The returned string should
 * not be freed or modified.
 */

EXTERN(void, sm_ted_buf_set_fname, (Pane *p, char *fname));
/*
 * Make the name of the file associated with the buffer for p be fname
 */

EXTERN(char *, sm_ted_buf_get_fname, (Pane *p));
/*
 * Get the name of the file associated with the buffer for with p.
 * The returned string should not be freed or modified.
 */

EXTERN(char *, sm_ted_buf_get_text, (Pane *p));
/*
 *  Gets (a copy of) the current buffer text.
 */

EXTERN(void, sm_ted_buf_set_text, (Pane *p, char *text, int len));
/*
 * Sets the window to be text.
 */

EXTERN(void, sm_ted_buf_use_file, (Pane *p, char *fname));


EXTERN(int, sm_ted_buf_erase, (Pane *p));
/* Erase the buffer for p */


EXTERN(Generic, sm_ted_win_get_owner, (Pane *p));


EXTERN(void, sm_ted_win_set_owner, (Pane *p));


EXTERN(void, sm_ted_win_set_style, (Pane *p, char style));
/*
 * Change the style used in the entire pane to style.
 */

EXTERN(unsigned char, sm_ted_win_get_style, (Pane *p));
/*
 * Returns the style used in the entire pane.
 */

EXTERN(void, sm_ted_win_change_font, (Pane *p, short font));
/*
 * Change the font of this pane, add damage as appropriate.
 */

EXTERN(void, sm_ted_win_active, (Pane *p, Boolean status));
/*
 * Change the active status of a window.
 * Inactive windows don't display a cursor, and pass mouse events
 * with no alteration.
 */

EXTERN(void, sm_ted_win_set_xy, (Pane *p, Point loc));
/*
 * Attempt to place the cursor at pane location loc.
 * Cursor may be placed elsewhere because it must lie
 * on text, which may not be present at loc.
 * (Region selection is not run by this call).
 */

EXTERN(Point, sm_ted_win_get_xy, (Pane *p));
/*
 * Returns the current location of the cursor in the pane.
 */

EXTERN(Point, sm_ted_size, (Pane *p));


EXTERN(Point, sm_ted_pane_size, (Point size, short font_id));


EXTERN(void, sm_ted_damaged_win, (Pane *p));


EXTERN(void, sm_ted_damaged_win_to_end, (Pane *p, int loc));


EXTERN(void, sm_ted_damaged_line_to_end, (Pane *p, int loc));


EXTERN(void, sm_ted_handle_keyboard, (Pane *p, KbChar ch));


EXTERN(void, sm_ted_repair, (Pane *p));


EXTERN(Boolean, sm_ted_search, (Pane *p));


typedef FUNCTION_POINTER(void, sm_ted_message_callback, (Pane *p, Generic id, char *mes, int sev));
/*      Pane *p   the ted_sm pane
 *      Generic id  the owner supplied id
 *      char *mes   the message
 *      short sev   the severity (0 == information, 1 == complaint)
 */

EXTERN(void, sm_ted_message_handler_initialize, (Pane *p, Generic id, sm_ted_message_callback message_handler));
/* Pane *p,  	the ted_sm pane
 * Generic id,	the id for the call back
 * sm_ted_message_callback message_handler the callback to make for a message
 */

EXTERN(Boolean, sm_ted_buf_save, (Pane *p, char *filename));
/*	Pane *p;		save the current buffer in filename, which
 *	char *filename;		may be changed using a dialog
 */

EXTERN(Boolean, sm_ted_buf_save_okay, (Pane *p, char *filename));
/*	Pane *p;		returns if the buffer is saved in filename.
 *	char *filename;
 */

EXTERN(void, sm_ted_buf_set_save_mode, (Pane *p, int mode));
/* 	Pane *p;
 *	int  mode;		one of BEHAV_SAVEIT, BEHAV_SAVEAS, BEHAV_SAVEACOPY
 */

/* The following calls are exported, but are uncommented by the author. -DGB */
EXTERN(void, sm_ted_buf_set_pname, (Pane *p, char *pname));
EXTERN(int, sm_ted_buf_written, (Pane *p));
EXTERN(Generic, sm_ted_get_buf, (Pane *p));
EXTERN(void, sm_ted_set_buf, (Pane *p, Generic buf));
EXTERN(int, sm_ted_buf_insert_file, (Pane *p));
EXTERN(int, sm_ted_buf_mark_modify, (Pane *p));
EXTERN(int, sm_ted_buf_reread_current_buffer, (Pane *p));
EXTERN(void, sm_ted_win_create, (Pane *p));
EXTERN(void, sm_ted_win_destroy, (Pane *p));
EXTERN(Generic, sm_ted_get_win, (Pane *p));
EXTERN(void, sm_ted_set_win, (Pane *p, Generic win));
EXTERN(int, sm_ted_find_dialog_create, (Pane *p));
EXTERN(Boolean, sm_ted_modified, (Pane *p));
typedef FUNCTION_POINTER(void, sm_ted_modify_callback, (Generic owner));
EXTERN(void, sm_ted_register_mod_callback, (Pane *p, sm_ted_modify_callback func));
EXTERN(int, sm_ted_search_backward, (Pane *p));
EXTERN(int, sm_ted_search_forward, (Pane *p));

#endif
