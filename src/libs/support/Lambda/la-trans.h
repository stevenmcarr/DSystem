/****************************************************************************
 * Copyright (C) 1993, 1992, 1991 Cornell University -- The Typhoon Project
 *
 * la-trans.h,v 1.1.1.1 1993/09/23 19:09:39 stodghil Exp
 *
 *  Transformation Construction Module
 *       This module provides routines to construct
 *    a transformation matrix.
 *
 * Originally written by Wei Li.
 *
 * la-trans.h,v
 * Revision 1.1.1.1  1993/09/23  19:09:39  stodghil
 * Wei's Lambda Toolkit.
 *
 *
 ****************************************************************************/

#ifndef la_trans_h
#define la_trans_h

/* transformation matrix */
typedef struct {
  int ** matrix;
  int row_size;
  int col_size;
  int denom;                       /* used to represent a rational 
				      matrix A/denom, where A is
				      always an interger matrix. */
} *LA_MATRIX_T, LA_MATRIX_T1;

#define LA_MATRIX(T)            ((T)->matrix)
#define LA_MATRIX_ROWSIZE(T)    ((T)->row_size)
#define LA_MATRIX_COLSIZE(T)    ((T)->col_size)
#define LA_MATRIX_DENOM(T)      ((T)->denom)


/*-----------------------------------------------------------------*/
/* Create a matrix struct with rowsize and colsize.                */
/*-----------------------------------------------------------------*/

LA_MATRIX_T LA_PROTO(la_matrix_new,(int rowsize,
				    int colsize));

/*-----------------------------------------------------------------*/
/* Create a matrix struct                                          */
/*-----------------------------------------------------------------*/

LA_MATRIX_T LA_PROTO(la_matrix_allocate,( void ));

/*-----------------------------------------------------------------*/
/* Free a matrix struct.                                           */
/*-----------------------------------------------------------------*/

void LA_PROTO(la_matrix_free,( LA_MATRIX_T matrix));

/*------------------------------------*/
/*                                    */
/*  Nonsingularity                    */
/*                                    */
/*------------------------------------*/

/*--------------------------------------------------------------------------*/
/* la_is_nonsingular: check if nonsingular.                                 */
/*--------------------------------------------------------------------------*/

int  LA_PROTO(la_is_nonsingular,(LA_MATRIX_T T));

/*--------------------------------------------------------------------------*/
/* la_is_fullrank: check if fulll row rank.                                 */
/*--------------------------------------------------------------------------*/

int  LA_PROTO(la_is_fullrank,(LA_MATRIX_T pT));

/*--------------------------------------------------------------------------*/
/* la_rank:  computes the rank of the matrix.                               */
/*--------------------------------------------------------------------------*/

int  LA_PROTO(la_rank,(LA_MATRIX_T pT));


/*--------------------------------------------------------------------------*/
/* la_base: computes the base matrix.                                       */
/*--------------------------------------------------------------------------*/

LA_MATRIX_T LA_PROTO(la_base,(LA_MATRIX_T pT));

/*--------------------------------------------------------------------------*/
/* la_padding: padding of the legal base matrix to an invertible matrix.    */
/*--------------------------------------------------------------------------*/

LA_MATRIX_T  LA_PROTO(la_padding,(LA_MATRIX_T pT));

/*--------------------------------------------------------------------------*/
/* la_inverse: computes the inverse of a transformation.                    */
/*--------------------------------------------------------------------------*/

LA_MATRIX_T  LA_PROTO(la_inverse,(LA_MATRIX_T T));


/*------------------------------------*/
/*                                    */
/*  Data Dependences                  */
/*                                    */
/*------------------------------------*/

/*--------------------------------------------------------------------------*/
/* la_is_legal: check if legal w.r.s.t dependences.                         */
/*--------------------------------------------------------------------------*/

int  LA_PROTO(la_is_legal,(LA_MATRIX_T T,
			   LA_DEP_M_T D));

/*--------------------------------------------------------------------------*/
/* la_is_legal_par:  check if legal.                                        */
/*--------------------------------------------------------------------------*/

int  LA_PROTO(la_is_legal_par,(LA_MATRIX_T pT,
			       LA_DEP_M_T D));

/*--------------------------------------------------------------------------*/
/* la_base_legal:computes the legal base matrix.                            */
/*--------------------------------------------------------------------------*/

LA_MATRIX_T  LA_PROTO(la_base_legal,(LA_MATRIX_T pT,
				     LA_DEP_M_T D));

/*--------------------------------------------------------------------------*/
/* la_padding_legal:padding of the legal base matrix w.r.t the dep matrix.  */
/*--------------------------------------------------------------------------*/

LA_MATRIX_T  LA_PROTO(la_padding_legal,(LA_MATRIX_T pT,
					LA_DEP_M_T D));


/*--------------------------------------------------------------------------*/
/* la_complete:  forms  the transformation matrix.                          */
/*--------------------------------------------------------------------------*/

LA_MATRIX_T LA_PROTO(la_complete,(LA_MATRIX_T pT,
				  LA_DEP_M_T D));





/*--------------------------------------------------------------------------*/
/* la_dep_delete_carried: delete the dep's carried by x.                    */
/*                        computes xD.                                      */
/*--------------------------------------------------------------------------*/

void  LA_PROTO(la_dep_delete_carried,(LA_DEP_M_T D,
				      la_vect x));

/*--------------------------------------------------------------------------*/
/* la_vec_X_dep: dot product of an integer vector and a dep vector.         */
/*--------------------------------------------------------------------------*/

LA_DEP_T LA_PROTO(la_vec_X_dep,(la_vect vec1,
				LA_DEP_V_T d,
				int dim));

/*--------------------------------------------------------------------------*/
/* la_matrix_X_depM: an integer matrix times  a dep matrix.                 */
/*--------------------------------------------------------------------------*/

LA_DEP_M_T  LA_PROTO(la_matrix_X_depM,(LA_MATRIX_T A,
				       LA_DEP_M_T D));


/*-----------------------------------------------------------------*/
/* end of la-trans.h                                               */
/*-----------------------------------------------------------------*/
#endif
