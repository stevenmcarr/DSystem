/* $Id: la-trans.c,v 1.2 1997/03/27 20:47:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/****************************************************************************
 * Copyright (C) 1993, 1992, 1991 Cornell University -- The Typhoon Project
 *
 * la-trans.c,v 1.1.1.1 1993/09/23 19:09:35 stodghil Exp
 *
 * Routines for constructing transformations.
 *
 * Originally written by Wei Li.
 *
 * la-trans.c,v
 * Revision 1.1.1.1  1993/09/23  19:09:35  stodghil
 * Wei's Lambda Toolkit.
 *
 *
 ****************************************************************************/

#include <libs/support/Lambda/la-private.h>


/*------------------------------------------------------*/
/* Create a matrix struct with rowsize and colsize.     */
/*------------------------------------------------------*/

LA_MATRIX_T la_matrix_new(rowsize, colsize)
     int rowsize;
     int colsize;
{

  LA_MATRIX_T T;
  int i;

  T = (LA_MATRIX_T) calloc(1, sizeof(LA_MATRIX_T1));

  LA_MATRIX(T) = (int **) calloc(rowsize, sizeof(int *));

  for(i=0; i<rowsize; i++)
    {
      LA_MATRIX(T)[i] = (int *) calloc(colsize, sizeof(int));

    }

  LA_MATRIX_ROWSIZE(T) = rowsize;
  LA_MATRIX_COLSIZE(T) = colsize;
  LA_MATRIX_DENOM(T) = 1;

  return(T);
}
			

/*-----------------------------------------------------------------*/
/* Create a matrix struct                                          */
/*-----------------------------------------------------------------*/

LA_MATRIX_T la_matrix_allocate( )
{

  LA_MATRIX_T T;

  T = (LA_MATRIX_T) calloc(1, sizeof(LA_MATRIX_T1));

  LA_MATRIX_ROWSIZE(T) = 0;
  LA_MATRIX_COLSIZE(T) = 0;
  LA_MATRIX_DENOM(T) = 1;

  return(T);
}
			

/*-----------------------------------------------------------------*/
/* Free a matrix struct.                                           */
/*-----------------------------------------------------------------*/

void la_matrix_free( matrix )
     LA_MATRIX_T matrix;
{

  free (matrix);

}


/*--------------------------------------------------------------------------*/
/* la_is_nonsingular: check if nonsingular.                                 */
/* returns the row that is linearly dependent of rows above  it.            */
/* returns matrix size is nonsingular.                                      */
/*--------------------------------------------------------------------------*/

int  la_is_nonsingular(T)
     LA_MATRIX_T T;
{
      
  return(la_is_fullrank(T));

}

/*--------------------------------------------------------------------------*/
/* la_is_fullrank: check if fulll row rank.                                 */
/* returns the row that is linearly dependent of rows above  it.            */
/* returns matrix size is nonsingular.                                      */
/*--------------------------------------------------------------------------*/

int  la_is_fullrank(pT)
     LA_MATRIX_T pT;
{
  return(la_rank(pT) == LA_MATRIX_ROWSIZE(pT));
}

/*--------------------------------------------------------------------------*/
/* la_rank:  computes the rank of the matrix.                               */
/*--------------------------------------------------------------------------*/

int  la_rank(pT)
     LA_MATRIX_T pT;
{

  la_matrix parT;
  int rowSize, colSize;
  
  int i, j, nextR;

  la_matrix tempM;
  la_vect row;
  int minCol, fact;

  parT = LA_MATRIX(pT);
  rowSize = LA_MATRIX_ROWSIZE(pT);
  colSize = LA_MATRIX_COLSIZE(pT);

  tempM = la_matNew(rowSize, colSize);
  la_matCopy(parT, tempM, rowSize, colSize);

  j = 0;

  while ((j<colSize) && (j<rowSize))
    {
      /* consider the submatrix A[j:m, j:n].                    */
      /* search for the first row k>=j such that A[k, j:n] != 0 */

      nextR = la_eFirstNonZeroVect(tempM, rowSize, colSize, j);

      if (nextR != j ) 
	return(j);

      /* delete rows j .. nextR-1 */

      la_eDeleteRows(tempM, rowSize, j, nextR);
      la_eDeleteRows(parT, rowSize, j, nextR);

      rowSize = rowSize - nextR + j;

      /* nextR becomes row j+1 in the matrix, */
      /* although not necessarily row j+1 in the array.   */

      /* apply elementary column operations to make the diag element nonzero */
      /* and others zero.                                                    */
      
      row = tempM[j];

      /* make every element of tempM[j, j:colSize] positive. */

      for(i=j; i<colSize; i++)
	if(row[i] < 0)
	  {
	    la_eNegate(tempM, rowSize, i);
	  }

      /* stop when only the diag element is nonzero.           */
      while (la_eFirstNonZero(row, colSize, j+1) < colSize)
	{
	  minCol = la_eMinInVec(row, colSize, j);

	  la_eExchange(tempM, rowSize, j, minCol);
	  

	  for (i = j+1; i<colSize; i++)
	    {
	      if(row[i])
		{
		  fact = row[i] / row[j];
		  /* add (-1)*fact of col j to col i. */
		  la_eAdd(tempM, rowSize, j, i, (-1)*fact);
		  
		}

	    }
	}

      j++;
    }
  
  
  
  la_matFree(tempM, rowSize, colSize);
  
  return(rowSize);

}

/*--------------------------------------------------------------------------*/
/* la_is_legal: check if legal w.r.s.t dependencies.                        */
/* returns the row that violates the dependencies.                          */
/* returns matrix size is legal.                                            */
/*--------------------------------------------------------------------------*/

int  la_is_legal(T, D)
     LA_MATRIX_T T;
     LA_DEP_M_T D;
{

  return(la_is_legal_par(T, D));

}

/*--------------------------------------------------------------------------*/
/* la_is_legal_par:  check if legal.                                        */
/* returns the row that violates the dependencies.                          */
/* returns matrix row size is legal.                                        */
/*--------------------------------------------------------------------------*/

int  la_is_legal_par(pT, D)
     LA_MATRIX_T pT;
     LA_DEP_M_T D;
{
  int i, legal=1;

  LA_DIR_T* flagV;
  la_vect row;

  la_matrix parT;
  int depSize;

  depSize = LA_DEP_M_SIZE(D);

  parT = LA_MATRIX(pT);

  flagV = (LA_DIR_T *) la_vecNew( LA_DEP_M_SIZE(D) );

  for(i=0; (i<LA_MATRIX_ROWSIZE(pT)) && legal; i++)
    {
      row = parT[i];
      la_depMult(row, D, flagV);
      /* check if the row is legal. */

      if(!la_allPosEq(flagV, depSize))
	legal = 0;
	
    }

  return(legal);
}



/*--------------------------------------------------------------------------*/
/* la_complete:  forms  the transformation matrix.                          */
/*--------------------------------------------------------------------------*/

LA_MATRIX_T la_complete(pT, D)
     LA_MATRIX_T pT;
     LA_DEP_M_T D;
{
  LA_MATRIX_T T;
  
  /*  computes the base matrix from parT */

  T = la_base(pT);

  if (la_print)
    {
      printf("\nThe base matrix is:\n");
      la_matrix_print(T);
    }
 
  /*  computes the legal base matrix */

  T = la_base_legal(T, D);

  if (la_print)
    {
      printf("\nThe legal base matrix is:\n");
      la_matrix_print(T);
 
      printf("\nThe unsatisfied dependeces:\n");
      la_dep_matrix_print(D);
    }


  /*  padding of the legal matrix w.r.t the dependence matrix.  */

 
  T = la_padding_legal(T, D);

  if (la_print)
    {
      printf("\nThe legal partial matrix after padding w.r.t depM is:\n");
      la_matrix_print(T);
    }
 
  /*  padding of the  matrix to a legal invertible matrix */
  T = la_padding(T);

  if (la_print)
    {
      printf("\nThe non-singular matrix is:\n");
      la_matrix_print(T);
    }

  return(T);
}

/*--------------------------------------------------------------------------*/
/* la_base: computes the base matrix.                                       */
/*--------------------------------------------------------------------------*/

LA_MATRIX_T la_base(pT)
     LA_MATRIX_T pT;
{
  
  int rowSize, colSize;
  int i, j, nextR;

  la_matrix parT, tempM;
  la_vect row;
  int minCol, fact;

  LA_MATRIX_T base;

  rowSize = LA_MATRIX_ROWSIZE(pT);
  colSize = LA_MATRIX_COLSIZE(pT);

  base = la_matrix_new(rowSize, colSize);
  parT = LA_MATRIX(base);
  la_matCopy(LA_MATRIX(pT), parT, rowSize, colSize);

  tempM = la_matNew(rowSize, colSize);
  la_matCopy(parT, tempM, rowSize, colSize);

  j = 0;

  while ((j<colSize) && 
	 (nextR = la_eFirstNonZeroVect(tempM, rowSize, colSize, j)) < rowSize )
    {
      /* consider the submatrix A[j:m, j:n].                    */
      /* search for the first row k>=j such that A[k, j:n] != 0 */


      /* delete rows j .. nextR-1 */

      la_eDeleteRows(tempM, rowSize, j, nextR);
      la_eDeleteRows(parT, rowSize, j, nextR);

      rowSize = rowSize - nextR + j;

      /* nextR becomes row j+1 in the matrix, */
      /* although not necessarily row j+1 in the array.   */

      /* apply elementary column operations to make the diag element nonzero */
      /* and others zero.                                                    */
      
      row = tempM[j];

      /* make every element of tempM[j, j:colSize] positive. */

      for(i=j; i<colSize; i++)
	if(row[i] < 0)
	  {
	    la_eNegate(tempM, rowSize, i);
	  }

      /* stop when only the diag element is nonzero.           */
      while (la_eFirstNonZero(row, colSize, j+1) < colSize)
	{
	  minCol = la_eMinInVec(row, colSize, j);

	  la_eExchange(tempM, rowSize, j, minCol);
	  

	  for (i = j+1; i<colSize; i++)
	    {
	      if(row[i])
		{
		  fact = row[i] / row[j];
		  /* add (-1)*fact of col j to col i. */
		  la_eAdd(tempM, rowSize, j, i, (-1)*fact);
		  
		}

	    }
	}

      j++;
    }
  
  
  /* store the rank */
  LA_MATRIX_ROWSIZE(base) = j;
  
  la_matFree(tempM, rowSize, colSize);
  
  return(base);

}

/*--------------------------------------------------------------------------*/
/* la_base_legal:computes the legal base matrix.                            */
/*--------------------------------------------------------------------------*/

LA_MATRIX_T  la_base_legal(pT, D)
     LA_MATRIX_T pT;
     LA_DEP_M_T D;
{
  int i, k;

  int rowSize, colSize;
  int depSize;
  la_vect row;
  LA_DIR_T *flagV;

  LA_MATRIX_T base;

  rowSize = LA_MATRIX_ROWSIZE(pT);
  colSize = LA_MATRIX_COLSIZE(pT);

  base = la_matrix_allocate();
  LA_MATRIX_COLSIZE(base) = colSize;

  LA_MATRIX(base) = la_newVect(rowSize, la_vect);

  depSize = LA_DEP_M_SIZE(D);

  flagV = (LA_DIR_T *) la_vecNew(depSize);


  /* k = number of legal rows.   */
  k = 0;

  for(i=0; (i<rowSize) && (depSize>0); i++)
    {
      row = LA_MATRIX(pT)[i];
      la_depMult(row, D, flagV);
 
      if(la_allNegEq(flagV, depSize))
	{
	  la_vecNegate(row, row, colSize);
	  la_flagNegate(flagV, depSize);
	}

      /* check if the row is legal. */

      if(la_allPosEq(flagV, depSize))
	{
	  /* delete satisfied dependences. */
	  la_delPosDep(D, flagV);
	  depSize = LA_DEP_M_SIZE(D);

	  LA_MATRIX(base)[k] = row;
	  k++;
	}
      else
	{
	  la_vecFree(row);
	}

/*
      fprintf(stderr, "new base: k= %d\n", k);
      la_printM(ParT, k, colSize);
      fprintf(stderr, "new depM:\n");
      la_printM(depM, *depSize, colSize);
*/
	
    }
  for(; i<rowSize; i++)
    {
      LA_MATRIX(base)[k] = LA_MATRIX(pT)[i];
      k++;
    }

  LA_MATRIX_ROWSIZE(base) = k;

  return(base);

}

/*--------------------------------------------------------------------------*/
/* la_padding_legal:padding of the legal base matrix w.r.t the dep matrix.  */
/*--------------------------------------------------------------------------*/

LA_MATRIX_T  la_padding_legal(pT, D)
     LA_MATRIX_T pT;
     LA_DEP_M_T D;
{
  int  r, k;
  LA_DIR_T *flagV;

  LA_MATRIX_T T;
  la_matrix parT;
  int rowSize, colSize;

  rowSize = LA_MATRIX_ROWSIZE(pT);
  colSize = LA_MATRIX_COLSIZE(pT);

  T = la_matrix_new(colSize, colSize);
  parT = LA_MATRIX(T);
  la_matCopy(LA_MATRIX(pT), parT, rowSize, colSize);


  flagV = (LA_DIR_T *) la_vecNew(LA_DEP_M_SIZE(D));

  r = rowSize;	  


  /* padding of the base mapping until no dependences left. */

  while (LA_DEP_M_SIZE(D) > 0)
    {
      /* get the first e_k within 90 degrees with every dep */
      k = la_firstEk(D);

      /* proj e_k to the null space of row space of parT   */
      la_projToNull(parT, r, colSize, k, parT[r]);

      /* delete dependences satisfied by the new row        */
      la_depMult(parT[r], D, flagV);
 
      la_delPosDep(D, flagV);

      r++;

    }

  LA_MATRIX_ROWSIZE(T) = r;
  return(T);

}

/*--------------------------------------------------------------------------*/
/* la_padding: padding of the legal base matrix to la invertible matrix.    */
/*--------------------------------------------------------------------------*/

LA_MATRIX_T  la_padding(pT)
     LA_MATRIX_T pT;
{
  int i, k;
  int curR, minCol, fact;

  la_matrix tempM, padM;
  la_vect row;

  LA_MATRIX_T T;
  la_matrix parT;
  int rowSize, colSize;

  rowSize = LA_MATRIX_ROWSIZE(pT);
  colSize = LA_MATRIX_COLSIZE(pT);

  T = la_matrix_new(colSize, colSize);
  parT = LA_MATRIX(T);
  la_matCopy(LA_MATRIX(pT), parT, rowSize, colSize);

  /* full rlak, no need for padding */
  if (rowSize==colSize)
    return(T);

  tempM = la_matNew(rowSize, colSize);
  la_matCopy(parT, tempM, rowSize, colSize);

  padM = la_matNew(colSize, colSize);
  la_matId(padM, colSize);


  for(curR=0; curR<rowSize; curR++)
    {
      /* consider the submatrix A[i:m, i:n].                    */

      /* apply elementary column operations to make the diag element nonzero */
      /* and others zero.                                                    */

      /* only consider columns from curR to colSize.           */
      
      row = tempM[curR];

      /* make every element of tempM[curR, curR:colSize] positive. */

      for(i=curR; i<colSize; i++)
	if(row[i] < 0)
	  {
	    la_eNegate(tempM, rowSize, i);
	  }

      /* stop when only the diag element is nonzero.           */
      while (la_eFirstNonZero(row, colSize, curR+1) < colSize)
	{
	  minCol = la_eMinInVec(row, colSize, curR);
	  
	  la_eExchange(tempM, rowSize, curR, minCol);
	  
	  la_rowExchange(padM, curR, minCol);

	  for (i = curR+1; i<colSize; i++)
	    {
	      if(row[i])
		{
		  fact = row[i] / row[curR];
		  la_eAdd(tempM, rowSize, curR, i, (-1)*fact);
		  
		}
      

	    }
	}

      
      
    }


  for(k =rowSize; k<colSize; k++)
    {
      parT[k] = padM[k];
    }

  la_matFree(tempM, rowSize, colSize);

  return(T);

}

/*--------------------------------------------------------------------------*/
/* la_inverse: computes the inverse of a transformation.                    */
/*--------------------------------------------------------------------------*/

LA_MATRIX_T  la_inverse(T)
     LA_MATRIX_T T;
{
  LA_MATRIX_T inv;
  int det;

  inv = la_matrix_new(LA_MATRIX_ROWSIZE(T), LA_MATRIX_COLSIZE(T));

  det = la_matInv(LA_MATRIX(T), LA_MATRIX(inv), LA_MATRIX_ROWSIZE(T));

  LA_MATRIX_DENOM(inv) = det;

  return(inv);


}



/*--------------------------------------------------------------------------*/
/* la_vec_X_dep: dot product of an integer vector and a dep vector.         */
/*--------------------------------------------------------------------------*/

LA_DEP_T la_vec_X_dep(vec1, d, dim)
     la_vect vec1;
     LA_DEP_V_T d;
     int dim;
{

  int j;
  int pVal, cVal, vVal;
  int vecSize;
  LA_DIR_T pType, cType;
  LA_DEP_T result;

  vecSize = dim;

  pType = dK;
  pVal = 0;
  for(j=0; j<vecSize; j++)
    {
      /* current type and value */
      cType = LA_DEP_DIR( LA_DEP_V_VECTOR(d)[j] );
      cVal = LA_DEP_DIST( LA_DEP_V_VECTOR(d)[j] );
      
      vVal = vec1[j];

/*
  fprintf(stderr, "pType is: ");
  la_printDirV(&pType, 1); 
  fprintf(stderr, "cType is: ");
  la_printDirV(&cType, 1); 
  fprintf(stderr, "vVal is: %d \n", vVal);
*/


      /* do elemnet-wise multuplication */
      /* if zero, then zero (=).        */
      if(!vVal)
	{
	  cType = dEQ;
	}
      else
	switch(cType)
	  {
	  case dLT:
	    if (vVal < 0) cType = dGT;
	    break;
	    
	  case dK:
	    cVal = cVal*vVal;
	    break;
	    
	  case dGT:
	    if (vVal < 0) cType = dLT;
	    break;
	    
	  case dLEQ:
	    if (vVal < 0) cType = dGEQ;
	    break;
	    
	  case dSTAR:
	    
	    break;
		
	  case dGEQ:
	    if (vVal < 0) cType = dLEQ;
	    break;
	    
	  case dEQ:
	    break;
	    
	  default:
	    
	    fprintf(stderr, "In depMult: unexpected dist/dir \n");
	    
	  }
      
      /* do addition */
      switch(pType)
	{
	case dLT:
	  switch(cType)
	    {
	    case dLT:
	    case dEQ:
	    case dLEQ:
	      break;
	      
	    case dK:
	      if (cVal < 0)
		pType = dSTAR;
	      break;
	      
	    case dGT:
	    case dGEQ:
	    case dSTAR:
	      pType = dSTAR;
	      break;


	    default:
	      fprintf(stderr, "In depMult 2: unexpected dist/dir\n");
	      
	    }
	  
	  break;
	  
	case dK:
	  if(cType == dK)
	    pVal = cVal + pVal;
	  else if(cType != dEQ)
	    pType = cType;
	  break;
	  
	case dGT:
	  switch(cType)
	    {
	    case dGT:
	    case dGEQ:
	    case dEQ:
	      break;
	      
	    case dK:
	      if (cVal > 0)
		pType = dSTAR;
	      break;
	      
	    case dLT:
	    case dLEQ:
	    case dSTAR:
	      pType = dSTAR;
	      break;
	      
	    default:
	      fprintf(stderr, "In depMult 3 : unexpected dist/dir\n");
	      
	    }
	  break;
	  
	case dLEQ:
	  switch(cType)
	    {
	    case dLT:
	      pType = dLT;
	      break;
	      
	    case dLEQ:
	    case dEQ:
	      break;
	      
	    case dK:
	      if (cVal > 0)
		pType = dSTAR;
	      else if (cVal < 0)
		pType = dLT;
	      
	      break;
	      
	    case dGT:
	    case dGEQ:
	    case dSTAR:
	      pType = dSTAR;
	      break;
	      
	    default:
	      fprintf(stderr, "In depMult 4 : unexpected dist/dir\n");
	      
	    }
	  
	  break;

	case dSTAR:
	  
	  break;
	  
	case dGEQ:
	  switch(cType)
	    {
	    case dGT:
	      pType = dGT;
	      break;
	      
	    case dGEQ:
	    case dEQ:
	      break;
	      
	    case dK:
	      if (cVal < 0)
		pType = dSTAR;
	      else if (cVal > 0)
		pType = dGT;
	      break;
	      
	    case dLT:
	    case dSTAR:
	    case dLEQ:
	      pType = dSTAR;
	      break;
	      
	    default:
	      fprintf(stderr, "In depMult 5: unexpected dist/dir\n");
	      
	    }
	  
	  break;
	  
	case dEQ:
	  pType = cType;
	  pVal = cVal;
	  break;
	  
	default:
	  fprintf(stderr, "In depMult 6: unexpected dist/dir\n");
	  
	}
    }

  /* converting dist to directions (tags) */
/*
  if (pType == dK)
    if(pVal>0)
      pType = dLT;
    else if (pVal<0)
      pType = dGT;
    else
      pType = dEQ;

  return(pType);
*/

  LA_DEP_DIR(result) = pType;
  LA_DEP_DIST(result) = pVal;

  return(result);
  
}


/*----------------------------------------------------------------*/
/* la_matrix_X_depM: an integer matrix times  a dep matrix.       */
/*----------------------------------------------------------------*/

LA_DEP_M_T  la_matrix_X_depM(A, D)
     LA_MATRIX_T A;
     LA_DEP_M_T D;
{


  int i, j;
  int dim, vecSize, rowA;
  LA_DEP_V_T d, vec, last;
  LA_DEP_M_T newD;

  vecSize = LA_DEP_M_SIZE(D);
  dim = LA_DEP_M_DIM(D);

  rowA = LA_MATRIX_ROWSIZE(A);

  newD = la_dep_matrix_new(vecSize, 0);
  last = NULL;

  for(i=0; i<rowA; i++)
    {
      /* compute the i'th row */
      vec = la_dep_vec_new( vecSize );

      d = LA_DEP_M_VECTORS(D);
      j = 0;
      for(; d; d = LA_DEP_V_NEXT(d))
	LA_DEP_V_VECTOR(vec)[j++] = 
	  la_vec_X_dep(LA_MATRIX(A)[i], d, dim);

/*
      printf("a flag vec is (vecSize= %d)", vecSize);
      la_dep_vec_print(vec, vecSize);
*/
      
      la_dep_add_to_list_last(vec, newD, last);
      last = vec;
    }

  return newD;

}


/*--------------------------------------------------------------------------*/
/* la_dep_delete_carried: delete the dep's carried by x.                    */
/*                        computes xD.                                      */
/*--------------------------------------------------------------------------*/

void  la_dep_delete_carried(D, x)
     LA_DEP_M_T D;
     la_vect x;
{
  LA_DIR_T *flagV;

  flagV = (LA_DIR_T *) la_vecNew(LA_DEP_M_SIZE(D));
  
  /* delete dependences satisfied by the new row        */
  la_depMult(x, D, flagV);
 
  la_delPosDep(D, flagV);

}

/*--------------------------------------------------------------------------*/

/*-----------------------------------------------------------------*/
/* end of la-trans.c                                               */
/*-----------------------------------------------------------------*/
