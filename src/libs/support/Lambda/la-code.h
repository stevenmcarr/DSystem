/****************************************************************************
 * Copyright (C) 1993, 1992, 1991 Cornell University -- The Typhoon Project
 *
 * la-code.h,v 1.1.1.1 1993/09/23 19:09:38 stodghil Exp
 *
 *  Code Restructuring Module
 *       This module provides routines to compute the new loop nest
 *    given the transformation matrix and the dependence matrix.
 *
 * Originally written by Wei Li.
 *
 * la-code.h,v
 * Revision 1.1.1.1  1993/09/23  19:09:38  stodghil
 * Wei's Lambda Toolkit.
 *
 *
 ****************************************************************************/

#ifndef la_code_h
#define la_code_h

/*-----------------------------------------------------------------*/
/* data structure and routines dealing with linear expressions.    */
/*-----------------------------------------------------------------*/

/* definition of a piece-wise linear expression          */
/* e.g. in (i+j+2+x+y)/3,                                */
/*         coef = (1 1) for i+j,                         */
/*         c0 = 2,                                       */
/*         blob_coef = (0, 1 ,...) if x+y = blob1        */
/*         denom = 3,                                    */

typedef struct _la_expr{
  la_vect coef;           /* array of coefficients from a linear expression */
  int c0;                 /* constant */
  la_vect blob_coef;      /* array of coefficients from a linear expression
			   of blob's */
  int denom;              /* denominator */
  struct _la_expr *next;
} *LA_EXPR_T, LA_EXPR_T1;

#define LA_EXPR_COEF(expr)       ((expr)->coef)
#define LA_EXPR_C0(expr)         ((expr)->c0)
#define LA_EXPR_BLOB_COEF(expr)  ((expr)->blob_coef) 
#define LA_EXPR_DENOM(expr)      ((expr)->denom)
#define LA_EXPR_NEXT(expr)       ((expr)->next)

/*-----------------------------------------------------------------*/
/* Create an expr struct with the dimension dim and                */
/*	  the total number of blobs blobs                          */
/*-----------------------------------------------------------------*/

LA_EXPR_T LA_PROTO(la_expr_new, (int dim, 
				 int blobs));
void LA_PROTO(la_expr_free,( LA_EXPR_T expr ));

/*-----------------------------------------------------------------*/
/* data structure and routines dealing with loops.                 */
/*-----------------------------------------------------------------*/
  
/* definition of a loop                                        */
/* e.g. DO i = max( ceil((i+j)/2), ...), min(...), step 2      */

typedef struct _la_loop{
  LA_EXPR_T low;            /* lower bound: a set of expressions */
  LA_EXPR_T up;             /* upper bound: a set of expressions */
  int step;                 /* step size                         */
  LA_EXPR_T offset;         /* offset                            */
} *LA_LOOP_T, LA_LOOP_T1;

#define LA_LOOP_LOW(loop)    ((loop)->low)
#define LA_LOOP_UP(loop)     ((loop)->up)
#define LA_LOOP_STEP(loop)   ((loop)->step)
#define LA_LOOP_OFFSET(loop) ((loop)->offset)

/*-----------------------------------------------------------------*/
/* Create a loop struct.                                           */
/*-----------------------------------------------------------------*/

LA_LOOP_T LA_PROTO(la_loop_new,( void ));
void LA_PROTO(la_loop_free,( LA_LOOP_T loop ));

/*-----------------------------------------------------------------*/
/* data structure and routines dealing with loop nests.            */
/*-----------------------------------------------------------------*/

/* definition of a loop nest */
typedef struct _la_loopnest{
  LA_LOOP_T *loops;            /* an array of loops                  */
  int depth;                   /* the depth of the loop nest         */
  int blobs;                   /* number of blob's in the loop nest  */
} *LA_LOOPNEST_T, LA_LOOPNEST_T1;

#define LA_NEST_LOOPS(nest) ((nest)->loops) 
#define LA_NEST_DEPTH(nest) ((nest)->depth) 
#define LA_NEST_BLOBS(nest) ((nest)->blobs)

/*-----------------------------------------------------------------*/
/* Create a loop nest struct.                                      */
/*-----------------------------------------------------------------*/

LA_LOOPNEST_T LA_PROTO(la_nest_new,(int depth,
				    int blobs ));
void LA_PROTO(la_nest_free,( LA_LOOPNEST_T nest ));

/*-----------------------------------------------------------------*/
/* Compute the new loop nest.                                      */
/*-----------------------------------------------------------------*/

LA_LOOPNEST_T LA_PROTO(la_nest,(LA_LOOPNEST_T nest,
				LA_MATRIX_T T));

/*-----------------------------------------------------------------*/
/* routines dealing with expressions in the loop body.             */
/*-----------------------------------------------------------------*/

/* definition of a simple vector */
typedef struct {
  la_vect coef;               /* coef of the vector          */
  int size;                   /* the size                    */
  int denom;                  /* for rational vectors        */
} *LA_VECTOR_T, LA_VECTOR_T1;

#define LA_VECTOR_COEF(v)      ((v)->coef)
#define LA_VECTOR_SIZE(v)      ((v)->size)
#define LA_VECTOR_DENOM(v)     ((v)->denom)

/*-----------------------------------------------------------------*/
/* Create a vector struct of length size.                          */
/*-----------------------------------------------------------------*/

LA_VECTOR_T LA_PROTO(la_vector_new,( int size ));
void LA_PROTO(la_vector_free,( LA_VECTOR_T v ));

/*--------------------------------------------------------------------------*/
/* la_vector: computes the new expression in the transformed loop nest.     */
/*--------------------------------------------------------------------------*/

LA_VECTOR_T LA_PROTO(la_vector,(LA_MATRIX_T invT,
				LA_VECTOR_T v));
	       

/*--------------------------------------------------------------------------*/
/* la_dep_trans: new dep after transformation.                              */
/*--------------------------------------------------------------------------*/

LA_DEP_V_T LA_PROTO(la_dep_trans,(LA_DEP_V_T d, 
				  LA_MATRIX_T T));


/*-----------------------------------------------------------------*/
/* end of la-code.h                                                */
/*-----------------------------------------------------------------*/
#endif
