/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
#include <general.h>
#include <mh.h>
#include <mh_ast.h>
#include <fort/walk.h>
#include <fort/groups.h>
#include <pt_util.h>
#include <header.h>
#include <annotate.h>
#include <mem_util.h>
#include <string.h>
#include <DirectivesInclude.h>

#include <misc/Arena.h>

EXTERN(void, message,(char *str));

static int AddCall(AST_INDEX node,
		   CallInfoType *CallInfo)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX ArgList,Stmt,NewStmt;
   AST_INDEX node1;
   char      TextConstant[80];

     if (is_subscript(node))
       {
	ArgList = list_create(tree_copy_with_type(node));
	NewStmt = pt_gen_call(CallInfo->CacheRoutine,ArgList);
	if (is_guard(CallInfo->Stmt))
	  Stmt = tree_out(CallInfo->Stmt);
	else
	  Stmt = CallInfo->Stmt;
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

static void ConvertToCall(AST_INDEX stmt,
			  SymDescriptor symtab)

  {
   Directive Dir;
   char *Function;
   AST_INDEX Call;

     if (a2i_string_parse(gen_get_text(gen_COMMENT_get_text(stmt)),&Dir,symtab))
       {
	if (Dir.Instr == PrefetchInstruction)
	  Function = "prefetch_line";
	else
	  Function = "dead_line";
	Call = pt_gen_call(Function,Dir.Subscript);
	pt_tree_replace(stmt,Call);
       }
  }
	

static int InsertCacheCalls(AST_INDEX     stmt,
			    int           level,
			    CallInfoType  *CallInfo)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {

     CallInfo->Stmt = stmt;
     strcpy(CallInfo->CacheRoutine,"cache_load");
     if (is_assignment(stmt))
       {
	walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),NOFUNC,
			(WK_EXPR_CLBACK)AddCall,(Generic)CallInfo);
	strcpy(CallInfo->CacheRoutine,"cache_store");
	walk_expression(gen_ASSIGNMENT_get_lvalue(stmt),NOFUNC,
			(WK_EXPR_CLBACK)AddCall,(Generic)CallInfo);
       }
     else if (is_guard(stmt))
       walk_expression(gen_GUARD_get_rvalue(stmt),NOFUNC,(WK_EXPR_CLBACK)AddCall,
		       (Generic)CallInfo);
     else if (is_write(stmt))
       walk_expression(gen_WRITE_get_data_vars_LIST(stmt),NOFUNC,
		       (WK_EXPR_CLBACK)AddCall,(Generic)CallInfo);
     else if (is_print(stmt))
       walk_expression(gen_PRINT_get_data_vars_LIST(stmt),NOFUNC,
		       (WK_EXPR_CLBACK)AddCall,(Generic)CallInfo);
     else if (is_read_short(stmt))
       {
	strcpy(CallInfo->CacheRoutine,"cache_store");
	walk_expression(gen_READ_SHORT_get_data_vars_LIST(stmt),NOFUNC,
			(WK_EXPR_CLBACK)AddCall,(Generic)CallInfo);
       }
     else if (is_read_long(stmt))
       {
	strcpy(CallInfo->CacheRoutine,"cache_store");
	walk_expression(gen_READ_LONG_get_io_LIST(stmt),NOFUNC,
			(WK_EXPR_CLBACK)AddCall,(Generic)CallInfo);
       }
     else if (is_logical_if(stmt))
       walk_expression(gen_LOGICAL_IF_get_rvalue(stmt),NOFUNC,
		       (WK_EXPR_CLBACK)AddCall,(Generic)CallInfo);
     else if (is_arithmetic_if(stmt))
       walk_expression(gen_ARITHMETIC_IF_get_rvalue(stmt),NOFUNC,
		       (WK_EXPR_CLBACK)AddCall,(Generic)CallInfo);
     else if (is_do(stmt))
       walk_expression(gen_DO_get_control(stmt),NOFUNC,(WK_EXPR_CLBACK)AddCall,
		       (Generic)CallInfo);
     else if (is_if(stmt) || is_continue(stmt) || is_goto(stmt) ||
	      is_computed_goto(stmt) || is_call(stmt) || 
	      is_format(stmt) || is_stop(stmt));
     else if (is_comment(stmt))
       {
	ConvertToCall(stmt,CallInfo->symtab);
	return(WALK_FROM_OLD_NEXT);
       }
     else if (executable_stmt(stmt))
       { 
	char errmsg[30];
	
	if (!is_return(stmt))
	  {
	   sprintf(errmsg,"Statement not handled %d\n",NT(stmt));	 
	   message(errmsg);
	  }
       }
     return(WALK_CONTINUE);
  }

void memory_AnnotateWithCacheCalls(AST_INDEX    root,
				   int          level,
				   char          *routine,
				   FortTextTree  ftt,
				   SymDescriptor symtab)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   CallInfoType CallInfo;

     CallInfo.routine = routine;
     CallInfo.symtab = symtab;
     walk_statements(root,level,(WK_STMT_CLBACK)InsertCacheCalls,NOFUNC,
		     (Generic)&CallInfo);
  }
