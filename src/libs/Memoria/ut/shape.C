/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
#include <mh.h>
#include <shape.h>

Boolean check_inner_control(AST_INDEX inner_control,
			    char *outer_inductvar)
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
  
  {
   return(BOOL(
	  pt_find_var(gen_INDUCTIVE_get_rvalue1(inner_control),outer_inductvar)
	  ||
	  pt_find_var(gen_INDUCTIVE_get_rvalue2(inner_control),outer_inductvar)
	  ||
	  pt_find_var(gen_INDUCTIVE_get_rvalue3(inner_control),outer_inductvar)
	  ));
}

static loop_shape_type check_trapezoidal(char       *outervar,
					 AST_INDEX  expr,
					 char       *fn1,
					 char       *fn2)

  {
   char      *fname;
   Boolean   lin1;
   int       coeff1,
             coeff2;
   AST_INDEX alist;

     /* checks only upper bound for now */

     if (!is_invocation(expr))
       return(COMPLEX);
     fname = gen_get_text(gen_INVOCATION_get_name(expr));
     if (strcmp(fname,fn1) != 0 && strcmp(fname,fn2) != 0)
       return(COMPLEX);
     alist = gen_INVOCATION_get_actual_arg_LIST(expr);
     if (list_length(alist) != 2)
       return(COMPLEX);
     else
       {
	pt_get_coeff(list_first(alist),outervar,&lin1,&coeff1);
	if (NOT(lin1) || coeff1 < 0)
	  return(COMPLEX);
	else
	  {
	   pt_get_coeff(list_last(alist),outervar,&lin1,&coeff2);
	   if (NOT(lin1) || coeff2 < 0 || (coeff1 != 0 && coeff2 != 0))
	     return(COMPLEX);
	   else
	     return(TRAP);
	  }
       }
  }

static void determine_loop_type(model_loop *loop_data,
				int        outer,
				int        inner)
  
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   char      *outervar;
   AST_INDEX inner1,inner2;
   AST_INDEX outer_control,inner_control,step;
   AST_INDEX fac,con,temp;
   int       coeff1,coeff2,step_vi,step_vo;
   Boolean   lin1,lin2;

     inner_control = gen_DO_get_control(loop_data[inner].node);
     outer_control = gen_DO_get_control(loop_data[outer].node);
     if ((step = gen_INDUCTIVE_get_rvalue3(inner_control)) == AST_NIL)
       step_vi = 1;
     else 
       (void)pt_eval(step,&step_vi);
     if ((step = gen_INDUCTIVE_get_rvalue3(outer_control)) == AST_NIL)
       step_vo = 1;
     else 
       (void)pt_eval(step,&step_vo);
     if (step_vi != 1 || step_vo != 1)
       loop_data[outer].type = COMPLEX;
     else
       {
	outervar = gen_get_text(gen_INDUCTIVE_get_name(outer_control));
	inner1 = gen_INDUCTIVE_get_rvalue1(inner_control);
	inner2 = gen_INDUCTIVE_get_rvalue2(inner_control);
	pt_get_coeff(inner1,outervar,&lin1,&coeff1);
	pt_get_coeff(inner2,outervar,&lin2,&coeff2);
	if (NOT(lin1))
	  if(lin2)
	    {
	     if (coeff1 >= 0)
	       {
		loop_data[outer].type = check_trapezoidal(outervar,inner1,
							  "max","max0");
		if (loop_data[outer].type == TRAP)
		  {
		   loop_data[outer].trap_fn = FN_MAX;
		   loop_data[outer].trap_loop = inner;
		  }
	       }
	     else
	       loop_data[outer].type = COMPLEX;
	    }
	  else if (check_trapezoidal(outervar,inner1,"max","max0") == TRAP &&
		   check_trapezoidal(outervar,inner2,"min","min0") == TRAP)
	    {
	     loop_data[outer].type = TRAP;
	     loop_data[outer].trap_fn = FN_BOTH;
	     loop_data[outer].trap_loop = inner;
	    }
	  else
	    loop_data[outer].type = COMPLEX;
	else if (NOT(lin2))
	  if (coeff2 >= 0)
	    {
	     loop_data[outer].type = check_trapezoidal(outervar,inner2,"min",
						       "min0");
	     if (loop_data[outer].type == TRAP)
	       {
		loop_data[outer].trap_fn = FN_MIN;
		loop_data[outer].trap_loop = inner;
	       }
	    }
	  else
	    loop_data[outer].type = COMPLEX;
	else if ((coeff1 == 0) && (coeff2 == 0))
	  loop_data[outer].type = COMPLEX;
	else if ((coeff1 != 0) && (coeff2 != 0))
	  if (coeff1 != coeff2)
	    loop_data[outer].type = COMPLEX;
	  else
	    if (coeff1 > 0)
	      {
	       loop_data[outer].type = RHOM;
	       loop_data[outer].tri_loop = inner;
	       loop_data[outer].tri_coeff = coeff1;
	       pt_separate_linear(inner1,outervar,&lin1,&fac,&con);
	       loop_data[outer].tri_const = tree_copy_with_type(con);
	       pt_separate_linear(inner2,outervar,&lin1,&fac,&con);
	       loop_data[outer].rhom_const = tree_copy_with_type(con);
	      }
	    else
	      loop_data[outer].type = COMPLEX;
	else 
	  if (coeff1 == 0)
	    {     /* right side */
	     pt_separate_linear(inner2,outervar,&lin1,&fac,&con);
	     loop_data[outer].tri_coeff = coeff2;
	     loop_data[outer].tri_const = tree_copy_with_type(con);
	     if (coeff2 > 0)
	       loop_data[outer].type = TRI_LR;
	     else 
	       loop_data[outer].type = TRI_LL;
	     loop_data[outer].tri_loop = inner;
	    }
	  else 
	    {                /* left side */
	     pt_separate_linear(inner1,outervar,&lin1,&fac,&con);
	     loop_data[outer].tri_coeff = coeff1;
	     loop_data[outer].tri_const = tree_copy_with_type(con);
	     if (coeff1 > 0) 
	       loop_data[outer].type = TRI_UL;
	     else
	       loop_data[outer].type = TRI_UR;
	     loop_data[outer].tri_loop = inner;
	    }
       }
     if (loop_data[outer].type == COMPLEX)
       {
	loop_data[outer].max = 0;
	loop_data[outer].val = 0;
       }
  }

void ut_compare_loops(model_loop *loop_data,
		      int        outer,
		      int        inner)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX inner_control,
             outer_control;
   char      *ivar;

     inner_control = gen_DO_get_control(loop_data[inner].node);
     outer_control = gen_DO_get_control(loop_data[outer].node);
     ivar = gen_get_text(gen_INDUCTIVE_get_name(outer_control));
     if (check_inner_control(inner_control,ivar))
       switch(loop_data[outer].type) {
	 case RECT: 
	   determine_loop_type(loop_data,outer,inner);
	   break;
	 case TRI_LL:
	 case TRI_LR:
	 case TRI_UL:
	 case TRI_UR:
	   if (loop_data[inner].level == 
	       loop_data[loop_data[outer].tri_loop].level)
	     loop_data[outer].type = MULT;
	   loop_data[outer].max = 0;
	   loop_data[outer].val = 0;
	   break;
	 default:
	   loop_data[outer].type = COMPLEX;
	   loop_data[outer].max = 0;
	   loop_data[outer].val = 0;
	  }
  }
       

static void check_inner_loops(model_loop *loop_data,
			      int        outer,
			      int        inner)
  
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
  
  {
   int next;

     for (next = loop_data[inner].inner_loop;
	  next != -1;
	  next = loop_data[next].next_loop)
       if (loop_data[outer].type != COMPLEX)
	 {
	  ut_compare_loops(loop_data,outer,next);
	  check_inner_loops(loop_data,outer,next);
	 }
  }

void ut_check_shape(model_loop *loop_data,
		    int        loop)

  {
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

   int next;
   
     check_inner_loops(loop_data,loop,loop);
     for (next = loop_data[loop].inner_loop;
	  next != -1;
	  next = loop_data[next].next_loop)
       ut_check_shape(loop_data,next);
  }
