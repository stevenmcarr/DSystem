/* $Id: PEditorCP.h,v 1.3 1997/03/11 14:31:13 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef PEditorCP_h
#define PEditorCP_h

#include <libs/graphicInterface/oldMonitor/include/mon/cp_def.h>

typedef Generic PEditorCP;

extern aProcessor	ped_Processor;

EXTERN(int, pedcp_OptsProcess, (int argc, char **argv));
EXTERN(int, pedcp_Edit, (int argc, char **argv));

#endif
