/* $Id: FlexibleArray.h,v 1.5 1997/03/11 14:36:31 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef flex_h
#define flex_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

typedef enum {
	Single_hole,
	Block_list,
	Block_tree	
} FlexType;

struct BufferSingleHole {
	char *start;	/* pointer to the start of the physical buffer */
	char *endtext;	/* points to character after buffer text (ignores hole position) */
	char *hole;	/* points to the start of the hole in the buffer */
	int hole_size;	/* size of the hole in bytes */
};

typedef struct {
	Generic block_id;	/* "id" of the block (pointer to?) */
	short usage;		/* # of elements used in this block */
	short info;		/* Dirty? */
} BL_block;

struct BufferBlockList {
	BL_block *table;	/* List of blocks */
	int tablesize;		/* Number of blocks in the list */
	Generic current;	/* Block we looked at last */
	int offset;		/* Offset of that block */
};

typedef struct buffers_struct {
	FlexType type;	/* type of this Buffer */
	short	   size;
	short      length;
	union {
	struct BufferSingleHole SH;
	struct BufferBlockList	BL;
	} flex_union;
} Flex;


/* subroutines in flex.c */

EXTERN(Flex*, flex_create, (short element));
/*
 * Creates a flex array for objects of size "element".  It should be
 * freed later using flex_destroy.
 */

EXTERN(void, flex_destroy, (Flex* f));
/*
 * Frees the flex array "f".
 */

EXTERN(int, flex_size, (Flex* f));
/*
 * The size of the flex array in bytes is returned.
 */

EXTERN(int, flex_length, (Flex* f));
/*
 * The length of the flex array in elements is returned.
 */

EXTERN(int, flex_index, (Flex* f, int start, char* elt));
/*
 * Searches for the first element in "f" at or after index "start" that
 * matches "elt".  If such an element exists, its index (in the range
 *  [start..flex_size(f)-1]) is returned.
 * If no match exists at or after index "start", -1 is returned.
 */

EXTERN(int, flex1_index, (Flex* f, int start, char elt));
/*
 * A special version of flex_index for flex arrays whose element size is 1.
 * Note that "elt" is (char), rather than (char *).
 */

EXTERN(int, flex_rindex, (Flex* f, int start, char* elt));
/*
 * Searches for the first element in "f" before index "start" that
 * matches "elt".  If such an element exists, its index (in the range
 *  [0..start-1]) is returned.
 * If no match exists before index "start", -1 is returned.
 */

EXTERN(int, flex1_rindex, (Flex* f, int start, char elt));
/*
 * A special version of flex_rindex for flex arrays whose element size is 1.
 * Note that "elt" is (char), rather than (char *).
 */

EXTERN(int, flex_count_occurrences, (Flex* f, int start, int finish, char* elt));
/*
 * Returns the number of occurrences of elt in the range [start..end] of
 * the flex array f.
 */

EXTERN(int, flex1_count_occurrences, (Flex* f, int start, int finish, char elt));
/*
 * Returns the number of occurrences of elt in the range [start..end] of
 * the flex array f.
 */

/****** insert *******/

EXTERN(void, flex_insert_general, (Flex *f, int start, int n));

EXTERN(void, flex_insert, (Flex *f, int start, int n));
/* insert n undefined elements */
/*
 * Insert n undefined elements in a flex array.  This inserts space for "n"
 * elements at "start" but doesn't give them a value.
 */

EXTERN(void, flex_insert_one, (Flex *f, int start, char *elem));
/* insert element at start */
/*
 * Insert "elem" at index "index" in "f".
 */

EXTERN(void, flex_insert_buffer, (Flex *f, int start, int n, char *buf));
/* insert n elements from a buffer */
/*
 * Insert the "n" elements in "buf" into "f" at position "start".
 */

EXTERN(int, flex_insert_file, (Flex *f, int start, char *fname));
/*
 * The contents of the file specified by "filename" are inserted into "f"
 * at the point "start".  The number of bytes inserted is returned.
 */

/****** delete *******/
EXTERN(void, flex_delete, (Flex *f, int start, int n));
/* delete n elements */
/*
 * Delete from "f" the "n" elements in the region [start..start+n-1].
 */

EXTERN(char *, flex_delete_one, (Flex *f, int start, char *buf));
/* delete element at start */
/*
 * Delete the element of "f" at index "index".  The deleted element is
 * copied into "buf".  If buf is 0, then it is also allocated.  The "buf"
 * is returned.
 */

EXTERN(char *, flex_delete_buffer, (Flex *f, int start, int n, char *buf));
/*
 * Delete from "f" the elements in the region [start..start+n-1].
 * The deleted elements are copied into "buf".  If "buf" is 0, it
 * is also allocated. "buf" is returned.
 */
 
	/* delete n elements at start to buf if buf == 0, alloc buf. 
	   Return it regardless*/

/****** set *******/
EXTERN(void, flex_set_one, (Flex *f, int offset, char *elem));
/* set element at offset */
/*
 * Set the element in "f" at offset "offset" to "elem".
 */

EXTERN(void, flex_set_buffer, (Flex *f, int start, int n, char *buf));
/* set n elements at start from buf */
/*
 * Set the n elements of "f" in the range [n..n+start-1] to the elements
 * [0..n] in "buf".
 */

/***** get *****/
EXTERN(char *, flex_get_one, (Flex *f, int offset));
/* return element at offset */
/*
 * Returns a pointer to a copy of the element at offset "offset".
 * The caller is responsible for freeing the memory gotten to hold the copy.
 */

EXTERN(char, flex1_get_one, (Flex *f, int offset));
/* return element at offset */
/*
 * A version of flex_get_one for flex arrays whose element size is 1.
 * Note that it returns (char), rather than (char *).
 */

EXTERN(char *, flex_get_buffer, (Flex *f, int start, int n, char *buf));
/*
 * Get the elements of "f" in the region [start..start+n-1] into the
 * buffer "buf.  If "buf" is 0, it is allocated.  "buf" is returned.
 */

EXTERN(int, flex_get_file, (Flex *f, char *fname));
/*
 * The contents of "f" are gotten into the file specified by "filename".
 * The return value is <0 if the operation was not performed successfully.
 */

EXTERN(Flex *, flex_get_flex, (Flex *f, int start, int count));
/*
 * Return a new flex array which is a partial copy of the original flex.
 */

#endif
