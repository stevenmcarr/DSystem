/* $Id: arch.C,v 1.6 1997/03/11 14:27:38 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/******************************************************************************
 *
 * File
 *    arch.C
 *
 * Author
 *    Kevin Cureton - major revision
 *
 * Date
 *    August 1994
 *
 * Description
 *    Architecture naming and enumeration
 *    
 *
 *         Copyright 1990, 1991, 1992, 1993, 1994 Rice University
 *                         All Rights Reserved
 *
 ******************************************************************************/

#include <string.h>

#include <libs/config/arch.h>

    /* official architecture names */
static char* arch_names[] = 
{
  "IBM3090",
  "sequent",
  "sun3",
  "sun4",
  "RT",
  "i860",
  "RS6k",
  "cray",
  "iPSC"
};


    /* Convert an architecture enumerator to its standard name */
char*
arch_to_name(Architecture arch)
{
  if (arch < ARCH_FIRST || arch > ARCH_LAST) return (char*)0;
  else return arch_names[(int)arch];
}


    /* Convert a standard architecture name to its architecture enumerator */
Architecture
name_to_arch(char* name)
{
  int arch;

  for (arch = ARCH_FIRST; arch <= ARCH_LAST; arch++)
    {
      if (strcmp(arch_names[arch], name) == 0) return (Architecture)arch;
    }

  return ARCH_unknown;
}
