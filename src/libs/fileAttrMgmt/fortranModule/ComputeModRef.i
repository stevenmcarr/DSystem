/* $Id: ComputeModRef.i,v 1.1 1997/03/11 14:27:54 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// ComputeModRef.i
//
// Author: John Mellor-Crummey                                January 1994
//
// Copyright 1994, Rice University
//***************************************************************************

#ifndef ComputeModRef_i
#define ComputeModRef_i

#include <libs/frontEnd/fortTree/FortTree.h>

ProcLocalInfo *ComputeProcScalarModRefInfo(FortTree ft, AST_INDEX proc);

#endif
