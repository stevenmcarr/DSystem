/* $Id: timer.C,v 1.1 1997/06/25 15:21:03 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <sys/types.h>
#if (THIS_IS_UNA == 1)
#include <sys/vtimes.h>
#endif
#include <stdio.h>
#include <libs/support/misc/general.h>
#include <libs/support/time/timer.h>
#include <libs/support/memMgmt/mem.h>

/****************************************************************/
/* timer *timer;                                                */
/* int stat;                                                    */
/* int which;                                                   */
/*                                                              */
/* timer = timer_init();                                        */
/* timer_show(timer);                                           */
/* stat = timer_stat(timer, which);                             */
/* timer_free(timer);                                           */
/*                                                              */
/* Where which is one of:                                       */
/*                                                              */
/* TIMER_UTIME	User mode time in 60ths of a second             */
/* TIMER_STIME	System mode time in 60ths of a second           */
/* TIMER_WALL	Wall time in seconds                            */
/* TIMER_FAULTS	Page faults which resulted in disk activity     */
/* TIMER_READS	Reads which resulted in disk activity           */
/* TIMER_WRITES	Writes which resulted in disk activity          */
/* TIMER_SWAPS	Number of process swaps                         */
/*                                                              */
/****************************************************************/

timer *timer_init(timer *time_clock)
	{
	if (time_clock == 0) time_clock =
		(timer *) get_mem(sizeof(struct time_buffer), "timer");
	timer_get_times(time_clock);
	return time_clock;
	}

void timer_get_times(timer *buffer)
	{
#if (THIS_IS_UNA == 1)
	extern time_t time();
	struct vtimes temp;

	(void) vtimes(&temp, (struct vtimes *) 0);

	buffer->utime  = temp.vm_utime;
	buffer->stime  = temp.vm_stime;
	buffer->wall   = time((time_t *) 0);
	buffer->faults = temp.vm_majflt;
	buffer->reads  = temp.vm_inblk;
	buffer->writes = temp.vm_oublk;
	buffer->swaps  = temp.vm_nswap;
#endif
	}

# define update(f) (buf->f -= start->f)

void timer_update(timer *start, timer *buf)
	{
	timer_get_times(buf);

	update(utime);
	update(stime);
	update(wall);
	update(faults);
	update(reads);
	update(writes);
	update(swaps);
	}

void timer_show(timer *time_clock)
	{
	timer temp;
	int hours, minutes, seconds;

	timer_update(time_clock, &temp);

	seconds = temp.wall % 60;
	minutes = (temp.wall / 60) % 60;
	hours = temp.wall / 3600;
	
	(void) printf("\
%.1fu, %.1fs, %d:%02d:%02d wall, %d faults, %d reads, %d writes, %d swaps\n",
		temp.utime/60.0,
		temp.stime/60.0,
		hours, minutes, seconds,
		temp.faults,
		temp.reads,
		temp.writes,
		temp.swaps);
	}

int timer_stat(timer *time_clock, int which)
	{
	timer temp;

	timer_update(time_clock, &temp);

	switch(which)
		{
		case TIMER_UTIME:	return temp.utime;
		case TIMER_STIME:	return temp.stime;
		case TIMER_WALL:	return temp.wall;
		case TIMER_FAULTS:	return temp.faults;
		case TIMER_READS:	return temp.reads;
		case TIMER_WRITES:	return temp.writes;
		case TIMER_SWAPS:	return temp.swaps;
		}

	return -1;
	}

void timer_free(timer *time_clock)
	{
	free_mem((void*)time_clock);
	}

