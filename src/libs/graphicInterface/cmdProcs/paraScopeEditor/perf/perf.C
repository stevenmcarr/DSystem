/* $Id: perf.C,v 1.1 1997/06/25 14:41:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/***************************************************************************
  perf.c --  Estimate communication and exec time performance for selected 
             program segment.
 
  >> The software in this file is public domain. You may copy, modify and 
  >> use it as you wish, provided you cite the author for the original source.
  >> Remember that anything free comes with no guarantees. 
  >> Use it at your own risk. 

  Author:  Vas, July 1990.

  Modification history:
  
 ***************************************************************************/

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped.h>
#include <math.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/fort.h>
#include <libs/frontEnd/include/walk.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/perf.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>
#include <libs/graphicInterface//oldMonitor/include/dialogs/message.h>

#define  Arith   Train_Data_List->arithdata
#define  Cntrl   Train_Data_List->arithdata
 
/* boundaries of the selected program segment */
AST_INDEX   first_stmt;
AST_INDEX   last_stmt;

/* global var to keep pointer to training set data read by perf_init() */
TrainData   *Train_Data_List;

/* global var to keep number of processors used */
int   Num_Procs;

/* global Dependence pointer */
PedInfo  Global_Dep_Ptr;

/* global ast index of PROGRAM statement */
AST_INDEX  Program_Root;

/* structure for passing parameters during recursive tree walk */
struct Prmtr {
  float        exec_cost_lb; /* lower bound on execution time */
  float        comm_cost_lb; /* lower bound on communication time */
  float        exec_cost_ub; /* upper bound on execution time */
  float        comm_cost_ub; /* upper bound on communication time */
  AST_INDEX    lvalue;
  struct Prmtr *prev;
};

int ped_link_perf = 1;  /* Fortran D performance estimator linked in */

Parm  *Top_of_stack;

/* forward declarations */
int check_if_Express();
STATIC(int, post_stmt_walk,(AST_INDEX stmt, int nesting_level, Generic parms));
STATIC(int, post_expr_walk,(AST_INDEX node, Generic parms));
STATIC(int, pre_stmt_walk,(AST_INDEX stmt, int nesting_level, Generic parms));
float time_comm();
void get_msg_info(); 
void correct_for_overlap();
int check_if_int();
Parm * push();
Parm * pop();

/*-------------------------------------------------------------------------
   perf_estimate -- estimates performance costs of selected program segment.
   Returns a code to indicate whether procedure terminated correctly.
   Entry routine that interfaces the performance estimation module to
   ParaScope.
  -------------------------------------------------------------------------*/
int
perf_estimate(AST_INDEX fstmt, AST_INDEX lstmt, float *ccost_lb, float *ecost_lb, 
              float *ccost_ub, float *ecost_ub)
{
  int        nlevel1, nlevel2;
  AST_INDEX  curr;
  int        found = 0;
  Generic    parms;
  Parm       *pp, *tos;
  float      execcost_lb = 0.0, commcost_lb = 0.0;
  float      execcost_ub = 0.0, commcost_ub = 0.0;

  pp = (Parm *) get_mem(sizeof(Parm), "perf_estimate");

  first_stmt = fstmt;
  last_stmt = lstmt;
  pp->exec_cost_lb = 0.0;
  pp->comm_cost_lb = 0.0;
  pp->exec_cost_ub = 0.0;
  pp->comm_cost_ub = 0.0;
  pp->lvalue = AST_NIL;
  
  /*
   * first_stmt and last_stmt define the boundaries of the selected
   * program segment for which performance is to be analyzed.
   *
   * NOTE: a valid selection of first_stmt and last_stmt must satisfy
   * the following property:
   *    1. Either both first_stmt and last_stmt have nesting level 1
   *       (i.e., both are at outermost level), or
   *    2. Both are on the same LIST, or
   *    3. last_stmt is AST_NIL, which means perf_estimate will only
   *       walk the ast with first_stmt as root.
   * If neither of these conditions hold, the selection is invalid and
   * the procedure will abort.
   */

  Top_of_stack = NULL;
  nlevel1 = get_nesting_level(fstmt);
  nlevel2 = get_nesting_level(lstmt);

  if (nlevel1 >= 0 && nlevel2 == -1) {
    if (lstmt == AST_NIL)
      last_stmt = fstmt;
    /* walk the statement tree starting from first_stmt */
    tos = push(pp);
    walk_statements(fstmt, nlevel1,
		    pre_stmt_walk, post_stmt_walk, parms);
    tos = pop(&pp);
    *ecost_lb = pp->exec_cost_lb;
    *ccost_lb = pp->comm_cost_lb;
    *ecost_ub = pp->exec_cost_ub;
    *ccost_ub = pp->comm_cost_ub;
  }
  else {
    if (nlevel1 == 1 && nlevel2 == 1) {
      found = 1;
    }
    else {
      /* check if fstmt and lstmt are on the same LIST */
      curr = fstmt;
      while (found == 0 && curr != AST_NIL) {
	if (curr == lstmt) {
	  found = 1;
	}
	curr = list_next(curr);
      }
    }
    if (found == 1) {
      /* walk the list of statements from first_stmt to last_stmt */
      for (curr = fstmt; curr != AST_NIL; curr = list_next(curr)) {
	pp->exec_cost_lb = 0.0;
	pp->comm_cost_lb = 0.0;
	pp->exec_cost_ub = 0.0;
	pp->comm_cost_ub = 0.0;
	pp->lvalue = AST_NIL;
	tos = push(pp);
	walk_statements(curr, nlevel1,
			pre_stmt_walk, post_stmt_walk, parms);
	tos = pop(&pp);
	execcost_lb += pp->exec_cost_lb;
	commcost_lb += pp->comm_cost_lb;
	execcost_ub += pp->exec_cost_ub;
	commcost_ub += pp->comm_cost_ub;
	if (curr == lstmt)
	  break;
      }
    *ecost_lb = execcost_lb;
    *ccost_lb = commcost_lb;
    *ecost_ub = execcost_ub;
    *ccost_ub = commcost_ub;
    }
    else { /* not found */
      return (-1);  /* illegal selection */
    }
  }

  free_mem(pp);

  return (0);

}

/*------------------------------------------------------------------------
   pre_stmt_walk --  action prior to walking stmt.
  ------------------------------------------------------------------------*/
static int
pre_stmt_walk(AST_INDEX stmt, int nesting_level, Generic parms)
{
  AST_INDEX   expr;
  Parm        *ppnew, *tos;
  AST_INDEX   parmlist;
  static char name[20];
  int         nparms;

  ppnew = (Parm *) get_mem(sizeof(Parm), "pre_stmt_walk");
  
  /* initialize exec and comm costs before visiting this stmt */
  ppnew->exec_cost_ub = 0.0;
  ppnew->comm_cost_ub = 0.0;
  ppnew->exec_cost_lb = 0.0;
  ppnew->comm_cost_lb = 0.0;
  ppnew->lvalue = AST_NIL;

  if (is_assignment(stmt)) {
    /* walk the expression on the rhs of the assignment stmt */
    get_assignment_info(stmt, &(ppnew->lvalue), &expr);
    walk_expression(expr, NULL, post_expr_walk, (Generic) ppnew);
    if (is_subscript(ppnew->lvalue)) {
      /* walk the subscript expression */
      walk_expression(ppnew->lvalue, NULL, post_expr_walk, (Generic) ppnew);
    }
  }

  /* push ppnew on stack */
  tos = push(ppnew);

  return(WALK_CONTINUE);
}

     
/*---------------------------------------------------------------------
   post_stmt_walk -- compute costs for statement just finished walking.
  ---------------------------------------------------------------------*/
static int
post_stmt_walk(AST_INDEX stmt, int nesting_level, Generic parms)
{
  AST_INDEX   expr, stmt_list, parmlist;
  int         rc;
  int         niters, nparms;
  AST_INDEX   ivar, lbound, ubound, step;
  static AST_INDEX   sym[4];
  static int         val[4];
  Parm        *pp, *pp_prev, *tos;
  static char name[20];
  float       prob_lb, prob_ub;

  pp_prev = Top_of_stack->prev;
  if (pp_prev == NULL) {
    printf("Panic: stack handling is screwed up!\n");
  }

  /* pop top element of stack */
  tos = pop(&pp);

  /* compute costs for this statement */

  if (is_do(stmt)) {
    /* multiply the costs by the number of iters of the DO loop */
    pt_get_loop_bounds(stmt, &ivar, &lbound, &ubound, &step);
    sym[1] = lbound;
    sym[2] = ubound;
    sym[3] = step;
    val[1] = 0;
    val[2] = 0;
    val[3] = 1;
    resolve_symbols(stmt, 3, sym, val);
    niters = (val[2] - val[1] + 1) / val[3];
    pp_prev->exec_cost_ub += (pp->exec_cost_ub * niters)
                             + Cntrl[T_DOLOOP];
    pp_prev->exec_cost_lb += (pp->exec_cost_lb * niters)
                             + Cntrl[T_DOLOOP];
    /*
     * NOTE: At this point we should put a test to check if
     * the msg size can be increased in the communication calls.
     * i.e., if they can be "vectorized" and then pulled out
     * of the loop.
     */
    pp_prev->comm_cost_ub += pp->comm_cost_ub * niters;
    pp_prev->comm_cost_lb += pp->comm_cost_lb * niters;
  }
  else if (is_if(stmt)) {
    /* walk the guard expression */
    expr = gen_IF_get_guard_LIST(stmt);
    walk_expression(expr, NULL, post_expr_walk, (Generic) pp);    
    /* get the branch probability */
    get_branch_probability(stmt, &prob_lb, &prob_ub);
    pp_prev->exec_cost_ub += Cntrl[T_IFTHEN] + (pp->exec_cost_ub * prob_ub);
    pp_prev->comm_cost_ub += pp->comm_cost_ub * prob_ub;
    pp_prev->exec_cost_lb += Cntrl[T_IFTHEN] + (pp->exec_cost_lb * prob_lb);
    pp_prev->comm_cost_lb += pp->comm_cost_lb * prob_lb;
  }
  else if (is_goto(stmt)){
    pp_prev->exec_cost_ub += pp->exec_cost_ub + Cntrl[T_GOTO];
    pp_prev->exec_cost_lb += pp->exec_cost_lb + Cntrl[T_GOTO];
  }
  else if (is_call(stmt)){
    /* walk the statements of the subroutine body */
    get_call_info(stmt, name, &parmlist, &nparms, &stmt_list);
    if (stmt_list != AST_NIL) {
      walk_statements(stmt_list, 1, pre_stmt_walk, 
		      post_stmt_walk, parms);
    }
    pp_prev->exec_cost_ub += pp->exec_cost_ub + Cntrl[T_CALL];
    pp_prev->exec_cost_lb += pp->exec_cost_lb + Cntrl[T_CALL];
    pp_prev->comm_cost_ub += pp->comm_cost_ub;
    pp_prev->comm_cost_lb += pp->comm_cost_lb;
  }
  else {
    /* assume stmt does not contribute to any of the costs */
    pp_prev->exec_cost_ub += pp->exec_cost_ub;
    pp_prev->comm_cost_ub += pp->comm_cost_ub;
    pp_prev->exec_cost_lb += pp->exec_cost_lb;
    pp_prev->comm_cost_lb += pp->comm_cost_lb;
  }

  /* free storage for pp */
  free_mem(pp);

  if (stmt == last_stmt)
    return (WALK_ABORT);
  else
    return (WALK_CONTINUE);

}

/*------------------------------------------------------------------
   post_expr_walk -- compute costs for expression just walked.
  -----------------------------------------------------------------*/
static int
post_expr_walk(AST_INDEX node, Generic parms)
{
  static AST_INDEX  sym[2];
  static int        val[2];
  AST_INDEX         lvalue, parmlist, stmt_list;
  Parm              *pp;
  int               rc, nparms;
  int               msg_size, data_stride;
  float             commtime, StartCost, pktization;
  static char       name[20];
  int               is_int;
  
  pp = (Parm *) parms;

  is_int = check_if_int(node);
  if (is_int < 0)
    is_int = (int) is_integer(pp->lvalue);

  if (is_binary_plus(node)) {
    if (is_int) {
      pp->exec_cost_ub += Arith[T_INTADD];
      pp->exec_cost_lb += Arith[T_INTADD];
    }
    else {
      pp->exec_cost_ub += Arith[T_FLOATADD];
      pp->exec_cost_lb += Arith[T_FLOATADD];
    }
  }
  else if (is_binary_minus(node)) {
    if (is_int) {
      pp->exec_cost_ub += Arith[T_INTMINUS];
      pp->exec_cost_lb += Arith[T_INTMINUS];
    }
    else {
      pp->exec_cost_ub += Arith[T_FLOATMINUS];
      pp->exec_cost_lb += Arith[T_FLOATMINUS];
    }
  }
  else if (is_binary_times(node)) {
    if (is_int) {
      pp->exec_cost_ub += Arith[T_INTTIMES];
      pp->exec_cost_lb += Arith[T_INTTIMES];
    }
    else {
      pp->exec_cost_ub += Arith[T_FLOATTIMES];
      pp->exec_cost_lb += Arith[T_FLOATTIMES];
    }
  }
  else if (is_binary_divide(node)) {
    if (is_int) {
      pp->exec_cost_ub += Arith[T_INTDIVIDE];
      pp->exec_cost_lb += Arith[T_INTDIVIDE];
    }
    else {
      pp->exec_cost_ub += Arith[T_FLOATDIVIDE];
      pp->exec_cost_lb += Arith[T_FLOATDIVIDE];
    }
  }
  else if (is_binary_exponent(node)) {
    pp->exec_cost_ub += Arith[T_EXPONENT] * val[1];
    pp->exec_cost_lb += Arith[T_EXPONENT] * val[1];
  }
  else if (is_subscript(node) || is_invocation(node)) {
    /* is this an Express communication call? */
    /* note: function invocations are sometimes confused to be
       subscript references, due to an unfixed Rn bug */
    rc = check_if_Express(node);
    if (rc == NOT_EXPRESS) {
      /* not an Express communication call */
      /* walk body of function if possible */
      get_function_info(node, name, &parmlist, &nparms, &stmt_list);
      if (stmt_list != AST_NIL) {
	walk_statements(stmt_list, 1, pre_stmt_walk, 
			post_stmt_walk, parms);
      }
      else {
	if (is_subscript(node)) {
	  pp->exec_cost_ub += Arith[T_ARRAYREF]; 
	  pp->exec_cost_lb += Arith[T_ARRAYREF]; 
	}
	else {
	  pp->exec_cost_ub += Arith[T_FUNCTION];
	  pp->exec_cost_lb += Arith[T_FUNCTION];
	}
      }
    }
    else {
      /* process the Express communication call */
      get_msg_info(rc, node, &msg_size, &data_stride);
      commtime = time_comm(rc, msg_size, data_stride, &StartCost, &pktization);
      pp->comm_cost_ub += commtime;
      pp->exec_cost_ub += commtime;
      /* correct for overlapped communication */
      correct_for_overlap(rc, node, &commtime);
      pp->comm_cost_lb += commtime;
      pp->exec_cost_lb += commtime;
    }
  }
  else {
    /* assume it does not contribute to cost */
  }

  return (WALK_CONTINUE);

}

/*----------------------------------------------------------------------
    check_if_int -- return 1 if this is an integer operation.
  ----------------------------------------------------------------------*/
int
check_if_int(AST_INDEX node)
{
  int  nsons;
  AST_INDEX op1, op2;

  nsons = gen_how_many_sons(gen_get_node_type(node));
  if (nsons == 2) {
    op1 = ast_get_son_n(node, 1);
    op2 = ast_get_son_n(node, 2);
    if ((is_identifier(op1) || is_constant(op1))
	&& (is_identifier(op2) || is_constant(op2))) {
      if (is_integer(op1) && is_integer(op2))
	return (1);
      else
	return (0);
    }
  }
  return (-1);
}


/*----------------------------------------------------------------------
   check_if_Express  -- checks if stmt is an Express communication
   utility. Returns the type of the communication. 
  ---------------------------------------------------------------------*/
int
check_if_Express(AST_INDEX stmt)
{
  static char  name[20];
  AST_INDEX    parmlist, body;
  int          numparms;

  get_function_info(stmt, name, &parmlist, &numparms, &body);

  /*
   * NOTE: for kxrea and kxwri, only the read part of the msg
   * is processed, so as not to count the costs twice. We assume
   * that the Express program has matching writes and reads. If not,
   * this is an error that would be detected by Cubix when the program
   * is run.
   */
  
  if (strcmp("kxread", name) == 0) {
    return(iSR);
  }
  else if (strcmp("kxvrea", name) == 0) {
    return(vSR);
  }
  else if (strcmp("kxvcha", name) == 0) {
    return(EXCH1);
  }
  else if (strcmp("kxbrod", name) == 0) {
    return(BCAST);
  }
  else if (strcmp("kxcomb", name) == 0) {
    return(COMBN);
  }
  else {
    return (NOT_EXPRESS);
  }

}


/*-----------------------------------------------------------------------
   get_msg_info -- return size (in bytes) of msg communicated by stmt,
   and the data stride in local memory (used to gather/scatter the data).
 -----------------------------------------------------------------------*/
void get_msg_info(int commtype, AST_INDEX stmt, int *msg_size, int *data_stride)
{
  AST_INDEX         parmlist, body, curr;
  static AST_INDEX  parm[20];
  int               count, i, t, t1, t2;
  static AST_INDEX  parg[10];
  static int        value[10];
  static char       name[20];

  get_function_info(stmt, name, &parmlist, &count, &body);

  curr = list_first(parmlist);
  for (i = 1; i <= count; i++) {
    /* get all the parameters into array parms[] */
    parm[i] = curr;
    curr = list_next(curr);
    value[i] = 0; /* default values */
  }

  /* parm[n] now contains the nth actual parameter in the Express function "stmt" */

  switch (commtype) {

    case iSR:
      parg[1] = parm[2];
      resolve_symbols(stmt, 1, parg, value);
      *msg_size = value[1];
      *data_stride = 1;
      break;

    case vSR:
      parg[1] = parm[2];
      parg[2] = parm[4];
      parg[3] = parm[3];
      resolve_symbols(stmt, 3, parg, value);
      *msg_size = value[1] * value[2];
      *data_stride = value[3];
      break;

    case EXCH1:
      parg[1] = parm[2];
      parg[2] = parm[4];
      parg[3] = parm[8];
      parg[4] = parm[10];
      parg[5] = parm[3];
      resolve_symbols(stmt, 5, parg, value);
      t1 = value[1] * value[2];
      t2 = value[3] * value[4];
      t = (t1 > t2)? t1 : t2;
      *msg_size = t;
      *data_stride = value[5];
      break;

    case BCAST:
      parg[1] = parm[3];
      resolve_symbols(stmt, 1, parg, value);
      *msg_size = value[1];
      *data_stride = 1;
      break;

    case COMBN:
      parg[1] = parm[3];
      parg[2] = parm[4];
      resolve_symbols(stmt, 2, parg, value);
      *msg_size = value[1] * value[2];
      *data_stride = 1;
      break;

    default:
      message("fatal error in get_msg_size (perf.c)");
      
  }
  
}

/*-------------------------------------------------------------------------
   time_comm -- return the communication time for given Express call and
   msg size, using table look up of data table initialized by perf_init().
 -------------------------------------------------------------------------*/
float
time_comm(int commtype, int msg_size, int dstride, float *StartCost, float *pktization)
{
  CommInfo  *ci;
  CommData  *cd, *prevcd;
  TrainData *td, *prevtd;
  float     constant, slope, t;

  if (dstride > sizeof(float))
    dstride = NON_UNIT;  /* non-unit data stride */
  else
    dstride = UNIT;

  td = Train_Data_List;
  prevtd = NULL;

  while (td != NULL) {
    /* locate appropriate TrainData entry */
    if (td->nprocs >= Num_Procs && td->dstride == dstride)
      break;
    else {
      prevtd = td;
      td = td->next;
    }
  }
  if (td == NULL) {
    if (prevtd == NULL) /* no TrainData list found */
      return (0.0);  
    /* could not find train data, so use data of last in Train list */
    td = prevtd;
  }

  ci = &(td->comminfo[commtype]);
  cd = ci->commdata;
  prevcd = NULL;

  while (cd != NULL) {
    /* check if msg_size falls within this data range */
    if (msg_size >= cd->xrange[0] && msg_size <= cd->xrange[1])
      break;
    else {
      prevcd = cd;
      cd = cd->next;
    }
  }
  if (cd == NULL) {
    if (prevcd == NULL) /* no Commdata list found */
      return (0.0);  
    /* could not find msg_size in measured data range,
       so use the last data entry to approximate for msg_size */
    cd = prevcd;
    constant = ci->StartCost + (ci->pktization * floor(msg_size/PACKET_SIZE));
    slope = cd->b;  /* use slope of previous packet data */
    t = constant + (slope * msg_size);
    *StartCost = ci->StartCost;
    *pktization = ci->pktization;
    return (t);
  }

  *StartCost = ci->StartCost;
  *pktization = ci->pktization;
  t = cd->a + (cd->b * msg_size);

  return (t);

}

/*-------------------------------------------------------------------------
    correct_for_overlap -- reverse search the list of stmts from node,
    and check if the first executable stmt encountered is another
    Express communication call. If so, then commtime(prev communication) -
    startup(prev comm) should be subtracted from ctime. This is an
    optimistic estimate of time saved due to overlap. 
  -------------------------------------------------------------------------*/
void correct_for_overlap(int rc, AST_INDEX node, float *ctime)
{
  AST_INDEX   stmt, left, right;
  float       prevctime, correction, StartCost=0.0, pktization=0.0;
  int         dstride, msg_size;

  stmt = out(node);
  if (stmt == AST_NIL || stmt == first_stmt)
    return;  /* no overlap correction */

  stmt = list_prev(stmt);

  while (stmt != AST_NIL && stmt != first_stmt) {
    if (! is_comment(stmt) && is_statement(stmt))
      break;
    else
      stmt = list_prev(stmt);
  }
  if (stmt == AST_NIL)
    return;  /* no overlap correction */

  if (! is_assignment(stmt))
    return;  /* no overlap correction */

  get_assignment_info(stmt, &left, &right);
  if (! is_invocation(right))
    return;  /* no overlap correction */
    
  rc = check_if_Express(right);
  if (rc == NOT_EXPRESS)
    return;  /* no overlap correction */

  /* found an Express call: need to make overlap correction */
  get_msg_info(rc, right, &msg_size, &dstride);
  prevctime = time_comm(rc, msg_size, dstride, &StartCost, &pktization);
  correction = prevctime - StartCost;
  *ctime = (*ctime > correction)? *ctime - correction : correction - *ctime;
  
}

/*-------------------------------------------------------------------------
  push, pop -- routines to explicitly manage the calling stack during
  recursive tree walk. Returns current Top_of_stack.
  -------------------------------------------------------------------------*/
Parm *
push(Parm *item)
{
  item->prev = Top_of_stack;
  Top_of_stack = item;
  return (Top_of_stack);
}

Parm *
pop(Parm **item)
{
  Parm  *temp;

  if (Top_of_stack == NULL)
    return (NULL);
  /* set *item to element about to be popped off the stack */
  *item = Top_of_stack;
  temp = Top_of_stack->prev;
  Top_of_stack->prev = NULL;
  Top_of_stack = temp;
  return (Top_of_stack);
}

