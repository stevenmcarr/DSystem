/* $Id: val.i,v 2.16 1997/03/11 14:36:18 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * -- val.i
 * 
 *          This file is the header file for abstract value graph
 *		construction and manipulation.
 *
 */


#ifndef val_i
#define val_i

#include <libs/support/misc/general.h>
#include <math.h>
#include <libs/moduleAnalysis/valNum/val.h>
#include <libs/frontEnd/ast/ast.h>
#include <libs/frontEnd/ast/forttypes.h>
#include <libs/frontEnd/ast/cd_branch.h>
#include <libs/support/tables/StringHashTable.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#include <libs/support/memMgmt/mem.h>
#include <assert.h>
#include <libs/moduleAnalysis/valNum/val_ht.h>

typedef Generic ValField;
class FormattedFile;  // minimal external declaration

inline Boolean val_is_value(ValNumber v)
{
    /*
     *  Not NIL, BOTTOM, or TOP
     */
    return BOOL(v > VAL_TOP);
}

/*
 *  The structure for an entry in the value table.  We malloc this so that
 *  the size of the field[] member can vary.
 *
 *  Hash on:
 *	type	The type of value (constant, arithmetic op, etc.)
 *	expType	Fortran data type of the value (INTEGER, REAL, LOGICAL, etc.)
 *	level	Loop-variance level
 *	field	array of fields -- last one unhashed for some ValType's
 *
 *  Ignore while hashing:
 *	constRange	Value number for constant range/bounds
 *			(VAL_NIL if not computed yet)
 */
struct ValEntry {
    ValNumber	constRange;		// Ignore while hashing.

    ValField	field[1];		// Hash on most fields.

    unsigned int hash(int size);	// return value in [0,size)
    int compare(ValEntry &v2);		// return 0 if equal
    ValEntry *Read(FormattedFile &port);
    void Write(FormattedFile &port);
};

extern ValEntry *val_new(ValType type, int arity = 0);
extern void val_delete(ValEntry *ve);

/*
 *  Fields of Value Entries
 *
 *  These are implemented as a set of ve_xxx(ValEntry &ve) inline functions.
 *
 *  B == Boolean
 *  I == integer
 *  V == ValNumber
 *  ID == VAL_TEXT for name
 *  N == Node Number
 *  E == ExpType
 *  C == CdBranchType
 *  VO == ValOpType
 *  G == Generic/ValField (depends on cfgval/ipval usage)
 *  u! == field not included in matching
 *  blank fields should be VAL_NIL
 *
 *  for all types with enough fields
 *	field[0] is expType	expression type
 *			see gi.h and typetable.h in /rn/src/fort-include
 *			TYPE_UNKNOWN, TYPE_INTEGER, TYPE_REAL, 
 *			TYPE_DOUBLE_PRECISION, TYPE_COMPLEX, TYPE_LOGICAL, 
 *			TYPE_LABEL, TYPE_CHARACTER, TYPE_ERROR, TYPE_EXACT, 
 *			TYPE_LAST
 *
 *	field[1] is level	level of loop variance
 *			0	for loop-invariant
 *			L	for Mu at loop header of nesting depth L
 *			L-1	for Eta of value exiting loop of depth L
 *			otherwise, maximum level of argument values
 *
 *  Don't hash on expType and level for ip values (ENTRY, RETURN, DUMMY).
 *
 *  field[2](type) #fields  field[3]  field[4] field[5] field[6]    field[7]
 *  -------------- -------  --------  -------- -------- --------    ------
 *  VAL_CONST      4        const I
 *  VAL_TEXT       3+string string------characters------> ...
 *  VAL_RANGE      8        lo V      hi V     align V  step V      simple B
 *  VAL_IVAR       5        flipped B bounds V
 *  VAL_VARIANT    4        occur G
 *  VAL_ENTRY      7        entry ID  name ID  offset I length I u!
 *  VAL_RETURN     8        call N    name ID  offset I length I u! input V u!
 *  VAL_OP         arity+5  opType VO arity I  kid[0] ...
 *  VAL_INTRINSIC  arity+5  entry ID  arity I  kid[0] ...
 *  VAL_PURE_FN    arity+5  entry ID  arity I  kid[0] ...
 *  VAL_PHI        arity+5  stmt G    arity I  kid[0] ...
 *  VAL_GAMMA      arity+5  test V    arity I  kid[0] ...
 *  VAL_LIST       arity+5  VAL_NIL   arity I  kid[0] ...
 *  VAL_TEST       6        occur G   itest V  testType I
 *  VAL_ETA        5        test V    final V
 *  VAL_OK_MU      5        iter V    init V
 *  VAL_VAR_MU     5        occur G   init V   varIter V u!
 *  VAL_MU_PH      3
 *  VAL_DUMMY      7        state V   name ID  offset I length I u!
 *  VAL_ARRAY      6        state V   rhs V    subs VAL_LIST
 *
 *  The opType values are the AST node type for corresponding tree ops.
 *
 *  VAL_OP
 *	An arithmetic or other operator normally represented in the tree.
 *	The opType is a ValOpType for tree op or intrinsic.
 *
 *  VAL_CONST
 *	A constant -- logical, single char, or numeric -- that may be
 *	accurately represented as an integer.  Logical is represented
 *	as 0 or 1, single char in ASCII.  Operations on constants are
 *	evaluated according to the true type (expType of the ValEntry)
 *	except when operations on real constants would overflow or go
 *	rational. 
 *
 *  VAL_TEXT
 *	A constant that cannot be accurately represented as an integer. 
 *	The string gives its textual representation. 
 *
 *  VAL_RANGE
 *	The set of integers i such that
 *		lo <= i <= hi
 *		i mod step == align mod step	(if align or step is symbolic)
 *		i mod step == align		(if align and step are constant)
 *
 *	If simple is true, then either lo <= hi or the range is empty. 
 *
 *  VAL_IVAR
 *	Represents the value of a loop index variable.  The bounds field
 *	gives the range of the index.  If flipped is true, then the index
 *	runs from hi to lo. 
 *
 *  VAL_VARIANT
 *	An unanalyzable value, such as from a READ statement. 
 *	The name of the variable and the location (SSA node for cfgval)
 *	where the definition occurs are given. 
 *
 *  VAL_ENTRY
 *	Value on entry to a procedure.  Fields give the string table index
 *	for the entry name and for the variable name (formal var name or
 *	common block name), plus the offset and length of the variable
 *	if in a common block. 
 *
 *  VAL_IP_BASE
 *	Just like VAL_ENTRY except that this represents the base of the
 *	fist of a set of variables related by pairwise linear equalities.
 *	Each of the variables in the set is computed as a linear expression
 *      in this value.  The fields give information for a variable
 *	representative of the set (the first variable in the set, in the
 *	order that the ip REF iterator gives the variables).
 *
 *  VAL_RETURN
 *	Value on return from a procedure call.  The name is that of the 
 *	formal parameter, local variable, or common block.  The input field
 *	gives the value of this variable passed to the call. 
 *
 *  VAL_INTRINSIC
 *  VAL_PURE_FN
 *	Support precise handling of intrinsics and side-effect-free user
 *	functions.  The name of the function is given by entry -- note that
 *	somebody else is responsible for distinguishing between intrinsics
 *	and user functions with the same name (but we can never have both
 *	visible in the current procedure).  The kid array gives value numbers
 *	for passed values explicit arguments and referenced common block
 *	variables. 
 *
 *  VAL_PHI
 *	Represents a merged value; can only match with identical merges
 *	at the same stmt (CFG node for cfgvals).  Kids are ordered 
 *	according to the order of inedges to the merge.
 *
 *  VAL_TEST
 *	Used when the test for a GAMMA or ETA is not a VAL_IVAR, or not
 *	logical (from logical or block IF), or not from a COMPUTED GOTO.
 *	Most of the remaining types of tests are intimately tied to control
 *	flow -- then occur gives the place where the test is (SSA node for
 *	cfgvals).
 *
 *  VAL_GAMMA
 *	Represents a merged value with the test controlling the merge.
 *	Can match identical merges even at different statements, if the
 *	test is also the same.  Kids are indexed by the edge label from
 *	the test corresponding to that value's reaching the merge.
 *
 *  VAL_ETA
 *	Loop-variant value on exit from loop.  The test field gives the 
 *	loop-exit predicate; final gives the value on exit.
 *
 *  VAL_OK_MU
 *	Loop-variant value, with initial value and iterative value --
 *	which is either constant, or else depends on itself in a nice
 *	way, indicated by embedding a Mu placeholder in the iterative
 *	value.  Some MUs are converted to VAL_IVARs, if the iterative 
 *	value is nice enough for recognition as an auxiliary induction
 *	variable.  Other MUs are converted to VAL_VARIANTs, because 
 *	their iterative value is confusing (the function used to compute
 *	it is loop-variant).
 *
 *  VAL_VAR_MU
 *	Loop-variant value, with initial value and iterative value that
 *	depends on itself in a loop-variant way.  Essentially the same
 *	as a VAL_VARIANT, but have some chance of refining in the 
 *	presence of more information, such as after IP MOD analysis.
 *
 *  VAL_MU_PH
 *	Placeholder to represent Mu value while building iterative 
 *	value.  Used only when iterative value ends up being pretty
 *	simple.
 * 
 *  VAL_DUMMY
 *	Reference to hidden common block var or other global without local
 *	MODs.  Modeled as selection of value from the reaching definition
 *	of DUMMY_GLOBAL.  "state" is the reaching def of DUMMY_GLOBAL,
 *	the remaining fields describe the global variable.
 *
 *  VAL_ARRAY
 *	Subscripted array definition or use.  For use, represents 
 *	a scalar value derived from the input array (previous array value
 *	given by "state") and the subscripts (a list given by "subs").
 *	For definition, represents an array value derived from the input
 *	array, the subscripts, and the "rhs" value which replaces the 
 *	element value indicated by the subscripts.  Dimension of the 
 *	array is given by "arity" field of the subscript VAL_LIST.
 *
 *  VAL_LIST
 *	Resurrected for representing array subscripts.
 */

/*
 *  These map a ValEntry (e) to a field
 */
inline ValField	&ve_cRange(ValEntry &e)	    { return e.constRange; }
inline ValField	&ve_expType(ValEntry &e)    { return e.field[0]; }
inline ValField	&ve_level(ValEntry &e)	    { return e.field[1]; }
inline ValType	&ve_type(ValEntry &e)    { return *((ValType *)&(e.field[2])); }
inline ValField	&ve_const(ValEntry &e)	    { return e.field[3]; }
inline char	*ve_string(ValEntry &e)	    { return (char *)&(e.field[3]); }
inline ValField	&ve_lo(ValEntry &e)	    { return e.field[3]; }
inline ValField	&ve_hi(ValEntry &e)	    { return e.field[4]; }
inline ValField	&ve_align(ValEntry &e)	    { return e.field[5]; }
inline ValField	&ve_step(ValEntry &e)	    { return e.field[6]; }
inline ValField	&ve_simple(ValEntry &e)	    { return e.field[7]; }
inline Boolean	&ve_flipped(ValEntry &e) { return *((Boolean *)&(e.field[3])); }
inline ValField	&ve_bounds(ValEntry &e)	    { return e.field[4]; }
inline ValField	&ve_occur(ValEntry &e)	    { return e.field[3]; }
inline ValField	&ve_name(ValEntry &e)	    { return e.field[4]; }
inline ValField	&ve_entry(ValEntry &e)	    { return e.field[3]; }
inline ValField	&ve_call(ValEntry &e)	    { return e.field[3]; }
inline ValField	&ve_offset(ValEntry &e)	    { return e.field[5]; }
inline ValField	&ve_length(ValEntry &e)	    { return e.field[6]; }
inline ValField	&ve_input(ValEntry &e)	    { return e.field[7]; }
inline ValOpType &ve_opType(ValEntry &e) {return *((ValOpType *)&(e.field[3]));}
inline ValField	&ve_stmt(ValEntry &e)	    { return e.field[3]; }
inline ValField	&ve_arity(ValEntry &e)	    { return e.field[4]; }
inline ValField	&ve_kid(ValEntry &e, int i) { return e.field[i+5]; }
inline ValField	&ve_itest(ValEntry &e)	    { return e.field[4]; }
inline ValField	&ve_testType(ValEntry &e)   { return e.field[5]; }
inline ValField	&ve_test(ValEntry &e)	    { return e.field[3]; }
inline ValField	&ve_final(ValEntry &e)	    { return e.field[4]; }
inline ValField	&ve_iter(ValEntry &e)	    { return e.field[3]; }
inline ValField	&ve_init(ValEntry &e)	    { return e.field[4]; }
inline ValField	&ve_varIter(ValEntry &e)    { return e.field[5]; }
inline ValField	&ve_left(ValEntry &e)	    { return ve_kid(e,0); }
inline ValField	&ve_right(ValEntry &e)      { return ve_kid(e,1); }
inline ValField	&ve_state(ValEntry &e)	    { return e.field[3]; }
inline ValField	&ve_rhs(ValEntry &e)	    { return e.field[4]; }
inline ValField	&ve_subs(ValEntry &e)	    { return e.field[5]; }

inline Boolean VAL_bogus(ValEntry *e)
    { return BOOL((ve_type(*e) <= VAL_TOP_TYPE) ||
		  (ve_type(*e) >= VAL_BOT_TYPE)); }

inline Boolean VAL_bogus(ValEntry &e)
    { return BOOL((ve_type(e) <= VAL_TOP_TYPE) ||
		   (ve_type(e) >= VAL_BOT_TYPE)); }

/*
 *  external functions for value graph builders only
 */
/*
 *  from values.c
 */
EXTERN(void, val_swap, 			(ValNumber* val1,
					 ValNumber* val2));

EXTERN(ExpType, val_op_exptype,		(ValTable &V, ValOpType op,
					 ValNumber left, ValNumber right));
/*
 *  from val_simplify.c
 */
EXTERN(ValNumber, val_simplify,		(ValTable &V,
					 ValEntry* value));

/*
 *  from val_lookup.c
 */
EXTERN(ValNumber, val_lookup_const,	(ValTable &V,
					 ExpType expType,
					 int constVal,
					 Boolean insert = true));

EXTERN(ValNumber, val_lookup_text,	(ValTable &V,
					 ExpType expType,
					 char *name,
					 Boolean insert = true));

EXTERN(ValNumber, val_lookup_phi,	(ValTable &V,
					 ExpType expType,
					 ValField stmt,
					 unsigned int arity,
					 ValNumber kids[],
					 Boolean insert = true));

EXTERN(ValNumber, val_lookup_gamma,	(ValTable &V,
					 ExpType expType,
					 ValNumber test,
					 unsigned int arity,
					 ValNumber kids[],
					 Boolean insert = true));

EXTERN(ValNumber, val_lookup_intrinsic,	(ValTable &V,
					 ExpType expType,
					 char *name,
					 unsigned int arity,
					 ValNumber kids[],
					 Boolean insert = true));

EXTERN(ValNumber, val_lookup_pure,	(ValTable &V,
					 ExpType expType,
					 char *name,
					 unsigned int arity,
					 ValNumber kids[],
					 Boolean insert = true));

EXTERN(ValNumber, val_lookup_eta, 	(ValTable &V,
					 ValNumber test,
					 ValNumber final,
					 Boolean insert = true));

EXTERN(ValNumber, val_lookup_variant, 	(ValTable &V,
					 ExpType expType,
					 Generic occur,
					 int level,
					 Boolean insert = true));

EXTERN(ValNumber, val_lookup_entry,	(ValTable &V,
					 ExpType expType,
					 char *entry,
					 char *name,
					 int offset,
					 int length,
					 Boolean insert = true));
//  shorthand version
//
EXTERN(ValNumber, val_lookup_ip_entry,	(ValTable &V,
					 char *entry,
					 char *name,
					 int offset));
//  linear base gottenn from ip constraints
//
EXTERN(ValNumber, val_lookup_ip_base,	(ValTable &V,
					 ExpType expType,
					 char *entry,
					 char *name,
					 int offset,
					 int length,
					 Boolean insert = true));

EXTERN(ValNumber, val_lookup_return,	(ValTable &V,
					 ExpType expType,
					 int level,
					 ValField call,
					 char *name,
					 int offset,
					 int length,
					 ValNumber input,
					 Boolean insert = true));
EXTERN(ValNumber, val_lookup_ip_return,	(ValTable &V,
					 ValField call,
					 char *name,
					 int offset));

EXTERN(ValNumber, val_lookup_range, 	(ValTable &V,
					 ValNumber lo,
					 ValNumber hi,
					 ValNumber align,
					 ValNumber step,
					 Boolean insert = true));

EXTERN(ValNumber, val_lookup_ivar, 	(ValTable &V,
					 int level,
					 ValNumber range,
					 Boolean flipped,
					 Boolean insert = true));

EXTERN(ValNumber, val_lookup_test,	(ValTable &V,
					 ValNumber itest,
					 int type,
					 ValField occur,
					 Boolean insert = true));

EXTERN(ValNumber, val_lookup_dummy,	(ValTable &V,
					 ExpType expType,
					 ValNumber state,
					 char *name,
					 int offset,
					 int length,
					 Boolean insert = true));

EXTERN(ValNumber, val_lookup_array,	(ValTable &V,
					 ExpType expType,
					 ValNumber state,
					 ValNumber rhs,
					 ValNumber subs,
					 Boolean insert = true));

EXTERN(ValNumber, val_lookup_list,	(ValTable &V,
					 unsigned int arity,
					 ValNumber kids[],
					 Boolean insert = true));
/* from val_range.c */
EXTERN(ValNumber, val_get_cRange,	(ValTable &V,
					 ValEntry *ve));
EXTERN(ValNumber, val_merge_two, 	(ValTable &V,
					 ValNumber valNum1,
					 ValNumber valNum2));
EXTERN(ValNumber, val_standardize,	(ValTable &V,
					 ValEntry * valNode));

/* */
EXTERN(ValNumber, val_lookup_ok_mu, 	(ValTable &V,
					 ValNumber init,
					 ValNumber iter,
					 int level,
					 Boolean insert = true));

EXTERN(ValNumber, val_lookup_var_mu, 	(ValTable &V,
					 ValField occur,
					 ValNumber init,
					 int level,
					 Boolean insert = true));

EXTERN(ValNumber, val_lookup_mu_ph, 	(ValTable &V,
					 int level,
					 Boolean insert = true));

EXTERN(ValNumber, val_lookup_op,	(ValTable &V,
					 ExpType expType,
					 ValOpType opType,
					 unsigned int arity,
					 ValNumber kids[],
					 Boolean insert = true));

EXTERN(ValType, val_get_val_type, (Values Vp, ValNumber vn));
EXTERN(char *, val_get_const_text, (Values Vp, ValNumber vn));
EXTERN(int, val_table_max, (Values Vp));
extern const char *valType[];
extern const char **valOpType;

/*
 *  Returns base after subtracting constant part and dividing by largest
 *  constant divisor of symbolic part.  Added constant and constant coeff
 *  are set.
 */
EXTERN(ValNumber, val_get_base,		(ValTable &V,
					 ValNumber vn,
					 int &coeff,
					 int &added));
/*
 *  Returns constant added part and largest constant divisor of symbolic 
 *  part, without building the base value number.
 */
EXTERN(void, val_const_parts,		(ValTable &V,
					 ValNumber vn,
					 int &coeff,
					 int &added));

EXTERN(void, val_merge_steps,	(ValTable &V, ValNumber &newS, ValNumber &newA,
				 ValNumber s1, ValNumber s2,
				 ValNumber a1, ValNumber a2));

EXTERN(ValNumber, val_div_floor, (ValTable *V, ValNumber L, ValNumber R));
EXTERN(ValNumber, val_div_ceil, (ValTable *V, ValNumber L, ValNumber R));
#endif /* ! val_i */
