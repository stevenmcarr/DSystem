/* $Id: la-utilities.h,v 1.2 1997/03/27 20:47:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/****************************************************************************
 * Copyright (C) 1993, 1992, 1991 Cornell University -- The Typhoon Project
 *
 * la-utilities.h,v 1.1.1.1 1993/09/23 19:09:40 stodghil Exp
 *
 * Other routines.
 *
 * Originally written by Wei Li.
 *
 * la-utilities.h,v
 * Revision 1.1.1.1  1993/09/23  19:09:40  stodghil
 * Wei's Lambda Toolkit.
 *
 *
 ****************************************************************************/

#ifndef la_utilities_h
#define la_utilities_h

#include <stdlib.h>
#include <malloc.h>
/*--------------------------------------------------------------------------*/
/* newVect: Allocate a vector of length n.                                  */
/*--------------------------------------------------------------------------*/

#define la_newVect(n, type)  (type *) calloc (n, sizeof(type))
#define la_vecNew(n)  (la_vect) calloc (n, sizeof(int))

/*--------------------------------------------------------------------------*/
/* la_matNew: Allocate a matrix of mxn.                                     */
/*--------------------------------------------------------------------------*/

la_matrix LA_PROTO(la_matNew,(int m,
			      int n));

/*--------------------------------------------------------------------------*/
/* Hermite operations.                                                      */
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/* la_eExchange: Elementary operation -- exchange to columns.               */
/* m is the number of rows.                                                 */
/*--------------------------------------------------------------------------*/

void LA_PROTO(la_eExchange, (la_matrix mat,
			     int m,
			     int col1,
			     int col2 ));

/*--------------------------------------------------------------------------*/
/* la_eAdd: Elementary operation -- add an integral multiple of column col1 */
/* to column col2.                                                          */
/*--------------------------------------------------------------------------*/

void LA_PROTO(la_eAdd, (la_matrix mat,
			int m,
			int col1,
			int col2,
			int fact ));

/*--------------------------------------------------------------------------*/
/* la_eNegate: Elementary operation -- negate one column.                   */
/*--------------------------------------------------------------------------*/

void LA_PROTO(la_eNegate, (la_matrix mat,
		 int m,
		 int col));

/*--------------------------------------------------------------------------*/
/* la_eColConst: Elementary operation -- mult one column by a const.        */
/*--------------------------------------------------------------------------*/

void LA_PROTO(la_eColConst, (la_matrix mat,
			     int m,
			     int col,
			     int fact));

/*--------------------------------------------------------------------------*/
/* la_eMinInVec: computes the min non-zero in the vector from start..n.     */
/*--------------------------------------------------------------------------*/

int LA_PROTO(la_eMinInVec, (la_vect vec,
			    int n,
			    int start));

/*--------------------------------------------------------------------------*/
/* la_eFirstNonZero: return the first non-zero in the postfix i--n.         */
/*                return  n if all zeroes.                                  */
/*--------------------------------------------------------------------------*/

int LA_PROTO(la_eFirstNonZero, (la_vect vec,
				int n,
				int i));

/*--------------------------------------------------------------------------*/
/* la_matHermite:   apply elementary column operations to decompose         */
/* mat to a product of a lower triangular and a unimodular matrix.          */
/*--------------------------------------------------------------------------*/

void LA_PROTO(la_matHermite, (la_matrix T,
			      int depth,
			      la_matrix H,
			      la_matrix U));

/*--------------------------------------------------------------------------*/
/* la_eFirstNonZeroVect: computes the first non-zero vector.                */
/*--------------------------------------------------------------------------*/

int LA_PROTO(la_eFirstNonZeroVect, (la_matrix mat,
				    int rowSize,
				    int colSize,
				    int startRow));


/*--------------------------------------------------------------------------*/
/* matrix operations.                                                       */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* la_matId: return an identity matrix.                                     */
/*--------------------------------------------------------------------------*/

void LA_PROTO(la_matId, (la_matrix mat,
			 int n));

/*--------------------------------------------------------------------------*/
/* la_matCopy: make copy of the matrix.                                     */
/*--------------------------------------------------------------------------*/

void LA_PROTO(la_matCopy, (la_matrix mat1,
			   la_matrix mat2,
			   int m, int n));

/*--------------------------------------------------------------------------*/
/* la_matNegate:  matrix negated.                                           */
/*--------------------------------------------------------------------------*/

void LA_PROTO(la_matNegate, (la_matrix mat1,
			     la_matrix mat2,
			     int m,
			     int n));

/*--------------------------------------------------------------------------*/
/* la_matT: make transpose of the matrix.                                   */
/*--------------------------------------------------------------------------*/

void LA_PROTO(la_matT, (la_matrix mat1,
			la_matrix mat2,
			int m,
			int n));

/*--------------------------------------------------------------------------*/
/* la_matrix_is_id: is an identity matrix?                                  */
/*--------------------------------------------------------------------------*/

int LA_PROTO(la_matrix_is_id, (LA_MATRIX_T M));

/*--------------------------------------------------------------------------*/
/* la_matAdd: add two matrices.                                             */
/*--------------------------------------------------------------------------*/

void LA_PROTO(la_matAdd, (la_matrix mat1,
			  la_matrix mat2,
			  la_matrix mat3, int m, int n));

/*--------------------------------------------------------------------------*/
/* la_matAddF: add two matrices with constant factors.                      */
/*--------------------------------------------------------------------------*/

void LA_PROTO(la_matAddF, (la_matrix mat1,
			   int f1,
			   la_matrix mat2,
			   int f2,
			   la_matrix mat3,
			   int m,
			   int n));

/*--------------------------------------------------------------------------*/
/* la_matMult: mult two matrices.                                           */
/*--------------------------------------------------------------------------*/

void LA_PROTO(la_matMult, (la_matrix mat1, 
			   la_matrix mat2,
			   la_matrix mat3,
			   int m,
			   int r,
			   int n));

/*--------------------------------------------------------------------------*/
/* la_matGetCol: get a column from the matrix.                              */
/*--------------------------------------------------------------------------*/

void LA_PROTO(la_matGetCol, (la_matrix mat,
			     int m, 
			     int col,
			     la_vect vec));

/*--------------------------------------------------------------------------*/
/* la_matConcat: concat two matrices.                                       */
/*--------------------------------------------------------------------------*/

void LA_PROTO(la_matConcat, (la_matrix mat1,
			     int m1, 
			     la_matrix mat2, 
			     int m2,  
			     la_matrix mat3));


/*--------------------------------------------------------------------------*/
/* la_rowExchange:  -- exchange two rows.                                   */
/*--------------------------------------------------------------------------*/

void LA_PROTO(la_rowExchange,  (la_matrix mat,
				int i1,
				int  i2 ));



/*--------------------------------------------------------------------------*/
/* la_rowAdd:  -- adds a multiple of i1 to i2.                              */
/*--------------------------------------------------------------------------*/

void LA_PROTO(la_rowAdd,  (la_matrix mat,
			   int n,
			   int i1, 
			   int i2, 
			   int fact));

/*--------------------------------------------------------------------------*/
/* la_matInv:  computes the inverse  of the matrix.                         */
/*--------------------------------------------------------------------------*/

int LA_PROTO(la_matInv, (la_matrix mat, 
			 la_matrix inv,
			 int n));

/*--------------------------------------------------------------------------*/
/* la_eDeleteRows: Delete row r1 upto r2 (not include r2).                  */
/*--------------------------------------------------------------------------*/

void LA_PROTO(la_eDeleteRows, (la_matrix matrix,
			       int m, 
			       int r1, 
			       int r2 ));


/*--------------------------------------------------------------------------*/
/* la_projToNull: projection of Ek to the null space of B.                  */
/*--------------------------------------------------------------------------*/

void  LA_PROTO(la_projToNull, (la_matrix B, 
			       int rowSize,
			       int colSize,
			       int k,
			       la_vect x));

/*--------------------------------------------------------------------------*/
/* la_matFree: free a matrix.                                               */
/*--------------------------------------------------------------------------*/

void  LA_PROTO(la_matFree, (la_matrix mat,
			    int m,
			    int n));


/*--------------------------------------------------------------------------*/
/* vector operations.                                                       */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* la_vecNegate:  -- negate one vector.                                     */
/*--------------------------------------------------------------------------*/

void LA_PROTO(la_vecNegate,  (la_vect vec1, 
			      la_vect vec2,
			      int n));

/*--------------------------------------------------------------------------*/
/* la_vecConst:    mult vector by a constant.                               */
/*--------------------------------------------------------------------------*/

void LA_PROTO(la_vecConst,  (la_vect vec1,
			     la_vect vec2,
			     int n,
			     int c));

/*--------------------------------------------------------------------------*/
/* la_vecAdd: adds two vectors.                                             */
/*--------------------------------------------------------------------------*/

void LA_PROTO(la_vecAdd, (la_vect vec1,
			  la_vect vec2, 
			  la_vect vec3,
			  int n));

/*--------------------------------------------------------------------------*/
/* la_vecAddF: adds two vectors.                                            */
/*--------------------------------------------------------------------------*/

void LA_PROTO(la_vecAddF, (la_vect vec1, 
			   int f1, 
			   la_vect vec2,
			   int f2,
			   la_vect vec3,
			   int n));

/*--------------------------------------------------------------------------*/
/* la_vecConcat: concat two vectors.                                        */
/*--------------------------------------------------------------------------*/

void  LA_PROTO(la_vecConcat, (la_vect vec1, 
			      int n1, 
			      la_vect vec2, 
			      int n2, 
			      la_vect vec3));

/*--------------------------------------------------------------------------*/
/* la_matConcat_newM: concat two matrices. create a new mat.                */
/*--------------------------------------------------------------------------*/

void LA_PROTO(la_matConcat_newM, (la_matrix mat1,
				  int m1, 
				  int col,
				  la_matrix mat2, 
				  int m2,  
				  la_matrix mat3));

/*--------------------------------------------------------------------------*/
/* la_vecCopy: make copy of the vector.                                     */
/*--------------------------------------------------------------------------*/

void LA_PROTO(la_vecCopy, (la_vect vec1, 
			   la_vect vec2,
			   int n));

/*--------------------------------------------------------------------------*/
/* la_vecIsZero: check if it is zero vector.                                */ 
/*--------------------------------------------------------------------------*/

int LA_PROTO(la_vecIsZero, (la_vect vec1,
			    int n));

/*--------------------------------------------------------------------------*/
/* la_vecClear:  clear a vector.                                            */ 
/*--------------------------------------------------------------------------*/

void LA_PROTO(la_vecClear, (la_vect vec1,
			    int n));

/*--------------------------------------------------------------------------*/
/* la_vecEq:  check if two vectors are equal.                               */
/*--------------------------------------------------------------------------*/

int LA_PROTO(la_vecEq, (la_vect vec1,
			la_vect vec2,
			int len));

/*--------------------------------------------------------------------------*/
/* la_freeV: free a vector.                                                 */
/*--------------------------------------------------------------------------*/

void  LA_PROTO(la_vecFree, (la_vect vec));


/*--------------------------------------------------------------------------*/
/* vector/matrix operations.                                                */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* la_matVecMult: mult a matrix and a vec.                                  */
/*--------------------------------------------------------------------------*/

void LA_PROTO(la_matVecMult, (la_matrix mat,
			      int m, 
			      int n,
			      la_vect vec,
			      la_vect  vec1));

/*--------------------------------------------------------------------------*/
/* la_vecMatMult: mult  a vec by a matrix.                                  */
/*--------------------------------------------------------------------------*/

void LA_PROTO(la_vecMatMult, (la_vect vec,
			      int m,
			      la_matrix mat,
			      int  n, 
			      la_vect vec1));

/*--------------------------------------------------------------------------*/
/* elementary integer operations.                                           */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* la_gcd: computes the gcd of two integers.                                */
/*--------------------------------------------------------------------------*/

int LA_PROTO(la_gcd, (int a,
		      int b));

/*--------------------------------------------------------------------------*/
/* la_gcdV: computes the gcd of an array of integers.                       */
/*--------------------------------------------------------------------------*/

int LA_PROTO(la_gcdV, (la_vect v,
		       int n ));

/*--------------------------------------------------------------------------*/
/* la_lcm: computes the lcm of two integers.                                */
/*--------------------------------------------------------------------------*/

int LA_PROTO(la_lcm, (int a,
		      int b));

/*--------------------------------------------------------------------------*/
/* la_abs: computes the abs of an integer.                                  */
/*--------------------------------------------------------------------------*/

int LA_PROTO(la_abs, (int a));


/*--------------------------------------------------------------------------*/
/* matrix print routines.                                                   */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* la_printM: prints a matrix.                                              */
/*--------------------------------------------------------------------------*/

void LA_PROTO(la_printM, (la_matrix mat,
			  int m,
			  int n ));

/*--------------------------------------------------------------------------*/
/* la_printV: prints a vector.                                              */
/*--------------------------------------------------------------------------*/

void LA_PROTO(la_printV, (la_vect vec,
			  int n ));

/*--------------------------------------------------------------------------*/
/* la_printE: prints an expression.                                         */
/*--------------------------------------------------------------------------*/

int LA_PROTO(la_printE, (la_vect e,
			 int n,
			 int c,
			 la_vect blob_list,
			 int blobs,
			 int d,
			 int start,
			 int cf));
/*--------------------------------------------------------------------------*/
/* la_printLinearE: prints a linear expression.                             */
/*--------------------------------------------------------------------------*/

int LA_PROTO(la_printLinearE, (la_vect e,
			       int n,
			       int start));

/*--------------------------------------------------------------------------*/
/* la_printDirV: prints a direction vect.                                   */
/*--------------------------------------------------------------------------*/

void LA_PROTO(la_printDirV, ( LA_DIR_T *vec, int m ));

/*--------------------------------------------------------------------------*/
/* loop bounds and body.                                                    */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* la_aux: computes the loop bounds for the aux space.                      */
/*         Input system is Ax<=b. U is a unimodular transformation.         */
/*--------------------------------------------------------------------------*/

LA_LOOPNEST_T LA_PROTO(la_aux, (LA_LOOPNEST_T nest,
				LA_MATRIX_T U));


/*--------------------------------------------------------------------------*/
/* la_tar: computes the loop bounds for the target space.                   */
/*         Input:  bounds of the aux space.                                 */
/*         Output: new set of piece-wise linear bounds and linear offsets.  */
/*--------------------------------------------------------------------------*/

LA_LOOPNEST_T LA_PROTO(la_tar, (LA_LOOPNEST_T nest_aux,
				LA_MATRIX_T H,
				la_vect stepS));


/*--------------------------------------------------------------------------*/
/* la_step_sign: keep track of step signs.                                  */
/*--------------------------------------------------------------------------*/

la_vect  LA_PROTO(la_step_signs, (LA_MATRIX_T T,
				  la_vect S));

/*--------------------------------------------------------------------------*/
/* dependences.                                                             */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* la_dep_vec_copy:                                                         */
/*   copy a dependence vector.                                              */
/*--------------------------------------------------------------------------*/

void LA_PROTO(la_dep_vec_copy, (LA_DEP_V_T d,
				LA_DEP_V_T d1,
				int dim));

/*-----------------------------------------------------------------*/
/* la_dep_add_to_list:                                             */
/*   add the dependence vector to the matrix.                      */
/*-----------------------------------------------------------------*/

void LA_PROTO(la_dep_add_to_list, (LA_DEP_V_T d,
				   LA_DEP_M_T D));

/*-----------------------------------------------------------------*/
/* la_dep_add_to_list_last:                                        */
/*   add the dependence vector to the matrix (last in the list).   */
/*-----------------------------------------------------------------*/

void LA_PROTO(la_dep_add_to_list_last, (LA_DEP_V_T d,
					LA_DEP_M_T D,
					LA_DEP_V_T last));

/*--------------------------------------------------------------------------*/
/* la_allNegEq: check if every element is neg or zero.                      */
/*--------------------------------------------------------------------------*/

int LA_PROTO(la_allNegEq, (LA_DIR_T *flagV,
			   int flagSize));

/*--------------------------------------------------------------------------*/
/* la_allPosEq: check if every element is positive or zero.                 */
/*--------------------------------------------------------------------------*/

int LA_PROTO(la_allPosEq, (LA_DIR_T *flagV, 
			   int flagSize));

/*--------------------------------------------------------------------------*/
/* la_flagNegate: negate the dep flag.                                      */
/*--------------------------------------------------------------------------*/

void  LA_PROTO(la_flagNegate, (LA_DIR_T *flagV, 
			       int flagSize));


/*--------------------------------------------------------------------------*/
/* la_vec_depVec_mult: dot product of an integer vector and a dep vector.   */
/*--------------------------------------------------------------------------*/

LA_DIR_T LA_PROTO(la_vec_depVec_mult, (la_vect vec1,
				       LA_DEP_V_T d,
				       int dim));

/*--------------------------------------------------------------------------*/
/* la_vec_depV_mult: dot product of an integer vector and a dep vector.     */
/*--------------------------------------------------------------------------*/

LA_DEP_T LA_PROTO(la_vec_depV_mult, (la_vect vec1,
				     LA_DEP_V_T d,
				     int dim));

/*--------------------------------------------------------------------------*/
/* la_depMult: dot product of an integer vector and a dep matrix.           */
/*--------------------------------------------------------------------------*/

void  LA_PROTO(la_depMult, (la_vect vec1,
			    LA_DEP_M_T D,
			    LA_DIR_T *prod));

/*--------------------------------------------------------------------------*/
/* la_firstEk: return the first Ek within 90 degrees of every dep.          */
/*--------------------------------------------------------------------------*/

int  LA_PROTO(la_firstEk, (LA_DEP_M_T D));

/*--------------------------------------------------------------------------*/
/* la_delPosDep: delete the dependence with the positive flag.              */
/*--------------------------------------------------------------------------*/

void LA_PROTO(la_delPosDep, (LA_DEP_M_T D,
			     LA_DIR_T * flagV));

/*--------------------------------------------------------------------------*/
/* la_dep_vec_eq: check if two dependence vectors are equal.                */
/*--------------------------------------------------------------------------*/

int LA_PROTO(la_dep_vec_eq, ( LA_DEP_V_T d1, LA_DEP_V_T d2, int size));

/*--------------------------------------------------------------------------*/
/* la_dep_eq: check if two dependences are equal.                           */
/*--------------------------------------------------------------------------*/

int LA_PROTO(la_dep_eq, ( LA_DEP_T d1, LA_DEP_T d2));

















/*-----------------------------------------------------------------*/
/* end of  la-utilities.h:                                         */
/*-----------------------------------------------------------------*/
#endif
