/* $Id: general.h,v 1.13 1997/06/25 15:16:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*			       general.h				*/
/*			      						*/
/* 									*/
/* This include file provides macros to be used in files (usually	*/
/* include files) that are to be compatible with different varieties of	*/
/* the C/C++ programming language.  Currently, Kernighan & Ritchie C,	*/
/* ANSI C, and C++ are supported.  Support has also been added for our	*/
/* own special lint wrapper, rnlint, to take advantage of the		*/
/* prototyping information in include files, even though during normal	*/
/* compilation of Kernighan & Ritchie C would normally ignore such	*/
/* information.								*/
/*								 	*/
/* The macros in this file deal mostly with handling function		*/
/* prototype information properly in each environment.  There are	*/
/* three situations were these macros should be used.  See below.  In	*/
/* all of these cases, arguments and types must be given, although	*/
/* such information is thrown away under some circumstances.		*/
/*								 	*/
/*								 	*/
/* EXTERN--defines an externally scoped function declaration.  All	*/
/* functions to be shared between different varieties of the C		*/
/* language must be declared with the EXTERN macro.  The macro		*/
/* takes three arguments.  The first argument is the return type of	*/
/* the function.  The second argument is the function name.  The	*/
/* second argument is a parenthesized list of comma-separated		*/
/* arguments, each preceded by its type.  For example, if you were	*/
/* to declare the function "max" that takes two integer arguments	*/
/* and returns an integer argument, you'd declare it as follows:	*/
/*								 	*/
/* 	EXTERN(int, max, (int a, int b));				*/
/*								 	*/
/* If your function takes no arguments, EXTERN's third argument		*/
/* should be "(void)".  For example:					*/
/*								 	*/
/* 	EXTERN(int, func1, (void));					*/
/*								 	*/
/* Note that if the function is declared with a variable number of	*/
/* arguments, the types and names of the fixed arguments must be	*/
/* listed and then a "..." should be given as the last element in the	*/
/* argument list.  There must be at least one fixed argument given.	*/
/* For example:								*/
/* 									*/
/* 	EXTERN(char *, sprintf, (char *fmt, ...));			*/
/*								 	*/
/* 									*/
/* STATIC defines a statically scoped function declaration.		*/
/* Usually STATIC declarations occur in C files that might be		*/
/* compiled either with an ANSI C compiler or a Kernighan & Ritchie	*/
/* C compiler.  The STATIC declaration takes the place of a forward	*/
/* declaration of the a static function in that file.  When the		*/
/* function is actually defined, it is done with the K & R style	*/
/* mentioning the names of the arguments in parenthesis and a list	*/
/* of arguments and types following.  The STATIC macro is given the	*/
/* same arguments as EXTERN, above.  For example, say you have a	*/
/* static function for square root in a file.  You'd declare it		*/
/* early in the file as follows:					*/
/* 									*/
/* 	STATIC(double, square_root, (double x));			*/
/* 									*/
/* Later, you'd define it:						*/
/* 									*/
/* 	static								*/
/* 	double square_root(x)						*/
/* 	double x;							*/
/* 	{								*/
/*		...							*/
/* 	}								*/
/* 									*/
/* 									*/
/* FUNCTION_POINTER defines a function pointer type with argument	*/
/* types.  This construct is most often used to define the types of	*/
/* callbacks or structures of functions.  If you are declaring		*/
/* function types, be sure you choose an obscure namespace for your	*/
/* type (just like global functions).  Also, suffix the type with the	*/
/* word "callback" or "func".  This macro takes three arguments, the	*/
/* first two of which are simply the function return type and		*/
/* function pointer type name separated by a comma.  The third		*/
/* argument is analagous to the EXTERN macro, above.  For example:	*/
/* 									*/
/* 	typedef FUNCTION_POINTER(void, cfg_create_callback,		*/
/* 					(int a, int b));		*/
/* 									*/
/* The FUNCTION_POINTER() macro can be used directly in structures,	*/
/* but it is _considerably_ less useful as it prevents the client	*/
/* from casting function to your type for the sake of argument type	*/
/* casting:								*/
/* 									*/
/* 	---Undesirable usage---						*/
/* 	typedef struct cfg_client_struct {				*/
/* 		Generic     handle;					*/
/* 		FUNCTION_POINTER(void, create_instance,			*/
/* 				(Generic handle, CfgInstance cfg));	*/
/* 		FUNCTION_POINTER(void, destroy_instance,		*/
/* 				(Generic handle, CfgInstance cfg));	*/
/* 		FUNCTION_POINTER(void, dump_instance,			*/
/* 				(Generic handle, CfgInstance cfg));	*/
/* 		struct cfg_client_struct *next;				*/
/* 	} *CfgClient;							*/
/* 									*/
/************************************************************************/


#ifndef general_h
#define general_h

/* Commonly used types */
#if defined(__GNUC__)
typedef bool Boolean;   /* BOOLEAN DATATYPE             */
#else
typedef	enum { false, true } Boolean;	/* BOOLEAN DATATYPE		*/
#endif
#define	NOT(x)	((Boolean) !(x))	/* negate an int or bool	*/
#define	BOOL(x)	((Boolean)((x) ? 1 : 0))/* convert an int to a boolean	*/

#ifdef LONG_POINTER
typedef	long	Generic;		/* GENERIC DATATYPE		*/
#else
typedef	int	Generic;		/* GENERIC DATATYPE		*/
#endif
typedef	int	SmallGeneric;		/* GENERIC DATATYPE		*/
typedef enum { Unordered, PreOrder, PostOrder,
	       ReversePreOrder, ReversePostOrder, PreAndPostOrder } TraversalOrder;


#ifdef __cplusplus

/* C++ Definitions */

#define EXTERN(rettype, name, arglist) extern "C" rettype name arglist
#define STATIC(rettype, name, arglist) static rettype name arglist
#define FUNCTION_POINTER(rettype, name, arglist) rettype(*name)arglist

#elif __STDC__

/* ANSI C Definitions */

#define EXTERN(rettype, name, arglist) rettype name arglist
#define STATIC(rettype, name, arglist) static rettype name arglist
#define FUNCTION_POINTER(rettype, name, arglist) rettype(*name)arglist

#else

/* Kernighan & Ritchie  C Definitions */

#ifdef RN_LINT_LIBRARY
/* LINTLIBRARY */
#define EXTERN(rettype, name, arglist) rettype name ~ arglist ~
#else
#define EXTERN(rettype, name, arglist) rettype name()
#endif /* RN_LINT_LIBRARY */

#define STATIC(rettype, name, arglist) static rettype name()
#define FUNCTION_POINTER(rettype, name, arglist) rettype(*name)()

#endif

    /***** OBSOLETE--do not use in new stuff (K & R C only!) ******/

typedef FUNCTION_POINTER (void, PFV, ());
typedef FUNCTION_POINTER (int, PFI, ());
typedef FUNCTION_POINTER (char*, PFS, ());
typedef FUNCTION_POINTER (Boolean, PFB, ());
typedef FUNCTION_POINTER (Generic, PFG, ());

/* Commonly used function definitions and Macros */

#define	UNUSED		(-1)	 /* Often used magic number		*/

#define	MIN(a,b) (((a)<(b))?(a):(b))
#define	MAX(a,b) (((a)>(b))?(a):(b))

EXTERN(int, max, (int a, int b));
/* Takes two parameters (int a, b) and returns the largest.		*/

EXTERN(int, min, (int a, int b));
/* Takes two parameters (int a, b) and returns the smallest.		*/

EXTERN(void, die_with_message, (char *format, ...));
/* Takes a variable number of arguments.  The first argument must always*/
/* be the format parameter for the output string.  Following arguments	*/
/* are based upon the sprintf-style argument list for the format string.*/
/* The routine prints a message based upon the input and terminates the	*/
/* environment abnormally.						*/

#endif  /* general_h */
