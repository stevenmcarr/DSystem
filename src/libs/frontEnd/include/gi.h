/* $Id: gi.h,v 1.8 1997/03/11 14:29:59 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef gi_h
#define gi_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef forttypes_h
#include <libs/frontEnd/ast/forttypes.h>
#endif

typedef enum 
	{
	  GI_PROGRAM, GI_SUBROUTINE, GI_FUNCTION, GI_BLOCK_DATA,
	  GI_UNKNOWN_ROUTINE
	} EntryUsage ;

typedef enum 
	{
	  LABEL,VARIABLE,PROCEDURE,EXPRESSION
	} ArgUsage ;

typedef struct arg_info   {
	ArgUsage   usage;
        Generic    type;
	char       name[8];
        } GI_ARG;

typedef struct entry_info {
	EntryUsage usage;
	Generic    type;
	char       *name;
	Generic    scratch;
	Generic	   num_args;
	GI_ARG	   *args;
       	} GI_ENT;

EXTERN(GI_ENT *, gi_new_entry, (void));
EXTERN(void, gi_free_entry, (GI_ENT *e));
EXTERN(void, gi_write_entry, (Generic port, GI_ENT *e));
EXTERN(GI_ENT *, gi_read_entry, (Generic port));
EXTERN(GI_ENT *, gi_copy_entry, (GI_ENT *oldent));
EXTERN(char *, gi_entry_name, (GI_ENT *e));
EXTERN(char *, gi_entryUsage_text, (EntryUsage entryUsage));
EXTERN(char *, gi_argUsage_text, (ArgUsage argUsage));
EXTERN(void, gi_print_entry, (GI_ENT *e));

#endif
