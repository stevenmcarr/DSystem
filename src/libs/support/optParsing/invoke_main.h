/* $Id: invoke_main.h,v 1.6 1997/03/11 14:37:08 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef invoke_main_h
#define invoke_main_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

typedef
FUNCTION_POINTER(int, InvokeMain_FunctPtr, (int argc, char** argv));

EXTERN(int, invoke_main, (InvokeMain_FunctPtr mainFuctionName, char* optsList));

# endif
