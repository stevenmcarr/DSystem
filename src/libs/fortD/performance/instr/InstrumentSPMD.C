/* $Id: InstrumentSPMD.C,v 1.13 1997/03/11 14:28:50 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// $Id: InstrumentSPMD.C,v 1.13 1997/03/11 14:28:50 carr Exp $
//**********************************************************************
// Routines for instrumenting SPMD code put out by the compiler.
//**********************************************************************

//-------------------------------------------------------------------------
// INCLUDES
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef groups_h			// From the Fortran D world
#include <libs/frontEnd/ast/groups.h>
#endif
#ifndef AstIterators_h
#include <libs/frontEnd/ast/AstIterators.h>
#endif
#ifndef FD_CODEGEN
#include <libs/fortD/codeGen/private_dc.h>
#endif

#ifndef general_h
#include <libs/support/misc/general.h>
#endif					//   not already in private_dc.h
#ifndef gen_h
#include <libs/frontEnd/ast/gen.h>
#endif
#ifndef gi_h
#include <libs/frontEnd/include/gi.h>
#endif
#ifndef strutil_h
#include <libs/frontEnd/ast/strutil.h>
#endif
					// From the performance tool world
#include <libs/fortD/performance/instr/InstrumentSPMD.h>
#include <libs/fortD/performance/instr/PabloLib.h>
#include <libs/fortD/performance/instr/OldPabloLib.h>


//-------------------------------------------------------------------------
// GLOBAL (EXTERN) DECLARATIONS
//

//-------------------------------------------------------------------------
// PRIVATE DECLARATIONS
//

//---- Symbolic constants; Macros ----

//-------------------------------------------------------------------------
// Declarations of functions private to this file
//

STATIC(void, myInstrLoops,		(Dist_Globals* dh,
					 SPMDInstrumentation* instr));
// ----COPIED----
STATIC(AST_INDEX, findLogicalIF,	(AST_INDEX node));
STATIC(AST_INDEX, convertLogicalIF,	(AST_INDEX node));
STATIC(void,      tree_print_node,	(AST_INDEX node, Generic level,
					 Boolean sides_p, FILE *fp));


//**********************************************************************
// Member functions for class SPMDInstrumentation 
//**********************************************************************

// Constructor and Destructor

SPMDInstrumentation::SPMDInstrumentation(Fd_opts* fd_opts)
{
    instrOpts = new InstrumentationOptions(fd_opts);
    if (! instrOpts->InstrRequested())
	return;				// instrOpts still needed till the end
    
    // Else Instrumentation is requested.
    // Verify that it is possible for this architecture and continue.
    
    if (instrOpts->InstrArch() != FD_NX) { // The only one supported by Pablo
	fputs("\n*** FATAL ERROR: Instrumentation is supported only for the\n"
	      "***\t\t NX/2 message passing library on Intel iPSC machines.\n"
	      "*** EXITING...\n\n", stderr);
	exit(1);
    }

#ifdef INSTR_USE_OLD_PABLO_LIB
    iLib = (InstrumentationLibrary *) new PabloOldTraceCaptureLib;
#else
    iLib = (InstrumentationLibrary *) new PabloTraceCaptureLib;
#endif
    systemInfo  = (SystemInfo*) iLib->SetupSystemInfo(instrOpts->InstrArch());
    
    curProcInfo = (CurrentProcInstrInfo *) NULL; // setup for each program unit
    
    strcpy(startTimeVarName, "pgmStart");
    strcpy(endTimeVarName, "pgmEnd");
}

SPMDInstrumentation::~SPMDInstrumentation()
{
    delete instrOpts;
    if (InstrumentationOn()) {
	systemInfo = iLib->DeleteSystemInfo();
	delete iLib;
    }
}

Boolean	SPMDInstrumentation::InstrumentationOn()
{
    return (instrOpts->InstrRequested());
}

// Instrument a single message call
// "invocation" must contain the AST_INDEX of the function call.
// Does *NOT* check to ensure that the specified call is indeed
// a message call.  (This is more general, but assumes that the
// client knows what its doing when calling this function.)
// Instead, assume it is a message call and invoke the low-level
// instrumentation function on it.

AST_ChangeFlags SPMDInstrumentation::instrumentMesgCall
	(Dist_Globals *dh, AST_INDEX invocation, AST_INDEX stmt, int staticId)
{
    doSetupIfRequired(dh);
    return iLib->InstrMesgCall(invocation, stmt, staticId);
}

// Instrument procedure entry and exit.

AST_ChangeFlags SPMDInstrumentation::instrumentProcedureEntryExit
	(Dist_Globals *dh, AST_INDEX header, int enterId, int exitId)
{
    doSetupIfRequired(dh);

    AST_INDEX stmt;
    AST_ChangeFlags rc = INSTR_INSERTED_NONE;
    AstIterator tree_walk(header, PreOrder, AST_ITER_STMTS_ONLY);

    while ((stmt = tree_walk.Current()) != AST_NIL) {
	tree_walk++;		// Advance the walk *before* inserting
				// instrumentation statements
	if (curProcInfo->is_first_stmt(stmt) || is_entry(stmt)) {
	    if (is_entry(stmt)) {
		AST_INDEX instrBefore = curProcInfo->placeHolderAfter(stmt);
		if (iLib->InstrProcEntry(instrBefore, enterId)
		    				== INSTR_INSERTED_AFTER)
		    rc |= INSTR_INSERTED_AFTER;	// Ignore other return values
		curProcInfo->deletePlaceHolder(instrBefore);
	    }
	    else {
		AST_INDEX instrBefore = stmt;
		rc |= iLib->InstrProcEntry(instrBefore, enterId);
	    }
	}
	if (curProcInfo->is_program_exit_point(stmt) ||
	    curProcInfo->is_subprogram_exit_point(stmt))
	{
	    if (! curProcInfo->is_program_unit_exit_stmt(stmt)) {
		AST_INDEX instrBefore = curProcInfo->placeHolderAfter(stmt);
		if (iLib->InstrProcExit(instrBefore, exitId)
		    				== INSTR_INSERTED_AFTER)
		    rc |= INSTR_INSERTED_AFTER;	// Ignore other return values
		curProcInfo->deletePlaceHolder(instrBefore);
	    }
	    else {
		AST_INDEX instrBefore = stmt;
		rc |= iLib->InstrProcExit(instrBefore, exitId);
	    }
	}
    }
    return rc;
}

AST_ChangeFlags SPMDInstrumentation::measureLoopTime
	(Dist_Globals* dh, AST_INDEX header, int enterId, int exitId)
{
    doSetupIfRequired(dh);
    if (! is_do(header)) return INSTR_INSERTED_NONE;
    
    AST_ChangeFlags rc = INSTR_INSERTED_NONE;
    
    // If the do statement has a label, add a CONTINUE statement before 
    // it and move the label to the continue.
    if (curProcInfo->checkAndMoveLabel(header))
	rc |= INSTR_INSERTED_BEFORE;
    
    rc |= iLib->InstrLoopEntry(header, enterId);
    
    // Insert instrumentation immediately after the loop
    Assert (in_list(header));
    AST_INDEX nextStmt = list_next(header);
    if (nextStmt == AST_NIL) {
	nextStmt = curProcInfo->placeHolderAfter(header);
	if (iLib->InstrLoopExit(nextStmt, enterId) == INSTR_INSERTED_AFTER)
	    rc |= INSTR_INSERTED_AFTER;	// Ignore other return values
	curProcInfo->deletePlaceHolder(nextStmt);
    }
    else
	rc |= iLib->InstrLoopExit(nextStmt, exitId);
    
    // Check for other exit points from the loop and
    // insert instrumentation immediately before each such point
    AstIterator tree_walk(header, PreOrder, AST_ITER_STMTS_ONLY);
    while ((nextStmt = tree_walk.Current()) != AST_NIL) {
	tree_walk++;	// Advance the walk *before* inserting instrumentation 
	if (curProcInfo->is_loop_exit_stmt(nextStmt)) {
	    rc |= iLib->InstrLoopExit(nextStmt, exitId);
	}
    }
    
    return rc;
}
    
// Measure total execution time of the program.
// This routine is the only one that does not call on the underlying
// instrumentation library.
// Instead, it directly insert calls to system timing routines to measure
// the start-end interval on each processor and prints these out directly
// to stdout at the end of the program.

AST_ChangeFlags SPMDInstrumentation::measureProgramTime(Dist_Globals* dh,
							AST_INDEX header)
{
    doSetupIfRequired(dh);
    if (! curProcInfo->is_main_program())
	return INSTR_INSERTED_NONE;
    
    AST_INDEX stmt;
    AST_ChangeFlags rc = INSTR_INSERTED_NONE;
    AstIterator tree_walk(header, PreOrder, AST_ITER_STMTS_ONLY);

    while ((stmt = tree_walk.Current()) != AST_NIL) {
	tree_walk++;		// Advance the walk *before* inserting
				// instrumentation statements
	if (curProcInfo->is_first_stmt(stmt) || is_entry(stmt)) {
	    if (is_entry(stmt)) {
		AST_INDEX instrBefore = curProcInfo->placeHolderAfter(stmt);
		if (InstrPgmEntry(dh, instrBefore) == INSTR_INSERTED_AFTER)
		    rc |= INSTR_INSERTED_AFTER;	// Ignore other return values
		curProcInfo->deletePlaceHolder(instrBefore);
	    }
	    else {
		AST_INDEX instrBefore = stmt;
		rc |= InstrPgmEntry(dh, instrBefore);
	    }
	}
	if (curProcInfo->is_program_exit_point(stmt)) {
	    if (! curProcInfo->is_program_unit_exit_stmt(stmt)) {
		AST_INDEX instrBefore = curProcInfo->placeHolderAfter(stmt);
		if (InstrPgmExit(dh, instrBefore) == INSTR_INSERTED_AFTER)
		    rc |= INSTR_INSERTED_AFTER;	// Ignore other return values
		curProcInfo->deletePlaceHolder(instrBefore);
	    }
	    else {
		AST_INDEX instrBefore = stmt;
		rc |= InstrPgmExit(dh, instrBefore);
	    }
	}
    }
    return rc;
}

AST_ChangeFlags SPMDInstrumentation::PrintSymbolic(Dist_Globals *dh,
						   AST_INDEX	intExpr,
						   AST_INDEX	stmt,
						   StaticInfo	static_info,
						   Boolean	addGuard,
						   int		initialValue)
{
    doSetupIfRequired(dh);
    return iLib->PrintSymbolic(intExpr,stmt,static_info,addGuard,initialValue);
}

AST_ChangeFlags SPMDInstrumentation::measureIntervalTime
	(Dist_Globals* /* dh */,
	 AST_INDEX /* start_stmt */,
	 AST_INDEX /* end_stmt */,
	 int /* enterId */,
	 int /* exitId */)
{
    return INSTR_INSERTED_NONE;
}

AST_ChangeFlags SPMDInstrumentation::wrapupProgramUnit(Dist_Globals *dh)
{
    doSetupIfRequired(dh);

    //------------------------------------------------------------//
    // call this here. later move it to SDDF_SetupCleanup.C       //
    if (GetInstrOpts()->InstrLoops())
	myInstrLoops(dh, this);
    //------------------------------------------------------------//

    AST_INDEX declPoint = curProcInfo->get_first_stmt_after_decl();

    AST_INDEX stmt;
    AST_ChangeFlags rc = INSTR_INSERTED_NONE;
    AstIterator tree_walk(dh->root, PreOrder, AST_ITER_STMTS_ONLY);

    if (curProcInfo->is_main_program()) {
	AST_INDEX initPoint = curProcInfo->get_orig_first_stmt();
	rc |= iLib->InitAtPgmEntry(declPoint,initPoint);
	rc |= InitPgmTiming(declPoint,initPoint);

	while ((stmt = tree_walk.Current()) != AST_NIL) {
	    tree_walk++;	// Advance the walk *before* inserting
				// instrumentation statements
	    if (curProcInfo->is_program_exit_point(stmt)) {
		AST_INDEX instrBefore =
		    (curProcInfo->is_program_unit_exit_stmt(stmt))?
			stmt : curProcInfo->placeHolderAfter(stmt);
		rc |= iLib->WrapupAtPgmExit(instrBefore);
		if (! curProcInfo->is_program_unit_exit_stmt(stmt))
		    curProcInfo->deletePlaceHolder(instrBefore);
	    }
	}
    }
    else {
	while ((stmt = tree_walk.Current()) != AST_NIL) {
	    tree_walk++;	// Advance the walk *before* inserting
				// instrumentation statements
	    if (curProcInfo->is_first_stmt(stmt) || is_entry(stmt)) {
		AST_INDEX instrBefore = (is_entry(stmt))?
		    curProcInfo->placeHolderAfter(stmt) : stmt;
		rc |= iLib->InitAtProcEntry(declPoint, instrBefore);
		if (is_entry(stmt))
		    curProcInfo->deletePlaceHolder(instrBefore);
	    }
	    if (curProcInfo->is_subprogram_exit_point(stmt)) {
		AST_INDEX instrBefore =
		    (curProcInfo->is_program_unit_exit_stmt(stmt))?
			stmt : curProcInfo->placeHolderAfter(stmt);
		rc |= iLib->WrapupAtProcExit(instrBefore);
		if (! curProcInfo->is_program_unit_exit_stmt(stmt))
		    curProcInfo->deletePlaceHolder(instrBefore);
	    }
	}
    }
    curProcInfo = iLib->DeleteCurProcInfo();
    return rc;
}

//-----------------------------------------------------------------------
// Private member functions for Class SPMDInstrumentation

void SPMDInstrumentation::doSetupIfRequired(Dist_Globals *dh)
{
    Assert(InstrumentationOn());
    if (curProcInfo == (CurrentProcInstrInfo *) NULL)
	curProcInfo = iLib->SetupCurProcInfo(dh);
}

AST_ChangeFlags SPMDInstrumentation::InstrPgmEntry(Dist_Globals* /* dh */,
						   AST_INDEX stmt)
{
    assert (curProcInfo->is_main_program());
    
    AST_INDEX callAST, callArgs, callStmt;
    
    // Generate call to timer to read value at program entry
    callStmt = GetSystemInfo()->Timer(startTimeVarName);
    assert (callStmt != AST_NIL);
    list_insert_before(stmt, callStmt);
    return INSTR_INSERTED_BEFORE;
}

AST_ChangeFlags SPMDInstrumentation::InstrPgmExit(Dist_Globals* dh,
						  AST_INDEX stmt)
{
    assert (curProcInfo->is_main_program());
    
    AST_INDEX callAST, callArgs, invoc, callStmt;
    
    // First, generate call to timer to read value at program exit
    callStmt = GetSystemInfo()->Timer(endTimeVarName);
    assert(callStmt != AST_NIL);
    (void) list_insert_before(stmt, callStmt);
    
    // Generate stmt to print out start-end value
    callStmt = GetSystemInfo()->
	TimerPrint(gen_get_text(ast_get_logical_myproc(dh)),
		   curProcInfo->get_proc_name(),
		   startTimeVarName, endTimeVarName);
    (void) list_insert_before(stmt, callStmt);
    
    return INSTR_INSERTED_BEFORE;
}

AST_ChangeFlags SPMDInstrumentation::InitPgmTiming(AST_INDEX declPoint,
						   AST_INDEX initPoint)
{
    assert (curProcInfo->is_main_program());
    
    AST_INDEX callAST, callArgs, callStmt;

    // First generate call to initialize the timer
    if ((callStmt = GetSystemInfo()->InitTimerCall()) != AST_NIL)
	list_insert_before(initPoint, callStmt);

    // Then, declare the start and end program timing variables
    // Assume they are of type double precision.
    AST_INDEX typeAST, nameList, typeStmt;
    typeAST = gen_DOUBLE_PRECISION();
    typeAST = gen_TYPE_LEN(typeAST, AST_NIL);
    nameList = gen_ARRAY_DECL_LEN(pt_gen_ident(startTimeVarName),
				  AST_NIL, AST_NIL, AST_NIL);
    nameList = list_create(nameList);
    nameList = list_insert_last(nameList,
	    gen_ARRAY_DECL_LEN(pt_gen_ident(endTimeVarName),
			       AST_NIL, AST_NIL, AST_NIL));
    typeStmt = gen_TYPE_STATEMENT(AST_NIL, typeAST, nameList);
    ft_SetComma(typeStmt, false);
    list_insert_before(declPoint, typeStmt);

    return INSTR_INSERTED_BEFORE;
}


//**********************************************************************
// Member functions for class CurrentProcInstrInfo

//-----------------------------------------------------------------------
// Constructor and Destructor

CurrentProcInstrInfo::CurrentProcInstrInfo(Dist_Globals *dh)
{
    repr.dh    = dh;
    repr.root  = dh->root;
    repr.numPh = 0;

    AST_INDEX stmt;
    AstIterator tree_walk(repr.root, PreOrder, AST_ITER_STMTS_ONLY);

    for (; stmt = tree_walk.Current(); tree_walk++)
	if (find_name_and_type(stmt))	// Must get type and name first
	    break;			// before doing anything else.
    for (tree_walk.Reset();
	 (stmt = tree_walk.Current()) && !executable_stmt(stmt);
	 tree_walk++)
	;
    if (stmt == AST_NIL) {
	Assert(! is_main_program());	// Cannot have empty main program
	return;				// Empty subroutine or function ok
    }
    mark_first_executable_stmt(stmt);	// Puts marker before comments

    AST_INDEX new_node;
    for (tree_walk.Reset(); stmt = tree_walk.Current(); tree_walk++)
	if ((new_node = findLogicalIF(stmt)) != AST_NIL)
	    tree_walk.ReplaceCurrent(new_node);
}

CurrentProcInstrInfo::~CurrentProcInstrInfo()
{
    deletePlaceHolder(AST_NIL, /* deleteAll = */ true);
}

//-----------------------------------------------------------------------

AST_INDEX CurrentProcInstrInfo::placeHolderAfter(AST_INDEX stmt)
{
    Assert(in_list(stmt));		// Else cannot insert place-holder
    Assert(repr.numPh < repr.MAX_PLACE_HOLDERS - 1); // Else phList full
    AST_INDEX endPh = gen_PLACE_HOLDER();
    list_insert_after(stmt, endPh);
    repr.phList[repr.numPh++] = endPh;	// Remember place-holders so
    return endPh;			//   they can be deleted later
}

void CurrentProcInstrInfo::deletePlaceHolder(AST_INDEX ph, Boolean deleteAll)
{
    if (deleteAll) {				// remove all place-holders
	for (int i=0; i < repr.numPh; i++) {
	    list_remove_node(repr.phList[i]);
	    ast_free(repr.phList[i]);
	}
	repr.numPh = 0;
    }
    else {
	for (int i=0; i < repr.numPh; i++)
	    if (repr.phList[i] == ph) {		// valid place-holder
		list_remove_node(ph);		// remove place-holder
		ast_free(ph);
		if (repr.numPh > 1)
		    repr.phList[i] = repr.phList[repr.numPh - 1];
		--repr.numPh;
		return;
	    }
	Assert(false);			// Not a place-holder inserted by us
    }
}

// If the given statement has a label, add a CONTINUE statement before 
// it and move the label to the CONTINUE. Guarantees control flow must
// come in via the CONTINUE statement.
Boolean
CurrentProcInstrInfo::checkAndMoveLabel(AST_INDEX stmt)
{
    AST_INDEX label;
    if (! is_statement(stmt) || (label = gen_get_label(stmt)) == AST_NIL)
	return false;
    AST_INDEX continueStmt = gen_CONTINUE(tree_copy(label));
    gen_put_label(stmt, AST_NIL);
    tree_free(label);
    list_insert_before(stmt, continueStmt);
    return true;
}

//-----------------------------------------------------------------------

void CurrentProcInstrInfo::mark_first_executable_stmt(AST_INDEX stmt)
{
    repr.first_executable_stmt = stmt;

    // Then back up above comments and insert place-holder instrPh there
    // Procedure entry instrumentation should go before instrPh
    AST_INDEX node, prev;
    for (node = stmt, prev = list_prev(node);
	 (prev != AST_NIL) && is_comment(prev);
	 node = prev, prev = list_prev(node))
	;
    repr.instrPh = placeHolderAfter(node);

    // Back up to last type stmt and mark the stmt after it
    // Declarations for instr. will go before the mark
    for (prev = list_prev(node);
	 (prev != AST_NIL) && !is_type_statement(prev) && !is_common(prev);
	 node = prev, prev = list_prev(node))
	;
    add_blank_line_before(node);
    repr.first_stmt_after_decl = list_prev(node); // i.e., second blank line

    if (is_main_program())
	(void) list_insert_before(repr.first_stmt_after_decl,
	      pt_gen_comment("--<< Declarations for instrumentation. >>--"));
}

//-----------------------------------------------------------------------

char *CurrentProcInstrInfo::get_proc_name()
{
    return repr.program_unit_name;
}

Dist_Globals *CurrentProcInstrInfo::get_dh_handle()
{
    return repr.dh;
}

Boolean CurrentProcInstrInfo::is_main_program()
{
    return BOOL(repr.program_unit_type == INSTR_MAIN_PROGRAM);
}

Boolean CurrentProcInstrInfo::is_first_stmt(AST_INDEX stmt)
{
    return BOOL(stmt == repr.first_executable_stmt);
}

Boolean CurrentProcInstrInfo::is_program_exit_point(AST_INDEX stmt)
{
    return BOOL(is_main_program()
		&& (is_last_stmt_in_program_unit(stmt)
		    || is_program_unit_exit_stmt(stmt)));
}

Boolean CurrentProcInstrInfo::is_subprogram_exit_point(AST_INDEX stmt)
{
    return BOOL(! is_main_program()
		&& (is_last_stmt_in_program_unit(stmt)
		    || is_program_unit_exit_stmt(stmt)));
}

AST_INDEX CurrentProcInstrInfo::get_orig_first_stmt()
{
    return repr.instrPh;
}

AST_INDEX CurrentProcInstrInfo::get_first_stmt_after_decl()
{
    return repr.first_stmt_after_decl;
}
//-----------------------------------------------------------------------

//
// find_name_and_type()	Treewalk callback function for init_instr_globals()
//		    	Finds the name and type of the current program unit
//
// Copied from dc.init.c and modified to also get the program unit type.
//

Boolean CurrentProcInstrInfo::find_name_and_type(AST_INDEX stmt)
{
    if (is_program(stmt)) {
	stmt = gen_PROGRAM_get_name(stmt);
	repr.program_unit_type = INSTR_MAIN_PROGRAM;
    }
    else if (is_subroutine(stmt)) {
	stmt = gen_SUBROUTINE_get_name(stmt);
	repr.program_unit_type = INSTR_SUBROUTINE;
    }
    else if (is_function(stmt)) {
	stmt = gen_FUNCTION_get_name(stmt);
	repr.program_unit_type = INSTR_FUNCTION;
    }
    else
	return false;

    strcpy(repr.program_unit_name, gen_get_text(stmt));
    return true;
}

//------------------------------------------------------------------------ 
// Check if specified stmt is last executable stmt in the current program
// unit, i.e. after which control will exit the program unit
// LOGIC: TRUE if:
// 		stmt is the last *executable* stmt in a list
//	   AND  the list's parent is a function, subroutine or program stmt

Boolean CurrentProcInstrInfo::is_last_stmt_in_program_unit(AST_INDEX stmt)
{
    if (stmt == AST_NIL || !in_list(stmt) || !executable_stmt(stmt))
	return false;
    Boolean last_executable_in_list = true;
    for (AST_INDEX s = list_next(stmt); s != AST_NIL; s = list_next(s)) {
	if (executable_stmt(s)) { last_executable_in_list = false; break; }
    }
    AST_INDEX node = tree_out(stmt);
    return BOOL(last_executable_in_list &&
		(is_function(node) ||is_subroutine(node) ||is_program(node)));
}

//------------------------------------------------------------------------ 
// Check if specified stmt causes flow of control to leave this program unit
// (i.e., return/stop stmt), in which case instr. must go *before* the stmt

Boolean CurrentProcInstrInfo::is_program_unit_exit_stmt(AST_INDEX stmt)
{
    return BOOL(stmt != AST_NIL && (is_return(stmt) || is_stop(stmt)));
}

// Check if specified stmt causes flow of control to leave enclosing loop
Boolean CurrentProcInstrInfo::is_loop_exit_stmt(AST_INDEX stmt)
{
    return BOOL(stmt != AST_NIL
		&& (is_program_unit_exit_stmt(stmt) || is_goto(stmt)));
}

//**********************************************************************
// Member functions for class InstrumentationLibrary:
// See the interface definition file.
//**********************************************************************



//**********************************************************************
// Member functions for class SystemInfo
//**********************************************************************

SystemInfo::SystemInfo(Fd_Arch_type arch)
{
    Assert(arch == FD_NX);		// Only NX/2 supported by Pablo
    archType = arch;
    numMesgCallsToTrace = ::IPSC_NUM_MESG_CALLS_TO_TRACE;
    mesgCallList = ::IPSC_MESG_CALL_LIST;
    numInvocations = new int[numMesgCallsToTrace];
    numInvocationsInCurrentProc = new int[numMesgCallsToTrace];
    for (int i = 0; i < numMesgCallsToTrace; i++)
	numInvocations[i] = numInvocationsInCurrentProc[i] = 0;
}
SystemInfo::~SystemInfo()
{
    delete[] numInvocations;
    delete[] numInvocationsInCurrentProc;
}   

Boolean SystemInfo::InstrumentableMesgCall(char* msgCallName)
{
    for (int i=0; i < numMesgCallsToTrace; i++)
        if (::strcasecmp(msgCallName, mesgCallList[i]) ==0)
            return true;
    return false;
}

int SystemInfo::NumMesgCallsToTrace() { return numMesgCallsToTrace; }

int SystemInfo::MessageNumber(char *msgCallName)
{
    for (int i=0; i < numMesgCallsToTrace; i++)
        if (::strcasecmp(msgCallName, mesgCallList[i]) ==0)
            return i;
    return -1;
}

char* SystemInfo::MessageName(int msgCallNum)
{
    Assert(msgCallNum >= 0 && msgCallNum < numMesgCallsToTrace);
    return (char *) mesgCallList[msgCallNum]; // cast to override const-ness
}

void SystemInfo::recordInvocation(char *msgCallName)
{
    int msgNum = MessageNumber(msgCallName);
    ++numInvocations[msgNum];
    ++numInvocationsInCurrentProc[msgNum];
}

int SystemInfo::NumInvocations(int msgCallNum)
{
    Assert(msgCallNum >= 0 && msgCallNum < numMesgCallsToTrace);
    return numInvocations[msgCallNum];
}

int SystemInfo::NumInvocationsInCurProc(int msgCallNum)
{
    Assert(msgCallNum >= 0 && msgCallNum < numMesgCallsToTrace);
    return numInvocationsInCurrentProc[msgCallNum];
}

void SystemInfo::ResetInvocationsInCurProc(int msgCallNum)
{
    Assert(msgCallNum >= 0 && msgCallNum < numMesgCallsToTrace);
    numInvocationsInCurrentProc[msgCallNum] = 0;
}

AST_INDEX SystemInfo::InitTimerCall()
{ assert(archType == FD_NX); return AST_NIL; }

AST_INDEX SystemInfo::Timer(char* timerVarName)
{
    assert(archType == FD_NX);
    char* timerName = "d_clock";		// NX only
    
    AST_INDEX callArgs = list_create(pt_gen_ident(timerVarName));
    AST_INDEX callStmt = pt_gen_call(timerName, callArgs);
    return callStmt;
}

// TimerPrint() :  Generate the call to runtime routine to print time:
// 		   printTimeInterval(my$p, 'mesg', startTime, endTime)

AST_INDEX SystemInfo::TimerPrint(char* myIndexVarName, char* printMesg,
				 char* startVarName, char* endVarName)
{
    assert(archType == FD_NX);
    char* timerPrintCall = "printTimeInterval";
    char printString[160];		// >= length of 2 Fortran lines
    AST_INDEX callArgs, constStr;
    
    callArgs = list_create(pt_gen_ident(myIndexVarName));
    
    // Actual string argument must be "'printMesg'", i.e., printMesg
    // needs additional ' ' around it in string constant:
    (void) strcpy(printString, "'");
    (void) strcat(printString, printMesg);
    (void) strcat(printString, "'");
    constStr = gen_CONSTANT();
    gen_put_text(constStr, printString, STR_CONSTANT_CHARACTER);

    callArgs = list_insert_last(callArgs, constStr);
    callArgs = list_insert_last(callArgs, pt_gen_ident(startVarName));
    callArgs = list_insert_last(callArgs, pt_gen_ident(endVarName));
    
    return pt_gen_call(timerPrintCall, callArgs);
}

//**********************************************************************
// Member functions for class InstrumentationOptions
//**********************************************************************

InstrumentationOptions::InstrumentationOptions(Fd_opts* fd_opts)
{
    UpdateOptions(fd_opts);
}
InstrumentationOptions::~InstrumentationOptions()
{
}

// For now, don't need any of the old state. Just initialize like
// the constructor
void InstrumentationOptions::UpdateOptions(Fd_opts* fd_opts)
{
    opts.instrPgmExecTime   = fd_opts->flags[Instr_exec_time];
    opts.instrProcedures    = fd_opts->flags[Instr_procedures];
    opts.instrMessages      = fd_opts->flags[Instr_messages];
    opts.instrLoops         = fd_opts->flags[Instr_loops];
    opts.instrFullSymbolics = fd_opts->flags[Instr_full_symb];
    opts.nprocs		    = fd_opts->nprocs;
    
    // choose target architecture, default is NX (Intel iPSC msg lib)
    if (!fd_opts->arch || !strcmp(fd_opts->arch, "nx"))
	opts.arch = FD_NX;
    else if (!strcmp(fd_opts->arch, "cmmd"))
	opts.arch = FD_CMMD;
    else
	Assert(false);
}


//**********************************************************************
// Other global (static) functions. Mostly minor helper functions.
//**********************************************************************

//-----------------------------------------------------------------------
// Generates AST for the stmt: integer int_name */

AST_INDEX
int_decl_stmt(char *int_name)
{
    AST_INDEX node, tnode, node_list;
    
    if (int_name == (char *) NULL)
	return AST_NIL;
    
    // typechecker requires scalars to be zero-dim arrays
    node = pt_gen_ident(int_name);
    node = gen_ARRAY_DECL_LEN(node, AST_NIL, AST_NIL, AST_NIL);
    node_list = list_create(node);
    
    tnode = gen_TYPE_LEN(gen_INTEGER(), AST_NIL);
    node = gen_TYPE_STATEMENT(AST_NIL, tnode, node_list);
    ft_SetComma(node, false);
    return node;
}

//-----------------------------------------------------------------------
// Generates AST for the stmt: PARAMETER(int_name = int_value) */

AST_INDEX
int_parameter_stmt(char *int_name, int int_value) 
{
    if (int_name == (char *) NULL) return AST_NIL;
    AST_INDEX
    node = gen_PARAM_ELT(pt_gen_ident(int_name), pt_gen_int(int_value));
    node = gen_PARAMETER(AST_NIL, list_create(node));
    return node;
}

//-----------------------------------------------------------------------
// add_blank_line_before()    Insert blank line before stmt if needed.
// Copied from dc.init.c because it is static there.
//

void
add_blank_line_before(AST_INDEX node)
{
    if (node == AST_NIL) return;
    AST_INDEX prev = list_prev(node);
    if (!is_comment(prev) || (gen_COMMENT_get_text(prev) != AST_NIL)) {
	(void) list_insert_before(node, pt_gen_comment("")); 
    }
    return;
}
//-----------------------------------------------------------------------

void
insert_commented_cpp_dir(AST_INDEX stmt, char *cpp_dir_text)
{
    char comment_text[80];
    strcpy(comment_text, "CPP$");
    strcat(comment_text, cpp_dir_text);
    (void) list_insert_before(stmt, pt_gen_comment(comment_text));
}
//-----------------------------------------------------------------------

//****************************************************
// COPIED FROM: /rn/rn2/src/libs/fort/asttree.ansi.c
// because these routines are private in that file.
//****************************************************

static void
indent(Generic i, FILE *fp) 
{
    while(i--) fprintf(fp, "  ");
}

static void
tree_print_node(AST_INDEX node, Generic level, Boolean sides_p, FILE *fp)
{
    STR_TEXT  text;
    STR_INDEX sym;
    Generic   type = ast_get_node_type(node);
    indent(level, fp);

    if (is_list(node)) {
	(void) fprintf(fp, "%s [%d] ", ast_get_node_type_name(type), node);
	(void) fprintf(fp, "\n");
	return;
    }
    else {
	(void) fprintf(fp, "%s [%d,%d] ", ast_get_node_type_name(type),
		       node, ast_get_meta_type(node));

	if (ast_get_real_type(node) != TYPE_UNKNOWN) 
	    (void) fprintf(fp, "R%s ",gen_type_get_text_short(ast_get_real_type(node)));
	if (ast_get_converted_type(node) != TYPE_UNKNOWN) 
	    (void) fprintf(fp, "C%s ",gen_type_get_text_short(ast_get_converted_type(node)));

        sym = ast_get_symbol(node);
        text = string_table_get_text(sym);

	if (text[0] != '\0') 
	    (void) fprintf(fp, "%d:\"%s\" ",sym,text);

	if (sides_p && (N(node)->Leafnode->side_array_ptr)) {
	    int i;
	    (void) fprintf(fp, "sides: ");
	    for (i = 0; i < asttab->stats.side_array_width; i++)
		if (asttab->stats.side_array_in_use[i])
		    (void) fprintf(fp, "%d ", N(node)->Leafnode->side_array_ptr[i]);
	}

	(void) fprintf(fp, "\n");
    }
}
//-----------------------------------------------------------------------

/*****************************************************
 * COPIED FROM: /rn/rn2/src/libs/ft_xforms/ftx_if.C
 * temporarily while all the infrastructure to use
 * the ftxforms routine is not available.
 *****************************************************/

/*
 * findLogicalIF checks to see if the current statement is a LOGICAL_IF
 * node, and if it is, this routine converts it to a block if.  
 * Returns the AST_INDEX of the new (block-if) ast node, for use by AstIterator
 *
 * NOTE: THIS ROUTINE MODIFIES THE TREE by its call to convertLogicalIF
 */

static AST_INDEX
findLogicalIF (AST_INDEX node)
{
    return ((is_logical_if(node))? convertLogicalIF(node) : AST_NIL);
}
/*---------------------------------------------------------------------*/

/*
 * convertLogicalIF converts logical IF statements to block IF statements.
 * NOTE: THIS ROUTINE MODIFIES THE TREE
 * specifically, it performs the following conversion:
 *
 * LOGICAL_IF				IF
 *  <comparison>		==>	 LIST_OF_NODES
 *   ...				  GUARD
 *  LIST_OF_NODES			   <comparison>
 *   ...				    ...
 *					   LIST_OF_NODES
 *					    ...
 *
 * it performs the translation by changing the node type of the LOGICAL_IF
 * to IF, creating the new LIST_OF_NODES and the GUARD, and inserting them
 * between the IF and the original LIST_OF_NODES.
 */
 

static AST_INDEX
convertLogicalIF ( AST_INDEX node )
{
    /* components of the logical if */
    AST_INDEX li_compare_node;	
    AST_INDEX li_stmt_list;	
    AST_INDEX li_lbl;	
    AST_INDEX new_node;	
  
    AST_INDEX bi_guard;	/* new GUARD node		    */
    AST_INDEX bi_list;	/* new LIST_OF_NODES node	    */
  
    /* allocate the LIST_OF_NODES for the new block if */
    bi_list = gen_LIST_OF_NODES();
    Assert(bi_list != AST_NIL);
  
    /* get fields out of old logical if */
    li_compare_node = gen_LOGICAL_IF_get_rvalue(node);
    li_stmt_list = gen_LOGICAL_IF_get_stmt_LIST(node);
    li_lbl = gen_LOGICAL_IF_get_lbl_def(node);
    
    /* stub them off so there are no complaints from check_tree_edges() */
    gen_LOGICAL_IF_put_rvalue(node, AST_NIL);
    gen_LOGICAL_IF_put_stmt_LIST(node, AST_NIL);
    gen_LOGICAL_IF_put_lbl_def(node, AST_NIL);
    
    /* for the block if: create GUARD node containing the comparison and 
     * the statement list;  the label field is empty */
    bi_guard = gen_GUARD(AST_NIL, li_compare_node, li_stmt_list);
    Assert(bi_guard != AST_NIL);
    
    /* insert the GUARD node into the LIST_OF_NODES for the block if */
    (void) list_insert_first(bi_list, bi_guard);
  
    /* now convert the LOGICAL_IF to a block IF */
    /* gen_coerce_node(node, GEN_IF); */

    new_node = gen_IF(li_lbl, AST_NIL, bi_list);
    
    /* gen_IF_put_lbl_def(new_node, li_lbl);
       gen_IF_put_close_lbl_def(new_node, AST_NIL);
       gen_IF_put_guard_LIST(node, bi_list);
       */
    tree_replace(node, new_node);
    tree_free(node);

    return new_node; // ADDED to use AstIterator instead of walk_statements
}
/*--------------------------------------------------------------------*/

// Get rid of this after SDDF records for loops are generated.
// Instruments only the outer-most loops in each procedure,
// just to test the loop instr. code

static void myInstrLoops(Dist_Globals* dh, SPMDInstrumentation* instr)
{
    AST_INDEX stmt;
    AstIterator tree_walk(dh->root, PreOrder, AST_ITER_STMTS_ONLY);
    
    //---- Walk tree to check and instrument outermost loops ----
    while ((stmt = tree_walk.Current()) != AST_NIL) {
	if (is_do(stmt)) {
	    AST_ChangeFlags rc = instr->measureLoopTime(dh, stmt, -1, -1);
	    if (rc & INSTR_INSERTED_REPLACED)
		tree_walk.ReplaceCurrent(instr->GetNewNode());
	    
	    // Ignore statements nested inside the do loop:
	    tree_walk.Advance(AST_ITER_SKIP_CHILDREN);
	}
	else tree_walk++;
    }
}    
