/* $Id: iptypes.h,v 1.7 1997/03/11 14:34:50 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef iptypes_h
#define iptypes_h

typedef enum {SEQBLOCK=0, LOOP=1, PARLOOP=2,BLOCKNONE=3} CodeBlockType;
typedef enum {MODREFTYPE_REF=0, MODREFTYPE_MOD=1, MODREFTYPE_NEITHER=3} 
	ModRefType;

/* flags for variable type used in in mod/use info  */

#define VTYPE_NO_ATTRIBUTES 	    0x0
#define VTYPE_FORMAL_PARAMETER 	    0x1
#define VTYPE_COMMON_DATA	    0x2
#define VTYPE_LOCAL_DATA	    0x4
#define VTYPE_COMPILER_TEMPORARY    0x8
#define VTYPE_PROCEDURE 	    0x10
#define VTYPE_INTRINSIC 	    0x20
#define VTYPE_CONSTANT              0x40
#define VTYPE_CONSTANT_EXPR         0x80
#define VTYPE_STAR                  0x100 /* formal alternate return spec.  */

/* the constant below is used to support rudimentary analysis to determine 
 * if an actual parameter is being used as a scalar or an array by any 
 * procedure reached from a callsite -- JMC 7/92
 */
#define VTYPE_USED_AS_ARRAY         0x200 


#define IP_NAME_STRING_LENGTH       80
#define INFINITE_INTERVAL_LENGTH    -1

typedef enum { LocalScope, GlobalScope } VarScope;

#endif /* iptypes_h */
