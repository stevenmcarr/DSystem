/* $Id: DEditorCP.h,v 1.2 1997/03/11 14:30:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef DEditorCP_h
#define DEditorCP_h

#include <libs/graphicInterface/oldMonitor/include/mon/cp_def.h>

extern aProcessor	DedCP_Processor;

EXTERN(int, dedcp_OptsProcess, (int argc, char **argv));
EXTERN(int, dedcp_Edit, (int argc, char **argv));

#endif
