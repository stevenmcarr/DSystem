/* $Id: LocalOverlap.C,v 1.8 1997/03/11 14:28:41 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//--------------------------------------------------------------------------
// author : Seema Hiranandani
// content: the routines in this file are used to compute a coarse
//          approximation of overlap areas for arrays. 
//          It is extremely naive in that it looks at subscript expressions
//          of the form  (n -/+ c) where c is a constant expression and
//          n is an index variable. 
//          It stores the value of c, the array and the dimension
//          the subscript expression occurs in
//          StoreOverlap is called on each procedure
// date   : August 1992
//--------------------------------------------------------------------------
#include <stdlib.h>

#include <libs/frontEnd/ast/ast_include_all.h>
#include <libs/frontEnd/ast/forttypes.h>
#include <libs/frontEnd/ast/treeutil.h>

#include <libs/fortD/misc/FortD.h>
#include <libs/fortD/localInfo/LocalOverlap.h>

//------------------------
// forward declarations

STATIC(int, WalkOverlap, (AST_INDEX stmt, int level, Generic x));
STATIC(int, ComputeOverlap, (AST_INDEX node, Generic overlap_info));
STATIC(int, ComputeOverlapP, (AST_INDEX node, Generic x));
STATIC(Boolean, is_plus_minus_subscript, (AST_INDEX node, int *const_term));

//--------------------------------------------------------------------------
// this function computes overlaps taking into account the
// the local reaching decomposition. This is done for the main
// program procedure. The current version is restricted in that
// it doesnot allow for decompositions to be declared within
// procedures.
//--------------------------------------------------------------------------
void FortranDInfo::ComputeOverlapProgram(AST_INDEX node)
{
 walk_expression(gen_ASSIGNMENT_get_rvalue(node), ComputeOverlapP, NULL, (Generic)this);
}

//--------------------------------------------------------------------------
// for each array entry, get the current reaching decomposition
// use that information to determine the correct overlap
//--------------------------------------------------------------------------
static int ComputeOverlapP(AST_INDEX node, Generic x)
{
 char  *node_name;
 int val = 0;
 int dim_num  = 0;
 OverlapList *overlap_infor;
 overlap_ent *entry;
 Dist_type  dist_type;

 if(is_subscript(node))
 {
  node_name = gen_get_text(gen_SUBSCRIPT_get_name(node));
 }
 else
  return(WALK_CONTINUE);
 
  FortranDInfo *fortd_info = (FortranDInfo*)x;
  overlap_infor = (OverlapList*)fortd_info->overlap_info;
  entry = overlap_infor->get_entry(node_name);
  
  if(entry == 0)
   entry = overlap_infor->append_entry(node_name);

 node = list_first(gen_SUBSCRIPT_get_rvalue_LIST(node));

 while(node != AST_NIL)
  {
   if (is_plus_minus_subscript(node, &val))
   {
     dist_type = fortd_info->GetDistributeType(node_name, dim_num);
     switch(dist_type) {
 
     case FD_DIST_BLOCK:
     if(val > 0)
      entry->union_t(val,0, dim_num); 
     else
      entry->union_t(0,val,dim_num);
     break;

     default:
     break;
    }
  }
   node = list_next(node);
   ++dim_num;
  }
 entry->dim(dim_num);

 return WALK_CONTINUE;

}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
void FortranDInfo::StoreOverlap(AST_INDEX node, OverlapList *overlap_info)
{
 walk_statements(node, LEVEL1, WalkOverlap, NULL, (Generic)overlap_info);
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
static int WalkOverlap(AST_INDEX stmt, int level, Generic x)
{

  switch(gen_get_node_type(stmt))
  {
   case GEN_ASSIGNMENT:
   walk_expression(gen_ASSIGNMENT_get_rvalue(stmt), ComputeOverlap, NULL, x);
   break;
  
   default:
   break;
  }
  return WALK_CONTINUE;
}


//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
static int ComputeOverlap(AST_INDEX node, Generic overlap_info)
{
 char  *node_name;
 int val = 0;
 int dim_num  = 0;
 OverlapList *overlap_infor;
 overlap_ent *entry;

 if(is_subscript(node))
 {
  node_name = gen_get_text(gen_SUBSCRIPT_get_name(node));
 }
 else
  return(WALK_CONTINUE);
 
  overlap_infor = (OverlapList*)overlap_info;
  entry = overlap_infor->get_entry(node_name);
  
  if(entry == 0)
   entry = overlap_infor->append_entry(node_name);
  
 node = list_first(gen_SUBSCRIPT_get_rvalue_LIST(node));
 while(node != AST_NIL)
  {
   if (is_plus_minus_subscript(node, &val))
   {
     if(val > 0)
       entry->union_t(val,0, dim_num); 
      else
       entry->union_t(0,val,dim_num);
   }
   node = list_next(node);
   ++dim_num;
  }
 entry->dim(dim_num);

 return WALK_CONTINUE;
}


//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
static Boolean
is_plus_minus_subscript(AST_INDEX node, int *const_term )
{
 AST_INDEX term1, term2, cterm;

 if (is_binary_plus(node))
    {
      term1 = gen_BINARY_PLUS_get_rvalue1(node);
      term2 = gen_BINARY_PLUS_get_rvalue2(node);
    }
  else if(is_binary_minus(node))
    {
      term1 = gen_BINARY_MINUS_get_rvalue1(node);
      term2 = gen_BINARY_MINUS_get_rvalue2(node);
    }
  else return(false);

    if (is_identifier(term1))
     cterm = term2;        
    else
      return(false);

     if (is_constant(cterm))
      {
      *const_term  = atoi(gen_get_text(cterm));
      if(is_binary_minus(node))
      *const_term = -(*const_term);
      return(true);
      }
     
// if it reaches this point, return false
  return(false);
}

//-------------------------------------------------------------------------
// read an overlap entry from the database
//-------------------------------------------------------------------------
int overlap_ent::ReadUpCall(FormattedFile &port)
{
  char nme[NAME_LENGTH];

  port.Read(nme, NAME_LENGTH);
  param_name = ssave(nme);
  port.Read(numdim);
  for(int i = 0; i < numdim; ++i)
  {
  port.Read(lower[i]);
  port.Read(upper[i]);
  }
  return 0; // success ?!
}
  
//-------------------------------------------------------------------------
// write an overlap entry to the database
//-------------------------------------------------------------------------
int  overlap_ent::WriteUpCall(FormattedFile &port)
{
  port.Write(param_name, NAME_LENGTH);
  port.Write(numdim);
  for(int i = 0; i < numdim; ++i)
    {
      port.Write(lower[i]);
      port.Write(upper[i]);
    }
  return 0; // success ?!
}
