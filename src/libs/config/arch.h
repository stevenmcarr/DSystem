/* $Id: arch.h,v 1.6 1997/03/27 20:29:39 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/******************************************************************************
 *
 * File
 *    arch.h
 *
 * Author
 *    Kevin Cureton - major revision
 *
 * Date
 *    Aug 1994
 *
 * Description
 *	  Architecture naming and enumeration
 *
 *         Copyright 1990, 1991, 1992, 1993, 1994 Rice University
 *                         All Rights Reserved
 *
 ******************************************************************************/

#ifndef arch_h
#define arch_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

extern char* Rn_arch_name;  /* official local arch name defined in config.c */

typedef enum Architecture_enum
{
  ARCH_unknown = -1,
  ARCH_IBM3090 = 0,
  ARCH_sequent,
  ARCH_sun3,
  ARCH_sun4,
  ARCH_RT,
  ARCH_i860,
  ARCH_RS6k,
  ARCH_Cray,
  ARCH_Alpha,
  ARCH_iPSC
} 
Architecture;

const int ARCH_FIRST = ARCH_IBM3090;
const int ARCH_LAST = ARCH_iPSC;

    /* Convert an architecture value to its standard name */
EXTERN(char *, arch_to_name, (Architecture arch));

    /* Convert a standard name to an architecture value */
EXTERN(Architecture, name_to_arch, (char *name));

#endif
