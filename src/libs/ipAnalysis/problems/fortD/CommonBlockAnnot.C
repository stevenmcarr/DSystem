/* $Id: CommonBlockAnnot.C,v 1.3 1997/03/11 14:34:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/callGraph/CallGraphAnnotReg.h>

#include <libs/ipAnalysis/problems/fortD/CommonBlockAnnot.h>

#if 0
char* FORTD_COMMON_BLOCK_ANNOT = "fortran D common block";



class FortDCommonBlkAnnotMgr : public CallGraphAnnotMgr {
//------------------------------------------------------
// create an instance of the fortran D reach annotation
//------------------------------------------------------
CallGraphAnnot *New(CallGraphNode *node); 

//------------------------------------------------------------
// compute an instance of the alias annotation for the edge or
// node 
//------------------------------------------------------------
CallGraphAnnot *FortDCommonBlkAnnotMgr::DemandAnnotation(CallGraphNode *node);
CallGraphAnnot *FortDCommonBlkAnnotMgr::DemandAnnotation(CallGraphEdge *edge);
};

REGISTER_CG_ANNOT_MGR(FORTD_COMMON_BLOCK_ANNOT, FortDCommonBlkAnnotMgr);

CallGraphAnnot *FortDCommonBlkAnnotMgr::New(CallGraphNode *node)
{  
 return new FD_CommBlkNodeAnnotation();
};

CallGraphAnnot* FortDCommonBlkAnnotMgr::DemandAnnotation(CallGraphNode *node)
{
 return(node->GetAnnotation(FORTD_COMMON_BLOCK_ANNOT));
}

CallGraphAnnot* FortDCommonBlkAnnotMgr::DemandAnnotation(CallGraphEdge *edge)
{
 return(edge->GetAnnotation(FORTD_COMMON_BLOCK_ANNOT));
}
#endif

//-----------------------------------------------------------------
// check if nme is a common block name
//-----------------------------------------------------------------
Boolean FD_CommBlkNodeAnnotation::fort_d_is_common(char *nme)
{
for(common_block_entry_list *clist = common_blks_decl->first_entry();
    clist != 0;              clist = common_blks_decl->next_entry())
    {
    if (strcmp(clist->name, nme) == 0)
     return(true);
   }
 return(false);
}

//-----------------------------------------------------------------------
// given the leader, offset and size, return the common block entry name
//-----------------------------------------------------------------------
char* FD_CommBlkNodeAnnotation::fort_d_get_common_entry_nme(char *leader, int offset, int size)
{
for(common_block_entry_list *clist =
      common_blks_decl->first_entry();
      clist != 0;
      clist = common_blks_decl->next_entry())
    {
     for(common_block_ent *e = clist->common_list->first_entry(); e != 0;
         e = clist->common_list->next_entry())

// for each entry check if the size and starting position are the
// same. If so then return the comon_block_entry

     if (strcmp(e->leader, leader) == 0)
			{
      if ((e->offset == offset) && (e->size == size))
       return (e->name);
      }
	 }
  return(0);
}
