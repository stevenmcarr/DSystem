/* $Id: ScanFile.h,v 1.2 1997/03/27 20:31:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef ScanFile_h
#define ScanFile_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef rn_varargs_h
#include <include/rn_varargs.h>
#endif

typedef FUNCTION_POINTER(void, ScanFileFileFunc, (const char *fileName, va_list args));
typedef FUNCTION_POINTER(void, ScanFileLineFunc, (const char *lineText, va_list args));

/* 0 on success, -1 on failure */

EXTERN(int, ScanFile, 
       (const char *srcFile, ScanFileFileFunc enterFile, ScanFileFileFunc exitFile,
	ScanFileLineFunc procFileLine, ...));

#endif
