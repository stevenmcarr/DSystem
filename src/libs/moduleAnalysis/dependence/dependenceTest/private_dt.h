/* $Id: private_dt.h,v 1.7 1997/03/11 14:35:55 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#ifndef	private_dt_h
#define	private_dt_h

/*-----------------------------------------------------------------------

	private_dt.h		Dependence Test Module Header File

	History
	~~~~~~~
	2 Apr 90  cwt  Created

*/


#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>

/*---------*/
/* defines */

#define NO_IVAR   (-1)		/* no match w/ induction var		*/

#define HASH_LOOP       2047	/* size of table for loops			*/
#define HASH_DEPS      16381	/* size of table for dependences	*/
#define HASH_KEY_LOOP      1	/* # of keys for loops				*/
#define HASH_KEY_DEPS      2	/* # of keys for dependences		*/

/*------------------------------*/
/* types for Dvect_data.ltype	*/

/* dependence hierarchy (for 1 dimension) */

#define DV_ANY  0		/* any 						 */
#define DV_NO   1		/* no dependence			 */
#define DV_PT   2		/* point					 */
#define DV_DIS  3		/* distance 				 */
#define DV_LN   4		/* line						 */
#define DV_EQ   5		/* equal					 */
#define DV_LT   6		/* less than				 */
#define DV_GT   7		/* greater than				 */
#define DV_LE   8		/* less than or equal		 */
#define DV_GE   9		/* greater than	or equal	 */
#define DV_NE  10		/* not equal				 */

/*------------*/
/* structures */

/*--------------------------------------------------*/
/* local structure for calculating dependence info	*/
/* Dependence info is stored as follows:			*/

/* ltype		data								*/
/*--------------------------------------------------*/
/* DV_PT		point    = <c1, c2>					*/
/* DV_LN 		line     = c1*x = c2*y + c3			*/
/* DV_DIS 		distance = c1			 			*/

typedef struct
{
	int c1[MAXLOOP];		/* data for loops			 */
	int c2[MAXLOOP];
	int c3[MAXLOOP];
	int ltype[MAXLOOP];		/* dist/dir type for loops	 */
} Dvect_data;

/*--------*/
/* macros */

#define DIVIDES(x,y)     ((y) == (x)*((y)/(x)))
#define ABS(x)           ((x) >= 0 ? (x) : -(x))
#define IN_BOUND(x,l,u)  (((x) <= (u)) && ((x) >= (l)))
#define SYM_EQ2(x,y)      ((*(x) == *(y)) && !strcmp((x), (y)))


#endif	private_dt_h
