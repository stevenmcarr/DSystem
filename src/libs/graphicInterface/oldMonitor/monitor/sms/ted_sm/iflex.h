/* $Id: iflex.h,v 1.5 1997/03/11 14:34:12 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/support/arrays/FlexibleArray.h>
#include <libs/support/stacks/xstack.h>
#include <libs/graphicInterface/oldMonitor/monitor/sms/ted_sm/undo.h>

/* subroutines in iflex.c */

/****** insert *******/
EXTERN(void, iflex_insert,(Stack h, Undo *u, Flex *f, int start, int n));	
/* insert n  undefined elements */
/*	Flex *f;		a flex array
 *	int start;		place to begin inserting
 *	int n;			number of elements to insert
 *
 * Insert n undefined elements in a flex array.  This inserts space for "n"
 * elements at "start" but doesn't give them a value.
 */

EXTERN(void, iflex_insert_one,(Stack h, Undo *u, Flex *f, int start, char *elem));
/* insert element at start */
/*	Flex *f;
 *	int index;
 *	char *elem;
 *
 * Insert "elem" at index "index" in "f".
 */

EXTERN(void, iflex_insert_buffer,(Stack h, Undo *u, Flex *f, int start, int n,
                                  char *buf)); 	
/* insert n elements from a buffer */
/*	Flex *f;
 *	int start;
 *	int n;
 *	char *buf;
 *
 * Insert the "n" elements in "buf" into "f" at position "start".
 */

EXTERN(int, iflex_insert_file,(Stack h, Undo *u, Flex *f, int start, char *fname));
/*	Flex *f;
 *	int start;
 *	char *filename;
 *
 * The contents of the file specified by "filename" are inserted into "f"
 * at the point "start".  The number of bytes inserted is returned.
 */

/****** delete *******/
EXTERN(void, iflex_delete,(Stack h, Undo *u, Flex *f, int start, int n));
  		/* delete n elements */
/*	Flex *f;
 *	int start;
 *	int n;
 *	char *buf;
 *
 * Delete from "f" the "n" elements in the region [start..start+n-1].
 * The deleted elements are copied to "buf".  If "buf" is 0, then it
 * is also allocated.  "buf" is returned.
 */

EXTERN(char, *iflex_delete_one,(Stack h, Undo *u, Flex *f, int start, char *buf));
 	/* delete element at start */
/*	Flex *f;
 *	int index;
 *	char *buf;
 *
 * Delete the element of "f" at index "index".  The deleted element is
 * copied into "buf".  If buf is 0, then it is also allocated.  The "buf"
 * is returned.
 */

EXTERN(char, *iflex_delete_buffer,(Stack h, Undo *u, Flex *f, int start, int n,
                                   char *buf));
/*	Flex *f;
 *	int start;
 *	int n;
 *	char *buf;
 *
 * Delete from "f" the elements in the region [start..start+n-1].
 * The deleted elements are copied into "buf".  If "buf" is 0, it
 * is also allocated. "buf" is returned.
 */
 
	/* delete n elements at start to buf if buf == 0, alloc buf. 
	   Return it regardless*/

/****** set *******/
EXTERN(void, iflex_set_one,(Stack h, Undo *u, Flex *f, int offset, char *elem));
/* set element at offset */
/*	Flex *f;
 *	int offset;
 *	char *elt;
 *
 * Set the element in "f" at offset "offset" to "elt".
 */

EXTERN(void, iflex_set_buffer,(Stack h, Undo *u, Flex *f, int start, 
                               int n, char *buf)); 
/* set n elements at start from buf */
/*	Flex *f;
 *	int start;
 *	int n;
 *	char *buf;
 *
 * Set the n elements of "f" in the range [n..n+start-1] to the elements
 * [0..n] in "buf".
 */
