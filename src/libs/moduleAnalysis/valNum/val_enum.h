/* $Id: val_enum.h,v 1.2 1997/03/11 14:36:19 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef val_enum_h
#define val_enum_h

/*
 *  val_enum.h
 *
 *  Declare some constants and types
 */

typedef Generic ValNumber;
typedef struct ValTable *Values;
typedef int ExpType;

/*
 *  VAL_NIL      no value number computed
 *
 *  VAL_BOTTOM   unknown (due to merge):   (a op BOTTOM) -> BOTTOM
 *  VAL_TOP      undefined (unexecutable): (a meet TOP)  -> a
 *
 *  Other common value numbers: 0, 1, -1, true, false
 */
enum {
    VAL_BOGUS  = -2,
    VAL_NIL    = -1, 
    VAL_BOTTOM = 0, 
    VAL_TOP    = 1,
    VAL_ZERO   = 2,
    VAL_ONE    = 3,
    VAL_M_ONE  = 4,
    VAL_TRUE   = 5,
    VAL_FALSE  = 6
};

#endif /* !val_enum_h */
