/* $Id: optim_clear.C,v 1.1 1997/06/25 14:58:38 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/graphicInterface/oldMonitor/monitor/sms/optim_sm/optim_private.h>

void optim_clear_line(register TextChar *line, register int length, TextChar clear)
{
    register short i;

    for (i=0; i<length; i++)
	line[i] = clear;
}


void
sm_optim_clear_eol(Pane *p, Point loc)
{
    osm_line *line = &(LINE(p)[loc.y]);
    short width = sm_optim_width(p);

    /* clear desired map to end of line */
    optim_clear_line( line->New+loc.x, width-loc.x, CLEAR(p) );

    /* recalculate line->first_mod */
    /* recalculate line->last_mod */
    line->first_mod = MIN(loc.x,line->first_mod);
    line->last_mod  = width - 1;
}


void
optim_clear_actual(Pane *p)
{
    Point size;
    short i;

    size = sm_optim_size(p);

    for (i=0;i<size.y;i++)
    {
        osm_line *line = &(LINE(p)[i]);

	optim_clear_line(line->old,size.x,CLEAR(p));

        line->first_mod = 0;
        line->last_mod  = size.x - 1;
    }
    sm_text_pane_clear( TP(p) ) ;
}


void
sm_optim_refresh(Pane *p)
{
    optim_clear_actual(p) ;
    sm_optim_touch(p) ;
}


/*
 * sm_optim_clear - clear the desired screen.
 *      (Try not to use this, it's expensive.)
 */
void sm_optim_clear(Pane *p)
{
    Point size;
    short i;

    size = sm_optim_size(p);

    for (i=0;i<size.y;i++)
    {
        osm_line *line = &(LINE(p)[i]);

	optim_clear_line(line->New,size.x,CLEAR(p));

        line->first_mod = 0;
        line->last_mod  = size.x - 1;
    }
}
