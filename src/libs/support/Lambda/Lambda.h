/* $Id: Lambda.h,v 1.2 1997/03/27 20:47:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/****************************************************************************
 * Copyright (C) 1993, 1992, 1991 Cornell University -- The Typhoon Project
 *
 * Lambda.h,v 1.1.1.1 1993/09/23 19:09:35 stodghil Exp
 *
 *  Lambda transformation toolkit
 *       This is the header file for the toolkit.
 *
 * Originally written by Wei Li.
 *
 * Lambda.h,v
 * Revision 1.1.1.1  1993/09/23  19:09:35  stodghil
 * Wei's Lambda Toolkit.
 *
 *
 ****************************************************************************/

#ifndef Lambda_h
#define Lambda_h

#include <malloc.h>

/*
 * Function declarations will be in the form,
 *
 *    type LA_PROTO(fname, (type arg, ...));
 *
 * In ANSI C and C++, this will expand to
 *
 *    type fname(type arg, ...);
 *
 * In K&R C, this will expand to
 *
 *    type fname();
 *
 */
#if defined(__STDC__) || defined(__cplusplus)
#define LA_PROTO(name, args) name args
#else
#define LA_PROTO(name, args) name()
#endif

#ifdef __cplusplus

/* C++ Definitions */

extern "C" {
#endif

typedef int * la_vect;

typedef la_vect * la_matrix;

#include <libs/support/Lambda/la-dep.h>
#include <libs/support/Lambda/la-trans.h>
#include <libs/support/Lambda/la-code.h>
#include <libs/support/Lambda/la-print.h>
#include <libs/support/Lambda/la-utilities.h>

#ifdef __cplusplus
}
#endif

/*----------------------------------------------------------------*/
/*  Add the following just for convenience in PALA and friends    */
/*----------------------------------------------------------------*/
#define     TRUE      1
#define     FALSE   0

/*-----------------------------------------------------------------*/
/* end of Lambda.h                                                 */
/*-----------------------------------------------------------------*/

#endif
