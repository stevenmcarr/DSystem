/* $Id: PerfEstExpr.h,v 1.7 1997/03/11 14:35:12 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * This header file contains C++ class definitions for the class
 * 'PerfEstExpr', along with its component structures and objects. A
 * PerfEstExpr object is a symbolic expression for the execution time of a
 * subroutine or loop. PerfEstExpr inherits from the class SymExprNode --
 * it is basically a SymExprNode expression tree with additional node
 * types.
 */

#ifndef PerfEstExpr_h_included
#define PerfEstExpr_h_included

#ifndef SymExpr_h_included
#include <libs/ipAnalysis/problems/perfEst/SymExpr.h>
#endif

class PerfEstExprNode;
class PerfEstExpr;

/*
 * This enumeration is used to determine the type of a given
 * PerfEstExprNode in an expression tree. The type of the node determines
 * how it is interpreted, the number of children it has, and how each
 * child is interpreted. Note that we can also have SymExprNode base node
 * types, like constants, variables, and operators.
 */

typedef enum {
  PNT_NONE=0,		/* dummy for catching uninitialized objects */
  PNT_BASE=1,		/* base class node (ex: a constant) */
  PNT_LOOP=2,		/* a loop */
  PNT_IF=3,		/* an IF statement */
  PNT_CALL=4,		/* a procedure/function call */
} PerfEstExprNodeType;

/*
 * Additional information at a loop node. Most of this additional junk
 * which has to do with parallelization.
 */

typedef struct __PNT_loopnode_info {
  int node_number;		/* node number */
  int iterations_guess;		/* guess as to number of iterations */
  int parallelizable;		/* TRUE -> can be run in parallel */
  int marked_parallel;		/* TRUE -> explicitly marked parallel in src */
  double par_time;		/* parallel execution time */
  int best_num_procs;		/* best # of processors for above time */
  int debug_astindex;		/* debugging only */
  char *debug_lbl;		/* debugging only */
} PNT_loopnode_info;

/*
 * The data at a function call node is the "node number" (as produced by
 * ft_NodeToNumber()) for the call site in question.. A function call node
 * one child (an expression) for each 'actual' at the call site.
 */

typedef struct __PNT_callnode_info {
  int node_number;		/* call node number */
  char *fname;			/* callee name */
} PNT_callnode_info;

/*
 * Additional information for nodes corresponding to IF statements -- 
 * the estimated branch probability. An IF node has two children, one
 * for the TRUE branch and one for the FALSE branch.
 */

typedef struct __PNT_ifnode_info {
  double branch_prob;		/* probability of TRUE branch taken */
} PNT_ifnode_info;

/*
 * The 'value' of a node. This is just a union which we dump everything
 * in; the meaning of the union is determined by the 'typ' field in the
 * node.
 */

typedef union _PerfEstExprNodeValue_union {
  PNT_loopnode_info loop;	/* loop info */
  PNT_callnode_info call;	/* call info */
  PNT_ifnode_info ifstmt;	/* IF stmt info */
} PerfEstExprNodeValue;

class PerfEstExprNode : public SymExprNode {

 private:

  /*
   * Data items at a node. If this set is added to or changed, the method
   * 'copynode' will have to be updated (among others).
   */
  PerfEstExprNodeType typ;	/* type of this node */
  PerfEstExprNodeValue val;	/* info for this node */

  void link(PerfEstExprNode *new_parent) {
    NonUniformDegreeTreeNode::link((NonUniformDegreeTreeNode *) new_parent);
  };

  PerfEstExprNode *get_nth_child(unsigned whichchild);

  /* Helper function for use with "New<mumble>" routines.
  */
  SymExprNode *copy_symexprnode_to_perfestexprnode(SymExprNode &x);

 public:

  /* Virtual destructor to support derived classes 
  */
  virtual ~PerfEstExprNode();

  /*
   * Vanilla constructor for creating "blank" nodes. As a rule, 
   * should only be used by routines which are about to overwrite
   * the contents of the newly-created PerfEstExpr (for example,
   * the 'read' method for this class). 
   */
  PerfEstExprNode(PerfEstExprNode *new_parent = 0) : 
    SymExprNode(SYMEXPR_ARITH_DERIVED, new_parent) { typ = PNT_NONE; };

  /* Construct a loop node */
  PerfEstExprNode(int node_number,
		  PerfEstExprNode *loop_body,
		  PerfEstExprNode *lower_bound,
		  PerfEstExprNode *upper_bound,
		  PerfEstExprNode *loop_step, int iterations_guess,
		  int is_parallelizable, int is_marked_parallel,
		  double parallel_time, int best_num_procs,
		  int astindex, char *labelstring);

  /* Construct a call node */
  PerfEstExprNode(int node_number, char *f_name,
		  int num_actuals, PerfEstExprNode **actuals_arr);

  /* Construct an 'if' node */
  PerfEstExprNode(double branch_prob,
		  PerfEstExprNode *cond_expr,
		  PerfEstExprNode *true_branch,
		  PerfEstExprNode *false_branch);

  /*
   * Override the virtual methods for constructing base type nodes
   * from class SymExprNode. This allows us to use the 'simplify_node()'
   * method provided by the parent class.
   */
  virtual SymExprNode *NewIconst(int i_const);
  virtual SymExprNode *NewRconst(double r_const);
  virtual SymExprNode *NewArithOp(SymExprNodeType a_type,
				  SymExprNode *left_son,
				  SymExprNode *right_son);
  virtual SymExprNode *NewVar(char *vname);
  virtual SymExprNode *NewNode(SymExprNodeType a_type);

  /*
   * Clone a node in the tree (ignore children). This method overrides a
   * virtual function of the same name in class SymExprNode.
   */
  virtual SymExprNode *nodeclone(); 

  /*
   * Copy the contents of one node to another, ignoring children. This
   * method overrides a virtual function of the same name in class
   * SymExprNode.
   */
  virtual void copynode(SymExprNode *target);

  /*
   * Since "nodeclone()" is a virtual function, we can use the base
   * class's clone routine without any changes.
   */
  PerfEstExprNode *clone();

  /* Simplify a node. Another virtual function in the SymExprNode class -- 
   * see that class definition for more details. 
   */
  virtual SymExprNode *simplify_node(Boolean &deletethis,
				     SymExprNode_mapnamefunc mapfunc = 0,
				     void *user = 0);

  /*
   * Simplify a tree. In fact, we can borrow the inherited routine
   * directly, but add a typecast so as not to have to put it elsewhere.
   */
  PerfEstExprNode *simplify(SymExprNode_mapnamefunc mapfunc = 0,
			    void *user = 0) {
    return (PerfEstExprNode *) SymExprNode::simplify(mapfunc, user);
  };

  PerfEstExprNode	*get_sum_firstchild() { return get_nth_child(1); };
  PerfEstExprNode	*get_sum_secondchild() { return get_nth_child(2); };
  PerfEstExprNode	*get_ifstmt_true_branch() { return get_nth_child(1); };
  PerfEstExprNode	*get_ifstmt_false_branch() { return get_nth_child(2);};
  PerfEstExprNode	*get_ifstmt_cond_expr() { return get_nth_child(3);};
  double		get_ifstmt_branchprob();
  PerfEstExprNode	*get_loop_lb() { return get_nth_child(1); };
  PerfEstExprNode	*get_loop_ub() { return get_nth_child(2); };
  PerfEstExprNode	*get_loop_step() { return get_nth_child(3); };
  PerfEstExprNode	*get_loop_body() { return get_nth_child(4); };
  PerfEstExprNode	*get_call_actual(unsigned i) {
    				return get_nth_child(i);
			      };

  PerfEstExprNodeType GetType() { return typ; };
  char *GetCallFname();

  void			get_estimate(double& re, int& isp);
  double		interpret_bottom();
  int 			is_bottom() { return (GetNodeType() ==
					      SYMEXPR_ARITH_BOTTOM) ? 1 : 0; };

  void			print(FILE *fp) { SymExprNode::print(fp); };
  void			get_loop_info_estimates(int &niterations,
						double& bodycost,
						int& is_precise);
  void			set_parallel_info(double parallel_etime,
					  int best_nprocs);
  void			estimate_best_num_procs(int& returned_estimate,
						int& is_precise);

  /*
   * See class SymExprNode for a description of these functions. These are
   * the same functions, but with PerfEstExprNode arguments/returns.
   */
  PerfEstExprNode *substitute(SymExprNode_substfunc, void *user);
  PerfEstExprNode *substitute_var(char *var, PerfEstExprNode *subst_expr);

  /*
   * The expression E we invoke this method on is assumed to be an
   * expression for a called routine X. Given the formal list for X, and a
   * call node C corresponding to a call to X, substitute in the actuals
   * at C for the formals of X in E.
   */
  PerfEstExprNode*
    substitute_actuals_for_formals(char **formals_list,
				   int nformals,
				   PerfEstExprNode *actualnode);
  
  /*
   * Perform "call substition" on an expression. Like the other
   * substitution functions, this returns a new expression without
   * modifying the original. This form of substitution works as follows:
   * Let E be an expression containing possible CALL nodes. Let F be a
   * function, and let G be an expression for the cost of F (possibly
   * containing references to F's formals). For each CALL node C in E
   * corresponding to a call to F, replace the CALL node with a new
   * expression G'. G' is constructed by taking a copy of G, and replacing
   * all the instances of F's formals with the corresponding actual at
   * callsite C.
   */
  PerfEstExprNode *substitute_call(char *called_fname,
				   char **formals_list,
				   int nformals,
				   PerfEstExprNode *callee_expr);
    
  /*
   * Override some of our inherited methods so we don't have to sprinkle
   * the code with casts in order to avoid warnings.
   */
  PerfEstExprNode *FirstChild() {
    return (PerfEstExprNode *) NonUniformDegreeTreeNode::FirstChild();
  };
  PerfEstExprNode *NextSibling() {
    return (PerfEstExprNode *) NonUniformDegreeTreeNode::NextSibling();
  };
  PerfEstExprNode *Parent() {
    return (PerfEstExprNode *) NonUniformDegreeTreeNode::Parent();
  };
  PerfEstExprNode *leftson() {
    return (PerfEstExprNode *) SymExprNode::leftson();
  };
  PerfEstExprNode *rightson() {
    return (PerfEstExprNode *) SymExprNode::rightson();
  };
  int ChildCount() { /* this is just to make it public */
    return SymExprNode::ChildCount();
  };

  /*
   * The following method is required by the ionudtree code so that when
   * it is reading in nodes of a derived class, the correct constructor
   * can be called.
   */
  NonUniformDegreeTreeNodeWithDBIO *
    New(NonUniformDegreeTreeNodeWithDBIO *new_parent) {
      return (NonUniformDegreeTreeNodeWithDBIO *)
	new PerfEstExprNode((PerfEstExprNode *) new_parent);
    };

  int ReadUpCall(FormattedFile& port);
  int WriteUpCall(FormattedFile& port);

  /*
   * Virtual function from SymExprNode. The 'print' routine is inherited
   * from SymExprNode.
   */
  virtual char **print_node(FILE *fp);

  friend void print_PerfEstExprNode(PerfEstExprNode *h);

};

/* Shorthand for a pointer to a PerfEstExprNode tree.
*/

typedef PerfEstExprNode *PerfEstExprHandle;
#define PERFEST_EXPR_HANDLE_NULL ((PerfEstExprHandle) 0)

/*
 * This class is an abstraction of a symbolic expression for the execution
 * time for a subroutine or loop. In fact, there isn't a lot of functionality
 * in this class which isn't already in the PerfEstExprNode class,
 * but for various reasons (some historical) this is a separate class
 * which contains a PerfEstExprNode tree as a data item (i.e. it's 
 * mainly a wrapper around a PerfEstExprNode tree).
 */

class PerfEstExpr {

 private:

  PerfEstExprHandle roothandle; /* the expression tree */
  
 public:

  /* Constructor and destructor */
  PerfEstExpr();
  ~PerfEstExpr();

  /* Get the parent node of a subexpression.
  */
  PerfEstExprHandle handle_parent(PerfEstExprHandle h);

  /* Create an exact clone of a PerfEstExpr.
  */
  PerfEstExpr *clone();

  /* Clone part of a PerfEstExpr.
  */
  PerfEstExprHandle clone_handle(PerfEstExprHandle h);

  /* Read/write the object to/from a database port.
  */
  int write(FormattedFile& port);
  int read(FormattedFile& port);

  /* Print the object to a file or terminal for debugging purposes.
  */
  void print(FILE *fp);

  /*
   * Set the root handle. Should be used with care (this could be a source
   * of memory leaks). Get the root handle.
   */
  void set_root(PerfEstExprHandle rhandle);
  PerfEstExprHandle get_root();

  /* Routines used for constructing an expression.
  */
  PerfEstExprHandle	c_constant(double const_value);
  PerfEstExprHandle	c_bottom();
  PerfEstExprHandle	c_sum(PerfEstExprHandle s1, PerfEstExprHandle s2);
  PerfEstExprHandle	c_ifstmt(double branch_prob,
				 PerfEstExprHandle cond_expr,
				 PerfEstExprHandle true_branch,
				 PerfEstExprHandle false_branch);
  PerfEstExprHandle	c_loop(int node_number,
			       PerfEstExprHandle loop_body,
			       PerfEstExprHandle lower_bound,
			       PerfEstExprHandle upper_bound,
			       PerfEstExprHandle loop_step,
			       int iterations_guess,
			       int is_parallelizable, int is_marked_parallel,
			       double parallel_time, int best_num_procs,
			       int astindex, char *labelstring);
  PerfEstExprHandle	c_variable(char *variable_name);
  PerfEstExprHandle	c_call(int node_number, char *f_name,
			       int num_actuals, PerfEstExprNode **actuals_arr);

  /*
   * Evaluate the specified expression to an actual number. May have to guess
   * along the way (if so, "is_precise" is set to 0).
   */
  void			get_estimate(PerfEstExprHandle handle,
				     double& returned_estimate,
				     int& is_precise);

  /* Get information about a loop.
  */
  void			get_loop_info_estimates(PerfEstExprHandle handle,
						int& niterations,
						double& bodycost,
						int& is_precise);

  void			set_parallel_info(PerfEstExprHandle handle, 
					  double parallel_etime,
					  int best_nprocs);
  
  /*
   * For a loop, return the estimate of the best number of processors
   * to use for that loop.
   */
  void			estimate_best_num_procs(PerfEstExprHandle handle,
						int& returned_estimate,
						int& is_precise);

  /*
   * Do constant folding and other simplification on an expression and
   * return a new expression. The old tree is unmodified. 
   */
  PerfEstExprHandle	simplify_expr(PerfEstExprHandle handle,
				      SymExprNode_mapnamefunc mapfunc = 0,
				      void *user = 0);

  /*
   * Simplify an entire expression. Take care -- this will invalidate any
   * PerfEstExprHandle's into the existing expression.
   */
  void			simplify_root(SymExprNode_mapnamefunc mapfunc = 0,
				      void *user = 0);

  /*
   * Perform "call substition" on an expression. Note that this is a 
   * destructive operation -- the PerfEstExpr object which this is
   * invoked on is modified. 
   */
  void substitute_call(char *called_fname,
		       char **formals_list,
		       int nformals,
		       PerfEstExpr *callee_expr);

  /*
   * Given a cost expression for a called routine, substitute in acutals
   * for the formals of the caller. This is a destructive process (the
   * data structure on which we invoke this method is modified).
   */
  void substitute_actuals_for_formals(char **formals_list,
				      int nformals,
				      PerfEstExprNode *actualnode);
  
  /* Ask if an expression is bottom
  */
  int is_bottom(PerfEstExprHandle handle) { return handle->is_bottom(); };

};

typedef PerfEstExprNode *PerfEstExprNode_ptr;

#define IO_CHAR_START_PERFESTEXPRNODE '['
#define IO_CHAR_END_PERFESTEXPRNODE ']'

/* Name of IP local info file for perfe est
 */
#define PE_IPFILE_NAME "performance-estimate"

/* Name of IP constants file written by dan grove's code
*/
#define PE_DGROVE_CONSTS_FILE_NAME "dgrove-constants"

/* Name of IP constants file derived from PFC
*/
#define PE_PFC_CONSTS_FILE_NAME "pfc-constants"

/* Strings used in error messages
*/
#define PE_IPFILE_STR "performance estimation initial information"
#define PE_CONSTS_FILE_STR "interprocedural constants"

#endif PerfEstExpr_h_included
