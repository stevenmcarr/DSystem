/* $Id: optim_put.C,v 1.1 1997/06/25 14:58:38 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <stdlib.h>

#include <libs/graphicInterface/oldMonitor/monitor/sms/optim_sm/optim_private.h>

short optim_advance(register short start, register short stop, register 
                    TextChar *old, register TextChar *New)
{
    while ( equalTextChar( New[start], old[start] ) )
	if (++start > stop)
	{
	    start = stop;
	    break;
	}
    return start;
}

short optim_retreat(register short start, register short stop, register 
                    TextChar *old, register TextChar *New)
{
    while ( equalTextChar( New[start], old[start]) )
	if (--start < stop)
	{
	    start = stop;
	    break;
	}
    return start;
}


void
sm_optim_putchar(register Pane *p, Point loc, TextChar tch)
{
    register	osm_line	*line	= &(LINE(p)[loc.y]);
    register	TextChar	*New	= line->New;
    short	col	= loc.x;

    if (optim_debugging)
    {
	if (NOT(pointInRect( loc, makeRectFromSize(Origin, sm_optim_size(p)) )))
		abort();
    }

    /* change the character in the desired map */
    New[col] = tch;

    line->first_mod = MIN(col,line->first_mod);
    line->last_mod  = MAX(col,line->last_mod);
}


/*
 * sm_optim_putstr -  write a TextString to the desired screen, with clipping
 */
void
sm_optim_putstr(register Pane *p, Point loc, TextString ts)
{
    register short i;
    short row = loc.y;
    short col = loc.x;
    short width = sm_optim_width(p);
    short len = MIN( ts.num_tc, width-col );
    register osm_line *line;
    register TextChar *New;

    if (optim_debugging)
    {
	Rectangle paneRect;

	paneRect = makeRectFromSize(Origin,sm_optim_size(p));

	if (NOT(pointInRect( loc, paneRect )))
		abort();
	if (NOT(pointInRect( makePoint(loc.x+len-1,loc.y), paneRect )))
		abort();
    }

    line = &(LINE(p)[row]);
    New = line->New;

    for (i=0; i<len; i++)
	New[col+i] = ts.tc_ptr[i];

    line->first_mod = MIN(col,line->first_mod);
    line->last_mod  = MAX(col+len-1,line->last_mod);
}


/*
 * sm_optim_putline -  write an array of TextChars to the desired screen by
 *	swapping it with a line of the desired screen map.  After this call
 *	the array is owned by the optim screen module.  The array should be
 *	freeable with free_mem() and the size of the array should be exactly
 *	the width of the screen.  A similar array is returned by this routine,
 *	which becomes the property of the caller, and should be freed via
 *	free_mem() if it is unneeded.
 */
TextChar *
sm_optim_putline(register Pane *p, register short row, register TextChar *ts)
{
    register short	width = sm_optim_width( p );
    register osm_line	*line = &(LINE(p)[row]);
    register TextChar	*swap;

    if (optim_debugging)
    {
	if ( NOT(0 <=row && row < width) )
		abort();
    }

    swap = line->New;
    line->New = ts;

    /* make `first' point to the first changed character */
    /* make `last' point to the last changed character */
    line->first_mod = 0;
    line->last_mod  = width-1;

    /*
     * It is cheaper to return a clear line, than to return a
     * dirty line and then later do a sm_optim_clear_eol().
     * Of course, whoever we're swapping with should take
     * advantage of the fact that we're giving them a clean line.
     */
    optim_clear_line( swap, width, CLEAR(p) );
    return swap;
}
