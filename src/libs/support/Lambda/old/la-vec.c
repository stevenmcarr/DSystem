/****************************************************************************
 * Copyright (C) 1993, 1992, 1991 Cornell University -- The Typhoon Project
 *
 * la-vec.c,v 1.1.1.1 1993/09/23 19:09:41 stodghil Exp
 *
 * Routines about vector operations.
 *
 * Originally written by Wei Li.
 *
 * la-vec.c,v
 * Revision 1.1.1.1  1993/09/23  19:09:41  stodghil
 * Wei's Lambda Toolkit.
 *
 *
 ****************************************************************************/

#include <la-private.h>


/*--------------------------------------------------------------------------*/
/* la_vecNegate:  -- negate one vector.                                     */
/*--------------------------------------------------------------------------*/

void la_vecNegate (vec1, vec2, n)
     la_vect vec1;
     la_vect vec2;
     int n;
{
  int i;


  for (i = 0; i < n; i++)
    vec2[i] =  (-1) * vec1[i];
    

}

/*--------------------------------------------------------------------------*/
/* la_vecConst:    mult vector by a constant.                               */
/*--------------------------------------------------------------------------*/


void la_vecConst (vec1, vec2, n, c)
     la_vect vec1;
     la_vect vec2;
     int n;
     int c;
{
  int i;


  for (i = 0; i < n; i++)
    vec2[i] =  c * vec1[i];
    

}


/*--------------------------------------------------------------------------*/
/* la_vecAdd: adds two vectors.                                             */
/*--------------------------------------------------------------------------*/

void la_vecAdd(vec1, vec2, vec3, n)
     la_vect vec1;
     la_vect vec2;
     la_vect vec3;
     int n;
{
  int i;

      
  for (i=0; i<n; i++)
    vec3[i] = vec1[i] + vec2[i];
	
    
}

/*--------------------------------------------------------------------------*/
/* la_vecAddF: adds two vectors.                                            */
/*--------------------------------------------------------------------------*/

void la_vecAddF(vec1, f1, vec2, f2, vec3, n)
     la_vect vec1;
     int f1;
     la_vect vec2;
     int f2;
     la_vect vec3;
     int n;
{
  int i;

      
  for (i=0; i<n; i++)
    vec3[i] = f1*vec1[i] + f2*vec2[i];
	
    
}

/*--------------------------------------------------------------------------*/
/* la_vecConcat: concat two vectors.                                        */
/*--------------------------------------------------------------------------*/

void  la_vecConcat(vec1, n1, vec2, n2, vec3)
     la_vect vec1;
     int n1;
     la_vect vec2;
     int n2;
     la_vect vec3;
{
  int i;


  for(i=0; i<n1; i++)
    vec3[i] = vec1[i];

  for(i=0; i<n2; i++)
    vec3[n1+i] = vec2[i];


}

/*--------------------------------------------------------------------------*/
/* la_vecCopy: make copy of the vector.                                     */
/*--------------------------------------------------------------------------*/

void la_vecCopy(vec1, vec2, n)
     la_vect vec1;
     la_vect vec2;
     int n;
{
  int i;

  for (i=0; i<n; i++)
    vec2[i] = vec1[i];
	
    
}


/*--------------------------------------------------------------------------*/
/* la_vecIsZero: check if it is zero vector.                                */ 
/*--------------------------------------------------------------------------*/

int la_vecIsZero(vec1, n)
     la_vect vec1;
     int n;
{
  int i, yes;

  yes = 1;
  for (i=0; i<n; i++)
    yes = yes && (vec1[i] == 0);
	
  return(yes);
    
}


/*--------------------------------------------------------------------------*/
/* la_vecClear:  clear a vector.                                            */ 
/*--------------------------------------------------------------------------*/

void la_vecClear(vec1, n)
     la_vect vec1;
     int n;
{
  int i;

  for (i=0; i<n; i++)
    vec1[i] = 0;
	
}


/*--------------------------------------------------------------------------*/
/* la_vecEq:  check if two vectors are equal.                               */
/*--------------------------------------------------------------------------*/

int la_vecEq(vec1, vec2, len)
     la_vect vec1;
     la_vect vec2;
     int len;
{

  int i;
  int same = TRUE;

  for(i = 0; (i<len) && same; i++)
    same = (vec1[i] == vec2[i]);
  
  return(same);



}


/*--------------------------------------------------------------------------*/
/* la_freeV: free a vector.                                                 */
/*--------------------------------------------------------------------------*/

void  la_vecFree(vec)
     la_vect vec;
{

  free( vec );

}
