/* $Id: IPQuery.C,v 1.3 1997/03/11 14:34:37 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/fileAttrMgmt/composition/Composition.h>
#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/interface/IPQuery.h>


C_CallGraph IPQuery_Init(Context pgm_context) 
{ 
  CallGraph *cg = (CallGraph*)pgm_context->AttachAttribute(CLASS_NAME(CallGraph));

  return (C_CallGraph)cg;
}

void IPQuery_Fini(C_CallGraph cg) 
{
  CallGraph *cgraph = (CallGraph *) cg;
  ((Composition *) cgraph->program)->DetachAttribute(cgraph);
#if 0
  delete (CallGraph *) cg;
#endif
}
