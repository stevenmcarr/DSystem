/* $Id: FortEditorCP.h,v 1.8 1997/03/11 14:32:59 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/FortEditorCP.h						*/
/*									*/
/*	FortEditorCP -- Rn CP Definition for New Fortran Editor		*/
/*									*/
/************************************************************************/




#ifndef FortEditorCP_h
#define FortEditorCP_h

#ifndef Cp_def_h
#include <libs/graphicInterface/oldMonitor/include/mon/cp_def.h>
#endif

#ifndef newdatabase_h
#include <libs/support/database/newdatabase.h>
#endif

typedef Generic FortEditorCP;

extern aProcessor	ned_Processor;

EXTERN(int, nedcp_OptsProcess, (int argc, char **argv));
EXTERN(int, nedcp_Edit,        (int argc, char **argv));

#endif
