/* $Id: OldPabloLib.h,v 1.1 1997/03/11 14:28:52 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// $Id: OldPabloLib.h,v 1.1 1997/03/11 14:28:52 carr Exp $ -*-c++-*-
//**************************************************************************
// Public interface for class PabloOldTraceCaptureLib
//**************************************************************************

#ifndef OldPabloLib_h
#define OldPabloLib_h

#ifndef NameValueTable_h
#include <libs/support/tables/NameValueTable.h>
#endif

#include <libs/fortD/performance/instr/InstrumentSPMD.h>
#include <libs/fortD/performance/instr/PabloLib.h>

//-----------------------------------------------------------------------
// Enums and structures used for Pablo instrumentation


    
//-----------------------------------------------------------------------
// System-specific info for this instrumentation library.
// For Pablo, only need to customize a few library routine names.

static char* OLD_IPSC_PROC_TRACE_PRE_INIT_CALL = "preinitproctrace";
static char* OLD_IPSC_PROC_TRACE_INIT_CALL     = "initproctrace";
static char* OLD_IPSC_LOOP_TRACE_PRE_INIT_CALL = "preinitlooptrace";
static char* OLD_IPSC_LOOP_TRACE_INIT_CALL     = "initlooptrace";
static char* OLD_IPSC_MESG_TRACE_PRE_INIT_CALL = "preinitmesgtrace";
static char* OLD_IPSC_MESG_TRACE_INIT_CALL     = "ipsctraceinit";

class PabloOldLibSystemInfo : public SystemInfo
{
  public:
    PabloOldLibSystemInfo(Fd_Arch_type arch);
    inline char*  ProcTracePreInitCall();
    inline char*  ProcTraceInitCall();
    inline char*  LoopTracePreInitCall();
    inline char*  LoopTraceInitCall();
    inline char*  MesgTracePreInitCall();
    inline char*  MesgTraceInitCall();
  private:
    char* procTracePreInitCall;
    char* procTraceInitCall;
    char* loopTracePreInitCall;
    char* loopTraceInitCall;
    char* mesgTracePreInitCall;
    char* mesgTraceInitCall;
};

//----- Inline function definitions for class PabloOldLibSystemInfo ----
inline char*  PabloOldLibSystemInfo::ProcTracePreInitCall()
					{ return procTracePreInitCall; }
inline char*  PabloOldLibSystemInfo::ProcTraceInitCall()
					{ return procTraceInitCall; }
inline char*  PabloOldLibSystemInfo::LoopTracePreInitCall()
					{ return loopTracePreInitCall; }
inline char*  PabloOldLibSystemInfo::LoopTraceInitCall()
					{ return loopTraceInitCall; }
inline char*  PabloOldLibSystemInfo::MesgTracePreInitCall()
					{ return mesgTracePreInitCall; }
inline char*  PabloOldLibSystemInfo::MesgTraceInitCall()
					{ return mesgTraceInitCall; }
    
//-----------------------------------------------------------------------
// class PabloOldTraceCaptureLib : Interface to Pablo2.0 trace capture library
//
// This class is derived from base class InstrumentationLibrary
// which is a generic interface for any low level instrumentation library.
// This class implements the interface functions for the Pablo trace
// capture library.
//-----------------------------------------------------------------------

class PabloOldTraceCaptureLib : public InstrumentationLibrary
{
  public:
    PabloOldTraceCaptureLib();
    ~PabloOldTraceCaptureLib();

    SystemInfo*	    	  SetupSystemInfo(Fd_Arch_type arch);
    SystemInfo*		  DeleteSystemInfo();
    CurrentProcInstrInfo* SetupCurProcInfo(Dist_Globals *dh);
    CurrentProcInstrInfo* DeleteCurProcInfo();
    
    // ---- Init and wrapup routines specialized for Pablo ----
    AST_ChangeFlags	InitAtPgmEntry	(AST_INDEX declPoint,
					 AST_INDEX initPoint); 
    AST_ChangeFlags	WrapupAtPgmExit	(AST_INDEX exitPoint);
    AST_ChangeFlags	InitAtProcEntry	(AST_INDEX declPoint,
					 AST_INDEX initPoint);
    AST_ChangeFlags	WrapupAtProcExit(AST_INDEX /* exitPoint */)
						{ return INSTR_INSERTED_NONE; }
					 
    // ---- Actual instrumentation routines specialized for Pablo ----
    AST_ChangeFlags	InstrProcEntry	(AST_INDEX stmt,
					 StaticInfo static_info);
    AST_ChangeFlags	InstrProcExit	(AST_INDEX stmt,
					 StaticInfo static_info);
    AST_ChangeFlags	InstrLoopEntry	(AST_INDEX stmt, 
					 StaticInfo static_info);
    AST_ChangeFlags	InstrLoopExit	(AST_INDEX stmt, 
					 StaticInfo static_info);
    AST_ChangeFlags	InstrMesgCall	(AST_INDEX invocation,
					 AST_INDEX stmt,
					 StaticInfo static_info);

  private:
    //---- PRIVATE DATA ----
    PabloOldLibSystemInfo*	systemInfo;
    LibCurrentProcInstrInfo*	curProcInfo;

    enum {                            // base values where sequences of tag 
      PROC_TAG_BASE_VALUE = 100,      // numbers for different instr types
      LOOP_TAG_BASE_VALUE = 1000      // start counting
    };
    // ASSUMES: All other tags < 100, #procedures <= 999, #loops <= 9999
    
    Boolean	procsInstrumented;	// Any procedures instrumented? 
    Boolean	loopsInstrumented;	// Any loops instrumented? 
    Boolean	mesgsInstrumented;	// Any messages instrumented? 
    
    char	curLoopName[MAX_NAME];  // "Name" of "current" loop
    char	curProcName[MAX_NAME];  // Name of current procedure
    
    TagTable*	procTagTable;		// procedure entry/exit tag pairs.
    TagTable*	loopTagTable;		// loop entry/exit tag pairs
    TagTable*	loopTagTableForCurProc;	// contents copied to loopTagTable
					// at end of current procedure
    
    char*	auxTagListFileName;	// File listing external procedures
					//   and their tags
    //---- PRIVATE MEMBERS ----
    
    AST_ChangeFlags PabloInitAtPgmEntry	(AST_INDEX initPoint);
    
    AST_ChangeFlags ProcDeclAtPgmEntry	(AST_INDEX declPoint);
    AST_ChangeFlags ProcDeclAtProcEntry	(AST_INDEX declPoint);
    AST_ChangeFlags ProcInitAtPgmEntry	(AST_INDEX initPoint);
    
    AST_ChangeFlags LoopDeclAtPgmEntry	(AST_INDEX declPoint);
    AST_ChangeFlags LoopDeclAtProcEntry	(AST_INDEX declPoint);
    AST_ChangeFlags LoopInitAtPgmEntry	(AST_INDEX initPoint);
    
    AST_ChangeFlags MesgDeclAtProcEntry	(AST_INDEX declPoint);
    AST_ChangeFlags MesgInitAtPgmEntry	(AST_INDEX initPoint);
    AST_ChangeFlags ReplaceMesgCall	(AST_INDEX invocation,
					 char *invocation_name);
    
    inline void	    InstrumentingEntryExit();	// procedure is instrumented
    inline void	    InstrumentingMesgs();	// loops are instrumented
    inline void	    InstrumentingLoops();	// messages are instrumented
    
    inline Boolean  EntryExitInstrumented();	// Any procedures instrumented?
    inline Boolean  LoopsInstrumented();	// Any loops instrumented?
    inline Boolean  MesgsInstrumented();	// Any mesgs instrumented?
    
    inline Boolean  EntryExitInstrumentedInCurProc(); // Above four flags, but
    inline Boolean  LoopsInstrumentedInCurProc();     // for current procedure
    inline Boolean  MesgsInstrumentedInCurProc();
    
    AST_ChangeFlags insertTraceevent	(AST_INDEX stmt, char *eventTagName,
					 int arg1, int arg2);
    
    void	getTagsFromAuxFile	(char *aux_tag_list_file_name);

    void	NewLoop                 ();
};

// Inline member functions for class PabloOldTraceCaptureLib 

inline void PabloOldTraceCaptureLib::InstrumentingEntryExit()
{ procsInstrumented = true; curProcInfo->InstrumentingEntryExit(); }

inline void PabloOldTraceCaptureLib::InstrumentingMesgs()
{ mesgsInstrumented = true; curProcInfo->InstrumentingMesgs(); }

inline void PabloOldTraceCaptureLib::InstrumentingLoops()
{ loopsInstrumented = true; curProcInfo->InstrumentingLoops(); }

inline Boolean  PabloOldTraceCaptureLib::EntryExitInstrumented()
{ return procsInstrumented; }

inline Boolean  PabloOldTraceCaptureLib::LoopsInstrumented()
{ return loopsInstrumented; }

inline Boolean  PabloOldTraceCaptureLib::MesgsInstrumented()
{ return mesgsInstrumented; }

inline Boolean  PabloOldTraceCaptureLib::EntryExitInstrumentedInCurProc()
{ return curProcInfo->EntryExitInstrumented();}

inline Boolean  PabloOldTraceCaptureLib::LoopsInstrumentedInCurProc()
{ return curProcInfo->LoopsInstrumented(); }

inline Boolean  PabloOldTraceCaptureLib::MesgsInstrumentedInCurProc()
{ return curProcInfo->MesgsInstrumented(); }


#endif	/* #ifndef PabloLib_h */
