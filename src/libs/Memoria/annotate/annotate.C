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
#include <std.h>
#include <mem_util.h>

#include <misc/Arena.h>

EXTERN(void, message,(char *str));

static int CountArrays(AST_INDEX       node,
		       TableInfoType   *TableInfo)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
     if (is_subscript(node))
       {
	TableInfo->count++;
	PutLineNumber(node,TableInfo->LineNum);
       }
     return(WALK_CONTINUE);
  }

static int GetTableInfo(AST_INDEX       stmt,
			int             level,
			TableInfoType   *TableInfo)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
     TableInfo->LineNum++;
     if (is_assignment(stmt))
       {
	walk_expression(gen_ASSIGNMENT_get_lvalue(stmt),CountArrays,NOFUNC,
			(Generic)TableInfo);
	walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),CountArrays,NOFUNC,
			(Generic)TableInfo);
       }
     else if (is_guard(stmt))
       walk_expression(gen_GUARD_get_rvalue(stmt),CountArrays,NOFUNC,
			(Generic)TableInfo);
     else if (is_logical_if(stmt))
       walk_expression(gen_LOGICAL_IF_get_rvalue(stmt),CountArrays,NOFUNC,
		       (Generic)TableInfo);
     else if (is_write(stmt))
       walk_expression(gen_WRITE_get_data_vars_LIST(stmt),CountArrays,NOFUNC,
		       (Generic)TableInfo);
     else if (is_read_short(stmt))
       walk_expression(gen_READ_SHORT_get_data_vars_LIST(stmt),CountArrays,
		       NOFUNC,(Generic)TableInfo);
     else if (is_read_long(stmt))
       walk_expression(gen_READ_LONG_get_io_LIST(stmt),CountArrays,
		       NOFUNC,(Generic)TableInfo);
     else if (is_arithmetic_if(stmt))
       walk_expression(gen_ARITHMETIC_IF_get_rvalue(stmt),CountArrays,NOFUNC,
		       (Generic)TableInfo);
     else if (is_call(stmt))
       walk_expression(gen_CALL_get_invocation(stmt),CountArrays,NOFUNC,
		       (Generic)TableInfo);
     return(WALK_CONTINUE);
  }

static int get_value(AST_INDEX     node,
		     int           *index)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   if (is_identifier(node))
     *index = *index + atoi(gen_get_text(node));
   else if (is_constant(node))
     *index = *index + atoi(gen_get_text(node));
   else if (is_binary_plus(node))
     *index = *index + GEN_BINARY_PLUS;
   else if (is_binary_minus(node))
     *index = *index + GEN_BINARY_MINUS;
   else if (is_binary_times(node))
     *index = *index + GEN_BINARY_TIMES;
   else if (is_binary_divide(node))
     *index = *index + GEN_BINARY_DIVIDE;
   return(WALK_CONTINUE);
  }
     

int memory_ArrayTableHash(ArrayTableType *ArrayTable,
			  AST_INDEX       node,
			  int             size)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/


  {
   int index;

     index = atoi(gen_get_text(gen_SUBSCRIPT_get_name(node)));
     walk_expression(gen_SUBSCRIPT_get_rvalue_LIST(node),
		     get_value,NOFUNC,(Generic)&index);
     index = index % size;
     while (!is_null_node(ArrayTable[index].node) && 
            !pt_expr_equal(node,ArrayTable[index].node))
       index = (index + 1) % size;
     return(index);
  }


static int add_elements(AST_INDEX      node,
			TableInfoType  *TableInfo)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   int index; 

     if (is_subscript(node))
       {
	index = memory_ArrayTableHash(TableInfo->ArrayTable,node,
				      TableInfo->count);
	if (TableInfo->ArrayTable[index].node == (Generic)NULL)
	  {
	   TableInfo->ArrayTable[index].node = tree_copy_with_type(node);
	   TableInfo->ArrayTable[index].node = tree_copy_with_type(node);
	   TableInfo->ArrayTable[index].Text = malloc(sizeof(char)*80);
	   ut_GetSubscriptText(node,TableInfo->ArrayTable[index].Text);
           TableInfo->MaxLength = 
	    (strlen(TableInfo->ArrayTable[index].Text) > TableInfo->MaxLength ?
	     strlen(TableInfo->ArrayTable[index].Text) : TableInfo->MaxLength);
	  }
       }
     return(WALK_CONTINUE);
  }
    
static int BuildArrayTable(AST_INDEX         stmt,
			   int               level,
			   Generic           TableInfo)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   if (is_assignment(stmt))
     {
      walk_expression(gen_ASSIGNMENT_get_lvalue(stmt),NOFUNC,add_elements,
		      TableInfo);
      walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),NOFUNC,add_elements,
		      TableInfo);
     }
   else if (is_guard(stmt))
     walk_expression(gen_GUARD_get_rvalue(stmt),NOFUNC,add_elements,
		     TableInfo);
   else if (is_logical_if(stmt))
     walk_expression(gen_LOGICAL_IF_get_rvalue(stmt),NOFUNC,add_elements,
		     TableInfo);
   else if (is_write(stmt))
     walk_expression(gen_WRITE_get_data_vars_LIST(stmt),NOFUNC,add_elements,
		     TableInfo);
   else if (is_read_short(stmt))
     walk_expression(gen_READ_SHORT_get_data_vars_LIST(stmt),NOFUNC,
		     add_elements,TableInfo);
   else if (is_arithmetic_if(stmt))
     walk_expression(gen_ARITHMETIC_IF_get_rvalue(stmt),NOFUNC,add_elements,
		     TableInfo);
   else if (is_call(stmt))
     walk_expression(gen_CALL_get_invocation(stmt),NOFUNC,add_elements,
		     TableInfo);
   return(WALK_CONTINUE);
  }

void memory_BuildArrayTableInfo(AST_INDEX root,
				int       level,
				TableInfoType *TableInfo,
				arena_type    *ar)

  {
   TableInfo->count = 0;
   TableInfo->LineNum = 1;
   walk_statements(root,level,GetTableInfo,NOFUNC,(Generic)TableInfo);
   TableInfo->ArrayTable = (ArrayTableType*)ar->
                arena_alloc_mem_clear(LOOP_ARENA,TableInfo->count*
				      sizeof(ArrayTableType));
   walk_statements(root,level,BuildArrayTable,NOFUNC,(Generic)TableInfo);
  }

static int AddCall(AST_INDEX node,
		   CallInfoType *CallInfo)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX ArgList;
   int       index;
   AST_INDEX node1;
   char      TextConstant[80];

     if (is_subscript(node))
       {
	ArgList = list_create(tree_copy_with_type(node));
	node1 = gen_CONSTANT();
	sprintf(TextConstant,"'%s\\0'",CallInfo->routine);
	gen_put_text(node1,TextConstant,STR_TEXT_STRING);
	list_insert_last(ArgList,node1);
	node1 = gen_CONSTANT();
	sprintf(TextConstant,"%%val(%d)",GetLineNumber(node));
	gen_put_text(node1,TextConstant,STR_TEXT_STRING);
        list_insert_last(ArgList,node1);
	index = memory_ArrayTableHash(CallInfo->TableInfo->ArrayTable,
				      node,CallInfo->TableInfo->count);
	node1 = gen_CONSTANT();
	sprintf(TextConstant,"%%val(%d)",index);
	gen_put_text(node1,TextConstant,STR_TEXT_STRING);
        list_insert_last(ArgList,node1);
	list_insert_before(CallInfo->Stmt,pt_gen_call(CallInfo->CacheRoutine,
						      ArgList));
       }
     return(WALK_CONTINUE);
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
	walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),NOFUNC,AddCall,
			(Generic)CallInfo);
	strcpy(CallInfo->CacheRoutine,"cache_store");
	walk_expression(gen_ASSIGNMENT_get_lvalue(stmt),NOFUNC,AddCall,
			(Generic)CallInfo);
       }
     else if (is_guard(stmt))
       walk_expression(gen_GUARD_get_rvalue(stmt),NOFUNC,AddCall,
		       (Generic)CallInfo);
     else if (is_write(stmt))
       walk_expression(gen_WRITE_get_data_vars_LIST(stmt),NOFUNC,AddCall,
		       (Generic)CallInfo);
     else if (is_read_short(stmt))
       {
	strcpy(CallInfo->CacheRoutine,"cache_store");
	walk_expression(gen_READ_SHORT_get_data_vars_LIST(stmt),NOFUNC,
			AddCall,(Generic)CallInfo);
       }
     else if (is_read_long(stmt))
       {
	strcpy(CallInfo->CacheRoutine,"cache_store");
	walk_expression(gen_READ_LONG_get_io_LIST(stmt),NOFUNC,
			AddCall,(Generic)CallInfo);
       }
     else if (is_logical_if(stmt))
       walk_expression(gen_LOGICAL_IF_get_rvalue(stmt),NOFUNC,AddCall,
		       (Generic)CallInfo);
     else if (is_arithmetic_if(stmt))
       walk_expression(gen_ARITHMETIC_IF_get_rvalue(stmt),NOFUNC,AddCall,
		       (Generic)CallInfo);
     else if (is_do(stmt))
       walk_expression(gen_DO_get_control(stmt),NOFUNC,AddCall,
		       (Generic)CallInfo);
     else if (is_if(stmt) || is_continue(stmt) || is_goto(stmt) ||
	      is_computed_goto(stmt) || is_call(stmt));
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
				   TableInfoType *TableInfo,
				   FortTextTree  ftt)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   CallInfoType CallInfo;

     CallInfo.routine = routine;
     CallInfo.TableInfo = TableInfo;
     walk_statements(root,level,InsertCacheCalls,NOFUNC,(Generic)&CallInfo);
  }
