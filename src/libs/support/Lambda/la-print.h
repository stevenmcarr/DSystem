/* $Id: la-print.h,v 1.2 1997/03/27 20:47:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/****************************************************************************
 * Copyright (C) 1993, 1992, 1991 Cornell University -- The Typhoon Project
 *
 * la-print.h,v 1.1.1.1 1993/09/23 19:09:39 stodghil Exp
 *
 *  Print Module
 *       This module provides routines to print out various data
 *    types. It is useful for debugging.
 *
 * Originally written by Wei Li.
 *
 * la-print.h,v
 * Revision 1.1.1.1  1993/09/23  19:09:39  stodghil
 * Wei's Lambda Toolkit.
 *
 *
 ****************************************************************************/

#ifndef la_print_h
#define la_print_h

/*-----------------------------------------------------------------*/
/*                                                                 */
/*  la-print.h:                                                    */
/*                                                                 */
/*  Author: Wei Li                                                 */
/*          Cornell University                                     */
/*  Version 0:  August 1992                                        */
/*  Version 1:  May    1993                                        */
/*-----------------------------------------------------------------*/

/*-----------------------------------------------------------------*/
/* Print a matrix.                                                 */
/*-----------------------------------------------------------------*/

void LA_PROTO(la_matrix_print,( LA_MATRIX_T mat ));


/*-----------------------------------------------------------------*/
/* Print a loop nest.                                              */
/* If the start is i, then index variables are i, j, k..           */
/* If the start is u, then index variables are u, v, w..           */
/* '/' represents floor, and '\' represents ceiling.               */
/*-----------------------------------------------------------------*/

void LA_PROTO(la_nest_print,( LA_LOOPNEST_T nest, int start));

/*-----------------------------------------------------------------*/
/* Print a loop.                                                   */
/*-----------------------------------------------------------------*/

void LA_PROTO(la_loop_print,( LA_LOOP_T loop, int depth, int blobs,
			     int start)); 


/*-----------------------------------------------------------------*/
/* Print an expr.                                                  */
/*-----------------------------------------------------------------*/

void LA_PROTO(la_expr_print,( LA_EXPR_T expr, int depth, int blobs,
			     int start));

/*-----------------------------------------------------------------*/
/* Print a dependence.                                             */
/*-----------------------------------------------------------------*/

void LA_PROTO(la_dep_print,(LA_DEP_T d));

/*-----------------------------------------------------------------*/
/* Print a dependence vector.                                      */
/*-----------------------------------------------------------------*/

void LA_PROTO(la_dep_vec_print,(LA_DEP_V_T d,
				int dim));

/*-----------------------------------------------------------------*/
/* Print a dependence matrix.                                      */
/*-----------------------------------------------------------------*/

void LA_PROTO(la_dep_matrix_print,( LA_DEP_M_T D));


















/*-----------------------------------------------------------------*/
/* end of print.h                                                  */
/*-----------------------------------------------------------------*/
#endif
