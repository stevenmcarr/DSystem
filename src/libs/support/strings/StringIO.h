/* $Id: StringIO.h,v 1.1 1997/03/11 14:37:31 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef StringIO_h
#define StringIO_h

//*****************************************************************
//                                                           
// StringIO 
//
//   simple interface to read/write ascii strings 
//
// Author: John Mellor-Crummey                         Sept 1993
//                                                           
// Copyright 1993, Rice University
//                                                           
//*****************************************************************

#include <sys/types.h>

#define STRINGIO_DEFAULT_MAX_STRING_LENGTH 1024

class FormattedFile;

// returns 0 on success, EOF on error 
extern int WriteString(const char *name, FormattedFile *file, 
		       uint maxlen = STRINGIO_DEFAULT_MAX_STRING_LENGTH);

// if successful (return code == 0), *string points to a string saved with ssave
extern int ReadString(char **string, FormattedFile *file, 
		      uint maxlen = STRINGIO_DEFAULT_MAX_STRING_LENGTH);

#endif
