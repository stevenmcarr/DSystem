/* $Id: FileUtilities.h,v 1.4 1997/03/11 14:36:37 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef fileutils_h

/********************************************************************
 * fileutils
 *
 *   a collection of miscellaneous functions for manipulating
 *   Unix files, filenames, etc.
 * 
 * Author:
 *  John Mellor-Crummey                                    May 1993
 *
 ********************************************************************/

#include <sys/types.h>

#include <libs/support/misc/general.h>

/*----------------------------------------------------------------------------
 * return the directory name component of a unix file path name.
 * trailing slashes are disregarded. "." will be returned if
 * the path does not contain any non-trailing slashes
 *--------------------------------------------------------------------------*/
EXTERN(char *, FileDirName, (const char *const pathName));


/*----------------------------------------------------------------------------
 * return the base name component of a unix file path name.
 * trailing slashes are disregarded. 
 *--------------------------------------------------------------------------*/
EXTERN(char *, FileBaseName, (const char *const pathName));


/*----------------------------------------------------------------------------
 * return 0 if the file exists
 *--------------------------------------------------------------------------*/
EXTERN(int, FileExists, (const char *const pathName));


/*----------------------------------------------------------------------------
 * return last modification time of a file or (time_t) 0 if file does not
 * exist
 *--------------------------------------------------------------------------*/
EXTERN(time_t, FileLastModificationTime, (const char *const pathName));


/*----------------------------------------------------------------------------
 * if file is a relative path name, prepend to it the directory in dir 
 *--------------------------------------------------------------------------*/
EXTERN(char *, FullPath, (const char *dir, const char *file));



#endif fileutils_h
