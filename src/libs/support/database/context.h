/* $Id: context.h,v 1.3 1997/03/11 14:36:34 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef context_h
#define context_h


#ifndef newdatabase_h
#include <libs/support/database/newdatabase.h>
#endif

#ifdef __cplusplus
class AttributedFile;
typedef class AttributedFile *Context;
#define CONTEXT_NULL 0
#else
typedef struct AttributedFile *Context;
#define CONTEXT_NULL 0
#endif

#endif
