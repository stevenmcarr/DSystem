/* $Id: la-dep2.C,v 1.1 1997/04/28 20:20:00 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/****************************************************************************
 * Copyright (C) 1993, 1992, 1991 Cornell University -- The Typhoon Project
 *
 * la-dep2.c,v 1.1.1.1 1993/09/23 19:09:35 stodghil Exp
 *
 * Originally written by Wei Li.
 *
 * la-dep2.c,v
 * Revision 1.1.1.1  1993/09/23  19:09:35  stodghil
 * Wei's Lambda Toolkit.
 *
 *
 ****************************************************************************/

#include <libs/support/Lambda/la-private.h>


/*--------------------------------------------------------------------------*/
/* la_vec_depV_mult: dot product of an integer vector and a dep vector.      */
/*--------------------------------------------------------------------------*/

LA_DEP_T la_vec_depV_mult(la_vect vec1, LA_DEP_V_T d, int dim)
//      la_vect vec1;
//      LA_DEP_V_T d;
//      int dim;
{

  int j;
  int pVal, cVal, vVal;
  int vecSize;
  LA_DIR_T pType, cType;
  LA_DEP_T dep;

  vecSize = dim;

  pType = dK;
  pVal = 0;
  for(j=0; j<vecSize; j++)
    {
      /* current type lad value */
      cType = LA_DEP_DIR( LA_DEP_V_VECTOR(d)[j] );
      cVal = LA_DEP_DIST( LA_DEP_V_VECTOR(d)[j] );
      
      vVal = vec1[j];

/*
  fprintf(stderr, "cType is: ");
  la_printDirV(&cType, 1); 
  fprintf(stderr, "pType is: ");
  la_printDirV(&pType, 1); 
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

  LA_DEP_DIR(dep) = pType;
  LA_DEP_DIST(dep) = pVal;


  return(dep);
  
  
}

/*--------------------------------------------------------------------------*/
/* la_vec_dep_mult: dot product of an integer vector and a dep vector.      */
/*--------------------------------------------------------------------------*/

LA_DIR_T la_vec_depVec_mult(la_vect vec1, LA_DEP_V_T d, int dim)
//      la_vect vec1;
//      LA_DEP_V_T d;
//      int dim;
{
  LA_DEP_T dep;

  dep = la_vec_depV_mult(vec1, d, dim);

  /* converting dist to directions (tags) */
  if (LA_DEP_DIR(dep) == dK)
    {
      if(LA_DEP_DIST(dep)>0)
	return dLT;
      else if(LA_DEP_DIST(dep)<0)
	return dGT;
      else
	return dEQ;
    }
  else
    return LA_DEP_DIR(dep);

  
}

/*--------------------------------------------------------------------------*/
/* la_depMult: dot product of an integer vector and a dep matrix.           */
/*--------------------------------------------------------------------------*/

void  la_depMult(la_vect vec1, LA_DEP_M_T D, LA_DIR_T *prod)
//      la_vect vec1;
//      LA_DEP_M_T D;
//      LA_DIR_T *prod;
{

  int i;
  int vecSize;
  LA_DEP_V_T d;

  vecSize = LA_DEP_M_DIM(D);

  d = LA_DEP_M_VECTORS(D);
  i = 0;
  for(; d; d = LA_DEP_V_NEXT(d))
    prod[i++] = la_vec_depVec_mult(vec1, d, vecSize);

}

/*--------------------------------------------------------------------------*/
/* la_allNegEq: check if every element is neg or zero.                      */
/*--------------------------------------------------------------------------*/

int la_allNegEq(LA_DIR_T *flagV, int flagSize)
//      LA_DIR_T *flagV;
//      int flagSize;
{
  int allNeg = TRUE, allEq = TRUE;
  int i;

  for (i=0; (i<flagSize) && allNeg; i++)
    switch(flagV[i])
      {
      case dLT:
      case dSTAR:
      case dLEQ:
	allNeg = FALSE;
	allEq = FALSE;
	
	break;

      case dGT:
      case dGEQ:
	allEq = FALSE;
	break;

      case dEQ:
	break;
	
      default:
	fprintf(stderr, "In allNegEq: unexpected dist/dir\n");
	
      }

  if(allEq==TRUE) allNeg = FALSE;
    
  return( allNeg );


}

/*--------------------------------------------------------------------------*/
/* la_allPosEq: check if every element is positive or zero.                 */
/*--------------------------------------------------------------------------*/

int la_allPosEq(LA_DIR_T *flagV, int flagSize)
//      LA_DIR_T *flagV;
//      int flagSize;
{
  int allPos = TRUE;
  int i;

  for (i=0; (i<flagSize) && allPos; i++)
    switch(flagV[i])
      {
      case dLT:
      case dLEQ:
      case dEQ:
	
	break;

      case dGT:
      case dGEQ:
      case dSTAR:
	allPos = FALSE;
	break;
	
      default:
	fprintf(stderr, "In la_allPosEq : unexpected dist/dir\n");
	
      }
    
  return( allPos );


}

/*--------------------------------------------------------------------------*/
/* la_flagNegate: negate the dep flag.                                      */
/*--------------------------------------------------------------------------*/

void  la_flagNegate(LA_DIR_T *flagV, int flagSize)
//      LA_DIR_T *flagV;
//      int flagSize;
{
  int i;

  for (i=0; i<flagSize; i++)
    switch(flagV[i])
      {
      case dLT:
	flagV[i] = dGT;
	break;

      case dSTAR:
      case dEQ:
	break;

      case dLEQ:
	flagV[i] = dGEQ;
	break;

      case dGT:
	flagV[i] = dLT;
	break;

      case dGEQ:
	flagV[i] = dLEQ;
	break;
	
      default:
	fprintf(stderr, "In la_flagNegate: unexpected dist/dir\n");
	
      }
    

}

/*--------------------------------------------------------------------------*/
/* la_delPosDep: delete the dependence with the positive flag.              */
/*--------------------------------------------------------------------------*/

void la_delPosDep(LA_DEP_M_T D, LA_DIR_T *flagV)
//      LA_DEP_M_T D;
//      LA_DIR_T * flagV;
{
  int i, size;

  LA_DEP_V_T d, prev;

  d = LA_DEP_M_VECTORS(D);
  size = LA_DEP_M_SIZE(D);
  prev = NULL;

  for (i=0; i<size; i++)
    if( flagV[i] == dLT ) 
      {
	if (!prev)
	  {
	    LA_DEP_M_VECTORS(D) = LA_DEP_V_NEXT(d);
	    la_dep_vec_free(d);
	    d = LA_DEP_M_VECTORS(D);
	  } 
	else
	  {
	    LA_DEP_V_NEXT(prev) = LA_DEP_V_NEXT(d);
	    la_dep_vec_free(d);
	    d = LA_DEP_V_NEXT(d);
	  }
	LA_DEP_M_SIZE(D) --;
      }
    else
      {
	prev = d;
	d = LA_DEP_V_NEXT(d);
      }
  


}


/*--------------------------------------------------------------------------*/
/* la_firstEk: return the first Ek within 90 degrees of every dep.          */
/*--------------------------------------------------------------------------*/

int  la_firstEk(LA_DEP_M_T D)
  // LA_DEP_M_T D;
{

  int j, k, found, colSize;
  LA_DEP_V_T d;


  k = LA_DEP_M_DIM(D);
  colSize = k;

  d = LA_DEP_M_VECTORS(D);
  for(; d; d = LA_DEP_V_NEXT(d))
    {
      
      found = FALSE;
      for(j=0; (j<colSize) && !found; j++) 
	switch(LA_DEP_DIR( LA_DEP_V_VECTOR(d)[j] ) )
	  {
	  case dLT:
	  case dLEQ:
	    found = TRUE;
	    break;

	  case dK:
	    if( LA_DEP_DIST( LA_DEP_V_VECTOR(d)[j] ) >0 )
	      found = TRUE;
	    else if( LA_DEP_DIST( LA_DEP_V_VECTOR(d)[j] ) <0 )
	      fprintf(stderr, 
		      "Error:(in la_firstEk) not lex graphically positive!\n");

	    break;
	      
	  case dEQ:
	    break;
	    
	  case dSTAR:
	  case dGT:
	  case dGEQ:
	    fprintf(stderr,
		    "Error:(in la_firstEk) not lex graphically positive!\n");
	    break;
	
	  default:
	    fprintf(stderr, "In la_flagNegate: unexpected dist/dir\n");
	
	  }

      if(j<k) k = j;
    }

  return(k);

}


/*-----------------------------------------------------------------*/
/* la_dep_add_to_list:                                             */
/*   add the dependence vector to the matrix.                      */
/*-----------------------------------------------------------------*/

void la_dep_add_to_list(LA_DEP_V_T d, LA_DEP_M_T D)
//      LA_DEP_V_T d;
//      LA_DEP_M_T D;
{

  LA_DEP_V_NEXT(d) = LA_DEP_M_VECTORS(D);
  LA_DEP_M_VECTORS(D) = d;

  LA_DEP_M_SIZE(D)++;

}



/*-----------------------------------------------------------------*/
/* la_dep_add_to_list_last:                                        */
/*   add the dependence vector to the matrix (last in the list).   */
/*-----------------------------------------------------------------*/

void la_dep_add_to_list_last(LA_DEP_V_T d, LA_DEP_M_T D, LA_DEP_V_T last)
//      LA_DEP_V_T d;
//      LA_DEP_M_T D;
//      LA_DEP_V_T last;
{

  if(last == NULL)
    LA_DEP_M_VECTORS(D) = d;
  else
    LA_DEP_V_NEXT(last) = d;

  LA_DEP_M_SIZE(D)++;

}


/*--------------------------------------------------------------------------*/
/* la_redundantDepElim: eliminates the redundant dep's.                     */
/*--------------------------------------------------------------------------*/

/* returns the size of the new dependence matrix.  */

int la_redundantDepElim(la_matrix depM, int depSize, int nestLevel, la_matrix newM)
//      la_matrix depM, newM;
//      int depSize, nestLevel;
{

  int i, j, k, rednt;
  la_vect row;


  j = 0;
  for(i=0; i<depSize; i++)
    {
      row = depM[i];

      rednt = 0;
      for(k=0; (k<j) && !rednt; k++)
	rednt = la_vecEq(row, newM[k], nestLevel*2);
      
      if(!rednt) 
	{
	  newM[j] = row;
	  j++;
	}
    }
  
  return( j );
}


/*-----------------------------------------------------------------*/
/* la_dep_vec_copy:                                                */
/*   copy a dependence vector.                                     */
/*-----------------------------------------------------------------*/

void la_dep_vec_copy(LA_DEP_V_T d, LA_DEP_V_T d1, int dim)
//      LA_DEP_V_T d;
//      LA_DEP_V_T d1;
//      int dim;
{

  int i;
  LA_DEP_T *vec, *vec1;

  for (i=0; i<dim; i++)
    {
      vec = LA_DEP_V_VECTOR(d);
      vec1 = LA_DEP_V_VECTOR(d1);
      
      LA_DEP_DIR( vec1[i] ) = LA_DEP_DIR( vec[i] );
      LA_DEP_DIST( vec1[i] ) = LA_DEP_DIST( vec[i] );
    }



}

/*--------------------------------------------------------------------------*/
/* la_dep_eq: check if two dependences are equal.                           */
/*--------------------------------------------------------------------------*/

int la_dep_eq( LA_DEP_T d1, LA_DEP_T d2 )
//      LA_DEP_T d1;
//      LA_DEP_T d2;
{


  if ( (LA_DEP_DIR(d1) == LA_DEP_DIR(d2)) &&
       (LA_DEP_DIST(d1) == LA_DEP_DIST(d2)) )
    return(1);
  else
    return(0);


}

/*--------------------------------------------------------------------------*/
/* la_dep_vec_eq: check if two dependence vectors are equal.                */
/*--------------------------------------------------------------------------*/

int la_dep_vec_eq( LA_DEP_V_T d1, LA_DEP_V_T d2, int size )
//      LA_DEP_V_T d1;
//      LA_DEP_V_T d2;
//      int size;
{

  int i;
  
  int eq = 1;

  for(i=0; i<size; i++)
    {
      if(! (la_dep_eq(LA_DEP_V_VECTOR(d1)[i],
		   LA_DEP_V_VECTOR(d2)[i]))
	 )
	{
	  eq = 0;
	  break;
	}
    }

  return(eq);


}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/


