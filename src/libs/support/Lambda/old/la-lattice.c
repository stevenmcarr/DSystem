/* $Id: la-lattice.c,v 1.2 1997/03/27 20:47:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/****************************************************************************
 * Copyright (C) 1993, 1992, 1991 Cornell University -- The Typhoon Project
 *
 * la-lattice.c,v 1.1.1.1 1993/09/23 19:09:39 stodghil Exp
 *
 * Routines for integer lattices.
 *
 * Originally written by Wei Li.
 *
 * la-lattice.c,v
 * Revision 1.1.1.1  1993/09/23  19:09:39  stodghil
 * Wei's Lambda Toolkit.
 *
 *
 ****************************************************************************/

#include <libs/support/Lambda/la-private.h>


/*--------------------------------------------------------------------------*/
/* la_lattice: computes the lattice base.                                   */
/*--------------------------------------------------------------------------*/


LA_LATTICE_T la_lattice(nest)
     LA_LOOPNEST_T nest;
{

  LA_LATTICE_T lattice;
  int depth, blobs;
  la_matrix L;

  int i, j, k, step;
  LA_LOOP_T loop;
  LA_EXPR_T expr;

  depth = LA_NEST_DEPTH(nest);
  blobs = LA_NEST_BLOBS(nest);

  lattice = la_lattice_new( depth, blobs );
  L = LA_LATTICE_BASE(lattice);
  LA_LATTICE_DIM(lattice) = depth;
  LA_LATTICE_BLOBS(lattice) = blobs;

  for(i=0; i<depth; i++)
    {
      loop = LA_NEST_LOOPS(nest)[i];

      if(!loop)
	{
	  fprintf(stderr, "Error, no loop %d (in la-lattice)", i);
	  exit(0);
	}

      step = LA_LOOP_STEP(loop);

      if(step == 1)
	{
	  /* if unit step, then 1 */
	  for(k=0; k<depth; k++)
	    L[i][k] = 0;
	  L[i][i] = 1;
							  
	  /* constant */
	  LA_LATTICE_ORIGIN(lattice)[i] = 0;

	  /* blob coef */
	  for(j=0; j<blobs; j++)
	    LA_LATTICE_ORIGIN_BLOB(lattice)[i][j] = 0; 
	}
      else
	{

	  /* need to read the lower bounds only */
	  expr = LA_LOOP_LOW(loop);

	  if(!expr)
	    {
	      fprintf(stderr,"Error, no low bound for loop %d (in la-lattice)",
		      i);
	      exit(0);
	    }
	  
	  if( (LA_EXPR_NEXT(expr)) || (LA_EXPR_DENOM(expr) != 1))
	    {
	      fprintf(stderr, 
		      "Error, lower bound can only be an affine function,");
	      fprintf(stderr, 
		      "if the step size of that loop is non-unit.");
	      exit(0);
	    }

	  /* processing one expression */
	      
	  /* no changne in linear coef  */
	  for(j=0; j<i; j++)
	    L[i][j] = LA_EXPR_COEF(expr)[j] * 
	      LA_LOOP_STEP(LA_NEST_LOOPS(nest)[j]);
	  L[i][i] = step;
	  for(j=i+1; j<depth; j++)
	    L[i][j] = 0;
							  
	  /* constant */
	  LA_LATTICE_ORIGIN(lattice)[i] = LA_EXPR_C0(expr);

	  /* blob coef */
	  for(j=0; j<blobs; j++)
	    LA_LATTICE_ORIGIN_BLOB(lattice)[i][j] = 
	      LA_EXPR_BLOB_COEF(expr)[j];

	} /* loop */
    } /* nest */

  return(lattice);


}



/*--------------------------------------------------------------------------*/
/* la_lattice_new: create a struct for a lattice.                           */
/*--------------------------------------------------------------------------*/


LA_LATTICE_T la_lattice_new(depth, blobs)
     int depth;
     int blobs;
{

  LA_LATTICE_T lattice;

  lattice = (LA_LATTICE_T) malloc (sizeof(LA_LATTICE_T1));

  LA_LATTICE_BASE(lattice) = la_matNew(depth, depth);
  LA_LATTICE_ORIGIN(lattice) = la_vecNew(depth);
  LA_LATTICE_ORIGIN_BLOB(lattice) = la_matNew(depth, blobs);

  return(lattice);

}


/*--------------------------------------------------------------------------*/
/* la_lattice_print: print a lattice.                                       */
/*--------------------------------------------------------------------------*/


void la_lattice_print(lattice)
     LA_LATTICE_T lattice;
{

  int dim;

  dim = LA_LATTICE_DIM(lattice);

  printf("Base is: \n");
  la_printM(LA_LATTICE_BASE(lattice), dim, dim);
  printf("Dimension is: %d \n", dim);

  printf("Constant Origin is : ");
  la_printV(LA_LATTICE_ORIGIN(lattice), dim);

  printf("Blob Origin is : \n");
  la_printM(LA_LATTICE_ORIGIN_BLOB(lattice), 
	    dim,
	    LA_LATTICE_BLOBS(lattice));


}

/*--------------------------------------------------------------------------*/
/* end of la_lattice.c                                                      */
/*--------------------------------------------------------------------------*/
