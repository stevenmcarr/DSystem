/****************************************************************************
 * Copyright (C) 1993, 1992, 1991 Cornell University -- The Typhoon Project
 *
 * la-hermite.c,v 1.1.1.1 1993/09/23 19:09:38 stodghil Exp
 *
 * Routines to compute Hermite form.
 *
 * Originally written by Wei Li.
 *
 * la-hermite.c,v
 * Revision 1.1.1.1  1993/09/23  19:09:38  stodghil
 * Wei's Lambda Toolkit.
 *
 *
 ****************************************************************************/

#include <la-private.h>


/*--------------------------------------------------------------------------*/
/* Each data access matrix is represented by a vector of rows.              */
/* Each dependence matrix is represented by a vector of columns.            */
/*--------------------------------------------------------------------------*/



/*--------------------------------------------------------------------------*/
/* la_eExchange: Elementary operation -- exchange to columns.               */
/* m is the number of rows.                                                 */
/*--------------------------------------------------------------------------*/

void la_eExchange (mat, m, col1, col2)
     la_matrix mat;
     int m;
     int col1;
     int col2;
{
  la_vect row;
  int i, temp;

  for ( i=0; i<m; i++)
    {
      row = mat[i];
      temp = row[col1];
      row[col1] =  row[col2];
      row[col2] =  temp;
      
    }
    

}

/*--------------------------------------------------------------------------*/
/* la_eAdd: Elementary operation -- add an integral multiple of column col1 */
/* to column col2.                                                          */
/*--------------------------------------------------------------------------*/

void la_eAdd (mat, m, col1, col2, fact)
     la_matrix mat;
     int m;
     int col1;
     int col2;
     int fact;
{
  la_vect row;
  int i;

  for (i = 0; i < m; i++)
    {
      row = mat[i];
      row[col2] = row[col2] + fact * row[col1];
    }

}

/*--------------------------------------------------------------------------*/
/* la_eNegate: Elementary operation -- negate one column.                   */
/*--------------------------------------------------------------------------*/


void la_eNegate (mat, m, col)
     la_matrix mat;
     int m;
     int col;
{
  la_vect row;
  int i;


  for (i = 0; i < m; i++)
    {
      row = mat[i];
      row[col] =  (-1) * row[col];
    }
    
}

/*--------------------------------------------------------------------------*/
/* la_eColConst: Elementary operation -- mult one column by a const.        */
/*--------------------------------------------------------------------------*/

void la_eColConst(mat, m, col, fact)
     la_matrix mat;
     int m;
     int col;
     int fact;
{
  la_vect row;
  int i;


  for (i = 0; i < m; i++)
    {
      row = mat[i];
      row[col] =  fact * row[col];
    }
    

}


/*--------------------------------------------------------------------------*/
/* la_eMinInVec: computes the min non-zero in the vector from start..n.     */
/*--------------------------------------------------------------------------*/

int la_eMinInVec(vec, n, start)
     la_vect vec;
     int n;
     int start;
{
  int j, minCol = -1;

  for ( j = start; j<n; j++)
    if(vec[j])
      if( (minCol<0) || (vec[j] < vec[minCol]) )
	minCol = j;

  if(minCol<0) fprintf(stderr, "Zero vect in la_eMinINVec!!! \n");

  return(minCol);
}

/*--------------------------------------------------------------------------*/
/* la_eFirstNonZero: return the first non-zero in the postfix i--n.         */
/*                return  n if all zeroes.                                  */
/*--------------------------------------------------------------------------*/

int la_eFirstNonZero(vec, n, i)
     la_vect vec;
     int n;
     int i;
{
  int j;

  j = i;
  while ( (j<n) && (vec[j]==0) )
    j++ ;

  return(j);
}


/*--------------------------------------------------------------------------*/
/* la_matHermite:   apply elementary column operations to decompose         */
/* mat to a product of a lower triangular and a unimodular matrix.          */
/*--------------------------------------------------------------------------*/

void  la_matHermite(mat, n, H, U)
     la_matrix mat;
     int n;
     la_matrix H;
     la_matrix U;
{

  la_vect row;

  int i, j, fact, minCol;

  la_matCopy(mat, H, n, n);
  la_matId(U, n);

  /* j is the current row of H. */
  for(j=0; j<n; j++)
    {
      
      row = H[j];

      /* make every element of H[j, j:n] positive. */

      for(i=j; i<n; i++)
	if(row[i] < 0)
	  {
	    la_eNegate(H, n, i);
	    la_vecNegate(U[i], U[i], n);
	  }

      /* stop when only the diag element is nonzero.           */
      while (la_eFirstNonZero(row, n, j+1) < n)
	{
	  minCol = la_eMinInVec(row, n, j);

	  la_eExchange(H, n, j, minCol);

	  la_rowExchange(U, j, minCol);

	  for (i = j+1; i<n; i++)
	    {
	      fact = row[i] / row[j];
	      /* add (-1)*fact of col j to col i. */
	      la_eAdd(H, n, j, i, (-1)*fact);

	      /* add fact of row i to row j. */
	      la_rowAdd(U, n, i, j, fact);
      

	    }
	}
    }



}

/*--------------------------------------------------------------------------*/
/* la_eFirstNonZeroVect: computes the first non-zero vector.                */
/*--------------------------------------------------------------------------*/

int la_eFirstNonZeroVect(mat, rowSize, colSize, startRow)
     la_matrix mat;
     int rowSize;
     int colSize;
     int startRow;
{

  int j, found = 0;

  for (j=startRow; (j<rowSize) && !found; j++)
    if((mat[j] != NULL) && 
       (la_eFirstNonZero(mat[j], colSize, startRow) < colSize))
      found = 1;
      

  if(found) return(j-1);

  /* if not found, return the max */
  return(rowSize);

}



/*--------------------------------------------------------------------------*/

/* end of la-hermite.c.  */
