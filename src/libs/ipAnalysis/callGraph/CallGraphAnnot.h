/* $Id: CallGraphAnnot.h,v 1.5 1997/03/11 14:34:29 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
//
// CallGraphAnnot.h
//
// Definitions for CallGraph Annotations. This is for backward compatability
// only to support old code. Not to be used in new code.
//
// Author: John Mellor-Crummey                        December 1994
//  
//***************************************************************************

#ifndef CallGraphAnnot_h
#define CallGraphAnnot_h

#include <libs/support/annotation/Annotation.h>
#include <libs/support/sets/DataFlowSet.h>

#define CallGraphAnnot Annotation
#define FlowGraphAnnot Annotation
#define CallGraphDFAnnot DataFlowSet
#define FlowGraphDFAnnot DataFlowSet

#endif /* CallGraphAnnot_h */
