/* $Id: regex.h,v 1.5 1997/03/11 14:33:11 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/* include/items/regex.h						*/
/* regex item include file						*/
/*									*/
/* Last edited: 15 October 1991 at 5:05 pm  				*/
/*									*/
/* Regex provides a way for the user to modify a string datum 		*/
/* constrained to match a regular expression.  The data is		*/
/* always in the form of a pointer to allocated, null-terminated memory,*/
/* which may be modified by the item, but which must be initially	*/
/* allocated and later freed by the client.				*/
/* 									*/
/************************************************************************/

#ifndef items_regex_h
#define items_regex_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef dialog_h
#include <libs/graphicInterface/oldMonitor/include/mon/dialog.h>
#endif

EXTERN(DiaDesc*, item_regex, (Generic item_id, char *title,
                              short font, char **re_str_ptr, 
                              char *re_ptr, short width));
/* regular string item descriptor	*/
/* Takes six parameters:  (Generic item_id) the id of this item,	*/
/* (char *title) the text name, (short font) the font to use,		*/
/* (char **re_str_ptr) a pointer to an allocated, null-terminated string*/
/* to be edited, (char *re_ptr) a regular expression a la "man 3 regex",*/
/* and (short width) the width of the editing region in characters.	*/
/* A dialog descriptor of the item is returned.				*/


#ifdef item_h
/* The following are the functions used for the fields of the Item	*/
/* associated with item_regex().  They are made public for items and	*/
/* clients that desire to have a regex_item manage the checking of the	*/
/* RE syntax of their textual representation of their item (eg.		*/
/* item_integer).  See <item.h> for a description of these functions.	*/

EXTERN(Point, regex_item_get_size, (DiaDesc *dd));
EXTERN(void, regex_item_initialize, (DiaDesc *dd, Point ulc));
EXTERN(FocusStatus, regex_item_handle_event, (DiaDesc *dd));
EXTERN(DiaDesc*, regex_item_set_focus, (DiaDesc *dd, Boolean fs));
EXTERN(void, regex_item_modified, (DiaDesc *dd, Boolean self));
EXTERN(void, regex_item_destroy, (DiaDesc *dd));
#endif

/* more routines...							*/

typedef FUNCTION_POINTER(Boolean, item_gregex_toval_func,
 (char *text, Generic *nval, Boolean conforms));
/* receives as its first argument a string that matches the supplied	*/
/* supplied regular expression "re".  It should convert this to the	*/
/* representation of values, and store that value in nval its second	*/
/* argument.  On entry, nval is set to a value.  The third argument,	*/
/* "conforms" is true if "text" is matched by the regular expression	*/
/* supplied to item_gregex.  If the value to be returned in nval is the	*/
/* same as nval's value on entry, toVal should return false (but is not	*/
/* required to return false), signifying that nval has not changed.	*/
/* If the value of nval has changed, it must true.			*/

typedef FUNCTION_POINTER(Boolean, item_gregex_fromval_func,
 (Generic pval, char **text));
/* receives as its first argument a value that should be converted to	*/
/* its textual representation, and returned in text.  On entry, text	*/
/* is set to the printing representation of some value.  If the the	*/
/* string in text matches the printing representing of pval, then this	*/
/* function should return false (but is not required to return false),	*/
/* signifying that text has not changed.  If text is given a new value,	*/
/* then this function must return true.					*/

EXTERN(Boolean, toText, (char *otext, char ***ntext, Boolean conforms));
/* toVal for item_gregex() w/ strings	*/
EXTERN(Boolean, fromText, (char **ntext, char **otext));
/* fromVal for item_gregex() w/ strings	*/

EXTERN(DiaDesc*, item_gregex, (Generic item_id, char *title, short font, 
                               Generic val, char *re, short width, Boolean conform,
                               item_gregex_toval_func toVal, 
                               item_gregex_fromval_func fromVal));
/* general purpose RE item descriptor	*/
/* Takes nine parameters: (Generic item_id) the id of this item,	*/
/* (char *title) the text name, (short font) the font to use,		*/
/* (Generic val) the value maintained by this item, (char *re) the	*/
/* regular expression describing the text representation of values	*/
/* maintained by this item, (short width) the width in characters of	*/
/* the item, (Boolean conform) a flag indicating whether the user	*/
/* should be allowed to enter text that is not matched by the regular	*/
/* expression "re", (item_gregex_toval_func toVal) a function which	*/
/* converts a string representation of a value into that value, and	*/
/* (item_gregex_fromval_func fromVal) a function that converts a value	*/
/* into its printing representation.					*/

typedef enum {
	itemRegexError,
	itemRegexUnmatched,
	itemRegexMatched,
	itemRegexStillUnmatched,
	itemRegexStillMatched
} ItemRegexEnum;

EXTERN(ItemRegexEnum, item_regex_status, (DiaDesc *dd));
/* Query regex event	*/
/* Takes one parameter (DiaDesc *dd) the regex item to question.	*/
/* One of the members of the enumerated type above is returned,		*/
/* depending on how the most recent change to the items contents agreed	*/
/* with the regular expression for this item.				*/

EXTERN(Boolean, item_regex_set_standout, (DiaDesc *dd, Boolean emphasize));
/* highlight string if needeed	*/

#endif
