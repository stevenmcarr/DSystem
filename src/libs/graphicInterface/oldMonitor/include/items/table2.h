/* $Id: table2.h,v 1.4 1997/03/11 14:33:12 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*			        table2.h				*/
/*			 table item include file			*/
/*									*/
/************************************************************************/

#ifndef items_table2_h
#define items_table2_h

typedef FUNCTION_POINTER(char *, item_table2_entry_func,
 (Generic client, int i, int j));

EXTERN(DiaDesc *, item_table2, (Generic item_id, short font,
 Point *numentries, int *item_width, item_table2_entry_func entry,
 Generic client, Point *selection, Point *changed_item, Boolean *redraw_table,
 Boolean *shift_table, Point size));
/*
Create a dialog descriptor for a 2 dimensional grid of strings with scrolling
in both dimensions available via scroll bars.  The strings are provided via
a callback so that they don't have to be computed unless they are being
displayed.

The numerous parameters have the following types and meanings:

  Generic        item_id;	-- the client's handle to the item
  short          font;		-- the font to use for displaying the strings
  Point         *numentries;	-- the number of entries in the whole grid
  int		*item_width;	-- (ptr to) width (in # of chars) of each item
					Note: in addition to specifying the
					original width of the strings, this
					parameter can also be used to cause
					a change in the element width--
					If *item_width is modified and
					then the item is notified
					    (via dialog_item_modified())
					the entire table will be redrawn using
					the new width for each string.

					Note 2: strings that are longer than
					*item_width will be truncated.
  item_table2_entry_func
                 entry;		-- the client-provided function that returns a
					table entry;  it should look like:

						char *entry( client, i, j )
						Generic client;
						int i, j;

					where "client" is the next parameter.
					The function should return the string
					to be put in the <i,j> location of
					the (entire) table.
  Generic	 client;	-- the handle to be passed back to "entry"
					(an instance variable works very well)
  Point		*selection;	-- (ptr to) the indices of string that
					is (or should be) highlighted
					If the client changes *selection and
					then notifies the item--the table
					will be redisplayed.
  Point		*changed_item;	--  (ptr to) the indices of string that has
					changed--only that elt is redisplayed
  Boolean	*redraw_table;	-- whether entire table should be redisplayed
					completely when the item is notified
					of a modification
  					- perhaps because strings have all
					  been changed
  Boolean	*shift_table;	-- whether the table should be redisplayed
					around the selected element when
					the item is notified of a mod.
  Point		 size;		-- the number of entries to display initially
					(it may change if item_width is
					 changed.  The pixel size of the
					 table won't change, however.)
*/

#endif
