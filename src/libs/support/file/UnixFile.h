/* $Id: UnixFile.h,v 1.3 1997/03/11 14:36:39 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef unix_file_h
#define unix_file_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

EXTERN(Boolean, file_ok_to_write, (const char *filename));

/* create the specified file, return true if successful */
EXTERN(Boolean, file_touch,       (const char *filename));

/* returns true if the specified file exists and has 0 length */
EXTERN(Boolean, file_is_empty,    (const char *filename));

EXTERN(Boolean, file_access,      (const char *filename, int mode));

#endif
