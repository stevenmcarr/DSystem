/* $Id: PEditorCP_opt.C,v 1.9 1997/03/11 14:32:03 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *	[-M <fortran module name>]
 *	[-P <program program name>]
 */

#include <libs/support/misc/general.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <libs/support/optParsing/Options.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/pedCP/PEditorCP_opt.i>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PEditorCP.h>

#include <libs/support/strings/rn_string.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/message.h>

static char *ped_cp_mod_name;
static char *ped_cp_pgm_name;
static Boolean ped_cp_ipt_flag = false;

char *ped_cp_module_name()
{
  return ped_cp_mod_name;
}

char *ped_cp_program_name()
{
  return ped_cp_pgm_name;
}

Boolean ped_cp_input_flag()
{
  return ped_cp_ipt_flag;
}

static void 
ped_cp_opt_mod_context(Generic handle, char *filename)
{
  ped_cp_mod_name = ssave(filename);
}

static void 
ped_cp_opt_pgm_context(Generic handle, char *filename)
{
  ped_cp_pgm_name = ssave(filename);
}

static struct string_	pgm_context_s = {( OPT_STRING_CLBK_FN)
  ped_cp_opt_pgm_context,
  "program composition name",
  "program composition name",
  512, 50,
  ".*"
};

static struct string_	mod_context_s = {( OPT_STRING_CLBK_FN)
  ped_cp_opt_mod_context,
  "fortran file name",
  "fortran file name",
  512, 50,
  ".*"
};

void ped_cp_opt_input(Generic state)

  {ped_cp_ipt_flag = true;}

static struct flag_	input_f = {( OPT_FLAG_CLBK_FN)
  ped_cp_opt_input,
  "Input Dependences",
  "compute input dependences",
};

static Option ped_cp_mod_option = 
{ string, PED_CP_MOD_CTXT_OPT,  (Generic) "", true, (Generic)&mod_context_s };

static Option ped_cp_pgm_ctxt_option = 
{ string, PED_CP_PGM_CTXT_OPT,  (Generic) "", true, (Generic)&pgm_context_s };

static Option ped_cp_input_flag_option = 
{ flag,   PED_CP_INPUT_FLAG, (Generic)false,true, (Generic)&input_f };


void ped_cp_options_init(Options &opt)
{
  opt.Add(&ped_cp_mod_option);
  opt.Add(&ped_cp_pgm_ctxt_option);
  opt.Add(&ped_cp_input_flag_option);
}

int pedcp_OptsProcess(int argc, char **argv)
{
  Options ped_cp_options("ped executable command line options");
  ped_cp_options_init(ped_cp_options);
  
  ped_cp_mod_name = 0;
  ped_cp_pgm_name = 0;

  if (opt_parse_argv(&ped_cp_options, 0, argc, argv))
  {
    ped_cp_options.Usage(argv[0]);
    return -1;
  }

  /* should be part of processing of MANDATORY options */
  if (ped_cp_mod_name == 0)
  {/* no module name specified */
    ped_cp_options.Usage(argv[0]);
    return -1;
  }
 
  return 0;
}

