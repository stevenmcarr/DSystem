/* $Id: regexp.h,v 1.4 1997/03/11 14:37:11 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * Definitions etc. for regexp(3) routines.
 *
 * Caveat:  this is V8 regexp(3) [actually, a reimplementation thereof],
 * not the System V one.
 */
#ifndef regexp_h
#define regexp_h

#define REGEX_ERROR	-1
#define REGEX_NOMATCH	0
#define REGEX_MATCH	1

#define NSUBEXP  10
typedef struct _regexp {
	char *startp[NSUBEXP];
	char *endp[NSUBEXP];
	char regstart;		/* Internal use only. */
	char reganch;		/* Internal use only. */
	char *regmust;		/* Internal use only. */
	int regmlen;		/* Internal use only. */
	char program[1];	/* Unwarranted chumminess with compiler. */
} regexp;

EXTERN(regexp *, regcomp, (char *exp));
EXTERN(int, regexec, (regexp *prog, char *string));
EXTERN(void, regsub, (regexp *prog, char *source, char *dest));
EXTERN(void, regerror, (char *s));
EXTERN(void, regfree, (regexp *r));

#endif
