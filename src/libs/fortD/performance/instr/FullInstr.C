/* $Id: FullInstr.C,v 1.1 1997/03/11 14:28:49 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// $Id: FullInstr.C,v 1.1 1997/03/11 14:28:49 carr Exp $
//**********************************************************************
// 
//**********************************************************************

// INCLUDES
//
#include <stdio.h>
#include <string.h>

#ifndef groups_h			// From the Fortran D world
#include <libs/frontEnd/ast/groups.h>
#endif
#ifndef AstIterators_h
#include <libs/frontEnd/ast/AstIterators.h>
#endif
#ifndef fd_types_h
#include <libs/fortD/misc/fd_types.h>
#endif
#ifndef FD_CODEGEN
#include <libs/fortD/codeGen/private_dc.h>
#endif

#include <libs/fortD/performance/instr/InstrumentSPMD.h>

//-----------------------------------------------------------------------

// GLOBAL (EXTERN) DECLARATIONS

// PRIVATE DECLARATIONS

// LOCAL (STATIC) FUNCTIONS

// This EXTERN decl is here to prevent the name dc_instrument_spmd
// from being mangled, so that it can be called from a .c file
// That's also why this is under LOCAL functions.

EXTERN(void, dc_instrument_spmd, (Dist_Globals* dh,
				  Generic Instr,
				  Fd_opts* fd_opts));
STATIC(void, instrument_program_unit,	(Dist_Globals* dh,
					 SPMDInstrumentation* instr,
					 Fd_opts* fd_opts));
STATIC(AST_ChangeFlags, check_and_instrument_loop,
       					(Dist_Globals* dh,
					  AST_INDEX stmt,
					  SPMDInstrumentation* instr));
STATIC(AST_ChangeFlags, check_and_instrument_mesg_call,
       					(Dist_Globals* dh,
					  AST_INDEX stmt,
					  SPMDInstrumentation* instr));
STATIC(int, NextStaticId,		());

//---- Symbolic constants; Macros ----
#define NO_SUCH_STATIC_ID	(-1)


//********************** FUNCTION DEFINITIONS ***************************


// Entry point from compiler to instrumentation routines
void dc_instrument_spmd(Dist_Globals* dh, Generic instr, Fd_opts* fd_opts)
{
    instrument_program_unit(dh, (SPMDInstrumentation *) instr, fd_opts);
}
//-----------------------------------------------------------------------


void instrument_program_unit(Dist_Globals* dh,
			     SPMDInstrumentation* instr,
			     Fd_opts* /* fd_opts */)
{
    // Must test this separately, so that instrumentation can be turned
    // off without have to clear individual insrumentation flags.
    if (! instr->GetInstrOpts()->DoInstrumentation())
	return;

    if (instr->GetInstrOpts()->InstrProcedures())
	(void) instr->instrumentProcedureEntryExit(dh, dh->root,
			       NextStaticId(), NextStaticId());

    // Instruments only the outer-most loops in each procedure,
    // just to test the loop instr. code
    if (instr->GetInstrOpts()->InstrLoops()) {
	//---- Walk tree to check and instrument outermost loops ----
	AST_INDEX stmt;
	AstIterator tree_walk(dh->root, PreOrder, AST_ITER_STMTS_ONLY);
	while ((stmt = tree_walk.Current()) != AST_NIL) {
	    if (is_do(stmt)) {
		AST_ChangeFlags rc = instr->measureLoopTime(dh, stmt,
						NextStaticId(),NextStaticId());
		if (rc & INSTR_INSERTED_REPLACED)
		    tree_walk.ReplaceCurrent(instr->GetNewNode());

		tree_walk.Advance(AST_ITER_SKIP_CHILDREN);
	    }
	    else tree_walk++;
	}
    }
    
    if (instr->GetInstrOpts()->InstrMessages()) {
	//---- Walk tree to check and instrument message-passing stmts ----
	AST_INDEX stmt;
	AstIterator tree_walk(dh->root, PreOrder, AST_ITER_STMTS_ONLY);
	for ( ; (stmt = tree_walk.Current()) != AST_NIL; tree_walk++) {
	    AST_ChangeFlags rc = check_and_instrument_mesg_call(dh,stmt,instr);
	    if (rc & INSTR_INSERTED_REPLACED)
		tree_walk.ReplaceCurrent(instr->GetNewNode());
	}
    }

    if (instr->GetInstrOpts()->InstrProcedures()
	|| instr->GetInstrOpts()->InstrLoops()
	|| instr->GetInstrOpts()->InstrMessages())
	(void) instr->wrapupProgramUnit(dh);	// Ignore AST changes
}
//-----------------------------------------------------------------------

AST_ChangeFlags check_and_instrument_loop
	(Dist_Globals* dh, AST_INDEX stmt, SPMDInstrumentation* instr)
{
    if (is_do(stmt))
	return instr->measureLoopTime(dh, stmt, NextStaticId(),NextStaticId());
    else
	return INSTR_INSERTED_NONE;
}
//-----------------------------------------------------------------------

AST_ChangeFlags check_and_instrument_mesg_call
	(Dist_Globals* dh, AST_INDEX stmt, SPMDInstrumentation* instr)
{
    AST_INDEX invocation;

    if (is_call(stmt))
	invocation = gen_CALL_get_invocation(stmt);
    else if (is_assignment(stmt)) {
	invocation = gen_ASSIGNMENT_get_rvalue(stmt);
	if (! is_invocation(invocation))
	    return INSTR_INSERTED_NONE;
    }
    else
	return INSTR_INSERTED_NONE;

    // Now, <invocation> contains the AST_INDEX of the function call
    AST_INDEX invocation_name_ast = gen_INVOCATION_get_name(invocation);
    char* invocation_name =
	string_table_get_text(ast_get_symbol(invocation_name_ast));
    Assert(invocation_name != (char *) NULL);

    if ((instr->GetSystemInfo())->InstrumentableMesgCall(invocation_name))
	return instr->instrumentMesgCall(dh, invocation, stmt, NextStaticId());
					 
    return INSTR_INSERTED_NONE;
}
//-----------------------------------------------------------------------

int NextStaticId()
{
    static int currentId = 0;
    return ++currentId;
}
//**********************************************************************
