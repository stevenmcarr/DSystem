/* $Id: FortTree.i,v 1.19 1997/03/11 14:29:48 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef FortTree_i
#define FortTree_i

/* $Header */
/************************************************************************/
/*									*/
/*	ned_cp/FortTree/FortTree.i					*/
/*									*/
/*	FortTree -- a Fortran AST with context-sensitive semantics	*/
/*									*/
/************************************************************************/




#include <libs/frontEnd/fortTree/FortTree.h>


/************************/
/*  Semantic Error Codes*/
/************************/

/* These match up with the table declared in FortTree.c */

#define ft_NO_ERROR	                    0
#define ft_CANNOT_PARSE			    (ft_NO_ERROR + 1)
#define ft_INCOMPLETE		            (ft_CANNOT_PARSE + 1)
#define ft_VM_FORTRAN			    (ft_INCOMPLETE + 1)
#define ft_PARASCOPE		            (ft_VM_FORTRAN + 1)
#define ft_RN		                    (ft_PARASCOPE + 1)
#define ft_NOT_INTRINSIC		    (ft_RN + 1)
#define ft_DUPLICATE_DECLARATION	    (ft_NOT_INTRINSIC + 1)
#define ft_BAD_TYPE			    (ft_DUPLICATE_DECLARATION + 1)
#define ft_ILLEGAL_INVOCATION		    (ft_BAD_TYPE + 1)
#define ft_INVALID_TYPE_LENGTH		    (ft_ILLEGAL_INVOCATION + 1)
#define ft_TYPE_LEN_TOO_COMPLEX		    (ft_INVALID_TYPE_LENGTH + 1)
#define ft_TYPE_FAULT		            (ft_TYPE_LEN_TOO_COMPLEX + 1)
#define ft_WRONG_NO_ARGS	            (ft_TYPE_FAULT + 1)
#define ft_INVALID_COMPLEX_CONSTANT	    (ft_WRONG_NO_ARGS + 1)
#define ft_WRONG_NO_SUBSCRIPTS		    (ft_INVALID_COMPLEX_CONSTANT + 1)
#define ft_BAD_TYPE_IN_DO	            (ft_WRONG_NO_SUBSCRIPTS + 1)
#define ft_STMT_OUT_OF_ORDER	            (ft_BAD_TYPE_IN_DO + 1)
#define ft_ADJUSTABLE_DIMENSION_ERROR	    (ft_STMT_OUT_OF_ORDER + 1)
#define ft_BAD_DIMENSION_SPECIFIER          (ft_ADJUSTABLE_DIMENSION_ERROR + 1)
#define ft_DUPLICATE_LABEL_DECLARATION	    (ft_BAD_DIMENSION_SPECIFIER + 1)
#define ft_BAD_IMPLICIT_SPECIFIER           (ft_DUPLICATE_LABEL_DECLARATION +1)
#define ft_EQUIV_TWO_COMMON_BLOCKS          (ft_BAD_IMPLICIT_SPECIFIER + 1)
#define ft_INVALID_VARIABLE_IN_EQUIV        (ft_EQUIV_TWO_COMMON_BLOCKS + 1)
#define ft_UNEXPECTED_DATA_TYPE		    (ft_INVALID_VARIABLE_IN_EQUIV + 1)
#define ft_INCONSISTENT_EQUIVALENCES	    (ft_UNEXPECTED_DATA_TYPE + 1)
#define ft_RECURSIVE_PARAMETER_DEFN	    (ft_INCONSISTENT_EQUIVALENCES + 1)
#define ft_UNEXPECTED_TYPE_IN_EXPR	    (ft_RECURSIVE_PARAMETER_DEFN + 1)
#define ft_NON_CONST_IN_CONST_INT_EXPR	    (ft_UNEXPECTED_TYPE_IN_EXPR + 1)
#define ft_NON_INT_IN_CONST_INT_EXPR	    (ft_NON_CONST_IN_CONST_INT_EXPR +1)
#define ft_COMMON_VAR_NAME_CONFLICT	    (ft_NON_INT_IN_CONST_INT_EXPR + 1)
#define ft_INTRINSIC_AND_EXTERNAL	    (ft_COMMON_VAR_NAME_CONFLICT + 1)
#define ft_DUPL_INTRINSIC		    (ft_INTRINSIC_AND_EXTERNAL + 1)
#define ft_IMPLICIT_OVERLAP		    (ft_DUPL_INTRINSIC + 1)
#define ft_MULT_ARRAY_DECLARATORS           (ft_IMPLICIT_OVERLAP + 1)
#define ft_INTRINSIC_AS_STFUNC_ARG	    (ft_MULT_ARRAY_DECLARATORS + 1)
#define ft_ENTRY_IS_DUMMY_ARG		    (ft_INTRINSIC_AS_STFUNC_ARG + 1)
#define ft_ENTRY_IS_EXTERNAL		    (ft_ENTRY_IS_DUMMY_ARG + 1)
#define ft_ENTRY_CONTEXT		    (ft_ENTRY_IS_EXTERNAL + 1)
#define ft_TOO_MANY_DIMS		    (ft_ENTRY_CONTEXT + 1)
#define ft_ARRAY_UPPER_LT_LOWER		    (ft_TOO_MANY_DIMS + 1)
#define ft_FORMAL_IN_EQUIVALENCE	    (ft_ARRAY_UPPER_LT_LOWER + 1)
#define ft_EQUIV_NON_CONST_SUBSCRIPT	    (ft_FORMAL_IN_EQUIVALENCE + 1)
#define ft_CHAR_LEN_SPEC_REQUIRED	    (ft_EQUIV_NON_CONST_SUBSCRIPT + 1)
#define ft_BAD_CHAR_CONST_EXPR		    (ft_CHAR_LEN_SPEC_REQUIRED + 1)
#define ft_PARAM_BAD_ENTITY		    (ft_BAD_CHAR_CONST_EXPR + 1)
#define ft_ICHAR_ARG_LEN	            (ft_PARAM_BAD_ENTITY + 1)
#define ft_CHAR_FN_LEN_UNDEFINED	    (ft_ICHAR_ARG_LEN + 1)
#define ft_DUPLICATE_ARGUMENTS		    (ft_CHAR_FN_LEN_UNDEFINED + 1)
#define ft_EQUIV_MULT_DEFINED		    (ft_DUPLICATE_ARGUMENTS + 1)
#define ft_EQUIV_NEG_EXTEND_COMMON          (ft_EQUIV_MULT_DEFINED + 1)
#define ft_UNEXPECTED_NODE_TYPE		    (ft_EQUIV_NEG_EXTEND_COMMON + 1)
#define ft_MANIFEST_CONST_REDEFINITION	    (ft_UNEXPECTED_NODE_TYPE + 1)
#define ft_BAD_STMT_IN_BLOCK_DATA           (ft_MANIFEST_CONST_REDEFINITION +1)
#define ft_INVOCATION_NAME_CONFLICT	    (ft_BAD_STMT_IN_BLOCK_DATA + 1)
#define ft_WRONG_STFUNC_ARG_COUNT	    (ft_INVOCATION_NAME_CONFLICT + 1)
#define ft_STFUNC_ARG_TYPE_MISMATCH	    (ft_WRONG_STFUNC_ARG_COUNT + 1)
#define ft_STFUNC_CHAR_LEN_MISMATCH	    (ft_STFUNC_ARG_TYPE_MISMATCH + 1)
#define ft_INCOMPAT_TYPES_IN_ASSIGN	    (ft_STFUNC_CHAR_LEN_MISMATCH + 1)
#define ft_LEN_MUST_BE_INT_CONST_EXP	    (ft_INCOMPAT_TYPES_IN_ASSIGN + 1)
#define ft_PRIVATE_DECL_FOR_NON_DATA	    (ft_LEN_MUST_BE_INT_CONST_EXP + 1)
#define ft_PRIVATE_DUPLICATE_DEF	    (ft_PRIVATE_DECL_FOR_NON_DATA + 1)
#define ft_PRIVATE_IN_BAD_CONTEXT	    (ft_PRIVATE_DUPLICATE_DEF + 1)
#define ft_PRIVATE_DECL_FOR_PARAM	    (ft_PRIVATE_IN_BAD_CONTEXT + 1)
#define ft_PRIVATE_DECL_FOR_COMMON	    (ft_PRIVATE_DECL_FOR_PARAM + 1)
#define ft_INVALID_SUBSTRING		    (ft_PRIVATE_DECL_FOR_COMMON + 1)
#define ft_PROC_PARAM_NO_EXTERNAL	    (ft_INVALID_SUBSTRING + 1)
#define ft_PARAM_IS_STFUNC		    (ft_PROC_PARAM_NO_EXTERNAL + 1)
#define ft_PROC_NAME_INVALID_IN_EXPR	    (ft_PARAM_IS_STFUNC + 1)
#define ft_NO_SUBR_CALL_IN_EXPR		    (ft_PROC_NAME_INVALID_IN_EXPR + 1)
#define ft_NO_FUNC_INVOC_IN_CALL	    (ft_NO_SUBR_CALL_IN_EXPR + 1)
#define ft_BUILTIN_INCOMPAT_ARG_TYPES	    (ft_NO_FUNC_INVOC_IN_CALL + 1)
#define ft_BUILTIN_INVALID_ARG_TYPE	    (ft_BUILTIN_INCOMPAT_ARG_TYPES + 1)
#define ft_BUILTIN_WRONG_NARGS		    (ft_BUILTIN_INVALID_ARG_TYPE + 1)
#define ft_FORMAL_IN_STATIC		    (ft_BUILTIN_WRONG_NARGS + 1)
#define ft_NONDATA_IN_STATIC		    (ft_FORMAL_IN_STATIC + 1)
#define ft_STMT_FUNC_ORDER                  (ft_NONDATA_IN_STATIC + 1)
#define ft_STMT_FUNC_ARGS_NOT_IDENTS        (ft_STMT_FUNC_ORDER +1)
#define ft_LABEL_REF_NOT_DEFINED            (ft_STMT_FUNC_ARGS_NOT_IDENTS +1)
#define ft_CONSTANT_IN_COMMON               (ft_LABEL_REF_NOT_DEFINED + 1)
#define ft_COMMON_BLOCK_UNDEFINED           (ft_CONSTANT_IN_COMMON +1)

#define ft_STMT_NOT_IN_SMP_DIALECT          (ft_COMMON_BLOCK_UNDEFINED +1)
#define ft_STMT_NOT_ALLOWED_IN_LOGICAL_IF   (ft_STMT_NOT_IN_SMP_DIALECT +1)
#define ft_STMT_NOT_ALLOWED_IN_CONTEXT      (ft_STMT_NOT_ALLOWED_IN_LOGICAL_IF +1)

#define ft_DEF_CONFLICTS_WITH_PRIOR_USE     (ft_STMT_NOT_ALLOWED_IN_CONTEXT +1)
#define ft_USE_CONFLICTS_WITH_PRIOR_DEF     (ft_DEF_CONFLICTS_WITH_PRIOR_USE +1)
#define ft_USE_CONFLICTS_WITH_PRIOR_USE     (ft_USE_CONFLICTS_WITH_PRIOR_DEF +1)

#define ft_RESTRICTION_STMT_FUNC_NO_PROC_CALLS (ft_USE_CONFLICTS_WITH_PRIOR_USE +1)
#define ft_RESTRICTION_STMT_FUNC_ARG_CONTAINS_PROC_CALL (ft_RESTRICTION_STMT_FUNC_NO_PROC_CALLS +1)

#define ft_BAD_ERROR_CODE                   (ft_RESTRICTION_STMT_FUNC_ARG_CONTAINS_PROC_CALL +1)
#define ft_LAST_SEMANTIC_ERROR	            (ft_BAD_ERROR_CODE + 1)	
	/* added for FortTree.c	   */

EXTERN(void, tc_ERROR,
	 (SymDescriptor d, AST_INDEX node, int code));
EXTERN(void, ft_SetSemanticErrorCode,
	 (FortTree ft, FortTreeNode node, short value));
EXTERN(void, ft_SetSemanticErrorForStatement,
	 (FortTree ft, FortTreeNode node, int value));
EXTERN(char *, ft_SemanticErrorMessage, (int code));
EXTERN(AST_INDEX, get_name_astindex_from_ref, (AST_INDEX evar));
EXTERN(Boolean,	isFunctionInvocation, (FortTreeNode node));
EXTERN(Boolean,	isStatement, (FortTreeNode node));

EXTERN(void, ft_InitNeedProvs, (FortTree ft));
EXTERN(void, ft_BuildNeedProvs, (FortTree ft, FortTreeNode node, SymDescriptor d));

#endif /* FortTree_i */
