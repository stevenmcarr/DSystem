/* $Id: CheckerOptions.h,v 1.1 1997/03/11 14:27:27 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/******************************************************************************
 *
 * File
 *    CheckerOptions.h
 *
 * Original Author
 *    Kevin Cureton
 *
 * Creation Date
 *    August 1994
 *
 * Description
 *    Header file for the CheckerOptions.C.
 *
 * History
 *    08/94 - Kevin Cureton - original revision.
 *
 ******************************************************************************/

#ifndef CheckerOptions_h
#define CheckerOptions_h

/**************************** System Include Files ****************************/

/***************************** User Include Files *****************************/

#include <libs/support/misc/general.h>

/**************************** Variable Definitions ****************************/

extern char* global_pgm_loc;
extern char* global_mod_loc;
extern char* global_list_loc;

extern FILE* global_dep_ptr;

extern Boolean global_dep_opt;

/************************* Extern Function Prototypes *************************/

EXTERN(int, CheckerOptsProcess, (int argc, char **argv));

#endif
