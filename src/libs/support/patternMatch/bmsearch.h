/* $Id: bmsearch.h,v 1.4 1997/03/11 14:37:10 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef bmsearch_h
#define bmsearch_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

typedef enum {
	bogus,
	forward,
	backward,
	forward_folded,
	backward_folded
} bm_searchtype;

typedef struct {
	unsigned char delta1[256];
	unsigned char *delta2;
	unsigned char *f;
	char *pattern;				/* pattern after possible reversing/folding */
	char *truepattern;			/* original */
	int patlen;
	int maxpatlen;
	bm_searchtype oldsearch;		/* searchtype last used */
	bm_searchtype newsearch;		/* searchtype to be used next */
} BMS_private;


EXTERN(BMS_private *, bm_create, (BMS_private *info));
/* create a Boyer-Moore searching handle */

EXTERN(void, bm_destroy, (BMS_private *info));
/* destroy a Boyer-Moore searching handle */

EXTERN(void, bm_newpattern, (BMS_private *info, char *newpat, int length));
/* provide a new search pattern */

EXTERN(void, bm_calc_funcs, (BMS_private *info));

EXTERN(int, bm_search, (BMS_private *info, char *buffer, int length, 
int offset));
/* search for a pattern in a buffer */

EXTERN(void, bm_dump, (BMS_private *info));

EXTERN(Boolean, bm_is_casefolded, (BMS_private *info));
/* returns true iff case folding will be used for next search */

EXTERN(Boolean, bm_is_forward, (BMS_private *info));
/* returns true iff next search will be a forward search */

EXTERN(Boolean, bm_toggle_case_fold, (BMS_private *info));
/* toggles case folding, returns true iff case folding for next search */

EXTERN(Boolean, bm_toggle_dir, (BMS_private *info));
/* toggles direction, returns true iff forward search for next search */

EXTERN(char *, bm_pattern, (BMS_private *info));
/* returns the current search string */

#endif
