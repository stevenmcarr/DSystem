/* $Id: text_sm.h,v 1.7 1997/03/11 14:33:25 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
                /********************************************************/
                /*                                                      */
                /*                       text_sm.h                      */
                /*          The text screen module include file.	*/
                /*                                                      */
                /********************************************************/

/* The text screen module handles the display of text in a pane.  Glyph coordinates (as	*/
/* opposed to bitmap coords) are used to place a character in the pane.  It is intended	*/
/* to be used as a slave pane as it does not remember what was on the screen for a	*/
/* resize.  Batch touch mode may be nestedly entered to begin saving touches.  Ending	*/
/* batch mode will propagate those changes at one time.					*/



/* Returned events are as follows:							*/
/* EVENT_SELECT, EVENT_MOVE, EVENT_HELP events are returned with info set to the glyph	*/
/* coordinate of the event.  Events are not returned when they are outside of the map.	*/

#ifndef text_sm_h
#define text_sm_h

#define	TSM_CLIP	true			/* clipping is to be done		*/
#define	TSM_NO_CLIP	false			/* clipping is not to be done		*/


EXTERN(short, sm_text_get_index, (void));
/* get the text sm index		*/
/* Takes no parameters.  Returns the registered index for the text screen module.	*/

EXTERN(Point, sm_text_pane_size, (Point size, short font_id));
/* char to pane size conversion		*/
/* Takes two parameters:  (Point size) the size of the character block and (short font)	*/
/* the id of the font.  Returns the minimum text pane size necessary to display the	*/
/* character block.									*/

EXTERN(void, sm_text_change_font, (Pane *p, short font_id));
/* Change the default font for a pane	*/
/* Takes two parameters:  (Pane *p) the text pane and (short font_id) the new font id.	*/
/* The character map will be recalculated.  The default value is DEF_FONT_ID.		*/

EXTERN(void, sm_text_set_move_status, (Pane *p, Boolean horiz, Boolean vert));
/* Change return EVENT_MOVE status	*/
/* Takes three parameters:  (Pane *p) the text pane, (Boolean horiz, vert) the statusus	*/
/* for each direction.  A false value indicates that the character position of the down	*/
/* click is to be put in the appropriate mon_event.info position.  A true value		*/
/* indicates that the difference between the down and up click character positions is	*/
/* to be put in the appopriate mon_event.info position.  If either are true, the user	*/
/* is assisted through feedback.  The default values are both false.			*/
#define	TSM_MOVE_HORIZ		true		/* allow horizontal moves		*/
#define	TSM_NO_MOVE_HORIZ	false		/* don't allow horizontal moves		*/
#define	TSM_MOVE_VERT		true		/* allow vertical moves			*/
#define	TSM_NO_MOVE_VERT	false		/* don't allow vertical moves		*/


	/* SIZE / CONVERSION CALLS (text_size.c) */

EXTERN(Point, sm_text_size, (Pane *p));
/* Return the char map size		*/
/* Takes one parameter:  (Pane *p) the text pane.  Returns the size of the char map.	*/

EXTERN(short, sm_text_height, (Pane *p));
/* Returns the char map height		*/
/* Takes one parameter:  (Pane *p) the text pane.  Returns the height of the char. map.	*/

EXTERN(short, sm_text_width, (Pane *p));
/* Returns the char map width		*/
/* Takes one parameter:  (Pane *p) the text pane.  Returns the width of the char. map.	*/

EXTERN(Point, sm_text_char_pos_next, (Pane *p, Point pos));
/* Return the next character position	*/
/* Takes two parameters:  (Pane *p) the text pane and (Point pos) the current character	*/
/* position.  Returns the next character position.  Overflows go to zero.		*/

EXTERN(Point, sm_text_char_pos_prev, (Pane *p, Point pos));
/* Return the prev. character position	*/
/* Takes two parameters:  (Pane *p) the text pane and (Point pos) the current character	*/
/* position.  Returns the previous character position.  Underflows go to max.		*/

EXTERN(Point, sm_text_point_pg, (Pane *p, Point loc, Boolean clip));
/* Convert pixel to character coords	*/
/* Takes three parameters:  (Pane *p) the text pane, (Point loc) the pixel coordinates,	*/
/* and (Boolean clip) whether or not to clip within the character map.  Returns the	*/
/* position of the corresponding character.						*/

EXTERN(Rectangle, sm_text_rect_gp, (Pane *p, Rectangle r));
/* Convert a text rect to its pixel rect*/
/* Takes two parameters:  (Pane *p) the text pane and (Rectangle r) the text rectangle.	*/
/* Returns the corresponding pixel rectangle.						*/


	/* WRITE / MOVEMENT / CLEARING CALLS (text_blt.c) */

EXTERN(void, sm_text_pane_clear, (Pane *p));
/* Clear out the pane			*/
/* Takes one parameter:  (Pane *p) the text pane.					*/

EXTERN(void, sm_text_line_insert, (Pane *p, short lineno));
/* Insert a blank line			*/
/* Takes two parameters:  (Pane *p) the text pane and (short lineno) the line number at	*/
/* which to insert a the blank line.  Following lines are shifted down.			*/

EXTERN(void, sm_text_line_delete, (Pane *p, short lineno));
/* Delete a line			*/
/* Takes two paramters:  (Pane *p) the text pane and (short lineno) the line number to	*/
/* delete.  Following lines are shifted up.						*/

EXTERN(void, sm_text_block_clear, (Pane *p, Rectangle src));
/* Clear out a block of characters	*/
/* Takes two paramters:  (Pane *p) the text pane and (Rectangle src) the rectangle of	*/
/* characters to be cleared.								*/

EXTERN(void, sm_text_block_clear_color, (Pane *p, Rectangle src, Color fg, Color bg));
/* Clear out a block of characters	*/
/* Takes four parameters:  (Pane *p) the text pane, (Rectangle src) the rectangle of	*/
/* characters to be cleared, (Color fg) and (Color bg) the colors to clear to.		*/

EXTERN(void, sm_text_block_copy, (Pane *p, Rectangle src, Point dst_ul,
 Boolean clip));
/* Non-destructively copy a text block	*/
/* Takes four parameters:  (Pane *p) the text pane, (Rectangle src) the character	*/
/* rectangle to be copied, (Point dst) the destination character coord, (Boolean clip)	*/
/* TSM_CLIP if clipping is to be done and TSM_NO_CLIP otherwize.			*/

EXTERN(void, sm_text_lines_clear, (Pane *p, short start, short end));
/* Clear out a line group.		*/
/* Takes three parameters:  (Pane *p) the text pane and (short start, end) the beginning*/
/* and end of a list of text lines (start <= end).  The lines are cleared.		*/

EXTERN(void, sm_text_lines_copy, (Pane *p, short start, short end, short dst));
/* non-destructively copy a line group	*/
/* Takes four parameters:  (Pane *p) the text pane, (short start, end) the beginning and*/
/* ending source lines (start <= end) and (short dst) the destination line number.	*/

#define	TSM_NOW		true			/* propagate the change immediately	*/
#define	TSM_NEVER	false			/* do not propagate the change		*/

EXTERN(void, sm_text_char_put, (Pane *p, Point loc, TextChar tch, Boolean now));
/* TextChar to pane			*/
/* Takes four parameters:  (Pane *p) the text pane, (Point loc) the location of the	*/
/* write, (TextChar tch) the TextChar to write, and (Boolean now) TSM_NOW if the write	*/
/* should be propagated (TSM_NEVER otherwise).  The character must fit into the map	*/
/* boundary of the text pane.								*/

EXTERN(void, sm_text_char_put_color, (Pane *p, Point loc, TextChar tch, Boolean now, Color foreground, Color background));
/* colored TextChar to pane		*/
/* Takes six parameters:  (Pane *p) the text pane, (Point loc) the location of the	*/
/* write, (TextChar tch) the TextChar to write, (Boolean now) TSM_NOW if the write	*/
/* should be propagated (TSM_NEVER otherwise), and (Color foreground, background) the	*/
/* colors of the text and behind the text.  The character must fit into the map		*/
/* boundary of the text pane.  To use a pane's default color, use NULL_COLOR as a color	*/
/* argument.										*/

EXTERN(void, sm_text_string_put, (Pane *p, Point start, TextString ts,
 Boolean now));
/* TextString to pane			*/
/* Takes four parameters:  (Pane *p) the text pane, (Point start) the location of the	*/
/* start of the write, (TextString ts) the TextString to write, and (Boolean now)	*/
/* TSM_NOW if the write should be propagated (TSM_NEVER otherwise).  The string must fit*/
/* into the map boundary of the text pane.						*/

EXTERN(void, sm_text_string_put_color, (Pane *p, Point start, TextString ts,
 Boolean now, Color foreground, Color background));
/* colored TextString to pane		*/
/* Takes six parameters:  (Pane *p) the text pane, (Point start) the location of the	*/
/* start of the write, (TextString ts) the TextString to write, (Boolean now)		*/
/* TSM_NOW if the write should be propagated (TSM_NEVER otherwise), and (Color		*/
/* foreground, background) the colors of the text and behind the text.  The string must	*/
/* fit into the map boundary of the text pane.  To use a pane's default color, use	*/
/* NULL_COLOR as a color argument.							*/

EXTERN(void, sm_text_cursor_invert, (Pane *p, Point loc, Boolean on,Boolean now));
/* invert cursor character		*/
/* Takes four parameters:  (Pane *p) the text pane, (Point loc) the location of the	*/
/* character, (Boolean on) true if the cursor should be printed and false if it should	*/
/* be erased, and (Boolean now) TSM_NOW if the character should be propagated and	*/
/* (TSM_NEVER otherwise).  The character location must be in the map boundary.		*/


	/* PROPAGATION CALLS (text_touch.c) */

EXTERN(void, sm_text_start_batch_touch, (Pane *p));
/* begin batch touch mode		*/
/* Takes one parameter:  (Pane *p) the text pane.  In batch mode, touch calls for a pane*/
/* are saved until the end of batch mode when all touches are done at once.  Calls to	*/
/* start batch may be nested with the outermost calls doing the actual work.  Note:	*/
/* batch mode need not be used.								*/

EXTERN(void, sm_text_end_batch_touch, (Pane *p));
/* end batch touch mode		*/
/* Takes one parameter:  (Pane *p) the text pane.  Saved touches are done at once.	*/

EXTERN(void, sm_text_pane_touch, (Pane *p));
/* propagate changes in a pane		*/
/* Takes one parameter:  (Pane *p) the text pane.					*/

EXTERN(void, sm_text_half_pane_touch, (Pane *p, short l));
/* propagate changes in a half pane	*/
/* Takes two parameters:  (Pane *p) the text pane and (short l) the first line to be	*/
/* propagated.  Propagates from that line downward.					*/

EXTERN(void, sm_text_lines_touch, (Pane *p, short start, short end));
/* propagate changes in a line group	*/
/* Takes three parameters:  (Pane *p) the text pane and (short start, end) the first and*/
/* last lines to be touched.  (start <= end).						*/

EXTERN(void, sm_text_block_touch, (Pane *p, Rectangle src));
/* propagate changes in a text block	*/
/* Takes two parameters:  (Pane *p) the text pane and (Rectangle src) the block to be	*/
/* touched.										*/

EXTERN(void, sm_text_char_touch, (Pane *p, Point loc));
/* propagate a change in a character	*/
/* Takes two parameters:  (Pane *p) the text pane and (Point loc) the character position*/
/* to be touched.									*/

EXTERN(void, sm_text_pixels_touch, (Pane *p, Rectangle src));
/* propagate a change in a pane region	*/
/* Takes two parameters:  (Pane *p) the text pane and (Rectangle src) the pane relative	*/
/* region to touch.									*/

#endif
