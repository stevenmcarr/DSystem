/* $Id: optim_update.C,v 1.1 1997/06/25 14:58:38 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <stdlib.h>
#include <string.h>

#include <include/bstring.h>

#include <libs/graphicInterface/oldMonitor/monitor/sms/optim_sm/optim_private.h>

void hash_enter_actual (register Pane *p, register osm_line *line);

static osm_line *
line_of(register UtilNode *node)
{
    if (node != NULLNODE)
        return (osm_line *)util_node_atom(node);
    else
        return (osm_line *)NULL_LINE;
}


/*
 *  optim_hash -- a hash function. This one takes an array of shorts,
 *  the size of the array, and the size of the hash table as
 *  arguments.
 */ 
static short
optim_hash(register short *short_array, register short asize, register short hsize)
{
    register unsigned int result = 0;

    while( --asize >= 0 )
    {
        result += *short_array++;
        result = (result << 5) + (result >> 27);
    }

    return result % hsize;
}

#ifdef unused

static short
optim_rehash(short hsize, short prevhash)
{
    /* Of course its silly.  Hope 13 is a lucky number! */
    return (prevhash + 13) % hsize;
}

#endif /* unused */

/*
 *  same_line -- returns true iff the two lines (arrays of TextChar, ie. screen contents) are the same
 */
static Boolean
same_line(Pane *p, register TextChar *line_1, register TextChar *line_2)
{
    register int width = sm_optim_width( p );
    register int i;

    for (i=0; i<width;i++)
    {
        if (line_1->ch != line_2->ch || line_1->style != line_2->style)
            return false;
        line_1++;
        line_2++;
    }

    return true;
}

/*
 * hash_collide - returns true if "line" has collided with "bucklist"
 *      That is to say, "bucklist" is a list of lines that are all the same.
 *      If "line" doesn't match the existing lines in "bucklist", we have a collision.
 *      If "line does match, or if the list is empty, no collision.
 */
static Boolean
hash_collide(register Pane *p, register TextChar *line, register bucket *buck)
{
    register UtilNode *first;

    /*
     * See if line matches existing lines (if any) in the actual list of `buck'
     */
    first = util_head( &(buck->actual) );
    if (first != NULLNODE)
    {
        if (NOT( same_line(p,line,line_of(first)->old) ))
            return true;
    }

    /*
     * See if line matches existing lines (if any) in the desired list of `buck'
     */
    first = util_head( &(buck->desired) );
    if (first != NULLNODE)
    {
        if (NOT( same_line(p,line,line_of(first)->New) ))
            return true;
    }

    return false;
}

static
int hash_find(register Pane *p, register TextChar *line)
{
    register short width = sm_optim_width( p );
    register short hash;
    register bucket *buck;

    hash = optim_hash( (short *)line, width, HT_SIZE(p) );
    buck = &(HASH_TABLE(p)[hash]);

    /*
     * compute new hash values until we don't collide with either of the
     *      members of the two lists hanging off each hash bucket
     */
    while ( hash_collide( p, line, buck ) )
    {
        hash = (hash+13) % HT_SIZE(p);
        buck = &(HASH_TABLE(p)[hash]);
    }

    return hash;
}

static
void hash_enter_desired(register Pane *p, register osm_line *line)
{
    line->nhash = hash_find( p, line->New );
    util_push( &(line->nlist), &(HASH_TABLE(p)[line->nhash].desired) );
}


void hash_enter_actual(register Pane *p, register osm_line *line)
{
    line->ohash = hash_find( p, line->old );
    util_push( &(line->olist), &(HASH_TABLE(p)[line->ohash].actual) );
}


static
void hash_remove_desired(register osm_line *line)
{
    /*
     * remove this line from the list "HASH_TABLE(p)[thisline->nhash].desired",
     * which is the bucket for the desired line contents of this line.
     */
    util_pluck( &(line->nlist) );
}

static
void hash_remove_actual(register osm_line *line)
{
    /*
     * remove this line from the list "HASH_TABLE(p)[thisline->ohash].actual",
     * which is the bucket for the actual line contents of this line.
     */
    util_pluck( &(line->olist) );
}


void
sm_optim_badline(register Pane *p, register short lineno)
{
    register osm_line *line = &(LINE(p)[lineno]);

    if (line->info == TRASHED)
            return;

    line->info = TRASHED;
#ifdef notyet
    hash_remove_actual( line );
#endif
}


void
sm_optim_badscreen(register Pane *p)
{
    register short height = sm_optim_height( p );
    register short i;

    for (i=0;i<height;i++)
        sm_optim_badline(p,i);
}


/*
 *  optim_update_line_dumb - updates a line.
 *      assumes that the screen contents for this line are trashed
 */
static Rectangle
optim_update_line_dumb(register Pane *p, register short lineno)
{
    register osm_line *line = &(LINE(p)[lineno]);
    register short width = sm_optim_width( p );
             Rectangle affected;
             TextString ts;

    ts.tc_ptr = line->New;
    ts.num_tc = width;
    ts.ephemeral = false;

    sm_text_string_put( TP(p), makePoint(0,lineno), ts, TSM_NEVER);

    bcopy( (char *)line->New, (char *)line->old, width*sizeof(TextChar) );

    affected = makeRect(makePoint(0,lineno),makePoint(width-1,lineno));

    line->last_mod  = 0; 
    line->first_mod = width - 1;

    return affected;
}


/*
 *  optim_update_line_smart -- updates a line.
 *      Tests to see how much of the new line is the same as the old.
 *      Returns the rect describing what parts of the screen were
 *      actually changed as a result of this call.
 */
static Rectangle
optim_update_line_smart(register Pane *p, register short lineno)
{
    register osm_line *line = &(LINE(p)[lineno]);
    register short first = line->first_mod;
    register short last  = line->last_mod;
             Rectangle affected;
    register short width = sm_optim_width( p );

    if (first <= last && width != 0)
    {
	TextString ts;

	ts.tc_ptr = &(line->New[first]);
	ts.num_tc = last-first+1;
	ts.ephemeral = false;

	sm_text_string_put( TP(p), makePoint(first,lineno), ts, TSM_NEVER);

	bcopy( (char *)(line->New+first), (char *)(line->old+first), (last-first+1)*sizeof(TextChar) );

	affected = makeRect( makePoint(first,lineno), makePoint(last,lineno) );
    }
    else
    {
	affected = makeRect(Origin,Origin);
    }

    line->last_mod  = 0; 
    line->first_mod = width - 1;

    return affected;
}

/*
 * sm_optim_touch_line - propagate to the screen all changes to a single line since last touch
 */
void
sm_optim_touch_line(register Pane *p, register short lineno)
{
    register osm_line *line = &(LINE(p)[lineno]);
             Rectangle affected;

    switch (line->info)
    {
        case OLD:
        case UNNEEDED:
        case NEEDED:
        case NEW:
	    /* When we are through, the old actual line will no longer exist */
#ifdef notyet
	    hash_remove_actual( line );
#endif
	    /*
	     * Squeeze the first and last modified indices inward, marking the changed infix of the line.
	     */
	    line->first_mod = optim_advance(line->first_mod,line->last_mod, line->old,line->New);
	    line->last_mod  = optim_retreat(line->last_mod, line->first_mod,line->old,line->New);

            affected = optim_update_line_smart( p, lineno );
            break;
        case TRASHED:
	    /* Since the line is trashed, it's not hashed as an actual line */
            affected = optim_update_line_dumb( p, lineno );
            break;
    }

    /* enter the new actual line */
#ifdef notyet
    hash_enter_actual( p, line );
#endif

    /* If we're resizing, the whole pane will be touched for us */
    if (! RESIZING(p) )
	sm_text_block_touch(TP(p),affected);

    line->info = OLD;
}

static linestatus
classify(register Pane *p, register osm_line *line)
{
    register bucket *buck;

    if (line->info == TRASHED)
        return line->info;

    buck = &(HASH_TABLE(p)[line->ohash]);
    if ( util_list_empty(&(buck->desired)) )
        return UNNEEDED;
    else
        return NEEDED;
}

static void fix_line(register Pane *p, register osm_line *src, register osm_line *dst)
{
             Rectangle affected;

    if (src == NULL_LINE)
    {
	    switch (dst->info)
	    {
	        case UNNEEDED:
        	case NEEDED:
	            affected = optim_update_line_smart( p, dst->lineno );

		    /* If we're resizing, the whole pane will be touched for us */
		    if (! RESIZING(p))
		    	if (positiveArea(affected))
			    sm_text_block_touch(TP(p),affected);
		    dst->info = OLD;
        	    break;

	        case TRASHED:
        	    (void) optim_update_line_dumb( p, dst->lineno );

		    dst->info = NEW;
	            break;

		/* Never legal during full-screen update? */
        	case OLD:
	        case NEW:
		default:
		    abort();
	    }
    }
    else
    {
        sm_text_lines_copy( TP(p), src->lineno, src->lineno, dst->lineno );

	dst->info = NEW;
    }
#ifdef notyet
    hash_remove_desired( dst );
    hash_enter_actual( p, dst );
#endif
}

/*
 *  sm_optim_touch -- update the screen, the best way we know how
 */
void
sm_optim_touch(Pane *p)
{
             Point size;
    register osm_line *line;
    register short i;

    size = sm_optim_size( p );

    /*
     * for each line of the desired screen, hash the line, and enter
     * it in the desired half of the hash table.
     */
    for (i=0;i<size.y;i++)
    {
	line = &(LINE(p)[i]);

	/*
	 * Squeeze the first and last modified indices inward, marking the changed infix of the line.
	 */
	line->first_mod = optim_advance(line->first_mod,line->last_mod, line->old,line->New);
	line->last_mod  = optim_retreat(line->last_mod, line->first_mod,line->old,line->New);

	/* do a quick check to see if the screen contents are unchanged for this line */
        if ( line->first_mod > line->last_mod && line->info != TRASHED)
	{
            line->info = OLD;
	}
	else
	{
#ifdef notyet
	        hash_enter_desired( p, line );
#endif
	}

    }

    for (i=size.y - 1;i>=0;i--)
    {
        line = &(LINE(p)[i]);

        switch( line->info = classify(p,line) )
        {
            case NEEDED:    /* actual contents of screen for this line are needed for update */
                util_push( &(line->Class), &NEEDED_LIST(p) );
                break;
            
            case UNNEEDED:  /* actual contents of screen for this line are not used in update */
            case TRASHED:   /* bad_line was called previously for this line (screen contents unknown) */
                util_push( &(line->Class), &UNUSED_LIST(p) );
                break;

            default:;
        }
    }

    sm_text_start_batch_touch(TP(p));

   /* update the remaining entries in the desired half of the hash table */
    /* This code is currently kludged, and looks nothing like what we envision */
    line = line_of( util_pop( &NEEDED_LIST(p) ) );
    while ( line != NULL_LINE )
    {
        fix_line( p, NULL_LINE, line );
        line = line_of( util_pop( &NEEDED_LIST(p) ) );
    }

    line = line_of( util_pop( &UNUSED_LIST(p) ) );
    while ( line != NULL_LINE )
    {
        fix_line( p, NULL_LINE, line );
        line = line_of( util_pop( &UNUSED_LIST(p) ) );
    }

    /*
     * Finally, we try to find NEW lines, ie. lines that need to be touched,
     * so that changes made to them propagate to the display.  We try to find
     * blocks of them to touch all at once, rather than simply touching each
     * individually.  This is because touching appears (currently at least) to
     * have a fair amount of constant overhead.  As the NEW lines are found
     * they are marked OLD.  When all the touching is done, all lines should be
     * OLD.
     */
    for (i=0;i<size.y;i++)
    {
	if (LINE(p)[i].info == NEW)
	{
	    short start;	/* Number of the first line of a block of NEW lines */
	    short stop;		/* Number of the first line after a block of NEW lines */
	    Rectangle affected;	/* The rect containing a block of NEW lines */

	    start = i;
	    while (i<size.y && LINE(p)[i].info == NEW)
	    {
	    	LINE(p)[i].info = OLD;
		i++;
	    }
	    stop = i;

	    affected = makeRect( makePoint(0, start), makePoint(size.x - 1, stop-1) );

	    /* If we're resizing, the whole pane will be touched for us */
	    if (! RESIZING(p))
		sm_text_block_touch(TP(p),affected);
	}
    }
    sm_text_end_batch_touch(TP(p));
}








