/* $Id: FortEditorSave.h,v 1.1 1997/03/11 14:30:51 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef FortranEditorSave_h
#define FortranEditorSave_h

#ifndef interact_h
#include <libs/support/msgHandlers/interact.h>
#endif

class FortranModule;

EXTERN(Boolean,  ed_RefSrcSave, (FortranModule * module,
				 MessageFunction message_func,
				 YesNoFunction yes_no_func));

#endif
