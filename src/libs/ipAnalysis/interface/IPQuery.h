/* $Id: IPQuery.h,v 1.2 1997/03/11 14:34:37 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef IPQuery_h
#define IPQuery_h

#ifndef context_h
#include <libs/support/database/context.h>
#endif 

typedef void *C_CallGraph;

EXTERN(C_CallGraph, IPQuery_Init, (Context pgm_context));
EXTERN(void, IPQuery_Fini, (C_CallGraph cg));

#endif
