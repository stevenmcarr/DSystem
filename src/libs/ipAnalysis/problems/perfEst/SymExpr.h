/* $Id: SymExpr.h,v 1.8 1997/03/11 14:35:13 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * This header file defines a "symbolic expression" class. This class is an
 * abstraction for simple symbolic expression trees involving constants,
 * variables, and arithmetic operators. Leaf nodes may be variables,
 * constants, or "bottom" (indicates an unknown quanitity). Interior nodes
 * may be *, +, - (unary and binary), and /.
 * 
 * Why write a class such as this when you can just use the AST? The AST is a
 * fairly heavyweight abstraction, and is costly to read in and write out.
 * In addition, AST routine assume that you are going to be working with a
 * single tree read from a single file -- if you want to read bits of your
 * tree from different files, you're out of luck.
 * 
 * An example of an application of this class might be creating symbolic jump
 * functions for interprocedural constant propagation. The performance
 * estimator relies on this class for writing out symbolic expressions for
 * actuals at call sites.
 * 
 * Numeric constants in this tree may be integer or floating point. Semantics
 * for arithmetic operations (+*-/) are as follows: if both operands are
 * integers, then the result will be an integer. If either operand is
 * floating point, then both operands will be converted to floating point,
 * a floating point operation will be performed, and the result will be
 * floating point. Division by 0 is defined as 0, so for example the
 * expression "1 / 0" returns 0.
 * 
 * The expression simplification routines may still be used even by a derived
 * class, which might add different types of nodes. A derived class can
 * mark a node as derived by setting the 'atype' field o of the node to
 * SYMEXPR_ARITH_DERIVED. When this class goes to try to simplify an
 * expression, say "(- X 1)" where X is a node of type DERIVED, then it
 * will just give up and return the original expression. Note that the
 * assumption is made that addition is still associative, even in the
 * presence of derived nodes, i.e. the expression (+ 1 (+ 1 X)) <where X
 * is a derived node) will simplify to (+ 2 X).
 * 
 * The treatment of "bottom" in this class is somewhat peculiar, for
 * historical reasons. Any arithmetic expression involving only base class
 * nodes simplifies to bottom. Example: (+ BOT 3), (- VAR BOT), and (+ (+
 * 1 2) BOT) all simplify to BOT. Derived nodes, however, are treated as
 * sacrosanct -- an expression involving BOT and a derived node will not
 * be simplified. Example: (+ BOT X) <where X is derived> simplifies to (+
 * BOT X). This behavior can be overridden in derived classes by altering
 * the virtual function "simplify_node()", however.
 * 
 * This class is designed to be inherited from. It inherits from
 * NonUniformDegreeTreeWithDBIO.
 * 
 * Author: N. McIntosh
 * 
 * Copyright 1992, Rice University, as part of the ParaScope Programming
 * Environment Project
 * 
 */

#ifndef SymExpr_h_included
#define SymExpr_h_included

#ifndef rn_string_h
#include <libs/support/strings/rn_string.h>
#endif

#ifndef forttypes_h
#include <libs/frontEnd/ast/forttypes.h>
#endif

#ifndef ionudtree_h
#include <libs/ipAnalysis/ipInfo/ionudtree.h>
#endif

/*
 * Types of "arithmetic expression" nodes in the expression abstraction.
 * If a node is of type SYMEXPR_ARITH_DERIVED, then it is treated as
 * un-simplifiable; expressions involving nodes of this type are not
 * simplified (example -- "(+ 2 <derived>)").
 */

typedef enum _e_SymExprNT {
  SYMEXPR_ARITH_NONE=0,		/* dummy for catching uninitialized objects */
  SYMEXPR_ARITH_BOTTOM=1,	/* an unknown quantity */
  SYMEXPR_ARITH_DERIVED=2,	/* indentifies node as from derived class */
  SYMEXPR_ARITH_CONST=3,	/* a numeric constant */
  SYMEXPR_ARITH_VAR=4,		/* a variable */
  SYMEXPR_ARITH_ADD,		/* addition */
  SYMEXPR_ARITH_SUB,		/* subtraction */
  SYMEXPR_ARITH_MULT,		/* multiplication */
  SYMEXPR_ARITH_DIV,		/* division */
  SYMEXPR_LOG_NOT,		/* logical NOT */
  SYMEXPR_LOG_AND,		/* logical AND */
  SYMEXPR_LOG_OR,		/* logical OR */
  SYMEXPR_COND_EQ,		/* conditional EQ */
  SYMEXPR_COND_NE,		/* conditional NE */
  SYMEXPR_COND_LT,		/* conditional LT */
  SYMEXPR_COND_GT,		/* conditional GT */
  SYMEXPR_COND_LE,		/* conditional LE */
  SYMEXPR_COND_GE,		/* conditional GE */
  } SymExprNodeType;

/* Node data for arithmetic expression nodes
*/

typedef union _u_SymExprArithData {
  char *name;		/* for variables */
  int iconst;		/* integer constants */
  double rconst;	/* floating point constants */
} SymExprArithData;

/* Forward declarations
*/
class SymExprNode;

/*
 * A pointer to a "mapping" function which maps variables to values. A
 * function of this type must behave as follows. It is passed variable
 * name in 'name'. If it can map the variable to an integer value, then it
 * returns TYPE_INTEGER and places the variable's value in "ival". If it
 * can map the variable to a floating point value, it returns TYPE_REAL
 * and places the value in 'fval'. If the variable cannot be mapped to a
 * value, it returns TYPE_UNKNOWN.
 */

typedef int (*SymExprNode_mapnamefunc)(char *name, int &ival, double &fval,
				    void *user);

/*
 * A pointer to a function which is used during expression substitution.
 */
typedef SymExprNode *ptrto_SymExprNode;
typedef ptrto_SymExprNode (*SymExprNode_substfunc)
                              (SymExprNode *node, void *user);

/* The class definition itself
*/

class SymExprNode : public NonUniformDegreeTreeNodeWithDBIO {

 private:

  /*
   * Data items at a node. If you add to this list, make sure you update
   * the method 'copynode', among others.
   */
  SymExprNodeType atype;	/* type of this node */
  SymExprArithData arith;	/* data */
  int dtype;			/* value type */
  int shelper;			/* flag for use in simplification */

  /*
   * The following routine performs construction of SymExprNode objects;
   * it is called by the various constructors. It is not intended to be
   * called from anything other than a constructor. Arguments are
   * meaningful only in the context of the specified node type. The
   * following table describes which arguments are looked at depending on
   * the type. An 'X' indicates that the argument is used; an empty space
   * indicates that the argument is ignored.
   * 
   * type     dtype         i_const  r_const  leftson   rightson   new_parent
   * ---      -----         ------   -------  -------   --------   ---------
   * CONST    TYPE_INTEGER     X                                       X
   * CONST    TYPE_REAL                X                               X
   * <arith>                                     X         X           X
   * <cond>                                      X         X           X
   * <log>                                       X         X           X
   * NOT                                         X                     X
   * DERIVED                                                           X
   * BOTTOM                                                            X
   *
   * where <arith> = ADD/SUB/MULT/DIV, <cond> = EQ/NE/GT/LT,..
   * and <log> = AND/OR
   */
  void Construct(SymExprNodeType a_type, int d_type = TYPE_UNKNOWN,
	    int i_const = 0, double r_const = 0.0,
	    char *vname = 0, SymExprNode *leftson = 0,
	    SymExprNode *rightson = 0, SymExprNode *new_parent = 0);

  /*
   * Vanilla constructor -- not for general use. See the comments for this
   * routine in the source.
   */
  SymExprNode(SymExprNode *new_parent = 0);

  /* Helpers for simplification
   */
  void node_to_cvals(int &constinfo, int &iconst, double &rconst);
  SymExprNode *cvals_to_node(int constinfo, int iconst, double rconst);
  SymExprNode *test_and_sum(SymExprNode *l, SymExprNode *r);
  void sum_constant_terms(int &constinfo, int &iconst, 
			  double &rconst, int cil, int icl, double rcl,
			  int cir, int icr, double rcr);
  SymExprNode *combine_cvals_and_node(SymExprNode *n, int constinfo, 
				      int iconst,  double rconst);
  SymExprNode *simplify_add(Boolean &deletethis, 
			    SymExprNode *orig,
			    int &constinfo, int &iconst, 
			    double &rconst, SymExprNode_mapnamefunc mapfunc,
			    void *user);

  SymExprNode *simplify_internal(int &constinfo, int &iconst, 
				 double &rconst,
				 SymExprNode_mapnamefunc mapfunc,
				 void *user);

  SymExprNode *eval_op();

 public:

  /* Virtual destructor for supporting derived classes
  */
  virtual		~SymExprNode();

  /*
   * Constructors of various flavors. All of these call the private
   * routine 'Construct()'.
   */
  SymExprNode(int i_const, SymExprNode *new_parent = 0);
  SymExprNode(double r_const, SymExprNode *new_parent = 0);
  SymExprNode(SymExprNodeType a_type, SymExprNode *new_parent = 0);
  SymExprNode(char *vname, SymExprNode *new_parent = 0);
  SymExprNode(SymExprNodeType a_type, SymExprNode *leftson,
	      SymExprNode *rightson, SymExprNode *new_parent = 0);

  /* Give a node a new parent. */
  void link(SymExprNode *new_parent);

  /*
   * The routine "simplify_node" uses virtual functions when it creates
   * nodes during siplification of arithmetic expressions. These virtual
   * functions can be overridden by derived classes if the derived class
   * wishes to take advantage of SymExprNode::simplify_node().
   */
  virtual SymExprNode *NewIconst(int i_const);
  virtual SymExprNode *NewRconst(double r_const);
  virtual SymExprNode *NewArithOp(SymExprNodeType a_type,
				  SymExprNode *leftson,
				  SymExprNode *rightson);
  virtual SymExprNode *NewCondOp(SymExprNodeType a_type,
				 SymExprNode *leftson,
				 SymExprNode *rightson);
  virtual SymExprNode *NewVar(char *vname);
  virtual SymExprNode *NewNode(SymExprNodeType a_type);

  /* Clone a node. Ignores the node's children (i.e. the created node will
   * have no children). This has to be a virtual function in order for
   * the 'simplify' method to be useful for derived classes.
   */
  virtual SymExprNode *nodeclone();

  /*
   * Copy one node to another. The contents of the current node are
   * copied to the node 'target'. The children of the node are unaffected.
   * Node data will be duplicated, but children will not.
   */
  virtual void copynode(SymExprNode *target);

  /* Create a clone of a tree and return it. This is a deep copy.
  */
  SymExprNode *clone();

  /* Be very careful with SetNodeType().  */
  void			SetNodeType(SymExprNodeType typ) { atype = typ; };
  SymExprNodeType	GetNodeType() { return atype; };
  int			GetConstDataType();
  int			GetIntegerConstVal();
  double		GetRealConstVal();
  char			*GetVarName();

  /* For getting at the children of internal nodes such as +, *, etc. */
  SymExprNode *leftson();
  SymExprNode *rightson();

  /*
   * Helper function for use when a tree is simplified. Evaluates the
   * given node and returns a new, hopefully simplified node. The
   * additional parameters are as follows. The 'constinfo' flag is used to
   * indicate that the expression being passed back has a constant added
   * to it. If constinfo == TYPE_UNKNOWN, then no constant is being passed
   * back. If constinfo == TYPE_INTEGER, then an integer constant is being
   * returned in 'iconst'. If constinfo == TYPE_REAL, then a
   * double-precision floating point constant is being returned in
   * 'rconst'. If the return value is NULL, this means that the result of
   * the simplification is only a constant. If the return value is
   * non-null, and a constant is being passed back, then this indicates an
   * implied "add" of the constant to the non-constant term.
   * 
   * The flag 'deletethis' is set to true if the caller should delete the
   * node once the routine returns (this is to avoid using the code
   * "delete this;" in the method).
   * 
   * Examples: if you were to invoke this method on the expression (+ 3 4),
   * constinfo would be set to TYPE_INTEGER, iconst would be set to 7,
   * rconst would be undefined, deletethis would be set to true
   * (indicating that the old expression, (+ 3 4), can be thrown away),
   * and the return value would be NULL.
   * 
   * The optional 'mapfunc' argument is a function which maps names to values
   * (see def for SymExprNode_mapnamefunc above). The value passed in
   * 'user' will be passed to the mapfunc each time it is called.
   */
  SymExprNode *
    simplify_node_internal(Boolean &deletethis,
			   int &constinfo,
			   int &iconst, 
			   double &rconst,
			   SymExprNode_mapnamefunc mapfunc = 0,
			   void *user = 0);
  virtual SymExprNode *
    simplify_node(Boolean &deletethis,
		  SymExprNode_mapnamefunc mapfunc = 0,
		  void *user = 0);

  
  /*
   * Return a simplified (i.e. constant folded) copy of this tree.
   * Returns a completely new tree -- NO CHANGES ARE MADE TO THE ORIGINAL
   * TREE. This function is not virtual, but it calls 'simplify_node()',
   * which is virtual.
   */
  SymExprNode *simplify(SymExprNode_mapnamefunc mapfunc = 0, void *user = 0);

  /*
   * Try to evaluate the expression. If the tree can be evaluated to an
   * integer, then TYPE_INTEGER is returned and 'ival' is filled in with
   * the result of the evaluation. If the tree can be evaluated to a
   * floating point number, then TYPE_REAL is returned and 'fval' is
   * filled in with the result of the evaluation. If the tree cannot be
   * evaluated, then TYPE_UNKNOWN is returned.
   */
  int eval(int &ival, double &fval,
	   SymExprNode_mapnamefunc mapfunc = 0, void *user = 0);

  /*
   * Perform some sort of arbitrary substution on an expression. The type
   * of substitution performed is determine by the function pointer passed
   * to this routine. See the header comment for the routine to find out
   * about how to use it. Note that this routine returns a new tree; the
   * current tree is left untouched.
   */
  SymExprNode *substitute(SymExprNode_substfunc, void *user);

  /*
   * Perform variable substitution on a given expression. Given a variable
   * 'x', an expression 't', and a second expression, 's', find call
   * occurrences of x in t and change them to s. Example: subsituting "(+
   * 3 y)" for all ocurrences of "z". in "(+ z (- 4 (* z z)))" yields "(+
   * (+ 3 y) (- 4 (* (+ 3 y) (+ 3 y))))". This function returns a
   * completely new tree -- the old tree is left unmodified.
   */
  SymExprNode *substitute_var(char *var, SymExprNode *subst_expr);

  /* Print the expression to a file. This is for debugging. Note that
   * if the 'print_node()' is defined correctly in derived classes,
   * then the derived class can use the base class's print routine.
   */
  virtual char **print_node(FILE *fp);
  void print(FILE *fp, int indent_level = 0, Boolean indent = true);

  friend void print_SymExprNode(SymExprNode *p);
  friend Boolean compare(SymExprNode &, SymExprNode &);

  /* Read/write the tree to/from a database port.
  */
  int ReadUpCall(FormattedFile& port);
  int WriteUpCall(FormattedFile& port);

  /* The following function is required by the 'read()' function in
   * the NonUniformDegreeTreeNodeWithDBIO class. Since this class
   * has others derived from it, it has to be virtual as well.
   */
  virtual NonUniformDegreeTreeNodeWithDBIO *
    New(NonUniformDegreeTreeNodeWithDBIO *new_parent) {
      return new SymExprNode((SymExprNode *) new_parent);
    };

  /*
   * Override some of our inherited methods so we don't have to sprinkle
   * the code with casts in order to avoid warnings.
   */
  SymExprNode *FirstChild() {
    return (SymExprNode *) NonUniformDegreeTreeNode::FirstChild();
  };
  SymExprNode *Parent() {
    return (SymExprNode *) NonUniformDegreeTreeNode::Parent();
  };
  SymExprNode *NextSibling() {
    return (SymExprNode *) NonUniformDegreeTreeNode::NextSibling();
  };
/* The following are additions made by Johnny Chen (8-3-94)
 * The == operator traverses the whole expression tree and 
 * checks to see if all the elements are the same.  If they
 * are, the function will return 1, 0 otherwise.
 * The = operator will deep copy a node-tree
 */
  int operator==(SymExprNode &);
  int operator!=(SymExprNode &);  
  SymExprNode & operator=(SymExprNode &);
};

typedef SymExprNode *SymExprNode_ptr;

#define IO_CHAR_START_SYMEXPRNODE '('
#define IO_CHAR_END_SYMEXPRNODE ')'

#endif SymExpr_h_included
