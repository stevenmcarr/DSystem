/* $Id: FortDMisc.C,v 1.4 1997/03/11 14:28:41 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/support/misc/general.h>
#include <libs/fortD/misc/FortD.h>
#include <libs/frontEnd/ast/ast_include_all.h>
#include <libs/frontEnd/ast/treeutil.h>
//------------------------------------------------------------------
// void fd_store_param()
//
// identify parameter statement Parameter(n$proc = <const_expr>)
// store the value of n$proc in the hash table
//-------------------------------------------------------------------
void fd_store_param(AST_INDEX param, FortranDInfo *f)
{
  AST_INDEX name_id, value_id;
  char *name;
  int value;

  param = gen_PARAMETER_get_param_elt_LIST(param);
  param = list_first(param);
  name_id = gen_PARAM_ELT_get_name(param);
  name = gen_get_text(name_id);

  /* look for assignment to n$proc (or nproc) */

  if (strcmp("n$proc", name) && strcmp("nproc", name) &&
      strcmp("N$PROC", name) && strcmp("NPROC", name))
    return;

  value_id = gen_PARAM_ELT_get_rvalue(param);
  value = 0;

  if (ast_eval(value_id, &value))
    die_with_message("illegal n$proc specification");

  /* store total # of processors in Dist_Globals */
   f->def_numprocs = true;
   f->numprocs = value;
}

