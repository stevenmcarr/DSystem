/* $Id: la-code.C,v 1.1 1997/04/28 20:20:00 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/****************************************************************************
 * Copyright (C) 1993, 1992, 1991 Cornell University -- The Typhoon Project
 *
 * la-code.c,v 1.1.1.1 1993/09/23 19:09:35 stodghil Exp
 *
 * Routines for restructuring loop nests.
 *
 * Originally written by Wei Li.
 *
 * la-code.c,v
 * Revision 1.1.1.1  1993/09/23  19:09:35  stodghil
 * Wei's Lambda Toolkit.
 *
 *
 ****************************************************************************/

#include <libs/support/Lambda/la-private.h>

/*-----------------------------------------------------------------*/
/* Create an expr struct with the dimension dim and                */
/*	  the total number of blobs blobs                          */
/*-----------------------------------------------------------------*/

LA_EXPR_T la_expr_new(int dim, int blobs )
//      int dim;
//      int blobs;
{

  LA_EXPR_T expr;

  expr = (LA_EXPR_T) calloc(1, sizeof(LA_EXPR_T1));

  LA_EXPR_COEF(expr) = (int *) calloc(dim, sizeof(int));

  LA_EXPR_C0(expr) = 0;
  LA_EXPR_BLOB_COEF(expr) = (int *) calloc(blobs, sizeof(int));

  LA_EXPR_DENOM(expr) = 1;
  LA_EXPR_NEXT(expr) = NULL;

  return(expr);
}

/*-----------------------------------------------------------------*/
/* Free an expr struct.                                            */
/*-----------------------------------------------------------------*/

void la_expr_free(LA_EXPR_T expr )
  // LA_EXPR_T expr;
{
  free ( LA_EXPR_COEF(expr) );

  free ( LA_EXPR_BLOB_COEF(expr) );

  free ( expr );
}

/*-----------------------------------------------------------------*/
/* Create a loop struct.                                           */
/*-----------------------------------------------------------------*/

LA_LOOP_T la_loop_new( )
{

  LA_LOOP_T loop;

  loop =  (LA_LOOP_T) malloc(sizeof(LA_LOOP_T1));

  LA_LOOP_LOW(loop) = NULL;
  LA_LOOP_UP(loop) = NULL;

  LA_LOOP_OFFSET(loop) = NULL;
  return(loop);

}
			

/*-----------------------------------------------------------------*/
/* Free a loop struct.                                             */
/*-----------------------------------------------------------------*/

void la_loop_free(LA_LOOP_T loop )
  // LA_LOOP_T loop;
{

  LA_EXPR_T expr, expr1;

  /* free lower bounds */
  expr = LA_LOOP_LOW(loop);
  while ( expr )
    {
      expr1 = expr;
      expr = LA_EXPR_NEXT(expr);
      la_expr_free(expr1);
    }

  /* free upper bounds */
  expr = LA_LOOP_UP(loop);
  while ( expr )
    {
      expr1 = expr;
      expr = LA_EXPR_NEXT(expr);
      la_expr_free(expr1);
    }
  

  /* free offset */
  la_expr_free (LA_LOOP_OFFSET(loop));


  free ( loop );

}
			
			

/*-----------------------------------------------------------------*/
/* Create a loop nest struct.                                      */
/*-----------------------------------------------------------------*/

LA_LOOPNEST_T la_nest_new(int depth, int blobs)
  // int depth;
  // int blobs;
{

  LA_LOOPNEST_T nest;

  nest =  (LA_LOOPNEST_T) malloc(sizeof(LA_LOOPNEST_T1));

  LA_NEST_LOOPS(nest) = (LA_LOOP_T *) calloc(depth, sizeof(LA_LOOP_T));
  LA_NEST_DEPTH(nest) = depth;
  LA_NEST_BLOBS(nest) = blobs;

  return( nest );

}

/*-----------------------------------------------------------------*/
/* Free a loop nest struct.                                        */
/*-----------------------------------------------------------------*/

void la_nest_free(LA_LOOPNEST_T nest )
  // LA_LOOPNEST_T nest;
{

  int depth, i;

  depth = LA_NEST_DEPTH(nest);

  for (i=0; i<depth; i++)
    la_loop_free ( LA_NEST_LOOPS(nest)[i] );

  free ( nest );

}
			

/*-----------------------------------------------------------------*/
/* Compute the new loop nest.                                      */
/*-----------------------------------------------------------------*/

LA_LOOPNEST_T la_nest(LA_LOOPNEST_T nest,LA_MATRIX_T T)
//      LA_LOOPNEST_T nest;
//      LA_MATRIX_T T;
{
  LA_LOOPNEST_T nest_aux, nest_tar;

  int depth, blobs;
  int i, j;

  LA_LATTICE_T lattice;

  LA_MATRIX_T T1, H, U;
  LA_LOOP_T loop;
  LA_EXPR_T expr;

  la_vect origin;
  la_matrix origin_blob;

  la_vect stepS;

  int f;

  if(LA_MATRIX_ROWSIZE(T) != LA_MATRIX_COLSIZE(T) )
    {
      fprintf(stderr, "Not a square matrix! \n");
      exit(0);
    }

  if(la_print)
    {
      printf("The init nest is :\n");
      la_nest_print(nest, 'i');
    }

  if(la_matrix_is_id(T))
    return nest;

  depth = LA_NEST_DEPTH(nest);
  blobs = LA_NEST_BLOBS(nest);

  
  stepS = la_vecNew(depth);
  for(i=0; i<depth; i++)
    {
      /* read the step sign of loop i */
      
      if (LA_LOOP_STEP(LA_NEST_LOOPS(nest)[i]) > 0)
	stepS[i] = 1;
      else
	stepS[i] = -1;
    }

  /* let x be the original space, and y be the base space. */
  /* x = L*y + origin, where L is the lattice base.        */
  /* T*x = T*(L*y + origin) = T*L*y + T*origin             */

  /* Let z be the target space.             */
  /* z = Tx = T(Ly+origin) = TLy + T*origin */
  /* z1 = z-T*origin = TLy = HUy            */

  lattice = la_lattice( nest );

  /* let T1 = T*L */
  T1 = la_matrix_new(depth, depth);
  la_matMult(LA_MATRIX(T), 
	     LA_LATTICE_BASE(lattice),
	     LA_MATRIX(T1),
	     depth,
	     depth,
	     depth);

  if(la_print)
    {
      printf("Lattice is: \n");
      la_lattice_print(lattice);
      printf("T1 is: \n");
      la_matrix_print(T1);
    }

  H= la_matrix_new(depth, depth);
  U = la_matrix_new(depth, depth);
  la_matHermite(LA_MATRIX(T1), depth, LA_MATRIX(H), LA_MATRIX(U));
  
  if(la_print)
    {
      printf("T1 = HU \n");
      printf("H is: \n");
      la_matrix_print(H);
      
      printf("U is: \n");
      la_matrix_print(U);
    }

  /* computes the bounds for the auxiliary space. */

  nest_aux = la_aux(nest, U);

  if(la_print)
    {
      printf("The aux nest is :\n");
      la_nest_print(nest_aux, 'p');
    }

  /* compute the new step signs */

  stepS = la_step_signs(T1, stepS);

  /* compute the target space z1 */
  nest_tar = la_tar(nest_aux, H, stepS);

  /* compute the target space z */
  /* z = z1 + T*origin          */
 
  origin = la_vecNew(depth);
  origin_blob = la_matNew(depth, blobs);

  la_matVecMult(LA_MATRIX(T),
		depth,
		depth,
		LA_LATTICE_ORIGIN(lattice),
		origin);

  la_matMult(LA_MATRIX(T),
	     LA_LATTICE_ORIGIN_BLOB(lattice),
	     origin_blob,
	     depth,
	     depth,
	     blobs);

  for(i=0; i<depth; i++)
    {
      loop = LA_NEST_LOOPS(nest_tar)[i];
      expr = LA_LOOP_OFFSET(loop);

      /* from (i+j)/2 (no const nor blobs) --> Add 3+b --> (i+j+2*3+2*b)/2  */
      /* constant */
      if(la_vecIsZero(LA_EXPR_COEF(expr), depth))
	f = 1; 
      else
	f = LA_EXPR_DENOM(expr);

      LA_EXPR_C0(expr) += f * origin[i];
	  
      /* blob coef */
      for(j=0; j<blobs; j++)
	LA_EXPR_BLOB_COEF(expr)[j] += f * origin_blob[i][j];
    }

  if(la_print)
    {
      printf("The tar nest is :\n");
      la_nest_print(nest_tar, 'u');
    }

  return(nest_tar);

}



/*-----------------------------------------------------------------*/
/* Create a vector struct of length size.                          */
/*-----------------------------------------------------------------*/

LA_VECTOR_T la_vector_new( int size )
  // int size;
{

  LA_VECTOR_T v;

  v = (LA_VECTOR_T) malloc(sizeof(LA_VECTOR_T1));

  LA_VECTOR_COEF(v) = (int *) calloc(size, sizeof(int));

  LA_VECTOR_SIZE(v) = size;
  LA_VECTOR_DENOM(v) = 1;

  return(v);
}
			

/*--------------------------------------------------------------------------*/
/* la_vector: computes the new expression in the transformed loop nest.     */
/*--------------------------------------------------------------------------*/

LA_VECTOR_T la_vector(LA_MATRIX_T invT, LA_VECTOR_T v)
//      LA_MATRIX_T invT;
//      LA_VECTOR_T v;

{
  LA_VECTOR_T v1;
  int depth;

  if (LA_MATRIX_ROWSIZE(invT) != LA_MATRIX_COLSIZE(invT))
    {
      fprintf(stderr, " Not a square matrix! (in la_expr) \n");
      exit(0);
    }

  depth = LA_MATRIX_ROWSIZE(invT);

  v1 = la_vector_new(depth);
  LA_VECTOR_DENOM(v1) = LA_VECTOR_DENOM(v) * LA_MATRIX_DENOM(invT);

  la_vecMatMult(LA_VECTOR_COEF(v), 
		depth,
		LA_MATRIX(invT),
		depth,
		LA_VECTOR_COEF(v1));

  LA_VECTOR_SIZE(v1) = LA_VECTOR_SIZE(v);

  return(v1);
}
  
/*--------------------------------------------------------------------------*/

/*-----------------------------------------------------------------*/
/* end of code.c                                                   */
/*-----------------------------------------------------------------*/
