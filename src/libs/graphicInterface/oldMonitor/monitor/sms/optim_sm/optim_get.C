/* $Id: optim_get.C,v 1.1 1997/06/25 14:58:38 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <stdlib.h>

#include <libs/graphicInterface/oldMonitor/monitor/sms/optim_sm/optim_private.h>

# undef sm_optim_width
# undef sm_optim_height
# undef sm_optim_size

short
sm_optim_width(Pane* p)
{
    return sm_text_width( TP(p) );
}

short
sm_optim_height(Pane* p)
{
    return sm_text_height( TP(p) );
}

Point
sm_optim_size(Pane* p)
{
    return sm_text_size( TP(p) );
}

/*
 * sm_optim_getchar - Returns the TextChar at position 'loc'.  If 'actual' is true the
 *	actual contents of the pane at 'loc' are returned.  Otherwise, the current
 *	contents of the screen map (the desired screen) are returned.
 */
TextChar
sm_optim_getchar(Pane* p, Point loc, Boolean actual )
   /* actual - One of (SM_OPTIM_DESIRED, SM_OPTIM_ACTUAL) */
{
    if (optim_debugging)
    {
	if (NOT(pointInRect( loc, makeRectFromSize(Origin, sm_optim_size(p)) )))
		abort();
    }

    if (actual == SM_OPTIM_ACTUAL)
	return LINE(p)[loc.y].old[loc.x];
    else
	return LINE(p)[loc.y].New[loc.x];
}
