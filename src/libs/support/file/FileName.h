/* $Id: FileName.h,v 1.5 1997/03/11 14:36:36 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef filename_h
#define filename_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

EXTERN(char*, file_shortname, (char* filename));
/*
 * Takes one parameter "filename" (char *) and returns a "shorthand" name
 * for it. This consists of the all characters from the end back to the
 * first encountered slash.
 */

EXTERN(char**, getComponents, (char* s, char sep));
/* Takes two parameters (char *s) the string from which to clip components
 * and (char sep) the separator character between components.  A (char *)0
 * terminated array of the components is returned.  Example:
 *	a = getComponents(":/bin:/usr/bin",':')
 * where a is
 *	a[0]	""
 *	a[1]	"/bin"
 *	a[2]	"/usr/bin"
 *	a[3]	(char *)0
 *	
 * The array returned (in this case 'a') and the strings within in it
 * should be freed with freeComponents().  The original string is not
 * affected by getComponents().
 */

EXTERN(char**, getFilenameComponents, (char* f, int* countp));
/*
 * Takes two parameters (char *f) the filename from which components are
 * clipped and (int *countp), an output parameter set to the number of
 * components found.  The returned array of pointers to strings is an
 * array of the clipped components, [0 .. *countp-1].  A component
 * of a filename is the string of non-slash characters bounded by slashes,
 * or preceeding the first slash, or trailing the last slash.
 * Notice that *countp and the return value together make an argc,argv
 * style list.
 */

EXTERN(void, freeComponents, (char** c));
/*
 * Takes one parameter (char **c) an array of components
 * gotten from getComponents() or getFilenameComponents().
 * All memory allocated for "c" is freed by this call.
 */

#endif
