/****************************************************************************
 * Copyright (C) 1993, 1992, 1991 Cornell University -- The Typhoon Project
 *
 * la-vecMat.c,v 1.1.1.1 1993/09/23 19:09:40 stodghil Exp
 *
 * Routines for vector and matrix operations.
 *
 * Originally written by Wei Li.
 *
 * la-vecMat.c,v
 * Revision 1.1.1.1  1993/09/23  19:09:40  stodghil
 * Wei's Lambda Toolkit.
 *
 *
 ****************************************************************************/

#include <la-private.h>

/*--------------------------------------------------------------------------*/
/* la_matVecMult: mult a matrix and a vec.                                  */
/*--------------------------------------------------------------------------*/

void la_matVecMult(mat, m, n, vec, vec1)
     la_matrix mat;
     int m;
     int n;
     la_vect vec;
     la_vect  vec1;
{
  int i, k;
  la_vect  row;

      

  for (i=0; i<m; i++)
    {
      row = mat[i];
      vec1[i] = 0;
      for (k=0; k<n; k++)
	vec1[i] += row[k] * vec[k];
    }
	
    
}

/*--------------------------------------------------------------------------*/
/* la_vecMatMult: mult  a vec by a matrix.                                  */
/*--------------------------------------------------------------------------*/

void la_vecMatMult(vec, m, mat, n, vec1)
     la_vect vec;
     int m;
     la_matrix mat;
     int n;
     la_vect vec1;
{
  int i, k;

  for (i=0; i<n; i++)
    {
      vec1[i] = 0;
      for (k=0; k<m; k++)
	vec1[i] += mat[k][i] * vec[k];
    }
	
    

}

/* end of la-vecMat.c.                  */ 
