/* $Id: keymap.h,v 1.5 1997/03/11 14:33:37 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 			keymap.h			*/
		/*     Implements a keysequence mapping abstraction.	*/
		/* 							*/
		/********************************************************/


/* This abstraction implements a mapping of sequences of KbChars to Generics.  It is	*/
/* intended to be used with keyboard input mapping, however other uses are possible.	*/
/* After a keymap is created, bindings may be added which define which sequences map to	*/
/* which outputs.  A queue of input is kept for the keymap and it may grow arbitrarily	*/
/* big.  Mappings are made from this queue and completed mappings may be queried.	*/

#ifndef keymap_h
#define keymap_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

typedef	struct  keymap	Keymap;			/* THE KEYMAP DATATYPE (hidden)		*/
#define NULL_KEYMAP	(Keymap *) 0		/* an null keymap pointer		*/

typedef	struct	kmrep	{			/* KEYMAP REPORT RETURN TYPE		*/
	KbString	seq;			/* mapping sequence so far (don't free)	*/
	Generic		info;			/* current binding at this mapping	*/
	Boolean		done;			/* is the map complete?			*/
			} Keymap_report;


	/* CREATION / DESTRUCTION CALLS */

EXTERN(Keymap *, keymap_create, (Generic unbound_info));
/* Create a new keymap			*/
/* Takes one parameter:  (Generic unbound_info) the value bound to otherwise unbound	*/
/* key sequences.  The new keymap is returned.  The created keymap should be freed with	*/
/* keymap_destroy().									*/

EXTERN(Keymap *, keymap_create_fancy, (Generic unbound_info,
 Generic default_info));
/* Create a new keymap w/printable bindg*/
/* Takes two parameters:  (Generic unbound_info) the value bound to otherwise unbound	*/
/* key sequences and (Generic default_info) the value bound to the printable characters	*/
/* as defined in <kb.h>.  This call is useful if printable characters are to be handled	*/
/* similarly.  An initialized keymap is returned with the above bindings made.  The	*/
/* keymap should be later freed with keymap_destroy().					*/

EXTERN(Keymap *, keymap_share, (Keymap *existing_map));
/* Creates a shared copy of a keymap	*/
/* Takes one parameter:  (Keymap *existing_map) the existing keymap to be shared.  The	*/
/* shared keymap is returned.  Note that all bindings made in one of a group of shared 	*/
/* keymap will appear in the other.  The keymap should be later freed with 		*/
/* keymap_destroy().									*/

EXTERN(void, keymap_destroy, (Keymap *root));
/* Destroy a keymap			*/
/* Takes one parameter:  (Keymap *root) the keymap to be destroyed.			*/


	/* INITIALIZATION CALLS */

EXTERN(void, keymap_set_push, (Keymap *map, Boolean push));
/* Set pushback value for a keymap	*/
/* Takes two parameters:  (Keymap *map) the keymap to modify and (Boolean push) the new	*/
/* pushback status value.  Pushback allows the keymap to try to map to the longest	*/
/* binding if it is possible, but will map to the next shorter binding upon error and it*/
/* will then remap the remainder.  The default value is false.				*/

EXTERN(void, keymap_set_prune, (Keymap *map, Boolean prune));
/* Set prune value for a keymap		*/
/* Takes two parameters:  (Keymap *map) the keymap to modify and (Boolean prune) the new*/
/* prune status value.  While prune is true, a new binding which is a prefix of older	*/
/* bindings will delete those older bindings.  The default value is false.		*/


	/* BINDING CALLS */

EXTERN(void, keymap_bind_KbChar, (Keymap *map, Generic info, KbChar ch));
/* Bind a single KbChar			*/
/* Takes three parameters:  (Keymap *km) the keymap in which to place the binding,	*/
/* (Generic info) the value to bind, and (KbChar ch) the KbChar to bind.  Note:  a	*/
/* binding may be made at any time.							*/

EXTERN(void, keymap_bind_KbString, (Keymap *map, Generic info, KbString ks));
/* Bind a KbString			*/
/* Takes three parameters:  (Keymap *map) the keymap in which to place the binding,	*/
/* (Generic info) the value to bind, and (KbString ks) the KbString to bind.  Note:  a	*/
/* binding may be made at any time.							*/

EXTERN(void, keymap_bind_range, (Keymap *map, Generic info, KbString prefix,
 KbChar start, KbChar end));
/* Bind a range of with common prefix	*/
/* Takes five parameters:  (Keymap *map) the keymap in which to place the binding,	*/
/* (Generic info) the value to bind, (KbString prefix) the common prefix of the range,	*/
/* (KbChar start, end) the beginning and ending of the range.  Note:  a binding may be	*/
/* made at any time.									*/


	/* INPUT QUEUE CALLS */

EXTERN(void, keymap_enqueue_KbChar, (Keymap *map, KbChar kbc));
/* Append a KbChar to the queue		*/
/* Takes two parameters:  (Keymap *map) the keymap in which to add (KbChar kbc) the     */
/* KbCharto add to the keymap's input queue.						*/

EXTERN(void, keymap_enqueue_KbString, (Keymap *map, KbString kbs));
/* Append a KbString to the queue	*/
/* Takes two parameters:  (Keymap *map) the keymap in which to add (KbString kbs).	*/

EXTERN(Boolean, keymap_queue_empty, (Keymap *map));
/* Check to see if input queue is empty	*/
/* Takes one parameter:  (Keymap *map) the keymap to check.  Returns true if there are	*/
/* unprocessed characters on the input queue.						*/

EXTERN(void, keymap_queue_flush, (Keymap *map));
/* Forcibly empty the input queue	*/
/* Takes one parameter:  (Keymap *map) the keymap to flush.  Does not affect the current*/
/* mapping.										*/


	/* MAPPING CALLS */

EXTERN(void, keymap_reset, (Keymap *root));
/* flush the current mapping		*/
/* Takes one parameter:  (Keymap *root) the keymap to be reset.  This call discards the	*/
/* current mapping and should be used to put the queue into a known state.  Does not	*/
/* affect the input queue.								*/

EXTERN(Boolean, keymap_mapping_complete, (Keymap *map));
/* checks current mapping		*/
/* Takes one parameter:  (Keymap *map) the keymap to check.  Returns true if there is a	*/
/* complete (meaningful) mapping.							*/
/* Notes:  This call may be made any number of times after a mapping has been completed.*/
/* A completed mapping is discarded when this call is made after one or more "queries"	*/
/* (below) have been made on the completed mapping.					*/

EXTERN(Boolean, keymap_mapping_extend_auto, (Keymap *map));
/* try to extend a partial mapping	*/
/* Takes one parameter:  (Keymap *map) the keymap to extend.  Returns true if any       */
/* extension occurs and false if a unique path could not be made into the mapping tree.	*/
/* To determine if the extension actually completed a mapping, it is necessary to call	*/
/* keymap_mapping_complete().								*/

EXTERN(Boolean, keymap_retract, (Keymap *map));
/* deletes the last character mapped	*/
/* Takes one parameter:  (Keymap *map) the keymap to manipulate.  Tries to delete the   */
/* most recent character mapped.  Returns its success.					*/


	/* INQUIRIES */

EXTERN(Keymap_report, keymap_report_all, (Keymap *map));
/* get all public info for keymap	*/
/* Takes one parameter: (Keymap *map) the keymap to get info about. Returns a structure	*/
/* containing the info.  Free the non-ephemeral "seq" field.  Can be called at anytime.	*/

EXTERN(Keymap_report, keymap_get_binding, (Keymap *map, KbString kbs));
/* get info about a specified binding	*/
/* Takes two parameters: (Keymap *map) the keymap in which to look for the binding, and	*/
/* (KbString kbs) the string for which we are looking up the binding.  Returns a	*/
/* structure containing the public info about the binding.  Free the non-ephemeral	*/
/* "seq" field after use.  Can be called at anytime.					*/

EXTERN(short, keymap_seq_len, (Keymap *map));
/* get the length of the current mapping*/
/* Takes one parameter:  (Keymap *map) the keymap of interest.  Returns the number of	*/
/* KbChars mapped so far.  May be called at any time.					*/

EXTERN(KbString, keymap_seq_KbString, (Keymap *map));
/* get the current mapping		*/
/* Takes one parameter:  (Keymap *map) the keymap of interest.  Returns the KbString	*/
/* mapped so far.  Returns an ephemeral KbString which must be freed with freeKbString()*/
/* mapped.  This call is most useful just after a mapping has been completed.		*/

EXTERN(Generic, keymap_info, (Keymap *map));
/* get the current mapping binding	*/
/* Takes one parameter:  (Keymap *km) the keymap of interest.  Returns the binding at	*/
/* the current mapping.  This call is most useful just after a mapping has been		*/
/* completed.										*/

	/* WALKING THE BINDINGS TREE */

struct KeymapWalkStruct;
typedef FUNCTION_POINTER(int, keymap_walk_callback, (struct KeymapWalkStruct *kw));

typedef struct KeymapWalkStruct {
	Generic	handle;			/* Available for client's use.				*/
	keymap_walk_callback
		interior_handler;	/* Function to call when visiting interior nodes	*/
	keymap_walk_callback
		leaf_handler;		/* Function to call when visiting single leaf nodes	*/
	keymap_walk_callback
		leaves_handler;		/* Function to call when visiting range leaf nodes	*/
	int	walk_control;		/* See below						*/
	Generic	binding;		/* Binding at the current node				*/
	int	depth;			/* Depth of the current node [1..n]			*/
	KbString low;			/* Lowest KbString represented at current node		*/
	KbString high;			/* Highest KbString represented at current node		*/
	Keymap	*priv;			/* Private to the keymap walker.  Do not touch.		*/
} *KeymapWalk;

/* Parameters that control tree walking */
#define TW_BEFORE	0x010	/* Visit interior node before traversing children */
#define TW_AFTER	0x008	/* Visit interior node after traversing children */
#define	TW_LEAF		0x004	/* Visit leaf nodes */
#define	TW_DESCEND	0x002	/* Traverse children */
#define TW_QUIT_SIBS	0x001	/* Don't traverse remaining siblings of a node */

/* Some handy predefined tree walk patterns */
#define TW_EXIT		(TW_QUIT_SIBS)
#define	TW_LEAVES	(TW_LEAF | TW_DESCEND)
#define TW_PATH		(TW_AFTER | TW_QUIT_SIBS)
#define	TW_POSTORDER	(TW_AFTER | TW_LEAF | TW_DESCEND)
#define TW_SIBLINGS	(TW_BEFORE | TW_LEAF)
#define TW_PREORDER	(TW_BEFORE | TW_LEAF | TW_DESCEND)
#define TW_FULLORDER	(TW_BEFORE | TW_AFTER | TW_LEAF | TW_DESCEND)

/*
The following is a full listing of the walking patterns produced by various
settings of the "walk_control" field.  Note that this field can be changed
while the tree is being walked; only using this technique can some of the
described effects be produced.

B A L D Q
e f e e u
f t a s i
o e f c t
r r   e S
e     n i
      d b
        s

F F F F F	+	Exit (after walking but not visiting siblings)
F F F F T	#	Exit
F F F T F	+	Walk tree without visiting nodes
F F F T T	+	Silly
F F T F F	#	if at leaf
				then visit siblings
			FFFFF
F F T F T	+	Exit
F F T T F		Visit leaves only
F F T T T	+	Exit
F T F F F	#	Postorder visit of interior nodes on path to root, and their remaining siblings
F T F F T	#	Path to root
F T F T F		Postorder visit of interior nodes
F T F T T	+	Path to root
F T T F F	#	Postorder visit of nodes on path to root (including current), and their remaining siblings
F T T F T	+	FTFFT
F T T T F		Postorder visit
F T T T T	+	Silly
T F F F F	#	visit remaining siblings that are interior nodes
T F F F T	+	Exit (like FFFFT, but silly)
T F F T F		Preorder visit of interior nodes
T F F T T	+	Silly
T F T F F	#	Visit remaining siblings
T F T F T	+	Like TFFFT, but sillier
T F T T F		Preorder visit
T F T T T	+	Silly
T T F F F	#	Like FTFFF, but remaining siblings are visited twice each
T T F F T	+	Like FTFFT, but silly
T T F T F		Fullorder visit of interior nodes
T T F T T	+	Silly
T T T F F	#	Like FTTFF, but remaining siblings of interior nodes are visited twice each
T T T F T	+	Like FTTTT, but silly
T T T T F		Fullorder visit
T T T T T	+	Silly


Notes:
	#	This really makes sense only when used *during* a treewalk
	+	Non-sensical, or a silly duplicate of another.  Don't use.

*/

EXTERN(int, keymap_walk_noop, (KeymapWalk kw));
/*
 * The default handler for keymap walking, installed by keymap_walk_create().
 * It does nothing at the current node, and returns 0, which tells the
 * walker to keep walking.
 */

EXTERN(KeymapWalk, keymap_walk_create, (Keymap *map));
/*
 * Allocates a KeymapWalk structure and initializes all its fields.
 * The keymap_walk_noop is installed for all three handlers.
 * do_parent_before, do_parent_after, do_leaves, and do_children are all set to
 * true.  The client's handle is set to UNUSED.
 */

EXTERN(int, keymap_walk_begin, (KeymapWalk kw));
/*
 */

EXTERN(void, keymap_walk_destroy, (KeymapWalk kw));


/* routines for debugging */

EXTERN(void, keymap_dump_bindings, (Keymap *map, PFS getbindingname,
                                     Boolean do_unbound, Boolean do_internal));

EXTERN(KbString, keymap_get_queue, (Keymap *map));

EXTERN(char, *keymap_queue_debug, (Keymap *map));

#endif
