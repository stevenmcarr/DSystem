/* $Id: ExtendedGCD.C,v 1.1 1997/06/25 15:17:54 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/****************************************************************************/
/*                                                                          */
/*   % in C is the remainder function, not mod.  Below is the Euclidean     */
/*   mod function (same as % when args are both positive).                  */
/*                                                                          */
/*   EMod(a,b) = (a % b) < 0? (a%b + abs(b)) : (a % b)		    	    */
/*                                                                          */
/*   Euclidean div (EDiv) is defined so that (EDiv(a,b))*b + EMod(a,b) = a  */
/*                                                                          */
/*   This file also defines Gcd and Lcm functions                           */
/*                                                                          */
/****************************************************************************/

#include <stdlib.h>

#include <libs/support/numerical/ExtendedGCD.h>

    /* can't these OS people get some standard on include files... */
EXTERN(int, abs, (int a));

int EMod(int a, int b)
{
    int rem = a % b;
    return (rem < 0)? (rem + abs(b)) : rem;
}

int EDiv(int a, int b)
{
    int rem = a % b;
    int div = a / b;

    if (rem < 0)
    {
	if (b < 0)
	    div++;
	else
	    div--;
    }
    
    return div;
}


/****************************************************************************/
/*  greatest common divisor                                                 */
/****************************************************************************/
int Gcd(int a, int b)
{
    int r;
    int x = abs(a);
    int y = abs(b);

    while (y != 0)
    {
	/*
	 *  If x < y, then first iteration swaps them.
	 */
	r = x % y;
	x = y;
	y = r;
    }
    return(x);
}

/****************************************************************************/
/*  least common multiple                                                   */
/****************************************************************************/
int Lcm(int a, int b)
{
    if ((a*b) == 0) return 0;	/* overkill to catch case of both 0 */

    return( (abs(a*b)) / (Gcd(a,b)) );
}

/*
 *  Extended Gcd, from Knuth, The Art of Computer Science, Fundamental 
 *	Algorithms, Second Edition, p. 14 (variables renamed here)
 *
 *	Given integers a and b, return d = gcd(a,b) and set
 *	x and y so that ax + by = d.
 */
int EGcd(int a, int b, int* x, int* y)
{
    int x1, y1, c, d, q, r, t;

    /* 
     *  E1. Initialize. 
     */
    x1 = *y = 1;
    y1 = *x = 0;
    c = a;
    d = b;

    for (;;) 
    {
	/* 
	 *  E2. Divide.  Use Euclidean div and mod.
	 *  Let q, r be the quotient and the remainder, respectively, 
	 *  of c divided by d.  (We have c = qd + r, 0 <= r < d.)
	 */
	q = EDiv(c, d);
	r = EMod(c, d);

	/*
	 *  E3. Remainder zero?
	 *  If r == 0, the algorithm terminates; we have in this case
	 *  ax + by = d as desired.
	 */
	if (r == 0) break;

	/*
	 *  E4. Recycle.  (Set things and go back to E2)
	 */
	c = d;
	d = r;
	t = x1;
	x1 = *x;
	*x = t - q*(*x);
	t = y1;
	y1 = *y;
	*y = t - q*(*y);
    }
    return d;
}
