/* $Id: NumericalRecipeUtils.C,v 1.1 1997/06/25 15:17:54 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/* $Id: NumericalRecipeUtils.C,v 1.1 1997/06/25 15:17:54 carr Exp $
*/

/*
 * Utility routines copied from "Numerical Recipes in C", by Press et al.
 * Most of this module consists of support routines; the main useful
 * routines is a chi-squared fit routine ("nrutil_fit()"). 
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <libs/support/misc/general.h>
#include <libs/support/memMgmt/mem.h>
#include <libs/support/numerical/NumericalRecipeUtils.h>

#define NRUTIL_GETMEM "Numerical Recipes Utility Routines"

static float sqrarg;

#define SQR(a) (sqrarg=(a),sqrarg*sqrarg)

void nrutil_error(char* error_text)
{
  fprintf(stderr, "Numerical Recipes run-time error...\n");
  fprintf(stderr, "%s\n", error_text);
  fprintf(stderr, "...now exiting to system...\n");
  exit(1);
}

float *nrutil_alloc_vector(int nl, int nh)
{
  float *v;
  
  v = (float *) get_mem((unsigned) (nh - nl + 1) * sizeof(float),
			NRUTIL_GETMEM);
  if (!v)
    nrutil_error("allocation failure in nrutil_alloc_vector()");
  return v - nl;
}

int *nrutil_alloc_ivector(int nl, int nh)
{
  int *v;
  
  v = (int *) get_mem((unsigned) (nh - nl + 1) * sizeof(int), NRUTIL_GETMEM);
  if (!v)
    nrutil_error("allocation failure in nrutil_alloc_ivector()");
  return v - nl;
}

double *nrutil_alloc_dvector(int nl, int nh)
{
  double *v;
  
  v = (double *) get_mem((unsigned) (nh - nl + 1) * sizeof(double),
			 NRUTIL_GETMEM);
  if (!v)
    nrutil_error("allocation failure in nrutil_alloc_dvector()");
  return v - nl;
}

float **nrutil_alloc_matrix(int nrl, int nrh, int ncl, int nch)
{
  int i;
  float **m;
  
  m = (float **) get_mem((unsigned) (nrh - nrl + 1) * sizeof(float *),
			 NRUTIL_GETMEM);
  if (!m)
    nrutil_error("allocation failure 1 in nrutil_alloc_matrix()");
  m -= nrl;
  
  for (i = nrl; i <= nrh; i++) {
    m[i] = (float *) get_mem((unsigned) (nch - ncl + 1) * sizeof(float),
			     NRUTIL_GETMEM);
    if (!m[i])
      nrutil_error("allocation failure 2 in nrutil_alloc_matrix()");
    m[i] -= ncl;
  }
  return m;
}

double **nrutil_alloc_dmatrix(int nrl, int nrh, int ncl, int nch)
{
  int i;
  double **m;
  
  m = (double **) get_mem((unsigned) (nrh - nrl + 1) * sizeof(double *),
			  NRUTIL_GETMEM);
  if (!m)
    nrutil_error("allocation failure 1 in nrutil_alloc_dmatrix()");
  m -= nrl;
  
  for (i = nrl; i <= nrh; i++) {
    m[i] = (double *) get_mem((unsigned) (nch - ncl + 1) * sizeof(double),
			      NRUTIL_GETMEM);
    if (!m[i])
      nrutil_error("allocation failure 2 in nrutil_alloc_dmatrix()");
    m[i] -= ncl;
  }
  return m;
}

int **nrutil_alloc_imatrix(int nrl, int nrh, int ncl, int nch)
{
  int i, **m;
  
  m = (int **) get_mem((unsigned) (nrh - nrl + 1) * sizeof(int *),
		       NRUTIL_GETMEM);
  if (!m)
    nrutil_error("allocation failure 1 in nrutil_alloc_imatrix()");
  m -= nrl;
  
  for (i = nrl; i <= nrh; i++) {
    m[i] = (int *) get_mem((unsigned) (nch - ncl + 1) * sizeof(int),
			   NRUTIL_GETMEM);
    if (!m[i])
      nrutil_error("allocation failure 2 in nrutil_alloc_imatrix()");
    m[i] -= ncl;
  }
  return m;
}

float **nrutil_alloc_submatrix(float **a, int oldrl, int oldrh, int oldcl, 
                               int oldch, int newrl, int newcl)
{
  int i, j;
  float **m;
  
  m = (float **) get_mem((unsigned) (oldrh - oldrl + 1) * sizeof(float *),
			 NRUTIL_GETMEM);
  if (!m)
    nrutil_error("allocation failure in nrutil_alloc_submatrix()");
  m -= newrl;
  
  for (i = oldrl, j = newrl; i <= oldrh; i++, j++)
    m[j] = a[i] + oldcl - newcl;
  
  return m;
}

void nrutil_free_vector(float *v, int nl, int nh)
{
  free_mem((void*) (v+nl));
}

void nrutil_free_ivector(int *v, int nl, int nh)
{
  free_mem((void*) (v+nl));
}

void nrutil_free_dvector(double *v, int nl, int nh)
{
  free_mem((void*) (v+nl));
}

void nrutil_free_matrix(float **m, int nrl, int nrh, int ncl, int nch)
{
  int i;
  
  for (i = nrh; i >= nrl; i--)
    free_mem((void *) (m[i] + ncl));
  free_mem((void *) (m + nrl));
}

void nrutil_free_dmatrix(double **m, int nrl, int nrh, int ncl, int nch)
{
  int i;
  
  for (i = nrh; i >= nrl; i--)
    free_mem((void *) (m[i] + ncl));
  free_mem((void *) (m + nrl));
}

void nrutil_free_imatrix(int **m, int nrl, int nrh, int ncl, int nch)
{
  int i;
  
  for (i = nrh; i >= nrl; i--)
    free_mem((void *) (m[i] + ncl));
  free_mem((void *) (m + nrl));
}

void nrutil_free_submatrix(float **b, int nrl, int nrh, int ncl, int nch)
{
  free_mem((void*) (b+nrl));
}

float **nrutil_convert_matrix(float *a, int nrl, int nrh, int ncl, int nch)
{
  int i, j, nrow, ncol;
  float **m;
  
  nrow = nrh - nrl + 1;
  ncol = nch - ncl + 1;
  m = (float **) get_mem((unsigned) (nrow) * sizeof(float *), NRUTIL_GETMEM);
  if (!m)
    nrutil_error("allocation failure in nrutil_convert_matrix()");
  m -= nrl;
  for (i = 0, j = nrl; i <= nrow - 1; i++, j++)
    m[j] = a + ncol * i - ncl;
  return m;
}

void nrutil_free_convert_matrix(float **b, int nrl, int nrh, int ncl, int nch)
{
  free_mem((void*) (b+nrl));
}

/*-------------------------------------------------------------------------
  Chi-squared fit routine, taken from Numerical Recipes in C,
  page 527. Read that page for more info on how this routine works.
  --------------------------------------------------------------------------*/
void nrutil_fit(float x[], float y[], int ndata, float sig[], int mwt, float *a, 
                float *b, float *siga, float *sigb, float *chi2, float *q)
{
  int i;
  float wt, t, sxoss, sx = 0.0, sy = 0.0, st2 = 0.0, ss, sigdat;
  /** float gammq(); **/
  
  *b = 0.0;
  if (mwt) {
    ss = 0.0;
    for (i = 1; i <= ndata; i++) {
      wt = 1.0 / SQR(sig[i]);
      ss += wt;
      sx += x[i] * wt;
      sy += y[i] * wt;
    }
  } else {
    for (i = 1; i <= ndata; i++) {
      sx += x[i];
      sy += y[i];
    }
    ss = ndata;
  }
  sxoss = sx / ss;
  if (mwt) {
    for (i = 1; i <= ndata; i++) {
      t = (x[i] - sxoss) / sig[i];
      st2 += t * t;
      *b += t * y[i] / sig[i];
    }
  } else {
    for (i = 1; i <= ndata; i++) {
      t = x[i] - sxoss;
      st2 += t * t;
      *b += t * y[i];
    }
  }
  *b /= st2;
  *a = (sy - sx * (*b)) / ss;
  *siga = sqrt((1.0 + sx * sx / (ss * st2)) / ss);
  *sigb = sqrt(1.0 / st2);
  *chi2 = 0.0;
  if (mwt == 0) {
    for (i = 1; i <= ndata; i++)
      *chi2 += SQR(y[i] - (*a) - (*b) * x[i]);
    *q = 1.0;
    sigdat = sqrt((*chi2) / (ndata - 2));
    *siga *= sigdat;
    *sigb *= sigdat;
  } else {
    for (i = 1; i <= ndata; i++) {
      *chi2 += SQR((y[i] - (*a) - (*b) * x[i]) / sig[i]);
    }
    
    /*** (*q) = gammq(0.5*(ndata-2),0.5*(*chi2));  ***/
  }
}



