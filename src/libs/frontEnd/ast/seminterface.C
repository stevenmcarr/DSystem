/* $Id: seminterface.C,v 1.1 1997/06/24 17:41:50 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <string.h>
#include <include/bstring.h>

#include <libs/support/database/newdatabase.h>

#include <libs/frontEnd/ast/astlist.h>
#include <libs/frontEnd/ast/asttree.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/frontEnd/include/gi.h>
#include <libs/frontEnd/ast/strutil.h>

#include <libs/support/memMgmt/mem.h>
#include <libs/support/strings/rn_string.h>

/*
 * an invariant:
 *    if GI_ENT args is non-zero then num_args indicates how many
 *	 args there really are
 *    if GI_ENT args is zero then num_args == -1 means that we
 *       don't know how many arguments there really are
 */
void
gi_print_entry(GI_ENT *e)
{
    int i;

    (void) printf("%s %s %d args\n", gi_entryUsage_text(e->usage), e->name,e->num_args);
    if (e->num_args > 0)
    {
        for( i=0;i<e->num_args;i++ )
	{
	   (void) printf("  %s %s type %d\n",
		  gi_argUsage_text(e->args[i].usage),
		  e->args[i].name,
		  e->args[i].type);
	}
    }

    (void) printf("\n");
}

GI_ENT *
gi_new_entry ()
{
    GI_ENT *e;

    e = (GI_ENT *) get_mem (sizeof (struct entry_info), "gi_ent");

    e->usage    = GI_UNKNOWN_ROUTINE;
    e->name     = ssave("");
    e->type     = TYPE_UNKNOWN;
    e->scratch  = 0;
    e->num_args = -1;
    e->args     = (GI_ARG *) 0;
    return e;
}

void 
gi_free_entry (GI_ENT *e)
{
    if (e <= (GI_ENT*)0)
	return;
    if (e->args)
	free_mem((void*)e->args);
    sfree (e->name);
    free_mem((void*)e);
}

GI_ENT *
gi_copy_entry (GI_ENT *oldent)
{
    GI_ENT          *ent;

    ent = gi_new_entry ();
    ent->usage = oldent->usage;
    ent->type  = oldent->type;
    ent->name  = ssave (oldent->name);
    ent->scratch = oldent->scratch;
    ent->num_args = oldent->num_args;
    if (oldent->args)
    {
       ent->args = (GI_ARG *) get_mem(sizeof(GI_ARG) * oldent->num_args, "args");
       bcopy((char *) oldent->args, (char *) ent->args, sizeof(GI_ARG) * oldent->num_args);
    }
    else
       ent->args = 0;

    return ent;
}


void gi_write_entry (Generic port, GI_ENT *e)
{

    /* the entry */
    db_buffered_write((DB_FP*)port, (char*)e, sizeof(GI_ENT));
    db_buffered_write_name ((DB_FP*)port, gi_entry_name(e));

    /* the args */
    if (e->args)
	db_buffered_write((DB_FP*)port, (char*)e->args, sizeof(GI_ARG) * e->num_args);
}

GI_ENT* gi_read_entry (Generic port)
{
    GI_ENT         *e;

    e = gi_new_entry ();
    /* the entry */
    db_buffered_read((DB_FP*)port, (char*)e, sizeof(GI_ENT));
    e->name = db_buffered_read_name ((DB_FP*)port, "gi entry name");

    /* the args */
    if (e->args)
    {
	e->args = (GI_ARG*)get_mem(sizeof(GI_ARG) * e->num_args, "args");
	db_buffered_read((DB_FP*)port, (char*)e->args, sizeof(GI_ARG) * e->num_args);
    }
    return e;
}

char* gi_entry_name(GI_ENT *e)
{
    return e->name;
}

char *
gi_argUsage_text(ArgUsage argUsage)
{
   switch (argUsage)
   {
      case LABEL:
	return "label";
      case VARIABLE:
	return "variable";
      case PROCEDURE:
	return "procedure";
      case EXPRESSION:
	return "expression";
      default:
	return "<unknown>";
   }
}

char *
gi_entryUsage_text(EntryUsage entryUsage)
{
   switch (entryUsage)
   {
	case GI_PROGRAM:
		return "program";
        case GI_SUBROUTINE:
		return "subroutine";
	case GI_FUNCTION:
		return "function";
	case GI_BLOCK_DATA:
		return "block data";
	case GI_UNKNOWN_ROUTINE:
		return "unknown routine";
	default:
		return "<unknown>";
   }
}
