/* $Id: ExtendedGCD.h,v 1.5 1997/03/11 14:37:04 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *  Routines to compute greatest common denominator, least common multiple,
 *  and Euclidean mod and div -- for last two, see
 *
 *  @article{Boute:Euclidean,
 *  Author={Raymond T. Boute},
 *  Title={The Euclidean Definition of the Functions div and mod},
 *  Journal=TOPLAS,
 *  Volume=14,
 *  Number=2,
 *  Pages={127-144},
 *  Month={April},
 *  Year={1992}}
 *
 *  Also Extended Gcd algorithm from Knuth, The Art of Computer Programming,
 *		Fundamental Algorithms, 2nd Ed., p. 14
 *	Given arbitrary integers a and b, find d = gcd(a,b) and
 *		integers x and y such that ax + by = d.
 *
 *
 *  On generally invalid inputs, the functions return:
 *
 *		Input		Return
 *	Gcd	(x,0)		x
 *		(0,x)		x
 *		(0,0)		0 (guarantees that Gcd is assoc/commut.)
 *		(x<0,y<0)	Gcd(|x|,|y|)
 *	Lcm	(0,x)		0
 *		(x,0)		0
 *		(0,0)		0
 *	EMod	(x,0)		error
 *	EDiv	(x,0)		error
 */

#ifndef emod_h
#define emod_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

EXTERN(int, EMod,(int a, int b));
EXTERN(int, EDiv,(int a, int b));
EXTERN(int, Gcd,(int a, int b));
EXTERN(int, Lcm,(int a, int b));
EXTERN(int, EGcd, (int a, int b, int *x, int *y));

#endif
