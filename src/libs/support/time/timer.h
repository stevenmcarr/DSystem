/* $Id: timer.h,v 1.4 1997/03/11 14:37:46 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef timer_h
#define timer_h

/*
 * definitions for structures used in util/timer.c
 * 
 */
struct time_buffer
	{
	int utime;
	int stime;
	int wall;
	int faults;
	int reads;
	int writes;
	int swaps;
	};

typedef struct time_buffer timer;

/* Which values for timer_stat */

# define TIMER_UTIME	1
# define TIMER_STIME	2
# define TIMER_WALL	3
# define TIMER_FAULTS	4
# define TIMER_READS	5
# define TIMER_WRITES	6
# define TIMER_SWAPS	7

EXTERN(timer, *timer_init, (timer *time_clock));
EXTERN(void, timer_get_times, (timer *buffer));
EXTERN(void, timer_update, (timer *start, timer *buf));
EXTERN(void, timer_show, (timer *time_clock));
EXTERN(int, timer_stat, (timer *time_clock, int which));
EXTERN(void, timer_free, (timer *time_clock));

#endif
