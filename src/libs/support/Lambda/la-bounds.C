/* $Id: la-bounds.C,v 1.1 1997/04/28 20:20:00 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/****************************************************************************
 * Copyright (C) 1993, 1992, 1991 Cornell University -- The Typhoon Project
 *
 * la-bounds.c,v 1.1.1.1 1993/09/23 19:09:37 stodghil Exp
 *
 * Routines for computing the new loop bounds.
 *
 * Originally written by Wei Li.
 *
 * la-bounds.c,v
 * Revision 1.1.1.1  1993/09/23  19:09:37  stodghil
 * Wei's Lambda Toolkit.
 *
 *
 ****************************************************************************/

#include <libs/support/Lambda/la-private.h>


/* This version handles a system of max size of EQ_MAX inequalities. */
#define EQ_MAX 500

/*--------------------------------------------------------------------------*/
/* la_aux: computes the loop bounds for the aux space.                      */
/*         Input system is Ax<=b. U is a unimodular transformation.         */
/*--------------------------------------------------------------------------*/


LA_LOOPNEST_T la_aux(LA_LOOPNEST_T nest, LA_MATRIX_T U)
//      LA_LOOPNEST_T nest;
//      LA_MATRIX_T U;
{

  la_matrix A, B;  /* Ax <= a + B */
  la_vect a;

  la_matrix A1, B1;    /* temp arrays */
  la_vect a1;

  la_matrix invU;
  int det;

  int depth, blobs;

  la_matrix  temp0;
  la_vect temp1;

  int size, sizeNew; /* size of the linear system */
  int i, j, k;

  LA_LOOPNEST_T nest_aux;
  LA_LOOP_T loop;
  LA_EXPR_T expr;

  LA_LATTICE_T lattice;

  int lcm, f1, f2;

  depth = LA_NEST_DEPTH(nest);
  blobs = LA_NEST_BLOBS(nest);
  A = la_matNew(EQ_MAX, depth);
  B = la_matNew(EQ_MAX, blobs);
  a = la_vecNew( EQ_MAX );
  
  /* another set of temp arrays used to store the system of inequalities. */
  /* it includes A1, B1 and a1 */

  A1 = la_matNew(EQ_MAX, depth);
  B1 = la_matNew(EQ_MAX, blobs);
  a1 = la_vecNew(EQ_MAX);

	  
  /* store bounds in the matrix A, constant vector a and blob matrix B */

  /* Ax <= a + B */
  size = 0; /* number of inequalities */
  for(i=0; i<depth; i++)
    {
      loop = LA_NEST_LOOPS(nest)[i];

      for(k=0; k<2; k++)
	{
	  
	  if(k==0)
	    {
	      /* processing the lower bounds */
	      if(LA_LOOP_STEP(loop)>0)
		expr = LA_LOOP_LOW(loop);
	      else
		expr = LA_LOOP_UP(loop);
	    }
	  else
	    {
	      /* processing the upper bounds */
	      if(LA_LOOP_STEP(loop)>0)
		expr = LA_LOOP_UP(loop);
	      else
		expr = LA_LOOP_LOW(loop);
	    }

	  for(; expr; expr = LA_EXPR_NEXT(expr))
	    {
	      /* processing one expression */
	      
	      /* coef  */
	      for(j=0; j<i; j++)
		A[size][j] = LA_EXPR_COEF(expr)[j];
      
	      /* constant */
	      a[size] = LA_EXPR_C0(expr);

	      /* blob coef */
	      for(j=0; j<blobs; j++)
		B[size][j] = LA_EXPR_BLOB_COEF(expr)[j];
	      if(k==0) 
		{
		  /* if lower bound */
		  /* change (2x+3y+2+b)/4  <= z to 2x+3y-4z <= -2-b */
		  
		  A[size][i] = (-1) * LA_EXPR_DENOM(expr);
		  a[size] = (-1) * a[size];
		  for(j=0; j<blobs; j++)
		    B[size][j] = (-1) * B[size][j];
		}
	      else
		{
		  /* if upper bound */
		  /* change z <= (2x+3y+2+b)/4  to -2x-3y+4z <= 2+b */
		  
		  for(j=0; j<i; j++)
		    A[size][j] = (-1) * A[size][j];
		  
		  A[size][i] = LA_EXPR_DENOM(expr);

		}
	      
	      size++;
	    } /* expr */
	} /* bound */

      
    }
  
/*
  printf("old system: \n");
  printf("A : \n");
  la_printM(A, size, depth);
*/

  /* let x = L*y + origin, i.e. y is the base space */
  lattice = la_lattice( nest );

  /* Ax <= a+B ---> ALy <= a+B - A*origin */

  /* A1 = AL */
  la_matMult(A, 
	     LA_LATTICE_BASE(lattice),
	     A1,
	     size,
	     depth,
	     depth );
  
  /* a1 = a-A*origin_constant */
  la_matVecMult(A, 
		size,
		depth,
		LA_LATTICE_ORIGIN(lattice),
		a1);
  la_vecAddF(a, 1, a1, -1, a1, size);

  /* B1 = B- A*origin_blob */
  la_matMult(A, 
	     LA_LATTICE_ORIGIN_BLOB(lattice),
	     B1,
	     size,
	     depth,
	     blobs );
  
  la_matAddF(B, 1, B1, -1, B1, size, blobs);

/*  
  printf("base system: \n");
  printf("A1 : \n");
  la_printM(A1, size, depth);
*/

  /* Now we have A1*y <= a1 + B1 */

  /* Let z be the target space.             */
  /* z = Tx = T(Ly+origin) = TLy + T*origin */
  /* z1 = z-T*origin = TLy = HUy            */

  /* The following will compute the aux space for z1=HUy */
  /* given A1*y <= b1 + B1.                              */
 

  invU = la_matNew(depth, depth);
  /* computes the inverse of U */
  det = la_matInv(LA_MATRIX(U), invU, depth);
  
/*
  printf("invU : \n");
  la_printM(invU, depth, depth);
*/

  /* A = A1 U^{-1} */
  la_matMult(A1, invU, A, size, depth, depth );
  
/*
  printf("new system: \n");
  printf("A : \n");
  la_printM(A, size, depth);
*/

  /* Fourier-Motzkin on Ax <= a + B */

  temp0 = B1;
  B1 = B;
  B = temp0;

  temp1 = a1;
  a1 = a;
  a = temp1;

  nest_aux = la_nest_new( depth, blobs );

  /* Comptues the bounds for loop i */
  for (i=depth-1; i>=0; i--)
    {
/*
      printf("linear system: \n");
      printf("A : \n");
      la_printM(A, size, depth);
      printf("a : \n");
      la_printV(a, size);
      printf("B : \n");
      la_printM(B, size, blobs);
*/
      if(size>EQ_MAX) 
	{
	  fprintf(stderr, "Linear system too large! Change EQ_MAX!\n");
	  exit(0);
	}

      loop = la_loop_new();
      LA_NEST_LOOPS(nest_aux)[i] = loop;

      /* The step size is always 1 in the aux space. */
      LA_LOOP_STEP(loop) = 1;

      for (j=0; j<size; j++)
	{
	  if( A[j][i] < 0 )
	    { /* lower bound */
	      expr = la_expr_new( depth, blobs );
	      for (k=0; k<i; k++)
		LA_EXPR_COEF(expr)[k] = A[j][k];
	      LA_EXPR_C0(expr) = (-1) * a[j];

	      for (k=0; k<blobs; k++)
		LA_EXPR_BLOB_COEF(expr)[k] = (-1) * B[j][k];

	      LA_EXPR_DENOM(expr) = (-1) * A[j][i];

	      LA_EXPR_NEXT(expr) = LA_LOOP_LOW(loop);
	      LA_LOOP_LOW(loop) = expr;

	    }
	  else if( A[j][i] > 0 )
	    { /* upper bound */
	      expr = la_expr_new( depth, blobs );
	      for (k=0; k<i; k++)
		LA_EXPR_COEF(expr)[k] = (-1) * A[j][k];
	      LA_EXPR_C0(expr) =  a[j];

	      for (k=0; k<blobs; k++)
		LA_EXPR_BLOB_COEF(expr)[k] =  B[j][k];

	      LA_EXPR_DENOM(expr) =  A[j][i];

	      LA_EXPR_NEXT(expr) = LA_LOOP_UP(loop);
	      LA_LOOP_UP(loop) = expr;

	    }
	}

      /* creates a new system by deleting the i'th variable. */
      
      sizeNew = 0;
      for (j=0; j<size; j++)
	{
	  if(A[j][i] == 0) 
	    { /* no need to delete. */
	      la_vecCopy(A[j], A1[sizeNew], depth);
	      la_vecCopy(B[j], B1[sizeNew], blobs);
	      a1[sizeNew] = a[j];
	      sizeNew++;

	    }
	  else if(A[j][i] > 0)
	    {
	      for (k=0; k<size; k++)
		{
		  if(A[k][i] < 0) 
		    {
		      lcm = la_lcm(A[j][i], A[k][i]);
		      f1 = lcm / A[j][i]; 
		      f2 = (-1) * lcm / A[k][i]; 


		      /* add two vectors. both length i */
		      la_vecAddF(A[j], f1, A[k], f2, A1[sizeNew], depth);
		      la_vecAddF(B[j], f1, B[k], f2, B1[sizeNew], blobs);
		      a1[sizeNew] = f1*a[j] + f2*a[k];


		      sizeNew++;
		    }
		}
	    }

	  if(sizeNew>EQ_MAX) 
	    {
	      fprintf(stderr, "Linear system too large! Change EQ_MAX!\n");
	      exit(0);
	    }

	}

      temp0 = A;
      A = A1;
      A1 = temp0;

      temp0 = B;
      B = B1;
      B1 = temp0;

      temp1 = a;
      a = a1;
      a1 = temp1;

      size = sizeNew;

      
    }

  la_matFree(A, EQ_MAX, depth);
  la_matFree(B, EQ_MAX, blobs);
  la_vecFree(a);

  la_matFree(A1, EQ_MAX, depth);
  la_matFree(B1, EQ_MAX, blobs);
  la_vecFree(a1);


  return(nest_aux);

}

/*--------------------------------------------------------------------------*/
/* la_tar: computes the loop bounds for the target space.                   */
/*         Input:  bounds of the aux space.                                 */
/*         Output: new set of piece-wise linear bounds and linear offsets.  */
/*--------------------------------------------------------------------------*/


LA_LOOPNEST_T la_tar(LA_LOOPNEST_T nest_aux, LA_MATRIX_T H, la_vect stepS)
//      LA_LOOPNEST_T nest_aux;
//      LA_MATRIX_T H;
//      la_vect stepS;
{
  la_matrix inv, H1;
  int det, i, j, k;
  int gcd1, gcd2;

  LA_LOOPNEST_T nest_tar;
  int depth, blobs;
  la_matrix tarF;

  LA_LOOP_T loop_aux, loop_tar;
  LA_EXPR_T expr, expr_aux, expr_tar, expr_tmp;

  depth = LA_NEST_DEPTH(nest_aux);
  blobs = LA_NEST_BLOBS(nest_aux);


  inv = la_matNew(depth, depth);
  det = la_matInv(LA_MATRIX(H), inv, depth);

  /* H1 is H excluding its diag. */
  H1 = la_matNew(depth, depth);
  la_matCopy(LA_MATRIX(H), H1, depth, depth);

  for (i=0; i<depth; i++)
    {
      H1[i][i] = 0;
    }

  /* Computes the linear offsets of loop bounds. */
  /* DON'T deallocate tarF......, its space is used in the nest_tar */
  tarF = la_matNew(depth, depth);
  la_matMult(H1, inv, tarF, depth, depth, depth);

  nest_tar = la_nest_new( depth, blobs );
  for (i=0; i<depth; i++)
    {
      /* get a new loop struct */
      loop_tar = la_loop_new();
      LA_NEST_LOOPS(nest_tar)[i] = loop_tar;

      /* computes the gcd of the coefficients of the linear part. */
      gcd1 = la_gcdV(tarF[i], i);
      
      /* includes the denominator. */
      gcd1 = la_gcd(gcd1, det);

      /* updates. */
      for (j=0; j<i; j++)
	{
	  tarF[i][j] = tarF[i][j] / gcd1;
	}
      expr = la_expr_new( depth, blobs );
      la_vecCopy(tarF[i], LA_EXPR_COEF(expr), depth);
      LA_EXPR_DENOM(expr) = det / gcd1;
      LA_EXPR_C0(expr) = 0;
      la_vecClear(LA_EXPR_BLOB_COEF(expr), blobs);

      LA_LOOP_OFFSET(loop_tar) = expr;
    }


  for (i=0; i<depth; i++)
    { /* for each loop, computes the piece-wise linear portion of bounds. */

      loop_aux = LA_NEST_LOOPS(nest_aux)[i];
      loop_tar = LA_NEST_LOOPS(nest_tar)[i];
      LA_LOOP_STEP(loop_tar) = LA_MATRIX(H)[i][i];

      for ( k=0; k<2; k++ )
	{ /* k=0  processing lower bound expressions */
	  /* k=1  processing upper bound expressions */

	  if(k==0)
	    expr_aux = LA_LOOP_LOW(loop_aux);
	  else
	    expr_aux = LA_LOOP_UP(loop_aux);

	  for(; expr_aux; expr_aux=LA_EXPR_NEXT(expr_aux))
	    {
	      expr_tar = la_expr_new(depth, blobs);
	      la_vecMatMult(LA_EXPR_COEF(expr_aux),
			    depth,
			    inv,
			    depth,
			    LA_EXPR_COEF(expr_tar));


	      /* e.g ( (u+v)/4 + 2+b )/2  <= w */
	      LA_EXPR_C0(expr_tar) = LA_EXPR_C0(expr_aux);
	      la_vecCopy(LA_EXPR_BLOB_COEF(expr_aux), 
			 LA_EXPR_BLOB_COEF(expr_tar),
			 blobs);
	      LA_EXPR_DENOM(expr_tar) = LA_EXPR_DENOM(expr_aux);

	      /* Updates the constants, blobs and the denominators. */
	      /* to ( (u+v + 8+4b) / 8 ) <= w */
	      if(!la_vecIsZero(LA_EXPR_COEF(expr_tar), depth))
		{
		  LA_EXPR_C0(expr_tar) = LA_EXPR_C0(expr_tar) * det;
		  la_vecConst(LA_EXPR_BLOB_COEF(expr_tar), 
			      LA_EXPR_BLOB_COEF(expr_tar),
			      blobs,
			      det);
		  LA_EXPR_DENOM(expr_tar) = LA_EXPR_DENOM(expr_tar) * det;
		}
	      
	      /* Simplify the bound expression */
	      /* by dividing their common divisor.*/
	      /* This is an optimization, not a correctness criterion. */
	      
	      /* Computes the gcd of the coefficients of the linear part. */
	      gcd1 = la_gcdV(LA_EXPR_COEF(expr_tar), depth);
      
	      /* gcd of blob coef's */
	      gcd2 = la_gcdV(LA_EXPR_BLOB_COEF(expr_tar), depth);
	      
	      gcd1 = la_gcd(gcd1, gcd2);
	      
	      /* includes the constant. */
	      gcd1 = la_gcd(gcd1, LA_EXPR_C0(expr_tar));
	      
	      /* includes the denominator. */
	      gcd1 = la_gcd(gcd1, LA_EXPR_DENOM(expr_tar));
	      
	      /* updates. */
	      for (j=0; j<depth; j++)
		{
		  LA_EXPR_COEF(expr_tar)[j] = 
		    LA_EXPR_COEF(expr_tar)[j] / gcd1;
		}
	      LA_EXPR_C0(expr_tar) = 
		LA_EXPR_C0(expr_tar) / gcd1;
	      for (j=0; j<blobs; j++)
		{
		  LA_EXPR_BLOB_COEF(expr_tar)[j] = 
		    LA_EXPR_BLOB_COEF(expr_tar)[j] / gcd1;
		}
	      LA_EXPR_DENOM(expr_tar) = 
		LA_EXPR_DENOM(expr_tar) / gcd1;
	      
	      if(k==0)
		{ 
		  /* lower bound */
		  LA_EXPR_NEXT(expr_tar) = LA_LOOP_LOW(loop_tar);
		  LA_LOOP_LOW(loop_tar) = expr_tar;
		}
	      else
		{
		  /* upper bound */
		  LA_EXPR_NEXT(expr_tar) = LA_LOOP_UP(loop_tar);
		  LA_LOOP_UP(loop_tar) = expr_tar;
		}
	    } /* end of expression list */
	} /* end of k-loop: lower/upper lists */
    } /* end of i-loop: loops */

  
  /* change back according to the step signs. */

  for (i=0; i<depth; i++)
    { /* for each loop, computes the piece-wise linear portion of bounds. */

      loop_tar = LA_NEST_LOOPS(nest_tar)[i];

      if(stepS[i]<0)
	{ /* exchange lower and upper bounds, negate step size */

	  LA_LOOP_STEP(loop_tar) = (-1) * LA_LOOP_STEP(loop_tar);

	  expr_tmp = LA_LOOP_LOW(loop_tar);
	  LA_LOOP_LOW(loop_tar) = LA_LOOP_UP(loop_tar);
	  LA_LOOP_UP(loop_tar) = expr_tmp;
	}
	      
    } /* end of i-loop: loops */

  return(nest_tar);

}




/*--------------------------------------------------------------------------*/
/* la_step_signs: keep track of step signs.                                 */
/*--------------------------------------------------------------------------*/

la_vect  la_step_signs(LA_MATRIX_T T, la_vect S)
//      LA_MATRIX_T T;
//      la_vect S;
{

  la_matrix mat, H;
  la_vect row;

  int n;
  la_vect newS;
  int i, j, fact, minCol;
  int temp;

  mat = LA_MATRIX(T);
  n = LA_MATRIX_ROWSIZE(T);
  H = la_matNew(n, n);

  newS = la_vecNew(n);
  la_vecCopy(S, newS, n);

  la_matCopy(mat, H, n, n);
/*
  la_matId(U, n);
*/

  /* j is the current row of H. */
  for(j=0; j<n; j++)
    {
      
      row = H[j];

      /* make every element of H[j, j:n] positive. */

      for(i=j; i<n; i++)
	if(row[i] < 0)
	  {
	    la_eNegate(H, n, i);
/* no change sign for reversal
	    la_vecNegate(U[i], U[i], n);
*/
	  }

      /* stop when only the diag element is nonzero.           */
      while (la_eFirstNonZero(row, n, j+1) < n)
	{
	  minCol = la_eMinInVec(row, n, j);

	  la_eExchange(H, n, j, minCol);

/*
	  la_rowExchange(U, j, minCol);
*/
	  /* interchange signs as well */
	  temp = newS[j];
	  newS[j] = newS[minCol];
	  newS[minCol] = temp;
	  /* end of interchange signs as well */

	  for (i = j+1; i<n; i++)
	    {
	      fact = row[i] / row[j];
	      /* add (-1)*fact of col j to col i. */
	      la_eAdd(H, n, j, i, (-1)*fact);

	      /* add fact of row i to row j. */
/*
	      la_rowAdd(U, n, i, j, fact);
*/
      

	    }
	}
    }


  return newS;

}


/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

/* end of la-bounds.c */
