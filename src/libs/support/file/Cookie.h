/* $Id: Cookie.h,v 1.1 1997/03/11 14:36:35 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/* $Id */

/******************************************************************
// Cookie.h:
//
//  read/write a file marker. presence of the marker at the end of a 
//  file section when reading indicates that the file section was 
//  previously written in its entirety
//
// Author: 
//   John Mellor-Crummey                              October 1993
//
// Copyright 1993, Rice University
/ ******************************************************************/

#ifndef Cookie_h
#define Cookie_h

#ifndef newdatabase_h
#include <libs/support/database/newdatabase.h>
#endif

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

EXTERN(int, ReadCookie, (DB_FP *file));
EXTERN(int, WriteCookie, (DB_FP *file));

#endif
