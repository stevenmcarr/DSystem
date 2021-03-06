/* $Id: ldst.C,v 1.4 1997/03/27 20:22:30 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
#include <libs/support/misc/general.h>
#include <libs/Memoria/include/mh.h>
#include <libs/Memoria/include/mh_ast.h>
#include <libs/frontEnd/include/walk.h>
#include <libs/frontEnd/ast/groups.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>
#include <libs/Memoria/include/header.h>
#include <libs/Memoria/annotate/ldst.h>
#include <libs/Memoria/include/mem_util.h>
#include <string.h>

#include <libs/support/memMgmt/Arena.h>

EXTERN(void, message,(char *str));
 
static void AddPrints(AST_INDEX Stmt)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX NewStmt,node1,PrintList;
   char TextConstant[80];

     node1 = gen_CONSTANT();
     sprintf(TextConstant,"'%s'","Number of Loads = ");
     gen_put_text(node1,TextConstant,STR_TEXT_STRING);
     PrintList = list_create(node1);
     PrintList = list_insert_last(PrintList,pt_gen_ident("NumLoads"));
     NewStmt = gen_PRINT(AST_NIL,gen_STAR(),PrintList);
     if (gen_get_label(Stmt) != AST_NIL)
       {
	gen_ASSIGNMENT_put_lbl_def(NewStmt,
				   tree_copy_with_type(gen_get_label(Stmt)));
	pt_tree_replace(gen_get_label(Stmt),AST_NIL);
       }
     list_insert_before(Stmt,NewStmt);
     node1 = gen_CONSTANT();
     sprintf(TextConstant,"'%s'","Number of Stores = ");
     gen_put_text(node1,TextConstant,STR_TEXT_STRING);
     PrintList = list_create(node1);
     PrintList = list_insert_last(PrintList,pt_gen_ident("NumStores"));
     NewStmt = gen_PRINT(AST_NIL,gen_STAR(),PrintList);
     list_insert_before(Stmt,NewStmt);
     node1 = gen_CONSTANT();
     sprintf(TextConstant,"'%s'","Number of Memory Accesses = ");
     gen_put_text(node1,TextConstant,STR_TEXT_STRING);
     PrintList = list_create(node1);
     PrintList = list_insert_last(PrintList,pt_gen_add(pt_gen_ident("NumStores"),
						       pt_gen_ident("NumLoads")));
     NewStmt = gen_PRINT(AST_NIL,gen_STAR(),PrintList);
     list_insert_before(Stmt,NewStmt);
  }


static int AddIncrement(AST_INDEX node,
			IncInfoType *IncInfo)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX Stmt,NewStmt;

     if (is_subscript(node))
       {
	if (IncInfo->IsLoad)
	  NewStmt = gen_ASSIGNMENT(AST_NIL,pt_gen_ident("NumLoads"),
				   pt_gen_add(pt_gen_ident("NumLoads"),pt_gen_int(1)));
	else
	  NewStmt = gen_ASSIGNMENT(AST_NIL,pt_gen_ident("NumStores"),
				   pt_gen_add(pt_gen_ident("NumStores"),pt_gen_int(1)));
	if (is_guard(IncInfo->Stmt))
	  Stmt = tree_out(IncInfo->Stmt);
	else
	  Stmt = IncInfo->Stmt;
	if (gen_get_label(Stmt) != AST_NIL)
	  {
	   gen_ASSIGNMENT_put_lbl_def(NewStmt,
				      tree_copy_with_type(gen_get_label(Stmt)));
	   pt_tree_replace(gen_get_label(Stmt),AST_NIL);
	  }
	list_insert_before(Stmt,NewStmt);
       }
     return(WALK_CONTINUE);
  }

static int InsertIncrements(AST_INDEX     stmt,
			    int           level,
			    IncInfoType  *IncInfo)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {

     IncInfo->Stmt = stmt;
     IncInfo->IsLoad = true;
     if (is_assignment(stmt))
       {
	walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),NOFUNC,
			(WK_EXPR_CLBACK)AddIncrement,(Generic)IncInfo);
	IncInfo->IsLoad = false;
	walk_expression(gen_ASSIGNMENT_get_lvalue(stmt),NOFUNC,
			(WK_EXPR_CLBACK)AddIncrement,(Generic)IncInfo);
       }
     else if (is_guard(stmt))
       walk_expression(gen_GUARD_get_rvalue(stmt),NOFUNC,(WK_EXPR_CLBACK)AddIncrement,
		       (Generic)IncInfo);
     else if (is_write(stmt))
       walk_expression(gen_WRITE_get_data_vars_LIST(stmt),NOFUNC,
		       (WK_EXPR_CLBACK)AddIncrement,(Generic)IncInfo);
     else if (is_print(stmt))
       walk_expression(gen_PRINT_get_data_vars_LIST(stmt),NOFUNC,
		       (WK_EXPR_CLBACK)AddIncrement,(Generic)IncInfo);
     else if (is_read_short(stmt))
       {
	IncInfo->IsLoad = false;
	walk_expression(gen_READ_SHORT_get_data_vars_LIST(stmt),NOFUNC,
			(WK_EXPR_CLBACK)AddIncrement,(Generic)IncInfo);
       }
     else if (is_read_long(stmt))
       {
	IncInfo->IsLoad = false;
	walk_expression(gen_READ_LONG_get_io_LIST(stmt),NOFUNC,
			(WK_EXPR_CLBACK)AddIncrement,(Generic)IncInfo);
       }
     else if (is_logical_if(stmt))
       walk_expression(gen_LOGICAL_IF_get_rvalue(stmt),NOFUNC,
		       (WK_EXPR_CLBACK)AddIncrement,(Generic)IncInfo);
     else if (is_arithmetic_if(stmt))
       walk_expression(gen_ARITHMETIC_IF_get_rvalue(stmt),NOFUNC,
		       (WK_EXPR_CLBACK)AddIncrement,(Generic)IncInfo);
     else if (is_do(stmt))
       walk_expression(gen_DO_get_control(stmt),NOFUNC,(WK_EXPR_CLBACK)AddIncrement,
		       (Generic)IncInfo);
     else if (is_if(stmt) || is_continue(stmt) || is_goto(stmt) ||
	      is_computed_goto(stmt) || is_call(stmt) || 
	      is_format(stmt) || is_return(stmt) || is_data(stmt) || is_entry(stmt) ||
	      is_close(stmt) || is_open(stmt) || is_rewind_short(stmt) ||
	      is_rewind_long(stmt));
     else if (is_stop(stmt))
       AddPrints(stmt);
     else if (executable_stmt(stmt))
       { 
	char errmsg[30];
	
	sprintf(errmsg,"Statement not handled %d\n",NT(stmt));	 
	message(errmsg);
       }
     return(WALK_CONTINUE);
  }

void memory_AnnotateWithLDSTCount(AST_INDEX    root,
				  int          level,
				  Boolean      MainProgram)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   IncInfoType IncInfo;

     IncInfo.MainProgram = MainProgram;
     walk_statements(root,level,(WK_STMT_CLBACK)InsertIncrements,NOFUNC,
		     (Generic)&IncInfo);
  }
