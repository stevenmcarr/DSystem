/* $Id: forttypes.h,v 1.7 1997/03/11 14:29:26 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef forttypes_h
#define forttypes_h


/****************************************************************************
 * Fortran Types                                                            *
 *                                                                          *
 * (1) type codes for fortran types and defined constants for their sizes   *
 * (2) tables that map tuples of types of operands and an operator into the *
 *     type of the result                                                   *
 * (3) a table used to perform procedure interface argument and result type *
 *     matching for compositions.                                           *
 *                                                                          *
 ****************************************************************************/


/* integer type codes for fortran types */

# define TYPE_UNKNOWN             0
# define TYPE_INTEGER             1
# define TYPE_REAL                2
# define TYPE_DOUBLE_PRECISION    3
# define TYPE_COMPLEX             4
# define TYPE_LOGICAL             5
# define TYPE_LABEL               6
# define TYPE_CHARACTER           7
# define TYPE_ERROR               8
# define TYPE_EXACT               9
# define TYPE_NONE                10
# define TYPE_EVENT               11
# define TYPE_SEMAPHORE           12
# define TYPE_BARRIER             13
# define TYPE_LAST                14


/* byte sizes for fortran types */
# define SIZE_PER_INTEGER         4
# define SIZE_PER_REAL            4
# define SIZE_PER_DB_PREC         (2 * SIZE_PER_REAL)
# define SIZE_PER_COMPLEX         (2 * SIZE_PER_REAL)
# define SIZE_PER_LOGICAL         4
# define SIZE_PER_LABEL           4
# define SIZE_PER_CHAR            1
# define SIZE_PER_EXACT           SIZE_PER_DB_PREC
# define SIZE_PER_EVENT           4
# define SIZE_PER_SEMAPHORE       4
# define SIZE_PER_BARRIER         4


extern int type_to_size_in_bytes[TYPE_LAST];


typedef int MAP_TYPE; 

/* ---- type tables that map operand types and operators to result types --- */

/* unary minus */
extern MAP_TYPE minus_type_map[TYPE_LAST];

/* unary minus */
extern MAP_TYPE not_type_map[TYPE_LAST];

/* times, divide, plus, minus */
extern MAP_TYPE mult_type_map[TYPE_LAST][TYPE_LAST];

/* ge gt le lt */
extern MAP_TYPE ge_type_map[TYPE_LAST][TYPE_LAST];

/* ge gt le lt */
extern MAP_TYPE ge_arg_type_map[TYPE_LAST][TYPE_LAST];

/* eq ne */
extern MAP_TYPE eq_type_map[TYPE_LAST][TYPE_LAST];

/* eq ne */
extern MAP_TYPE eq_arg_type_map[TYPE_LAST][TYPE_LAST];

/* and or eqv neqv */
extern MAP_TYPE and_type_map[TYPE_LAST][TYPE_LAST];

/* and or eqv neqv */
extern MAP_TYPE concat_type_map[TYPE_LAST][TYPE_LAST];

/* ASSIGNMENT */
extern MAP_TYPE assign_type_map[TYPE_LAST][TYPE_LAST];
extern MAP_TYPE rhs_assign_type_map[TYPE_LAST][TYPE_LAST];

/* EXPONENTIAL */
extern MAP_TYPE exp_type_map[TYPE_LAST][TYPE_LAST];
extern MAP_TYPE lhs_exp_type_map[TYPE_LAST][TYPE_LAST];
extern MAP_TYPE rhs_exp_type_map[TYPE_LAST][TYPE_LAST];

extern MAP_TYPE is_arith_type[TYPE_LAST];
extern MAP_TYPE is_char_type[TYPE_LAST];
extern MAP_TYPE is_log_type[TYPE_LAST];
extern MAP_TYPE is_integer_type[TYPE_LAST];
extern MAP_TYPE is_label_type[TYPE_LAST];

/*------ table that indicates compatiblity for a pair of types ------*/

typedef enum {
    TYPE_MATCH_OK, 
    TYPE_MATCH_WARNING, 
    TYPE_MATCH_ERROR
} CompTypeMatch;

CompTypeMatch composition_type_match_table[TYPE_LAST][TYPE_LAST];

#endif
