/* $Id: OldPabloLib.C,v 1.1 1997/03/11 14:28:51 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// $Id: OldPabloLib.C,v 1.1 1997/03/11 14:28:51 carr Exp $
//**********************************************************************
// Interface to Pablo trace capture library routines
//**********************************************************************

//-------------------------------------------------------------------------
// INCLUDES
//
#include <stdio.h>
#include <string.h>
// #include <Assert.h>

#ifndef groups_h
#include <libs/frontEnd/ast/groups.h>
#endif

#ifndef FD_CODEGEN			// For Dist_Globals* (and ?)
#include <libs/fortD/codeGen/private_dc.h>
#endif

#ifndef HashTable_h			// For canned string hash functions
#include <libs/support/tables/HashTable.h>
#endif
					// From the performance tool world
#include <libs/fortD/performance/instr/OldPabloLib.h>
#ifndef PabloLib_h
#include <libs/fortD/performance/instr/PabloLib.h>
#endif
#ifndef instrument_spmd_h
#include <libs/fort_d/performance/instr/InstrumentSpmd.h>
#endif	/* #ifndef instrument_spmd_h */

//-------------------------------------------------------------------------
// GLOBAL (EXTERN) DECLARATIONS

//-------------------------------------------------------------------------
// PRIVATE DECLARATIONS
// /*---- Symbolic constants; Macros ----*/


//-------------------------------------------------------------------------
// Declarations of functions private to this file

// This should be private (i.e., static) but cannot be because it is
// declared friend of class TagTable in PabloLib.h
void CleanupEntryCallback(Generic /*name*/, Generic value);


//**********************************************************************
// Member functions for class PabloOldTraceCaptureLib
//**********************************************************************

// all functions have `trace',
// versions, so use full list

//-----------------------------------------------------------------------
// Constructor and Destructor

PabloOldTraceCaptureLib::PabloOldTraceCaptureLib()
    : auxTagListFileName("AUX_INSTR_TAG_LIST")
{
      procTagTable = new TagTable(PROC_TAG_BASE_VALUE,	
				     "PROCCOUNT", "ProcENT", "ProcEXT");
      
      // LOOP_TAG_BASE_VALUE specified in tag table for cur proc
      loopTagTable = new TagTable(0, "LOOPCOUNT", "LoopENT", "LoopEXT");
      
      systemInfo = (PabloOldLibSystemInfo*) NULL;
      curProcInfo = (LibCurrentProcInstrInfo*) NULL;
      curLoopName[0] = '\0';
      curProcName[0] = '\0';
      loopTagTableForCurProc = (TagTable*) NULL;	// for purify
}

PabloOldTraceCaptureLib::~PabloOldTraceCaptureLib()
{
    delete procTagTable;
    delete loopTagTable;
    delete loopTagTableForCurProc;
    assert (curProcInfo == (LibCurrentProcInstrInfo*) NULL);
    assert (systemInfo == (PabloOldLibSystemInfo *) NULL);
}

//-----------------------------------------------------------------------

SystemInfo* PabloOldTraceCaptureLib::SetupSystemInfo(Fd_Arch_type arch)
{
    assert (systemInfo == (PabloOldLibSystemInfo*) NULL);
    return (SystemInfo*) (systemInfo = new PabloOldLibSystemInfo(arch));
}

SystemInfo* PabloOldTraceCaptureLib::DeleteSystemInfo()
{
    assert (systemInfo != (PabloOldLibSystemInfo*) NULL);
    delete systemInfo;
    return (SystemInfo*) (systemInfo = (PabloOldLibSystemInfo*) NULL);
}

CurrentProcInstrInfo*
PabloOldTraceCaptureLib::SetupCurProcInfo(Dist_Globals *dh)
{
    assert (curProcInfo == (LibCurrentProcInstrInfo*) NULL);
    return curProcInfo = new LibCurrentProcInstrInfo(dh);
}

CurrentProcInstrInfo* PabloOldTraceCaptureLib::DeleteCurProcInfo()
{
    assert (curProcInfo != (LibCurrentProcInstrInfo*) NULL);
    delete curProcInfo;
    return curProcInfo = (LibCurrentProcInstrInfo*) NULL;
}

//-----------------------------------------------------------------------
// Public interface functions: Initialization and Wrapup for overall program.

// ---- Init for Pablo: initialize each kind of instrumentation. ----

AST_ChangeFlags
PabloOldTraceCaptureLib::InitAtPgmEntry(AST_INDEX declPoint,
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
    // if (EntryExitInstrumented()|| LoopsInstrumented()|| MesgsInstrumented())
    
    ret |= ProcDeclAtPgmEntry(declPoint);
    ret |= LoopDeclAtPgmEntry(declPoint);
    ret |= ProcInitAtPgmEntry(initPoint);
    ret |= LoopInitAtPgmEntry(initPoint);
    ret |= MesgInitAtPgmEntry(initPoint);
    return ret;
}

// ---- Wrapup for Pablo: just call endtracing() ----

AST_ChangeFlags
PabloOldTraceCaptureLib::WrapupAtPgmExit(AST_INDEX exitPoint)
{
    Assert(curProcInfo->is_main_program());
    if (EntryExitInstrumented()|| LoopsInstrumented() || MesgsInstrumented()) {
	list_insert_before(exitPoint, pt_gen_call("endtracing", AST_NIL));
	return INSTR_INSERTED_BEFORE;
    }
    else
	return INSTR_INSERTED_NONE;
}

//-----------------------------------------------------------------------
// Public interface functions: Initialization and Wrapup in each procedure

AST_ChangeFlags
PabloOldTraceCaptureLib::InitAtProcEntry(AST_INDEX declPoint,
				      AST_INDEX /* initPoint */)
{
    Assert(! curProcInfo->is_main_program());
    AST_ChangeFlags ret = INSTR_INSERTED_NONE;
    ret |= ProcDeclAtProcEntry(declPoint);	// Only declarations here
    ret |= LoopDeclAtProcEntry(declPoint);	// and here,
    ret |= MesgDeclAtProcEntry(declPoint);	// no initialization.
    return ret;
}
    
//-----------------------------------------------------------------------
// Public interface functions: Procedure entry/exit instrumentation:

// Entry:
// -- Create entry, exit tags for this procedure
// -- Generate: "call traceevent(entry-tag-name, 0, 0)

AST_ChangeFlags
PabloOldTraceCaptureLib::InstrProcEntry(AST_INDEX stmt,
					StaticInfo /* staticInfo */)
{
    if (curProcInfo->is_main_program()) return INSTR_INSERTED_NONE;
    char *procName = curProcInfo->get_proc_name();
    InstrumentingEntryExit();
    procTagTable->CreateTags(procName);
    return insertTraceevent(stmt, procTagTable->GetEntryTagName(procName),0,0);
}
					 
// Exit:
// -- Generate: "call traceevent(exit-tag-name, 0, 0)"

AST_ChangeFlags
PabloOldTraceCaptureLib::InstrProcExit(AST_INDEX stmt,
				       StaticInfo /* staticInfo */)
{
    if (curProcInfo->is_main_program()) return INSTR_INSERTED_NONE;
    char *exitTagName =
	procTagTable->GetExitTagName(curProcInfo->get_proc_name());
    return insertTraceevent(stmt, exitTagName, 0, 0);
}

//-----------------------------------------------------------------------
// Public interface functions: Loop entry/exit instrumentation:

// Entry:
// -- Create entry, exit tags for this loop
// -- Generate: "call FDtrace(1, entry-tag-name, staticId, SIZEOF(staticId))"
// ASSUMES that this is the first call to instrument this loop.

AST_ChangeFlags
PabloOldTraceCaptureLib::InstrLoopEntry(AST_INDEX stmt,
				     StaticInfo /* staticInfo */)
{
    NewLoop();
    char *loopName = curLoopName;
    InstrumentingLoops();
    loopTagTableForCurProc->CreateTags(loopName);
    return insertTraceevent(stmt,
	    loopTagTableForCurProc->GetEntryTagName(loopName), 0, 0);
}

// Exit:
// -- Generate: "call FDtrace(1, exit-tag-name, staticId, SIZEOF(staticId))"
// ASSUMES that InstrLoopEntry has already been called for this loop.

AST_ChangeFlags
PabloOldTraceCaptureLib::InstrLoopExit(AST_INDEX stmt,
				       StaticInfo /* staticInfo */)
{
    //curProcInfo->InstrumentingLoops();
    char *loopName = curLoopName;
    return insertTraceevent(stmt,
	    loopTagTableForCurProc->GetExitTagName(loopName), 0, 0);
}

void PabloOldTraceCaptureLib::NewLoop()
{
    if (strcmp(curProcName, curProcInfo->get_proc_name()) != 0) {
	// OThen, this is the first loop of a new procedure.
	// The contents of loopTagTableForCurProc should have been copied
	// into loopTagTable at the end of the last procedure:
	
	Assert(loopTagTableForCurProc == (TagTable*) NULL);
	strcpy(curProcName, curProcInfo->get_proc_name());
	loopTagTableForCurProc = new TagTable(LOOP_TAG_BASE_VALUE
					        + loopTagTable->NumberOfTags(),
					      "", "", "");
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
PabloOldTraceCaptureLib::InstrMesgCall(AST_INDEX invocation,
				       AST_INDEX  /* stmt */,
				       StaticInfo /* static_info */)
{
    AST_INDEX invocation_name_ast = gen_INVOCATION_get_name(invocation);
    char *invocation_name =
	string_table_get_text(ast_get_symbol(invocation_name_ast));
    if (systemInfo->InstrumentableMesgCall(invocation_name)) {
	InstrumentingMesgs();
	return ReplaceMesgCall(invocation, invocation_name);
    }
    else return INSTR_INSERTED_NONE;
}

//-----------------------------------------------------------------------
// Private class functions: Procedure entry/exit instrumentation

// ---- Declarations in main program ----

AST_ChangeFlags
PabloOldTraceCaptureLib::ProcDeclAtPgmEntry(AST_INDEX declPoint)
{
    Assert(curProcInfo->is_main_program());
    
    //----- Get tags for external Fortran routines from aux file ----
    // Tag entries are inserted in procTagTable
    getTagsFromAuxFile(auxTagListFileName);

    if (procTagTable->NumberOfTags() == 0) { 	  // i.e., no internal *or*
	assert(EntryExitInstrumented() == false); // external tags
	return INSTR_INSERTED_NONE;
    }

    //---- Insert declarations for all procedure instrumentation variables ----
    return procTagTable->insertAllTagDeclarations(declPoint);
}

// ---- Initialization in main program ----
// Generate:
//    data ProcENT/.../ and data ProcEXT/.../
//    call preinitproctrace()
//    call initproctrace(PROCCOUNT, ProcENT, ProcEXT)

AST_ChangeFlags
PabloOldTraceCaptureLib::ProcInitAtPgmEntry(AST_INDEX initPoint)
{
    Assert(curProcInfo->is_main_program());
    if (procTagTable->NumberOfTags() == 0) {
	assert(EntryExitInstrumented() == false);
	return INSTR_INSERTED_NONE;
    }
    
    AST_ChangeFlags rc = INSTR_INSERTED_NONE;
    rc |= procTagTable->insertTagArrayDataStatements(initPoint);
    
    (void) list_insert_before(initPoint,
                 pt_gen_call(systemInfo->ProcTracePreInitCall(), AST_NIL));    
    AST_INDEX
    node =list_create(pt_gen_ident(procTagTable->GetCountVarName()));
    node =list_insert_last(node,pt_gen_ident(procTagTable->GetEntryVarName()));
    node =list_insert_last(node, pt_gen_ident(procTagTable->GetExitVarName()));
    (void) list_insert_before(initPoint,
			  pt_gen_call(systemInfo->ProcTraceInitCall(), node));
    rc |= INSTR_INSERTED_BEFORE;
    
    return rc;
}
			       
// Declarations in current procedure:
//		common /<proc_name/ <proc_entry_tag>, <proc_exit_tag>
//		integer <proc_entry_tag>, <proc_exit_tag>
// ASSUMES: lib_instr_proc_entry has already been called (see Assertion).

AST_ChangeFlags
PabloOldTraceCaptureLib::ProcDeclAtProcEntry(AST_INDEX declPoint)
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
PabloOldTraceCaptureLib::getTagsFromAuxFile(char *aux_tag_list_file_name)
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
PabloOldTraceCaptureLib::LoopDeclAtPgmEntry(AST_INDEX declPoint)
{
    Assert(curProcInfo->is_main_program());
    if (! LoopsInstrumented()) {
	assert(loopTagTable->NumberOfTags() == 0);
	return INSTR_INSERTED_NONE;
    }
    else
	return loopTagTable->insertAllTagDeclarations(declPoint);
}

// ---- Initialization in main program ----
// Generate: call FDinitLoopTrace(LOOPCOUNT, LoopENT, LoopEXT)

AST_ChangeFlags
PabloOldTraceCaptureLib::LoopInitAtPgmEntry(AST_INDEX initPoint)
{
    Assert(curProcInfo->is_main_program());
    if (! LoopsInstrumented()) {
	assert(loopTagTable->NumberOfTags() == 0);
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
PabloOldTraceCaptureLib::LoopDeclAtProcEntry(AST_INDEX declPoint)
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
// Instrumentation version returns a value only if original msg routine did  
// replace      <msg-routine> ( <args> )
// with    trace<msg-routine> ( <args> )

AST_ChangeFlags
PabloOldTraceCaptureLib::ReplaceMesgCall(AST_INDEX invocation,
				      char *invocation_name)
{
    // ---- First replace the name of the call <name> with trace<name>
    char new_name[MAX_NAME];
    strcpy(new_name, "trace");
    strcat(new_name, invocation_name);
    gen_INVOCATION_put_name(invocation, pt_gen_ident(new_name));
    systemInfo->recordInvocation(invocation_name);
    return INSTR_INSERTED_NONE;		// Since only a son of the
}					// ASTnode stmt was replaced.

AST_ChangeFlags
PabloOldTraceCaptureLib::MesgDeclAtProcEntry(AST_INDEX declPoint)
{
    if (! MesgsInstrumentedInCurProc()) return INSTR_INSERTED_NONE;
    char new_name[MAX_NAME];
    for (int i = 0; i < systemInfo->NumMesgCallsToTrace(); i++) {
	if (systemInfo->NumInvocationsInCurProc(i) > 0
            && (   strcasecmp(systemInfo->MessageName(i), "isend") == 0
                || strcasecmp(systemInfo->MessageName(i), "irecv") == 0))
	{
	    strcpy(new_name, "trace");
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
PabloOldTraceCaptureLib::MesgInitAtPgmEntry(AST_INDEX initPoint)
{
    Assert(curProcInfo->is_main_program());
    if (! MesgsInstrumented()) return INSTR_INSERTED_NONE;
    (void)list_insert_before(initPoint,
                  pt_gen_call(systemInfo->MesgTracePreInitCall(), AST_NIL));
    (void)list_insert_before(initPoint,
		  pt_gen_call(systemInfo->MesgTraceInitCall(), AST_NIL));
    return INSTR_INSERTED_BEFORE;
}

//-----------------------------------------------------------------------
// Private class functions: interface to basic traceevent routine

// Generate: call traceevent(eventTagVarName, dataPtr, dataLen)

AST_ChangeFlags
PabloOldTraceCaptureLib::insertTraceevent(AST_INDEX stmt, char *eventTagName,
					  int arg1, int arg2)
{
    AST_INDEX node = list_create(pt_gen_ident(eventTagName));
    node = list_insert_last(node, list_create(pt_gen_int(arg1)));
    node = list_insert_last(node, list_create(pt_gen_int(arg2)));
    node = pt_gen_call("traceevent", node);
    (void) list_insert_before(stmt, node); 
    return INSTR_INSERTED_BEFORE;
}


//**********************************************************************
// Member functions for class PabloOldLibSystemInfo for Pablo
//**********************************************************************

PabloOldLibSystemInfo::PabloOldLibSystemInfo(Fd_Arch_type arch)
: SystemInfo(arch)
{
    if (arch != FD_NX) {
	fputs("ERROR: Only NX (iPSC) mesg-passing instr. supported", stderr);
	Assert(false);
    }
    procTracePreInitCall = ::OLD_IPSC_PROC_TRACE_PRE_INIT_CALL;
    procTraceInitCall    = ::OLD_IPSC_PROC_TRACE_INIT_CALL;
    loopTracePreInitCall = ::OLD_IPSC_LOOP_TRACE_PRE_INIT_CALL;
    loopTraceInitCall    = ::OLD_IPSC_LOOP_TRACE_INIT_CALL;
    mesgTracePreInitCall = ::OLD_IPSC_MESG_TRACE_PRE_INIT_CALL;
    mesgTraceInitCall    = ::OLD_IPSC_MESG_TRACE_INIT_CALL;
}


//**********************************************************************
// Global (static) helper functions.
//**********************************************************************


//**********************************************************************
