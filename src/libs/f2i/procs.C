/* $Id: procs.C,v 1.4 2001/09/14 18:18:35 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/************************************************************************/
/*                                      				*/
/* Process one fortran procedure, program, etc  			*/
/*                                      				*/
/************************************************************************/
#include <libs/support/misc/general.h>
#include <sys/file.h>

#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/support/lists/list.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/frontEnd/include/gi.h>
#include <libs/frontEnd/ast/asttree.h>
#include <libs/frontEnd/ast/aphelper.h>
#include <stdio.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/ipAnalysis/interface/IPQuery.h>
#include <libs/support/strings/rn_string.h>

#include <libs/f2i/ai.h>
#include <libs/f2i/sym.h>
#include <libs/f2i/f2i_label.h>
#include <libs/f2i/classes.h>
#include <libs/f2i/call.h>

#include <libs/f2i/mnemonics.h>

/* Modifications -
 *
 *	No longer insert the procedure name into the Symbol Table
 *
 *
 *
 *
 *
 */

/* local space */
/************************************************************************/
/* local and global variables and forward procedure declarations	*/
/************************************************************************/
static char name_buffer[64];

extern int proc_type;	/* shared with prepass.c, stmts.c & static.c */
extern AST_INDEX formal_list;

static void initialize(void);
static void AddFormalName(int, int);


/************************************************************************/
/* aiProcedures                         				*/
/* 1) initialize                        				*/
/* 2) foreach procedure:                				*/
/*    a) get "parts" (statement list, name, formal args, etc)   	*/
/*    b) process the procedure, generating iloc 	        	*/
/* 3) print debugging info (symbol table, labels, etc)  		*/
/************************************************************************/
void aiProcedures(AST_INDEX root, FortTree ft)
  // AST_INDEX root;
  // FortTree             ft;

{
  AST_INDEX 		procedure_list;
  AST_INDEX 		procedure;
  AST_INDEX 		stmt_list;
  int                   index;

  /* do the necessary initializations */

  LabelCreateTable( LABEL_DEFAULT_SIZE );
  iloc_rep_init();	/* the tables to decode the iloc representation */
  
  /* now walk the list of procedures */
  procedure_list = gen_GLOBAL_get_subprogram_scope_LIST(root);
  procedure 	  = list_first(procedure_list);
  /* save the root -- needed later */
  root_node = procedure;
  
  while (procedure != AST_NIL) {
    proc_name	= NULL;
    formal_list = AST_NIL;
    stmt_list	= AST_NIL;
    proc_type	= gen_get_node_type(procedure);
    
    switch (proc_type) {
    case GEN_PLACE_HOLDER:
      stmt_list = ast_null_node;
      break;
    case GEN_PROGRAM:
      proc_name	= (char *) gen_get_text(gen_PROGRAM_get_name(procedure));
      stmt_list	= gen_PROGRAM_get_stmt_LIST(procedure);
      break;
    case GEN_SUBROUTINE:
      proc_name	= (char *) gen_get_text(gen_SUBROUTINE_get_name(procedure));
      formal_list = gen_SUBROUTINE_get_formal_arg_LIST(procedure);
      stmt_list	= gen_SUBROUTINE_get_stmt_LIST(procedure);
      break;
    case GEN_FUNCTION:
      proc_name	= (char *) gen_get_text(gen_FUNCTION_get_name(procedure));
      formal_list = gen_FUNCTION_get_formal_arg_LIST(procedure);
      stmt_list	= gen_FUNCTION_get_stmt_LIST(procedure);
      break;
    case GEN_BLOCK_DATA:
      proc_name	= (char *) gen_get_text(gen_BLOCK_DATA_get_name(procedure));
      stmt_list	= gen_BLOCK_DATA_get_stmt_LIST(procedure);
      break;
    case GEN_COMMENT:
      if (aiDebug > 0)
	(void) fprintf(stdout, "Encountered inter-procedure comment\n");
      break;
    default:
      ERROR("Ilocgen", "Unknown top level node!", FATAL);
    }
    
    /* process the procedure */
    if (proc_name != NULL) {

      ft_SymTable = ft_SymGetTable(ft, proc_name);

/*      if (aiSymDump > 0)
	fst_Dump(ft_SymTable);
*/
      fst_ForAll(ft_SymTable, InitSymTab, 0);

      initialize();

/*      if (aiSymDump > 0)
	fst_Dump(ft_SymTable);
*/
      (void) fprintf(stderr, "ai: processing '%s':\n", proc_name);
      /* r0 is the STACK POINTER */

      index = SymInsertSymbol("_stack", TYPE_INTEGER, OC_IS_SPECIAL,
			     0, SC_NO_MEMORY, NO_ALIAS);

      fst_PutFieldByIndex(ft_SymTable, index, SYMTAB_REG, 0);

      if (proc_type == GEN_PROGRAM) {
	/* r1 is the STATIC DATA AREA POINTER */
	if (aiSparc) 
	{
	  proc_name = ssave("_MAIN_");
	  aiRecordStaticLabel("_.MAIN_");
	}
	else if (aiRt) 
	{
	  proc_name = ssave("_.MAIN_");
	  aiRecordStaticLabel("_MAIN_");
	}
	else if (aiRocket)
	  {
	    proc_name = ssave("main");
	    aiRecordStaticLabel("__main");
	  }
	else
	  {
	    proc_name = ssave("_main");
	    aiRecordStaticLabel("_main");
	  }
	proc_text = ssave("main");

	aiParameters(stmt_list);
	aiStmtList(stmt_list);

	/* all we need for an end of a routine is a stop for ROCKET 
		is a HALT
	generate(0, COMMENT, 0, 0, 0, "Procedure epilog code");
	generate(aiEpilogue, NOP, 0, 0, 0, NOCOMMENT);
	aiExit(getIntConstantInRegister("0"), "0"); */
	generate(0, HALT, 0, 0, 0, "This is an end");

	aiGenerateCommons();
	aiGenerateStaticArea();
      }
      else if (proc_type == GEN_BLOCK_DATA) {
	/* r1 is the STATIC DATA AREA POINTER */
	if (aiSparc > 0) {
	  (void) sprintf(name_buffer, "_.%s_", proc_name);
	  aiRecordStaticLabel(name_buffer);
	  (void) sprintf(name_buffer, "_%s_", proc_name);
	}
	else if (aiRocket) {
	  (void) sprintf(name_buffer, "__%s", proc_name);
	  aiRecordStaticLabel(name_buffer);
	  (void) sprintf(name_buffer, "%s", proc_name);
	}
	else {
	  (void) sprintf(name_buffer, "_%s_", proc_name);
	  aiRecordStaticLabel(name_buffer);
	  (void) sprintf(name_buffer, "_.%s_", proc_name);
	}
	
	proc_text = proc_name;
	proc_name = ssave(name_buffer);

	aiParameters(stmt_list);
	aiStmtList(stmt_list);	/* builds up the DATA representation */
	
	aiGenerateStaticArea();	/* outputs it */
      }
      else {
	/* r1 is the STATIC DATA AREA POINTER */
	if (aiSparc > 0)
	  (void) sprintf(name_buffer, "_.%s_", proc_name);
	else if (aiRocket)
	  (void) sprintf(name_buffer, "__%s", proc_name);
	else
	  (void) sprintf(name_buffer, "_%s_", proc_name);
	
	aiRecordStaticLabel(name_buffer);

	/* if this is a function, r2 is the returned value */
	/* change 6/10/91 - grn
	 * if the function is of type character, the name
	 * should be treated as a parameter data object
	 * because the caller allocates (stack) space for the character
	 * string
	 */
	if (proc_type == GEN_FUNCTION && 
	    gen_get_real_type(gen_FUNCTION_get_name(procedure))
	    == TYPE_CHARACTER) {

	  index = SymInsertSymbol(proc_name,
		  gen_get_real_type(gen_FUNCTION_get_name(procedure)),
		  OC_IS_DATA | OC_IS_FORMAL_PAR, 0, SC_NO_MEMORY,
		  NO_ALIAS);
	  aiRecordFunctionValueIndex(index);
	}
	else if (proc_type == GEN_FUNCTION)
	  {
	    index = SymInsertSymbol(proc_name,
		    gen_get_real_type(gen_FUNCTION_get_name(procedure)),
		    OC_IS_DATA, 0, SC_NO_MEMORY, NO_ALIAS);
          aiRecordFunctionValueIndex(index);
	  }
	if (aiSparc > 0)
	  (void) sprintf(name_buffer, "_%s_", proc_name);
	else if (aiRocket)
	  (void) sprintf(name_buffer, "%s", proc_name);
	else
	  (void) sprintf(name_buffer, "_.%s_", proc_name);
	
	proc_text = proc_name;
	proc_name = ssave(name_buffer);
	
	if (formal_list != AST_NIL)
	  aiFormals(formal_list);
	
	aiParameters(stmt_list);
	aiStmtList(stmt_list);
	aiProcedureEpilogue(procedure, formal_list, proc_type);
	aiGenerateCommons();
	aiGenerateStaticArea();
      }
      /* clean up */
      (void) sfree(proc_name);
      aiIdfaFini();
    }
    
    /* and move on to the next procedure */
    procedure	= list_next(procedure);
  }
  
  /* and perform the necessary finalizations */
  if (aiSymDump) {
   fst_Short_Dump();
    LabelDumpTable();
    if (aiSymDump >= 3) 
      fst_ForAll (ft_SymTable, CommonDumpTable, 0);
  }
  LabelDestroyTable();
} /* aiProcedures */


static int first_time = TRUE;




/* fail if file contains multiple procedures */
static void initialize()
{
  if (first_time)
     first_time = FALSE;
  else {
    ERROR("Reinitialize", "Multiple procedures in a single file", WARNING);
    ERROR("Reinitialize", "Reinitialization is not yet implemented", FATAL);
  }
} /* initialize */


static int FormalNames[MAXPARMS];




/* add the symbol table index for a formal parameter to the list */
static void AddFormalName(int i, int sym)
//      int  i;
//      int  sym;
{
  if (i < MAXPARMS)
    FormalNames[i] = sym;
  else
    ERROR("AddFormalName",
	  "FormalNames index is out of range.  Raise MAXPARMS", FATAL);
} /* AddFormalName */




/* return the symbol table index for the ith formal parameter */
int aiFormalName(int i)
  //     int i;
{
  if (i < 1 || aiNumParameters < i) {
    (void) sprintf(error_buffer, "Formal parameter index is out of range (%d)", i);
    ERROR("aiFormalName", error_buffer, SERIOUS);
    i = 1;
  }
  return FormalNames[i];
} /* aiFormalName */




/* process the formal parameters */
void aiFormals(AST_INDEX node)
  // AST_INDEX	node;
{
  STR_TEXT	text;
  int		index;
  int		type;
  
  if (aiDebug > 0)
    (void) fprintf(stdout, "aiFormals(%d).\n", node);
  
  node = list_first(node);	/* arg checked for == NIL before call */
  
  while (node != ast_null_node) {
    text  = gen_get_text( node );
    type  = gen_get_real_type(node);
    
    if (type == TYPE_UNKNOWN) {
      type = TYPE_INTEGER;
      (void) sprintf(error_buffer,
		 "Type checker hasn't assigned a type to formal '%s' (%d)",
		 text, aiNumParameters);
      ERROR("aiFormals", error_buffer, WARNING);
      ERROR("aiFormals", "Type integer assumed (procedure?)", WARNING);
    }

    index = fst_QueryIndex (ft_SymTable, text);
  
    fst_PutFieldByIndex(ft_SymTable, index, SYMTAB_alias, ALIASED);
    fst_PutFieldByIndex(ft_SymTable, index, SYMTAB_STORAGE_CLASS, SC_NO_MEMORY
);
    node  = list_next( node );
    
    aiNumParameters ++;
    fst_my_PutFieldByIndex(ft_SymTable, index, SYMTAB_NEXT_COMMON,  aiNumParameters); 
    
    AddFormalName(aiNumParameters, index);
    
    if (aiDebug > 0)
      (void) fprintf(stdout, "aiFormals() found '%s' as formal # %d.\n",
		     text, aiNumParameters);
  }
} /* aiFormals */




/* generate code for an exit generated by a2i    */
/* HandleStop should be used for STOP statements */
void aiExit(int reg, char *message)
//      int  reg;
//      char *message;
{
  char list[16];	
  char refs[32];
  char *code;

  int address_reg;
  int storage_class;

  /* set the labels for the data and the code */
  if (aiSparc > 0)
    code = "_exit";
  else if (aiRt > 0)
    code = "_.exit";
  else
    code = "exit";

  /* save old storage class and change symbol table to indicate "STACK" */
  storage_class = fst_my_GetFieldByIndex(ft_SymTable, reg, SYMTAB_STORAGE_CLASS);
  fst_my_PutFieldByIndex(ft_SymTable, reg, SYMTAB_STORAGE_CLASS, SC_STACK);

  /* compute and save the offset */
  fst_my_PutFieldByIndex(ft_SymTable, reg, SYMTAB_offset, aiNextStack);
  aiNextStack += fst_my_GetFieldByIndex(ft_SymTable, reg, SYMTAB_SIZE);

  /* retrieve the address register */
  address_reg = getAddressInRegister(reg);

  /* store the constant */
  generate_store(address_reg, reg, TYPE_INTEGER, reg,"&unknown");

  /* restore the appropriate storage class */
  fst_my_PutFieldByIndex(ft_SymTable, reg, SYMTAB_STORAGE_CLASS, storage_class);

  /* create the parameter list and refs list */
  (void) sprintf(refs, "@%s", message);
  address_reg = fst_my_GetFieldByIndex(ft_SymTable, address_reg, SYMTAB_REG);
  (void) sprintf(list, "r%d", address_reg);

  /* declare the name */
  generate(0, NAME, (Generic) code, 0, 0, NOCOMMENT);

  /* generate the subroutine call for the exit */
  generate_long(0, JSRl, (Generic) code, aiStackBase(), (Generic) list, 
		(Generic) /*refs*/ "?", (Generic) "", 0, GEN_STRING, NOCOMMENT);
} /* aiExit */


