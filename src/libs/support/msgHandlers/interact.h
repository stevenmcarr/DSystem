/* $Id: interact.h,v 1.3 1997/03/11 14:37:02 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef interact_h
#define interact_h
/*
 * It is necessary, in some cases, to parameterize functions depending on 
 * whether they are running with or without the window system. These typedefs
 * should be used as the types of arguments that are functions that can be invoked
 * to perform the necessary operation.
 */

#ifndef general_h
#include <libs/support/misc/general.h>
#endif 

typedef FUNCTION_POINTER(void, MessageFunction, (char *format, ...));

typedef FUNCTION_POINTER(Boolean, YesNoFunction, (char *prompt, Boolean *answer, 
                                                  Boolean def));


EXTERN(void,    message_non_windowing, (char *format, ...));
EXTERN(Boolean, yes_no_non_windowing,  (char *prompt, Boolean *answer, Boolean def));

#endif
