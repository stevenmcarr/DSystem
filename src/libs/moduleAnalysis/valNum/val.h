/* $Id: val.h,v 2.24 1997/06/25 15:13:17 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef val_h
#define val_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef context_h
#include <libs/support/database/context.h>
#endif

#ifndef val_enum_h
#include <libs/moduleAnalysis/valNum/val_enum.h>
#endif

#ifndef dt_h
#include <libs/moduleAnalysis/dependence/dependenceTest/dep_dt.h>
#endif


/*
 *  val.h
 *
 *  Public declarations for the symbolic analysis and value numbering 
 *  routines.
 */

typedef FUNCTION_POINTER(void, ValPrintFn, (Generic callback, ValNumber v));

/*
 *  Value number types
 */
typedef enum {
    VAL_TOP_TYPE,  /* type for VAL_TOP */
    VAL_CONST,	   /* integral logical/numeric */
    VAL_TEXT,	   /* non-integral constant -- gives textual form */
    VAL_RANGE,	   /* {i | (L <= i <= U) & ((i mod S)==(A mod S))} */
    VAL_IVAR,	   /* varies inductively with a loop */
    VAL_VARIANT,   /* unknowable value (e.g., read from a file) */
    VAL_ENTRY,     /* variable (formal, common, static) value on entry */
    VAL_RETURN,    /* variable (actual, common, static) value after call */
	           /* nil variable is the return value if an impure fn */
    VAL_IP_BASE,   /* used in building value number from interprocedural */
                   /* relations (pairwise between variables) */
    VAL_OP,	   /* tree operator or intrinsic -- opType ValOpType */
    VAL_INTRINSIC, /* Value returned by builtin/intrinsic function */
    VAL_PURE_FN,   /* Value returned by pure (side-effect free) user function */
    VAL_PHI,	   /* value at unstructured merge */
    VAL_GAMMA,	   /* value at merge with predicate (built on GSA form only) */
    VAL_LIST,      /* a list of values, such as array subscripts */
    VAL_TEST,	   /* non-Boolean test with type for use by gamma */
    VAL_ETA,	   /* value at loop exit with predicate (GSA/SSA form) */
    VAL_OK_MU,	   /* value at loop entry merge (GSA form and our SSA form) */
    VAL_VAR_MU,	   /* value at loop entry merge (GSA form and our SSA form) */
    VAL_MU_PH,	   /* placeholder for Mu (GSA form and our SSA form) */
    VAL_DUMMY,     /* reference to ip variable with no local MODs */
    VAL_ARRAY,     /* a subscripted array reference */
    VAL_BOT_TYPE   /* type for VAL_BOTTOM */
} ValType;

typedef enum {
    VAL_OP_PLUS = VAL_BOT_TYPE + 1,
    VAL_OP_TIMES,
    VAL_OP_MINUS,
    VAL_OP_DIVIDE,
    VAL_OP_EXP,		/* x ** y */
    VAL_OP_NOT,
    VAL_OP_AND,
    VAL_OP_OR,
    VAL_OP_GE,
    VAL_OP_GT,
    VAL_OP_LE,
    VAL_OP_LT,
    VAL_OP_EQ,
    VAL_OP_NE,
    VAL_OP_CONC,
    VAL_OP_ABS,		/* start of intrinsics */
    VAL_OP_MOD,
    VAL_OP_MAX,
    VAL_OP_MIN,
    VAL_OP_DIM,		/* positive difference max(a-b,0) */
    VAL_OP_SIGN,	/* transfer of sign abs(a)*(b/abs(b)) */
    VAL_OP_LEN,
    VAL_OP_INT,		/* truncate */
    VAL_OP_NINT,	/* round */
    VAL_OP_REAL,
    VAL_OP_DOUBLE
} ValOpType;


/*
 *  Return values of the comparison between value numbers or 
 *  sign of value number.
 *  There are three bits -- 
 *	first bit set means free to assume value is not negative
 *	second bit set "     "   "   "     "     "   "  zero
 *	third bit set  "     "   "   "     "     "   "  positive
 *
 *  Note that TOP is free to be any value, so it has sign ANY which
 *  can be freely interpreted as positive, negative, or zero (or not).
 */
typedef enum {
    VAL_SIGN_UNKNOWN = 0,     VAL_CMP_UNKNOWN = VAL_SIGN_UNKNOWN,
    VAL_SIGN_NONNEG  = 4,     VAL_CMP_GE      = VAL_SIGN_NONNEG,
    VAL_SIGN_NONZERO = 2,     VAL_CMP_NE      = VAL_SIGN_NONZERO,
    VAL_SIGN_NONPOS  = 1,     VAL_CMP_LE      = VAL_SIGN_NONPOS,
    VAL_SIGN_ZERO    = 4 | 1, VAL_CMP_EQ      = VAL_SIGN_ZERO,
    VAL_SIGN_POS     = 4 | 2, VAL_CMP_GT      = VAL_SIGN_POS,
    VAL_SIGN_NEG     = 2 | 1, VAL_CMP_LT      = VAL_SIGN_NEG,
    VAL_SIGN_ANY     = 4|2|1, VAL_CMP_ANY     = VAL_SIGN_ANY
} ValSign;

/*
 *  Note that these macros take a ValSign -- returned by 
 *  val_sign() or val_compare() -- see below
 */
#define VAL_is_unknown(s)  BOOL(((int) s) == ((int) VAL_SIGN_UNKNOWN) )
#define VAL_is_nonzero(s)  BOOL(((int) s) & ((int) VAL_SIGN_NONZERO))
#define VAL_is_nonpos(s)   BOOL(((int) s) & ((int) VAL_SIGN_NONPOS) )
#define VAL_is_nonneg(s)   BOOL(((int) s) & ((int) VAL_SIGN_NONNEG) )
#define VAL_is_zero(s)     BOOL(VAL_is_nonneg(s) && VAL_is_nonpos(s))
#define VAL_is_neg(s)      BOOL(VAL_is_nonzero(s) && VAL_is_nonpos(s))
#define VAL_is_pos(s)      BOOL(VAL_is_nonzero(s) && VAL_is_nonneg(s))

#define VAL_is_ne(s)       (VAL_is_nonzero(s))
#define VAL_is_ge(s)       (VAL_is_nonneg(s))
#define VAL_is_le(s)       (VAL_is_nonpos(s))
#define VAL_is_eq(s)       (VAL_is_zero(s))
#define VAL_is_gt(s)       (VAL_is_pos(s))
#define VAL_is_lt(s)       (VAL_is_neg(s))

/* 
 * External Functions 
 */
/*
 *  from val_main.c
 */
EXTERN(Values, val_Open,	(void) );		/* create and init */
EXTERN(void, val_Close,		(Values Vp) );		/* destroy and free */
EXTERN(void, val_Dump,		(Values Vp) );		/* print table */
EXTERN(void, val_print_entry,	(Values Vp, ValNumber valNum)); /* print one */

/*
 *  Classifying value numbers (VAL_TOP and VAL_BOTTOM are separate)
 */
EXTERN(Boolean, val_is_const, (Values Vp, ValNumber v));
EXTERN(int, val_get_const,    (Values Vp, ValNumber v));

EXTERN(Boolean, val_is_sym,   (Values Vp, ValNumber v)); /* not proven const */

/*
 *  Fortran type of value (TYPE_INTEGER, etc.)
 */
EXTERN(ExpType, val_get_exp_type, (Values Vp, ValNumber v));
/*
 *  Do a type conversion (evaluated if safe, otherwise represented
 *  structurally)
 */
EXTERN(ValNumber, val_convert, (Values Vp, ValNumber v, ExpType et));
/*
 *  Type of computation (VAL_OP, VAL_GAMMA, etc.)
 */
EXTERN(ValType, val_get_val_type, (Values Vp, ValNumber v));
/*
 *  Loop-variance level (outside of all loops is 0, count in by natural loops)
 */
EXTERN(ExpType, val_get_level, (Values Vp, ValNumber v));

/*
 *  Range information.
 *
 *  The range for a value number v is a quad [L:U^S@A] such that 
 *  any actual value i assumed by v at run time satisfies
 *
 *	(L <= i <= U) & (emod(i,S) == emod(A,S))
 *
 *		where emod is the Euclidean mod:
 *		    #define emod(j,k) (((j % k) >= 0)? (j % k) : ((j % k) + k))
 *		(for constant S and A, (emod(i,S) == A)).
 *
 *  All but the explicit "sym" (for symbolic) forms return conservative 
 *  constant values.  The symbolic forms only work for linear expressions
 *  in single loop inductive values [(c*i + k), c and k constant, i inductive].
 *  (If called on another value number v, they return lo==v, hi==v, align==v,
 *  step==VAL_TOP.)
 */
EXTERN(ValNumber, val_get_lo, 	 (Values Vp, ValNumber v) );
EXTERN(ValNumber, val_get_hi, 	 (Values Vp, ValNumber v) );
EXTERN(ValNumber, val_get_align, (Values Vp, ValNumber v) );
EXTERN(ValNumber, val_get_step,  (Values Vp, ValNumber v) );
EXTERN(ValNumber, val_sym_lo, 	 (Values Vp, ValNumber v) );
EXTERN(ValNumber, val_sym_hi, 	 (Values Vp, ValNumber v) );
EXTERN(ValNumber, val_sym_align, (Values Vp, ValNumber v) );
EXTERN(ValNumber, val_sym_step,  (Values Vp, ValNumber v) );
/*
 *  Get constant bounds, alignment, stride all in one package
 */
EXTERN(ValNumber, val_get_const_range, (Values Vp, ValNumber v));

/*
 *  These support many tree operations and implicit functions.
 *  Mapping between ValOpType and AST operator types is provided
 *  by cfgval stuff.
 */
EXTERN(ValNumber, val_unary, 		(Values Vp,
					 ValOpType op,
					 ValNumber valNum));
EXTERN(ValNumber, val_binary, 		(Values Vp,
					 ValOpType op,
					 ValNumber valNum1,
					 ValNumber valNum2));
/*
 *  from val_arith.c
 *  Arithmetic operators -- guaranteed to return exact result for constants,
 *	best possible approximation for VAL_RANGE (should have constant
 *	bounds).  For symbolics, see detail for each function.  Results
 *	are VAL_BOTTOM (VAL_TOP) if any argument is VAL_BOTTOM (VAL_TOP).
 */
EXTERN( ValSign, val_sign, 		(Values Vp,
					 ValNumber valNum) );
	/*  result for symbolic depends on precision with which we can
	 *  compute constant bounds.
	 */
EXTERN( ValSign, val_compare, 		(Values Vp,
					 ValNumber valNum1,
					 ValNumber valNum2) );
	/*  result for symbolics depends on results of symbolic differencing
	 *  and/or comparison of constant bounds.
	 */
EXTERN( ValNumber, val_max, 		(Values Vp,
					 ValNumber valNum1,
					 ValNumber valNum2) );
	/*  If we cannot determine which argument is larger, a value
	 *  number representing the VAL_OP_MAX operation is returned.
	 */
EXTERN( ValNumber, val_min, 		(Values Vp,
					 ValNumber valNum1,
					 ValNumber valNum2) );
	/*  If we cannot determine which argument is larger, a value
	 *  number representing the VAL_OP_MIN operation is returned.
	 */
EXTERN( ValNumber, val_mod, 		(Values Vp,
					 ValNumber valNum1,
					 ValNumber valNum2) );
	/*  If valNum1 % valNum2 cannot be evaluated to a constant,
	 *  and the inputs were non-range symbolics, then a value
	 *  number representing the VAL_OP_MOD operation (Fortran 
	 *  intrinsic MOD) is returned.  In range cases, VAL_BOTTOM
	 *  is frequently returned.
	 */
EXTERN( ValNumber, val_emod, 		(Values Vp,
					 ValNumber valNum1,
					 ValNumber valNum2) );
	/*  If the Euclidean mod cannot be evaluated to a constant,
	 *  VAL_BOTTOM is returned.
	 */
EXTERN( ValNumber, val_gcd, 		(Values Vp,
					 ValNumber valNum1,
					 ValNumber valNum2) );
	/*  Both symbolic and constant greatest common divisors may
	 *  be evident.  If the constant GCD divides the symbolic one,
	 *  the symbolic one is returned; otherwise the constant one.
	 */
EXTERN( ValNumber, val_merge, 		(Values Vp,
					 ValNumber valNum1,
					 ValNumber valNum2) );
	/*  If the values are identical, this returns that value number,
	 *  otherwise, it builds an approximate constant range guaranteed
	 *  to contain any values that valNum1 or valNum2 may assume.
	 */

/*
 *  Symbolic range for induction variable (built from loop bounds).
 */
EXTERN(ValNumber, val_get_ivar_range, 	(Values Vp, ValNumber));
/*
 *  True if ending bound was proven less than starting bounds, so 
 *  that loop was run backwards and bounds have been flipped to
 *  standardize the range.
 */
EXTERN(Boolean, val_is_ivar_flipped, 	(Values Vp, ValNumber));
/*
 *  True if the "lo" bound has been proven less than the "hi" bound or
 *  the step proven positive -- so that the range either runs forward
 *  or is empty.
 */
EXTERN(Boolean, val_is_range_simple, 	(Values Vp, ValNumber));

/*
 *  get a value number for the integer...
 */
EXTERN(ValNumber, val_const, 	(Values Vp, int constval));

/*
 *  Handle to subscripts for array reference
 */
EXTERN(ValNumber, val_get_subs, (Values val, ValNumber valNum));


/*
 *  DEPENDENCE TESTING STUFF:
 *	Parse a value number into coefficients of loop index variables and
 *	variant terms at each level.
 */
typedef struct CoVarPairStructTag
{
    int coeff;		/* coefficient of loop index variable */
    ValNumber sym;	/* non-inductive variant symbolics at same level */
} CoVarPair;

EXTERN(CoVarPair *, val_dep_parse, (Values Vp, ValNumber v, int level));
	/*
	 *  Return vector of CoVarPair objects.  Return_value[0].coeff
	 *  gives the constant added part of the subscript, 
	 *  return_value[0].sym gives the loop-invariant symbolic 
	 *  part.  Where there is no symbolic part, the sym field is
	 *  given as VAL_ZERO.
	 *  Length of pair vector is max(level, variance level of v),
	 *  where d is the depth of the deepest containing loop.
	 */

EXTERN(void, val_dep_free, (CoVarPair *cv));
	/*
	 *  Free the array of CoVarPair structures returned by 
	 *  val_dep_parse().
	 */

EXTERN(void, val_dep_parse_sub, (Values Vp, ValNumber v,
				 int level, Subs_data *sub));

EXTERN(void, val_dep_parse_loop_bound, (Values Vp, ValNumber v, int level,
					AST_INDEX node, Expr *expr, int **vec));

#endif /* !val_h */
