/* $Id: stopwatch.h,v 1.6 1997/03/11 14:37:45 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef stopwatch_h
#define stopwatch_h

/* Now in milliseconds instead of microseconds... */

#include <libs/support/time/swatch.h>

class StopWatch {
  private:
    SWTime begin;
    SWTime total;
  public:
    StopWatch()	   { total = 0; begin = 0; }
    void start()   { total = 0; begin = (SWTime) clock()/1000; }
    void restart() { begin = (SWTime) clock()/1000; }
    SWTime check() { return total; }
    SWTime lap()
    {
	SWTime now = (SWTime) clock()/1000;
	total += (now - begin);
	begin = now;
	return total;
    }
};
#endif stopwatch_h
