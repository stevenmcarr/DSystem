/* $Id: PabloLib.C,v 1.2 1997/03/27 20:33:53 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// $Id: PabloLib.C,v 1.2 1997/03/27 20:33:53 carr Exp $
//**********************************************************************
// Interface to Pablo trace capture library routines
//**********************************************************************

//-------------------------------------------------------------------------
// INCLUDES
//
#include <stdio.h>
#include <string.h>

#ifndef groups_h
#include <libs/frontEnd/ast/groups.h>
#endif
#ifndef gen_h
#include <libs/frontEnd/ast/gen.h>
#endif

#ifndef FD_CODEGEN			// For Dist_Globals* (and ?)
#include <libs/fortD/codeGen/private_dc.h>
#endif

#ifndef HashTable_h			// For canned string hash functions
#include <libs/support/tables/HashTable.h>
#endif
					// From the performance tool world
#include <libs/fortD/performance/instr/PabloLib.h>
#ifndef instrument_spmd_h
#include <libs/fort_d/performance/instr/InstrumentSpmd.h>
#endif	/* #ifndef instrument_spmd_h */

//-------------------------------------------------------------------------
// GLOBAL (EXTERN) DECLARATIONS

// These should be private (i.e., STATIC) but cannot be because they are
// declared friend of class TagTable and class PabloTraceCaptureLib
// respectively (see PabloLib.h)
void CleanupTagsEntryCallback(Generic /*name*/, Generic value);

//-------------------------------------------------------------------------
// PRIVATE DECLARATIONS
// /*---- Symbolic constants; Macros ----*/


//-------------------------------------------------------------------------
// Declarations of functions private to this file

void SymVarTableEntryCleanup(Generic name, Generic value);


//**********************************************************************
// Member functions for class PabloTraceCaptureLib
//**********************************************************************

// all functions have `trace',
// versions, so use full list

//-----------------------------------------------------------------------
// Constructor and Destructor

PabloTraceCaptureLib::PabloTraceCaptureLib()
    : auxTagListFileName("AUX_INSTR_TAG_LIST")
{
    procTagTable = new TagTable(PROC_TAG_BASE_VALUE,
				   "PROCCOUNT", "ProcENT", "ProcEXT");
    loopTagTable = new TagTable(0, "LOOPCOUNT", "LoopENT", "LoopEXT");
    loopTagTableForCurProc = (TagTable*) NULL;
    SymOldVarTable = (NameValueTable*) NULL;
    
    systemInfo = (LibSystemInfo*) NULL;
    curProcInfo = (LibCurrentProcInstrInfo*) NULL;
    curProcName[0] = '\000';
    curLoopName[0] = '\000';
}

PabloTraceCaptureLib::~PabloTraceCaptureLib()
{
    delete procTagTable;
    delete loopTagTable;
    if (loopTagTableForCurProc != (TagTable*) NULL)
	delete loopTagTableForCurProc;
    Assert (SymOldVarTable == (NameValueTable*) NULL); // deleted for each proc
    Assert (curProcInfo == (LibCurrentProcInstrInfo*) NULL);
    Assert (systemInfo == (LibSystemInfo *) NULL);
}

//-----------------------------------------------------------------------

SystemInfo* PabloTraceCaptureLib::SetupSystemInfo(Fd_Arch_type arch)
{
    Assert (systemInfo == (LibSystemInfo*) NULL);
    return (SystemInfo*) (systemInfo = new LibSystemInfo(arch));
}

SystemInfo* PabloTraceCaptureLib::DeleteSystemInfo()
{
    Assert (systemInfo != (LibSystemInfo*) NULL);
    delete systemInfo;
    return (SystemInfo*) (systemInfo = (LibSystemInfo*) NULL);
}

CurrentProcInstrInfo* PabloTraceCaptureLib::SetupCurProcInfo(Dist_Globals *dh)
{
    Assert (curProcInfo == (LibCurrentProcInstrInfo*) NULL);
    return curProcInfo = new LibCurrentProcInstrInfo(dh);
}

CurrentProcInstrInfo* PabloTraceCaptureLib::DeleteCurProcInfo()
{
    Assert (curProcInfo != (LibCurrentProcInstrInfo*) NULL);
    delete curProcInfo;
    return curProcInfo = (LibCurrentProcInstrInfo*) NULL;
}

//-----------------------------------------------------------------------
// Public interface functions: Initialization and Wrapup for overall program.

// ---- Init for Pablo: initialize each kind of instrumentation. ----

AST_ChangeFlags
PabloTraceCaptureLib::InitAtPgmEntry(AST_INDEX declPoint,
				     AST_INDEX initPoint)
{
    Assert(curProcInfo->is_main_program());
    AST_ChangeFlags ret = INSTR_INSERTED_NONE;
    
    // I would like to test here for efficiency but ProcDeclAtPgmEntry()
    // needs to declare tags for instrumented runtime procedures, even when
    // EntryExitInstrumented() == false.
    // Once instrumenting at call-sites is implemented, these external
    // tags shouldnt be necessary, so this test could be put back.
    // 
    // if (EntryExitInstrumented() || LoopsInstrumented()
    // 	   || MesgsInstrumented()  || SymbsInstrumented())
    //
    
    ret |= ProcDeclAtPgmEntry(declPoint);
    ret |= LoopDeclAtPgmEntry(declPoint);
    ret |= SymbDeclAndInitAtEntry(declPoint, initPoint);
    ret |= PabloInitAtPgmEntry(initPoint);
    ret |= ProcInitAtPgmEntry(initPoint);
    ret |= LoopInitAtPgmEntry(initPoint);
    ret |= MesgInitAtPgmEntry(initPoint);

    return ret;
}

// ---- Wrapup for Pablo: just call endtracing() ----

AST_ChangeFlags
PabloTraceCaptureLib::WrapupAtPgmExit(AST_INDEX exitPoint)
{
    Assert(curProcInfo->is_main_program());
    if (EntryExitInstrumented() || LoopsInstrumented()
	|| MesgsInstrumented()  || SymbsInstrumented())
    {
	list_insert_before(exitPoint, pt_gen_call("endtracing", AST_NIL));
	return INSTR_INSERTED_BEFORE;
    }
    else
	return INSTR_INSERTED_NONE;
}

//-----------------------------------------------------------------------
// Public interface functions: Initialization and Wrapup in each procedure

AST_ChangeFlags
PabloTraceCaptureLib::InitAtProcEntry(AST_INDEX declPoint,
				      AST_INDEX initPoint)
{
    Assert(! curProcInfo->is_main_program());
    AST_ChangeFlags ret = INSTR_INSERTED_NONE;
    ret |= ProcDeclAtProcEntry(declPoint);	// Only declarations in the
    ret |= LoopDeclAtProcEntry(declPoint);	// first three cases, 
    ret |= MesgDeclAtProcEntry(declPoint);	// no initialization.
    ret |= SymbDeclAndInitAtEntry(declPoint, initPoint); // Needs init.
    return ret;
}
    
//-----------------------------------------------------------------------
// Public interface functions: Procedure entry/exit instrumentation:

// Entry:
// -- Create entry, exit tags for this procedure
// -- Generate: "call FDtrace(1, entry-tag-name, staticId, SIZEOF(staticId))"

AST_ChangeFlags
PabloTraceCaptureLib::InstrProcEntry(AST_INDEX stmt,
				     StaticInfo staticInfo)
{
    // if (curProcInfo->is_main_program()) return INSTR_INSERTED_NONE;
    char *procName = curProcInfo->get_proc_name();
    InstrumentingEntryExit();
    procTagTable->CreateTags(procName);
    return insertFDtrace(stmt, TRACE_FULL,
			 procTagTable->GetEntryTagName(procName), staticInfo);
}
					 
// Exit:
// -- Generate: "call FDtrace(1, exit-tag-name, staticId, SIZEOF(staticId))"

AST_ChangeFlags
PabloTraceCaptureLib::InstrProcExit(AST_INDEX stmt,
				    StaticInfo staticInfo)
{
    // if (curProcInfo->is_main_program()) return INSTR_INSERTED_NONE;
    char *exitTagName =
	procTagTable->GetExitTagName(curProcInfo->get_proc_name());
    return insertFDtrace(stmt, TRACE_FULL, exitTagName, staticInfo);
}

//-----------------------------------------------------------------------
// Public interface functions: Loop entry/exit instrumentation:

// Entry:
// -- Create entry, exit tags for this loop
// -- Generate: "call FDtrace(1, entry-tag-name, staticId, SIZEOF(staticId))"
// ASSUMES that this is the first call to instrument this loop.

AST_ChangeFlags
PabloTraceCaptureLib::InstrLoopEntry(AST_INDEX stmt,
				     StaticInfo staticInfo)
{
    NewLoop();
    char *loopName = curLoopName;
    InstrumentingLoops();
    loopTagTableForCurProc->CreateTags(loopName);
    return insertFDtrace(stmt, TRACE_FULL,
	     loopTagTableForCurProc->GetEntryTagName(loopName), staticInfo);
}

// Exit:
// -- Generate: "call FDtrace(1, exit-tag-name, staticId, SIZEOF(staticId))"
// ASSUMES that InstrLoopEntry has already been called for this loop.

AST_ChangeFlags
PabloTraceCaptureLib::InstrLoopExit(AST_INDEX stmt,
				    StaticInfo staticInfo)
{
    //curProcInfo->InstrumentingLoops();
    char *loopName = curLoopName;
    return insertFDtrace(stmt, TRACE_FULL,
	     loopTagTableForCurProc->GetExitTagName(loopName), staticInfo);
}

void PabloTraceCaptureLib::NewLoop()
{
    if (strcmp(curProcName, curProcInfo->get_proc_name()) != 0) {
	// Then, this is the first loop of a new procedure.
	// The contents of loopTagTableForCurProc should have been copied
	// into loopTagTable at the end of the last procedure:
	
	Assert(loopTagTableForCurProc == (TagTable*) NULL);
	strcpy(curProcName, curProcInfo->get_proc_name());
	loopTagTableForCurProc = new TagTable(LOOP_TAG_BASE_VALUE
					        + loopTagTable->NumberOfTags(),
					      "", "", "");
    }
    else {
	// Not the first loop in the current procedure:
	Assert(loopTagTableForCurProc != (TagTable*) NULL);
    }
	
    char strtmp[20];
    pt_itoa(1 + loopTagTableForCurProc->NumberOfTags(), strtmp);
    strcpy(curLoopName, curProcName);
    strcat(curLoopName, "_loop");
    strcat(curLoopName, strtmp);
}
					 
//-----------------------------------------------------------------------
// Public interface functions: Message passing instrumentation

AST_ChangeFlags
PabloTraceCaptureLib::InstrMesgCall(AST_INDEX invocation,
				    AST_INDEX stmt,
				    StaticInfo static_info)
{
    AST_INDEX invocation_name_ast = gen_INVOCATION_get_name(invocation);
    char *invocation_name =
	string_table_get_text(ast_get_symbol(invocation_name_ast));
    if (systemInfo->InstrumentableMesgCall(invocation_name)) {
	InstrumentingMesgs();
	return ReplaceMesgCall(invocation, invocation_name, stmt, static_info);
    }
    else return INSTR_INSERTED_NONE;
}

//-----------------------------------------------------------------------
// Public interface functions: Print the value of an integer expression
// at runtime as a "Symbolic" record.
// Insert the print call before specified statement stmt.
// If addGuard==true, include a guard to test if the expression has changed
// value since the last time it was printed.

// NOTE: Calling routine must ensure that intExpr can be evaluated multiple
// (1 or 3) times with no undesirable side-effects.
// Evaluate into a tmp var and pass that in to avoid such problems.

AST_ChangeFlags
PabloTraceCaptureLib::PrintSymbolic(AST_INDEX intExpr, AST_INDEX stmt,
				    StaticInfo static_info,
				    Boolean addGuard, int initialValue)
{
    // Assert(curProfInfo->getTypeOfExpr(intExpr) == INTTYPE);
    
    // In this case, the eventTagNum should be equal to the Static Id, so
    // make  sure given static tag doesn't overlap other dynamic tag values
    Assert((int) static_info < SYM_TAG_MAX_VALUE);
    
    AST_INDEX oldValueVar, guard, stmtList, ifStmt;
    AST_ChangeFlags rc = INSTR_INSERTED_NONE;

    InstrumentingSymbs();
    
    if (addGuard) {
	// If guard requested, generate the following code sequence:
	//        if (LASTvalue .ne. value) then
	//            call FDtrace(...)
	//            LASTvalue = value
	//        endif
	// In this case, insert the FDtrace() call before assignment inside
	// guard and insert the entire IF stmt before given stmt

	oldValueVar = MakeOldValueAst(intExpr, static_info, initialValue);
	guard = gen_BINARY_NE(oldValueVar, tree_copy(intExpr));
	stmtList = gen_ASSIGNMENT(AST_NIL,
				  tree_copy(oldValueVar), tree_copy(intExpr));
	guard = gen_GUARD(AST_NIL, guard, list_create(stmtList));
	ifStmt = gen_IF(AST_NIL, AST_NIL, list_create(guard));
	list_insert_before(stmt, ifStmt);
	rc |= INSTR_INSERTED_BEFORE;
    }
    else				// If guard not needed, directly insert
	stmtList = stmt;		// FDtrace() call before given stmt
    
    rc |= insertFDtrace(stmtList, TRACE_SYMBOLIC, static_info, intExpr);
    return rc;
}

//-----------------------------------------------------------------------
// Private class functions: Overall library initialization

AST_ChangeFlags
PabloTraceCaptureLib::PabloInitAtPgmEntry(AST_INDEX initPoint)
{
    Assert(curProcInfo->is_main_program());
    if (EntryExitInstrumented() || LoopsInstrumented()
	|| MesgsInstrumented()  || SymbsInstrumented())
    {
	// Generate: call FDinitTrace()
	list_insert_before(initPoint, pt_gen_call("FDinitTrace", AST_NIL));
	return INSTR_INSERTED_BEFORE;
    }
    else
	return INSTR_INSERTED_NONE;
}
			       
//-----------------------------------------------------------------------
// Private class functions: Procedure entry/exit instrumentation

// ---- Declarations in main program ----

AST_ChangeFlags
PabloTraceCaptureLib::ProcDeclAtPgmEntry(AST_INDEX declPoint)
{
    Assert(curProcInfo->is_main_program());
    
    //----- Get tags for external Fortran routines from aux file ----
    // Tag entries are inserted in procTagTable
    getTagsFromAuxFile(auxTagListFileName);

    if (procTagTable->NumberOfTags() == 0) { 	  // i.e., no internal *or*
	Assert(EntryExitInstrumented() == false); // external tags
	return INSTR_INSERTED_NONE;
    }

    //---- Insert declarations for all procedure instrumentation variables ----
    return procTagTable->insertAllTagDeclarations(declPoint);
}

// ---- Initialization in main program ----
// First, insert data statements for tag arrays, ProcENT and ProcEXT)
// Then, generate: call FDinitProcTrace(PROCCOUNT, ProcENT, ProcEXT)

AST_ChangeFlags
PabloTraceCaptureLib::ProcInitAtPgmEntry(AST_INDEX initPoint)
{
    Assert(curProcInfo->is_main_program());
    if (procTagTable->NumberOfTags() == 0) {
	Assert(EntryExitInstrumented() == false);
	return INSTR_INSERTED_NONE;
    }
    
    AST_ChangeFlags rc = INSTR_INSERTED_NONE;
    rc |= procTagTable->insertTagArrayDataStatements(initPoint);
    
    AST_INDEX
    node =list_create(pt_gen_ident(procTagTable->GetCountVarName()));
    node =list_insert_last(node,pt_gen_ident(procTagTable->GetEntryVarName()));
    node =list_insert_last(node, pt_gen_ident(procTagTable->GetExitVarName()));
    (void) list_insert_before(initPoint,
			  pt_gen_call(systemInfo->ProcTraceInitCall(), node));
    add_blank_line_before(initPoint);	// Since loop data stmts may be next
    rc |= INSTR_INSERTED_BEFORE;
    
    return rc;
}
			       
// Declarations in current procedure:
//		common /<proc_name/ <proc_entry_tag>, <proc_exit_tag>
//		integer <proc_entry_tag>, <proc_exit_tag>
// ASSUMES: lib_instr_proc_entry has already been called (see Assertion).

AST_ChangeFlags
PabloTraceCaptureLib::ProcDeclAtProcEntry(AST_INDEX declPoint)
{
    Assert(! curProcInfo->is_main_program());
    if (! EntryExitInstrumentedInCurProc())
	 return INSTR_INSERTED_NONE;
    else return procTagTable->insertTagDeclarations(declPoint,
					       curProcInfo->get_proc_name());
}


// ---- Get names of runtime library routines to be traced ----
// (e.g., buf1d...).  The file lists: proc_name, entry_tag_name, exit_tag_name
// Tag names must be procnameENTRY, procnameEXIT (as created in TagTable)

void
PabloTraceCaptureLib::getTagsFromAuxFile(char *aux_tag_list_file_name)
{
    char procname[MAX_NAME], entry_tag[MAX_NAME], exit_tag[MAX_NAME];
    FILE *aux_tag_list_file;

    if ((aux_tag_list_file = fopen(aux_tag_list_file_name, "r")) == NULL) {
	// fprintf(stderr, "PabloLib: Can't open aux file %s.\n",
	// 	aux_tag_list_file_name);
	return;
    }

    while (fscanf(aux_tag_list_file, "%s %s %s", procname,entry_tag,exit_tag)
	   == 3)
    {
	procTagTable->CreateTags(procname);
	char* entry = (char *) procTagTable->lookupEntry(procname); // using
				// char* because actual type not visible here
	if (strcmp(procTagTable->GetEntryTagName(entry), entry_tag)
	    || strcmp(procTagTable->GetExitTagName(entry), exit_tag))
	    fprintf(stderr, "\nWARNING: Tags in aux file for procedure %s\n"
		    	    "\t do not match computed tags = %s, %s\n"
		    	    "\t Trace files may be corrupted.\n\n",
		    procname,
		    procTagTable->GetEntryTagName(entry),
		    procTagTable->GetExitTagName(entry));
    }
    (void) fclose(aux_tag_list_file);
}

//-----------------------------------------------------------------------
// Private class functions: Loop entry/exit instrumentation

// ---- Declarations in main program ----
// Insert declarations for all loop instrumentation variables

AST_ChangeFlags
PabloTraceCaptureLib::LoopDeclAtPgmEntry(AST_INDEX declPoint)
{
    Assert(curProcInfo->is_main_program());
    
    if (LoopsInstrumentedInCurProc()) {
	Assert(loopTagTableForCurProc != (TagTable*) NULL);
	loopTagTable->Absorb(loopTagTableForCurProc);
	loopTagTableForCurProc = (TagTable*) NULL;
    }
    
    if (! LoopsInstrumented()) {
	Assert(loopTagTable->NumberOfTags() == 0);
	return INSTR_INSERTED_NONE;
    }
    else
	return loopTagTable->insertAllTagDeclarations(declPoint);
}

// ---- Initialization in main program ----
// First, insert data statements for tag arrays, LoopENT and LoopEXT
// Then, generate: call FDinitLoopTrace(LOOPCOUNT, LoopENT, LoopEXT)

AST_ChangeFlags
PabloTraceCaptureLib::LoopInitAtPgmEntry(AST_INDEX initPoint)
{
    Assert(curProcInfo->is_main_program());
    Assert(loopTagTableForCurProc == (TagTable*) NULL);	// LoopDeclAtPgmEntry 
				// must be called before LoopInitAtPgmEntry.
    if (! LoopsInstrumented()) {
	Assert(loopTagTable->NumberOfTags() == 0);
	return INSTR_INSERTED_NONE;
    }
    
    AST_ChangeFlags rc = INSTR_INSERTED_NONE;
    rc |= loopTagTable->insertTagArrayDataStatements(initPoint);
    
    AST_INDEX
    node =list_create(pt_gen_ident(loopTagTable->GetCountVarName()));
    node =list_insert_last(node,pt_gen_ident(loopTagTable->GetEntryVarName()));
    node =list_insert_last(node, pt_gen_ident(loopTagTable->GetExitVarName()));
    (void) list_insert_before(initPoint,
			  pt_gen_call(systemInfo->LoopTraceInitCall(), node));
    rc |= INSTR_INSERTED_BEFORE;
    
    return rc;
}
			       
// Declarations in current procedure:
//		common /<loop_name/ <loop_entry_tag>, <loop_exit_tag>
//		integer <loop_entry_tag>, <loop_exit_tag>

AST_ChangeFlags
PabloTraceCaptureLib::LoopDeclAtProcEntry(AST_INDEX declPoint)
{
    Assert(! curProcInfo->is_main_program());
    if (! LoopsInstrumentedInCurProc()) {
	Assert(loopTagTableForCurProc == (TagTable*) NULL); // Never allocated
	return INSTR_INSERTED_NONE;
    }
    AST_ChangeFlags rc =
    loopTagTableForCurProc->insertAllTagDeclarations(declPoint, false);
    loopTagTable->Absorb(loopTagTableForCurProc);
    loopTagTableForCurProc = (TagTable*) NULL;
    
    return rc;
}

//-----------------------------------------------------------------------
// Private class functions: Message instrumentation
//
// All instrumentation versions of msg routines return a value.
// If original stmt was:
//	call <msg-routine> ( <args> )
// replace it with:
//	instrDummy = FDtrace<msg-routine> ( <args>, <new_args> )
// else original statement was:
//      <ret>  = <msg-routine> ( <args> )
// replace it with:
//      <ret>  = FDtrace<msg-routine> ( <args>, <new_args> )

AST_ChangeFlags
PabloTraceCaptureLib::ReplaceMesgCall(AST_INDEX invocation,
				      char *invocation_name,
				      AST_INDEX stmt,  
				      StaticInfo staticInfo)
{
    // ---- First replace the name of the call <name> with FDtrace<name>
    char new_name[MAX_NAME];
    strcpy(new_name, "FDtrace");
    strcat(new_name, invocation_name);
    gen_INVOCATION_put_name(invocation, pt_gen_ident(new_name));
    systemInfo->recordInvocation(invocation_name);
    
    // ---- Then add 2 new arguments ----
    AST_INDEX node = gen_INVOCATION_get_actual_arg_LIST(invocation);
    Assert(is_list(node));
    node = list_insert_last(node, list_create(pt_gen_int(staticInfo)));
    node = list_insert_last(node, list_create(pt_gen_int(sizeof(int))));
    gen_INVOCATION_put_actual_arg_LIST(invocation, node);

    // ---- Finally, if its a "call", replace with function invocation:
    if (is_call(stmt)) {
	AST_INDEX new_node = tree_copy(invocation);
	new_node = gen_ASSIGNMENT(AST_NIL,pt_gen_ident("instrDummy"),new_node);
	tree_replace(stmt, new_node);
	tree_free(stmt);
	curProcInfo->saveReplaceNode(new_node);
	return INSTR_INSERTED_REPLACED;
    }
    return INSTR_INSERTED_NONE;		// Since only a son of the
}					// ASTnode stmt was replaced.

AST_ChangeFlags
PabloTraceCaptureLib::MesgDeclAtProcEntry(AST_INDEX declPoint)
{
    if (! MesgsInstrumentedInCurProc()) return INSTR_INSERTED_NONE;
    char new_name[MAX_NAME];
    for (int i = 0; i < systemInfo->NumMesgCallsToTrace(); i++) {
	if (systemInfo->NumInvocationsInCurProc(i) > 0) {
	    strcpy(new_name, "FDtrace");
	    strcat(new_name, systemInfo->MessageName(i));
	    list_insert_before(declPoint, int_decl_stmt(new_name));
	}
	systemInfo->ResetInvocationsInCurProc(i);
    }
    return INSTR_INSERTED_BEFORE;
}

// Initialization routines in main program:
//		call TRACE_INIT_CALL (name is system-specific)

AST_ChangeFlags
PabloTraceCaptureLib::MesgInitAtPgmEntry(AST_INDEX initPoint)
{
    Assert(curProcInfo->is_main_program());
    if (! MesgsInstrumented()) return INSTR_INSERTED_NONE;
    (void)list_insert_before(initPoint,
			pt_gen_call(systemInfo->MesgTraceInitCall(), AST_NIL));
    return INSTR_INSERTED_BEFORE;
}

//-----------------------------------------------------------------------
// Private class functions: Symbolics instrumentation

// MakeOldValueAst() :	Compose the name of the variable to hold "last" value
// 			of given symbolic expr to be traced at runtime.
// Current impl: OldVarName = "LAST<intExpr><staticID>", where <intExpr>
//                            is omitted if intExpr is not just an identifier

AST_INDEX PabloTraceCaptureLib::MakeOldValueAst(AST_INDEX intExpr,
						StaticInfo static_info,
						int initialValue)
{
    char  staticInfoStr[MAX_NAME];
    char* oldValueName = new char[MAX_NAME]; // Need name on heap to store
					     // in NameValueTable
    strcpy(oldValueName, "LAST");
    if (is_identifier(intExpr))
	strcat(oldValueName, gen_get_text(intExpr));
    pt_itoa((int) static_info, staticInfoStr);
    strcat(oldValueName, staticInfoStr);
    
    if (SymOldVarTable == (NameValueTable*) NULL) { // First time in this proc
	SymOldVarTable = new NameValueTable();
	SymOldVarTable->Create(SYM_VAR_TABLE_INIT_SIZE,
			    (NameHashFunctFunctPtr) StringHashFunct,
			    (NameCompareFunctPtr) StringEntryCompare,
			    (NameValueCleanupFunctPtr)SymVarTableEntryCleanup);
    }
    
    SymOldVarTable->AddNameValue((Generic) oldValueName,
				 (Generic) initialValue, /*oldValue*/ NULL);

    return pt_gen_ident(oldValueName);
}

void SymVarTableEntryCleanup(Generic name, Generic /*value*/)
{
    delete[] (char*) name;		// Allocated in MakeOldValueAst(..)
}

// SymbDeclAndInitAtEntry() :	Insert decl and initialization stmts in the
// 				current procedure for variables holding 
// 				last value of symbolics traced at runtime.

AST_ChangeFlags
PabloTraceCaptureLib::SymbDeclAndInitAtEntry(AST_INDEX declPoint,
					     AST_INDEX initPoint)
{
    if (! SymbsInstrumentedInCurProc()) {
	Assert(SymOldVarTable == (NameValueTable*) NULL);
	return INSTR_INSERTED_NONE;
    }
    Assert(SymOldVarTable != (NameValueTable*) NULL);
    
    int initialValue;
    char* symVarName;
    Boolean anEntryFound = false;
    NameValueTableIterator entries(SymOldVarTable);
    for ( ; symVarName = (char*) entries.name; ++entries) {
	anEntryFound = true;
	initialValue = (int) entries.value; // This may legitimately be 0
	(void) list_insert_before(declPoint, int_decl_stmt(symVarName));
	(void) list_insert_before(initPoint,
		  gen_ASSIGNMENT(AST_NIL, pt_gen_ident(symVarName),
				 pt_gen_int(initialValue)));
    }
    // Delete table. A new copy is created for each procedure.
    SymOldVarTable->Destroy();
    delete SymOldVarTable;
    SymOldVarTable = (NameValueTable*) NULL; // Used to find start of new proc
    
    Assert(anEntryFound);		// Else, SymbsInstrumentedInCurProc()
    add_blank_line_before(declPoint);	//   should have been false above.
    add_blank_line_before(initPoint);
    return INSTR_INSERTED_BEFORE;
}

//-----------------------------------------------------------------------
// Private class functions: interface to basic FDtrace routine
// Generates: call FDtrace(traceLevel, eventTagVarName, dataPtr, dataLen)

// Version 1: Where dataPtr field is a static ID (traceLevel != TRACE_SYMBOLIC)
// NOTE: if the value is known here, i.e., at compile time, there's no need
// to print a runtime value for it, so verify traceLevel != TRACE_SYMBOLIC.

AST_ChangeFlags
PabloTraceCaptureLib::insertFDtrace(AST_INDEX stmt, FD_TraceLevel traceLevel,
				    char *eventTagName, StaticInfo staticInfo)
{
    Assert(traceLevel != TRACE_SYMBOLIC);
    AST_INDEX node;
    node = list_create(pt_gen_int(traceLevel));
    node = list_insert_last(node, list_create(pt_gen_ident(eventTagName)));
    node = list_insert_last(node, list_create(pt_gen_int(staticInfo)));
    node = list_insert_last(node, list_create(pt_gen_int(4))); // SIZEOF(int)
    node = pt_gen_call("FDtrace", node);
    (void) list_insert_before(stmt, node); 
    return INSTR_INSERTED_BEFORE;
}

// Version 2: Where dataPtr field is an expression (must be integer type).
// Only useful for traceLevel == TRACE_SYMBOLIC since for all other cases,
// this field is the StaticId, which is known at compile time and so should
// use version 1.

AST_ChangeFlags
PabloTraceCaptureLib::insertFDtrace(AST_INDEX stmt, FD_TraceLevel traceLevel,
				    int eventTagNum, AST_INDEX intExpr)
{
    Assert(traceLevel == TRACE_SYMBOLIC);
    AST_INDEX
    node = list_create(pt_gen_int(traceLevel));
    node = list_insert_last(node, pt_gen_int(eventTagNum));
    node = list_insert_last(node, tree_copy(intExpr));
    node = list_insert_last(node, pt_gen_int(4)); // SIZEOF(int)
    node = pt_gen_call("FDtrace", node);
    (void) list_insert_before(stmt, node); 
    return INSTR_INSERTED_BEFORE;
}



//**********************************************************************
// Member functions for class LibSystemInfo for Pablo
//**********************************************************************

LibSystemInfo::LibSystemInfo(Fd_Arch_type arch) : SystemInfo(arch)
{
    if (arch != FD_NX) {
	fputs("ERROR: Only NX (iPSC) mesg-passing instr. supported", stderr);
	Assert(false);
    }
    procTraceInitCall = ::IPSC_PROC_TRACE_INIT_CALL;
    loopTraceInitCall = ::IPSC_LOOP_TRACE_INIT_CALL;
    mesgTraceInitCall = ::IPSC_MESG_TRACE_INIT_CALL;
}


//**********************************************************************
// Member functions for class LibCurrentProcInstrInfo::
// See interface definition file.
//**********************************************************************


//**********************************************************************
// Member functions for class TagTable
//**********************************************************************

//-----------------------------------------------------------------------
// Constructor and Destructor

TagTable::TagTable(int _tagBase, char* _countVarName,
		   char* _entryVarName, char* _exitVarName) :
tagBase(_tagBase)
{
    strcpy(countVarName, _countVarName);
    strcpy(entryVarName, _entryVarName);
    strcpy(exitVarName,  _exitVarName);

    curTagNum = 0;
    curInstrTags = (TagListEntry *) NULL;
    nameEntryPairs = new NameValueTable();
    nameEntryPairs->Create(TAG_LIST_INIT_SIZE,
			   (NameHashFunctFunctPtr) StringHashFunct,
			   (NameCompareFunctPtr) StringEntryCompare,
			   (NameValueCleanupFunctPtr)CleanupTagsEntryCallback);
}

TagTable::~TagTable()
{
    // TagListEntry* entry = (TagListEntry*) NULL;
    // NameValueTableIterator entries(nameEntryPairs);
    // for (; entry = (TagListEntry*) entries.value; entries++) {
	// delete entry;
    // }
    nameEntryPairs->Destroy(); // CleanupTagsEntryCallback (below) is
    delete nameEntryPairs;     //   automatically invoked on each entry.
}

// This is invoked on each name,value pair from NameValueTable::Destroy()
void CleanupTagsEntryCallback(Generic /*name*/, Generic value)
{
    delete (TagTable::TagListEntry*) value;	// friend of TagTable, so
}						// TagListEntry is accessible

void TagTable::CreateTags(char* unitName)
{
    ++curTagNum;			// Values start at 1.
    curInstrTags = new TagListEntry();
    curInstrTags->entryTagNumber = tagBase + curTagNum;
    curInstrTags->exitTagNumber = -curInstrTags->entryTagNumber;

    strcpy(curInstrTags->unitName, unitName);
    strcpy(curInstrTags->entryTagName, unitName);
    strcat(curInstrTags->entryTagName, "ENTRY");
    strcpy(curInstrTags->exitTagName,  unitName);
    strcat(curInstrTags->exitTagName,  "EXIT");

    // Now add (unitName, curInstrTags) pair to the HashTable.
    // Abort if there was already an entry for this unitName.
    // NOTE: 1st arg to AddNameValue must point to name in *permanent* storage.
    if (nameEntryPairs->AddNameValue((Generic) curInstrTags->unitName,
				     (Generic) curInstrTags, NULL/*oldValue*/))
	Assert(false);
}

//----- Get a pointer to the list entry corresponding to unitName -----
// Returns a void ptr to external clients which can be used to extract
// multiple fields from the same entry without avoid repeated hashing
// Calls QueryNameValue() on the hash table to get pointer matching unitName.

void* TagTable::lookupEntry(char* unitName)
{
    TagListEntry* entry = (TagListEntry*) NULL;
    if (! nameEntryPairs->QueryNameValue((Generic) unitName, (Generic*) &entry)
	|| entry == (TagListEntry*) NULL) Assert(false);
    return entry;
}

//--------------------------------------------------------------------
// Moved here because g++ gives a weird error if this code (with or without
// inlining) is put into the .h file.  Move back to the .h file and inline
// these functions when g++ is fixed.

// Specified by entry pointer ----

char* TagTable::GetUnitName(void* entry)
{ return ((TagListEntry *) entry)->unitName; }

char* TagTable::GetEntryTagName(void* entry)
{ return ((TagListEntry *) entry)->entryTagName; }

char* TagTable::GetExitTagName(void* entry)
{ return ((TagListEntry *) entry)->exitTagName; }

int TagTable::GetEntryTagNum(void* entry)
{ return ((TagListEntry *) entry)->entryTagNumber; }

int TagTable::GetExitTagNum(void* entry)
{ return ((TagListEntry *) entry)->exitTagNumber; }


// Specified by unitName

char* TagTable::GetEntryTagName(char* unitName)
{ return ((TagListEntry *) lookupEntry(unitName))->entryTagName; }

char* TagTable::GetExitTagName(char* unitName)
{ return ((TagListEntry *) lookupEntry(unitName))->exitTagName; }

int TagTable::GetEntryTagNum(char* unitName)
{ return ((TagListEntry *) lookupEntry(unitName))->entryTagNumber; }

int TagTable::GetExitTagNum(char* unitName)
{ return ((TagListEntry *) lookupEntry(unitName))->exitTagNumber; }

AST_ChangeFlags
TagTable::insertTagDeclarations(AST_INDEX here, char* unitName)
{ return insertTagDeclarations(here, (TagListEntry *) lookupEntry(unitName)); }

//--------------------------------------------------------------------
// Inserts the required declarations for a given tag pair entry

AST_ChangeFlags
TagTable::insertTagDeclarations(AST_INDEX here, TagListEntry* listEntry)
{
    AST_ChangeFlags
    ret  = insertTagsTypeDecl(here, listEntry);
    ret |= insertTagsParameterStmts(here, listEntry);
    return ret;
}
//------------------------------------------------------------------------ 

//******* UNUSED ********
// ---- Common block: ----
// Generates: COMMON /proc_name/ entry_tag_name, exit_tag_name

AST_ChangeFlags
TagTable::insertTagsCommonBlock(AST_INDEX here, TagListEntry* listEntry)
{
    char common_block_name[MAX_NAME];

    strcpy(common_block_name, "/");
    strcat(common_block_name, GetUnitName(listEntry));
    strcat(common_block_name, "/");

    AST_INDEX
    node = list_create(pt_gen_ident(GetEntryTagName(listEntry)));
    node = list_insert_last(node, pt_gen_ident(GetExitTagName(listEntry)));
    node = gen_COMMON_ELT(pt_gen_ident(common_block_name), node);
    node = gen_COMMON(AST_NIL, list_create(node));
    (void) list_insert_before(here, node); 

    return INSTR_INSERTED_BEFORE;
}
//***************

//------------------------------------------------------------------------ 
// Inserts: integer <tag_name>  for each tag in a tag pair

AST_ChangeFlags
TagTable::insertTagsTypeDecl(AST_INDEX here, TagListEntry* listEntry)
{
    (void) list_insert_before(here, int_decl_stmt(GetEntryTagName(listEntry)));
    (void) list_insert_before(here, int_decl_stmt(GetExitTagName(listEntry)));
    return INSTR_INSERTED_BEFORE;
}
//------------------------------------------------------------------------ 
// Inserts: parameter(<tag_name> = <tag_value>) stmt for a tag pair

AST_ChangeFlags
TagTable::insertTagsParameterStmts(AST_INDEX here, TagListEntry* listEntry)
{
    (void) list_insert_before(here,
			      int_parameter_stmt(GetEntryTagName(listEntry),
						 GetEntryTagNum(listEntry)));
    (void) list_insert_before(here,
			      int_parameter_stmt(GetExitTagName(listEntry),
						 GetExitTagNum(listEntry)));
    return INSTR_INSERTED_BEFORE;
}
//------------------------------------------------------------------------ 
// Inserts entry, exit tag declarations, and (if this is main pgm) also
// the declarations of the ENT() and EXT() arrays.

AST_ChangeFlags
TagTable::insertAllTagDeclarations(AST_INDEX here, Boolean isMainProgram)
{
    TagListEntry* entry;
    AST_INDEX node, tnode, node_list1, dim_list;

    //--- Insert decl and parameter stmts for proc entry and exit tags ----
    NameValueTableIterator entries(nameEntryPairs);
    for (; entry = (TagListEntry*) entries.value; ++entries) {
	insertTagDeclarations(here, entry);
	add_blank_line_before(here);
    }

    if (! isMainProgram)		// Tag arrays only used in main
	return INSTR_INSERTED_BEFORE;	// pgm, so return otherwise

    /*--- Generate: declaration and PARAMETER stmts for Count ----*/
    (void) list_insert_before(here, int_decl_stmt(countVarName));
    (void) list_insert_before(here,int_parameter_stmt(countVarName,curTagNum));
    add_blank_line_before(here);

    /*---- Generate decl INTEGER ENT(Count), EXT(Count) ----*/

    dim_list = list_create(gen_DIM(AST_NIL,pt_gen_ident(countVarName)));
    node = gen_ARRAY_DECL_LEN(pt_gen_ident(entryVarName), AST_NIL,
			      dim_list, AST_NIL);
    node_list1 = list_create(node);
    node = gen_ARRAY_DECL_LEN(pt_gen_ident(exitVarName), AST_NIL,
			      tree_copy(dim_list), AST_NIL);
    node_list1 = list_insert_last(node_list1, node);

    tnode = gen_TYPE_LEN(gen_INTEGER(), AST_NIL);
    node = gen_TYPE_STATEMENT(AST_NIL, tnode, node_list1);
    ft_SetComma(node, false);

    (void) list_insert_before(here, node);
    add_blank_line_before(here);

    return INSTR_INSERTED_BEFORE;
}

//------------------------------------------------------------------------ 
// Inserts data statements for initializing the ENT() and EXT() arrays:
// Generate DATA ENT/ ... /, DATA EXT/ ... / statements

AST_ChangeFlags
TagTable::insertTagArrayDataStatements(AST_INDEX here)
{
    TagListEntry* entry;
    AST_INDEX node, node_list1, node_list2;
    
    NameValueTableIterator entries(nameEntryPairs);
    node_list1 = list_create(AST_NIL);
    for (; (entry = (TagListEntry*) entries.value); ++entries) {
	node = pt_gen_ident(GetEntryTagName(entry));
	(void) list_insert_last(node_list1, node);
    }
    
    node_list2 = list_create(pt_gen_ident(entryVarName));
    node = gen_DATA_ELT(node_list2, node_list1);
    node = gen_DATA(AST_NIL, list_create(node));
    (void) list_insert_before(here, node);
    add_blank_line_before(here);

    node_list1 = list_create(AST_NIL);
    for (entries.Reset(); (entry = (TagListEntry*) entries.value); ++entries) {
	node = pt_gen_ident(GetExitTagName(entry));
	(void) list_insert_last(node_list1, node);
    }
    node_list2 = list_create(pt_gen_ident(exitVarName));
    node = gen_DATA_ELT(node_list2, node_list1);
    node = gen_DATA(AST_NIL, list_create(node));

    (void) list_insert_before(here, node);
    add_blank_line_before(here);

    return INSTR_INSERTED_BEFORE;
}

//------------------------------------------------------------------------ 
// Append contents of given table into this table

void TagTable::Absorb(TagTable* aTagTable)
{
    if (aTagTable == (TagTable*) NULL)
	return;
    TagListEntry *entry, *newEntry;
    NameValueTableIterator entries(aTagTable->nameEntryPairs);
    for (; entry = (TagListEntry*) entries.value; ++entries) {
	++curTagNum;
	newEntry = new TagListEntry();				 // Allocate,
	strcpy(newEntry->unitName,     entry->unitName);	 // then copy
	strcpy(newEntry->entryTagName, entry->entryTagName);	 // contents,
	strcpy(newEntry->exitTagName,  entry->exitTagName);
	newEntry->entryTagNumber     = entry->entryTagNumber;
	newEntry->exitTagNumber      = entry->exitTagNumber;
					// And insert into our hash table
	if (nameEntryPairs->AddNameValue((Generic) newEntry->unitName,
					 (Generic) newEntry, NULL/*oldValue*/))
	    Assert(false);
    }
    delete aTagTable;
}


//**********************************************************************
// Global (static) helper functions.
//**********************************************************************


//**********************************************************************
