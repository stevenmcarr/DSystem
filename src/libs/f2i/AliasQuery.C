/* $Id: AliasQuery.C,v 1.1 1997/04/10 14:21:41 carr Exp $ */
#include <libs/support/misc/general.h>
#include <libs/f2i/AliasQuery.h>

#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/problems/alias/AliasAnnot.h>

Boolean IPQuery_IsAliased(Generic callGraph,char *proc_name, 
			  char *vname, int length, int offset,
			  Boolean IsGlobal)

  {
   CallGraph *cg;
   CallGraphNode *node;
   AliasAnnot *aa;

     cg = (CallGraph *) callGraph;
     if (cg == NULL) 
       return true;

     node = cg->LookupNode (proc_name);
     if (node == NULL) 
       return true;

     aa = (AliasAnnot *) node->GetAnnotation(ALIAS_ANNOT, true);
     
     if (IsGlobal)
       return (aa->AliasedGlobal(vname,offset,length));
     else
       return(aa->AliasedFormal(vname));
       
  }

