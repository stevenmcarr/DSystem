/* $Id: la-dep1.C,v 1.1 1997/04/28 20:20:00 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/****************************************************************************
 * Copyright (C) 1993, 1992, 1991 Cornell University -- The Typhoon Project
 *
 * la-dep1.c,v 1.1.1.1 1993/09/23 19:09:36 stodghil Exp
 *
 *    Routines for dependences.
 *
 * Originally written by Wei Li.
 *
 * la-dep1.c,v
 * Revision 1.1.1.1  1993/09/23  19:09:36  stodghil
 * Wei's Lambda Toolkit.
 *
 *
 ****************************************************************************/

#include <libs/support/Lambda/la-private.h>

/*-----------------------------------------------------------------*/
/* Create a dependence vector of size size.                        */
/*-----------------------------------------------------------------*/

LA_DEP_V_T la_dep_vec_new( int size )
  // int size;
{
  LA_DEP_V_T d;

  d = (LA_DEP_V_T) malloc(sizeof(LA_DEP_V_T1));

  LA_DEP_V_VECTOR(d) = (LA_DEP_T *) calloc(size, sizeof(LA_DEP_T));

  LA_DEP_V_NEXT(d) = NULL;

  return( d );

}

/*-----------------------------------------------------------------*/
/* Free a dev vector.                                              */
/*-----------------------------------------------------------------*/

void la_dep_vec_free( LA_DEP_V_T d )
  // LA_DEP_V_T d;
{

  free( d);

}

/*-----------------------------------------------------------------*/
/* Create a dep matrix.                                            */
/*-----------------------------------------------------------------*/

LA_DEP_M_T la_dep_matrix_new(int dim, int size)
  // int dim;
  // int size;
{

  LA_DEP_M_T D;

  D = (LA_DEP_M_T) malloc(sizeof(LA_DEP_M_T1));

  LA_DEP_M_VECTORS(D) = NULL;

  LA_DEP_M_DIM(D) = dim;

  LA_DEP_M_SIZE(D) = size;

  return(D);
}

/*-----------------------------------------------------------------*/
/* Free a dep matrix.                                              */
/*-----------------------------------------------------------------*/

void la_dep_matrix_free(LA_DEP_M_T D)
  // LA_DEP_M_T D;
{
  LA_DEP_V_T d, d1;

  d = LA_DEP_M_VECTORS(D);

  while ( d )
    {
      d1 = d;
      d = LA_DEP_V_NEXT(d);
      la_dep_vec_free(d1);
    }

  free ( D );


}

/*-----------------------------------------------------------------*/
/* Create a legal dep matrix.                                      */
/*   e.g. (*, 1) becomes (<, 1) and (=, 1). The illegal component  */
/*   (>, 1) is deleted.                                            */ 
/*-----------------------------------------------------------------*/
			
LA_DEP_M_T la_dep_legal(LA_DEP_M_T D)
  // LA_DEP_M_T D;
{

  LA_DEP_M_T D1;
  int dim;
  LA_DEP_V_T d, d1, temp, last;
  LA_DEP_T * vec;

  int done, j;

  dim = LA_DEP_M_DIM(D);

  D1 = la_dep_matrix_new(dim, 0);
  last = NULL;

  temp = la_dep_vec_new(dim);

  d = LA_DEP_M_VECTORS(D);
  for(; d; d = LA_DEP_V_NEXT(d))
    {

      la_dep_vec_copy(d, temp, dim);
      
      vec = LA_DEP_V_VECTOR(temp);
      done = 0;
      for(j=0; (j<dim) && !done ; j++)
        {
          switch(LA_DEP_DIR(vec[j]))
            {
            case dK:
              if(LA_DEP_DIST(vec[j])>0)
                {
                  /* legal */
		  d1 = la_dep_vec_new(dim);
		  la_dep_vec_copy(temp, d1, dim);

		  la_dep_add_to_list_last(d1, D1, last);
		  last = d1;

                  done = 1;
                }
              else if(LA_DEP_DIST(vec[j])<0)
                {
                  /* illegal */
                  done = 1;
                }

              /* do nothing for 0 */

              break;

            /* legal */
            case dLT:
	      d1 = la_dep_vec_new(dim);
	      la_dep_vec_copy(temp, d1, dim);

	      la_dep_add_to_list_last(d1, D1, last);
	      last = d1;

	      done = 1;
              break;

            /* illegal */
            case dGT:
              done = 1;
              break;

            /* split the legal component */
            case dLEQ:
            case dSTAR:
              /* copy the legal part */
	      d1 = la_dep_vec_new(dim);
	      la_dep_vec_copy(temp, d1, dim);
              LA_DEP_DIR( LA_DEP_V_VECTOR(d1)[j] ) = dLT;

	      la_dep_add_to_list_last(d1, D1, last);
	      last = d1;


              /* continue on the other part */
              LA_DEP_DIR( LA_DEP_V_VECTOR(temp)[j] ) = dEQ;
              break;

            case dGEQ:
              /* delete the illegal part, lad */
              /* continue on the other part   */
              LA_DEP_DIR( LA_DEP_V_VECTOR(temp)[j] ) = dEQ;
              break;

            case dEQ:
              break;

            default:
              fprintf(stderr, "In la_legalDepM: unexpected dist/dir\n");
            }
        }

    }


  return(D1);

}




/*--------------------------------------------------------------------------*/
/* la_dep_no_redundant: eliminates the redundant dep's.                     */
/*--------------------------------------------------------------------------*/

LA_DEP_M_T la_dep_no_redundant(LA_DEP_M_T D)
  // LA_DEP_M_T D;
{

  LA_DEP_M_T D1;
  int dim, newdep;
  LA_DEP_V_T d, d1, next, last;

  dim = LA_DEP_M_DIM(D);

  D1 = la_dep_matrix_new(dim, 0);
  last = NULL;

  d = LA_DEP_M_VECTORS(D);
  for(; d; d=next ) 
    {
      next = LA_DEP_V_NEXT(d);
      LA_DEP_V_NEXT(d) = NULL;
      
      newdep = 1;
      for(d1=LA_DEP_M_VECTORS(D1); d1; d1=LA_DEP_V_NEXT(d1))
	if(la_dep_vec_eq(d, d1, dim))
	  {
	    newdep = 0;
	    break;
	  }
	
      if(newdep)
	{
	  la_dep_add_to_list_last(d, D1, last);
	  last = d;
	}

    }
  
  return( D1 );
}


/*--------------------------------------------------------------------------*/
/* la_dep_trans: new dep after transformation.                              */
/*--------------------------------------------------------------------------*/

LA_DEP_V_T la_dep_trans(LA_DEP_V_T d, LA_MATRIX_T T)
//      LA_DEP_V_T d;
//      LA_MATRIX_T T;
{

  int dim;
  LA_DEP_V_T d1;
  int i;

  dim = LA_MATRIX_ROWSIZE(T);

  d1 = la_dep_vec_new(dim);

  for(i=0; i<dim; i++)
    {
      LA_DEP_V_VECTOR(d1)[i] = la_vec_depV_mult(LA_MATRIX(T)[i],
						d,
						dim);
    }

  return d1;
}


/*-----------------------------------------------------------------*/
/* end of la-dep1.c                                                */
/*-----------------------------------------------------------------*/
