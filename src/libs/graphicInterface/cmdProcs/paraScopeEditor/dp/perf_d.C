/* $Id: perf_d.C,v 1.1 1997/06/25 14:38:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/***************************************************************************
  perf_dialog.c -- routines that implement the PED dialogs providing the
  interface to the performance estimation module.

  (c) Rice University, Caltech C3P and CRPC

  Author:  Vas, July 1990.

  Modification History:

 ***************************************************************************/

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped_dialogs.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/perf.h>
#include <libs/graphicInterface/oldMonitor/include/items/text.h>

#define FIRST_STMT    94
#define LAST_STMT     95
#define PERF_ESTIMATE 96
#define CLEARSELECS   97
#define DONE          98
#define DEFLT         99

typedef struct RSDia {
  Dialog     *di;
  DiaDesc    *title;
  DiaDesc    *text;
  char       *values;
  int        nsym;
  int        *vptr;
  int        *sindex;
} ResSymDia;

typedef struct BPDia {
  Dialog     *di;
  DiaDesc    *title;
  DiaDesc    *text;
  char       *prob1;
  char       *prob2;
  float      probval1;
  float      probval2;
} BranchProbDia;


/* forward declarations */
STATIC(Boolean, resolve_symbols_handler,(Dialog *di, Generic RS, Generic item_id));
STATIC(Boolean, branch_probability_handler,(Dialog *di, Generic BP, Generic item_id));
STATIC(Boolean, perf_handler,(Dialog *di, Generic PF, Generic item_id));

/*--------------------------------------------------------------------------
   perf_dialog_create -- create primary performance estimator dialog.
  --------------------------------------------------------------------------*/
Dialog *
perf_dialog_create(PerfDia *pf)
{
  Dialog  *di;

  di = dialog_create(
	  "Performance Estimator", perf_handler, (dialog_helper_callback) 0, (Generic) pf,
	  dialog_desc_group(
	     DIALOG_VERT_CENTER, 5,
	     pf->title = item_title(
			   UNUSED,
		     "....................................................\n\n\n",
			   DEF_FONT_ID
	     ),
	     dialog_desc_group(
	       DIALOG_HORIZ_CENTER, 2, 
	       item_button(FIRST_STMT, "First stmt ->", DEF_FONT_ID, false),
	       pf->ftext = item_title(
			     UNUSED,
			     "...............................................",
			     DEF_FONT_ID
	       )
	     ),
	     dialog_desc_group(
	       DIALOG_HORIZ_CENTER, 2, 			  
  	       item_button(LAST_STMT, "Last stmt  ->", DEF_FONT_ID, false),
	       pf->ltext = item_title(
			     UNUSED,
			     "...............................................",
			     DEF_FONT_ID
	       )
	     ),
	     dialog_desc_group(
	       DIALOG_HORIZ_CENTER, 2, 
	       item_button(PERF_ESTIMATE, " Estimate Performance ", DEF_FONT_ID, false),
	       item_button(CLEARSELECS, " Clear Selections ", DEF_FONT_ID, false)
	     ),
	     pf->text = item_title(
			  UNUSED,
		    "\n................................................................\n",
			  DEF_FONT_ID
	     )
	  )
       );

  item_title_justify_left(pf->title);
  item_title_justify_left(pf->text);
  item_title_justify_left(pf->ftext);
  item_title_justify_left(pf->ltext);

  item_title_change(pf->title, "Select program segment to be analyzed by clicking\n on the boundary stmts defining the segment");
  (void) item_title_change(pf->text, "");
  (void) item_title_change(pf->ftext, "");
  (void) item_title_change(pf->ltext, "");
  pf->first_stmt = AST_NIL;
  pf->last_stmt = AST_NIL;
  return (di);
  
}

/*-------------------------------------------------------------------------
   perf_dialog_run -- run the performance dialog.
  -------------------------------------------------------------------------*/
void 
perf_dialog_run(PerfDia *pf)
{
  dialog_modeless_show(pf->di);
}

/*------------------------------------------------------------------------
   perf_dialog_destroy -- destroy performance dialog.
  ------------------------------------------------------------------------*/
void 
perf_dialog_destroy(PerfDia *pf)
{
  pf->first_stmt = AST_NIL;
  pf->last_stmt = AST_NIL;
  dialog_destroy(pf->di);
}

/*-------------------------------------------------------------------------
   perf_handler -- handler for performance estimator dialog.
  -------------------------------------------------------------------------*/
static Boolean
perf_handler(Dialog *di, Generic PF, Generic item_id)
{
  PerfDia     *pf;
  static char str[100];
  PedInfo     dep;
  float       execcost_lb, commcost_lb, execcost_ub, commcost_ub;
  int         rc;
  float       commcost_pct;

  pf = (PerfDia *) PF;
  dep = Global_Dep_Ptr;

  switch (item_id) {

  case DIALOG_CANCEL_ID:
    return(DIALOG_QUIT);
    break;

  case FIRST_STMT:
    if (dep->selection == NO_SELECTION)
      break;
    if (is_statement(dep->selection)) {
      pf->first_stmt = dep->selection;
      sprintf(str, "[%d]  %s",
	      (int) pf->first_stmt, pt_get_stmt_text(dep, pf->first_stmt));
      item_title_change(pf->ftext, str);
      item_title_change(pf->text, "");
    }
    else
      item_title_change(pf->text, "Illegal selection: must be a statement.");
    
    break;

  case LAST_STMT:
    if (dep->selection == NO_SELECTION)
      break;
    if (is_statement(dep->selection)) {
      pf->last_stmt = dep->selection;
      sprintf(str, "[%d]  %s",
	      (int) pf->last_stmt, pt_get_stmt_text(dep, pf->last_stmt));
      item_title_change(pf->ltext, str);
      item_title_change(pf->text, "");
    }
    else
      item_title_change(pf->text, "Illegal selection: must be a statement.");
    
    break;

  case PERF_ESTIMATE:
    if (pf->first_stmt == AST_NIL && pf->last_stmt == AST_NIL)
      item_title_change(pf->text, "Specify boundaries of the program segment\nto be analyzed.");
    else {
      /* invoke the performance estimator */
      commcost_lb = 0.0;
      execcost_lb = 0.0;
      commcost_ub = 0.0;
      execcost_ub = 0.0;
      item_title_change(pf->text, "Invoking Performance Estimator ... please wait");
      rc = perf_estimate(pf->first_stmt, pf->last_stmt,
			 &commcost_lb, &execcost_lb, &commcost_ub, &execcost_ub);
      if (rc < 0) {
	item_title_change(pf->text, "Program segment violates selection rules.\nPlease select again.");
	break;
      }
      else {
	commcost_pct = (commcost_ub / execcost_ub) * 100.0;
	if (commcost_pct == 100.0)
	  commcost_pct = 99.99;
	if (execcost_lb == execcost_ub) {
	  sprintf(str, "Execution time estimate of segment  = %.2e secs\n%% of time spent in Communication   = %.2f %% ", execcost_ub, commcost_pct);
	}
	else {
	  sprintf(str, "Execution time estimate of segment = %.2e : %.2e secs\n%% of time spent in Communication   = %.2f %% ",
		  execcost_lb, execcost_ub, commcost_pct);
	}
	item_title_change(pf->text, str);
      }
    }

    break;

  case CLEARSELECS:
    pf->first_stmt = AST_NIL;
    item_title_change(pf->ftext, "");
    pf->last_stmt = AST_NIL;
    item_title_change(pf->ltext, "");
    item_title_change(pf->text, "");
    break;

  default:
    break;

  }

  return (DIALOG_NOMINAL);

}

/*--------------------------------------------------------------------------
   resolve_symbols_dialog -- dialog that prompts user to supply values
   for the list of symbols (specified in string symstr) that occur in the
   statement "stmt". The user-supplied values are
   returned via the integer array values[]. The indices into the values[]
   array where the values are expected by the calling routine are specified
   in the array sindex[0..num_symbols+1]. 
  --------------------------------------------------------------------------*/
void
resolve_symbols_dialog(AST_INDEX stmt, int num_symbols, char *symstr, 
                       int *sindex, int *values)
{
  ResSymDia  *rs;
  Dialog     *di;
  static char str[200], stmtstr[100];
  PedInfo    dep;

  dep = Global_Dep_Ptr;

  /* create the dialog */

  rs = (ResSymDia *) get_mem(sizeof(ResSymDia), "resolve_sym_dialog");
  rs->nsym   = num_symbols;
  rs->sindex = sindex;
  rs->vptr   = values;
  rs->values = ssave("");

  di = dialog_create(
	  "Resolve Symbols", resolve_symbols_handler, (dialog_helper_callback) 0,
	  (Generic) rs,
	  dialog_desc_group(
	     DIALOG_VERT_CENTER, 4,
	     rs->title = item_title(
			  UNUSED,
		 "........................................................\n\n\n\n",
			  DEF_FONT_ID
	     ),
	     item_text(UNUSED, "", DEF_FONT_ID, &rs->values, 50),
	     rs->text = item_title(
			  UNUSED,
		      ".......................................................\n\n",
			  DEF_FONT_ID
	     ),
	    item_button(DONE, " Continue Processing ", DEF_FONT_ID, false)
	  )
       );

  item_title_justify_left(rs->title);
  sprintf(stmtstr, "%s", pt_get_stmt_text(dep, stmt));
  sprintf(str, "Now processing statement:\n  [%d] %s\nEnter estimate of run-time values for the variables:\n    %s",
          stmt, stmtstr, symstr);
  
  (void) item_title_change(rs->title, str);
  (void) item_title_change(rs->text, "");

  /* run the dialog */
  dialog_modal_run(di);

  sfree(rs->values);
  free_mem(rs);
  dialog_destroy(di);

}

/*----------------------------------------------------------------------------
   resolve_symbols_handler -- dialog handler for resolving symbols.
  ----------------------------------------------------------------------------*/
static Boolean
resolve_symbols_handler(Dialog *di, Generic RS, Generic item_id)
{
  ResSymDia    *rs;
  char         *buf;
  static char  temp[20];
  static int   val[40];
  int          i, num, k, rc, len;

  rs = (ResSymDia *) RS;
  
  switch (item_id) {
    
  case DIALOG_CANCEL_ID:
    return (DIALOG_QUIT);
    break;

  case DONE:
    item_title_change(rs->text, "");
    /* process the values given by the user */
    if (strlen(rs->values) <= (size_t)0) {
      (void) item_title_change(rs->text,
	"Enter values separated by blanks;\nuse \"?\" to denote unknown values.");
      break;
    }
    len = strlen(rs->values);
    k = 0;
    num = 1;
    buf = rs->values;
    while (k < len) {
      buf = rs->values + k;
      rc = sscanf(buf, "%s %*s", temp);
      if (rc <= 0) {
	(void) item_title_change(rs->text, "Bad entry: enter values separated by blanks;\nuse \"?\" to denote unknown values.");
	break;
      }
      if (strcmp(temp, "?") != 0) { /* not an unknown value */
	rc = sscanf(temp, "%d", &(val[num]));
	if (rc <= 0) {
	  (void) item_title_change(rs->text, "Bad entry: enter values separated by blanks;\nuse \"?\" to denote unknown values.");
	  break;
	}
      }
      else
	val[num] = MINUS_INFTY;

      num++;
      k += strlen(temp) + 1;
      sprintf(temp, "\0");
    }

    if (rs->nsym == num-1) {
      /* got all values: copy into values array */
      for (i = 1; i <= rs->nsym; i++) {
	k = rs->sindex[i];
	if (val[i] != MINUS_INFTY)
	  rs->vptr[k] = val[i];
      }
      return (DIALOG_QUIT);
    }
    else {
	(void) item_title_change(rs->text, "Bad entry: enter values separated by blanks;\nuse \"?\" to denote unknown values.");
      break;
    }
    break;

  default:
    break;

  }

  return (DIALOG_NOMINAL);

}


/*---------------------------------------------------------------------------
  get_branch_probability -- prompt user for branch probability, and return it.
  ---------------------------------------------------------------------------*/
void
get_branch_probability(AST_INDEX stmt, float *prob_lb, float *prob_ub)
{
  BranchProbDia  *bp;
  Dialog     *di;
  static char str[200], stmtstr[100];
  PedInfo     dep;

  dep = Global_Dep_Ptr;

  /* create the dialog */

  bp = (BranchProbDia *) get_mem(sizeof(BranchProbDia), "get_branch_probability");
  bp->prob1 = ssave("");
  bp->prob2 = ssave("");
  bp->probval1 = -1.0;
  bp->probval2 = -1.0;

  di = dialog_create(
	  "Branch Probability", branch_probability_handler, (dialog_helper_callback) 0,
	  (Generic) bp,
	  dialog_desc_group(
	     DIALOG_VERT_CENTER, 4,
	     bp->title = item_title(
			  UNUSED,
  	         "........................................................\n\n\n",
			  DEF_FONT_ID
	     ),
	     dialog_desc_group(
	       DIALOG_HORIZ_CENTER, 2, 
	         item_text(UNUSED, "Lower bound: ", DEF_FONT_ID, &bp->prob1, 10),
	         item_text(UNUSED, "  Upper bound: ", DEF_FONT_ID, &bp->prob2, 10)
	       ),
	     bp->text = item_title(
			  UNUSED,
		    ".........................................................\n",
			  DEF_FONT_ID
	     ),
	    dialog_desc_group(
              DIALOG_HORIZ_CENTER, 2,
	      item_button(DONE, " Continue Processing ", DEF_FONT_ID, false),
	      item_button(DEFLT, " Use Default Values ", DEF_FONT_ID, false)
            )
	  )
       );

  item_title_justify_left(bp->title);
  sprintf(stmtstr, "%s", pt_get_stmt_text(dep, stmt));
  sprintf(str, "Now processing statement:\n   [%d] %s\nEnter estimate of branch probability:", stmt, stmtstr);
  
  (void) item_title_change(bp->title, str);
  (void) item_title_change(bp->text, "");

  /* run the dialog */
  dialog_modal_run(di);

  if (bp->probval1 < 0.0 || bp->probval2 < 0.0) {
    /* use default values */
    *prob_lb = DEFAULT_BRANCH_PROB_LB;
    *prob_ub = DEFAULT_BRANCH_PROB_UB;
  }

  sfree(bp->prob1);
  sfree(bp->prob2);
  free_mem(bp);
  dialog_destroy(di);

}

/*----------------------------------------------------------------------------
   branch_probability_handler -- dialog handler for branch probability.
  ----------------------------------------------------------------------------*/
static Boolean
branch_probability_handler(Dialog *di, Generic BP, Generic item_id)
{
  BranchProbDia *bp;
  static char  temp[10];
  int          rc;
  float        num1, num2;

  bp = (BranchProbDia *) BP;
  
  switch (item_id) {
    
  case DIALOG_CANCEL_ID:
    return (DIALOG_QUIT);
    break;

  case DONE:
    item_title_change(bp->text, "");
    /* process the value given by the user */
    if (strlen(bp->prob1) <= (size_t)0 || strlen(bp->prob2) <= (size_t)0) {
      (void) item_title_change(bp->text,
	"Enter both probabilities (between 0 and 1)");
      break;
    }
    sscanf(bp->prob1, "%s %*s", temp);
    rc = sscanf(temp, "%f", &num1);
    if (rc <= 0) {
      (void) item_title_change(bp->text,
			       "Probabilities must be between 0 and 1");
      break;
    }
    sscanf(bp->prob2, "%s %*s", temp);
    rc = sscanf(temp, "%f", &num2);
    if (rc <= 0) {
      (void) item_title_change(bp->text,
			       "Probabilities must be between 0 and 1");
      break;
    }
    if (num1 < 0.0 || num1 > 1.0 || num2 < 0.0 || num2 > 1.0) {
      (void) item_title_change(bp->text,
			       "Probabilities must be between 0 and 1");
      break;
    }
    bp->probval1 = num1;
    bp->probval2 = num2;

    return (DIALOG_QUIT);

  case DEFLT:
    /* use default values */
    bp->probval1 = DEFAULT_BRANCH_PROB_LB;
    bp->probval2 = DEFAULT_BRANCH_PROB_UB;

    return (DIALOG_QUIT);
    
  default:
    break;

  }

  return (DIALOG_NOMINAL);

}


