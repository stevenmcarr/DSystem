/* $Id: la-print2.c,v 1.2 1997/03/27 20:47:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/****************************************************************************
 * Copyright (C) 1993, 1992, 1991 Cornell University -- The Typhoon Project
 *
 * la-print2.c,v 1.1.1.1 1993/09/23 19:09:36 stodghil Exp
 *
 * Print routines.
 *
 * Originally written by Wei Li.
 *
 * la-print2.c,v
 * Revision 1.1.1.1  1993/09/23  19:09:36  stodghil
 * Wei's Lambda Toolkit.
 *
 *
 ****************************************************************************/

#include <libs/support/Lambda/la-private.h>

/*--------------------------------------------------------------------------*/
/* la_printM: prints a matrix.                                              */
/*--------------------------------------------------------------------------*/

void la_printM(mat, m, n)
     la_matrix mat;
     int m;
     int n;
{

  int i, j;
  la_vect row;

  if (!mat) 
    {
      fprintf(stdout,"empty matrix \n");
      return;
    }
  
  for (i=0; i<m; i++)
    {
      row = mat[i];
      for (j=0; j<n; j++)
	fprintf(stdout,"%3d ", row[j]);
      fprintf(stdout,"\n");
    }

  fprintf(stdout,"\n");
}

/*--------------------------------------------------------------------------*/
/* la_printV: prints a vector.                                              */
/*--------------------------------------------------------------------------*/

void la_printV(vec, n)
     la_vect vec;
     int n;
{

  int j;
  
  for (j=0; j<n; j++)
    fprintf(stdout,"%3d ", vec[j]);
  fprintf(stdout,"\n");

  fprintf(stdout,"\n");

}

/*--------------------------------------------------------------------------*/
/* la_printE: prints an expression.                                         */
/*--------------------------------------------------------------------------*/

/* return 1 for empty */

int la_printE(e, n, c, blob_list, blobs, d, start, cf)
     la_vect e;
     int n;
     int c;
     la_vect blob_list;
     int blobs;
     int d;
     int start;
     int cf;
{
  int empty, empty1;

  empty = 1;

  if(!e) return(empty);


  empty =  la_printLinearE(e, n, start);
  
  if (c != 0)
    {
      if (!empty)
	printf(" + ");
      else
	empty = 0;
      printf("%d", c);
    }

  empty1 =  la_printLinearE(blob_list, blobs, 'M');
  
  empty = (empty && empty1);
  
  if ( !empty )
    {
      if ( (d != 1) )
	printf(")%c%d", cf, d);
    }
      
  return(empty);
}

/*--------------------------------------------------------------------------*/
/* la_printLinearE: prints a linear expression.                             */
/*--------------------------------------------------------------------------*/

/* return 1 for empty */

int la_printLinearE(e, n, start)
     la_vect e;
     int n;
     int start;
{
  int j, first;

  first = 1;

  if(!e) return(first);

  for (j=0; j<n; j++)
    {

      if (e[j] != 0)
	{
	  if (first)
	    {

	      if (e[j] < 0)
		printf("-");

	      first = 0;
	    }
	  else if (e[j] > 0)
	    printf(" + ");
	  else
	    printf(" - ");
	  
	  if ( (e[j] == 1) || (e[j] == -1) )
	    printf("%c", start+j);
	  else 
	    printf("%d%c", la_abs(e[j]), start+j);
	  
	}
      
    }

      
  return(first);
}


/*--------------------------------------------------------------------------*/
/* la_printDirV: prints a direction vect.                                   */
/*--------------------------------------------------------------------------*/

void la_printDirV( vec, m )
     LA_DIR_T *vec;
     int m;
{

  int i;

  if (!vec)
    {
      fprintf(stdout,"empty dir vect \n");
      return;
    }


  for (i=0; i<m; i ++)
    switch(vec[i])
      {
      case dK:
        fprintf(stdout," K  ");
        break;
      case dLT:
        fprintf(stdout," <  ");
        break;
      case dLEQ:
        fprintf(stdout," <= ");
        break;
      case dSTAR:
        fprintf(stdout," *  ");
        break;
      case dEQ:
        fprintf(stdout," =  ");
        break;
      case dGEQ:
        fprintf(stdout," >= ");
        break;
      case dGT:
        fprintf(stdout," >  ");
        break;
      case dDOT:
        fprintf(stdout," .  ");
        break;
      default:
        fprintf(stdout,"unknown dir %d (in la_printDirV)", vec[i]);
        break;

      }
  fprintf(stdout,"\n");

  fprintf(stdout,"\n");

}



/*--------------------------------------------------------------------------*/

/* end of la_print2.c   */
