/* $Id: rsd.C,v 1.1 1997/06/25 15:08:54 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*-----------------------------------------------------------------------

	rsd.c		Regular Section Descriptors Module

	Currently, severely restricted

	History
	~~~~~~~
	25 May 90  roth    Created
	 8 Jun 91  tseng   Modifed to use DT_REF info

*/

#include <stdlib.h>
#include <string.h>

#include <include/bstring.h>

#include <libs/frontEnd/include/walk.h>

#include <libs/moduleAnalysis/dependence/dependenceTest/dt_info.h>
#include <libs/moduleAnalysis/dependence/utilities/side_info.h>

#include <libs/moduleAnalysis/dependence/dependenceTest/rsd.h>


typedef	struct	struct_RsdWalkParams
{
	DT_info		*dt;
	SideInfo	*infoPtr;
} RsdWalkParams;

/*---------------*/
/* global decls */

void rsd_build_section();
Rsd_section* rsd_ref_section();
Rsd_section* rsd_merge();
Boolean rsd_intersecting();
void rsd_build_vector();
void rsd_vector_init();

/*---------------*/
/* extern decls */

EXTERN(int, gcd1, (int a, int b, int *x, int *y));

/*---------------*/
/* forward decls */

STATIC(void, rsd_convert_expr_to_range,(Rsd_data *sdata, Rsd_data *result, Loop_list
                                        *loop_nest));
STATIC(void, rsd_make_section_bottom,(Rsd_data *sdata));
STATIC(void, rsd_merge_const_const,(Rsd_data *result, Rsd_data *sub1, Rsd_data *sub2));
STATIC(void, rsd_merge_const_range,(Rsd_data *result, Rsd_data *sub1, Rsd_data *sub2));
STATIC(void, rsd_merge_const_expr,(Rsd_data *result, Rsd_data *sub1, Rsd_data *sub2,
                                   Loop_list *loop_nest));
STATIC(void, rsd_merge_expr_expr,(Rsd_data *result, Rsd_data *sub1, Rsd_data *sub2, 
                                  Loop_list *loop_nest1, Loop_list *loop_nest2));
STATIC(void, rsd_merge_expr_range,(Rsd_data *result, Rsd_data *sub1, Rsd_data *sub2,
                                   Loop_list *loop_nest));
STATIC(void, rsd_merge_range_range,(Rsd_data *result, Rsd_data *sub1, Rsd_data *sub2));
STATIC(void, rsd_copy_section,(Rsd_data *to, Rsd_data *from));
STATIC(int, vector_init,(AST_INDEX stmt, int lvl, RsdWalkParams *rsdWalkParams));


/*------------------*/
/* global functions */
/*------------------*/

/*----------------------------------------------------------------------

    rsd_build_section()  

	Creates RSD from pre-parsed loop & subscript information

*/

void 
rsd_build_section(Rsd_section *r_section, Subs_list *subscript, Loop_list *loop_nest, 
                  int level)
{
	int i, coeff, stype;

	bzero((char *)r_section, sizeof(Rsd_section));
	r_section->dims = subscript->dims;
	r_section->loop_nest = loop_nest;

	level--;		/* internal level goes from 0..MAXDIM-1	*/

	/*---------------------------------------*/
	/* loop through each subscript dimension */

	for (i = 0; i < subscript->dims; i++)
	{
		/*--------------------------------------*/
		/* no index variables */

		stype = subscript->subs[i].stype;	/* type of subscript */

		if (stype == SUBS_ZIV)
		{
			r_section->subs[i].type = RSD_CONSTANT;
			r_section->subs[i].constant = subscript->subs[i].constant;
		}

		/*--------------------------------------*/
		/* single index variable expression */

		else if (stype <= SUBS_SIV)
		{
			r_section->subs[i].coeff = subscript->subs[i].coeffs[stype];
			r_section->subs[i].ivar = stype;

			if (stype < level)	/* outside level desired */
			{
				r_section->subs[i].type = RSD_EXPR;
				r_section->subs[i].constant = subscript->subs[i].constant;
			}
			else	/* inside loop nest, build range from expr */
			{
				if ((loop_nest->loops[stype].lo.val == MININT) &&
					(loop_nest->loops[stype].up.val == MAXINT))
				{
					rsd_make_section_bottom(r_section->subs + i);
				}
				else
				{
					r_section->subs[i].type = RSD_RANGE;
					coeff = subscript->subs[i].coeffs[stype];

					/* get lower bound */

					if (loop_nest->loops[stype].lo.val == MININT)
					{
						r_section->subs[i].lo_b = MININT;
					}
					else
					{
						r_section->subs[i].lo_b = 
							(coeff * loop_nest->loops[stype].lo.val) + 
							subscript->subs[i].constant;
					}

					/* get upper bound */

					if (loop_nest->loops[stype].up.val == MAXINT)
					{
						r_section->subs[i].up_b = MAXINT;
					}
					else
					{
						r_section->subs[i].up_b = 
							(coeff * loop_nest->loops[stype].up.val) + 
							subscript->subs[i].constant;
					}

					/* get step */

					r_section->subs[i].step = 
						coeff * loop_nest->loops[stype].step.val;

					/* get begin */

					r_section->subs[i].begin = r_section->subs[i].lo_b;

				}
			}		
		}

		/*--------------------------------------*/
		/* complex or symbolic expression */

		else
		{
			rsd_make_section_bottom(r_section->subs + i);
		}
	}
}


/*----------------------------------------------------------------------

    rsd_ref_section()  
       
    Given a reference and a loop, create the RSD for the reference.
    All subscript references to induction variables for loops at or
    inside the level of the given loop will be made into ranges; all
    those outside that level will be left as expressions. if a
    subscript uses symbolics or is not of the proper form, its section
    is set to bottom. Similarly, if a do-loop uses symbolics, any
    subscript using that loop's induction variable will have its
    section set to bottom.

    Returns NULL if the section descriptor cannot be built.

*/

Rsd_section* rsd_ref_section (DT_info *dt, SideInfo *infoPtr, AST_INDEX ref, 
                              AST_INDEX loop)
{
	Rsd_section* r_section;
	Subs_list *subscript;

	subscript = (Subs_list *) dg_get_info(infoPtr, ref, type_ref);
	r_section = dt_alloc_rsd( dt );

	rsd_build_section(r_section, subscript, 
		subscript->loop_nest, loop_level(loop));

	return r_section;
}


/*----------------------------------------------------------------------

    rsd_merge()  
       
    Given two section handles, this function will return a handle
    for a section containing all the elements in either input
    section (the UNION of the two). The merging is performed
    subscript-by-subscript, and results in the smallest section
    in the given regular section framework.

    The type of the resultant section is dependent upon the
    type of the sections being merged. The following table gives
    the desired mapping:

              bottom       constant        range         expr
             +----------+--------------+-------------+------------+
     bottom  | bottom   |  bottom      |   bottom    |  bottom    |
             +----------+--------------+-------------+------------+
     constant| bottom   | const/range  |   range     |  range     |
             +----------+--------------+-------------+------------+
     range   | bottom   |  range       |   range     |  range     |
             +----------+--------------+-------------+------------+
     expr    | bottom   |  range       |   range     | expr/range |
             +----------+--------------+-------------+------------+

    Unless ranges are identical, any tags will be dropped.

    Returns NULL if the section descriptors cannot be merged.

*/

Rsd_section* 
rsd_merge(Rsd_section *result, Rsd_section *rsd1, Rsd_section *rsd2)
{

  int             i;
  int             dims;       /*dimensions                */
  Rsd_data  *rsection;        /*single subscript of result*/
  Rsd_data  *section1;        /*single subscript of rsd1  */
  Rsd_data  *section2;        /*single subscript of rsd2  */

  if (rsd1->dims != rsd2->dims)
    return NULL;
  dims = rsd1->dims;

  /*if either rsd has has a subscript with no data, return NULL */
  for (i = 0; i < dims; i++)
    if (rsd1->subs[i].type == RSD_NO_DATA || 
	rsd2->subs[i].type == RSD_NO_DATA )
      return NULL;

  /*create rsd for result*/
  result->dims = dims;
  result->loop_nest = rsd1->loop_nest;	/* doesn't matter which */
  bzero((char *)result->subs, MAXDIM*sizeof(Rsd_data));

  /*perform merge; done subscript-by-subscript*/
  for (i = 0; i < dims; i++)
    {
      rsection = &result->subs[i];          /*get subscripts for rsd's*/
      section1 = &rsd1->subs[i];
      section2 = &rsd2->subs[i];


      /*if either section1 or section2 is bottom, make result be bottom*/
      if (section1->type == RSD_BOTTOM || section2->type == RSD_BOTTOM)
	{
	  rsd_make_section_bottom(rsection);
	}
      else
	{     
	  /*perform merge depending upon rsd_type*/
	  switch (section1->type)
	    {
	    /*----------------------------------------------*/
	    case RSD_CONSTANT:
	      switch (section2->type)
		{
		case RSD_CONSTANT:
		  rsd_merge_const_const(rsection,section1,section2);
		  break;		  
		case RSD_EXPR:
		case RSD_EXPR_RANGE:
		  rsd_merge_const_expr(rsection,section1,section2,rsd2->loop_nest);
		  break;
		case RSD_RANGE:
		  rsd_merge_const_range(rsection,section1,section2);
		  break;
		default:
		  printf("rsd_merge(): invalid RSD type\n");		  
		  return NULL;
		}
	      break;
	      
	    /*----------------------------------------------*/
	    case RSD_EXPR:
		case RSD_EXPR_RANGE:
	      switch (section2->type)
		{
		case RSD_CONSTANT:
		  rsd_merge_const_expr(rsection,section2,section1,rsd1->loop_nest);
		  break;
		case RSD_EXPR:
		case RSD_EXPR_RANGE:
		  rsd_merge_expr_expr(rsection,section1,
			section2,rsd1->loop_nest,rsd2->loop_nest);
		  break;
		case RSD_RANGE:
		  rsd_merge_expr_range(rsection,section1,section2,rsd1->loop_nest);
		  break;
		default:
		  printf("rsd_merge(): invalid RSD type\n");		  
		  return NULL;
		}
	      break;
	      
	    /*----------------------------------------------*/
	    case RSD_RANGE:
	      switch (section2->type)
		{
		case RSD_CONSTANT:
		  rsd_merge_const_range(rsection,section2,section1);
		  break;
		case RSD_EXPR:
		case RSD_EXPR_RANGE:
		  rsd_merge_expr_range(rsection,section2,section1,rsd2->loop_nest);
		  break;
		case RSD_RANGE:
		  rsd_merge_range_range(rsection,section1,section2);
		  break;
		default:
		  printf("rsd_merge(): invalid RSD type\n");		  
		  return NULL;
		}
	      break;
	      
	    /*----------------------------------------------*/
	    default:
	      printf("rsd_merge(): invalid RSD type\n");		  
	      return NULL;

	    }
	}
    }

  return result;
}


/*----------------------------------------------------------------------

    rsd_intersecting()  

    Conservative estimate of whether RSDs intersect.  

    Assume that RSD_EXPR & RSD_EXPR_RANGE basically correspond 
    to outer loops and can be ignored for this test.

    Returns: True   if intersection is possible
             False  otherwise
*/

Boolean 
rsd_intersecting(Rsd_section *rsd1, Rsd_section *rsd2)
{
  int i, coeff, stype;

  for (i = 0; i < rsd1->dims; i++)
  {
    switch (rsd1->subs[i].type)
    {
    case RSD_NO_DATA:		/* unknown RSD      */
	case RSD_BOTTOM:		/* bottom           */
      return true;

	case RSD_EXPR:			/* single index var */
	case RSD_EXPR_RANGE:	/* index var range  */
      break;

	case RSD_CONSTANT:		/* single constant  */

      if (rsd2->subs[i].type == RSD_CONSTANT)
      {
        if (rsd1->subs[i].constant == rsd2->subs[i].constant)
          return true;
      }
      else if (rsd2->subs[i].type == RSD_RANGE)
      {
        if ((rsd1->subs[i].constant >= rsd2->subs[i].lo_b) && 
            (rsd1->subs[i].constant <= rsd2->subs[i].up_b) &&
            ((rsd2->subs[i].step == 1)  ||
             (((rsd1->subs[i].constant - rsd2->subs[i].begin) % 
                rsd2->subs[i].step) == 0)))
          return true;
      }

      break;

	case RSD_RANGE:			/* constant range   */
      if (rsd2->subs[i].type == RSD_CONSTANT)
      {
        if ((rsd2->subs[i].constant >= rsd1->subs[i].lo_b) && 
            (rsd2->subs[i].constant <= rsd1->subs[i].up_b) &&
            ((rsd1->subs[i].step == 1)  ||
             (((rsd2->subs[i].constant - rsd1->subs[i].begin) % 
                rsd1->subs[i].step) == 0)))
          return true;
      }
      else if (rsd2->subs[i].type == RSD_RANGE)
      {
        if ((rsd2->subs[i].lo_b <= rsd1->subs[i].up_b) ||
            (rsd1->subs[i].lo_b <= rsd2->subs[i].up_b))
          return true;
      }

      break;

    }

    switch (rsd2->subs[i].type)
    {
    case RSD_NO_DATA:		/* unknown RSD      */
	case RSD_BOTTOM:		/* bottom           */
      return true;
    }

  }

  return false;
}


/*----------------------------------------------------------------------

    rsd_build_vector()  
       
    Given a statement, build RSDs describing its references for
    all enclosing loops, then store in the type_ref field of the
    side array.  Just builds lhs RSDs for now.

*/

void
rsd_build_vector(DT_info *dt, SideInfo *infoPtr, AST_INDEX stmt)
{
	AST_INDEX lhs;
	Rsd_section *rsd;
	Rsd_vector *rvec;
	Subs_list *subs;
	int i;

    lhs = gen_ASSIGNMENT_get_lvalue(stmt);

	if (!is_subscript(lhs))		/* build vector only for arrays */
		return;

	subs = (Subs_list *) dg_get_info( infoPtr, lhs, type_ref);

    rvec = dt_alloc_rsd_vector(dt);

    for (i = loop_level(stmt); i >= 0; i--)
    {
		rsd = dt_alloc_rsd(dt);
		rsd_build_section(rsd, subs, subs->loop_nest, i+1);
		rvec->lhs[i] = rsd;
    }

	dg_put_info( infoPtr, stmt, type_ref, (Generic)rvec);
}

/*----------------------------------------------------------------------

    rsd_vector_init()  
       
    Builds Rsd_vectors for all statements in tree.

*/

void
rsd_vector_init(DT_info *dt_info, SideInfo *infoPtr, AST_INDEX root)
{
	RsdWalkParams	rsdWalkParams;

	rsdWalkParams.dt	= dt_info;
	rsdWalkParams.infoPtr	= infoPtr;
	walk_statements(root, LEVEL1, (WK_STMT_CLBACK)vector_init, NULL, (Generic)&rsdWalkParams);
}


/*-----------------*/
/* local functions */
/*-----------------*/


/*----------------------------------------------------------------------

    vector_init()  Helper function for rsd_vector_init()
       
*/

static int
vector_init(AST_INDEX stmt, int lvl, RsdWalkParams *rsdWalkParams)
{
	if (is_assignment(stmt))
		rsd_build_vector( rsdWalkParams->dt, rsdWalkParams->infoPtr, stmt);

	return WALK_CONTINUE;
}

/*-----------------------------------------------------------------------

    rsd_make_section_bottom()

    Change the rsd to bottom.

*/

static void 
rsd_make_section_bottom (Rsd_data *sdata)
{
  sdata->type = RSD_BOTTOM;
  sdata->lo_b = MININT;
  sdata->up_b = MAXINT;
  sdata->step = 1;
  sdata->begin = MININT;
}


/*----------------------------------------------------------------------

    rsd_merge_const_const()  
       
    Merge two constant sections. If both are the same constant,
    return a section with the same constant. If different, return
    a range with lo_b equal to the smaller constant and up_b
    equal the larger and step equal the difference; the begin
    field will be set equal to lo_b.

*/

static void rsd_merge_const_const (Rsd_data *result, Rsd_data *sub1, Rsd_data *sub2)
{

  int   const1, const2;

  const1 = sub1->constant;
  const2 = sub2->constant;

  if (const1 == const2)
    {
      result->type = RSD_CONSTANT;
      result->constant = const1;
    }

  else if (const1 < const2)
    {
      result->type = RSD_RANGE;
      result->lo_b = const1;
      result->up_b = const2;
      result->step = const2 - const1;
      result->begin = const1;
    }

  else /* (const1 > const2) */
    {
      result->type = RSD_RANGE;
      result->lo_b = const2;
      result->up_b = const1;
      result->step = const1 - const2;
      result->begin = const2;
    }
}



/*----------------------------------------------------------------------

    rsd_merge_const_expr()  
       
    Merge a constant section with an expression section. 
    First convert the expression section into a range section, then
    perform a const_range merge.

*/
static void 
rsd_merge_const_expr (Rsd_data *result, Rsd_data *sub1, Rsd_data *sub2, 
                      Loop_list *loop_nest)
{
  Rsd_data  temp;     /*temp tp hold converted expression*/

  rsd_convert_expr_to_range(sub2, &temp, loop_nest);  /*convert expression to range*/

  /*if  temp is not a range, the expression was not      */
  /*converted for some reason. Return result as NO DATA. */

  if (temp.type != RSD_RANGE)
    {
      result->type = RSD_NO_DATA;
    }
  else /*merge constant & range*/
    {
      rsd_merge_const_range(result,sub1,&temp);
    }
}


/*----------------------------------------------------------------------

    rsd_merge_const_range()  

    Merge a constant section with one that is a range.
    The result is always an untagged range.
    If the range includes the constant, the range is sufficient.
    If it does not include the constant, there are two cases:
    1) the constant is within lo_b..up_b, then alter the step
    to also include the constant.
    2) the constant is outside lo_b..up_b, then the range must
    be extended, and the stride possibly altered.

    The begin field will be set equal to the begin field of the range.
*/

static void rsd_merge_const_range (Rsd_data *result, Rsd_data *sub1, Rsd_data *sub2)
{

  int      constant,
           lo_b,
           up_b,
           step,
           tmp1,
           tmp2;

  constant = sub1->constant;
  lo_b = sub2->lo_b;
  up_b = sub2->up_b;
  step = sub2->step;

  /*result will always be some extension of the range sub2*/
  rsd_copy_section(result,sub2);    
  result->coeff = 0;
  result->constant = 0;

  
  if (constant >= lo_b && constant <= up_b)   /* is constant within lo_b .. up_b? */
    {
      if (constant == lo_b || constant == up_b || step == 1)  /*easy case*/
	{
	  return;    /*range of sub2 sufficies*/
	}
      else /*change step to include const*/
	{
	  result->step = gcd1(constant - lo_b, step, &tmp1, &tmp2);
	  return;
	}
    }

  else   /*const is outside lo_b..up_b; extend range and alter step*/
    {
      if (constant < lo_b)
	{
	  result->lo_b = constant;
	  result->step = gcd1(lo_b - constant, step, &tmp1, &tmp2);
	}
      else /*const > up_b*/
	{
	  result->up_b = constant;
	  result->step = gcd1(constant - up_b, step, &tmp1, &tmp2);
	}
    }
}


/*----------------------------------------------------------------------

    rsd_merge_expr_expr()  

    Merge two expression sections. If the two expressions are identical,
    the merge is the same expression. If they are not identical, convert
    each expression to a range then merge the two ranges.

*/

static void rsd_merge_expr_expr (Rsd_data *result, Rsd_data *sub1, Rsd_data *sub2, 
                                 Loop_list *loop_nest1, Loop_list *loop_nest2)
{
	Rsd_data  temp1, temp2;     /*temps to hold converted expressions*/

	/*----------------------------------------------------*/
	/* first check whether same index var with same coeff */

	/* conservatively also require same loop nest */

	if ((sub1->ivar == sub2->ivar) && (sub1->coeff == sub2->coeff) &&
		(loop_nest1 == loop_nest2))
	{
		rsd_copy_section(result, sub1);

		if (sub1->constant < sub2->constant)
		{
			result->type = RSD_EXPR_RANGE;
			result->lo_b = sub1->constant;
			result->up_b = sub2->constant;
		}
		else if (sub1->constant > sub2->constant)
		{
			result->lo_b = sub2->constant;
			result->type = RSD_EXPR_RANGE;
			result->up_b = sub1->constant;
		}

		return;
	}

  /* else convert to ranges */

  rsd_convert_expr_to_range(sub1, &temp1, loop_nest1);
  rsd_convert_expr_to_range(sub2, &temp2, loop_nest2);

  /*if either temp1 or temp2 is not a range, the expression was not*/
  /*converted for some reason. Return result as NO DATA.           */

  if (temp1.type != RSD_RANGE || temp2.type != RSD_RANGE)
    {
      result->type = RSD_NO_DATA;
    }
  else /*merge ranges*/
    {
      rsd_merge_range_range(result, &temp1, &temp2);
    }
}




/*----------------------------------------------------------------------

    rsd_merge_expr_range()  
       
    Merge an expression section with a range section. 
    First convert the expression section into a range section, then
    merge the two ranges.

*/

static void rsd_merge_expr_range (Rsd_data *result, Rsd_data *sub1, Rsd_data *sub2, 
                                  Loop_list *loop_nest)
{
  Rsd_data temp;     /*temp tp hold converted expression*/

  /*convert expression to range*/
  rsd_convert_expr_to_range(sub1, &temp, loop_nest);

  /*if  temp is not a range, the expression was not      */
  /*converted for some reason. Return result as NO DATA. */

  if (temp.type != RSD_RANGE)
    {
      result->type = RSD_NO_DATA;
    }
  else /*merge ranges*/
    {
      rsd_merge_range_range(result, &temp, sub2);
    }
}




/*----------------------------------------------------------------------

    rsd_merge_range_range()  
       
    Merge to sections, each being a range. 
    The lower bound is the minimum of the two
    lower bounds, and the upper bound the max of the two uppers.
    The step is gcd1(step1, step2, diff), where diff is the difference
    between the two lower bounds.
    The begin field will be set equal to the begin field of the
    first range.

    If the two ranges are identical and are tagged, the tag will
    be preserved, otherwise the tag is dropped.

*/

static void rsd_merge_range_range (Rsd_data *result, Rsd_data *sub1, Rsd_data *sub2)
{
  int    step,
         tmp1,tmp2;


  /*identical ranges?*/
  if (sub1->lo_b    == sub2->lo_b      &&
      sub1->up_b    == sub2->up_b      &&
      sub1->step    == sub2->step      &&
      sub1->begin   == sub2->begin)
    {
      rsd_copy_section(result,sub1);
      return;
    }

  result->type = RSD_RANGE;

  result->lo_b = min(sub1->lo_b, sub2->lo_b);
  result->up_b = max(sub1->up_b, sub2->up_b);

  step = gcd1(sub1->step, sub2->step, &tmp1, &tmp2);
  if (step != 1 )
    step = gcd1(step, abs(sub1->lo_b - sub2->lo_b), &tmp1, &tmp2);
  result->step = step;

  result->begin = sub1->begin;
}


/*----------------------------------------------------------------------

    rsd_copy_section()  
       

*/

static void rsd_copy_section (Rsd_data *to, Rsd_data *from)
{
	bcopy((const char *)from, (char *)to, sizeof(Rsd_data));
}


/*-----------------------------------------------------------------------

    rsd_convert_expr_to_range()  

    Convert a subscript expression into a subscript range.
    If it cannot be converted for some reason, it is left as 
    an expression.

*/

static void rsd_convert_expr_to_range(Rsd_data *sdata, Rsd_data *result, 
                                      Loop_list *loop_nest)
{
  Loop_data *loop;

  result->coeff = sdata->coeff;
  result->constant = sdata->constant;
  result->ivar = sdata->ivar;

  loop = loop_nest->loops + result->ivar;

  /*if loop data is bottom, make rsd to be bottom also*/

  if ((loop->lo.val == MININT) && (loop->up.val == MAXINT))
    {
      rsd_make_section_bottom(result); 
      return;
    }

  /*calculate range*/

  if (loop->lo.val == MININT)
     result->lo_b = MININT;
  else if (sdata->type == RSD_EXPR)
     result->lo_b = (sdata->coeff * loop->lo.val) + sdata->constant;
  else /* if (sdata->type == RSD_EXPR_RANGE) */
     result->lo_b = (sdata->coeff * loop->lo.val) + sdata->lo_b;

  if (loop->up.val == MAXINT)
     result->up_b = MAXINT;
  else if (sdata->type == RSD_EXPR)
     result->up_b = (sdata->coeff * loop->up.val) + sdata->constant;
  else /* if (sdata->type == RSD_EXPR_RANGE) */
     result->up_b = (sdata->coeff * loop->lo.val) + sdata->up_b;

  if (sdata->type == RSD_EXPR)
     result->step = sdata->coeff * loop->step.val;
  else /* if (sdata->type == RSD_EXPR_RANGE) */
     result->step = 1;	/* conservative assumption for now */

  result->begin = result->lo_b;

  result->type = RSD_RANGE;
}
  

/* eof */


