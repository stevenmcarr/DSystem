/* $Id: FortEditorCP_opt.C,v 1.6 1997/03/11 14:30:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *	[-M <fortran module name>]
 *	[-P <program program name>]
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <libs/support/optParsing/Options.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortEditorCP/FortEditorCP_opt.i>
#include <libs/graphicInterface/include/FortEditorCP.h>

#include <libs/support/strings/rn_string.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/message.h>

static char *ned_cp_mod_name;
static char *ned_cp_pgm_name;

char *ned_cp_module_name()
{
  return ned_cp_mod_name;
}

char *ned_cp_program_name()
{
  return ned_cp_pgm_name;
}

static void 
ned_cp_opt_mod_context(Generic handle, char *filename)
{
  ned_cp_mod_name = ssave(filename);
}

static void 
ned_cp_opt_pgm_context(Generic handle, char *filename)
{
  ned_cp_pgm_name = ssave(filename);
}

static struct string_	pgm_context_s = {( OPT_STRING_CLBK_FN)
  ned_cp_opt_pgm_context,
  "program composition name",
  "program composition name",
  512, 50,
  ".*"
};

static struct string_	mod_context_s = {( OPT_STRING_CLBK_FN)
  ned_cp_opt_mod_context,
  "fortran file name",
  "fortran file name",
  512, 50,
  ".*"
};

static Option ned_mod_ctxt_opt = 
{ string, NED_CP_MOD_CTXT_OPT,  (Generic) "", true, (Generic)&mod_context_s };

static Option ned_pgm_ctxt_opt = 
{ string, NED_CP_PGM_CTXT_OPT,  (Generic) "", true, (Generic)&pgm_context_s };

void init_ned_options(Options &opts)
{
  opts.Add(&ned_mod_ctxt_opt);
  opts.Add(&ned_pgm_ctxt_opt);
}

int nedcp_OptsProcess(int argc, char **argv)
{
  Options ned_cp_options("command line options for ned");

  ned_cp_mod_name = 0;
  ned_cp_pgm_name = 0;

  init_ned_options(ned_cp_options);

  if (opt_parse_argv(&ned_cp_options, 0, argc, argv))
  {
    ned_cp_options.Usage(argv[0]);
    return -1;
  }

  /* should be part of processing of MANDATORY options */
  if (ned_cp_mod_name == 0)
  {/* no module name specified */
    ned_cp_options.Usage(argv[0]);
    return -1;
  }
 
  return 0;
}

