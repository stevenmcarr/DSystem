/* $Id: bmsearch.C,v 1.3 1997/06/27 17:47:00 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *  search - an implementation of the Boyer-Moore "expected sublinear"
 *	     searching argorithm.
 *
 *  Written by William LeFebvre, Rice University
 *
 *  Modified 11 Feb 85 by Ben Chase (to suit his own diabolical purposes)
 *
 *  References: "A Fast String Searching Algorithm" by
 *		Robert S. Boyer and J. Strother Moore
 *		Communications of the ACM, Vol. 20, No. 10, Oct. 1977
 *
 *		"Fast Pattern Matching in Strings" by
 *		Donald E. Knuth, James H. Morris, Jr., and Vaughan R. Pratt
 *		Siam Journal of Computing, Vol. 6, No. 2, June 1977
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <include/bstring.h>

#include <libs/support/memMgmt/mem.h>
#include <libs/support/patternMatch/bmsearch.h>


STATIC(char*, bms_f, (char *bufp, char *bufend, BMS_private *info));
STATIC(char*, bms_b, (char *bufp, char *bufend, BMS_private *info));
STATIC(char*, bms_fc, (char *bufp, char *bufend, BMS_private *info));
STATIC(char*, bms_bc, (char *bufp, char *bufend, BMS_private *info));

BMS_private *bm_create(BMS_private *info)
{
	int length = 32;	/* initial search string allocation */

	if (info == (BMS_private*)0)
		info = (BMS_private *)get_mem(sizeof(BMS_private),"bm_create");
	bzero((char *)info, sizeof(BMS_private));

	info->delta2	  = (unsigned char *)get_mem(length+1,"bm_create");
	info->f		  = (unsigned char *)get_mem(length+1,"bm_create");
	info->pattern	  = (char *)get_mem(length+1,"bm_create");
	info->truepattern = (char *)get_mem(length+1,"bm_create");

	bzero((char *)info->delta2, length+1);
	bzero((char *)info->f, length+1);
	bzero((char *)info->pattern, length+1);
	bzero((char *)info->truepattern, length+1);

	info->maxpatlen = length;
	info->newsearch = forward;

	return info;
}

void bm_destroy(BMS_private *info)
{
	if (info == (BMS_private*)0)
		return;

	free_mem((void*)(info->delta2) );
	free_mem((void*)(info->f) );
	free_mem((void*)(info->pattern) );
	free_mem((void*)(info->truepattern) );

	free_mem((void*)info );
}


Boolean bm_is_casefolded(BMS_private *info)
{
	if (info->newsearch == forward_folded || info->newsearch == backward_folded)
		return true;
	else
		return false;
}

Boolean bm_is_forward(BMS_private *info)
{
	if (info->newsearch == forward || info->newsearch == forward_folded)
		return true;
	else
		return false;
}


/*
 * bm_toggle_case_fold(info) - Toggle the case mode used in next search,
 * 	and return true iff case folding will be used on next search.
 */
Boolean bm_toggle_case_fold(BMS_private *info)
{
	switch(info->newsearch)
	{	
	case forward:
		info->newsearch = forward_folded;
		return true;

	case backward:
		info->newsearch = backward_folded;
		return true;

	case forward_folded:
		info->newsearch = forward;
		return false;

	case backward_folded:
		info->newsearch = backward;
		return false;
	default:abort();
		return false;	/* lint */
	}
}

/*
 * bm_toggle_dir(info) - Toggle the direction used in next search,
 * 	and return true iff the next search will be forward.
 */
Boolean bm_toggle_dir(BMS_private *info)
{
	switch(info->newsearch)
	{	
	case forward:
		info->newsearch = backward;
		return false;

	case backward:
		info->newsearch = forward;
		return true;

	case forward_folded:
		info->newsearch = backward_folded;
		return false;

	case backward_folded:
		info->newsearch = forward_folded;
		return true;
	default:abort();
		return false;	/* lint */
	}
}



/*
 * bm_pattern - returns a pointer to the current search string.
 *	This string is read-only, and should not be freed.
 */
char *bm_pattern(BMS_private *info)
{
	return info->truepattern;
}


/*
 *  bm_newpattern(info, newpat, length) - search initialization
 *	"type" is the type of searching.  This
 *	routine should be called whenever a new search pattern is desired.
 *	If a change in type is desired without a change
 *	in the pattern, "bmpattern" need not be called.  The next call
 *	to bm_search will update the tables.  
 */

void bm_newpattern(BMS_private *info, char *newpat, int length)
{
    if (length <= 0)
	return;

    if (length > info->maxpatlen)
    {
	free_mem((void*)(info->delta2));
	free_mem((void*)(info->f));
	free_mem((void*)(info->pattern));
	free_mem((void*)(info->truepattern));

	info->delta2	  = (unsigned char *)get_mem(length+1,"bm_pattern");
	info->f		  = (unsigned char *)get_mem(length+1,"bm_pattern");
	info->pattern	  = (char *)get_mem(length+1,"bm_pattern");
	info->truepattern = (char *)get_mem(length+1,"bm_pattern");

	bzero((char *)info->delta2, length+1);
	bzero((char *)info->f, length+1);
	bzero((char *)info->pattern, length+1);
	bzero((char *)info->truepattern, length+1);

	info->maxpatlen = length;
    }

    /* save the pattern pointer and calculate info about pattern */
    info->patlen = length;
    info->oldsearch = bogus;				/* force recomputation of failure functions at next search */

    bcopy((const char *)newpat, (char *)info->truepattern, info->patlen + 1);	/* save it for later pattern requests of different searchtypes */
}


/*
 * bm_calc_funcs - Set up tables for searching.
 *	Compute the delta tables required by the Boyer-Moore string
 *	searching algorithm.  
 */
void bm_calc_funcs(BMS_private *info)
{
    register int i;
    register int j;
    register int k;
    register int t;
    register char *patp;
    register unsigned char *dp;
    int m = info->patlen;

    /* adjust for different flavors of search */
    if ( bm_is_forward(info) )
    {
	/* just copy it straight */
	bcopy((const char *)info->truepattern, (char *)info->pattern, m + 1 );
    }
    else
    {
	/* we must reverse the pattern before generating tables */
	register char *sp;

	patp = info->pattern;
	sp = info->truepattern + m;
	*(sp+1) = '\0';
	for (i = 0; i < m; i++)
	{
	    *patp++ = *--sp;
	}
    }

    if ( bm_is_casefolded(info) )
    {
	/* case fold search -- map all upper case to lower case */
	register int ch;

	patp = info->pattern;
	for (i = 0; i < m; i++, patp++)
	{
	    ch = *patp;
	    if (ch >= 'A' && ch <= 'Z')
	    {
		*patp = ch + ('a' - 'A');
	    }
	}
    }
    info->oldsearch = info->newsearch;

    /* set up the delta tables */
    /* Upper bound values for delta1 */
    for (i = 0, dp = info->delta1; i < 256; i++, dp++)
	*dp = m;

    /*
	This is taken straight from Computer Algorithms, by Baase,
	page 223-225.  The arrays "delta2" and "pattern" are 0-based,
	instead of 1-based, thus the occurrences of "-1" in indexing
	into these arrays.  "f" is treated as 1-based, however.
    */
    /* True values of delta1, upper bound for delta2 */
    for (k = 1; k <= m; k++)
    {
	info->delta1[info->pattern[k-1]] = m - k;
	info->delta2[k-1] = m + m - k;
    }

    j = m;
    t = m + 1;

    while (j > 0)
    {
	info->f[j] = t;
	while (t <= m && info->pattern[j-1] != info->pattern[t-1])
	{
	    info->delta2[t-1] = MIN((int)info->delta2[t-1],
				     (int)(m - j));
	    t = info->f[t];
	}
	t--;
	j--;
    }

    for (k = 1; k <= t; k++)
    {
	info->delta2[k-1] = MIN((int)info->delta2[k-1], (int)(m + t - k));
    }

    j = info->f[t];
    while (t <= m)
    {
	while (t <= j)
	{
	    info->delta2[t-1] = MIN((int)info->delta2[t-1],(int)(j-t+m));
	    t++;
	}
	j = info->f[j];
    }
}



/*
 *  char *bm_search(info, buffer, length, offset) - Boyer-Moore search.
 *	Search for the pattern specified in the last call to "bmpattern"
 *	in the "length" byte buffer called "buffer".
 *	Searches forward (backward) and returns a pointer to the first
 *	character after (the first character of) of the first occurrence of
 *	the pattern after (before) position "offset".
 *	Returns UNUSED if no occurrence was found.
 */
int bm_search(BMS_private *info, char *buffer, int length, int offset)
{
    register char *start;	/* place to start searching */
    register char *end;		/* place to stop searching */
    register char *loc = NULL;	/* location of the found occurrence */

    if (info->newsearch != info->oldsearch)
	bm_calc_funcs( info );

    if (bm_is_forward(info))
    {
	start = buffer + offset + info->patlen;
	end = buffer + length;
    }
    else
    {
	start = buffer + offset - info->patlen -1;
	end = buffer - 1;
    }

    /* Call an inner loop routine appropriate to the flavor of search */
    switch (info->oldsearch)
    {
	case forward:
	    loc = bms_f(start, end, info);
	    break;

	case backward:
	    loc = bms_b(start, end, info) ;
	    break;

	case forward_folded:
	    loc = bms_fc(start, end, info) ;
	    break;

	case backward_folded:
	    loc = bms_bc(start, end, info) ;
	    break;

        default:
            assert(0);
            break;
    }
    if (loc == NULL)
 	return -1;
    else
    	return loc - buffer;
}

static char *bms_f(char *bufp, char *bufend, BMS_private *info)
{
    register char *patp;

    while (bufp <= bufend)
    {
	patp = info->pattern + info->patlen;
    
	while (*--bufp == *--patp)
	{
	    if (patp == info->pattern)
	    {
		return bufp + info->patlen;
	    }
	}

	bufp = bufp + MAX(info->delta1[*bufp], info->delta2[patp - info->pattern]) + 1;
    }
    return NULL;
}

static char *bms_b(char *bufp, char *bufend, BMS_private *info)
{
    register char *patp;

    while (bufp >= bufend)
    {
	patp = info->pattern + info->patlen;
    
	while (*++bufp == *--patp)
	{
	    if (patp == info->pattern)
	    {
		return bufp - info->patlen + 1;
	    }
	}

	bufp = bufp - MAX(info->delta1[*bufp], info->delta2[patp - info->pattern]) - 1;
    }
    return NULL;
}

static char *bms_fc(char *bufp, char *bufend, BMS_private *info)
{
    register char *patp;
    register int  ch;

    while (bufp <= bufend)
    {
	patp = info->pattern + info->patlen;
    
	while (ch = *--bufp,
	       (ch >= 'A' && ch <= 'Z' ? (ch += 'a' - 'A') : ch) == *--patp)
	{
	    if (patp == info->pattern)
	    {
		return bufp + info->patlen;
	    }
	}

	bufp = bufp + MAX(info->delta1[ch], info->delta2[patp - info->pattern]) + 1;
    }
    return NULL;
}

static char *bms_bc(char *bufp, char *bufend, BMS_private *info)
{
    register char *patp;
    register char ch;

    while (bufp >= bufend)
    {
	patp = info->pattern + info->patlen;
    
	while (ch = *++bufp,
	       (ch >= 'A' && ch <= 'Z' ? (ch += 'a' - 'A') : ch) == *--patp)
	{
	    if (patp == info->pattern)
	    {
		return bufp - info->patlen + 1;
	    }
	}

	bufp = bufp - MAX(info->delta1[ch], info->delta2[patp - info->pattern]) - 1;
    }
    return NULL;
}

/* for debugging: */

void
bm_dump(BMS_private *info)
{
    int i;

    (void) fprintf(stderr, "bmdump: dump of bmsearch tables: patlen = %d\n",
	info->patlen);

    for (i=0; i<=info->patlen; i++)
    {
	(void) fprintf(stderr, "%3d ", i);
    }
    (void) fputc('\n', stderr);

    for (i=0; i<info->patlen; i++)
    {
	(void) fprintf(stderr, "  %c ", info->pattern[i]);
    }
    (void) fputc('\n', stderr);

    for (i=0; i<=info->patlen; i++)
    {
	(void) fprintf(stderr, "%3d ", info->f[i]);
    }
    (void) fputc('\n', stderr);

    for (i=0; i<=info->patlen; i++)
    {
	(void) fprintf(stderr, "%3d ", info->delta2[i]);
    }
    (void) fputc('\n', stderr);

    for (i=0; i<256; i++)
    {
	if (info->delta1[i] != info->patlen)
	{
	    (void) fprintf(stderr, "delta1[%3d] = %3d\n", i, info->delta1[i]);
	}
    }
    (void) fputc('\n', stderr);
}
