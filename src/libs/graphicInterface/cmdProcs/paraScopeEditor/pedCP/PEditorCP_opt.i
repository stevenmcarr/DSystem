/* $Id: PEditorCP_opt.i,v 1.4 1997/03/11 14:32:04 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef PEditorCP_opt_i
#define PEditorCP_opt_i

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#define PED_CP_MOD_CTXT_OPT 'M'
#define PED_CP_PGM_CTXT_OPT 'P'
#define PED_CP_INPUT_FLAG   'h'

EXTERN(char *, ped_cp_module_name, (void));
EXTERN(char *, ped_cp_program_name, (void));
EXTERN(Boolean, ped_cp_input_flag, (void));

#endif
