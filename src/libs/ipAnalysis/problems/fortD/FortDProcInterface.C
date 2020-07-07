/* $Id: FortDProcInterface.C,v 1.3 1997/03/11 14:34:59 carr Exp $  */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//--------------------------------------------------------------------
// author   :  Seema Hiranandani
//
// content  :  provides an interface to the interprocedural information to
//             be used by the D editor
//
// date     :  June 1993
//--------------------------------------------------------------------

#include <iostream>

#include <libs/fortD/codeGen/FortDInterface.h>
#include <libs/ipAnalysis/interface/IPQuery.h>
#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/callGraph/CallGraphAnnot.h>
#include <libs/ipAnalysis/problems/fortD/ReachAnnot.h>
#include <libs/ipAnalysis/problems/fortD/OverlapAnnot.h>
#include <libs/support/strings/OrderedSetOfStrings.h>
#include <libs/support/strings/StringBuffer.h>
#include <libs/fortD/localInfo/fd_symtable.h>
#include <libs/fortD/misc/FortDStr.h>
#include <libs/support/database/context.h>
#include <libs/frontEnd/ast/ast_include_all.h>

// links the interprocedural annotation files
extern char* FORTD_REACH_ANNOT;
extern char* FORTD_OVERLAP_ANNOT;

//--------------------------------------------------------------------
// computes the fortran d reaching and overlap annotations
//--------------------------------------------------------------------
void* FortDInterface::BuildCG(Context pgm_context)
{
  return (void*)IPQuery_Init(pgm_context);
}

//-------------------------------------------------------------------
// return the procedure name
//-------------------------------------------------------------------
char* FortDProc::BuildProcName(AST_INDEX node)
{ 
 if ((gen_get_node_type(node) !=  GEN_FUNCTION) && 
    (gen_get_node_type(node) != GEN_SUBROUTINE) && 
    (gen_get_node_type(node) != GEN_BLOCK_DATA) && 
    (gen_get_node_type(node) != GEN_PROGRAM))
  {
  cerr << "Illegal node type: procedure node required \n";
  return 0;
  }
  return gen_get_text(get_name_in_entry(node));
}

//-------------------------------------------------------------------
// get the reaching annotation for a node
//-------------------------------------------------------------------
void* FortDProc::GetReachAnnot()
{

 CallGraphNode *cgn;
 void* reach_annot;
//---------------------------------------------
// check to see if there is a valid call graph
 CallGraph *cg1 = (CallGraph*)GetCG();
 if (!cg1)
  {
  cerr << "Missing Call Graph \n";
  return 0;
  }

 else
//---------------------------------------------------
// demand the fortran d annotations for the procedure
  {
   cgn = cg1->LookupNode(GetName());
   if (cgn == NULL)
    { 
    cerr << "Unable to locate node " << GetName() << " in the call graph \n";
     return 0;
    }
    if ((reach_annot = (void*)
                         cgn->GetAnnotation(FORTD_REACH_ANNOT,true)) == NULL)
      cerr << "Unable to build the reaching annotation for node " << GetName() << "\n"; 
      return(reach_annot);
   }
}

//-------------------------------------------------------------------
// get the overlap annotation for a node
//-------------------------------------------------------------------
void* FortDProc::GetOverlapAnnot()
{
 CallGraphNode *cgn;
 void* overlap_annot;
 CallGraph *cg1;

//---------------------------------------------
// check to see if there is a valid call graph

 cg1 =  (CallGraph*)GetCG();
 if (!cg1)
  {
  cerr << "Missing Call Graph \n";
  return 0;
  }
 else
  {
   cgn = cg1->LookupNode(GetName());
   if (cgn == NULL)
    { 
    cerr << "Unable to locate node " << GetName() << " in the call graph \n";
    return 0;
    }
    if ((overlap_annot = (void*)
                         cgn->GetAnnotation(FORTD_OVERLAP_ANNOT,true))== NULL)
      cerr << "Unable to build the overlap annotation for node " << GetName() << "\n"; 
      return(overlap_annot);
  }

} 

//-------------------------------------------------------------------------
// create alignment string from the symbol table entry 
//------------------------------------------------------------------------- 
StringBuffer*  FortDRef::AlignSt()
{
  StringBuffer *s;
  SUBSCR_INFO *subs_info;
  int value, i, numdim, index = 0;
  char c;
  SNODE *sym_entry = (SNODE*)GetSymEntry();
  
  numdim =  sp_numdim(sym_entry);
  s = new StringBuffer(100);
  s->Append("%s ", AlignKeyWord);   // ALIGN
  s->Append("%s" , sym_entry->id);  // array 
  if (!sym_entry->perfect_align)
   {
    for(i= 0; i<numdim; ++i)
     {
      if(i==0)
       s->Append("%s", "(");
      s->Append("%c", 'i'+ i);
      if (i != numdim - 1)
       s->Append("%s", ",");
    }
   s->Append("%s", ")");
  }
   s->Append(" %s", "with");  // with

   s->Append(" %s", sp_decomp(sym_entry)->id); // decomp
   if(!sym_entry->perfect_align)
   {
    s->Append("%s", "(");

  numdim  = sp_numdim(sp_decomp(sym_entry));
  for(i=0; i<numdim; ++i)
  {
   switch(sp_align_stype(sym_entry,i))
   {
    case ALIGN_OFFSET:
     index = sp_align_index(sym_entry, i);
     value = sp_align_offset(sym_entry,i);
     c = 'i' + index - 1;

     if (value < 0)
      s->Append( "%c %s %d",  c, "-", abs(value));
     else
      s->Append("%c %s %d", c, "+", value);
    break;

    case ALIGN_PERFECT:
     index = sp_align_index(sym_entry, i) ;
     c = 'i' + index - 1;
     s->Append("%c", c);
    break;

    case ALIGN_CONST:
     value = sp_align_offset(sym_entry,i); 
     s->Append("%d", value);
    break;

   }
   if (i != numdim-1)
    s->Append("%s", ",");
  }
 s->Append("%s ", ")");
 s->Append("%c", '\n');
 }
 return s;

}

//-------------------------------------------------------------------------
// create distribute string from the symbol table entry
//-------------------------------------------------------------------------
StringBuffer* FortDRef::DistribSt()
{
  int i;
  StringBuffer *s;
  SUBSCR_INFO *subs_info;
  int index = 0;
  int numdim;
  SNODE *sym_entry = (SNODE*)GetSymEntry();

  numdim = sp_numdim(sym_entry);
  s = new StringBuffer(100);
  s->Append("%s %s", DistribKeyWord, sp_decomp(sym_entry)->id);
  s->Append("%s",LeftBracket);
  for (i=0; i < numdim; ++i)
   {
    if(sp_get_dist_info(sym_entry, i)->distr_type == FD_DIST_BLOCK)
       s->Append("%s", "BLOCK");
    else
     if(sp_get_dist_info(sym_entry, i)->distr_type == FD_DIST_CYCLIC)
       s->Append("%s", "CYCLIC");
    else
     if(sp_get_dist_info(sym_entry, i)->distr_type == FD_DIST_LOCAL)
       s->Append("%s", ":");
   if(i != numdim-1)
    s->Append("%s", ",");
   }
   s->Append("%s \n",RightBracket);
   return(s);
}

//-------------------------------------------------------------------------
// create the decomposition string
// Example:
// DECOMPOSITION D(size_1,......,size_n)
//-------------------------------------------------------------------------
StringBuffer* FortDRef::DecompSt()
{
  StringBuffer *s;
  SUBSCR_INFO *subs_info;
  int index = 0;
  SNODE *sym_entry = (SNODE*)GetSymEntry();

  s = new StringBuffer(100);
  
  s->Append("%s ", DecompKeyWord);
  s->Append("%s", sp_decomp(sym_entry)->id);

  for(int i = 0; i< sp_numdim(sp_decomp(sym_entry)); ++i)
    {
     if (i==0)
     { 
      s->Append("%s", LeftBracket);
     }
     typedescript_string(s, (sp_decomp(sym_entry))->idtype[i]);
     if (i != sp_numdim(sp_decomp(sym_entry)) - 1)
      s->Append("%c" , ',');
    }
    s->Append("%s \n", RightBracket);
    return(s);
}

//--------------------------------------------------------------------------
// create a list of strings containing decomposition information of every
// formal parameter and global in the procedure
//--------------------------------------------------------------------------
OrderedSetOfStrings* FortDProc::ReachingDecompsList()
{
  OrderedSetOfStrings *oss;
  oss = ((FD_Reach_Annot*)GetReachAnnot())->CreateOrderedSetOfStrings();
  return oss;
} 

//-------------------------------------------------------------------------
// returns the decomposition information for a particular array reference
//-------------------------------------------------------------------------
OrderedSetOfStrings* FortDRef::DecompositionInfo()
{
 StringBuffer *sb;
 SNODE *sp = (SNODE*)GetSymEntry();
 OrderedSetOfStrings *s = new OrderedSetOfStrings();

 s->Append("<===== Decomposition Information ====> \n");
 s->Append(DecompSt()->Finalize());
 s->Append(AlignSt()->Finalize());
 s->Append(DistribSt()->Finalize());
 return s; 
}

//--------------------------------------------------------------------------
// get the upper overlap area for a particular dimension
//--------------------------------------------------------------------------
StringBuffer* FortDRef::GetUpper(int dim)
{
  char str[10];
  StringBuffer *s;
  s = new StringBuffer(10);
  int upper = sp_max_access((SNODE*)GetSymEntry(), dim);
  sprintf(str, "%d", upper);
  s->Append("%s", str);
  return s;
}

//--------------------------------------------------------------------------
// get the lower overlap area for a particular dimension
//--------------------------------------------------------------------------
StringBuffer* FortDRef::GetLower(int dim)
{
 char str[10];
 StringBuffer *s;
 s = new StringBuffer(10);
 int lower = sp_min_access((SNODE*)GetSymEntry(), dim);
 sprintf(str, "%d", lower);
 s->Append("%s", str);
 return s;
}

//--------------------------------------------------------------------------
// return the overlap information for a reference
//--------------------------------------------------------------------------
OrderedSetOfStrings* FortDRef::OverlapStr()
{
 SNODE *sp = (SNODE*)GetSymEntry();
 OrderedSetOfStrings *s = new OrderedSetOfStrings;
 StringBuffer *sb = new StringBuffer(60);
 s->Append("<==== Overlap Information ====> \n");
 sb->Append(sp->id);

 for (int i=0; i<sp->numdim; ++i)
  {
   if(i==0) 
    sb->Append("%s", LeftBracket);
    sb->Append(GetLower(i));
    sb->Append("%c", ':');  
    sb->Append(GetUpper(i));
   if (i != sp->numdim-1)
   { sb->Append("%c", ','); }
  }    
  sb->Append("%s", RightBracket);

 s->Append(sb->Finalize());
 return s;
}

//--------------------------------------------------------------------------
// return the distribute information as a string
//--------------------------------------------------------------------------
OrderedSetOfStrings* FortDRef::DistribStr()
{
 OrderedSetOfStrings* s = new OrderedSetOfStrings;
 s->Append(DistribSt()->Finalize());
 return s;
}


//--------------------------------------------------------------------------
// return the Decomposition  as a string
//--------------------------------------------------------------------------
OrderedSetOfStrings* FortDRef::DecompStr()
{
 OrderedSetOfStrings* s = new OrderedSetOfStrings;
 s->Append(DecompSt()->Finalize());
 return s;

}

//--------------------------------------------------------------------------
// return the alignment information as a string
//--------------------------------------------------------------------------
OrderedSetOfStrings* FortDRef::AlignStr()
{
 OrderedSetOfStrings* s = new OrderedSetOfStrings;
 s->Append(AlignSt()->Finalize());
 return s;
}

