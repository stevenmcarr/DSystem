/* $Id: la-matrix.C,v 1.1 1997/04/28 20:20:00 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/****************************************************************************
 * Copyright (C) 1993, 1992, 1991 Cornell University -- The Typhoon Project
 *
 * la-matrix.c,v 1.1.1.1 1993/09/23 19:09:36 stodghil Exp
 *
 * Routines about matrix operations.
 *
 * Originally written by Wei Li.
 *
 * la-matrix.c,v
 * Revision 1.1.1.1  1993/09/23  19:09:36  stodghil
 * Wei's Lambda Toolkit.
 *
 *
 ****************************************************************************/

#include <libs/support/Lambda/la-private.h>

/*--------------------------------------------------------------------------*/
/* la_matNew: Allocate a matrix of mxn.                                     */
/*--------------------------------------------------------------------------*/

la_matrix la_matNew(int m, int n)
  //int m;
  //int n;
{
  la_matrix mat;
  int i, j;
  la_vect row;

  mat = la_newVect(m, la_vect);

  for (i=0; i<m; i++)
    {
      mat[i] = la_vecNew(n);
      row = mat[i];
      for (j=0; j<n; j++)
	row[j] = 0;
    }
	
    
  return( mat );

}

/*--------------------------------------------------------------------------*/
/* la_matId: return an identity matrix.                                     */
/*--------------------------------------------------------------------------*/

void la_matId(la_matrix mat, int n)
  // la_matrix mat;
  // int n;
{
  int i, j;
  la_vect row;


  for (i=0; i<n; i++)
    {
      row = mat[i];
      for (j=0; j<n; j++)
	if(i==j)
	  row[j] = 1;
	else
	  row[j] = 0;
    }
	
    
}

/*--------------------------------------------------------------------------*/
/* la_matCopy: make copy of the matrix.                                     */
/*--------------------------------------------------------------------------*/

void la_matCopy(la_matrix mat1, la_matrix mat2, int m, int n)
//      la_matrix mat1;
//      la_matrix mat2;
//      int m;
//      int n;
{
  int i, j;
  la_vect row1, row2;


  for (i=0; i<m; i++)
    {
      row1 = mat1[i];
      row2 = mat2[i];
      for (j=0; j<n; j++)
	row2[j] = row1[j];
    }
	
    

}

/*--------------------------------------------------------------------------*/
/* la_matNegate:  matrix negated.                                           */
/*--------------------------------------------------------------------------*/

void la_matNegate(la_matrix mat1, la_matrix mat2, int m, int n)
//      la_matrix mat1;
//      la_matrix mat2;
//      int m;
//      int n;
{
  int i;
  la_vect row1, row2;


  for (i=0; i<m; i++)
    {
      row1 = mat1[i];
      row2 = mat2[i];
      la_vecNegate(row1, row2, n);
    }
	
    
}

/*--------------------------------------------------------------------------*/
/* la_matT: make transpose of the matrix.                                   */
/*--------------------------------------------------------------------------*/

void la_matT(la_matrix mat1, la_matrix mat2, int m, int n)
//      la_matrix mat1;
//      la_matrix mat2;
//      int m;
//      int n;
{
  int i, j;
  la_vect row2;


  for (i=0; i<n; i++)
    {
      row2 = mat2[i];
      for (j=0; j<m; j++)
	row2[j] =  mat1[j][i];
    }
	
    
}

/*--------------------------------------------------------------------------*/
/* la_matrix_is_id: is an identity matrix?                                  */
/*--------------------------------------------------------------------------*/

int la_matrix_is_id(LA_MATRIX_T M)
  // LA_MATRIX_T M;
{
  int i, j, m, n;

  m = LA_MATRIX_ROWSIZE(M);
  n = LA_MATRIX_COLSIZE(M);

  if( m != n) 
    return 0;

  for (i=0; i<m; i++)
    for (j=0; j<n; j++)
      if(i == j)
	{
	  if( LA_MATRIX(M)[i][j] != 1 )
	    {
	      return 0;
	    }
	}
      else
	{
	  if( LA_MATRIX(M)[i][j] != 0 )
	    {
	      return 0;
	    }
	}
  return 1;
}

/*--------------------------------------------------------------------------*/
/* la_matAdd: add two matrices.                                             */
/*--------------------------------------------------------------------------*/

void la_matAdd(la_matrix mat1, la_matrix mat2, la_matrix mat3, int m, int n)
//      la_matrix mat1;
//      la_matrix mat2;
//      la_matrix mat3;
//      int m;
//      int n;
{
  int i, j;
  la_vect  row1, row2, row3;


  for (i=0; i<m; i++)
    {
      row1 = mat1[i];
      row2 = mat2[i];
      row3 = mat3[i];
      for (j=0; j<n; j++)
	row3[j] = row1[j] + row2[j];
    }
	


}

/*--------------------------------------------------------------------------*/
/* la_matAddF: add two matrices with constant factors.                      */
/*--------------------------------------------------------------------------*/

void la_matAddF(la_matrix mat1, int f1, la_matrix mat2, int f2, la_matrix mat3, int m,
		int n)
//      la_matrix mat1;
//      int f1;
//      la_matrix mat2;
//      int f2;
//      la_matrix mat3;
//      int m;
//      int n;
{
  int i, j;
  la_vect  row1, row2, row3;


  for (i=0; i<m; i++)
    {
      row1 = mat1[i];
      row2 = mat2[i];
      row3 = mat3[i];
      for (j=0; j<n; j++)
	row3[j] = f1*row1[j] + f2*row2[j];
    }
	


}

/*--------------------------------------------------------------------------*/
/* la_matMult: mult two matrices.                                           */
/*--------------------------------------------------------------------------*/

void la_matMult(la_matrix mat1, la_matrix mat2, la_matrix mat3,int m,int r,int n)
//      la_matrix mat1;
//      la_matrix mat2;
//      la_matrix mat3;
//      int m;
//      int r;
//      int n;
{
  int i, j, k;
  la_vect  row1, row3;

  for (i=0; i<m; i++)
    {
      row3 = mat3[i];
      for (j=0; j<n; j++)
	{
	  row1 = mat1[i];
	  row3[j] = 0;
	  for (k=0; k<r; k++)
	    row3[j] += row1[k] * mat2[k][j];
	}
    }
	
}



/*--------------------------------------------------------------------------*/
/* la_matGetCol: get a column from the matrix.                              */
/*--------------------------------------------------------------------------*/

void la_matGetCol(la_matrix mat, int m, int col, la_vect vec)
//      la_matrix mat;
//      int m;
//      int col;
//      la_vect vec;
{
  int i;

  for (i=0; i<m; i++)
    vec[i] =  mat[i][col];
	

}

/*--------------------------------------------------------------------------*/
/* la_matConcat: concat two matrices.                                       */
/*--------------------------------------------------------------------------*/

void la_matConcat(la_matrix mat1, int m1, la_matrix mat2, int m2, la_matrix mat3)
//      la_matrix mat1;
//      int m1;
//      la_matrix mat2;
//      int m2;
//      la_matrix mat3;
{
  int i;

  for(i=0; i<m1; i++)
    mat3[i] = mat1[i];

  for(i=0; i<m2; i++)
    mat3[m1+i] = mat2[i];



}

/*--------------------------------------------------------------------------*/
/* la_matConcat_newM: concat two matrices. create a new mat.                */
/*--------------------------------------------------------------------------*/

void la_matConcat_newM(la_matrix mat1, int m1, int col, la_matrix mat2, int m2, 
		       la_matrix mat3)
//      la_matrix mat1;
//      int m1;
//      int col;
//      la_matrix mat2;
//      int m2;
//      la_matrix mat3;
{
  int i;

  for(i=0; i<m1; i++)
    la_vecCopy(mat1[i], mat3[i], col);

  for(i=0; i<m2; i++)
    la_vecCopy(mat2[i], mat3[m1+i], col);


}

/*--------------------------------------------------------------------------*/
/* la_rowExchange:  -- exchange two rows.                                   */
/*--------------------------------------------------------------------------*/

void la_rowExchange (la_matrix mat, int i1, int i2)
//      la_matrix mat;
//      int i1;
//      int i2;
{
  la_vect row;

  row = mat[i1];
  mat[i1] = mat[i2];
  mat[i2] = row;


}

/*--------------------------------------------------------------------------*/
/* la_rowAdd:  -- adds a multiple of i1 to i2.                              */
/*--------------------------------------------------------------------------*/

void la_rowAdd (la_matrix mat, int n, int i1, int i2, int fact)
//      la_matrix mat;
//      int n;
//      int i1;
//      int i2;
//      int fact;
{
  int i;
  la_vect r1, r2;

  r1 = mat[i1];
  r2 = mat[i2];

  for (i = 0; i < n; i++)
    r2[i] +=  fact * r1[i];
    

}

/*--------------------------------------------------------------------------*/
/* la_matInv:  computes the inverse  of the matrix.                         */
/*--------------------------------------------------------------------------*/

int la_matInv(la_matrix mat, la_matrix inv,int n)
//      la_matrix mat;
//      la_matrix inv;
//      int n;
{

  la_vect row;
  la_matrix temp;

  int i, j, fact, minCol, diag, det;

  temp = la_matNew(n, n);
  la_matCopy(mat, temp, n, n);
  la_matId(inv, n);

  /* j is the current row of temp. */
  for(j=0; j<n; j++)
    {

      row = temp[j];

      /* make every element of temp[j, j:n] positive. */

      for(i=j; i<n; i++)
	if(row[i] < 0)
	  {
	    la_eNegate(temp, n, i);
	    la_eNegate(inv, n, i);
	  }

      /* stop when only the diag element is nonzero.           */
      while (la_eFirstNonZero(row, n, j+1) < n)
	{
	  minCol = la_eMinInVec(row, n, j);

	  la_eExchange(temp, n, j, minCol);

	  la_eExchange(inv, n, j, minCol);

	  for (i = j+1; i<n; i++)
	    {
	      fact = row[i] / row[j];
	      /* add (-1)*fact of col j to col i. */
	      la_eAdd(temp, n, j, i, (-1)*fact);

	      la_eAdd(inv, n, j, i, (-1)*fact);
      

	    }
	}
    }

  /* reduce temp from a lower triangular to the identity. */

  det = 1;
  for(j=n-1; j>=0; j--)
    {
      /* consider columns from j to 0.           */

      row = temp[j];

      /* keep the diag element */
      diag = row[j];

      if (diag == 0)
	{
	  fprintf(stderr, "Error in computing inverse: Matrix is singular!\n");
	  exit(0);
	}

      det = det*diag;
      /* if not equal to 1, then mult each row by that.                  */
      /* converting                                                      */
      /*        ( 1   0      0 )      ( diag  0  0    )                  */
      /*        ( 0  1/diag  0 )  to  ( 0     1  0    )                  */
      /*        ( 0   0      1 )      ( 0     0  diag )                  */

      if(diag != 1)
	{
	  for(i=0; i<j; i++)
	    la_eColConst(inv, n, i, diag);

	  for(i=j+1; i<n; i++)
	    la_eColConst(inv, n, i, diag);

	  row[j] = 1;
	}


      for (i = j-1; i>=0; i--)
	if(row[i])
	  {
	    fact = row[i] ;
	    la_eAdd(temp, n, j, i, (-1)*fact);
	    la_eAdd(inv, n, j, i, (-1)*fact);
	  }

    }

  return(det);
}

/*--------------------------------------------------------------------------*/
/* la_eDeleteRows: Delete row r1 upto r2 (not include r2).                  */
/*--------------------------------------------------------------------------*/

void la_eDeleteRows(la_matrix matrix, int m, int r1, int r2)
//      la_matrix matrix;
//      int m;
//      int r1;
//      int r2;
{

  int i, d;

  d = r2 - r1;
  for (i=r1; i<r2; i++)
    la_vecFree((la_vect) matrix[i]);

  for (i=r2; i<m; i++)
    {
      matrix[i-d] = matrix[i];
    }

  for (i=m-d; i<m; i++)
    {
      matrix[i] = NULL;
    }
}

/*--------------------------------------------------------------------------*/
/* la_projToNull: projection of Ek to the null space of B.                  */
/*--------------------------------------------------------------------------*/

void  la_projToNull(la_matrix B, int rowSize, int colSize, int k, la_vect x)
//      la_matrix B;
//      int rowSize;
//      int colSize;
//      int k;
//      la_vect x;
{
  la_matrix M1, M2, M3, I;
  int det;



  /* computes c(I-B^T (B B^T) ^{-1} B) e_k   */

  /* M1 the transpose of B */
  M1 = la_matNew(colSize, colSize);
  la_matT(B, M1, rowSize, colSize);

  /* M2 = B * B^T */
  M2 = la_matNew(colSize, colSize);
  la_matMult(B, M1, M2, rowSize, colSize, rowSize);

  /* M3 = (M2)^{-1} */
  /* ignore the determinant    */
  M3 = la_matNew(colSize, colSize);
  det = la_matInv(M2, M3, rowSize);

  /* M2 = B^T (B B^T) ^{-1} */
  la_matMult(M1, M3, M2, colSize, rowSize, rowSize);

  /* M1 = B^T (B B^T) ^{-1} B */
  la_matMult(M2, B, M1, colSize, rowSize, colSize);

  la_matNegate(M1, M1, colSize, colSize);

  I = la_matNew(colSize, colSize);
  la_matId(I, colSize);


  la_matAddF(I, det, M1, 1, M2, colSize, colSize);


  la_matGetCol(M2, colSize, k-1, x);


}


/*--------------------------------------------------------------------------*/
/* la_matFree: free a matrix.                                               */
/*--------------------------------------------------------------------------*/

void  la_matFree(la_matrix mat, int m, int n)
//      la_matrix mat;
//      int m;
//      int n;
{
  int i;


  for (i=0; i<m; i++)
    free(mat[i]);
	
  free( mat );

}


/*--------------------------------------------------------------------------*/
/* end of la-matrix.c     */
