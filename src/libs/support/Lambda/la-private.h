/* $Id: la-private.h,v 1.3 1997/04/07 13:40:08 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/****************************************************************************
 * Copyright (C) 1993, 1992, 1991 Cornell University -- The Typhoon Project
 *
 * la-private.h,v 1.2 1993/09/25 19:29:53 stodghil Exp
 *
 * Lambda private stuff.
 *
 * Originally written by Wei Li.
 *
 * la-private.h,v
 * Revision 1.2  1993/09/25  19:29:53  stodghil
 * Declarations of fprintf, etc. are surrounded by "#ifdef LA_SunOS".
 *
 * Revision 1.1.1.1  1993/09/23  19:09:41  stodghil
 * Wei's Lambda Toolkit.
 *
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>

#include <libs/support/Lambda/Lambda.h>

#if (defined(LA_SunOS) && !defined(LA_sun4m))
int LA_PROTO(fprintf,(FILE *fp, char *format, ...));
int LA_PROTO(printf,(char *format, ...));
FILE  *LA_PROTO(fopen,(char *, char *));
#endif

#define     TOP      1
#define     BOTTOM   0

#define     TRUE      1
#define     FALSE   0

#define la_print    0

typedef struct {
  la_matrix base;
  int dim;                           /* dimention */
  la_vect origin;                    /* constant origin */
  la_matrix origin_blob;             /* blobs in origin */
  int blobs;                         /* number of blobs */
} *LA_LATTICE_T, LA_LATTICE_T1;

#define LA_LATTICE_BASE(l)           ((l)->base)
#define LA_LATTICE_DIM(l)            ((l)->dim)
#define LA_LATTICE_ORIGIN(l)         ((l)->origin)
#define LA_LATTICE_ORIGIN_BLOB(l)    ((l)->origin_blob)
#define LA_LATTICE_BLOBS(l)          ((l)->blobs)


/*--------------------------------------------------------------------------*/
/* lattice operations.                                                      */
/*--------------------------------------------------------------------------*/

LA_LATTICE_T LA_PROTO(la_lattice,(LA_LOOPNEST_T nest));

LA_LATTICE_T LA_PROTO(la_lattice_new,(int depth, int blobs));

void LA_PROTO(la_lattice_print,( LA_LATTICE_T lattice));


