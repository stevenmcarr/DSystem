/* $Id: PerfEstDFProblem.h,v 1.4 1997/03/11 14:35:11 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * 
 * Class definitions for performance estimation interprocedural data flow
 * problem.
 * 
 * Author: N. McIntosh
 * 
 * Copyright 1992, Rice University, as part of the ParaScope Programming
 * Environment Project
 * 
 */

#ifndef InterProcDFProblem_h
#include <libs/ipAnalysis/include/InterProcDFProblem.h>
#endif

#ifndef PerfAnnot_h
#include <libs/ipAnalysis/problems/perfEst/PerfAnnot.h>
#endif

#include <libs/support/misc/dict.h>

class CallGraphNode; // external declaration
class CallGraphEdge; // external declaration

#define PERF_EST_PROBLEM "Performance_Estimation"

/*
 * Keep a dictionary (keyed on function name) of "initial data" This
 * structure is the data stored in the dictionary for each module.
 */

typedef struct PerfEstDF_idata_ {
  PerfEstExpr *e;
} PerfEstDF_idata;

class PerfEstDFProblem : public InterProcDFProblem { 

  CallGraph *cg;
  
  /*
   * The following is used to store initial information which is read in
   * separately from the normal "informals.initial" file. This dictionary
   * is keyed by function name, and the value for each key is a pointer to
   * a PerfEstDF_idata structure.
   */
  Dict *initial_data;

  /*
   * The following is used to hold information about IP constants. We read
   * in IP constants data and stash it in this dictionary to consult it
   * later.
   */
  Dict *ip_const_data;

  /* Add new mapping (name->expr) to the initial data dictionary. 
   */
  void add_to_initial_data_map(char *func, PerfEstDF_idata *data);

  /* See if there is a mapping for a name, and return it if possible
   */
  PerfEstDF_idata *query_initial_data(char *func);

  /*
   * Add new mapping ("func:var" -> <ptr to int>) to the ip constants
   * dictionary.
   */
  void add_to_ip_constant_map(char *func, char *var, int value);

  /*
   * Query ip constants data, if available. Returns 'true' if the variable
   * is constant upon entry to the procedure given by 'funcname', and
   * stashes value in 'rval'. Returns 'false' if no value is available,
   * Reads in constants if not already done.
   */
  Boolean query_ip_const(char *varname, char *funcname, int &rval);

  /* helper function */
  friend int ip_constants_mapfunc(char *name, int &ival, double &fval,
				    void *user);

  /* Read in the IP constants info.
  */
  void read_ip_constants(Context     program_context);

 public: 
  
  /* Constructor and Destructor.
   */
  PerfEstDFProblem (CallGraph *newcg);
  ~PerfEstDFProblem();

  /* Functions required by PcompDFProblem
   */
  void InitializeNode (CallGraphNode *node);

  /* functions for data flow algorithm
   */
  void  *DFtrans(void *in_v, void *prev_out_v, void *node_v, 
				unsigned char &changed);
  void  *DFmeet(void *partial_result_annot_v, void *, void *self_node_v, 
	       void *edge_v);
};
