/* $Id: DedCP_opt.C,v 1.2 1997/03/11 14:30:08 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/DedCP_opt.C						*/
/*									*/
/*	DedCP_opt -- Ded options processing				*/
/*	Last edited: September 2, 1993 at 11:48 am			*/
/*									*/
/************************************************************************/




/********************************/
/* [-M <fortran module name>]	*/
/* [-P <fortran program name>]	*/
/********************************/




#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <libs/support/optParsing/Options.h>
#include <libs/graphicInterface/cmdProcs/dEditor/DedCP_opt.i>
#include <libs/graphicInterface/cmdProcs/dEditor/DEditorCP.h>

#include <libs/support/strings/rn_string.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/message.h>





static char *ded_cp_mod_name;
static char *ded_cp_pgm_name;





char *ded_cp_module_name()
{
  return ded_cp_mod_name;
}




char *ded_cp_program_name()
{
  return ded_cp_pgm_name;
}




static void ded_cp_opt_mod_context(Generic handle, char *filename)
{
  ded_cp_mod_name = ssave(filename);
}




static void ded_cp_opt_pgm_context(Generic handle, char *filename)
{
  ded_cp_pgm_name = ssave(filename);
}




static struct string_ mod_context_s =
{
  (OPT_STRING_CLBK_FN)ded_cp_opt_mod_context,
  "fortran file name",
  "fortran file name",
  512, 50,
  ".*"
};




static struct string_ pgm_context_s =
{
  (OPT_STRING_CLBK_FN)ded_cp_opt_pgm_context,
  "program composition name",
  "program composition name",
  512, 50,
  ".*"
};




Option ded_mod_ctxt_opt = 
  {string, DED_CP_MOD_CTXT_OPT,  (Generic) "", true, (Generic)&mod_context_s};




Option ded_pgm_ctxt_opt = 
  {string, DED_CP_PGM_CTXT_OPT,  (Generic) "", true, (Generic)&pgm_context_s};




void init_ded_opts(Options &opts)
{
  opts.Add(&ded_mod_ctxt_opt);
  opts.Add(&ded_pgm_ctxt_opt);
}




int dedcp_OptsProcess(int argc, char **argv)
{
  Options ded_options("command line options for ded");
  ded_cp_mod_name = 0;
  ded_cp_pgm_name = 0;

  init_ded_opts(ded_options);

  if( opt_parse_argv(&ded_options, 0, argc, argv) )
    { ded_options.Usage(argv[0]);
      return -1;
    }

  /* should be part of processing of MANDATORY options */
    if( ded_cp_mod_name == 0 )    /* no module name specified */
      { ded_options.Usage(argv[0]);
        return -1;
      }
 
  return 0;
}

