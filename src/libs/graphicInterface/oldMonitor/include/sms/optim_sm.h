/* $Id: optim_sm.h,v 1.8 1997/03/11 14:33:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 		       optim_sm.h			*/
		/*       Optimal update screen module include file.	*/
		/* 							*/
		/********************************************************/

/* This screen module implements fixed width fonts in a pane.  Optimized updating	*/
/* in the style of Curses or SUM (see Rn newsletter #24) is provided.			*/
/* Small windows will probably find the overhead (time and space) of this module	*/
/* excessive, and will probably want something fast and lean.  This module attempts to	*/
/* satisfy the needs of applications displaying large windows of text which are changed	*/
/* often in complicated or non-trivial ways (eg. the Rn fortran editor).		*/

#ifndef optim_sm_h
#define optim_sm_h

EXTERN(void, optim_free, (Pane *p));

EXTERN(short, sm_optim_get_index, (void));
/* get the optim sm index			*/
/* Takes no parameters.
 * Returns the registered index for the optim screen module.
 */

EXTERN(Point, sm_optim_pane_size, (Point size, short font_id));
/* char to pane size conversion		*/
/*	Point	size;		The size in characters of the desired pane
 *	short	font_id;	The font with which the calculations will be done
 * Returns the minimum size pane necessary to display a block of characters of size
 * 'size' in font 'font_id'
 */

EXTERN(short, sm_optim_width, (Pane *p));
/* get the width of the pane in characters	*/
/*	Pane *p;
 * Returns the width in characters of `p'
 */

EXTERN(short, sm_optim_height, (Pane *p));
/* get the height of the pane in characters	*/
/*	Pane *p;
 * Returns the height in characters of `p'
 */

EXTERN(Point, sm_optim_size, (Pane *p));
/* get the size of the pane in characters	*/
/*	Pane *p;
 * Returns the size in characters of `p'
 */


#define SM_OPTIM_ACTUAL		true
#define SM_OPTIM_DESIRED	false

EXTERN(TextChar, sm_optim_getchar, (Pane *p, Point loc, Boolean actual));
/* read a TextChar from the pane map		*/
/*	Pane	*p;		The pane from which to get the character.
 *	Point	loc;		The location from which to get the character.
 *	Boolean	actual;		(one of SM_OPTIM_ACTUAL,SM_OPTIM_DESIRED)
 *
 * Returns the TextChar at position 'loc'.  If 'actual' is true the
 * actual contents of the pane at 'loc' are returned.  Otherwise, the current
 * contents of the screen map (the desired screen) are returned.
 */

EXTERN(void, sm_optim_resizing, (Pane *p, Boolean am_resizing));
/* tell optim that we're currently resizing	*/
/*	Pane *p;
 *	Boolean am_resizing;
 *
 * This call is useful for inhibiting actual touching of the pane "p", which is a
 * desirable thing during resizing.  Resizing code of master screen modules should
 * be bracketed with calls before and after the code.  The call before should have
 * "am_resizing" set to true, and the call after should have "am_resizing" set to
 * false, indicating that later touches to the screen should actually occur.
 */


EXTERN(void, sm_optim_change_font, (Pane *p, short font_id));
/* change the font for this pane		*/
/*	Pane*   p               The pane affected.
 *	short	font_id		The new font id.
 * Change the family of fonts that will be available in the pane `p'.
 * This routine is typically called once, early, when setting up a pane.  It
 * calculates the number of glyphs that will fit in a row/col, and calculates where
 * they will be placed.  The previous screen contents, if any, are erased.
 */

EXTERN(void, sm_optim_set_move_status, (Pane *p, Boolean horiz, Boolean vert));
/* set MOVE_EVENT return status		*/
/* 	Pane*	p		The pane affected.
 *	Boolean horiz, vert	the move status.
 * A false value indicates that the character position of the down
 * click is to be put in the appropriate mon_event.info position.  A true value
 * indicates that the difference between the down and up click character positions is
 * to be put in the appopriate mon_event.info position.  If either are true, the user
 * is assisted through feedback.  The default values are both false.
 */

EXTERN(void, sm_optim_clear, (Pane *p));
/* clear the desired screen			*/
/*	Pane *p;
 * Clears the map of the desired screen.
 */

EXTERN(void, optim_clear_actual, (Pane *p));

EXTERN(void, sm_optim_clear_eol, (Pane *p, Point loc));
/* clear a line of the desired screen		*/
/*	Pane *p;
 *	Point loc;
 * The the line containing `loc' is cleared from the column containing
 * `loc' (inclusive) to the right margin.  The clear is performed on
 * the map of the desired screen, so no changes occur to the contents of
 * the pane `p' until a touch is performed.
 */

EXTERN(void, sm_optim_putchar, (Pane *p, Point loc, TextChar tch));
/* place a character on the desired screen	*/
/*	Pane *p;	The pane in which to draw the character.
 *	Point loc;	The place to put the character
 *	TextChar tch;	the TextChar to write
 * This routine draws a single character on its map of the desired screen.
 * No change will appear on the screen until either `sm_optim_touch' is called,
 * or `sm_optim_touch_line' is called with `loc.y' as its second argument.
 */

EXTERN(void, sm_optim_putstr, (Pane *p, Point loc, TextString ts));
/* write a string on the desired screen		*/
/*	Pane		*p;
 *	Point		loc;
 *	TextString	ts;
 * Similar to `sm_optim_putchar', except that the string of characters `ts'
 * is written to the screen map, instead of a single character.
 *
 * The characters are placed in the screen map starting at `loc', and
 * advancing to the right, in a fairly normal way.  No wrapping
 * is performed if `ts' is too long to fit between `loc' and the right
 * edge of the pane `p'.  Instead, the string is clipped at the right
 * boundary of the pane.
 */


EXTERN(TextChar *, sm_optim_alloc_line, (Pane *p));
/*	Pane *p;
 *
 * Get a cleared array of TextChars of the width of the screen, for use
 * with sm_optim_putline().  The array should be freed via
 * sm_optim_free_line().
 */

EXTERN(void, sm_optim_free_line, (TextChar *line));
/*	TextChar *line;
 *
 * This routine frees a line that was obtained from
 * sm_optim_alloc_line().
 */

EXTERN(TextChar *, sm_optim_putline, (Pane *p, short row, TextChar *ts));
/* write a line by swapping TextChar arrays 	*/
/*	Pane		*p;	The pane.
 *	short		row;	The row that is being put.
 *	TextChar	*ts;	An array of TextChars as big as the pane is wide.
 *
 * Write an array of TextChars to the desrired screen by swapping it
 * with a line of the optim layer's map of the desired screen.  After
 * this call ts is owned by the optim screen module.  The array ts
 * should have been obtained from sm_optim_alloc_line().  A similar
 * array is returned by this routine, which becomes the property of the
 * caller, and should be freed via sm_optim_free_line() if it is
 * unneeded.  The returned array has already been cleared, so
 * sm_optim_clear_eol() is not necessary in conjunction with this
 * routine.
 */

EXTERN(void, sm_optim_touch_line, (Pane *p, short lineno));
/* propagate changes to a line to the screen	*/
/*	Pane *p;
 *	short lineno;
 * The contents of the pane `p' are altered to reflect the changes
 * made to the map of the desired screen for row `lineno'.
 * This routine should probably be 
 * used whenever small changes are made to a single line
 * (eg. character deletion in a text editor), since `sm_optim_touch'
 * will probably prove to be too slow.
 */

EXTERN(void, sm_optim_touch, (Pane *p));
/* propagate all changes to the screen		*/
/*	Pane *p;
 * The contents of the pane `p' are altered to reflect the differences
 * between the map of the desired screen and the map of the actual
 * screen.  This typically consists of all the changes since the last
 * call to `sm_optim_touch'.
 */


EXTERN(void, sm_optim_badline, (Pane *p, short lineno));
/* mark a line's actual contents unusable	*/
/*	Pane	*p;
 *	short	lineno;
 * Calling this function informs the update module that the
 * contents of line `lineno' in the pane `p' has been changed,
 * so that the update module won't make incorrect assumptions
 * about the panes's contents later when an update is requested.
 */

EXTERN(void, sm_optim_badscreen, (Pane *p));
/* mark the actual screen's contents unusable	*/
/*	Pane *p;
 * Similar to `optim_bad_line'.  This function tells the update
 * module to invalidate its entire map of the pane `p', because
 * the contents of the pane were changed outside of the control
 * of this module.
 */

EXTERN(void, sm_optim_refresh, (Pane *p));
/* redraw pane 'p' fully, no shortcuts		*/
/*	Pane *p;
 * This function redisplays the pane `p', making no assumptions
 * about the contents of the pane.  It is appropriate to use
 * for a full redisplay, eg. after the screen is somehow trashed.
 */

#endif
