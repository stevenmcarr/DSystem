/* $Id: PerfAnnot.h,v 1.3 1997/03/11 14:35:10 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *
 * Call graph node annotation class for performance estimation.
 * 
 * For interprocedural performance estimation, each node in the call graph is
 * annotated with a symbolic expression corresponding to the cost of
 * executing the procedure in question. This file defines the PerfEstAnnot
 * class, which acts as the annotation object for performance estimation.
 *
 * Author: N. McIntosh
 *
 * Copyright 1992, Rice University, as part of the ParaScope
 * Programming Environment Project
 *
 */

#ifndef PerfAnnot_h
#define PerfAnnot_h

#ifndef PerfEstExpr_h
#include <libs/ipAnalysis/problems/perfEst/PerfEstExpr.h>
#endif

#ifndef CallGraphAnnot_h
#include <libs/ipAnalysis/callGraph/CallGraphAnnot.h>
#endif

class CallGraphNode; // external declaration

/*
 * The following is the name of the annotation, which must be chosen
 * uniquely from the other annotations. At some point this should be
 * consolidated into a single header files somewhere so that people can
 * easily choose annotation names.
 */
#define PERFANNOT_NAME "Performance Estimation"

/*
 * There are two special PerfEstAnnot objects, one for Top and one for
 * bottom (in the dataflow sense), and then everything else is just an
 * annotation for a function.
 */

typedef enum {
  PerfEstAnnot_Top, PerfEstAnnot_Bottom, PerfEstAnnot_Function
} PerfEstAnnotType;

typedef char *pointer_to_char_type;

/*
 * The class itself. Since it inherits from Annotation, it has to provide
 * "read", "write", and "print" routines. 
 */

class PerfEstAnnot : public CallGraphAnnot {
  
 private:
  void Construct(PerfEstAnnotType at, char *pn, PerfEstExpr *ne);

 public:
  PerfEstAnnotType atype;	// type of node: top, bottom, or function
  PerfEstExpr *e;		// expression tree for function 
  Boolean isroot;		// set to 1 if root node in call graph
  double total_time;		// total exec time, including descendants
  double local_time;		// time spent here, not including descendants
  double exec_count;		// estimated number of times executed (called)
  char *pname;			// procedure name 

  /* Constructor and destructor */
  PerfEstAnnot(PerfEstAnnotType at, char *pn, PerfEstExpr *ne = 0);
  PerfEstAnnot(CallGraphNode *node);
  ~PerfEstAnnot();

  void Print();
  void Write(FormattedFile& port);
  void Read(FormattedFile& port);

  // upcall to read an annotation
  int ReadUpCall(FormattedFile &port);

  // upcall to write an annotation
  int WriteUpCall(FormattedFile &port);

  // copy a derived annotation
  CallGraphAnnot *Clone();

  // generate printable version of the annotation
  OrderedSetOfStrings *CreateOrderedSetOfStrings(unsigned int width);
  
  /* Get a pointer to node's prive PerfEstExpr
  */
  PerfEstExpr *get_PerfEstExpr() { return e; };

  /*
   * Get total time, local time, etc.
   */
  double get_local_time() { return local_time; };
  double get_total_time() { return total_time; };
  double get_exec_count() { return exec_count; };
  Boolean get_isroot() { return isroot; };

  /* Test for top, bottom */
  Boolean IsTop();
  Boolean IsBottom();

  /*
   * Allow the PerfEstDFProblem class to muck with the internals of this
   * class.
   */
  friend class PerfEstDFProblem;
};

extern char *PERF_ANNOT;

#endif /* PerfAnnot_h */


