/* $Id: builtins.h,v 1.13 1997/03/11 14:29:25 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef builtins_h
#define builtins_h


/***********************************************************************
 *  builtins.h:                                                        *
 *    interface to tables of information about Fortran 77 intrinsic    *
 *    and generic functions                                            *
 *                                                                     *
 *  author: John Mellor-Crummey                           May, 1992    *
 *                                                                     *
 * Copyright 1992, Rice University, as part of the ParaScope           *
 * Programming Environment Project                                     *
 ***********************************************************************/

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef forttypes_h
#include <libs/frontEnd/ast/forttypes.h>
#endif

/* NOTE: 
 *  in the intrinsic_descriptor and generic_descriptor structures
 *  defined below, 
 *  if the number of arguments = NARGS_MORE_THAN_2, it means >= 2 
 *  arguments
 *
 *  if the number of arguments = NARGS_CMPLX_1_OR_2, this 
 *  indicates a special situation for the generic CMPLX which can 
 *  have two arguments of type integer, real, double precision, 
 *  or a single argument of type complex.
 *
 *  otherwise, the number of arguments specifies 
 *  the particular number of arguments required.
 *
 *
 */
#define NARGS_MORE_THAN_2    0
#define NARGS_CMPLX_1_OR_2  -1
#define GENERIC_DESC_NARGS 4

#define MAX_INTRINSICS_PER_GENERIC 5

typedef struct {
  char *name;   /* name of the intrinsic function                  */
  int  numArgs; /* number of arguments                             */
  int  argType; /* type of arguments                               */
  int  resType; /* result type                                     */
} intrinsic_descriptor; 

typedef struct {
  char *name;   /* name of the generic function                    */
  int  resType; /* result type (TYPE_UNKNOWN => same as args)      */
  int  numArgs; /* number of arguments                             */
  int  args[GENERIC_DESC_NARGS]; /* valid argument types (padded with TYPE_UNKNOWN) */
} generic_descriptor;

typedef struct {
  char *name;
  char *names[MAX_INTRINSICS_PER_GENERIC];
} generic_to_intrinsic_descriptor;

/****************************************************************************
 * intrinsic_descriptor *builtins_intrinsicFunctionInfo:
 *   returns a structure that describes the attributes of a Fortran intrinsic 
 *   function 
 ***************************************************************************/
EXTERN(intrinsic_descriptor *, builtins_intrinsicFunctionInfo, (char *name));


/****************************************************************************
 * generic_descriptor *builtins_genericFunctionInfo:
 *   returns a structure that describes the attributes of a Fortran generic 
 *   function 
 ***************************************************************************/
EXTERN(generic_descriptor *, builtins_genericFunctionInfo, (char *name));


/****************************************************************************
 * Boolean builtins_isGenericFunction:
 *   returns true if "name" is the name of a Fortran generic function
 ***************************************************************************/
EXTERN(Boolean, builtins_isGenericFunction, (char *name));


/****************************************************************************
 * Boolean builtins_isIntrinsicFunction:
 *   returns true if "name" is the name of a Fortran intrinsic function
 ***************************************************************************/
EXTERN(Boolean, builtins_isIntrinsicFunction, (char *name));


/****************************************************************************
 * Boolean builtins_isBuiltinFunction:
 *   returns true if "name" is the name of a Fortran intrinsic or generic 
 *   function 
 ***************************************************************************/
EXTERN(Boolean, builtins_isBuiltinFunction, (char *name));


/****************************************************************************
 * Boolean  builtins_acceptsDifferentTypedArgs:
 *   returns true if "name" is the name of a Fortran intrinsic function
 *   which accepts arguments which may have different types 
 ***************************************************************************/
EXTERN(Boolean, builtins_acceptsDifferentTypedArgs, (char *name));


/****************************************************************************
 * int builtins_numGenerics:
 *   returns the number of Fortan generic functions.
 ***************************************************************************/
EXTERN(int, builtins_numGenerics, (char *name));


/****************************************************************************
 * int builtins_numIntrinsics:
 *   returns the number of Fortan intrinsic functions.
 ***************************************************************************/
EXTERN(int, builtins_numIntrinsics, (char *name));


/****************************************************************************
 * generic_descriptor *builtins_genericFunctionInfo_bynumber:
 *   returns a structure that describes the attributes of a Fortran generic 
 *   function. Used for iterating through all generic functions. Example:
 *     for (i = 0; i < builtins_numGenerics(); i++)
 *        ...  builtins_genericFunctionInfo_bynumber(i) ...
 ***************************************************************************/
EXTERN(generic_descriptor*, builtins_genericFunctionInfo_bynumber,
		(int number));


/****************************************************************************
 * intrinsic_descriptor *builtins_intrinsicFunctionInfo_bynumber:
 *   returns a structure that describes the attributes of a Fortran intrinsic 
 *   function. Used for iterating through all intrinsic functions. Example:
 *     for (i = 0; i < builtins_numIntrinsics(); i++)
 *        ...  builtins_intrinsicFunctionInfo_bynumber(i) ...
 ***************************************************************************/
EXTERN(intrinsic_descriptor*, builtins_intrinsicFunctionInfo_bynumber,
		(int number));

/****************************************************************************
 * char *builtins_genericToIntrinsic:
 *   returns the name of the intrinsic that corresponds to the version of
 *   the generic function invoked with args of type argType to return a
 *   value of type resType
 ***************************************************************************/
EXTERN(char*, builtins_genericToIntrinsic, 
                  (char* name, int argType, int resType));

/****************************************************************************
 * int generic_result_type(generic_descriptor *ginfo, int argType)
 *   returns the result type for a generic function, given the
 *   generic_descriptor for the function and the type of the
 *   arguments
 ***************************************************************************/
EXTERN(int, generic_result_type, (generic_descriptor* ginfo, int argType));



#endif /* builtins_h */
