/* $Id: PerfEstDFProblem.C,v 1.4 1997/03/11 14:35:11 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * 
 * This file contains methods for the class PerfEstDFProblem
 * 
 * Author: N. McIntosh
 * 
 * Copyright 1992, Rice University, as part of the ParaScope Programming
 * Environment Project
 * 
 */

#include <stdio.h>
#include <unistd.h>

#include <libs/graphicInterface/oldMonitor/OBSOLETE/db/FileContext.h>
#include <libs/support/file/FormattedFile.h>

#if 0
#include <libs/ipAnalysis/OBSOLETE/IODictionary.h>
#endif

#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/callGraph/CallGraphIterators.h>
#include <libs/ipAnalysis/problems/perfEst/PerfEstDFProblem.h>
#include <libs/frontEnd/include/ModuleDictionary.h>

/* Temporary structure for use with a callback 
*/

typedef struct _ip_const_mapfunc_bundle {
  PerfEstDFProblem *prob;
  char *funcname;
} ip_const_mapfunc_bundle;

/* Various flags for turning things on and off
 */
static int debug = 0;
static int use_ipcp = 0;

/* Debugging flags
*/
#define DBG_DFMEET_1 0x01
#define DBG_DFMEET_2 0x02
#define DBG_DFMEET_3 0x04
#define DBG_DFMEET_4 0x08
#define DBG_DFMEET_5 0x10

#define NAMELEN 512

/*
 * Constructor for this data flow problem object. Set up top, bottom
 * elements.
 */

PerfEstDFProblem::PerfEstDFProblem(CallGraph * newcg)
{
  FILE *fp;

  // name = ssave(PERF_EST_PROBLEM);
  bottom = (void *) new PerfEstAnnot(PerfEstAnnot_Bottom, "bottom");
  top = (void *) new PerfEstAnnot(PerfEstAnnot_Top, "top");
  cg = newcg;
  direction = Backward;
  ip_const_data = 0;

  /*
   * Make a dictionary to store initial info mappings. Key the dictionary
   * on function name. The value of each entry is a pointer to a
   * PerfEstExpr.
   */
  initial_data = new Dict(cmpstr, hashstr, 0);

  /* This is something of a hack, but it's convenient. 
  */
  if (cg) {
    if ((fp = fopen("pe.debug.options", "r")) != NULL) {
      fscanf(fp, "0x%x %d", &debug, &use_ipcp);
      fclose(fp);
      fprintf(stderr,
	      "debugging options for IP perf est: print=0x%x ipcp=%d\n", 
	      debug, use_ipcp);
    }
  }
}

/*
 * Destructor -- free initial data dictionary
 */

PerfEstDFProblem::~PerfEstDFProblem()
{
  PerfEstDF_idata *d;
  int *cvalue;
  
  if (initial_data) {
    forDict(initial_data) {
      d = (PerfEstDF_idata *) v;
      delete d->e;
      delete d;
    } endDict;
    delete initial_data;
  }
  if (ip_const_data) { 
    forDict(ip_const_data) {
      cvalue = (int *) v;
      sfree((char *) k);
      delete cvalue;
    } endDict;
    delete ip_const_data;
  }
}

/*
 * Add new mapping (name to expr) to the initial data dictionary.
 */
  
void
PerfEstDFProblem::add_to_initial_data_map(char *func,
					  PerfEstDF_idata *data)
{
  initial_data->Insert(func, (void *) data);
}

/*
 * Get the initial data for a node. If we can't find a mapping for it in
 * our dictionary, then it means that the data for the specified function
 * hasn't been read in yet. In this case, get the context for the module
 * and read in the initial information for the module from the appropriate
 * file.
 */

PerfEstDF_idata *
PerfEstDFProblem::query_initial_data(char *func)
{ 
  PcompModuleIPinfoList *pm;
  ModuleIPinfoListEntry *cur_mod;
  Context mod_context;
  PerfEstExpr *expr;
  char buf[NAMELEN];
  PerfEstDF_idata *rval;
  int mod_count;
  IPinfoTree *info;
  int rstat;
  PerfEstDF_idata *newdata;
  ip_const_mapfunc_bundle cinfo; // for use by callback function

  /*
   * Return existing mapping if there is one.
   */
  if (rval = (PerfEstDF_idata *) ((*initial_data)[func]))
    return rval;
  
  /*
   * Otherwise, get the context for the module and open the corresponding
   * initial information file. If the file doesn't exist, then we're in
   * trouble.
   */
  mod_context = module_dict->getContext(func);
  FileContext fileContext;
  int code = fileContext.Open(mod_context);

  File *peAttrFile = fileContext.GetAttribute(PE_IPFILE_NAME);
  if (peAttrFile == 0) {
    fprintf(stderr, "Cannot access %s file for function '%s'\n",
	    PE_IPFILE_STR, func);
    return 0;
  }

  FormattedFile port(peAttrFile);

  /*
   * Read a count of the number of functions in the initial info file.
   */
  if (port.Read(mod_count) == EOF) {
    fprintf(stderr, "Cannot read module count in %s file.\n", PE_IPFILE_STR);
    return 0;
  }

  /*
   * For each function, read in a PerfEstExpr object for the the function.
   * Get the function name and check it against the name in the module
   * list, as a sanity check. Insert a new record for the
   * function in the mapping dictionary.
   */
  pm = (PcompModuleIPinfoList *) module_dict->lookupModule(mod_context);

  /*
   * Check to make sure that the NED ip info and the performance
   * estimation IP info agree as to the number of functions in this file.
   */
  if (mod_count != pm->Count()) {
    fprintf(stderr, "Warning: mismatch between # of functions");
    fprintf(stderr, "in IP module list and in %s(!)\n", PE_IPFILE_STR);
  }
  
  if (mod_count) {
    cur_mod = pm->First();
    while (cur_mod && mod_count) {
      
      /*
       * Get a pointer to the info for this module; allocate a new structure
       * to hold the info for the module.
       */
      info = cur_mod->info;
      newdata = new PerfEstDF_idata;
      
      /*
       * Read the function name from the file, and compare it to the
       * function name in the ip info entry
       */
      port.Read(buf, 511);
      if (strcmp(buf, info->name)) {
	fprintf(stderr, "Warning: name in mod list (%s)", info->name);
	fprintf(stderr, "disagrees with name in %s (%s)\n",
		PE_IPFILE_STR, buf);
      }
      
      /*
       * Allocate and read in the expression for the file
       */
      expr = new PerfEstExpr;
      rstat = expr->read(port);
      if (rstat == EOF)
	fprintf(stderr, "Problems reading PerfEstExpr for function '%s'\n",
		info->name);
      
      /*
       * If IPCP is enabled, then simplify it immediately based on IPCP.
       */
      if (use_ipcp) {
	cinfo.prob = this;
	cinfo.funcname = func;
	expr->simplify_root(ip_constants_mapfunc, (void *) &cinfo);
      }
      
      newdata->e = expr;
      
      /*
       * Add the new structure to the mapping.
       */
      add_to_initial_data_map(info->name, newdata);
      cur_mod = cur_mod->Next();
      mod_count--;
      
    }
  }
  
  /*
   * Finally, return the result from the updated mapping.
   */
  return (PerfEstDF_idata *) ((*initial_data)[func]);
}

/*
 * Query ip constants data, if available. Returns 'true' if the variable
 * is constant upon entry to the procedure given by 'funcname', and
 * stashes value in 'rval'. Returns 'false' if no value is available,
 * Reads in constants if not already done.
 */

Boolean PerfEstDFProblem::query_ip_const(char *varname,
					 char *funcname,
					 int &rval)
{
  char key[NAMELEN];
  int *mapvalue;

  /* Read in the constants if we haven't already 
  */
  if (!ip_const_data)
    read_ip_constants(cg->GetProgramContext());

  /* Query the mapping
  */
  sprintf(key, "%s:%s", funcname, varname);
  if (mapvalue = (int *) ((*ip_const_data)[key])) {
    rval = *mapvalue;
    return true;
  } else {
    return false;
  }
}

/*
 * Add new mapping ("func:var" -> <ptr to int>) to the ip constants
 * dictionary.
 */

void
PerfEstDFProblem::add_to_ip_constant_map(char *func, char *var, int value)
{
  char key[256];
  int *newint = new int;
  sprintf(key, "%s:%s", func, var);
  *newint = value;
  ip_const_data->Insert(ssave(key), newint);
}

/*
 * The following function takes a single argument, the database context
 * for the program composition. It opens the "pfc-constants" file in
 * the composition directory and then reads the constants into the
 * dictionary "ip_const_data", a member of the PerfEstDFProblem object.
 * 
 * The format of the constants file is a series of lines of the
 * form "<function_name> <variablename> <value>". 
 */

void
PerfEstDFProblem::read_ip_constants(Context program_context)
{
  FILE *fp;
  char *pathname;
  char buf[1025], funcname[128], varname[128];
  int value, linecount = 0;
  
  /*
   * Make a dictionary to store IPCP information.  The dictionary is keyed
   * on strings of the form "function:formal", and the value is a pointer
   * to an integer value (if the pointer is NULL, then the variable is not
   * constant).
   */
  ip_const_data = new Dict(cmpstr, hashstr, 0);

  /*
   * Open the constants file
   */
  pathname = annotPath(program_context, PE_PFC_CONSTS_FILE_NAME);
  if (!(fp = fopen(pathname, "r"))) {
    fprintf(stderr, "Cannot access %s file for this program.\n",
	    PE_CONSTS_FILE_STR);
    return;
  }
  sfree(pathname);

  /*
   * Read the constants 
   */
  while (fgets(buf, 1024, fp)) {
    linecount++;
    if (sscanf(buf, "%s %s %d", funcname, varname, &value) != 3) {
      fprintf(stderr, "Badly formatted %s file, line %d\n",
	      PE_CONSTS_FILE_STR, linecount);
      return;
    }
    add_to_ip_constant_map(funcname, varname, value);
  }
}

/*
 * The following is an obsolete version of this routine. it reads a
 * constants file in a format written by dan grove's constant propagator,
 * which no longer really works well.
 */
#if 0
X /* Temporarily borrowed from Dan Grove.
X */
X typedef enum { bottom_level, constant_level, direct_link
X } dgrove_lattice_level;
X void
X PerfEstDFProblem::read_ip_constants(Context program_context)
X {
X   char function_name[NAMELEN], *formal_name;
X   char package[NAMELEN];
X   FILE *fp;
X   char buf[1024];
X   int numfunctions, num_params, num_globals;
X   dgrove_lattice_level llevel;
X   int value, i;
X   CallGraphNode *mynode;
X   
X   /*
X    * Make a dictionary to store IPCP information. This is a temporary
X    * solution based on Dan Grove's IP constant propagator. The dictionary
X    * is keyed on strings of the form "function:formal", and the value is a
X    * pointer to an integer value (if the pointer is NULL, then the
X    * variable is not constant).
X    */
X   ip_const_data = new Dict(cmpstr, hashstr, 0);
X 
X   /*
X    * Open the constants file
X    */
X   if (port.p_open(program_context, PE_PFC_CONSTS_FILE_NAME, "r") == EOF) {
X     fprintf(stderr, "Cannot access %s file for this program.\n",
X 	    PE_CONSTS_FILE_STR);
X     return;
X   }
X 
X   /*
X    * Read the constant info
X    */
X   while (fgets(buf, 1023, fp)
X   for (port.read(numfunctions); numfunctions; --numfunctions) {
X     
X     if (port.read(function_name) == EOF) {
X       fprintf(stderr, "Premature EOF while reading '%s'\n",
X 	      PE_CONSTS_FILE_STR);
X       return;
X     }
X 
X     /*
X      * Constant formal parameters
X      */
X     port.read(num_params);
X     for (i = 0; i < num_params; i++) {
X       port.read(package);
X       sscanf(package, "%d %d", &llevel, &value);
X       if (llevel == constant_level) {
X 	mynode = cg->LookupNode(function_name);
X 	formal_name = mynode->FormalPositionToName(i+1);
X 	add_to_ip_constant_map(function_name, formal_name, value);
X       }
X     }
X     
X     /*
X      * Constant globals -- ignore these for now
X      */
X     port.read(num_globals);
X     for (i = 0; i < num_globals; i++)
X       port.read(package);
X   }
X }
#endif

/*
 * Look the name of the function up in the "initial data" dictionary and
 * create an Annotation object using the resulting expression. Add the
 * resulting annotation to the annotation for the node.
 */

void PerfEstDFProblem::InitializeNode(CallGraphNode *node)
{
  PerfEstDF_idata *data;
  PerfEstAnnot *n;

  /* Paranoia check
  */
  if (!node->procName) {
    fprintf(stderr, "InitializeNode -- no proc name \n");
    return;
  }

  /* Get initial data (may need to go to disk).
  */
  if (!(data = query_initial_data(node->procName))) {
    fprintf(stderr, "No initial data mapping for name %s\n", node->procName);
    return;
  }

  /* Is there an annotation already? If so, get rid of it.
  */
  n = (PerfEstAnnot *) node->GetAnnotation(PERFANNOT_NAME);
  if (n) delete n;

  /* Create an attach the annotation 
  */
  n = new PerfEstAnnot(PerfEstAnnot_Function, node->procName, data->e);
  node->PutAnnotation(n);
}

int 
ip_constants_mapfunc(char *name, int &ival, double &,
				    void *user)
{
  ip_const_mapfunc_bundle *info = (ip_const_mapfunc_bundle *) user;
  PerfEstDFProblem *prob = info->prob;
  char *funcname = info->funcname;
  int rval;

  if (prob->query_ip_const(name, funcname, rval)) {
    ival = rval;
    return TYPE_INTEGER;
  } else {
    return TYPE_UNKNOWN;
  }
}

/*
 * The interprocedural propagation which we are doing isn't really data
 * flow analysis, in the conventional sense. It's a single bottom-up pass
 * over the call graph, disguised as a backwards data flow problem.
 * 
 * The data flow solver deals with backward problems by reversing the edges
 * in the graph and proceeding thereafter as for a forward problem. Thus
 * when the transfer function is called, the node corresponds to a caller
 * and in-edges of the node correspond to calls which this routine makes
 * to other subroutines (i.e. the sources of the in-edges are routines
 * which this function calls).
 * 
 * At this point in the propagation, the costs have been computed completely
 * for the routines which this one calls (i.e. they are expressed without
 * any references to called functions). The goal here is to take the
 * expressions for the called routines and substitute them into the
 * current node's expression where they appear there.
 * 
 * When this routine is called, we are considering a particular in-edge for a
 * particular node. Let X be the function corresponding to the node we are
 * visiting. Let Y correspond to the node at the src of the in-edge (i.e.
 * a routine called by X). The goal is to find places in X's cost
 * expression which refer to Y, and substitute the expression for Y into
 * the expression for X at those spots.
 */

void *PerfEstDFProblem::DFmeet(void *partial_result_annot_v,
			       void *pred_annot_v, void *self_node_v,
			       void *)
{
  PerfEstAnnot *this_annot;	// annotation being built for current node
  PerfEstAnnot *pred_annot;	// annotation for pred node
  CallGraphNode *this_node;	// CallGraphNode object for this node in graph
  CallGraphNode *pred_node;	// CallGraphNode object for predecessor node
  char *this_fname;		// name of function for this node
  char *pred_fname;		// name of function for pred node
  char **formals_list;		// array of formals for pred node
  int nformals;			// number of elements in above array
  int i;			// generic induction variable
  ip_const_mapfunc_bundle info; // for use by callback function

  /*
   * Unpack our various void-* parameters and get some other miscellaneous
   * information.
   */
  this_annot = (PerfEstAnnot *) partial_result_annot_v;
  pred_annot = (PerfEstAnnot *) pred_annot_v;
  this_node = (CallGraphNode *) self_node_v;
  this_fname = this_node->procName;
  pred_fname = pred_annot->pname;

  /*
   * If the annotation being built for this node is 'top', then this is an
   * indication that this is the first edge for this node. In this case,
   * initialize the annotation using the info in the "real" call graph.
   */
  if (this_annot->IsTop()) {
    delete this_annot;
    this_annot = (PerfEstAnnot *) this_node->GetAnnotation(PERFANNOT_NAME);
    if (!this_annot) {
      fprintf(stderr, "PerfEstDFProblem::DFmeet -- fatal error: ");
      fprintf(stderr, "node '%s' has no annotation.\n",
	      this_fname ? this_fname : "<noname>");
      assert(1 == 0);
    }
    this_annot = (PerfEstAnnot *) this_annot->Clone();
  }

  /* debugging */
  if (debug & DBG_DFMEET_1) {
    printf("Expression for node '%s' prior to subst/IPCP:\n", this_fname);
    this_annot->Print();
  }

  /*
   * Is this a leaf in the call graph? Check for this case by seeing if
   * the predecessor node annotation has a name. If it is a leaf, then
   * don't bypass the substitution phase.
   */
  if (pred_fname) { /* this code not executed for leaves */

    /*
     * Build a list of formals for the pred node (which corresponds to the
     * callee)
     */
    pred_node = cg->LookupNode(pred_fname);
    nformals = pred_node->nformals;
    formals_list = new pointer_to_char_type[nformals];
    for (i = 0; i < pred_node->nformals; i++)
      formals_list[i] = pred_node->FormalPositionToName(i+1);
    
    /* Debugging code */
    if (debug & DBG_DFMEET_2) {
      printf("Substituting expression for '%s' into expression for '%s'\n",
	     pred_fname, this_fname);
      printf("Expression for pred ('%s'):\n", pred_fname);
      pred_annot->Print();
    }
    
    /*
     * Perform the substitution. This will modify/update the existing
     * annotation, without changing the predecessor's annotation.
     */
    this_annot->e->substitute_call(pred_fname,
				   formals_list,
				   nformals,
				   pred_annot->e);
    if (!use_ipcp)
      this_annot->e->simplify_root();
    
    /* debugging */
    if (debug & DBG_DFMEET_3) {
      printf("Following substitution, expression for this ('%s'):\n",
	     this_fname);
      this_annot->Print();
    }
    
    /*
     * Free the data we allocated.
     */
    delete formals_list;
  }

  if (use_ipcp) {

    /* Simplify the annotation based on interprocedural constants.
     */
    info.prob = this;
    info.funcname = this_fname;
    this_annot->e->simplify_root(ip_constants_mapfunc, (void *) &info);
    
    /* debugging */
    if (debug & DBG_DFMEET_5) {
      printf("following subst/IPCP, expr for this ('%s'):\n", this_fname);
      this_annot->Print();
    }
  }

  /* Return the new partially-completed annotation for this node.
  */
  return (void *) this_annot;
}

/*
 * By the time the transfer function gets called, we're basically done
 * already. We just return the 'in' value, since it corresponds to the
 * result built up by the DFmeet function.
 */

void *PerfEstDFProblem::DFtrans(void *in_v, void *, void *, unsigned char &) 
{
  return in_v;
}

