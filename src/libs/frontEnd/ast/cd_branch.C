/* $Id: cd_branch.C,v 1.1 1997/06/24 17:41:50 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <libs/frontEnd/ast/cd_branch.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/frontEnd/ast/gen.h>

static char *cdBrType[] = {"UNCONDITIONAL", "LOGICAL_IF", "DO_LOOP",
			       "ARITHMETIC_IF", "COMPUTED_GOTO",
			       "ASSIGNED_GOTO", "OPEN_ASSIGNED_GOTO",
			       "I_O", "CASE", "ALTERNATE_RETURN",
			       "ALTERNATE_ENTRY"};

/*
 *  CdBranchType cd_branch_type(n)
 *
 *  Classifies an ast node as to the type of control flow it can
 *  originate.
 *
 *  Use cfg_branch_type() instead, if cfg has been built.
 *
 *  Answers are possibly confusing if this is called on a node that
 *  has only one outgoing control flow edge.
 *  Best used to classify reason why a node has multiple outedges.
 *  There can only be one reason per node type --
 *  thank Providence for small miracles.
 *
 */
CdBranchType cd_branch_type(AST_INDEX n)
{
    switch(gen_get_node_type(n))
    {
    case GEN_IF:
    case GEN_LOGICAL_IF:
    case GEN_GUARD:
	return CD_LOGICAL_IF;

    case GEN_DO:
    case GEN_DO_ALL:
    case GEN_PARALLELLOOP:
	return CD_DO_LOOP;

    case GEN_ARITHMETIC_IF:
	return CD_ARITHMETIC_IF;

    case GEN_COMPUTED_GOTO:
	return CD_COMPUTED_GOTO;

    case GEN_ASSIGNED_GOTO:
	if (!is_null_node(gen_ASSIGNED_GOTO_get_lbl_ref_LIST(n)))
	    return CD_ASSIGNED_GOTO;
	else
	    return CD_OPEN_ASSIGNED_GOTO;

    case GEN_READ_LONG:
    case GEN_WRITE:
    case GEN_OPEN:
    case GEN_CLOSE:
    case GEN_INQUIRE:
    case GEN_BACKSPACE_LONG:
    case GEN_ENDFILE_LONG:
    case GEN_REWIND_LONG:
	return CD_I_O;

    /*
     *  Fortran 90
     *  case GEN_CASE:
     *      return CD_CASE;
     */

    case GEN_CALL:
	return CD_ALTERNATE_RETURN;

    case GEN_NULL_NODE:
    case GEN_PROGRAM:
    case GEN_FUNCTION:
    case GEN_SUBROUTINE:
	return CD_ALTERNATE_ENTRY;

    /*
     *  case GEN_PARALLEL: ???
     */

    default:
	return CD_UNCONDITIONAL;
  }
}

char *cd_branch_text(CdBranchType type)
{
    return cdBrType[(int) type];
}
