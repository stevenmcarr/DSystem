/* $Id: builtins.C,v 1.2 2001/09/17 00:21:33 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/***********************************************************************/
/*  builtins.c:                                                        */
/*    this file contains                                               */
/*    (1) tables of information about intrinsic and generic functions  */
/*    (2) query functions to access the information in the tables      */
/*                                                                     */
/*  author: John Mellor-Crummey                           May, 1992    */
/*          (based on tables developed by Keith Cooper as part of the  */
/*           original ParaScope TypeChecker)                           */
/*                                                                     */
/*  NOTE:                                                              */
/*      the tables are sorted alphabetically so that they can be       */
/*      searched using binary search                                   */
/*                                                                     */
/*  modified: Alan Carle                                  Aug, 1992    */
/*  To provide mapping from generic function names to particular       */
/*  intrinsic function based on argument type.                         */
/*                                                                     */
/* Copyright 1992, Rice University, as part of the ParaScope           */
/* Programming Environment Project                                     */
/***********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <strings.h>
#include <string.h>

#include <libs/frontEnd/ast/builtins.h>

/******************************************/
/* some shorthand for types in the tables */
/******************************************/

#define RE	TYPE_REAL
#define IN	TYPE_INTEGER
#define DB	TYPE_DOUBLE_PRECISION
#define CM	TYPE_COMPLEX
#define CH	TYPE_CHARACTER
#define UN	TYPE_UNKNOWN
#define LO	TYPE_LOGICAL
  
/**************************************************/
/* table describing Fortran77 intrinsic functions */
/**************************************************/

static intrinsic_descriptor intrinsic_table[] = {
"abs",		1, RE, RE,
"acos",		1, RE, RE,
"aimag",	1, CM, RE,
"aint",		1, RE, RE,
"alog",		1, RE, RE,
"alog10",	1, RE, RE,
"amax0",	NARGS_MORE_THAN_2, IN, RE,
"amax1",	NARGS_MORE_THAN_2, RE, RE,
"amin0",	NARGS_MORE_THAN_2, IN, RE,
"amin1",	NARGS_MORE_THAN_2, RE, RE,
"amod",		2, RE, RE,
"anint",	1, RE, RE,
"asin",		1, RE, RE,
"atan",		1, RE, RE,
"atan2",	2, RE, RE,
"cabs",		1, CM, RE,
"ccos",		1, CM, CM,
"cexp",		1, CM, CM,
"char",		1, IN, CH,
"clog",		1, CM, CM,
"conjg",	1, CM, CM,
"cos",		1, RE, RE,
"cosh",		1, RE, RE,
"cshift",	NARGS_MORE_THAN_2, UN, UN,	/* F90 intrinsic */
"csin",		1, CM, CM,
"csqrt",	1, CM, CM,
"dabs",		1, DB, DB,
"dacos",	1, DB, DB,
"dasin",	1, DB, DB,
"datan",	1, DB, DB,
"datan2",	2, DB, DB,
"dcos",		1, DB, DB,
"dcosh",	1, DB, DB,
"ddim",		2, DB, DB,
"dexp",		1, DB, DB,
"dim",		2, RE, RE,
"dint",		1, DB, DB,
"dlog",		1, DB, DB,
"dlog10",	1, DB, DB,
"dmax1",	NARGS_MORE_THAN_2, DB, DB,
"dmin1",	NARGS_MORE_THAN_2, DB, DB,
"dmod",		2, DB, DB,
"dnint",	1, DB, DB,
"dprod",	2, RE, DB,
"dsign",	2, DB, DB,
"dsin",		1, DB, DB,
"dsinh",	1, DB, DB,
"dsqrt",	1, DB, DB,
"dtan",		1, DB, DB,
"dtanh",	1, DB, DB,
"eoshift",	NARGS_MORE_THAN_2, UN, UN,	/* F90 intrinsic */
"exp",		1, RE, RE,
"float",	1, IN, RE,
"iabs",		1, IN, IN,
"iand",		2, IN, IN,	/* IBM extension */
"ichar",	1, CH, IN,
"idim",		2, IN, IN,
"idint",	1, DB, IN,
"idnint",	1, DB, IN,
"ieor",		2, IN, IN,	/* IBM extension */
"ifix",		1, RE, IN,
"index",	2, CH, IN,
"int",		1, RE, IN,
"ior",		2, IN, IN,	/* IBM extension */
"ishft",	2, IN, IN,	/* IBM extension */
"isign",	2, IN, IN,
"len",		1, CH, IN,
"lge",		1, CH, LO,
"lgt",		1, CH, LO,
"lle",		1, CH, LO,
"llt",		1, CH, LO,
"max0",		NARGS_MORE_THAN_2, IN, IN,
"max1",		NARGS_MORE_THAN_2, RE, IN,
"min0",		NARGS_MORE_THAN_2, IN, IN,
"min1",		NARGS_MORE_THAN_2, RE, IN,
"mod",		2, IN, IN,
"nint",		1, RE, IN,
"not",		1, IN, IN,	/* IBM extension */
"real",		1, IN, RE,
"sign",		2, RE, RE,
"sin",		1, RE, RE,
"sinh",		1, RE, RE,
"sngl",		1, DB, RE,
"sqrt",		1, RE, RE,
"tan",		1, RE, RE,
"tanh",		1, RE, RE
};

/************************************************/
/* table describing Fortran77 generic functions */
/************************************************/

static generic_descriptor generic_table[] = {
"abs",		UN, 1, IN, RE, DB, CM, 
"acos",		UN, 1, RE, DB, UN, UN,
"aint",		UN, 1, RE, DB, UN, UN,
"anint",	UN, 1, RE, DB, UN, UN,
"asin",		UN, 1, RE, DB, UN, UN,
"atan",		UN, 1, RE, DB, UN, UN,
"atan2",	UN, 2, RE, DB, UN, UN,
"cmplx",	CM, NARGS_CMPLX_1_OR_2, IN, RE, DB, CM, 
"cos",		UN, 1, RE, DB, CM, UN,
"cosh",		UN, 1, RE, DB, UN, UN,
/*"cshift",	UN, NARGS_MORE_THAN_2, IN, RE, DB, CM,*/
"dble",		DB, 1, IN, RE, DB, CM, 
"dim",		UN, 2, IN, RE, DB, UN,
/*"eoshift",	UN, NARGS_MORE_THAN_2, IN, RE, DB, CM,*/
"exp",		UN, 1, RE, DB, CM, UN,
"int",		IN, 1, IN, RE, DB, CM, 
"log",		UN, 1, RE, DB, CM, UN,
"log10",	UN, 1, RE, DB, CM, UN,
"max",		UN, NARGS_MORE_THAN_2, IN, RE, DB, UN,
"min",		UN, NARGS_MORE_THAN_2, IN, RE, DB, UN,
"mod",		UN, 2, IN, RE, DB, UN,
"nint",		IN, 1, RE, DB, UN, UN,
"real",		RE, 1, IN, RE, DB, CM, 
"sign",		UN, 2, IN, RE, DB, UN,
"sin",		UN, 1, RE, DB, CM, UN,
"sinh",		UN, 1, RE, DB, UN, UN,
"sqrt",		UN, 1, RE, DB, CM, UN,
"tan",		UN, 1, RE, DB, UN, UN,
"tanh",		UN, 1, RE, DB, UN, UN
};

static generic_to_intrinsic_descriptor generic_to_intrinsic_table[] = {
  "abs",	"iabs","abs","dabs","cabs","",
  "acos",	"acos","dacos","","","",
  "aint",	"aint","dint","","","",
  "anint",      "anint","dnint","","","",
  "asin",	"asin","dasin","","","",
  "atan",	"atan","datan","","","",
  "atan2",      "atan2","datan2","","","",
  "cmplx",      "","","","","",
  "cos",	"cos","dcos","ccos","","",
  "cosh",	"cosh","dcosh","","","",
/*  "cshift",	"cshift","cshift","cshift","cshift","cshift",*/
  "dble",       "","","","","",
  "dim",	"idim","dim","ddim","","",
/*  "eoshift",	"eoshift","eoshift","eoshift","eoshift","eoshift",*/
  "exp",	"exp","dexp","cexp","","",
  "int",	"int","ifix","idint","","",
  "log",	"alog","dlog","clog","","",
  "log10",      "alog10","dlog10","","","",
  "max",	"max0","amax1","dmax1","amax0","max1",
  "min",	"min0","amin1","dmin1","","",
  "mod",	"mod","amod","dmod","","",
  "nint",	"nint","idnint","","","",
  "real",	"real","float","sngl","","",
  "sign",	"isign","sign","dsign","","",
  "sin",	"sin","dsin","csin","","",
  "sinh",	"sinh","dsinh","","","",
  "sqrt",	"sqrt","dsqrt","csqrt","","",
  "tan",	"tan","dtan","","","",
  "tanh",	"tanh","dtanh","","",""
};

/* Many Fortran 90 intrinsics accept arguments that have different types */
static char *different_typed_args_table[] = {
"cshift",
"eoshift"
};

static const int NUM_INTRINSIC_TABLE_ENTRIES =
    (sizeof(intrinsic_table) / sizeof(intrinsic_descriptor));

static const int NUM_GENERIC_TABLE_ENTRIES =
    (sizeof(generic_table) / sizeof(generic_descriptor));

static const int NUM_GENERIC_TO_INTRINSIC_TABLE_ENTRIES =
    (sizeof(generic_to_intrinsic_table) / sizeof(generic_to_intrinsic_descriptor));

static const int NUM_DIFFERENT_TYPED_ARGS_TABLE_ENTRIES =
    (sizeof(different_typed_args_table) / sizeof(char *));


typedef FUNCTION_POINTER(int, CompareFunctPtr, (const void*, const void*));

STATIC(int, intrinsic_compare_names, (char* name, intrinsic_descriptor* entry));
STATIC(int, generic_compare_names, (char* name, generic_descriptor* entry));
STATIC(int, generic_to_intrinsic_compare_names, (char* name, 
                                   generic_to_intrinsic_descriptor* entry));
STATIC(int, different_typed_compare_names, (char* name, char** entry));

/****************************************************************************
 * static int intrinsic_compare_names:
 *   comparison function that guides bsearch of a table filled with entries
 *   of type intrinsic_descriptor for one matching "name"
 ***************************************************************************/
static int intrinsic_compare_names(char* name, intrinsic_descriptor* entry)
{
  return strcmp(name, entry->name);
}


/****************************************************************************
 * static int generic_compare_names:
 *   comparison function that guides bsearch of a table filled with entries
 *   of type generic_descriptor for one matching "name"
 ***************************************************************************/
static int generic_compare_names(char* name, generic_descriptor* entry)
{
  return strcmp(name, entry->name);
}


/****************************************************************************
 * static int generic_to_intrinsic_compare_names:
 *   comparison function that guides bsearch of a table filled with entries
 *   of type generic_to_intrinsic_descriptor for one matching "name"
 ***************************************************************************/
static int generic_to_intrinsic_compare_names(char* name, 
                                   generic_to_intrinsic_descriptor* entry)
{
  return strcmp(name, entry->name);
}


/****************************************************************************
 * static int different_typed_compare_names:
 *   comparison function that guides bsearch of a table filled with entries
 *   of type (char *) for one matching "name"
 ***************************************************************************/
static int different_typed_compare_names(char* name, char** entry)
{
  return strcmp(name, *entry);
}


/****************************************************************************
 * intrinsic_descriptor* builtins_intrinsicFunctionInfo:
 *   returns a structure that describes the attributes of a Fortran intrinsic 
 *   function 
 ***************************************************************************/
intrinsic_descriptor* builtins_intrinsicFunctionInfo(char* name)
{
  return (intrinsic_descriptor *)
    bsearch(name, (char *) intrinsic_table, NUM_INTRINSIC_TABLE_ENTRIES, 
	    sizeof(intrinsic_descriptor), (CompareFunctPtr)intrinsic_compare_names); 
}


/****************************************************************************
 * generic_descriptor *builtins_genericFunctionInfo:
 *   returns a structure that describes the attributes of a Fortran generic 
 *   function 
 ***************************************************************************/
generic_descriptor* builtins_genericFunctionInfo(char* name)
{
  return (generic_descriptor *) 
    bsearch(name, (char *) generic_table, NUM_GENERIC_TABLE_ENTRIES, 
	    sizeof(generic_descriptor), (CompareFunctPtr)generic_compare_names); 
}


/****************************************************************************
 * Boolean builtins_isIntrinsicFunction:
 *   returns true if "name" is the name of a Fortran generic function
 ***************************************************************************/
Boolean builtins_isGenericFunction(char* name)
{  
  return (BOOL(builtins_genericFunctionInfo(name) != (generic_descriptor *) 0));
}


/****************************************************************************
 * Boolean builtins_isIntrinsicFunction:
 *   returns true if "name" is the name of a Fortran intrinsic function
 ***************************************************************************/
Boolean builtins_isIntrinsicFunction(char* name)
{  
  return (BOOL(builtins_intrinsicFunctionInfo(name) != (intrinsic_descriptor *) 0));
}


/****************************************************************************
 * Boolean builtins_isBuiltinFunction:
 *   returns true if "name" is the name of a Fortran intrinsic or generic 
 *   function 
 ***************************************************************************/
Boolean builtins_isBuiltinFunction(char* name)
{
  return (BOOL(builtins_isGenericFunction(name) || builtins_isIntrinsicFunction(name)));
}


/****************************************************************************
 * Boolean builtins_acceptsDifferentTypedArgs:
 *   returns true if "name" is the name of a Fortran intrinsic function
 *   which accepts arguments which may have different types 
 ***************************************************************************/
Boolean builtins_acceptsDifferentTypedArgs(char* name)
{
  return  
    (BOOL(bsearch(name, (char *) different_typed_args_table, 
	    NUM_DIFFERENT_TYPED_ARGS_TABLE_ENTRIES, sizeof(char *), 
	    (CompareFunctPtr)different_typed_compare_names)
     != (char *) 0));
}


/****************************************************************************
 * int builtins_numGenerics:
 *   returns the number of Fortan generic functions.
 ***************************************************************************/
int builtins_numGenerics(char* name)
{
  return NUM_GENERIC_TABLE_ENTRIES;
}


/****************************************************************************
 * int builtins_numIntrinsics:
 *   returns the number of Fortan intrinsic functions.
 ***************************************************************************/
int builtins_numIntrinsics(char* name)
{
  return NUM_INTRINSIC_TABLE_ENTRIES;
}


/****************************************************************************
 * generic_descriptor *builtins_genericFunctionInfo_bynumber:
 *   returns a structure that describes the attributes of a Fortran generic 
 *   function. Used for iterating through all generic functions. Example:
 *     for (i = 0; i < builtins_numGenerics(); i++)
 *        ...  builtins_genericFunctionInfo_bynumber(i) ...
 ***************************************************************************/
generic_descriptor* builtins_genericFunctionInfo_bynumber(int number)
{
  if (number < 0 || number >= NUM_GENERIC_TABLE_ENTRIES)
    return (generic_descriptor *) 0;
  return (generic_descriptor *) 
    &(generic_table[number]);
}


/****************************************************************************
 * intrinsic_descriptor *builtins_intrinsicFunctionInfo_bynumber:
 *   returns a structure that describes the attributes of a Fortran intrinsic 
 *   function. Used for iterating through all intrinsic functions. Example:
 *     for (i = 0; i < builtins_numIntrinsics(); i++)
 *        ...  builtins_intrinsicFunctionInfo_bynumber(i) ...
 ***************************************************************************/
intrinsic_descriptor* builtins_intrinsicFunctionInfo_bynumber(int number)
{
  if (number < 0 || number >= NUM_INTRINSIC_TABLE_ENTRIES)
    return (intrinsic_descriptor *) 0;
  return (intrinsic_descriptor *) 
    &(intrinsic_table[number]);
}

/****************************************************************************
 * char *builtins_genericToIntrinsic: 
 *   returns 0 or the name of a Fortran intrinsic function. The returned
 *   string is the name of the intrinsic function that corresponds to the
 *   generic function name "name" invoked with arguments of type "argType"
 *   and result of tyoe "resType". Return value of 0 indicates that no 
 *   such intrinsic function could be found.
 ***************************************************************************/
char *
builtins_genericToIntrinsic(char *name, int argType, int resType)
{
  int i;
  char *intrinsic_name;
  intrinsic_descriptor *intrinsic_desc;
  generic_to_intrinsic_descriptor *g_to_i_d;

  g_to_i_d = (generic_to_intrinsic_descriptor *) 
    bsearch(name, (char *) generic_to_intrinsic_table,
	    NUM_GENERIC_TO_INTRINSIC_TABLE_ENTRIES, 
	    sizeof(generic_to_intrinsic_descriptor),
	    (CompareFunctPtr)generic_to_intrinsic_compare_names); 

  for (i = 0; i < MAX_INTRINSICS_PER_GENERIC; i++)
  {
    intrinsic_name = g_to_i_d->names[i];
    if (strlen(intrinsic_name) == 0)
    {
      /* failed to find intrinsic, return failure */
      return 0;
    }
    else
    {
      intrinsic_desc = builtins_intrinsicFunctionInfo(intrinsic_name);
      if ((intrinsic_desc->argType == argType) &&
	  (intrinsic_desc->resType == resType))
	return intrinsic_desc->name;
    }
  }

  /* failed to find intrinsic, return failure */
  return 0;
}

/****************************************************************************
 * int generic_result_type(generic_descriptor *ginfo, int argType)
 *   returns the result type for a generic function, given the
 *   generic_descriptor for the function
 ***************************************************************************/

int generic_result_type(generic_descriptor *ginfo, int argType)
{
  int resType;
  int i;

  assert(ginfo != 0);

  /* if resType is specified return that type */
  if (ginfo->resType != TYPE_UNKNOWN)
    return ginfo->resType;

  /* if ginfo->resType is TYPE_UNKNOWN, then usually, we should
     return argType as resType, however the type that results from
     invoking "abs" with a TYPE_COMPLEX argument is a TYPE_REAL */

  if ((strcmp(ginfo->name, "abs") == 0) && (argType == TYPE_COMPLEX))
    return TYPE_REAL;
  else
    return argType;
}
