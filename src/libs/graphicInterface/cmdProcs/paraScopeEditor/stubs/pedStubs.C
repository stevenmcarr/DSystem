/* $Id: pedStubs.C,v 1.11 1997/03/11 14:32:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/support/misc/general.h>

EXTERN(void, pedRoot, (void));
EXTERN(void, subs_free, (void));
EXTERN(void, subs_attach, (void));

void pedRoot(void)
{
  return;
}

void subs_free(void)
{
  return;
}

void subs_attach(void)
{
  return;
}
