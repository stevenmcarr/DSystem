/* $Id: optim_private.h,v 1.7 1997/06/25 14:58:38 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/graphicInterface/oldMonitor/include/mon/sm_def.h>
#include <libs/support/lists/list.h>
#include <libs/graphicInterface/oldMonitor/include/sms/text_sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/optim_sm.h>

#include <libs/graphicInterface/oldMonitor/monitor/sms/optim_sm/optim.inlines.h>

typedef enum {
    TRASHED,        /* for lines for which optim_bad_line has been called (implicitly UNUSED, with unknown screen contents) */
    NEEDED,         /* for lines whose screen contents is still needed for update */
    UNNEEDED,       /* for lines whose screen contents are no longer (and likely never were) needed for update */
    OLD,            /* for lines that haven't been updated or otherwise classified (perhaps unused) */
    NEW            /* for lines that have been updated already, and hence are implicitly NEEDED */
} linestatus;

/*
 * A 'osm_line' contains the information for a single line within the optim screen module.
 */
typedef struct {
    TextChar	*old;		/* the line's previous contents (what is on the screen)					*/
    TextChar	*New;		/* the line's new contents (what will be on the screen after calling sm_optim_touch)	*/
    short       ohash;		/* Hash value of oline.									*/
    short       nhash;		/* Hash value of nline.									*/
    UtilNode	olist;		/* For placing osm_line in the 'actual'  list of a hash bucket.				*/
    UtilNode	nlist;		/* For placing osm_line in the 'desired' list of a hash bucket.				*/
    short       lineno;		/* number [0 .. sm_optim_height(p)-1] of this line.  Line 0 is top line.		*/
    short       first_mod;	/* first pos (inclusive) on this line that has changed since last update		*/
    short       last_mod;	/* last  pos (inclusive) on this line that has changed since last update		*/
    linestatus  info;
    UtilNode	Class;		/* For placing this osm_line in one of several lists (picked via info) during update	*/
} osm_line;

/*
 * A 'bucket' is in fact a hash bucket.  There are two doubly-linked lists of 'osm_lines' on each hash bucket.
 * However, we are not doing the obvious overflow hash here.  We instead are doing an offset hash, and the lists are
 * used for something completely different.
 *
 * The 'ohash' values of all the lines in the 'actual' list match the 'nhash' values of  the lines in the 'desired'
 * list.  These also match the index of the hash bucket, of course.
 * All the lines in the 'actual' list have identical 'old' contents (ie. not merely identical 'ohash' values), and all
 * the lines have identical 'new' contents, which also match the 'old' contents of the lines in the 'actual' list.
 * Notice that a line that has not been changed will be a member of both the 'actual' and 'desired' lists of a
 * certain bucket.  And of course, a line that has been changed will be a member of the 'actual' list of one bucket,
 * and a member of the 'desired' list of another.
 *
 * Thus when performing an update of the screen, for a given hash bucket, all the lines in 'desired' depend on any of
 * the lines in 'actual'.  Of course, only one line in 'actual' is needed.
 */
typedef struct {
    UtilList actual;	/* List of lines whose 'ohash' values and 'old' contents have selected this hash bucket */
    UtilList desired;	/* List of lines whose 'nhash' values and 'new' contents have selected this hash bucket */
} bucket;

#define HASHRANGE 32767         /* actual_hashval, desired_hashval are held in shorts */


struct optim {
	Pane		*tp;		/* the slaved text pane							*/
	osm_line	*line;		/* an array [0 .. sm_optim_height(p)-1] of osm_lines			*/
	bucket		*ht;		/* a hash table containing lists of osm_lines				*/
	short		htsize;		/* the size of the hash table (depends on pane height)			*/
	UtilList	needed;		/* list of lines whose screen contents is needed (during update)	*/
	UtilList	unused;		/* list of lines whose screen contents is not needed (during update)	*/
	Point		old_size;	/* the size of the pane in chars after the last call to optim_resize	*/
	int		resizing;	/* non-zero iff we're resizing (inhibits touch in sm_optim_touch)	*/
	TextChar	clear;		/* a "clear", ie. background, character					*/
	Generic		region_id;	/* the client id 							*/
	Point		(*region_mover)();/* interactive region mover						*/
};


#define PARENT(p)	((p)->parent)
#define IMAGE(p)	(PARENT(p)->image)
#define INFO(p)         ((struct optim *)((p)->pane_information))

#define TP(p)			(INFO(p)->tp)
#define	LINE(p)			(INFO(p)->line)
#define HASH_TABLE(p)		(INFO(p)->ht)
#define HT_SIZE(p)		(INFO(p)->htsize)
#define OLD_SIZE(p)		(INFO(p)->old_size)
#define NEEDED_LIST(p)		(INFO(p)->needed)
#define UNUSED_LIST(p)		(INFO(p)->unused)
#define RESIZING(p)		(INFO(p)->resizing)
#define CLEAR(p)		(INFO(p)->clear)
#define	REGION_ID(p)		(INFO(p)->region_id)
#define REGION_MOVER(p)		(INFO(p)->region_mover)
#define NULL_LINE		(osm_line *)0

EXTERN(void, optim_clear_line,(register TextChar *line, register int length,
                               TextChar clear));
EXTERN(short, optim_advance,(register short start, register short stop, register
                             TextChar *old, register TextChar *New));
EXTERN(short, optim_retreat,(register short start, register short stop, register
                             TextChar *old, register TextChar *New));
extern Boolean optim_debugging;		/* set this flag to true for additional runtime checking */
