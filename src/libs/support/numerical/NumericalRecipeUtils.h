/* $Id: NumericalRecipeUtils.h,v 1.1 1997/03/11 14:37:05 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/* $Id: NumericalRecipeUtils.h,v 1.1 1997/03/11 14:37:05 carr Exp $
*/

/*
 * External declarations for Numerical Recipes utility routines. Most of
 * these are support routines -- see nrutil.c for a description of the
 * useful routines.
 */

EXTERN(void, nrutil_error,
		(char *err_text));
EXTERN(float *, nrutil_alloc_vector,
		(int nl, int nh));
EXTERN(int *, nrutil_alloc_ivector,
		(int nl, int nh));
EXTERN(double *, nrutil_alloc_dvector,
		(int nl, int nh));
EXTERN(float * *,nrutil_alloc_matrix,
		(int nrl, int nrh, int ncl, int nch));
EXTERN(double * *,nrutil_alloc_dmatrix,
		(int nrl, int nrh, int ncl, int nch));
EXTERN(int * *,nrutil_alloc_imatrix,
		(int nrl, int nrh, int ncl, int nch));
EXTERN(float * *,nrutil_alloc_submatrix, 
		(float **a, int oldrl, int oldrh, int oldcl,
		 int oldch, int newrl, int newcl));
EXTERN(void, nrutil_free_vector,
		(float *v, int nl, int nh));
EXTERN(void, nrutil_free_ivector, 
		(int *v, int nl, int nh));
EXTERN(void, nrutil_free_dvector,
		(double *v, int nl, int nh));
EXTERN(void, nrutil_free_matrix,
		(float **m, int nrl, int nrh, int ncl, int nch));
EXTERN(void, nrutil_free_dmatrix,
		(double **m, int nrl, int nrh, int ncl, int nch));
EXTERN(void, nrutil_free_imatrix,
		(int **m, int nrl, int nrh, int ncl, int nch));
EXTERN(void, nrutil_free_submatrix,
		(float **b, int nrl, int nrh, int ncl, int nch));
EXTERN(float * *,nrutil_convert_matrix,
		(float *a, int nrl, int nrh, int ncl, int nch));
EXTERN(void, nrutil_free_convert_matrix,
		(float **b, int nrl, int nrh, int ncl, int nch));
EXTERN(void, nrutil_fit,
                (float x[], float y[], int ndata, float sig[], int mwt,
                 float *a, float *b, float *siga, float *sigb, float *chi2,
                 float *q));
EXTERN(void, numerical_recipes_fit,
                (float x[], float y[], int ndata,
		 float sig[], int mwt,
                 float *a, float *b, float *siga, float *sigb,
                 float *chi2, float *q));
