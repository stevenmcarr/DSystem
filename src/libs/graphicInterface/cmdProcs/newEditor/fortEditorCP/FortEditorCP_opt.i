/* $Id: FortEditorCP_opt.i,v 1.4 1997/03/11 14:30:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef FortEditorCP_opt_i
#define FortEditorCP_opt_i

#define NED_CP_MOD_CTXT_OPT 'M'
#define NED_CP_PGM_CTXT_OPT 'P'

EXTERN(char *, ned_cp_module_name, (void));
EXTERN(char *, ned_cp_program_name, (void));

#endif
