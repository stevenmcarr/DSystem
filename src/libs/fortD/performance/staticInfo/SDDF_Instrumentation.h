/* $Id: SDDF_Instrumentation.h,v 1.1 1997/03/11 14:28:58 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/* -*- C++ -*-

// Contains the external procedure calls for the instrumentation and
// SDDF data collection. This should be the only externally visible
// interface to the rest of fortran D compiler.  

// $Header: /home/cs/carr/cvsroot/DSystem/src/libs/fortD/performance/staticInfo/SDDF_Instrumentation.h,v 1.1 1997/03/11 14:28:58 carr Exp $
//
*/

#ifndef _SDDF_Instrumentation_h
#define _SDDF_Instrumentation_h

#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/fortD/misc/FortD.h>

class Dist_Globals;

class FD_ProcEntry;
class SPMDInstrumentation;


/* Do this before each call to dc_compile_proc */
EXTERN(void, SD_Build_Tree_Annot,(Context modContext, char* procName, AST_INDEX root, FortTextTree ftt, FortTree ft));

EXTERN(void ,SD_InitLocalAndGatherInfo,(FortTextTree ftt, FortTree ft));

// Do this after each call to dc_compile_proc
EXTERN(void,SD_CleanupLocal,());
EXTERN(void, SD_CleanupInstr,	 (Dist_Globals* dh));
EXTERN(void,SD_InitialSetup,(SPMDInstrumentation *instr));
EXTERN(void,SD_FinalCleanupAndOutput,());

/* This one goes into local_decomp.C */
EXTERN(void,SD_Get_Decomp_Info,
       (FortTree ft,AST_INDEX cur, FortranDHashTableEntry * htEnt));
EXTERN(void,SD_Get_Distrib_Info,
       (AST_INDEX cur, FortranDHashTableEntry * htEnt));

EXTERN(void,SD_Get_Align_Info,
       (AST_INDEX cur, FortranDHashTableEntry *decomp, 
	FortranDHashTableEntry *array));

EXTERN(void,SD_ProcessArrays,());

#endif



