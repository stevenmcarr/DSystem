/* $Id: stopwatch.C,v 1.2 1997/03/11 14:37:45 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/support/time/stopwatch.h>

Swatch swatch_new(void)
{
    Swatch sw = new StopWatch();
    sw->start();
    return sw;
}
void swatch_delete(Swatch sw)
{
    delete sw;
}
void swatch_start(Swatch sw)
{
    sw->start();
}
void swatch_restart(Swatch sw)
{
    sw->restart();
}
SWTime swatch_lap(Swatch sw)
{
    return sw->lap();
}
SWTime swatch_check(Swatch sw)
{
    return sw->check();
}
