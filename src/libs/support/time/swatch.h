/* $Id: swatch.h,v 1.5 1997/03/11 14:37:46 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef swatch_h
#define swatch_h

#include <libs/support/misc/general.h>

#ifdef _AIX
    /* on rs6k */
#ifndef _H_TIME
#include <time.h>
#endif 
    typedef clock_t SWTime;
#else
    /* not on rs6k */
    EXTERN(long int, clock, (void));
    typedef unsigned long SWTime;
#endif

typedef struct StopWatch *Swatch;

/*
 *  System timer call
 */
EXTERN(Swatch, swatch_new,     (void));
EXTERN(void,   swatch_delete,  (Swatch sw));
EXTERN(void,   swatch_start,   (Swatch sw));
EXTERN(void,   swatch_restart, (Swatch sw));
EXTERN(SWTime, swatch_lap,     (Swatch sw));
EXTERN(SWTime, swatch_check,   (Swatch sw));

#endif /* swatch_h */
