/* $Id: AnnotLink.C,v 1.4 1997/03/11 14:30:39 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * This module is part of the IP information browser in NED. In order for
 * the browser to work correctly, it has to be able to display all
 * available call graph node and edge annotations. As a result, all of the
 * annotation classes, etc. have to be linked into the NED excutable. This
 * module provides a mechanism for doing just that. This module also
 * controls the specific annotations which are made available within the
 * browser.
 * 
 * The main entry points are routines which return a list of the names of the
 * annotations which are available to display.
 * 
 * Instructions for adding new annotations to the browser:
 * 
 *	1) Add your annotation include file to the list below
 * 
 *	2) Add a reference to the global which holds your annotation name to
 *	either "edge_annot_list" or "node_annot_list" (or both) depending on
 *	whether yours is an edge or node annotation.
 * 
 *	3) Remake everything. You're done.
 * 
 * Author: N. McIntosh
 */

#include <stdio.h>
#include <libs/support/misc/general.h>
#include <libs/support/memMgmt/mem.h>
#include <libs/support/database/context.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/AnnotLink.h>

#if 0
#include <libs/ipAnalysis/problems/OBSOLETE/datarace/RaceAnnot.h>
#endif

#include <libs/ipAnalysis/problems/modRef/ScalarModRefAnnot.h>
#include <libs/ipAnalysis/problems/alias/AliasAnnot.h>
#include <libs/ipAnalysis/problems/rsd/AFormalAnnot.h>

#include <string.h>

#define GETMEMSTR "FortAnnot:AnnotLink"

/*
 * The following insures that the code for all the annotations gets linked
 * in.
 */

static char **edge_annot_list[] = {
#if 0
  &DRACE_INSTR_ANNOT,
  &DRACE_ALLOC_ANNOT,
  &DRACE_INITIAL_ANNOT,  
#endif
  &SCALAR_MOD_ANNOT,
  &SCALAR_REF_ANNOT,
  &ALIAS_ANNOT,
  &AFORMAL_ANNOT,
  0,
};

static char **node_annot_list[] = {
#if 0
  &DRACE_INSTR_ANNOT,
  &DRACE_ALLOC_ANNOT,
  &DRACE_INITIAL_ANNOT, 
#endif
  &ALIAS_ANNOT,
  &SCALAR_GMOD_ANNOT,
  &SCALAR_GREF_ANNOT,
  &AFORMAL_ANNOT,
  0,
};

char **
ipi_GetCGnodeAnnotList()
{
  int i, n = 0;
  char **t, **arr;

  for (i = 0; node_annot_list[i]; i++);
  n = i;
  arr = (char **) get_mem(sizeof(char *) * (n+1), GETMEMSTR);
  for (i = 0; i < n; i++) {
    t = node_annot_list[i];
    arr[i] = (char *) get_mem(strlen(*t)+1, GETMEMSTR);
    strcpy(arr[i], *t);
  }
  arr[n] = 0;
  return arr;
}

char **
ipi_GetCGedgeAnnotList()
{
  int i, n = 0;
  char **t, **arr;

  for (i = 0; edge_annot_list[i]; i++);
  n = i;
  arr = (char **) get_mem(sizeof(char *) * (n+1), GETMEMSTR);
  for (i = 0; i < n; i++) {
    t = edge_annot_list[i];
    arr[i] = (char *) get_mem(strlen(*t)+1, GETMEMSTR);
    strcpy(arr[i], *t);
  }
  arr[n] = 0;
  return arr;
} 
