/* $Id: dt_analyze.C,v 1.1 1997/06/25 15:08:54 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*------------------------------------------------------------------------

    analyze.c  -  Analyze processed subscripts & apply dependence tests

*/

#include <string.h>

#include <libs/moduleAnalysis/dependence/dependenceTest/dt_analyze.i>

#include <include/bstring.h>

#include <libs/moduleAnalysis/dependence/dependenceTest/dt_info.h>
#include <libs/moduleAnalysis/dependence/utilities/side_info.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dep_dg.h>
#include <libs/moduleAnalysis/dependence/dependenceTest/dep_dt.h>

#include <libs/moduleAnalysis/cfg/cfg.h>
#include <libs/moduleAnalysis/cfgValNum/cfgval.h>
					/* access to symbolic analysis for */
					/* dependence testing 		   */

/* #define	USE_SIMPLIFIER */
#include <libs/moduleAnalysis/expnSimplifier/simple.h>
#ifdef	USE_SIMPLIFIER
  #include <FortEditor.h>		/* in ned_cp/FortEditor.h	*/
  EXTERN(FortEditor, ped_cp_get_FortEditor, (PedInfo ped));
#endif

/*--------------*/
/* declarations */

/*---------*/
/* defines */

/* for solve()	*/

#define SOLVE_DEP    0			/* offset for solve 	*/
#define SOLVE_INT    1
#define SOLVE_PAR    2

#define OP_FLOOR	1
#define OP_CEILING	2

struct dt_BanjMemo;
typedef struct dt_BanjMemo dt_BanjMemo;

typedef struct {      /* structure for memoizing the coefficients */
  int	diff;		/* Difference between a0 and b0 constants */
  int	loDelta[MAXLOOP];		
  int	upDelta[MAXLOOP];		
  int	ltype[MAXLOOP];		/* direction vector scratch pad.*/
  int 	al[MAXLOOP][MAXLOOP];    
  int 	ar[MAXLOOP][MAXLOOP];
  int 	bl[MAXLOOP][MAXLOOP];
  int 	br[MAXLOOP][MAXLOOP];
  int   dir_vec[MAXLOOP];
} dt_TrapBanjMemo;

/*---------------*/
/* forward decls */

STATIC(Boolean,   dt_delta,(int *subs_flag, int max_dim, Loop_list *loops1, Loop_list
                            *loops2, Subs_list *subs1, Subs_list *subs2, Dvect_data
                            *dvect, int clevel, int max_level, int *untested, 
                            Boolean *sym_flag));
STATIC(Boolean,   dt_ziv,(int *subs_flag, int max_dim, Loop_list *loops1, Loop_list
                          *loops2, Subs_data *subs1, Subs_data *subs2, int *untested));
STATIC(Boolean,   dt_ziv1,(Subs_data *subs1, Subs_data *subs2));
STATIC(Boolean,   dt_siv,(Loop_list *loops1, Loop_list *loops2, Subs_data *subs1,
                          Subs_data *subs2, Dvect_data *dvect, int same_loop, int ilvl));
STATIC(Boolean,   dt_gcd,(int *subs_flag, int clevel, int max_dim, Loop_list *loops1,
                          Loop_list *loops2, Subs_data *subs1, Subs_data *subs2));
STATIC(Boolean,   dt_gcd1,(Loop_list *loops1, Loop_list *loops2, Subs_data *subs1,
                           Subs_data *subs2, int clevel));
STATIC(Boolean,   dt_banj,(int *subs_flag, int max_dim, Loop_list *loops1, Loop_list
                           *loops2, Subs_data *subs1, Subs_data *subs2, Dvect_data *dvect,
                           int same_loop, int max_level));
STATIC(Boolean,   dt_banj1,(Loop_list *loops1, Loop_list *loops2, Subs_data *subs1,
                            Subs_data *subs2, Dvect_data *dvect, int same_loop, 
                            int max_level));
STATIC(Boolean,   dt_banj_rect,(Loop_list *loops1, Loop_list *loops2, Subs_data *subs1,
                                Subs_data *subs2, Dvect_data *dvect, int same_loop,
                                int max_level));
STATIC(Boolean,   dt_banj_recurs, (int loSum, int upSum, int level, dt_BanjMemo *memo,
                                   int last_level, int common_level));
STATIC(void,      dt_banj_any, (int loBound1, int upBound1, int loBound2, int upBound2,
                                int a_coeff, int b_coeff, int *loDelta, int *upDelta));
STATIC(void,      dt_banj_eq, (int loBound, int hiBound, int step, int a_coeff, 
                               int b_coeff, int *loDelta, int *upDelta));
STATIC(void,      dt_banj_gt, (int loBound, int hiBound, int step, int a_coeff, 
                               int b_coeff, int *loDelta, int *upDelta));
STATIC(void,      dt_banj_lt, (int loBound, int hiBound, int step, int a_coeff,
                               int b_coeff, int *loDelta, int *upDelta));
STATIC(Boolean,   dt_trap_banj,(int *subs_flag, int max_dim, CfgInstance cfgInst,
                                Loop_list *loops1, Loop_list *loops2, Subs_data *subs1,
                                Subs_data *subs2, Dvect_data *dvect, int common_level,
                                int max_level));
STATIC(Boolean,   dt_trap_banj1, (Loop_list *loops1, Loop_list *loops2, Subs_data *subs1,
                                  Subs_data *subs2, Dvect_data *dvect, int common_level,
                                  int max_level));
STATIC(Boolean,   dt_trap_banj_recurs, (int level, int common_level, int last_level,
                                        dt_TrapBanjMemo *memo, int *al, int *ar, int *bl,
                                        int *br, Loop_list *loops));
STATIC(Boolean,   dt_trap_banj_in_bound, (int level, int last_level, dt_TrapBanjMemo 
                                          *memo, int *al, int *ar, int *bl, int *br,
                                          Loop_list *loops));
STATIC(void,      dt_trap_banj_any, (Loop_list *loops1, Loop_list *loops2, int *al,
                                     int *ar, int *bl, int *br, int *loDelta, 
                                     int *upDelta, int level, int max_level));
STATIC(void,      dt_trap_banj_eq, (Loop_list *loops, int *dir, int *al, int *ar, 
                                    int *bl, int *br, int *loDelta, int *upDelta, 
                                    int level));
STATIC(void,      dt_trap_banj_gt, (Loop_list *loops, int *al, int *ar, int *bl, 
                                    int *br, int *loDelta, int *upDelta, int level));
STATIC(void,      dt_trap_banj_lt, (Loop_list *loops, int *al, int *ar, int *bl, 
                                    int *br, int *loDelta, int *upDelta, int level));
STATIC(Boolean,   dt_check_bounds,());
STATIC(int,       dt_exact,(Loop_list *loops1, Loop_list *loops2, Subs_data *subs1,
                            Subs_data *subs2, int ilvl));
STATIC(Boolean,   dt_rdiv,(int *subs_flag, int max_dim, Loop_list *loops1, Loop_list 
                           *loops2, Subs_data *subs1, Subs_data *subs2, 
                           Dvect_data *dvect, int clevel));
STATIC(Boolean,   dt_rdiv1,(Loop_list *loops1, Loop_list *loops2, Subs_data *subs1,
                            Subs_data *subs2, int *ltype, int clevel));

STATIC(int,       dt_propd,(Subs_data *subs1, Subs_data *subs2, Dvect_data *dvect,
                            int clevel, int max_level, int orig_iv));
STATIC(void,      dt_convert,(DT_info *dt, DG_Edge *Edge, Dvect_data *dvect, int
                              max_level, int clevel, Boolean all_eq, Boolean proven));
STATIC(void,      dt_part,(int *subs_flag, int max_dim, Subs_list *subs1, 
                           Subs_list *subs2));
STATIC(void,      dt_reverse,(Loop_list *loops, Dvect_data *dvect, int clevel));
STATIC(Boolean,   dt_lprove,(Loop_list *loops));
STATIC(Boolean,   in_bound,());
STATIC(Boolean,	  *solve, (int a1, int b1, int c1, int a2, int b2, int c2, int *x, 
                           int *y));
STATIC(char,	  *pstr,(int a, int b, char c));
STATIC(int,       divide_plus,(int x, int y, int op));
STATIC(int,       dt_dv_meet1, (int dv1, int dv2));

STATIC(void,	 dt_consistent_test,(DG_Edge *Edge, int *subs_flag, int max_dim,
                                     Subs_list *subs1, Subs_list *subs2, Loop_list 
                                     *loops1, Loop_list *loops2, int min_level, 
                                     int clevel));
void	 	 dt_consistent_str();
STATIC(void,	 init_nesting_level_array,());
SymDescriptor 	 dt_get_symDescriptor();

STATIC(Boolean,  dt_check_bounds, (int *subs_flag, int max_dim, CfgInstance cfgInst,
                                   Boolean *sym_flag, Loop_list *loops1, Loop_list
                                   *loops2, Subs_data *subs1, Subs_data *subs2,
                                   Dvect_data *dvect));
STATIC(Boolean,  dt_check_bounds1, (CfgInstance cfgInstance, Loop_list *loops1,
                                    Loop_list *loops2, Subs_data *subs1, Subs_data
                                    *subs2, Dvect_data *dvect));
STATIC(Boolean,  dt_check_sym_bounds, (Values values, int const_diff, int a_coeff,
                                       int b_coeff, ValNumber sym_diff, ValNumber
                                       loBound1, ValNumber upBound1, ValNumber loBound2,
                                       ValNumber upBound2));

/*-------------------*/
/* global functions  */

/*-------------------------------------------------------------------

	dt_analyze()	check two references to single array

	Parameter:	description of loops (for both references)
			first array reference (list of subscripts)
			second array reference (list of subscripts)

	Returns:	Pointer to Dt_info if dependence found
			DT_NONE pointer otherwise

	Analyze dependences between pair of array references.

*/
void
dt_analyze(DT_info *dt, DG_Edge *Edge, CfgInstance cfgInst, Subs_list *subs1, 
           Subs_list *subs2, Loop_list *loops1, Loop_list *loops2, int clevel, 
           Boolean indep)
     /* Subs_list   *subs1;	src	*/
     /* Subs_list   *subs2;	sink	*/
     /* Loop_list   *loops1;	src	*/
     /* Loop_list   *loops2;	sink	*/
     /* int 	    clevel;	common loop level	*/
     /* Boolean     indep;	loop independent dep possible */
{
  int        i,dim;
  int        max_dim;		/* max dimension of array */
  int        max_level;		/* max loop nest level	 */
  int        min_level;		/* min loop nest level	 */
  Boolean    proven;		/* can dependence be proven? */
  Boolean    all_eq;		/* condition flags	 */ 
  Dvect_data dvect;		/* constraints on index var */
  int        subs_flag[MAXDIM];	/* flag for SIV subs	 */
  int        untested;		/* # of untested subs	 */
  Subs_list  subs1_buf;		/* src	*/
  Subs_list  subs2_buf;		/* sink	*/
  Boolean    sym_flag[MAXDIM]; 	/* flags for symbolic bounds */

  /*------------------------------*/
  /* perform some initializations */

  bzero((char *)subs_flag, sizeof(subs_flag));
  bzero((char *)&dvect, sizeof(Dvect_data));
  bzero((char *)sym_flag, sizeof(sym_flag));
  max_level = MAX(loops1->level, loops2->level);
  min_level = MIN(loops1->level, loops2->level);
  proven = true;
  Edge->dt_type = DT_NONE;

  /*------------------------------------------------*/
  /* check whether arrays have same # of dimensions */
  /* if not, just look at matching dims		    */
  /* set max_dim to max # of matching dims	    */

  if (subs1->dims == subs2->dims)
    {
      max_dim = subs1->dims;
      untested = max_dim;
    }
  else
    {
      max_dim = MIN(subs1->dims, subs2->dims);
      proven = false;

      /* should reshape arrays, but quit here for simple version */

      Edge->dt_type = DT_NONLIN;
      return;
    }

  if (cfgInst != NULL)
    {
      /* save the copy of subscripts before modifying them      */
      /* if that is done here, then delta doesn't have to do it */
      memcpy(&subs1_buf, subs1, sizeof(Subs_list));
      memcpy(&subs2_buf, subs2, sizeof(Subs_list));
      subs1 = &subs1_buf;
      subs2 = &subs2_buf;

      for (dim = 0; dim < subs1->dims; dim++)
	/* we've already assumed subs1->dim == subs2->dim */
	{
	  if (subs1->subs[dim].stype > SUBS_SYM && 
	      subs2->subs[dim].stype > SUBS_SYM &&
	      subs1->subs[dim].symbolic_constant == 
	      subs2->subs[dim].symbolic_constant)
	    {
	      subs1->subs[dim].stype -= SUBS_SYM_SIV_FIRST;
	      subs2->subs[dim].stype -= SUBS_SYM_SIV_FIRST;
	    }
	}
    }

  /*----------------------------------------------------*/
  /* initial pass to classify/partition subscript pairs */

  dt_part(subs_flag, max_dim, subs1, subs2);

  /*---------------------------------------------*/
  /* initial classification for consistent field */

  dt_consistent_test(Edge, subs_flag, max_dim, (Subs_list*)subs1->subs, (Subs_list*)subs2->subs,
		     loops1, loops2, min_level, clevel);

  /*------------------------------------*/
  /* perform ZIV test on ZIV subscripts */

  if (!dt_ziv(subs_flag, max_dim, loops1, loops2, 
	      subs1->subs, subs2->subs, &untested))
    return;

  /*------------------------------------------*/
  /* perform Delta test on SIV/MIV subscripts */

  /* make copies of subs, since delta test may simplify them           */
  /* if that is already done for symbolic constants, don't do it again */
  if (cfgInst == NULL)
    {
      memcpy(&subs1_buf, subs1, sizeof(Subs_list));
      memcpy(&subs2_buf, subs2, sizeof(Subs_list));
      subs1 = &subs1_buf;
      subs2 = &subs2_buf;
    }

  if (untested && !dt_delta(subs_flag, max_dim, loops1, loops2, 
			    subs1, subs2, &dvect, clevel, 
			    max_level, &untested, sym_flag))
    return;

  /*----------------------------------------------------------*/
  /* perform single dimensional RDIV test on RDIV subscripts  */

  if (untested && !dt_rdiv(subs_flag, max_dim, loops1, loops2, 
			   subs1->subs, subs2->subs, &dvect, clevel))
    return;

  /*----------------------------------------------------------*/
  /* perform single dimensional gcd test on MIV subscripts    */

  if (untested && !dt_gcd(subs_flag, clevel, max_dim, loops1, loops2, 
			  subs1->subs, subs2->subs))
    return;

  /*------------------------------------------------------------------*/
  /* perform single dimensional banerjee test on MIV, RDIV subscripts */

  if (untested && !dt_banj(subs_flag, max_dim, 
			   loops1, loops2, subs1->subs, subs2->subs, 
			   &dvect, clevel, max_level))
    return;

  /*-----------------------------------------------------------*/
  /* perform trapezoidal banerjee test on MIV, RDIV subscripts */

  if (untested && cfgInst &&
      !dt_trap_banj(subs_flag, max_dim, cfgInst, loops1, loops2, 
		    subs1->subs, subs2->subs, &dvect, clevel, max_level))
    return;

  /*-----------------------------------------------------------------*/
  /* perform simple bound check for symbolic ZIV/SIV/RDIV subscripts */

  if (cfgInst && 
      !dt_check_bounds(subs_flag, max_dim, cfgInst, sym_flag, 
		       loops1, loops2, subs1->subs, subs2->subs, &dvect))
    return;

  /*------------------------------------------------------*/
  /* check whether loop params make exact test impossible	*/
  /* no need to test if exact test already impossible		*/

  if (proven)
    proven = BOOL(dt_lprove(loops1) && dt_lprove(loops2));

  /*------------------------------------------------------*/
  /* check on accuracy (temp) 							*/

  for (i = 0; i < max_dim; i++)
    {
      /* MIV/symbolic subs make dep tests uncertain	*/

      if ((subs_flag[i] == SUBS_MIV) || (subs_flag[i] == SUBS_SYM))
	proven = false;	
    }
  /*------------------------------------------------------*/
  /* convert all distance/directions on 					*/
  /* non-common loops to loop indep dependences			*/

  for (i = clevel; i < max_level; i++)
    dvect.ltype[i] = DV_EQ;

  /*------------------------------------------------------*/
  /* reverse dep info for reversed loops					*/

  dt_reverse(loops1, &dvect, clevel);

  /*------------------------------------------------------*/
  /* check whether loop independent dependence exists		*/
  /* true only if all loops are DV_EQ	 					*/

  all_eq = true;		/* loop indep	 */
  for (i = 0; i < max_level; i++)
    {
      if (dvect.ltype[i] != DV_EQ)
	{ 
	  all_eq = false;
	}
    }

  /* if no loop independent dep is possible, then independent	*/

  if (all_eq && !indep)
    return;

  /*------------------------------------------------------*/
  /* create new copy for outside world					*/

  dt_convert( dt, Edge, &dvect, max_level, clevel, all_eq, proven);
}


/*----------------------------------------------------------------

	gcd1()	-	calculate greatest common divisor of two ints

	Returns: gcd

	This version of the Euclidean Algorithm for computing GCD
	is taken from the extended version found in The Design and
	Analysis of Computer Algorithms by Aho, Hopcroft and Ullman.

	Input: A,B -- variables whose GCD is found
	Output: X,Y if A,B ^= 0, these contain solution to
	       AX + BY = D where D is the GCD of A and B.

	PFC version:

		q = a0/a1;		r = a0 - q*a1;

		while (r)
			{
			a2 = a0 - q*a1;		a0 = a1;	a1 = a2;
			x2 = x0 - q*x1;		x0 = x1;	x1 = x2;
			y2 = y0 - q*y1;		y0 = y1;	y1 = y2;

			q = a0/a1;			r = a0 - q*a1;
			}
*/

int
gcd1(int a, int b, int *x, int *y)
{
	int             q, a0, a1, a2, x0, x1, x2, y0, y1, y2;

	if (!a)
	{
		q = ABS(b);
		*x = 0;
		*y = (q == 1) ? -b : -b/q;
		return (q);
	}
	if (!b)
	{
		q = ABS(a);
		*x = (q == 1) ? a : a/q;
		*y = 0;
		return (q);
	}

	a0 = ABS(a);
	a1 = ABS(b);
	x0 = 1;
	y0 = 0;
	x1 = 0;
	y1 = 1;

	while (a0 % a1)
	{
		q = a0 / a1;
		a2 = a0 - q * a1;
		a0 = a1;
		a1 = a2;
		x2 = x0 - q * x1;
		x0 = x1;
		x1 = x2;
		y2 = y0 - q * y1;
		y0 = y1;
		y1 = y2;
	}

	*x = a < 0 ? -x1 : x1;
	*y = b > 0 ? -y1 : y1;

	return (a1);
}

/*----------------------------------------------------------------

	gcd2()	-	calculate greatest common divisor of multiple ints

	Returns: gcd of all numbers

	Based on gcd algorithm in pp. 68-71 of:

		Utpal Banerjee  
		Dependence Analysis for Supercomputing
		1988
		Kluwer Academic Publishers

*/

int
gcd2(int num, int *ptr)
{
	int u;
	int v;
	int r;

	u = *ptr++;				/* start with one stick	*/
	u = ABS(u);

	while (--num > 0)			/* while gum left in pack */
	{
		v = *ptr++;			/* bite one stick off front */
		v = ABS(v);

		while (v)			/* while flavor lasts	*/
		{
			r = u % v;		/* chew on it a bit	*/
			u = v;
			v = r;
		}

		if (u == 1)			/* if no gum left	*/
			return 1;		/* stop chewing		*/
	}

	return u;				/* spit out remainder	*/
}

/*-----------------*/
/* local functions */

/*----------------------------------------------------------------

	dt_part()	-	examine & partition subscripts

	For dimension i, subs_flag[i] =  0-20        SIV sub at level
                                     SUBS_MIV    MIV subs
                                     SUBS_ZIV    ZIV subs
                                     SUBS_SYM    SYM subs

	Just classifies subs for now, partition in coupled subs later

*/

static void
dt_part(int *subs_flag, int max_dim, Subs_list *subs1, Subs_list *subs2)
{
	int	i;
	int dim;			/* dimension of array looking at  */
	Boolean not_done;		/* continue looking at dimension? */

	/* loop through all subscript dimensions, classify in subs_flag[] */

	for (dim = 0; dim < max_dim ; dim++)
	{
		/*-------------------------------------------------------*/
		/* give up if symbolics found in either subscript	 */

		if ((subs1->subs[dim].stype >= SUBS_SYM) ||
			(subs2->subs[dim].stype >= SUBS_SYM))
		{
			subs_flag[dim] = SUBS_SYM;
		}

		/*-------------------------------------------------------*/
		/* may be ZIV subscript pair if either subs is ZIV	 */

		else if ((subs1->subs[dim].stype == SUBS_ZIV) ||
				(subs2->subs[dim].stype == SUBS_ZIV))
		{
			/* find ZIV sub, pair type = type of other sub */

			if (subs1->subs[dim].stype == SUBS_ZIV)
				subs_flag[dim] = subs2->subs[dim].stype;
			else
				subs_flag[dim] = subs1->subs[dim].stype;
		}

		/*-------------------------------------------------------*/
		/* MIV subscript pair if either subs is already MIV	 */

		else if ((subs1->subs[dim].stype == SUBS_MIV) ||
				(subs2->subs[dim].stype == SUBS_MIV))
		{
			subs_flag[dim] = SUBS_MIV;
		}

		/*-------------------------------------------------------*/
		/* Both subscripts are SIV, check whether same IV	 */

		else if ((subs1->subs[dim].stype <= SUBS_SIV) ||
				(subs2->subs[dim].stype <= SUBS_SIV))
		{
			if (subs1->subs[dim].stype == subs2->subs[dim].stype)
				subs_flag[dim] = subs1->subs[dim].stype;
			else
				subs_flag[dim] = SUBS_RDIV;
		}

		/*-------------------------------------------------------*/
		/* No idea how we got here, label as SYM for safety	 */

		else
		{
			subs_flag[dim] = SUBS_SYM;
		}
	}
}



/*----------------------------------------------------------------

	dt_delta()	-	apply Delta test to subscript pairs
*/

static Boolean
dt_delta(int *subs_flag, int max_dim, Loop_list *loops1, Loop_list *loops2, 
         Subs_list *subs1, Subs_list *subs2, Dvect_data *dvect, int clevel, 
         int max_level, int *untested, Boolean *sym_flag)
{
	int i;
	Boolean new_constraint;	

	new_constraint = true;

	while (new_constraint)
	{
	  new_constraint = false;	/* no new constraints found */

	  /*--------------------------------------------------*/
	  /* Phase 1: perform SIV test on SIV subscript pairs */

	  for (i = 0; i < max_dim; i++)
	  {
	    /* merge dependence info into dvect		 */
	    /* set subs_flag wherever subscripts were SIV	 */

	    if (subs_flag[i] <= SUBS_SIV)
	    {
	      if (!dt_siv(loops1, loops2, subs1->subs + i, 
			  subs2->subs + i, dvect, clevel, subs_flag[i]))
		return false;				/* independent!	 */

	      if ((loops1->loops[subs_flag[i]].up.val == MAXINT &&
		   loops1->loops[subs_flag[i]].lo.val == MININT) ||
		  (loops2->loops[subs_flag[i]].up.val == MAXINT &&
		   loops2->loops[subs_flag[i]].lo.val == MININT))
		sym_flag[i] = true;
				
	      subs_flag[i] = SUBS_EMPTY;		/* subs consumed */
	      new_constraint = true;			/* found constraint */

	      if (!--*untested)				/* tested all subs */
		return true;
	    }
	  }

	  /*------------------------------------------------------------*/
	  /* Phase 2: propagate constraints into MIV subscript pairs 	*/

	  if (new_constraint)			/* skip if no constraints */
	  {
	    new_constraint = false;		/* no longer new */
	    
	    for (i = 0; i < max_dim; i++)
	    {
	      if ((subs_flag[i] == SUBS_MIV) || (subs_flag[i] == SUBS_RDIV))
	      {
		subs_flag[i] = dt_propd(subs1->subs + i, subs2->subs + i,
					dvect, clevel, max_level, subs_flag[i]);

		/*--------------------------------------------------*/
		/* if new ZIV subscript generated, test it here	    */

		if (subs_flag[i] == SUBS_ZIV)
		  {
		    if (!dt_ziv1(subs1->subs + i, subs2->subs + i))
		      return false;			/* independent! */

		    subs_flag[i] = SUBS_EMPTY;		/* subs consumed */

		    if (!--*untested)		/* tested all subs */
		      return true;
		  }

		/*--------------------------------------------------*/
		/* if new constraints are generated, repeat overall */

		else if (subs_flag[i] <= SUBS_SIV) 
		  new_constraint = true;
	      }
	    }
	  }
	}

	return true;		/* dependence found	*/
}

/*----------------------------------------------------------------

	dt_ziv()	-	Apply ZIV test on ZIV subscripts


	Returns:		false if no dep
				true  if possible dep
*/

static Boolean
dt_ziv(int *subs_flag, int max_dim, Loop_list *loops1, Loop_list *loops2, 
       Subs_data *subs1, Subs_data *subs2, int *untested)
{
	int i;

	for (i = 0; i < max_dim; i++)
	{
	  if (subs_flag[i] == SUBS_ZIV)	/* only need check ZIV subs */
	  {
	    if (!dt_ziv1(subs1 + i, subs2 + i))
	      return false;

	    if (!--*untested)	       	/* one more subscript pair tested */
	      return true;
	  }
	}

	return true;
}

/*----------------------------------------------------------------

	dt_ziv1()	-	Apply ZIV test on ZIV subscripts

	Add symbolic differencing here.

*/

static Boolean 
dt_ziv1(Subs_data *subs1, Subs_data *subs2)
{
	if (subs1->constant == subs2->constant)
		return true;

	else
		return false;
}


/*----------------------------------------------------------------

	dt_siv()	-	determines whether SIV test can be used
				apply it if possible

	Parameters:		loop information
				first subscript
				second subscript
				output data vector
				...

	Returns:		true   if dependence exists
				false  if no dependence

	Requirements for using SIV test:

	1) Each subscript position must have no more than one loop
 	   variable with a non-zero coefficient in the expression
	   for the subscript.

	2) Both references must have the same loop variables with
	   non-zero coefficients, in corresponding subscripts.

	3) Using nonsimple SIV, loop variables may have
	   different coefficients values (possibly zero).

	All subscript positions must show loop independent dependences for
	the two array references to have a loop independent dependence.

	Only one subscript must be independent for the
	two array references to be independent.	

	Since SIV tests are applied first, the only legit DV values 
	thus far are:  DV_LN, DV_EQ, DV_PT, DV_ANY

	----------------------------------------------------------------

	Use simple/nonsimple SIV test to find dependence, calculate
        direction/distance if dependence exists.

	Uses constraints passed in dvect for more precise test.
	Adds/modifies constraints as needed.

	Assumes no symbolic constants or coefficients for now.

	Loops have not been normalized.  Step > 0 since dt_step() 
	has been called, but loop bounds may be any value. 

*/

static Boolean
dt_siv(Loop_list *loops1, Loop_list *loops2, Subs_data *subs1, Subs_data *subs2,
       Dvect_data *dvect, int same_loop, int ilvl)
{
	int             i;
	Boolean         nonsimple;	/* flag for nonsimple sep	*/
	Boolean	       *sptr;		/* returned by solve()		*/

	int             coeff_gcd;	/* gcd of coefficients		*/
	int             coeff1;		/* coefficients of x		*/
	int             coeff2;		/* coefficients of y		*/
	int             x;
	int             y;
	int             diff;		/* const diff between subs	*/
	int             distance;	/* dependence distance		*/

	int             up_b;		/* upper loop bound		*/
	int             lo_b;		/* lower loop bound		*/
	int		step;		/* loop step size		*/

	/*--------------------------------------*/
	/* use exact test if not common loop	*/

	if (ilvl >= same_loop)
	{
		if (dt_exact(loops1, loops2, subs1, subs2, ilvl) == DV_NO)
			return false;

		dvect->ltype[ilvl] = DV_EQ;   /* must be EQ since not common */
		return true;
	}

	/*-----------------------*/
	/* some initializations	 */

	nonsimple = false;
	coeff1 = subs1->coeffs[ilvl];
	coeff2 = subs2->coeffs[ilvl];

	/*-----------------------------------------------*/
	/* if not same coefficient, then nonsimple SIV	 */

	if (coeff1 != coeff2)
	{
		nonsimple = true;
	}

	/*------------------------------------------------------*/
	/* if both coefficients and constant are same,		*/
	/* loop independent dependence				*/

	else if (subs1->constant == subs2->constant)
	{
		switch (dvect->ltype[ilvl])
		{
			case DV_ANY:	/* refine dep info	*/
				dvect->ltype[ilvl] = DV_EQ;	
				return true;

			case DV_EQ:	/* no new info		*/
				return true;

			case DV_DIS:	/* mismatched distances, indep!	*/
				return false;

			case DV_LN:  /* fall through for more detailed check */
			case DV_GT:
			case DV_GE:
			case DV_LT:
			case DV_LE:
			case DV_NE:
				break;

			case DV_PT:  /* point must be on line y=x, else indep */
				return (dvect->c1[ilvl] == dvect->c2[ilvl]) ? 
				  true : false;

			default:

				dvect->ltype[ilvl] = DV_EQ;
				return true;
		}
	}

	/*------------------------------------------------------*/
	/*------------------------------------------------------*/
	/* Apply SIV test */

	/*---------------------------------------------------------------*/
	/* First try to catch symbolic cases where the difference	 */
	/* of two subscripts exceeds the upper/lower bounds of the	 */
	/* loop.  If test succeeds, then dependence cannot exist.	 */

	/* implement symbolic test later...	*/

	/*-----------------------------------------------*/
	/* only constant cases left past this point	 */

	/*---------------------------------------------------------------*/
	/* Examine the difference between the two subscripts to see	 */
	/* if it could create a dependence.  				 */

	up_b = loops1->loops[ilvl].up.val;
	lo_b = loops1->loops[ilvl].lo.val;
	step = loops1->loops[ilvl].step.val;
	diff = subs2->constant - subs1->constant;

	/*----------------------*/
	/* if simple SIV 	*/

	if (!nonsimple)
	{
		coeff_gcd = coeff1 * step;

		if (coeff_gcd == 1)
		{
			distance = -diff;
		}
		else
		{
			distance = -diff / coeff_gcd;

			/*------------------------------------------------------*/
			/* if the coefficient does not divide the difference,  	*/
			/* then the gcd test would show independence.			*/

			if (diff && !DIVIDES(coeff_gcd, diff))
				return false;
		}

		/*------------------------------------------------------*/
		/* check what distance is, may be > loop bounds	*/

		if ((lo_b != MININT) && (up_b != MAXINT)) /* not symbolic bounds	*/
		{
			if (step == 1)
			{
				if (ABS(distance) > (up_b - lo_b))
					return false;
			}
			else
			{
				if (ABS(distance) > ((up_b - lo_b)/ step))	
					return false;
			}
		}

		/*------------------------------------------------------*/
		/* compare against current constraints for loop, 		 */
		/* if any, and update as required.						 */

		switch (dvect->ltype[ilvl])
		{
		case DV_EQ:/* loop independent dep		 */

			/* means distance of zero found elsewhere			*/
			/* if distance == 0, would have been caught before	*/
			/* thus distance can't be zero here, so indep!		*/

			return false;

		case DV_DIS:/* previous distance found	 */

			/* distance vector must be same for the SIV			*/
			/* subscripts on each loop, else no dep!			*/

			return (distance == dvect->c1[ilvl]) ? true : false;

		case DV_PT:	/* constraint = point			 */

			/* if point not on line x=y+b then independent!		 */
			/* else maintain point constraint					 */

			return (distance == (dvect->c2[ilvl] - dvect->c1[ilvl]))
							? true : false;

		case DV_LN:/* constraint = line			 */
		case DV_GT:
		case DV_GE:
		case DV_LT:
		case DV_LE:
		case DV_NE:

			/* check distance against constraints,		 */
			/* same as intersection with line x=y+b		 */

			/* no parallel or dependent lines since 	 */
			/* only one is SIV (slope = 1)		 */

			/* The distance vector passed to solve() 	 */
			/* must be set to -distance	since the 		 */
			/* equation is of the form x=y+diff.	 */

			sptr = solve(1, 1, -distance, dvect->c1[ilvl],
				 	 dvect->c2[ilvl], dvect->c3[ilvl], &x, &y);

			if (!sptr[SOLVE_INT])		/* no integer solution!	*/
			{
				return false;
			}
			else if (sptr[SOLVE_DEP] || sptr[SOLVE_PAR])
			{
				/* printf("dv_siv(): impossible solve() result\n"); */
				return true;
			}

			/* if point not in iteration space, then independent!	 */

			if (!IN_BOUND(x, lo_b, up_b) || !IN_BOUND(y, lo_b, up_b))
				return false;

			/* else make point the new constraint	 */

			dvect->ltype[ilvl] = DV_PT;
			dvect->c1[ilvl] = x;
			dvect->c2[ilvl] = y;
			return true;

		case DV_ANY:/* unknown					 */

			if (distance)
			{
				dvect->ltype[ilvl] = DV_DIS;	/* distance vec	 */
				dvect->c1[ilvl] = distance;
			}
			else
			{
				dvect->ltype[ilvl] = DV_EQ;	/* loop ind dep	 */
			}
			return true;

		default:
			/* printf("dv_siv(): DV_DIS, Illegal dvect type found\n"); */
			return true;
		}
	}

	/*--------------------------*/
	/* if nonsimple SIV			*/

	else			/* (nonsimple == true)	 */
	{
		/* can check via GCD test here, but pass on to dt_exact()	*/

		/* if dt_exact() not invoked, intersection with prev 		*/
		/* constraints will not yield integer solutions if 			*/
		/* none for this subscript, so GCD test is not needed		*/

		/*------------------------------------------------------*/
		/* compare against current constraints for loop, 		 */
		/* if any, and update as required.						 */

		switch (dvect->ltype[ilvl])
		{
		case DV_EQ:/* loop independent dep		 */

			/* means distance of zero found elsewhere		 */
			/* solve for intersection with line x=y			 */

			x = diff / (coeff1 - coeff2);

			/* if point (x,x) is not in bounds OR 				*/
			/* not on line (caused by noninteger point), indep! */

			if (!IN_BOUND(x, lo_b, up_b) || ((coeff1-coeff2) * x != diff))
				return false;

			/* else new constraint is DV_PT, since loop 	 */
			/* indep dep only exists on that iteration		 */

			/* else make point the new constraint	 */

			dvect->ltype[ilvl] = DV_PT;
			dvect->c1[ilvl] = x;
			dvect->c2[ilvl] = x;
			return true;

		case DV_PT:	/* constraint = point			 */

			/* if point not on current line then independent!	*/
			/* if it is on line then no new information found	*/

			return ((dvect->c1[ilvl] * coeff1) == 
					(dvect->c2[ilvl] * coeff2) + diff) ?
					true : false;

		case DV_LN:/* constraint = line			 */
		case DV_GT:
		case DV_GE:
		case DV_LT:
		case DV_LE:
		case DV_NE:

			/* solve for intersection of two lines			 */

			sptr = solve(coeff1, coeff2, diff, dvect->c1[ilvl],
					  dvect->c2[ilvl], dvect->c3[ilvl], &x, &y);

			if (!sptr[SOLVE_INT] || sptr[SOLVE_PAR])
			{
				/* no integer solution OR no intersection!	*/

				return false;
			}
			else if (sptr[SOLVE_DEP])
			{
				/* lines are dependent						 */
				/* no new information presented				 */

				return true;
			}


			/* if point not in iteration space, then independent!	 */

			if (!IN_BOUND(x, lo_b, up_b) || !IN_BOUND(y, lo_b, up_b))
				return false;

			/* else make point the new constraint	 */

			dvect->ltype[ilvl] = DV_PT;
			dvect->c1[ilvl] = x;
			dvect->c2[ilvl] = y;
			return true;

		case DV_DIS:/* previous distance found	 */

			/* must check distance against constraints	 */
			/* same as intersection with line x=y+b		 */

			/* The distance vector passed to solve() 	 */
			/* must be set to -distance	since the 		 */
			/* equation is of the form x=y+diff.	 */

			sptr = solve(coeff1, coeff2, diff,
						  1, 1, -dvect->c1[ilvl], &x, &y);

			if (!sptr[SOLVE_INT])		/* no integer solution!	*/
			{
				return false;
			}
			else if (sptr[SOLVE_DEP] || sptr[SOLVE_PAR])
			{
				/* printf("dv_siv(): impossible solve() result\n"); */
				return true;
			}

			/* if point not in iteration space, then independent!	 */

			if (!IN_BOUND(x, lo_b, up_b) || !IN_BOUND(y, lo_b, up_b))
				return false;

			/* else make point the new constraint	 */

			dvect->ltype[ilvl] = DV_PT;
			dvect->c1[ilvl] = x;
			dvect->c2[ilvl] = y;
			return true;

		case DV_ANY:/* unknown					 */

			/*------------------------------------------------------*/
			/* if dependence line is out of bounds then no dep		*/
			/* only need to check first time found!					*/

			i = dt_exact(loops1, loops2, subs1, subs2, ilvl);

			if (i == DV_NO)
				return false;

			/* assign new constraints		 */

			dvect->ltype[ilvl] = i;
			dvect->c1[ilvl] = coeff1;
			dvect->c2[ilvl] = coeff2;
			dvect->c3[ilvl] = diff;
			return true;

		default:
			/* printf("dv_siv(): DV_LN, Illegal dvect type found\n"); */
			return true;
		}
	}

	/* Shouldn't reach here	*/
}

/*----------------------------------------------------------------

	dt_exact()		Banerjee/Wolfe exact SIV test

	Returns:		Possible direction vectors

	{Fixed nonunit step 9/29/91 - tseng}

	Step is implicitly normalized within dt_exact() as follows

		do I = L, U, S
			A( a*I + ... )
		enddo

	becomes:

		do I = L, L+(U-L)/S, 1
			A( a*S*I - a*L*(S-1) + ... )
		enddo

	Note that step normalization is not possible if lower bound
	is unknown.  Conservatively pretend unit step in such cases.
		
*/

static int
dt_exact(Loop_list *loops1, Loop_list *loops2, Subs_data *subs1, 
         Subs_data *subs2, int ilvl)
{
	int up_b1, lo_b1, up_b2, lo_b2, x, y, diff, coeff1, coeff2, step;
	int coeff_gcd, up, lo, div, temp1, temp2, d_type, const1, const2;
	Boolean d_eq, d_lt, d_gt, no_X_bounds, no_Y_bounds;

	no_X_bounds = false;
	no_Y_bounds = false;

	/*----------------------------------------------------------*/
	/* dependence equation is:  coeff1 * X = coeff2 * Y + diff	*/

	/* first take care of some imperfectly nested cases	*/

	if (loops1->level <= ilvl)		/* no ivar, must be const	*/
	{
		if (subs1->coeffs[ilvl])	/* error!	*/
			return DV_NO;
		coeff1 = 0;
		const1 = subs1->constant;
		no_X_bounds = true;
	}
	else
	{
		lo_b1 = loops1->loops[ilvl].lo.val;
		step = loops1->loops[ilvl].step.val;

		if ((step == 1) || (lo_b1 == MININT))
		{
			coeff1 = subs1->coeffs[ilvl];
			up_b1 = loops1->loops[ilvl].up.val;
			const1 = subs1->constant;
		}
		else
		{
			coeff1 = subs1->coeffs[ilvl];
			up_b1 = lo_b1 + (loops1->loops[ilvl].up.val - lo_b1) / step;
			const1 = subs1->constant - (lo_b1 * coeff1 * (step - 1));
			coeff1 *= step;
		}
	}

	if (loops2->level <= ilvl)		/* no ivar, must be const	*/
	{
		if (subs2->coeffs[ilvl])	/* error!	*/
			return DV_NO;
		coeff2 = 0;
		const2 = subs2->constant;
		no_Y_bounds = true;
	}
	else
	{
		lo_b2 = loops2->loops[ilvl].lo.val;
		step = loops2->loops[ilvl].step.val;

		if ((step == 1) || (lo_b2 == MININT))
		{
			coeff2 = subs2->coeffs[ilvl];
			up_b2 = loops2->loops[ilvl].up.val;
			const2 = subs2->constant;
		}
		else
		{
			coeff2 = subs2->coeffs[ilvl];
			up_b2 = lo_b2 + (loops2->loops[ilvl].up.val - lo_b2) / step;
			const2 = subs2->constant - (lo_b2 * coeff2 * (step - 1));
			coeff2 *= step;
		}
	}

	diff = const2 - const1;

	/*-----------------------------------------------------------*/
	/* check if either coefficient is zero						 */
	/* if so, apply weak-zero SIV test instead of exact test	 */

	if (!coeff1)		/* 0x = by + diff	 */
	{
		if (coeff2 != 1)
		{
			if (-diff % coeff2)
				return DV_NO;
			y = -diff / coeff2;
		}
		else
		{
			y = -diff;
		}

		if (!IN_BOUND(y, lo_b2, up_b2))
			return DV_NO;
		else if (y == lo_b2)
			return DV_GE;
		else if (y == up_b2)
			return DV_LE;
		else
			return DV_LN;		/* all directions	*/
	}
	else if (!coeff2)	/* ax = 0y + diff	 */
	{
		if (coeff1 != 1)
		{
			if (diff % coeff1)
				return DV_NO;
			x = diff / coeff1;
		}
		else
		{
			x = diff;
		}

		if (!IN_BOUND(x, lo_b1, up_b1))
			return DV_NO;
		else if (x == lo_b1)
			return DV_LE;
		else if (x == up_b1)
			return DV_GE;
		else
			return DV_LN;		/* all directions	*/
	}

	/*-------------------------------------------------*/
	/* not weak-zero SIV, apply our general exact test */

	coeff_gcd = gcd1(coeff1, coeff2, &x, &y);

	/* coeff_gcd = gcd of coeff1 & coeff2	*/

	/* x, y are a solution to the equation:			*/
	/*    coeff1 * x - coeff2 * y = coeff_gcd		*/

	div = diff / coeff_gcd;

	if (diff != (div * coeff_gcd))		/* doesn't divide	*/
		return DV_NO;

	x *= div;
	y *= div;

	/*------------------------------------------------------*/
	/* all solutions to dep equation are now of the form	*/

	/* X = x + t * coeff2/coeff_gcd							*/
	/* Y = y + t * coeff1/coeff_gcd							*/

	/*------------------------------------------------------*/
	/* map bounds on X and Y to bounds on t (lo & up)		*/

	lo = MININT;
	up = MAXINT;

	/*--------------------------*/
	/* check bounds on X		*/

	if (no_X_bounds)
	{
		/* no bounds known for X, skip this section	*/
	}
	else if (!coeff2)
	{
		if ((x < lo_b1) || (x > up_b1))
			return DV_NO;
	}
	else if (coeff2 > 0)
	{
		temp1 = divide_plus((lo_b1-x)*coeff_gcd, coeff2, OP_CEILING);
		if (temp1 > lo)
			lo = temp1;

		temp1 = divide_plus((up_b1-x)*coeff_gcd, coeff2, OP_FLOOR);
		if (temp1 < up)
			up = temp1;
	}
	else		/* if (coeff2 < 0)	*/
	{
		temp1 = divide_plus((lo_b1-x)*coeff_gcd, coeff2, OP_FLOOR);
		if (temp1 < up)
			up = temp1;

		temp1 = divide_plus((up_b1-x)*coeff_gcd, coeff2, OP_CEILING);
		if (temp1 > lo)
			lo = temp1;
	}

	if (lo > up)
		return DV_NO;

	/*--------------------------*/
	/* check bounds on Y		*/

	if (no_Y_bounds)
	{
		/* no bounds known for Y, skip this section	*/
	}
	else if (!coeff1)
	{
		if ((y < lo_b2) || (y > up_b2))
			return DV_NO;
	}
	else if (coeff1 > 0)
	{
		temp1 = divide_plus((lo_b2-y)*coeff_gcd, coeff1, OP_CEILING);
		if (temp1 > lo)
			lo = temp1;

		temp1 = divide_plus((up_b2-y)*coeff_gcd, coeff1, OP_FLOOR);
		if (temp1 < up)
			up = temp1;
	}
	else		/* if (coeff1 < 0)	*/
	{
		temp1 = divide_plus((lo_b2-y)*coeff_gcd, coeff1, OP_FLOOR);
		if (temp1 < up)
			up = temp1;

		temp1 = divide_plus((up_b2-y)*coeff_gcd, coeff1, OP_CEILING);
		if (temp1 > lo)
			lo = temp1;
	}

	if (lo > up)
		return DV_NO;

	/*----------------------------------*/
	/* check for possible directions	*/

	d_eq = d_lt = d_gt = true;
	diff = coeff2 - coeff1;

	/*---*/
	/* = */

	if (!diff)
	{
		if (x != y)
			d_eq = false;
	}
	else		/* solve for t directly (store in div)	*/
	{
		temp1 = (y - x) * coeff_gcd;
		div = temp1 / diff;
		if ((temp1 != div * diff) || (div < lo) || (div > up))
			d_eq = false;
	}

	/*---*/
	/* < */

	if (!diff)
	{
		if (x > (y - 1))
			d_lt = false;
	}
	else
	{
		if (diff > 0)		/* get new upper bound	*/
		{
			temp1 = divide_plus((y-x-1)*coeff_gcd, diff, OP_FLOOR);
			if (temp1 < lo)
				d_lt = false;
		}
		else				/* get new lower bound	*/
		{
			temp1 = divide_plus((y-x-1)*coeff_gcd, diff, OP_CEILING);
			if (temp1 > up)
				d_lt = false;
		}
	}

	/*---*/
	/* > */

	if (!diff)
	{
		if (x < (y + 1))
			d_gt = false;
	}
	else
	{
		if (diff < 0)		/* get new upper bound	*/
		{
			temp1 = divide_plus((y-x+1)*coeff_gcd, diff, OP_FLOOR);
			if (temp1 < lo)
				d_gt = false;
		}
		else				/* get new lower bound	*/
		{
			temp1 = divide_plus((y-x+1)*coeff_gcd, diff, OP_CEILING);
			if (temp1 > up)
				d_gt = false;
		}
	}

	/*----------------------------------*/
	/* merge possible direction vectors	*/

	if (d_eq)
	{
		if (d_lt)
			return d_gt ? DV_LN : DV_LE;

		return d_gt ? DV_GE : DV_EQ;
	}

	if (d_lt)
		return d_gt ? DV_NE : DV_LT;

	return d_gt ? DV_GT : DV_NO;
}


/*----------------------------------------------------------------

	divide_plus()		Division plus FLOOR/CEILING operation

	Integer division in C truncates results, so we may need
	to clean up for FLOOR/CEILING operations.

*/

static int
divide_plus(int x, int y, int op)
{
	int result;

	result = x/y;

	if (x != (result * y))		/* division not exact, may need fix	*/
	{
		if (result > 0)				/* positive value, fix ceilings	*/
		{
			if (op == OP_CEILING)
				result++;
		}
		else if (op == OP_FLOOR)	/* negative	value, fix floors	*/
			result--;
	}

	return result;
}

/*----------------------------------------------------------------

	dt_propd()	-	propagate Delta constraints into one MIV subscript

	Returns:		SUBS_ZIV	converted all IV
					SUBS_MIV	still multiple IV left
					0-20		only one IV remains, level of IV returned

*/

static int 
dt_propd(Subs_data *subs1, Subs_data *subs2, Dvect_data *dvect, int clevel, 
         int max_level, int orig_iv)
{
	int             i;
	int             iv;			/* number of IV remaining 	 */
	int				coeff1;
	int				coeff2;

	iv = SUBS_ZIV;		/* initialize count of IV to zero	*/

	/*------------------------------------------------------*/
	/* assume dependences on all index vars					*/
	/* unless previously checked out						*/

	for (i = 0; i < clevel; i++)
	{
		/*----------------------------------*/
		/* look for IV for loop at level i	*/

		coeff1 = subs1->coeffs[i];
		coeff2 = subs2->coeffs[i];

		if (!coeff1 && !coeff2)
			continue;

		/*------------------------------------------*/
		/* found IV, check for constraints on it	*/

		switch (dvect->ltype[i])
		{
		case DV_PT:			/* point					 */

			subs1->coeffs[i] = 0;						/* elim IV 		*/
			subs2->coeffs[i] = 0;						/* elim IV 		*/

			subs1->constant += ((dvect->c1[i] * coeff1)	/* fix const	*/
							- (dvect->c2[i] * coeff2));
			break;

		case DV_DIS:		/* distance 				 */

			/*--------------------------------------*/
			/* exact reduction if coeffs are equal	*/

			if (coeff1 == coeff2)
			{
				subs1->coeffs[i] = 0;					/* elim IV 		*/
				subs2->coeffs[i] = 0;					/* elim IV 		*/

				/* tricky adjustment here	*/

				subs2->constant += dvect->c1[i] * coeff2;	/* add in diff	*/
			}

			/*------------------------------------------------------*/
			/* else need to choose which to elim, or expand both	*/

			else
			{
				iv = (iv == SUBS_ZIV) ? i : SUBS_MIV;	/* IV remains	*/

				/* for now just elim smaller term	*/

				if (ABS(coeff1) > ABS(coeff2))
				{
					subs1->coeffs[i] = coeff1 - coeff2;
					subs2->coeffs[i] = 0;

					/* tricky adjustment here	*/

					subs2->constant += dvect->c1[i] * coeff2;	/* add in diff	*/
				}
				else
				{
					subs1->coeffs[i] = 0;
					subs2->coeffs[i] = coeff2 - coeff1;

					/* tricky adjustment here	*/

					subs1->constant += dvect->c1[i] * coeff1;	/* add in diff	*/
				}
			}
			break;

		case DV_EQ:			/* equal					 */

			/*--------------------------------------*/
			/* exact reduction if coeffs are equal	*/

			if (coeff1 == coeff2)
			{
				subs1->coeffs[i] = 0;			/* elim IV 			*/
				subs2->coeffs[i] = 0;			/* elim IV 			*/
			}

			/*------------------------------------------------------*/
			/* else need to choose which to elim, or expand both	*/

			else
			{
				iv = (iv == SUBS_ZIV) ? i : SUBS_MIV;	/* IV remains	*/

				/* for now just elim smaller term	*/

				if (ABS(coeff1) > ABS(coeff2))
				{
					subs1->coeffs[i] = coeff1 - coeff2;
					subs2->coeffs[i] = 0;
				}
				else
				{
					subs1->coeffs[i] = 0;
					subs2->coeffs[i] = coeff2 - coeff1;
				}
			}
			break;

		case DV_LN:			/* line						 */
		case DV_LT:			/* less than				 */
		case DV_GT:			/* greater than				 */
		case DV_LE:			/* less than or equal		 */
		case DV_GE:			/* greater than	or equal	 */
		case DV_NE:			/* not equal				 */

			/* should apply constraint, but messy - leave for later	*/

			iv = (iv == SUBS_ZIV) ? i : SUBS_MIV;	/* IV remains	*/
			break;

		case DV_ANY:		/* any 						 */
		case DV_NO:			/* no dependence			 */

			iv = (iv == SUBS_ZIV) ? i : SUBS_MIV;	/* IV remains	*/
			break;
		}
	}

	/*------------------------------------------*/
	/* now check for IVs from non-shared loops	*/

	for (i = clevel; i < max_level; i++)
	{
		if (subs1->coeffs[i] || subs2->coeffs[i])
		{
			iv = (iv == SUBS_ZIV) ? i : SUBS_MIV;	/* IV remains	*/
		}
	}

	/* return ZIV or SIV if possible, else original IV */
 	/* do this because we don't distinguish between MIV and RDIV here */

	return ((iv == SUBS_ZIV) || (iv <= SUBS_SIV)) ? iv : orig_iv;
} /* end_dt_propd */


/*----------------------------------------------------------------

	dt_rdiv()	-	test all RDIV subscripts

	Returns:		false if no dep
					true  if possible dep
*/

static Boolean
dt_rdiv(int *subs_flag, int max_dim, Loop_list *loops1, Loop_list *loops2, 
        Subs_data *subs1, Subs_data *subs2, Dvect_data *dvect, int clevel)
{
	int i;

	for (i = 0; i < max_dim; i++)
	{
	  if (subs_flag[i] != SUBS_RDIV)	/* only need check RDIV subs */
	    continue;

	  if (!dt_rdiv1(loops1, loops2, subs1 + i, subs2 + i, 
			dvect->ltype, clevel))
	    return false;
	}

	return true;
} /* end_dt_rdiv */


/*----------------------------------------------------------------

	dt_rdiv1()	-	test single RDIV subscript

    Test for simple triangular loops.  Example:

    do i = 1,n           do i = 1,n        do i = 1,n
      do j = i+1,n         do j = 1,i-1      do j = i,n
        A(i,j) = A(i,i)      A(i,j) = A(i,i)    A(i,j) = A(i,i)

	Returns:		false if no dep
					true  if possible dep

    Currently does not return true, since not precise enough to prove 
    independence.  However, can eliminate dependences carried by 
    certain inner loops.  This is accomplished by restricting the
    number of possible direction vectors.

    Some imprecision for loops is caused by the collapsed representation
    for direction vectors.  For loops such as 

      do i = 1,n
        do j = i,n
          A(i) = A(j)

    there can be no dependences carried on the inner "j" loop,
    since the only dependence is on its first iteration.  Therefore
    only vectors of the form (=,=) are permitted.  However, vectors
    of the form (<,=) or (>,=) are still possible, so it's not 
    quite correct to force the entry for "j" to "=".  We do so
    anyway to eliminate dependences carried on the "j" loop.
    
*/

static Boolean
dt_rdiv1(Loop_list *loops1, Loop_list *loops2, Subs_data *subs1, 
         Subs_data *subs2, int *ltype, int clevel)
{
	int i, level, idx1, idx2, diff, constraint;
	Subs_data sdata;	/* used to parse symbolic loop bounds */

	/*---------------------------------------------------*/
	/* find coeffs of index var in first subscript		 */

	for (i = 0; i < loops1->level; i++)
	{
		if (subs1->coeffs[i])
		{
		  idx1 = i;
		  if ((subs1->coeffs[i] != 1) || (i >= clevel))
		    return true;  		/* test fails	*/
		}
	}
	
	/*---------------------------------------------------*/
	/* find coeffs of index var in second subscript	 */

	for (i = 0; i < loops2->level; i++)
	{
		if (subs2->coeffs[i])
		{
		  idx2 = i;
		  if ((subs2->coeffs[i] != 1) || (i >= clevel))		
		    return true;  		/* test fails	*/
		}
	}

	diff = subs1->constant - subs2->constant;  /* diff between subs */

	/*---------------------------------------------------*/
	/* check for triangular loop	 */

	if (idx1 < idx2)	/* 2nd subscript = deeper loop */
	{
	  /* if simple constraint between ivar1 & ivar2 found on lower bound */

		bzero((char *)&sdata, sizeof(Subs_data));
		if ((loops2->loops[idx2].lo.type != Expr_constant) &&
			(!dt_sub(loops2->loops[idx2].lo.ast, &sdata, loops2, true)) &&
			(sdata.coeffs[idx1] == 1))
		{
			if (sdata.constant > diff)
				ltype[idx1] = DV_NE;
			else if (sdata.constant == diff)
				ltype[idx2] = DV_EQ;
		}

		/* if simple constraint between ivar1 & ivar2 found on upper bound */

		bzero((char *)&sdata, sizeof(Subs_data));
		if ((loops2->loops[idx2].up.type != Expr_constant) &&
			(!dt_sub(loops2->loops[idx2].up.ast, &sdata, loops2, true)) &&
			(sdata.coeffs[idx1] == 1))
		{
			if (sdata.constant < diff)
				ltype[idx1] = DV_NE;
			else if (sdata.constant == diff)
				ltype[idx2] = DV_EQ;
		}
	}

	else if (idx1 > idx2)	/* 1st subscript = deeper loop */
	{
		/* if simple constraint between ivar1 & ivar2 found on lower bound */

		bzero((char *)&sdata, sizeof(Subs_data));
		if ((loops1->loops[idx1].lo.type != Expr_constant) &&
			(!dt_sub(loops1->loops[idx1].lo.ast, &sdata, loops1, true)) &&
			(sdata.coeffs[idx2] == 1))
		{
			if (sdata.constant > diff)
				ltype[idx2] = DV_NE;
			else if (sdata.constant == diff)
				ltype[idx1] = DV_EQ;
		}

		/* if simple constraint between ivar1 & ivar2 found on upper bound */

		bzero((char *)&sdata, sizeof(Subs_data));
		if ((loops1->loops[idx1].up.type != Expr_constant) &&
			(!dt_sub(loops1->loops[idx1].up.ast, &sdata, loops1, true)) &&
			(sdata.coeffs[idx2] == 1))
		{
			if (sdata.constant < diff)
				ltype[idx2] = DV_NE;
			else if (sdata.constant == diff)
				ltype[idx1] = DV_EQ;
		}
	}
	else /* (idx1 == idx2) */	/* error, should be SIV */
	{
		printf("dt_rdiv1(): SIV subscript found\n");
	}

	return true;
} /* end_dt_rdiv1 */


/*----------------------------------------------------------------

	dt_gcd()	-	apply gcd test to all MIV subscripts

	Returns:		false if no dep
					true  if possible dep
*/

static Boolean
dt_gcd(int *subs_flag, int clevel, int max_dim, Loop_list *loops1, 
       Loop_list *loops2, Subs_data *subs1, Subs_data *subs2)
{
	int i;

	for (i = 0; i < max_dim; i++)
	{
	  if (subs_flag[i] != SUBS_MIV)	     /* only need check MIV subs */
	    continue;

	  if (!dt_gcd1(loops1, loops2, subs1 + i, subs2 + i, clevel))
	    return false;
	}

	return true;
}

/*----------------------------------------------------------------

	dt_gcd1()	-	apply gcd test to single dim

	GCD of all coefficents must divide into difference of constant terms
	Coefficients are multiplied by the loop step
	Must be at least 2 coeffs, or else SIV subscript

	Returns:		false if no dep
				true  if possible dep

	Rewritten:		930602 (mpal and nenad)
	To handle differences based on lower bounds of loops
	and unmatched induction variables -- A(i) vs A(i+j)
*/

static Boolean
dt_gcd1(Loop_list *loops1, Loop_list *loops2, Subs_data *subs1, Subs_data *subs2,
        int clevel)
{
	int i;
	int diff;
	int num;
	int temp;
	int buf[128];

	/*--------------------------------------------------------------*/
	/* for each common level, find coeffs of index vars  		*/
	/* and compute difference caused by lower bounds of loops	*/
	
	diff	= 0;
	num	= 0;
	for (i = 0; i < clevel; i++)
	{
	  if (subs2->coeffs[i])
	    {
	      temp = subs2->coeffs[i] * loops2->loops[i].step.val;
	      if ((temp = ABS(temp)) == 1)		/* |coeff| = 1	*/
		return true;				/* gcd test fails */
	      buf[num++] = temp;			/* store	*/
	    }
	  if (subs1->coeffs[i])
	    {
	      temp = subs1->coeffs[i] * loops1->loops[i].step.val;
			if ((temp = ABS(temp)) == 1)	/* |coeff| = 1	*/
			  return true;			/* gcd test fails */
	      buf[num++] = temp;			/* store	*/
	    }
	  if( subs1->coeffs[i] != subs2->coeffs[i] ) {
	    if( loops1->loops[i].lo.type == Expr_constant ) {
	      diff       += subs1->coeffs[i] * loops1->loops[i].lo.val;
	      diff       -= subs2->coeffs[i] * loops2->loops[i].lo.val;
	    }
	    else  return true;
	  }
	  /* if coefficiants are equal, no difference	*/
	}

	/*--------------------------------------------------------------------*/
	/* for non-common levels,find coeffs of index vars in first subscript */
	
	for (i = clevel; i < loops1->level; i++)
	{
	  if (subs2->coeffs[i])
	    {
	      temp = subs2->coeffs[i] * loops2->loops[i].step.val;
	      if ((temp = ABS(temp)) == 1)		/* |coeff| = 1	*/
		return true;				/* gcd test fails */
	      buf[num++] = temp;			/* store	*/
	    }
	  if( loops1->loops[i].lo.type == Expr_constant ) {
	    diff       += subs1->coeffs[i] * loops1->loops[i].lo.val;
	  }
	  else	return true;
	}
	
	/*--------------------------------------------------------------------*/
	/* for non-common levels,find coeffs of index vars in second subscript*/
	
	for (i = clevel; i < loops2->level; i++)
	{
	  if (subs1->coeffs[i])
	    {
	      temp = subs1->coeffs[i] * loops1->loops[i].step.val;
	      if ((temp = ABS(temp)) == 1)		/* |coeff| = 1	*/
		return true;				/* gcd test fails */
	      buf[num++] = temp;	 	        /* store	*/
	    }
	  if( loops1->loops[i].lo.type == Expr_constant ) {
	    diff       -= subs2->coeffs[i] * loops2->loops[i].lo.val;
	  }
	  else	return true;
	}
	

	if (!num)		/* make sure IV actually found	*/
	{
		/* printf("dt_gcd1(): no IV found in subs!\n"); */
		return true;
	}
	
	/*-------------------------------*/
	/* find gcd of all coefficients	 */
	
	if ((temp = gcd2(num, buf)) == 1)
		return true;
	
	/*----------------------------------*/
	/* check that it divides into diff  */
	
	diff += subs1->constant - subs2->constant;
	diff  = ABS(diff);
	
	if (DIVIDES(temp, diff))
		return true;

	return false;		/* no integer solutions!	*/
}


/* ************************** BANERJEE's TESTs ***************************** */
/* Reference [Wolfe,1989, "Optimizing Supercompilers for Supercomputers",p25]*/
/* At each level we bound the subscript expression for that loop induction   */
/* variable (ie. at level 2 we are bound a2*i2 - b2*i2 , notation as above.  */
/* or 	a_coeff * i[level]	and	b_coeff * i[level]	here.	     */
/* ************************************************************************* */


/* ************************ Forward Declarations *************************** */
/*
	dt_banj_rect()	-  Apply Banerjee's test for possible directions
			   accumulating the data until reaching a 
			   dependence which is not possible.

	dt_banj_recurs() - Check Banerjee's inequalities for possible 
	                   directions. Using the accumulated data from 
			   dt_banj_rect().

	dt_banj_any()	-  Banerjee's test for direction	* 

	dt_banj_eq()	-  Banerjee's test for direction	= 

	dt_banj_gt()	-  Banerjee's test for direction	> 

	dt_banj_lt()	-  Banerjee's test for direction	<

	dt_dv_meet1( )	-  Translate from the local bit-vectors to 
	                   standard ped DV_...

*/

/*static Boolean  dt_banj_rect();
static Boolean  dt_banj_recurs();
static void 	dt_banj_any();
static void	dt_banj_eq();
static void	dt_banj_gt();
static void	dt_banj_lt();
static int	dt_dv_meet1();*/


/* ************************ Banerjee's Code ******************************* */

/*----------------------------------------------------------------

	dt_banj()	-	apply banerjee test to all MIV subscripts

	Returns:		false if no dep
	                        true  if possible dep
*/
static Boolean
dt_banj(int *subs_flag, int max_dim, Loop_list *loops1, Loop_list *loops2, 
        Subs_data *subs1, Subs_data *subs2, Dvect_data *dvect, int same_loop, 
        int max_level)
{
	int             dim;

	for (dim = 0; dim < max_dim; dim++)
	  {
	    if ( (subs_flag[dim] != SUBS_MIV)	/* only need check MIV subs */
		&& (subs_flag[dim] != SUBS_RDIV) ) 	/* can check RDIV */
	      continue;

	    if (!dt_banj1(loops1, loops2, subs1 + dim, subs2 + dim, 
			  dvect, same_loop, max_level))
	      return false;
	  }

	return true;
}

/*----------------------------------------------------------------

	dt_banj1()	-	Apply banerjee test to one dimension

	Use banerjee's inequality to find dependence, calculate 
	direction/distance vector if dependence exists.
	Mike Paleczny,  circa 90/07/--
	revised, 910515
*/

static Boolean
dt_banj1(Loop_list *loops1, Loop_list *loops2, Subs_data *subs1, 
         Subs_data *subs2, Dvect_data *dvect, int same_loop, int max_level)
{
	int             i;
	int             level;		/* loop nesting level	 */
	Loop_data      *loopDataPtr;	/* temporary pointer into Loop_data */
	int		diff;		/* diff between constants in subscript*/
	Boolean		rectangle;	/* indicates if iteration region is */
					/* rectangular or a trapezoid.	*/
	Dvect_data      localDVect;	/* locally computed direction vectors*/
	                   		/* init = DV_ANY = 0  */

	int		up_b;
	int		lo_b;
	int		l_step;
	int		coeff;		/* temporary for loop IV */
	int		lower;		/* accumulators	*/
	int		upper;


	/*------------------------------------------------------
	 * assume that all coefficiants are constants,
	 * Each subscript expression is of the form
	 * (   , a0 + a1*i1 + a2*i2 + ... + an*in,   )
	 * We are comparing two of these expressions for possible equality.
	 * a0 + a1*i1 + a2*i2 + ... + an*in = b0 + b1*j1 + b2*j2 + ... + bn*jn
	 * rearranging:
	 * a1*i1 + a2*i2 + ... + an*in - b1*j1 - b2*j2 - ... - bn*jn = b0 - a0

	 * For this Diophantine Equation we know that 
	 *	gcd( a1, a2, ..., an, b1, b2, ..., bn ) divides (b0 - a0)
	 * is a necessary condition for solution.
	 * <see gcd test above.>

	 * Using the bounds on each index variable we can obtain 
	 * upper and lower bounds for the possible value of the left-hand-side
	 * in the above equation.  if (b0 - a0) is outside these bounds
	 * there is no possibility of dependence.
	 *-----------------------------------------------------*/


	/* Check that region formed by IVs in subscript has constant bounds.
	 * This implies that it is a rectangle.  Already checked for SUBS_SYM.
	 */
	rectangle  = true;
	for( i=0; rectangle && (i < max_level); i++ )  {
	  if( subs1->coeffs[i] )		/* this loop index is used */
	    {
	      loopDataPtr	= &(loops1->loops[i]);
	      if( (loopDataPtr->lo.val == MININT) 
		 || (loopDataPtr->up.val == MAXINT)
		 || loopDataPtr->step.type != Expr_constant )
		/* symbolic parameters in loop	*/
		{
		  rectangle	= false;
		}
	    }
	  if( subs2->coeffs[i] )		/* this loop index is used */
	    {
	      loopDataPtr	= &(loops2->loops[i]);
	      if( (loopDataPtr->lo.val == MININT) 
		 || (loopDataPtr->up.val == MAXINT)
		 || loopDataPtr->step.type != Expr_constant )
		/* symbolic parameters in loop	*/
		{
		  rectangle	= false;
		}
	    }
	} /* rof	*/

	/* Simple Case:  Region formed by the index variables is a rectangle.
	 * This requires up_bound and lo_bound to be constants.
	 * To keep things simple, require step to be constant also.
	 */

	/* Prepare local direction vectors for analysis, convert line and point
	 * dependences to directions? These are probably exact, and there is
	 * no need to reanalyze.
	 */

    if( rectangle )
      {
	return dt_banj_rect(loops1, loops2, subs1, subs2, 
			    dvect, same_loop, max_level);
      }	/* end of rectangle tests */
    else  					/* deal with trapezoids	*/
      {
	return true;			/* no triangle tests yet	*/
      }

} /* end banj1()	*/



/* !! back to assuming perfect nesting, will have to split into two loops */

struct dt_BanjMemo	{
  int		diff;		/* Difference between a0 and b0 constants */
  int		loDelta_any[MAXLOOP];		
  int		upDelta_any[MAXLOOP];		
  int		loDelta_eq[MAXLOOP];		
  int		upDelta_eq[MAXLOOP];		
  int		loDelta_gt[MAXLOOP];		
  int		upDelta_gt[MAXLOOP];		
  int		loDelta_lt[MAXLOOP];		
  int		upDelta_lt[MAXLOOP];		
  int		ltype[MAXLOOP];		/* direction vector scratch pad.*/
  int		used[MAXLOOP];	/* nonzero -> loop indx var. used in subscript*/
};

/* Bit vector format for quick unions of direction vectors	910531	*/
/* still need to push through code changes.				*/
#define BANJ_NO	00
#define	BANJ_EQ		01
#define	BANJ_LT		02
#define	BANJ_LE		03
#define BANJ_GT		04
#define	BANJ_GE		05
#define	BANJ_NE		06
#define	BANJ_ANY	07

#define	BANJ_MASK_ANY	00
#define	BANJ_MASK_EQ	06
#define	BANJ_MASK_LT	05
#define BANJ_MASK_GT	03
#define BANJ_MASK_NONE	00

/*----------------------------------------------------------------

	dt_banj_rect()	-	Apply Banerjee's test for possible directions
	accumulating the data until reaching a dependence which is not possible

	Mike Paleczny,  TimeKey: 91/05/22

	Modified by nenad, 93/06/07 to support the case when the
	loops are not perfectly nested.
*/
static Boolean
dt_banj_rect(Loop_list *loops1, Loop_list *loops2, Subs_data *subs1, 
             Subs_data *subs2, Dvect_data *dvect, int same_loop, int max_level)
{
	int             i;
	int             level;		/* loop nesting level	 */
	Loop_data      *loopDataPtr;	/* temporary pointer into Loop_data */
	Loop_data      *loopDataPtr1;	/* temporary pointer into Loop_data */
	Loop_data      *loopDataPtr2;	/* temporary pointer into Loop_data */

	Boolean		rectangle;	/* indicates if iteration region is */
					/* rectangular or a trapezoid.	*/

	/*	Storage for partial sums of limits on upper and lower bounds.*/
	dt_BanjMemo	memo;

	int		up_b;
	int		lo_b;
	int		l_step;
	int		coeff;		/* temporary for loop IV */
	int		lower;		/* accumulators		 */
	int		upper;


	if( (max_level < 1) || (max_level > MAXLOOP) )
	  return	true;

	/*------------------------------------------------------*/
	/* assume dependences on all index vars			*/
	/* unless previously checked out			*/
	/* Initialize local Dirction Vectors to *               */

        for( level = 0; level < max_level; level++ )
	  {
	    memo.loDelta_any[level] = 0;
	    memo.upDelta_any[level] = 0;
	    memo.ltype[level] = BANJ_NO;
	    memo.used[level]  = (subs1->coeffs[level] | subs2->coeffs[level]);
	  }

	memo.diff = subs2->constant - subs1->constant;

	/* Compute the delta values for upper and lower bounds of DV_ANY
	 * dependence, but store the partial sums of these for dynamic use
	 * when investigating modifications to the direction vector.
	 */
	
	/* Peel off the first iteration to initialize array	*/
	level = max_level-1;
	loopDataPtr1 = &(loops1->loops[level]);
	loopDataPtr2 = &(loops2->loops[level]);
	dt_banj_any(loopDataPtr1->lo.val, loopDataPtr1->up.val, 
		    loopDataPtr2->lo.val, loopDataPtr2->up.val, 
		    subs1->coeffs[level], subs2->coeffs[level],
		    &lower, &upper );
	memo.loDelta_any[level]	= lower;
	memo.upDelta_any[level]	= upper;

	level--;
	for( ; level >= 0; level-- )
	  {
	    if( memo.used[level] )
	      {
		loopDataPtr1 = &(loops1->loops[level]);
		loopDataPtr2 = &(loops2->loops[level]);
		dt_banj_any(loopDataPtr1->lo.val, loopDataPtr1->up.val, 
			    loopDataPtr2->lo.val, loopDataPtr2->up.val, 
			    subs1->coeffs[level], subs2->coeffs[level],
			    &lower, &upper );
		memo.loDelta_any[level]	= lower + memo.loDelta_any[level+1];
		memo.upDelta_any[level]	= upper + memo.upDelta_any[level+1];
	      }
	    else
	      {
		memo.loDelta_any[level]	= memo.loDelta_any[level+1];
		memo.upDelta_any[level]	= memo.upDelta_any[level+1];
	      }
	  }
	
	/*--------------------------------------------------------------*/
	/* check for "*,*,*,..." direction, and then try subcomponents	*/

	if( !IN_BOUND(memo.diff,memo.loDelta_any[0],memo.upDelta_any[0]) )
	  {

/***********************************************************************
 *  Merging is not necessary when there is no dependence !!!
 * ----------------------------------------------------------
 *	    for( level=0; level < max_level; level++ )
 *	      dvect->ltype[level] = dt_dv_meet1(dvect->ltype[level], 
 *						memo.ltype[level]);
 ***********************************************************************/

	    return	false;		/* no dependence possible	*/
	  }

	/*--------------------------------------------------------------*/
	/* check for "<,*,*,... ", and other possible directions	*/

	/*---------------------------------------------------------------*/
	/* Compute contributions from each level (memoize for efficiency)*/
	for( level = 0; level < same_loop; level++ )
	  {
	    /* Check if either array uses this loop	*/
	    if( memo.used[level] )		
	      {
		loopDataPtr = &(loops1->loops[level]);
		dt_banj_eq(loopDataPtr->lo.val, loopDataPtr->up.val, loopDataPtr->step.val,
			   subs1->coeffs[level], subs2->coeffs[level],
			   &(memo.loDelta_eq[level]), &(memo.upDelta_eq[level]) );
		dt_banj_gt(loopDataPtr->lo.val, loopDataPtr->up.val, loopDataPtr->step.val,
			   subs1->coeffs[level], subs2->coeffs[level],
			   &(memo.loDelta_gt[level]), &(memo.upDelta_gt[level]) );
		dt_banj_lt(loopDataPtr->lo.val, loopDataPtr->up.val, loopDataPtr->step.val,
			   subs1->coeffs[level], subs2->coeffs[level],
			   &(memo.loDelta_lt[level]), &(memo.upDelta_lt[level]) );
	      }
	    else		/* Not used, contribution is zero	*/
	      {
		memo.loDelta_eq[level]  = 0;
		memo.upDelta_eq[level]  = 0;
		memo.loDelta_gt[level]  = 0;
		memo.upDelta_gt[level]  = 0;
		memo.loDelta_lt[level]  = 0;
		memo.upDelta_lt[level]  = 0;
	      }
	  }

	/*-----------------------------------------------------------------*/
	/* Recursively explore heirarchy, examine levels used in subscript */

	if (dt_banj_recurs(0, 0, 0, &memo, max_level-1, same_loop-1))
	  {
	    /*----------------------------------------------------------*/
	    /* Merge local dependence vector with this reference pair's */

	    for( level=0; level < max_level; level++ )
	      {
		dvect->ltype[level] = dt_dv_meet1(dvect->ltype[level], 
						  memo.ltype[level]);
	      }

	    return  true;
	  }
	
	else
	  return  false;

} /* end_dt_banj_rect */


/*----------------------------------------------------------------

	dt_banj_recurs()	- Check Banerjee's inequalities for possible 
	directions. Using the accumulated data from dt_banj_rect().

	Mike Paleczny,  TimeKey: 91/05/23

	Modified by nenad, 6/14/93 to explore the hierarchy only on the
	common levels (all other directions have to be assumed *)
*/
static Boolean
dt_banj_recurs(int loSum, int upSum, int level, dt_BanjMemo *memo, 
               int last_level, int common_level)
     /* int	loSum;	contribution to lower bound from outer levels */
     /* int	upSum;	contribution to upper bound from outer levels */
     /* int	level;	loop nesting level to examine, 0 based	 */
     /* dt_BanjMemo *memo;   memoized contributions for direction and level*/
     /* int	last_level;  last level to examine, 1 based 	 */
     /* int     common_level; maximum common loop level		 */
{
  int		lower;
  int		upper;
  Boolean 	return_flag = false;

  	lower	= loSum;
	upper	= upSum;

	if( level < common_level )	/* More levels to examine ...	*/
	  {
	    if( memo->used[level] )	/* Check if it is used in subscript */
	      {
		lower	+= memo->loDelta_any[level+1];
		upper	+= memo->upDelta_any[level+1];
		
		/* Check the directions at this level		*/
		
		if( IN_BOUND(memo->diff, lower + memo->loDelta_eq[level], 
			                 upper + memo->upDelta_eq[level]) )
		  { /* possible  =  dependence	*/
		    if (dt_banj_recurs(loSum + memo->loDelta_eq[level], 
				       upSum + memo->upDelta_eq[level],
				       level+1, memo, last_level,common_level))
		      {
			memo->ltype[level] |= BANJ_EQ;
			return_flag = true;
		      }
		  }
		
		if( IN_BOUND(memo->diff, lower + memo->loDelta_gt[level], 
			                 upper + memo->upDelta_gt[level]) )
		  { /* possible  >  dependence	*/
		    if (dt_banj_recurs(loSum + memo->loDelta_gt[level], 
				       upSum + memo->upDelta_gt[level],
				       level+1, memo, last_level,common_level))
		      {
			memo->ltype[level] |= BANJ_GT;
			return_flag = true;
		      }
		  }
		
		if( IN_BOUND(memo->diff, lower + memo->loDelta_lt[level], 
			                 upper + memo->upDelta_lt[level]) )
		  { /* possible  <  dependence	*/
		    if (dt_banj_recurs(loSum + memo->loDelta_lt[level], 
				       upSum + memo->upDelta_lt[level],
				       level+1, memo, last_level,common_level))
		      {
			memo->ltype[level] |= BANJ_LT;
			return_flag = true;
		      }
		  }
	      }
	    else			/* This Level Not Used	*/
	      {
		if (dt_banj_recurs(loSum, upSum, level+1, 
			       memo, last_level, common_level))
		  {
		    memo->ltype[level] = BANJ_ANY;
		    return_flag = true;
		  }
	      }
	    
	  }
	else				/* Last level to examine  ....	*/
	  {
	    if (level < last_level)     /* non-common directions are all * */
	      {
		lower  +=  memo->loDelta_any[level+1];
		upper  +=  memo->upDelta_any[level+1];
	      }

	    if( memo->used[level] )	/* Check if it is used in subscript */
	      {
		if( IN_BOUND(memo->diff, lower + memo->loDelta_eq[level], 
			                 upper + memo->upDelta_eq[level]) )
		  { /* possible  =  dependence	*/
		    memo->ltype[level]	|= BANJ_EQ;
		    return_flag = true;
		  }
		
		if( IN_BOUND(memo->diff, lower + memo->loDelta_gt[level], 
			                 upper + memo->upDelta_gt[level]) )
		  { /* possible  >  dependence	*/
		    memo->ltype[level]	|= BANJ_GT;
		    return_flag = true;
		  }
		
		if( IN_BOUND(memo->diff, lower + memo->loDelta_lt[level], 
			                 upper + memo->upDelta_lt[level]) )
		  { /* possible  <  dependence	*/
		    memo->ltype[level]	|= BANJ_LT;
		    return_flag = true;
		  }
	      }
	    else			/* This Level Not Used	*/
	      {
		memo->ltype[level]	= BANJ_ANY;
		return_flag = true;
	      }
	  }

  return  return_flag;
} /* end_dt_banj_recurs */


/*	==========================================================	*/
/*	The following routines compute the contribution to both upper
 *	and lower bounds in Banerjee's test for a particular instance
 *	of < array dimension, loop induction variable (level), and
 *	direction vector >.		mpal - 910522
 */
/*	==========================================================	*/

/*----------------------------------------------------------------

	dt_banj_any()	-	Banerjee's test for direction	* 

	Mike Paleczny,  TimeKey: 910522
	
	Modified by nenad, 6/7/93 to support the case when the
	loops are not perfectly nested.
*/
static void
dt_banj_any(int loBound1, int upBound1, int loBound2, int upBound2, 
	    int a_coeff, int b_coeff, int *loDelta, int *upDelta)
     /* int	 loBound1;		lower bound of loop	  */
     /* int	 upBound1;		upper bound of loop	  */
     /* int	 loBound2;		lower bound of loop	  */
     /* int	 upBound2;		upper bound of loop	  */
     /* int	 a_coeff;		linear coeffs for loop    */
     /* int 	 b_coeff;		induction variables	  */
     /* int	*loDelta;		change to lower bound     */
     /* int	*upDelta;		change to upper bound     */
{

  *loDelta = *upDelta = 0;

  /* perform correct action based on sign of coefficiants	*/
  if( a_coeff >0 )		
    {
      *loDelta	+= a_coeff * loBound1;
      *upDelta	+= a_coeff * upBound1;
    }
  else if( a_coeff < 0 )
    {
      *loDelta	+= a_coeff * upBound1;
      *upDelta	+= a_coeff * loBound1;
    }

  if( b_coeff >0 )
    {
      *loDelta	-= b_coeff * upBound2;
      *upDelta	-= b_coeff * loBound2;
    }
  else if( b_coeff < 0 )
    {
      *loDelta	-= b_coeff * loBound2;
      *upDelta	-= b_coeff * upBound2;
    }

  return;

} /* end_dt_banj_any */


/*----------------------------------------------------------------

	dt_banj_eq()	-	Banerjee's test for direction	= 

	Mike Paleczny,  TimeKey: 910522
*/
static void
dt_banj_eq(int loBound, int hiBound, int step, int a_coeff, int b_coeff, 
           int *loDelta, int *upDelta)
     /* int	 loBound;		lower bound of loop	*/
     /* int	 hiBound;		upper bound of loop	*/
     /* int	 step;			step size of loop	*/
     /* int	 a_coeff;		linear coeffs for loop  */
     /* int 	 b_coeff;		induction variables	*/
     /* int	*loDelta;		change to lower bound   */
     /* int	*upDelta;		change to upper bound   */
{
  int	range;
  int	iTemp;

	    iTemp	= a_coeff - b_coeff;
	    *loDelta	= iTemp * loBound;
	    *upDelta	= *loDelta;
	    range	= hiBound - loBound;

	    /* perform correct action based on sign of coefficiants	*/

	    if( iTemp >= 0 )		
	      {
		*upDelta	+= iTemp * range;
	      }
	    else 
	      {
		*loDelta	+= iTemp * range;
	      }

	    return;

} /* end_dt_banj_eq */


/*----------------------------------------------------------------

	dt_banj_gt()	-	Banerjee's test for direction	> 

	Mike Paleczny,  TimeKey: 910522
*/
static void
dt_banj_gt(int loBound, int hiBound, int step, int a_coeff, int b_coeff, 
           int *loDelta, int *upDelta)
     /* int	 loBound;		lower bound of loop	*/
     /* int	 hiBound;		upper bound of loop	*/
     /* int	 step;			step size of loop	*/
     /* int	 a_coeff;		linear coeffs for loop  */
     /* int 	 b_coeff;		induction variables	*/
     /* int	*loDelta;		change to lower bound   */
     /* int	*upDelta;		change to upper bound   */
{
  int	range;
  int	iTemp;

  	    iTemp	= a_coeff - b_coeff;
	    *loDelta	= iTemp * loBound + a_coeff * step;
	    *upDelta	= *loDelta;
	    range	= hiBound - loBound - step;

	    /* perform correct action based on sign of coefficiants	*/

	    if( b_coeff >= 0 )		
	      {
		if( iTemp < 0 )
		  *loDelta	+= iTemp * range;

		if( a_coeff > 0 )
		  *upDelta	+= a_coeff * range;
	      }
	    else 
	      {
		if( a_coeff < 0 )
		  *loDelta	+= a_coeff * range;
		
		if( iTemp > 0 )
		  *upDelta	+= iTemp * range;
	      }

	    return;

} /* end_dt_banj_gt */



/*----------------------------------------------------------------

	dt_banj_lt()	-	Banerjee's test for direction	<

	Mike Paleczny,  TimeKey: 910522
*/
static void
dt_banj_lt(int loBound, int hiBound, int step, int a_coeff, int b_coeff, 
           int *loDelta, int *upDelta)
     /* int	 loBound;		lower bound of loop	*/
     /* int	 hiBound;		upper bound of loop	*/
     /* int	 step;			step size of loop	*/
     /* int	 a_coeff;		linear coeffs for loop  */
     /* int 	 b_coeff;		induction variables	*/
     /* int	*loDelta;		change to lower bound   */
     /* int	*upDelta;		change to upper bound   */
{
  int	range;
  int	iTemp;

	    iTemp	= a_coeff - b_coeff;
	    *loDelta	= iTemp * loBound - b_coeff * step;
	    *upDelta	= *loDelta;
	    range	= hiBound - loBound - step;

	    /* perform correct action based on sign of coefficiants	*/
	    if( a_coeff > 0 )		
	      {
		if( b_coeff > 0 )
		  *loDelta	-= b_coeff * range;

		if( iTemp > 0 )
		  *upDelta	+= iTemp * range;
	      }
	    else if( a_coeff < 0 )
	      {
		if( b_coeff < 0 )
		  *upDelta	+= (-b_coeff) * range;

		if( iTemp < 0 )
		  *loDelta	+= iTemp * range;
	      }

	    return;

} /* end_dt_banj_lt */


/***************** TRAPEZOIDAL BANERJEE: nenad, 6/8/93 ****************/

/*-----------------------------------------------------------------
  The algorithm implemented here is from the unpublished notes by 
  R. Allen & K. Kennedy: Advanced Compiling for High Performance, 
  Chapter 3 - Dependence Testing, with some obvious bugs fixed.
------------------------------------------------------------------*/

/*-----------------------------------------------------------------

	dt_trap_banj1()	-  Apply trapezoidal banerjee test to one dimension

  	dt_trap_banj_recurs() -  Go through the hierarchy of direction
				 vectors checking Banerjee's inequalities

  	dt_trap_banj_in_bound()  - compute Banerjee's inequalities for the 
				   given direction vector saved in memo.

	dt_trap_banj_any()  -  Trapezoidal Banerjee's test for direction  * 

	dt_trap_banj_eq()   -  Trapezoidal Banerjee's test for direction  =

	dt_trap_banj_gt()   -  Trapezoidal Banerjee's test for direction  >

	dt_trap_banj_lt()   -  Trapezoidal Banerjee's test for direction  <

*/

/*static 	Boolean 	dt_trap_banj1();
static 	Boolean		dt_trap_banj_recurs();
static 	Boolean		dt_trap_banj_in_bound();
static 	void		dt_trap_banj_any();
static 	void		dt_trap_banj_eq();
static 	void		dt_trap_banj_gt();
static 	void		dt_trap_banj_lt();*/


/*----------------------------------------------------------------

	dt_trap_banj()	-	apply trapezoidal banerjee test 

	Returns:		false if no dep
	                        true  if possible dep
	nenad, 6/8/93
*/
static Boolean
dt_trap_banj(int *subs_flag, int max_dim, CfgInstance cfgInst, Loop_list *loops1,
             Loop_list *loops2, Subs_data *subs1, Subs_data *subs2, 
	     Dvect_data *dvect, int common_level, int max_level)
{
  Boolean	all_bounds_constant = true;
  int 		level;
  int		i;
  AST_INDEX   	temp;
  Loop_data    *loop;
  
  
  /*---------------------------------------------*/
  /* check that all loops are with the step of 1 */
  /* and that the loop nest is without symbolics */

  for (level = 0; level < common_level; level++)
    {
      loop = &(loops1->loops[level]);
      if (loop->step.type != Expr_constant         || 
	  loop->step.val  != 1                     ||
	  (loop->lo.type != Expr_constant  &&
	   loop->lo.type != Expr_index_var &&
	   loop->lo.type != Expr_linear_ivar_only) ||
	  (loop->up.type != Expr_constant  &&
	   loop->up.type != Expr_index_var &&
	   loop->up.type != Expr_linear_ivar_only))
	return true;

      if (loop->lo.type != Expr_constant || loop->up.type != Expr_constant)
	all_bounds_constant = false;
    }

  for (level = common_level; level < loops1->level; level++)
    {
      loop = &(loops1->loops[level]);
      if (loop->step.type != Expr_constant         || 
	  loop->step.val  != 1                     ||
	  (loop->lo.type != Expr_constant  &&
	   loop->lo.type != Expr_index_var &&
	   loop->lo.type != Expr_linear_ivar_only) ||
	  (loop->up.type != Expr_constant  &&
	   loop->up.type != Expr_index_var &&
	   loop->up.type != Expr_linear_ivar_only))
	return true;

      if (loop->lo.type != Expr_constant || loop->up.type != Expr_constant)
	all_bounds_constant = false;
    }

  for (level = common_level; level < loops2->level; level++)
    {
      loop = &(loops2->loops[level]);
      if (loop->step.type != Expr_constant         || 
	  loop->step.val  != 1                     ||
	  (loop->lo.type != Expr_constant  &&
	   loop->lo.type != Expr_index_var &&
	   loop->lo.type != Expr_linear_ivar_only) ||
	  (loop->up.type != Expr_constant  &&
	   loop->up.type != Expr_index_var &&
	   loop->up.type != Expr_linear_ivar_only))
	return true;

      if (loop->lo.type != Expr_constant || loop->up.type != Expr_constant)
	all_bounds_constant = false;
    }

  if (all_bounds_constant)
    return true; 		/* this case was handled in dt_banj_rect */

  for (i = 0; i < max_dim; i++)
    {
      if (subs_flag[i] != SUBS_EMPTY && subs_flag[i] != SUBS_SYM)
	if (!dt_trap_banj1(loops1, loops2, subs1+i, subs2+i, 
			   dvect, common_level, max_level))
	  return false;
    }

  return true;
}


/*typedef struct {  */    /* structure for memoizing the coefficients */
/*  int	diff; */		/* Difference between a0 and b0 constants */
/*  int	loDelta[MAXLOOP];		
  int	upDelta[MAXLOOP];		
  int	ltype[MAXLOOP];	*/	/* direction vector scratch pad.*/
/*  int 	al[MAXLOOP][MAXLOOP];    
  int 	ar[MAXLOOP][MAXLOOP];
  int 	bl[MAXLOOP][MAXLOOP];
  int 	br[MAXLOOP][MAXLOOP];
  int   dir_vec[MAXLOOP];
} dt_TrapBanjMemo;*/

/*----------------------------------------------------------------

	dt_trap_banj1()	-  Apply trapezoidal banerjee test to one dimension

	Returns:		false if no dep
	                        true  if possible dep
	nenad, 6/8/93
*/

static Boolean
dt_trap_banj1(Loop_list *loops1, Loop_list *loops2, Subs_data *subs1, 
              Subs_data *subs2, Dvect_data *dvect, int common_level, 
              int max_level)
{
  dt_TrapBanjMemo 	memo;
  int 			level;
  int			lower;
  int			upper;
  int			al[MAXLOOP];
  int			ar[MAXLOOP];
  int			bl[MAXLOOP];
  int			br[MAXLOOP];

  /*---------------------------*/
  /* initialize memo structure */

  for (level = 0; level < max_level; level++)
    {
      memo.ltype[level]   	  = BANJ_NO;
      memo.al[max_level-1][level] = subs1->coeffs[level];
      memo.ar[max_level-1][level] = subs1->coeffs[level];
      memo.bl[max_level-1][level] = subs2->coeffs[level];
      memo.br[max_level-1][level] = subs2->coeffs[level];
      memo.dir_vec[level] 	  = BANJ_ANY;
    }

  memo.diff = subs2->constant - subs1->constant;

  lower = 0;
  upper = 0;

  /*--------------------------------------------------------------*/
  /* compute and save contributions for direction * at each level */

  for (level = max_level-1; level >= 0; level--)
    {
      dt_trap_banj_any(loops1, loops2, 
		       memo.al[level], memo.ar[level], 
		       memo.bl[level], memo.br[level], 
		       &lower, &upper, level, max_level);

      memo.loDelta[level] = lower;
      memo.upDelta[level] = upper;
    }

  if( !IN_BOUND(memo.diff,memo.loDelta[0],memo.upDelta[0]) )
    {
      return false;			/* no dependence possible */
    }

  /*-------------------------------------------------*/
  /* recursively explore direction vector hieararchy */

  if (dt_trap_banj_recurs(0, common_level-1, max_level-1, &memo, 
			  al, ar, bl, br, loops1))
    {
      /*------------------------------------------------------------------*/
      /* Merge our local dependence directions with this reference pair's */

      for( level=0; level < max_level; level++ )
	dvect->ltype[level] = 
	  dt_dv_meet1(dvect->ltype[level], memo.ltype[level]);

      return true;
    }

  else
    return false;
}


/*----------------------------------------------------------------

  	dt_trap_banj_recurs() -  Go through the hierarchy of direction
	vectors checking Banerjee's inequalities

	Returns:		false if no dep
	                        true  if possible dep
	nenad, 6/8/93
*/
static Boolean
dt_trap_banj_recurs(int level, int common_level, int last_level, 
                    dt_TrapBanjMemo *memo, int *al, int *ar, int *bl, int *br, 
                    Loop_list *loops)
{
  Boolean	return_flag = false;
  
  memcpy(al, memo->al[level], (last_level+1)*sizeof(int));
  memcpy(ar, memo->ar[level], (last_level+1)*sizeof(int));
  memcpy(bl, memo->bl[level], (last_level+1)*sizeof(int));
  memcpy(br, memo->br[level], (last_level+1)*sizeof(int));
  memo->dir_vec[level] = BANJ_EQ;
  if (dt_trap_banj_in_bound(level, last_level, memo, al, ar, bl, br, loops))
    {
      if (level < common_level)
	{
	  if (dt_trap_banj_recurs(level+1, common_level, last_level, memo, 
				  al, ar, bl, br, loops))
	    {
	      return_flag = true;
	      memo->ltype[level] |= BANJ_EQ;
	    }
	}
      else
	{
	  return_flag = true;
	  memo->ltype[level] |= BANJ_EQ;
	}
    }

  memcpy(al, memo->al[level], (last_level+1)*sizeof(int));
  memcpy(ar, memo->ar[level], (last_level+1)*sizeof(int));
  memcpy(bl, memo->bl[level], (last_level+1)*sizeof(int));
  memcpy(br, memo->br[level], (last_level+1)*sizeof(int));
  memo->dir_vec[level] = BANJ_GT;
  if (dt_trap_banj_in_bound(level, last_level, memo, al, ar, bl, br, loops))
    {
      if (level < common_level)
	{
	  if (dt_trap_banj_recurs(level+1, common_level, last_level, memo, 
				  al, ar, bl, br, loops))
	    {
	      return_flag = true;
	      memo->ltype[level] |= BANJ_GT;
	    }
	}
      else
	{
	  return_flag = true;
	  memo->ltype[level] |= BANJ_GT;
	}
    }

  memcpy(al, memo->al[level], (last_level+1)*sizeof(int));
  memcpy(ar, memo->ar[level], (last_level+1)*sizeof(int));
  memcpy(bl, memo->bl[level], (last_level+1)*sizeof(int));
  memcpy(br, memo->br[level], (last_level+1)*sizeof(int));
  memo->dir_vec[level] = BANJ_LT;
  if (dt_trap_banj_in_bound(level, last_level, memo, al, ar, bl, br, loops))
    {
      if (level < common_level)
	{
	  if (dt_trap_banj_recurs(level+1, common_level, last_level, memo, 
				  al, ar, bl, br, loops))
	    {
	      return_flag = true;
	      memo->ltype[level] |= BANJ_LT;
	    }
	}
      else
	{
	  return_flag = true;
	  memo->ltype[level] |= BANJ_LT;
	}
    }

  memo->dir_vec[level] = BANJ_ANY;
  
  return  return_flag;;
}


/*----------------------------------------------------------------

  	dt_trap_banj_in_bound() - compute Banerjee's inequalities for the 
				  given direction vector saved in memo.

	Returns: 		  true, if inequalities are satisfied
				  false, otherwise
	nenad, 6/8/93
*/
static Boolean
dt_trap_banj_in_bound(int level, int last_level, dt_TrapBanjMemo *memo, int *al, 
                      int *ar, int *bl, int *br, Loop_list *loops)	
{
  int 	lower;
  int 	upper;
  int  	i;
  
  if (level < last_level)
    {
      lower = memo->loDelta[level+1];
      upper = memo->upDelta[level+1];
    }
  else
    {
      lower = 0;
      upper = 0;
    }
  
  for (i = level; i >= 0; i--)
    {
      switch (memo->dir_vec[i])
	{
	case BANJ_EQ:
	  dt_trap_banj_eq(loops, memo->dir_vec, al,ar, bl,br, &lower,&upper, i);
	  break;

	case BANJ_GT:
	  dt_trap_banj_gt(loops, al, ar, bl, br, &lower, &upper, i);
	  break;

	case BANJ_LT:
	  dt_trap_banj_lt(loops, al, ar, bl, br, &lower, &upper, i);
	  break;
	}
    }
  
  if (IN_BOUND(memo->diff, lower, upper))
    return true;
  else
    return false;
}	  


/*----------------------------------------------------------------

	dt_trap_banj_any()  -  Trapezoidal Banerjee's test for direction * 

	nenad, 6/8/93
*/
static void
dt_trap_banj_any(Loop_list *loops1, Loop_list *loops2, int *al, int *ar, int *bl,
                 int *br, int *loDelta, int *upDelta, int level, int max_level)
{
  int  j;
  int *loBound1 = loops1->loops[level].lo_vec;
  int *upBound1 = loops1->loops[level].up_vec;
  int *loBound2 = loops2->loops[level].lo_vec;
  int *upBound2 = loops2->loops[level].up_vec;
  int  al_coeff = al[level];
  int  ar_coeff = ar[level];
  int  bl_coeff = bl[level];
  int  br_coeff = br[level];
  
  /*------------------------------------------------------*/
  /* perform correct action based on sign of coefficients */

  if (al_coeff > 0)
    *loDelta  +=  al_coeff * loBound1[0];
  else if (al_coeff < 0)
    *loDelta  +=  al_coeff * upBound1[0];

  if (ar_coeff > 0)
    *upDelta  +=  ar_coeff * upBound1[0];
  else if (ar_coeff < 0)
    *upDelta  +=  ar_coeff * loBound1[0];

  if (bl_coeff > 0)
    *loDelta  -=  bl_coeff * upBound2[0];
  else if (bl_coeff < 0)
    *loDelta  -=  bl_coeff * loBound2[0];
  
  if (br_coeff > 0)
    *upDelta  -=  br_coeff * loBound2[0]; 
  else if (br_coeff < 0)
    *upDelta  -=  br_coeff * upBound2[0]; 

  /*--------------------------------------------------------*/
  /* now update the coefficients of ind.vars of outer loops */

  if (level > 0)
    {
      memcpy(al-MAXLOOP, al, max_level*sizeof(int));
      memcpy(ar-MAXLOOP, ar, max_level*sizeof(int));
      memcpy(bl-MAXLOOP, bl, max_level*sizeof(int));
      memcpy(br-MAXLOOP, br, max_level*sizeof(int));
      al  -=  MAXLOOP;
      ar  -=  MAXLOOP;
      bl  -=  MAXLOOP;
      br  -=  MAXLOOP;
      
      for (j = 0; j < level; j++)
	{
	  if (al_coeff > 0)
	    al[j]  +=  al_coeff * loBound1[j+1];
	  else if (al_coeff < 0)
	    al[j]  +=  al_coeff * upBound1[j+1];
	  
	  if (ar_coeff > 0)
	    ar[j]  +=  ar_coeff * upBound1[j+1];
	  else if (ar_coeff < 0)
	    ar[j]  +=  ar_coeff * loBound1[j+1];

	  if (bl_coeff > 0)
	    bl[j]  +=  bl_coeff * upBound2[j+1];
	  else if (bl_coeff < 0)
	    bl[j]  +=  bl_coeff * loBound2[j+1];
	  
	  if (br_coeff > 0)
	    br[j]  +=  br_coeff * loBound2[j+1];
	  else if (br_coeff < 0)
	    br[j]  +=  br_coeff * upBound2[j+1];
	}
    }
}


/*----------------------------------------------------------------

	dt_trap_banj_eq()  -  Trapezoidal Banerjee's test for direction =

	nenad, 6/9/93
*/
static void
dt_trap_banj_eq(Loop_list *loops, int *dir, int *al, int *ar, int *bl, int *br, 
                int *loDelta, int *upDelta, int level)
{
  int  j;
  int  temp;
  int  nU;
  int  nL;
  int *loBound = loops->loops[level].lo_vec;
  int *upBound = loops->loops[level].up_vec;
  int  al_coeff = al[level];
  int  ar_coeff = ar[level];
  int  bl_coeff = bl[level];
  int  br_coeff = br[level];

  if ((temp = al_coeff - bl_coeff) > 0)
    *loDelta  +=  temp * loBound[0];
  else if (temp < 0)
    *loDelta  +=  temp * upBound[0];

  if ((temp = ar_coeff - br_coeff) > 0)
    *upDelta  +=  temp * upBound[0];
  else if (temp < 0)
    *upDelta  +=  temp * loBound[0];
  
  nU = nL = 0;
  for (j = 0; j < level; j++)
    {
      if (dir[j] == BANJ_GT)
	{
	  nL  +=  loBound[j+1];
	  nU  +=  upBound[j+1];
	}
      else if (dir[j] == BANJ_LT)
	{
	  nL  -=  loBound[j+1];
	  nU  -=  upBound[j+1];
	}
    }

  if (nL <= 0)
    {
      for (j = 0; j < level; j++)
	{
	  if ((temp = al_coeff - bl_coeff) > 0)
	    bl[j]  -=  temp * loBound[j+1];
	  if ((temp = ar_coeff - br_coeff) < 0)
	    br[j]  -=  temp * loBound[j+1];
	}
    }
  else
    {
      for (j = 0; j < level; j++)
	{
	  if ((temp = al_coeff - bl_coeff) > 0)
	    al[j]  +=  temp * loBound[j+1];
	  if ((temp = ar_coeff - br_coeff) < 0)
	    ar[j]  +=  temp * loBound[j+1];
	}
    }
	  
  if (nU <= 0)
    {
      for (j = 0; j < level; j++)
	{
	  if ((temp = al_coeff - bl_coeff) < 0)
	    al[j]  +=  temp * upBound[j+1];
	  if ((temp = ar_coeff - br_coeff) > 0)
	    ar[j]  +=  temp * upBound[j+1];
	}
    }
  else
    {
      for (j = 0; j < level; j++)
	{
	  if ((temp = al_coeff - bl_coeff) < 0)
	    bl[j]  -=  temp * upBound[j+1];
	  if ((temp = ar_coeff - br_coeff) > 0)
	    br[j]  -=  temp * upBound[j+1]; 
	}
    }
}


/*----------------------------------------------------------------

	dt_trap_banj_gt()  -  Trapezoidal Banerjee's test for direction  >

	nenad, 6/9/93
*/
static void
dt_trap_banj_gt(Loop_list *loops, int *al, int *ar, int *bl, int *br, 
                int *loDelta, int *upDelta, int level)
{
  int  j;
  int  temp;
  int *loBound = loops->loops[level].lo_vec;
  int *upBound = loops->loops[level].up_vec;
  int  al_coeff = al[level];
  int  ar_coeff = ar[level];
  int  bl_coeff = bl[level];
  int  br_coeff = br[level];

  temp = al_coeff;
  if (bl_coeff > 0)
    temp  -=  bl_coeff;
  else if (bl_coeff < 0)
    *loDelta  -=  bl_coeff * loBound[0];
  if (temp < 0)
    *loDelta  +=  temp * (upBound[0] - 1);
  else if (temp > 0)
    *loDelta  +=  temp * loBound[0];
  *loDelta  +=  al_coeff;
  
  temp = ar_coeff;
  if (br_coeff < 0)
    temp  -=  br_coeff;
  else if (br_coeff > 0)
    *upDelta  -=  br_coeff * loBound[0];
  if (temp > 0)
    *upDelta  +=  temp * (upBound[0] - 1);
  else if (temp < 0)
    *upDelta  +=  temp * loBound[0];
  *upDelta  +=  ar_coeff;
  
  for (j = 0; j < level; j++)
    {
      temp = al_coeff;
      if (bl_coeff > 0)
	temp  -=  bl_coeff;
      else if (bl_coeff < 0)
	bl[j]  +=  bl_coeff * loBound[j+1];
      if (temp < 0)
	al[j]  +=  temp * upBound[j+1];
      else if (temp > 0)
	bl[j]  -=  temp * loBound[j+1];
      
      temp = ar_coeff;
      if (br_coeff < 0)
	temp  -=  br_coeff;
      else if (br_coeff > 0)
	br[j]  +=  br_coeff * loBound[j+1];
      if (temp > 0)
	ar[j]  +=  temp * upBound[j+1];
      else if (temp < 0)
	br[j]  -=  temp * loBound[j+1];
    }
}

/*----------------------------------------------------------------

	dt_trap_banj_lt()  -  Trapezoidal Banerjee's test for direction  <

	nenad, 6/9/93
*/
static void
dt_trap_banj_lt(Loop_list *loops, int *al, int *ar, int *bl, int *br, 
                int *loDelta, int *upDelta, int level)
{
  int 	j;
  int 	temp;
  int *loBound = loops->loops[level].lo_vec;
  int *upBound = loops->loops[level].up_vec;
  int  al_coeff = al[level];
  int  ar_coeff = ar[level];
  int  bl_coeff = bl[level];
  int  br_coeff = br[level];

  temp = bl_coeff;
  if (al_coeff < 0)
    temp  -=  al_coeff;
  else if (al_coeff > 0)
    *loDelta  +=  al_coeff * loBound[0];
  if (temp > 0)
    *loDelta  -=  temp * (upBound[0] - 1);
  else if (temp < 0)
    *loDelta  -=  temp * loBound[0];
  *loDelta  -=  bl_coeff;
  
  temp = -br_coeff;
  if (ar_coeff > 0)
    temp  +=  ar_coeff;
  else if (ar_coeff < 0)
    *upDelta  +=  ar_coeff * loBound[0];
  if (temp > 0)
    *upDelta  +=  temp * (upBound[0] - 1);
  else if (temp < 0)
    *upDelta  +=  temp * loBound[0];
  *upDelta  -=  br_coeff;
  
  for (j = 0; j < level; j++)
    {
      temp = bl_coeff;
      if (al_coeff < 0)
	temp  -=  al_coeff;
      else if (al_coeff > 0)
	al[j]  +=  al_coeff * loBound[j+1];
      if (temp < 0)
	al[j]  -=  temp * loBound[j+1];
      else if (temp > 0)
	bl[j]  +=  temp * upBound[j+1];
      
      temp = -br_coeff;
      if (ar_coeff > 0)
	temp  +=  ar_coeff;
      else if (ar_coeff < 0)
	ar[j]  +=  ar_coeff * loBound[j+1];
      if (temp < 0)
	ar[j]  +=  temp * loBound[j+1];
      else if (temp > 0)
	br[j]  -=  temp * upBound[j+1];
    }
}


/*----------------------------------------------------------------

	This performs a meet between two direction vector components.
	WARNING: may have to translate from DV_PT and/or DV_LN to _LT, _GT, ...
 */ 
static int
dt_dv_meet1(int dv1, int dv2)
{
  int	dv;

  /*	Translate from the local bit-vectors to standard ped_speak, and	*/
  /* take the intersection of current information with Banerjee's	*/

  /* This should be replaced by indexing into an array !!!		*/
  /* OR -- translate to DV_.. form and generalize intersection for that	*/
  switch( dv2 )
    {
    case	BANJ_NO :
      return( DV_NO );
      break;
    case	BANJ_EQ :
      switch( dv1 )
	{
	case	DV_ANY : return( DV_EQ );
	case	DV_EQ : return( DV_EQ );
	case	DV_LT : return( DV_NO );
	case	DV_GT : return( DV_NO );
	case	DV_LE : return( DV_EQ );
	case	DV_GE : return( DV_EQ );
	case	DV_NE : return( DV_NO );
	default:	return( dv1 );	
	}
      break;
    case	BANJ_LT :
      switch( dv1 )
	{
	case	DV_ANY : return( DV_LT );
	case	DV_EQ : return( DV_NO );
	case	DV_LT : return( DV_LT );
	case	DV_GT : return( DV_NO );
	case	DV_LE : return( DV_LT );
	case	DV_GE : return( DV_NO );
	case	DV_NE : return( DV_LT );
	default:	return( dv1 );	
	}
      break;
    case	BANJ_LE :
      switch( dv1 )
	{
	case	DV_ANY : return( DV_LE );
	case	DV_EQ : return( DV_EQ );	
	case	DV_LT : return( DV_LT );	
	case	DV_GT : return( DV_NO );	
	case	DV_LE : return( DV_LE );	
	case	DV_GE : return( DV_EQ );	
	case	DV_NE : return( DV_LT );	
	default:	return( dv1 );		
	}
      break;
    case	BANJ_GT :
      switch( dv1 )
	{
	case	DV_ANY : return( DV_GT );		
	case	DV_EQ : return( DV_NO );	
	case	DV_LT : return( DV_NO );	
	case	DV_GT : return( DV_GT );	
	case	DV_LE : return( DV_NO );	
	case	DV_GE : return( DV_GT );	
	case	DV_NE : return( DV_GT );	
	default:	return( dv1 );	
	}
      break;
    case	BANJ_GE :
      switch( dv1 )
	{
	case	DV_ANY : return( DV_GE );	
	case	DV_EQ : return( DV_EQ );	
	case	DV_LT : return( DV_NO );	
	case	DV_GT : return( DV_GT );	
	case	DV_LE : return( DV_EQ );	
	case	DV_GE : return( DV_GE );	
	case	DV_NE : return( DV_GT );	
	default:	return( dv1 );	
	}
      break;
    case	BANJ_NE :
      switch( dv1 )
	{
	case	DV_ANY : return( DV_NE );	
	case	DV_EQ : return( DV_NO );	
	case	DV_LT : return( DV_LT );	
	case	DV_GT : return( DV_GT );	
	case	DV_LE : return( DV_LT );	
	case	DV_GE : return( DV_GT );	
	case	DV_NE : return( DV_NE );	
	default:	return( dv1 );	
	}
      break;
    case	BANJ_ANY :
      return( dv1 );
      break;
    }

  return( dv1 );		/* Default is current DV_.. information		*/

} /* end_dt_dv_meet */


/*************** SYMBOLIC BOUNDS TEST  nenad, 6/15/93 *****************/

/*static 	Boolean 	dt_check_bounds1();
static 	Boolean		dt_check_sym_bounds();*/

/*----------------------------------------------------------------

	dt_check_bounds()   -	apply symbolic bound check

	Returns:		false if no dep
	                        true  if possible dep
	nenad, 6/15/93				
*/
static Boolean
dt_check_bounds(int *subs_flag, int max_dim, CfgInstance cfgInst, 
                Boolean *sym_flag, Loop_list *loops1, Loop_list *loops2, 
                Subs_data *subs1, Subs_data *subs2, Dvect_data *dvect)
{
  int i;

  for (i = 0; i < max_dim; i++)
    {
      /*--------------------------------------------------------*/
      /* check that the subscript has not already been tested   */

      if (subs_flag[i] != SUBS_EMPTY || sym_flag[i])
	if (!dt_check_bounds1(cfgInst, loops1,loops2, subs1+i, subs2+i, dvect))
	  return false;
    }

  return true;
}

/*----------------------------------------------------------------

	dt_check_bounds1()   -	apply symbolic bound check

	Returns:		false if no dep
	                        true  if possible dep
	nenad, 6/15/93				
*/
static Boolean
dt_check_bounds1(CfgInstance cfgInst, Loop_list *loops1, Loop_list *loops2, 
                 Subs_data *subs1, Subs_data *subs2, Dvect_data *dvect)
{
  Values	 values;
  Loop_data    	*loop1;
  Loop_data	*loop2;
  int        	 level1;
  int        	 level2;
  int        	 i;


  values  =  cfgval_get_values(cfgInst);	/* get value table */

  /*--------------------------------------------------------------------*/
  /* check that subscripts are SIV/ZIV and that loops steps are both 1  */

  if (subs1->stype <= SUBS_SIV)
    level1 = subs1->stype;
  else if (subs1->stype >= SUBS_SYM_SIV_FIRST &&
	   subs1->stype <= SUBS_SYM_SIV_LAST)
    level1 = subs1->stype - SUBS_SYM_SIV_FIRST;
  else if (subs1->stype == SUBS_ZIV || subs1->stype == SUBS_SYM_ZIV)
    level1 = -1;
  else 
    return true;

  if (level1 >= 0)
    {
      loop1 = &(loops1->loops[level1]);
      if (loop1->step.type !=  Expr_constant || loop1->step.val  !=  1 ||
	  val_get_level(values, loop1->up_val) > 0 ||
	  val_get_level(values, loop1->lo_val) > 0)
	return true;
    }

  if (subs2->stype <= SUBS_SIV)
    level2 = subs2->stype;
  else if (subs2->stype >= SUBS_SYM_SIV_FIRST &&
	   subs2->stype <= SUBS_SYM_SIV_LAST)
    level2 = subs2->stype - SUBS_SYM_SIV_FIRST;
  else if (subs2->stype == SUBS_ZIV || subs2->stype == SUBS_SYM_ZIV)
    level2 = -1;
  else 
    return true;

  if (level2 >= 0)
    {
      loop2 = &(loops2->loops[level2]);
      if (loop2->step.type !=  Expr_constant || loop2->step.val  !=  1)
	return true;     

      /*------------------------------*/
      /* check if loops are different */

      if (level1 == -1 || loop1->loop_index != loop2->loop_index)
	{
	  if (val_get_level(values, loop2->up_val) > 0 ||
	      val_get_level(values, loop2->lo_val) > 0)
	    return true;
	}
      else  	/* we have only one loop */
	{
	  if (subs1->sym == subs2->sym)  /* output dependence on itself */
	    {
	      dvect->ltype[level2] = DV_EQ;
	      return true;
	    }
	}
    }

  return   dt_check_sym_bounds(values, 
			       subs1->constant - subs2->constant,
			       level1 >= 0 ? subs1->coeffs[level1] : 0, 
			       level2 >= 0 ? subs2->coeffs[level2] : 0,
			       val_binary(values, VAL_OP_MINUS, 
					  subs1->symbolic_constant,
					  subs2->symbolic_constant),
			       level1 >= 0 ? loop1->lo_val : VAL_ZERO, 
			       level1 >= 0 ? loop1->up_val : VAL_ZERO, 
			       level2 >= 0 ? loop2->lo_val : VAL_ZERO, 
			       level2 >= 0 ? loop2->up_val : VAL_ZERO);
}

/*----------------------------------------------------------------

  	dt_check_sym_bounds() - Apply symbolic bound test

	Returns:		false if no dep
	                        true  if possible dep
	nenad, 6/15/93				
*/	

static Boolean
dt_check_sym_bounds(Values values, int const_diff, int a_coeff, int b_coeff, 
                    ValNumber sym_diff, ValNumber loBound1, ValNumber upBound1, 
                    ValNumber loBound2, ValNumber upBound2)
{
  ValNumber 	a_val;
  ValNumber 	b_val;
  ValNumber 	val1;
  ValNumber 	val2;
  int		difference;


  a_val = val_const(values, a_coeff);
  b_val = val_const(values, b_coeff);

  if (a_coeff * b_coeff > 0)  	/* same sign, nonzero */
    {
      val1 = val_binary(values, VAL_OP_TIMES, a_val, upBound1);
      val2 = val_binary(values, VAL_OP_TIMES, b_val, loBound2);
      val1 = val_binary(values, VAL_OP_MINUS, val1, val2);
      val1 = val_binary(values, VAL_OP_PLUS, val1, sym_diff);
      if (val_is_const(values, val1))
	{
	  difference = val_get_const(values, val1) + const_diff;
	  if ((a_coeff > 0 && difference < 0) || 
	      (a_coeff < 0 && difference > 0))
	    return false;
	}
      
      val1 = val_binary(values, VAL_OP_TIMES, a_val, loBound1);
      val2 = val_binary(values, VAL_OP_TIMES, b_val, upBound2);
      val1 = val_binary(values, VAL_OP_MINUS, val1, val2);
      val1 = val_binary(values, VAL_OP_PLUS, val1, sym_diff);
      if (val_is_const(values, val1))
	{
	  difference = val_get_const(values, val1) + const_diff;
	  if ((a_coeff > 0 && difference > 0) || 
	      (a_coeff < 0 && difference < 0))
	    return false;
	}
    }
      
  else  	/* different sign, or one coeff is zero */
    {
      val1 = val_binary(values, VAL_OP_TIMES, a_val, upBound1);
      val2 = val_binary(values, VAL_OP_TIMES, b_val, upBound2);
      val1 = val_binary(values, VAL_OP_MINUS, val1, val2);
      val1 = val_binary(values, VAL_OP_PLUS, val1, sym_diff);
      if (val_is_const(values, val1))
	{
	  difference = val_get_const(values, val1) + const_diff;
	  if ((a_coeff > 0 && difference < 0) || 
	      (a_coeff < 0 && difference > 0) ||
	      (a_coeff == 0 && ((b_coeff > 0 && difference > 0) ||
				(b_coeff < 0 && difference < 0) ||
				(b_coeff == 0 && difference != 0))))
	    return false;
	}

      val1 = val_binary(values, VAL_OP_TIMES, a_val, loBound1);
      val2 = val_binary(values, VAL_OP_TIMES, b_val, loBound2);
      val1 = val_binary(values, VAL_OP_MINUS, val1, val2);
      val1 = val_binary(values, VAL_OP_PLUS, val1, sym_diff);
      if (val_is_const(values, val1))
	{
	  difference = val_get_const(values, val1) + const_diff;
	  if ((a_coeff > 0 && difference > 0) || 
	      (a_coeff < 0 && difference < 0) ||
	      (a_coeff == 0 && ((b_coeff > 0 && difference < 0) ||
				(b_coeff < 0 && difference > 0) ||
				(b_coeff == 0 && difference != 0))))
	    return false;
	}
    }

  return true;
}
	  
	  
/*----------------------------------------------------------------

	dt_lprove()		-	examine loop info to see whether 
						dependence test can be exact

	Returns:	true	if exact test possible
				false 	otherwise

*/

static Boolean
dt_lprove(Loop_list *loops)
{
	int i;

	for (i = 0; i < loops->level; i++)		/* look at each level	*/
	{
		if ((loops->loops[i].step.type != Expr_constant) ||	/* sym step		*/
			(loops->loops[i].up.val == MAXINT) ||		/* sym upper bound	*/
			(loops->loops[i].up.val == MININT) ||		/* sym upper bound	*/
			(loops->loops[i].lo.val == MAXINT) ||		/* sym lower bound	*/
			(loops->loops[i].lo.val == MININT))		/* sym lower bound	*/

			return false;
	}

	return true;
}

/*----------------------------------------------------------------

	dt_reverse()	-	reverse dep info for reversed loops

	Also handles symbolic bounds that may hide reversed loops.

*/

static void
dt_reverse(Loop_list *loops, Dvect_data *dvect, int clevel)
{
	int i;
	int temp;

	/*------------------------------------------*/
	/* find loops that have been reversed OR 	*/
	/* have symbolic bounds/steps				*/

	for (i = 0; i < clevel; i++)
	{
		/* if step & either bound is symbolic		*/
		/* we can't even be sure of the direction	*/

		/* take both possible directions of element if one is present */

		if ((loops->loops[i].step.type != Expr_constant) && 
			((loops->loops[i].lo.val == MININT) ||
			 (loops->loops[i].up.val == MAXINT)))	
		{
			switch (dvect->ltype[i])
			{
			case DV_LT:
			case DV_GT:
			case DV_DIS:
				dvect->ltype[i] = DV_NE;
				break;

			case DV_LE:
			case DV_GE:
				dvect->ltype[i] = DV_ANY;
				break;

			case DV_LN:
				/* check whether "=" direction is possible */

				temp = dvect->c3[i] / (dvect->c1[i] - dvect->c2[i]);
				if ((temp * dvect->c1[i]) == 
					((temp * dvect->c2[i]) + dvect->c2[i]))
					dvect->ltype[i] = DV_ANY;
				else
					dvect->ltype[i] = DV_NE;
				break;

			case DV_PT:
				/* check whether "=" direction holds for point */

				if (dvect->c1[i] != dvect->c2[i])
					dvect->ltype[i] = DV_NE;
				break;
			}
		}

		else if (loops->loops[i].rev)		/* reversed loops	*/
		{
			/* if loop is reversed, switch direction of that element */

			switch (dvect->ltype[i])
			{
			case DV_LT:
				dvect->ltype[i] = DV_GT;
				break;

			case DV_GT:
				dvect->ltype[i] = DV_LT;
				break;

			case DV_LE:
				dvect->ltype[i] = DV_GE;
				break;

			case DV_GE:
				dvect->ltype[i] = DV_LE;
				break;

			case DV_DIS:
				dvect->c1[i] = -dvect->c1[i];
				break;

			case DV_LN:			/* constraint = line */

				/* reflect across line x=y	*/

				temp = dvect->c1[i];	
				dvect->c1[i] = dvect->c2[i];
				dvect->c2[i] = temp;
				dvect->c3[i] = -dvect->c3[i];
				break;

			case DV_PT:			/* constraint = point */

				/* reflect across line x=y	*/

				temp = dvect->c1[i];	
				dvect->c1[i] = dvect->c2[i];
				dvect->c2[i] = temp;
				break;
			}
		}
	}
}


/*----------------------------------------------------------------

	dt_convert()	-	convert info in to Dvect_data to Dt_info

	Returns:	pointer to new Dt_info 

	Warning: Only initializes type, data, str fields!

*/

static void
dt_convert(DT_info *dt, DG_Edge *Edge, Dvect_data *dvect, int max_level, 
           int clevel, Boolean all_eq, Boolean proven)
{
	char		cbuf[128];	/* use 128 for now		*/
	char		tbuf[128];
	int         i;
	Boolean		eq;
	Boolean		all_dis;
	
	/*--------------------------------------*/
	/* build string for dependence vector	*/

	/* save char into string, code/distance into data buf	*/

	cbuf[0] = '(';
	cbuf[1] = 0;

	eq = true;			/* init flags	*/
	all_dis = true;

	for (i = 0; i < max_level; i++)
	{
		/*--------------------------------------------------*/
		/* only loop indep dep possible if not shared level	*/

		/* strictly speaking, no dependence vector entry	*/

		if (i >= clevel)
		{
			Edge->dt_data[i] = 0;
			continue;
		}

		/*--------------*/
		/* shared level	*/

		switch (dvect->ltype[i])
		{
		case DV_LT:
			strcat(cbuf,"<");
			Edge->dt_data[i] = DDATA_LT;
			all_dis = false;
			eq = false;
			break;

		case DV_GT:
			strcat(cbuf,">");
			Edge->dt_data[i] = DDATA_GT;
			all_dis = false;
			eq = false;
			break;

		case DV_EQ:
			strcat(cbuf,"=");
			Edge->dt_data[i] = 0;
			break;

		case DV_LE:
			strcat(cbuf,"<=");
			Edge->dt_data[i] = DDATA_LE;
			all_dis = false;
			break;

		case DV_GE:
			strcat(cbuf,">=");
			Edge->dt_data[i] = DDATA_GE;
			all_dis = false;
			break;

		case DV_NE:
			strcat(cbuf,"<>");
			Edge->dt_data[i] = DDATA_NE;
			all_dis = false;
			eq = false;
			break;

		case DV_DIS:
			sprintf(tbuf,"%d", dvect->c1[i]);
			strcat(cbuf, tbuf);

			Edge->dt_data[i] = dvect->c1[i];		/* save actual distance	*/

			if (dvect->c1[i] < DDATA_BASE)	/* oops, kludge discovered :-(	*/
			{
				die_with_message("dt_convert(): excessive negative distance");
			}

			if (dvect->c1[i])
				eq = false;

			break;

		case DV_LN:	/* constraint = line, treat as DV_ANY */

			/*
			*	sprintf(tbuf,"[%s = ", pstr(dvect->c1[i], 0, 'x'));
			*	strcat(cbuf, tbuf);
			*	sprintf(tbuf,"%s]", pstr(dvect->c2[i], dvect->c3[i], 'y'));
			*	strcat(cbuf, tbuf);
			*/

		case DV_ANY:
			strcat(cbuf,"*");
			Edge->dt_data[i] = DDATA_ANY;
			all_dis = false;
			break;

		case DV_PT:	/* constraint = point		 */

			/* don't bother displaying actual dep point for now */

			/* sprintf(tbuf,"[%d,%d]", dvect->c1[i], dvect->c2[i]); */
			/* strcat(cbuf, tbuf);                                  */

			all_dis = false;

			/* convert point to direction for data buffer	*/

			if (dvect->c1[i] == dvect->c2[i])
			{
				strcat(cbuf,"=");
				Edge->dt_data[i] = 0;
			}
			else if (dvect->c1[i] <= dvect->c2[i])
			{
				strcat(cbuf,"<");
				Edge->dt_data[i] = DDATA_LE;
				eq = false;
			}
			else 	/* if (dvect->c1[i] >= dvect->c2[i])	*/
			{
				strcat(cbuf,">");
				Edge->dt_data[i] = DDATA_GT;
				eq = false;
			}
			break;

		case DV_NO:
		default:
			/* printf("dt_convert(): unknown DV type found\n"); */
			strcat(cbuf,"?");
			Edge->dt_data[i] = DDATA_ERROR;
			all_dis = false;
			eq = false;
			break;
		}

		if (i < clevel - 1)
			strcat(cbuf,",");
	}

	strcat(cbuf,")");

	/*--------------------------------------*/
	/* mark dependences proven exactly		*/

	if (proven)
		strcat(cbuf," !");

	/*--------------------------------------*/
	/* mark consistent dependences	(mpal:920207)	*/
#ifdef	USE_SIMPLIFIER
	 dt_consistent_str( Edge, cbuf );
#endif	/* USE_SIMPLIFIER */

	/*--------------------------------------*/
	/* alloc mem for new str, add to dinfo	*/

	Edge->dt_str = dt_ssave(cbuf, dt);

	/*--------------------------------------*/
	/* encode dependence vector type		*/

	Edge->dt_type = 0;

	gen_put_dt_LVL(Edge, max_level);
	gen_put_dt_CLVL(Edge, clevel);

	if (!proven)
		Edge->dt_type |= DT_NOPROVE; 
	if (all_eq)
		Edge->dt_type |= DT_ALL_EQ;
	if (eq)
		Edge->dt_type |= DT_EQ;
	if (all_dis)
		Edge->dt_type |= DT_DIS;

}



/*------------------------------*/
/* Utility Routines				 */
/*------------------------------*/

/*----------------------------------------------------------------

	solve()		Solve for the intersection of two lines

	Parameters:		a1,b1,c1,a2,b2,c2: describes two lines:

						a1X = b1Y + c1
						a2X = b2Y + c2

					x,y: pointers to location to store intersection

	Returns:		pointer to array of Booleans

		answer[SOLVE_DEP] 		true if equations are linearly dependent
								false otherwise

		answer[SOLVE_INT] 		true if intersect at integer point
								false otherwise

		answer[SOLVE_PAR] 		true if lines are parallel
								false otherwise
*/

static Boolean *
solve(int a1, int b1, int c1, int a2, int b2, int c2, int *x, int *y)
{
	static Boolean  answer[3];
	int             bottom;
	int xx;
	int yy;

	answer[SOLVE_DEP] = false;		/* defaults	*/
	answer[SOLVE_INT] = true;
	answer[SOLVE_PAR] = false;

	/*------------------------------*/
	/* check for linear dependence	*/

	if (!(bottom = (a2 * b1 - b2 * a1)))
	{
		/* slopes are same for both lines, check distance between them	*/

		if ((a1 * c2) != (c1 * a2))	
			answer[SOLVE_PAR] = true;		/* lines are parallel	 */
		else						
			answer[SOLVE_DEP] = true;		/* lines are dependent	 */

		return answer;
	}

	/*----------------------------------*/
	/* solve for point of intersection	*/

	if (!b1)		/* avoid division by 0	 */
	{
		xx = c1 / a1;
		yy = ((xx * a2) - c2) / b2;
	}
	else
	{
		xx = (c2 * b1 - b2 * c1) / bottom;
		yy = ((xx * a1) - c1) / b1;
	}

	/*------------------------------------------------------*/
	/* check that intersection actually is on both lines	*/
	/* otherwise truncation occurred, not integer point		*/

	if ((a1 * xx != b1 * yy + c1) || (a2 * xx != b2 * yy + c2))
		answer[SOLVE_INT] = false;

	*x = xx;		/* set coordinates for intersection	*/
	*y = yy;

	return answer;
}


#ifdef	USE_SIMPLIFIER


/*----------------------------------------------------------------

	init_nesting_level_array  --  construct an array containing pointers
		to the names of each induction variable in loop nest.

	Parameter:	nest_level_type defined in simple.h from simplifier.

	------------------------------------------ Modifications -

*/
static void init_nesting_level_array(nest_level_type nesting_level_ptr, 
                                     Loop_list *loops1, Loop_list *loops2, 
                                     int clevel)
     /* nest_level_type	nesting_level_ptr;	char *'s to ivars in nest */
     /* Loop_list		*loops1;	src	*/
     /* Loop_list		*loops2;	sink	*/
     /* int		         clevel;	common loop level	*/
{
  int	level;

  for( level=0; level<clevel; level++ )
    {
      (nesting_level_ptr)[level] = loops1->loops[level].ivar;
    }
  (nesting_level_ptr)[level] = NULL;

  return;
} /* end_init_nesting_level_array */




/*----------------------------------------------------------------

	dt_get_symDescriptor  -- obtains the symbol table descriptor for the
		simplifier 

	Parameter:	

	------------------------------------------ Modifications -

*/
SymDescriptor	dt_get_symDescriptor(AST_INDEX expr1, AST_INDEX expr2, 
                                     PedInfo ped)
{
  AST_INDEX	 scope;
  char		*prog_name;
  FortTree	 ft;
  SymDescriptor	 symD;
  
  /* "scope" should be PROGRAM, FUNCTION, SUBROUTINE, BLOCK_DATA, ENTRY	*/
  /* fort/aphelper.c	fort-include/aphelper.h	*/
  scope = expr1;
  while ( scope != AST_NIL && ! ftx_is_scope(scope) )
    scope = out(scope);
  
  prog_name = gen_get_text( get_name_in_entry(scope) );

  ed_GetTree( ped_cp_get_FortEditor(ped), &ft);
  ft_AstSelect(ft);
  symD	= ft_SymGetTable( ft, prog_name );

  return	symD;

} /* end_dt_get_symDescriptor */

#endif	/* USE_SIMPLIFIER */


/*----------------------------------------------------------------

	dt_consistent_test()	"quick" screening test for consistency,
				will be followed by test using simplifier.
*/

static void
dt_consistent_test(DG_Edge *Edge, int *subs_flag, int max_dim, 
		   Subs_list *subs1, Subs_list *subs2, Loop_list *loops1, 
                   Loop_list *loops2, int min_level, int clevel)
{
	nest_level_type	 nesting_level_ptr;
	char		*nesting_level_array[10];

	int		 dim, level;	/* dimension of array looking at	*/

	Edge->consistent	= consistent_SIV;	/* hopefull thought */
	Edge->symbolic		= false;

	/* only want nicely nested references for now		*/

	if( clevel != min_level )
	  {
	    Edge->consistent	= inconsistent;
	    return;
	  }
	
	for(dim = 0; dim < max_dim; dim++)
	  {
	    /* duplicate code in dt_part for now, 
	       may be possible to merge later for efficiency */
	    /*-------------------------------------------------------	*/
	    /* MIV subscript pair if either subs is already MIV		 */
	    
	    if ((subs1->subs[dim].stype == SUBS_MIV) ||
		(subs2->subs[dim].stype == SUBS_MIV))
	      {
		Edge->consistent = consistent_MIV;
	      }

	    /*-------------------------------------------------------	*/
	    /* Check for different SIV subscript variables		*/

	    if( subs_flag[dim] == SUBS_RDIV )
	      {
		Edge->consistent = inconsistent;
		break;
	      }

	    /*-------------------------------------------------------	*/
	    /* check if symbolics found in either subscript		 */
	    
	    if ((subs1->subs[dim].stype >= SUBS_SYM) ||
		(subs2->subs[dim].stype >= SUBS_SYM))
	      {
		int		sub;
		SymDescriptor	sym_t;
		AST_INDEX	expr;
		AST_INDEX	expr1;
		AST_INDEX	expr2;
		
		Edge->symbolic = true;		/* Set flag that this is symbolic */
#ifdef	USE_SIMPLIFIER
		/* Build the combined expression to simplify	*/
		expr1	= tree_out( Edge->src );
		expr1	= gen_SUBSCRIPT_get_rvalue_LIST( expr1 );
		expr1	= list_first( expr1 );		/* fort:astlist.c */
		expr2	= tree_out( Edge->sink );
		expr2	= gen_SUBSCRIPT_get_rvalue_LIST( expr2 );
		expr2	= list_first( expr2 );		/* fort:astlist.c */
		for( sub=0; sub<dim; sub++ )
		  {
		    expr1	= list_next( expr1 );
		    expr2	= list_next( expr2 );
		  }
		
		expr	= gen_BINARY_MINUS( tree_copy(expr1), tree_copy(expr2));
		
		/* Get the info needed by the simplifier	*/
	      
		nesting_level_ptr	= (nest_level_type)nesting_level_array;
		init_nesting_level_array( nesting_level_ptr, loops1, loops2, clevel );
		
		sym_t	= dt_get_symDescriptor( expr1, expr2, ped );

		/* Call the simplifier	*/

		simplify( expr, nesting_level_array, sym_t);

		/* Check resulting expression to see if it is consistent */

		tree_free( expr );
#endif	/* USE_SIMPLIFIER */
	      }
	    else
	      {
		/*-------------------------------------------------------	*/
		/* Check for different constant coefficiants		*/
		
		for( level = 0; level < clevel; level++ )
		  {
		    if( subs1->subs[dim].coeffs[level] != subs2->subs[dim].coeffs[level] )
		      {
			Edge->consistent = inconsistent;
			break;
		      }
		  }
	      }
	  }

} /* end_dt_consistent_test */

/*----------------------------------------------------------------

	dt_consistent_str()	construct label for output of consistent info
				FOR DEBUGGING PURPOSES !!??
*/

void dt_consistent_str(DG_Edge *Edge, char *cbuf)
	/* char	*cbuf;			use 128 for now	*/
{
	if( Edge->symbolic )
	  strcat( cbuf," sym" );

	switch( Edge->consistent )
	  {
	  case inconsistent:
	    strcat(cbuf," IN");
	    break;
	  case consistent_SIV:
	    strcat(cbuf," SIV");
	    break;
	  case consistent_MIV:
	    strcat(cbuf," MIV");
	    break;
	  default:
	    break;
	  }

} /* end_dt_consistent_str */



/*----------------------------------------------------------------

	pstr()		Return string for linear function of one var

*/

static char *
pstr(int a, int b, char c)
{
	static char     buf[80];

	if (a == 1)
	{
		if (b > 0)
			sprintf(buf, "%c + %d", c, b);
		else if (b < 0)
			sprintf(buf, "%c - %d", c, -b);
		else
			sprintf(buf, "%c", c);
	}
	else if (a)
	{
		if (b > 0)
			sprintf(buf, "%d*%c + %d", a, c, b);
		else if (b < 0)
			sprintf(buf, "%d*%c - %d", a, c, -b);
		else
			sprintf(buf, "%d*%c", a, c);
	}
	else
		sprintf(buf, "%d", b);

	return (buf);
}

/* eof */







