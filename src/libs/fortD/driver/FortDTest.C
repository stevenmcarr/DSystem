/* $Id: FortDTest.C,v 1.4 2001/10/12 19:31:58 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <stdio.h>
#include <libs/fortD/driver/driver.h>
#include <libs/fortD/misc/fd_code.h>
#include <libs/frontEnd/ast/ast.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/support/tables/symtable.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped.h>
#include <libs/frontEnd/include/walk.h>
#include <libs/support/strings/OrderedSetOfStrings.h>
#include <libs/fortD/codeGen/FortDInterface.h>
#include <libs/support/database/context.h>


struct s {
Generic Ped;
Generic fi;
};

static void sp_dump(SNODE *sp);
static int dc_check_assign(AST_INDEX stmt, int level, Generic ped);
static int dc_check_side_info(AST_INDEX node, Generic ped);

//------------------------------------------------------------------
// The next 5 routines are used to debug the side array information.
// It prints the  SNODE, symbol table 
// information stored for each array reference
//------------------------------------------------------------------
void dc_compile_proc_p(PedInfo ped, AST_INDEX pnode, FortDInterface *fi)
{
  struct s info;
  int d;
  info.Ped = (Generic)ped;
  info.fi = (Generic)fi;

  cout<<"\n---------Procedure---------\n";
  walk_statements(pnode, LEVEL1, dc_check_assign, NULL,((Generic)(&info)));
}

//------------------------------------------------------------------
// If the stmt is an assignment statement, look for all array 
// references
//------------------------------------------------------------------
static int dc_check_assign(AST_INDEX stmt, int level, Generic info )
{
  switch(gen_get_node_type(stmt))
  {
  case GEN_DO:
    {
      FortDLoop* fd_loop = ((FortDInterface*)((struct s*)info)->fi)->GetLoop(stmt);
      fd_loop->AllMesgsStr();
      delete (fd_loop);
    }
  break;

 case GEN_ASSIGNMENT:
   {
     FortDStmt* fd_stmt = ((FortDInterface*)((struct s*)info)->fi)->GetStmt(stmt);
     fd_stmt->Test();
     delete(fd_stmt);

     dc_check_side_info(gen_ASSIGNMENT_get_lvalue(stmt), info);
     walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),dc_check_side_info,NULL,info);
   }
  break;

  case GEN_TYPE_STATEMENT:
//  walk_expression(stmt, dc_check_side_info, NULL, info);  
  break;
  }
 return WALK_CONTINUE;
}


//------------------------------------------------------------------
//  print out the SNODE information stored in the side array
//  for an array reference
//------------------------------------------------------------------
static int dc_check_side_info(AST_INDEX node, Generic info)
{
 int decomp_id, align_id, distrib_id;
 AST_INDEX ast;
 Context contxt;
 FortTree ft;

 if(is_subscript(node))
 {
  FortDRef* fd_ref = ((FortDInterface*)((struct s*)info)->fi)->GetRef(gen_SUBSCRIPT_get_name(node));
  fd_ref->Test();
  SNODE *sp = 
       (SNODE *)get_info(PedInfo(((struct s *)info)->Ped), gen_SUBSCRIPT_get_name(node), type_fd);

  if (sp != (SNODE *) NO_DC_INFO)
  {
   sp_dump(sp);
  }
 }
 return WALK_CONTINUE;
}

//------------------------------------------------------------------
// print the SNODE fields
// current version checks for distribution information 
//------------------------------------------------------------------
static void sp_dump(SNODE *sp)
{
 int i;
 cout<<"---------------Begin SNODE-------------- \n";
 cout<<"SP ADDRESS = %d \n", (int)sp;
 cout<<"name =  %s \n", sp->id;
 cout<<"numdim = %d \n", sp->numdim;
 cout<<"type   = %d \n",  sp->fform; 
 for(i=0;i<sp->numdim;++i)
  {
    cout<<"Begin Distribution Information \n";
    cout<<"upper_bound = %d  lower_bound = %d \n", sp_get_upper(sp, i), sp_get_lower(sp,i);
    cout<<"overlap_upper = %d   overlap lower = %d \n", sp_max_access(sp, i), sp_min_access(sp, i);
    switch(sp->distinfo[i]->distr_type)
   {
    case FD_DIST_LOCAL:
    cout<<"dim number = %d  distribution = %s, blocksize1 = %d \n", i, "Local", sp->distinfo[i]->blocksize1;
    break;

    case FD_DIST_BLOCK:
    cout<<"dim number = %d  distribution = %s, blocksize1 = %d \n", i, "Block", sp->distinfo[i]->blocksize1;
    break;

    default: 
    cout<<"unknown distribution \n";
 }
 }
  cout<<"-------------End SNODE------------- \n"; 
}



