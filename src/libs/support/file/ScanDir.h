/* $Id: ScanDir.h,v 1.4 1997/03/27 20:49:09 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************
 *                                                                      *
 * File:     scandir utility                              June 1993     *
 *                                                                      *
 *                                                                      *
 *     Copyright 1993, Rice University, as part of the Rn/ParaScope     *
 *                    Programming Environment Project                   *
 *                                                                      *
 ************************************************************************/

#ifndef scandir_h
#define scandir_h

/*****************************include files******************************/

#include <dirent.h>

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

/********************* scandir function prototypes **********************/

typedef FUNCTION_POINTER (int, SelectFunctPtr, (struct dirent*));
typedef FUNCTION_POINTER (int, CompareFunctPtr, (struct dirent**, 
                                                 struct dirent**));

#ifndef OSF1
/********************** scandir extern functions ************************/

EXTERN (int, scandir, (char*, struct dirent***, SelectFunctPtr, CompareFunctPtr));

EXTERN (int, alphasort, (struct dirent**, struct dirent**));

#endif

#endif
