/* $Id: la-print1.c,v 1.2 1997/03/27 20:47:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/****************************************************************************
 * Copyright (C) 1993, 1992, 1991 Cornell University -- The Typhoon Project
 *
 * la-print1.c,v 1.1.1.1 1993/09/23 19:09:37 stodghil Exp
 *
 * Print routines.
 *
 * Originally written by Wei Li.
 *
 * la-print1.c,v
 * Revision 1.1.1.1  1993/09/23  19:09:37  stodghil
 * Wei's Lambda Toolkit.
 *
 *
 ****************************************************************************/

#include <libs/support/Lambda/la-private.h>

/*-----------------------------------------------------------------*/
/* Print a matrix.                                                 */
/*-----------------------------------------------------------------*/

void la_matrix_print( mat )
     LA_MATRIX_T mat;
{

  la_printM(LA_MATRIX(mat), 
	    LA_MATRIX_ROWSIZE(mat),
	    LA_MATRIX_COLSIZE(mat));

}


/*-----------------------------------------------------------------*/
/* Print a loop nest.                                              */
/* If the start is i, then index variables are i, j, k..           */
/* If the start is u, then index variables are u, v, w..           */
/* '/' represents floor, and '\' represents ceiling.               */
/*-----------------------------------------------------------------*/

void la_nest_print( nest, start )
     LA_LOOPNEST_T nest;
     int start;
{
  int depth, blobs;

  LA_LOOP_T loop;
  
  int i;

  depth = LA_NEST_DEPTH(nest);
  blobs = LA_NEST_BLOBS(nest);

  printf("Loopnest: depth=%d, blobs=%d \n", depth, blobs);
  for (i=0; i<depth; i++)
    {
      loop = LA_NEST_LOOPS(nest)[i];

      if(!loop) 
	{
	  printf("No loop %d (in la_nest_print) \n", i);
	  exit(0);
	}

      printf("Loop %c--------\n", start+i);

      la_loop_print( loop,  depth,  blobs,  start);

      printf("\n");
    }/* end of the nest */
  
  printf("\n\n");
}


/*-----------------------------------------------------------------*/
/* Print a loop.                                                   */
/*-----------------------------------------------------------------*/

void la_loop_print( loop, depth, blobs, start )
     LA_LOOP_T loop;
     int depth;
     int blobs;
     int start;
{
  int step;

  LA_EXPR_T expr;
  
  int k;


  if(!loop) 
    {
      printf("empty loop (in la_loop_print) \n");
      exit(0);
    }
  
  expr = LA_LOOP_OFFSET(loop);

  step = LA_LOOP_STEP(loop);
  
  if(expr)
    {
      printf("  linear offset: \n");
      la_expr_print( expr,  depth,  blobs, start);

    }
  
  for(k=0; k<2; k++)
    {
      if(k==0)
	{
	  printf("  lower bound: \n");
	  expr = LA_LOOP_LOW(loop);
	}
      else
	{
	  printf("  upper bound: \n");
	  expr = LA_LOOP_UP(loop);
	}
      
      for (; expr ; expr=LA_EXPR_NEXT(expr) )
	la_expr_print( expr,  depth,  blobs, start);
      
    } /* end of the loop */
  printf("  step size = %d \n", step);
  
}


/*-----------------------------------------------------------------*/
/* Print an expr.                                                  */
/*-----------------------------------------------------------------*/

void la_expr_print( expr, depth, blobs, start )
     LA_EXPR_T expr;
     int depth;
     int blobs;
     int start;
{
  
  int empty;

  printf("     linear: ");
  empty = la_printLinearE(LA_EXPR_COEF(expr),
			  depth,
			  start);
  printf("  constant: %d ", LA_EXPR_C0(expr));
  printf("  blobs: ");
  empty = la_printLinearE(LA_EXPR_BLOB_COEF(expr),
			  blobs,
			  'M');
  printf("  denom: %d ", LA_EXPR_DENOM(expr));
  
  printf("\n");

}

/*-----------------------------------------------------------------*/
/* Print a dependence.                                             */
/*-----------------------------------------------------------------*/

void la_dep_print(d)
     LA_DEP_T d;
{

  switch( LA_DEP_DIR(d) )
    {
    case dK: 
      printf("%3d", LA_DEP_DIST(d));
      break;
    case dLT:
      printf("  <");
      break;
    case dLEQ:
      printf(" <=");
      break;
    case dEQ:
      printf("  =");
      break;
    case dGEQ:
      printf(" >=");
      break;
    case dGT:
      printf("  >");
      break;
    case dDOT:
      printf("  .");
      break;
    case dSTAR:
      printf("  *");
      break;
    default: 
      printf("error-- unknown dep type. (in la_dep_matrix_print)\n");
	      break;
    } /* end of switch */
  
  
}


/*-----------------------------------------------------------------*/
/* Print a dependence vector.                                      */
/*-----------------------------------------------------------------*/

void la_dep_vec_print(d, dim)
     LA_DEP_V_T d;
     int dim;
{
  int i;

  for(i=0; i<dim; i++)
    la_dep_print(LA_DEP_V_VECTOR(d)[i]);

  printf("\n");
  

}

/*-----------------------------------------------------------------*/
/* Print a dependence matrix.                                      */
/*-----------------------------------------------------------------*/

void la_dep_matrix_print(D)
     LA_DEP_M_T D;
{

  int dim;
  int i;
  LA_DEP_V_T d;

  dim = LA_DEP_M_DIM(D);

  for(i=0; i<dim; i++)
    {
      d = LA_DEP_M_VECTORS(D);
      for(; d; d = LA_DEP_V_NEXT(d))
	{
    
	  la_dep_print(LA_DEP_V_VECTOR(d)[i]);
	} /* end of one row */
      printf("\n");
    } /* end of matrix */

}

/*-----------------------------------------------------------------*/
/* end of la-print1.c                                              */
/*-----------------------------------------------------------------*/
