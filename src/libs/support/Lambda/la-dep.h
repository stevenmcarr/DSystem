/****************************************************************************
 * Copyright (C) 1993, 1992, 1991 Cornell University -- The Typhoon Project
 *
 * la-dep.h,v 1.1.1.1 1993/09/23 19:09:38 stodghil Exp
 *
 *  Dependence Module
 *       This module provides routines to handle dependences.
 *
 * Originally written by Wei Li.
 *
 * la-dep.h,v
 * Revision 1.1.1.1  1993/09/23  19:09:38  stodghil
 * Wei's Lambda Toolkit.
 *
 *
 ****************************************************************************/

#ifndef la_dep_h
#define la_dep_h

/* data dependences */
typedef enum {
  dK, 
  dLT,
  dLEQ,
  dEQ,
  dGEQ,
  dGT,
  dLG,   /* <> */
  dDOT,
  dSTAR,
  dSIZE
  } LA_DIR_T;

/* distance and direction */
typedef struct {
  LA_DIR_T dir;
  int dist;
} LA_DEP_T;

#define LA_DEP_DIR(dep)     ((dep).dir)
#define LA_DEP_DIST(dep)    ((dep).dist)
  
/* dependence vector */
typedef struct _vector {
  LA_DEP_T * vector;
  struct _vector *next;
} *LA_DEP_V_T, LA_DEP_V_T1; 

#define LA_DEP_V_VECTOR(v)  ((v)->vector)
#define LA_DEP_V_NEXT(v)    ((v)->next)


/* dependence matrix */
typedef struct{
  LA_DEP_V_T vectors;
  int dim;                   /* length of vectors            */
  int size;
} *LA_DEP_M_T, LA_DEP_M_T1; 

#define LA_DEP_M_VECTORS(D)   ((D)->vectors)
#define LA_DEP_M_DIM(D)       ((D)->dim)
#define LA_DEP_M_SIZE(D)      ((D)->size)

/*-----------------------------------------------------------------*/
/* Create a dependence vector of size size.                        */
/*-----------------------------------------------------------------*/

LA_DEP_V_T LA_PROTO(la_dep_vec_new,( int size ));

/*-----------------------------------------------------------------*/
/* Free a dev vector.                                              */
/*-----------------------------------------------------------------*/

void LA_PROTO(la_dep_vec_free,( LA_DEP_V_T d ));

/*-----------------------------------------------------------------*/
/* Create a dep matrix.                                            */
/*-----------------------------------------------------------------*/

LA_DEP_M_T LA_PROTO(la_dep_matrix_new,(int dim,
			     int size));


/*-----------------------------------------------------------------*/
/* Free a dep matrix.                                              */
/*-----------------------------------------------------------------*/

void LA_PROTO(la_dep_matrix_free,( LA_DEP_M_T D)); 

/*-----------------------------------------------------------------*/
/* Create a legal dep matrix.                                      */
/*   e.g. (*, 1) becomes (<, 1) and (=, 1). The illegal component  */
/*   (>, 1) is deleted.                                            */ 
/*-----------------------------------------------------------------*/
			
LA_DEP_M_T LA_PROTO(la_dep_legal,(LA_DEP_M_T D));

/*-----------------------------------------------------------------*/
/* la_dep_no_redundant: eliminates the redundant dep's.            */
/*-----------------------------------------------------------------*/

LA_DEP_M_T LA_PROTO(la_dep_no_redundant,(LA_DEP_M_T D));

/*-----------------------------------------------------------------*/
/* end of la-dep.h                                                 */
/*-----------------------------------------------------------------*/
#endif
