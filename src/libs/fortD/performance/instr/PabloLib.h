/* $Id: PabloLib.h,v 1.1 1997/03/11 14:28:53 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// $Id: PabloLib.h,v 1.1 1997/03/11 14:28:53 carr Exp $ -*-c++-*-
//**************************************************************************
// Interface for PabloLib.h
//**************************************************************************

#ifndef PabloLib_h
#define PabloLib_h

#ifndef NameValueTable_h
#include <libs/support/tables/NameValueTable.h>
#endif

#include <libs/fortD/performance/instr/InstrumentSPMD.h>

//-----------------------------------------------------------------------
// Enums and structures used for Pablo instrumentation


//-----------------------------------------------------------------------
// Locally used class for managing entry/exit tag pairs.
// Currently these are required for: Procedures, Loops.

class TagTable
{
  public: //---- MEMBERS ----
    TagTable(int tagBase, char* countVarName,
	     char* entryVarName, char* exitVarName);
    ~TagTable();
    
    void		CreateTags	(char* unitName);

    inline int		NumberOfTags	();

    // lookupEntry returns a pointer to the a particular the list entry
    // (cast as void* since the client has no knowledge of what it points to). 
    // The client can invoke the get<field> calls using this pointer
    // instead of the unitName to avoid hashing multiple times when
    // accessing multiple fields of the same entry
    void*		lookupEntry	(char* unitName);

    // inlining avoided because of g++ problem described below.
    // restore inlining when the problem is fixed.
    /*inline*/ char*	GetUnitName	(void* entry);

    /*inline*/ char*	GetEntryTagName	(char* unitName);
    /*inline*/ char*	GetEntryTagName	(void* entry);

    /*inline*/ char*	GetExitTagName	(char* unitName);
    /*inline*/ char*	GetExitTagName	(void* entry);

    /*inline*/ int	GetEntryTagNum	(char* unitName);
    /*inline*/ int	GetEntryTagNum	(void* entry);

    /*inline*/ int	GetExitTagNum	(char* unitName);
    /*inline*/ int	GetExitTagNum	(void* entry);

    inline char* 	GetCountVarName	();
    inline char* 	GetEntryVarName	();
    inline char* 	GetExitVarName	();

    AST_ChangeFlags insertAllTagDeclarations	(AST_INDEX here,
						 Boolean isMainProgram = true);
    AST_ChangeFlags insertTagArrayDataStatements(AST_INDEX here);
    /*inline*/ AST_ChangeFlags insertTagDeclarations(AST_INDEX here,
						     char* unitName);
    void		Absorb		(TagTable* aTagTable);
    
  private:
    //---- DATA ----
    enum SIZES { TAG_LIST_INIT_SIZE  = 256 };

    typedef struct TagListEntry_struct
    {
	char unitName[MAX_NAME];		// currently: procedures, loops
	char entryTagName[MAX_NAME];
	char exitTagName[MAX_NAME];
	int  entryTagNumber;
	int  exitTagNumber;
    } TagListEntry;

    friend void		CleanupTagsEntryCallback(Generic /*name*/,
						 Generic value);

    TagListEntry*	curInstrTags;	// Pointer to current list entry
    int			curTagNum;	// Sequence number of next tag pair

    int			tagBase;	// Starting value for entry/exit tags.
    char		countVarName[MAX_NAME];	// name of count variable
    char		entryVarName[MAX_NAME];	// name of array of entry tags
    char		exitVarName[MAX_NAME];	// name of array of exit tags

    NameValueTable*	nameEntryPairs;

    //---- MEMBERS ----
    
    AST_ChangeFlags	insertTagDeclarations	(AST_INDEX here,
						 TagListEntry *listEntry);
    AST_ChangeFlags 	insertTagsCommonBlock	(AST_INDEX here,
						 TagListEntry *listEntry);
    AST_ChangeFlags	insertTagsTypeDecl	(AST_INDEX here,
						 TagListEntry *listEntry);
    AST_ChangeFlags 	insertTagsParameterStmts(AST_INDEX here,
						 TagListEntry *listEntry);
};

//----- Inline function definitions for class TagTable ----

// All functions that cast void* to TagListEntry* have been moved
// to the PabloLib.C because that cast produces a weird error message from
// g++ if those functions are defined here.  This error occurs whether or not
// those functions are declared/defined as inline here.
//
// Since the CC compiler on una doesn't produce this error, I assume that
// this is a problem with the g++ compiler.
// Move these functions back here and inline them when a new version of
// g++ accepts them.

// Total number of tags in the table:
inline int TagTable::NumberOfTags()		{ return curTagNum; }
    
// Names of the count, entry-tag-array and exit-tag-array variables
inline char* TagTable::GetCountVarName()	{ return countVarName; }
inline char* TagTable::GetEntryVarName()	{ return entryVarName; }
inline char* TagTable::GetExitVarName()		{ return  exitVarName; }

    
//-----------------------------------------------------------------------
// System-specific info for this instrumentation library.
// For Pablo, only need to customize a few library routine names.

static char* IPSC_PROC_TRACE_INIT_CALL = "FDinitProcTrace";
static char* IPSC_LOOP_TRACE_INIT_CALL = "FDinitLoopTrace";
static char* IPSC_MESG_TRACE_INIT_CALL = "FDiPSCtraceInit";

class LibSystemInfo : public SystemInfo
{
  public:
    LibSystemInfo(Fd_Arch_type arch);
    ~LibSystemInfo() {};
    inline char*  LoopTraceInitCall();
    inline char*  ProcTraceInitCall();
    inline char*  MesgTraceInitCall();
  private:
    char* procTraceInitCall;
    char* loopTraceInitCall;
    char* mesgTraceInitCall;
};

//----- Inline function definitions for class LibSystemInfo ----
inline char*  LibSystemInfo::LoopTraceInitCall() { return loopTraceInitCall; }
inline char*  LibSystemInfo::ProcTraceInitCall() { return procTraceInitCall; }
inline char*  LibSystemInfo::MesgTraceInitCall() { return mesgTraceInitCall; }
    
//-----------------------------------------------------------------------
// Current procedure info for this instrumentation library.

class LibCurrentProcInstrInfo : public CurrentProcInstrInfo
{
  public:
    LibCurrentProcInstrInfo(Dist_Globals *dh) : CurrentProcInstrInfo(dh),
    						entryExitInstrumented(false),
						mesgsInstrumented(false),
    						loopsInstrumented(false),
    						symbsInstrumented(false) {}
    ~LibCurrentProcInstrInfo() {};
    
    inline Boolean  EntryExitInstrumented();
    inline Boolean  MesgsInstrumented();
    inline Boolean  LoopsInstrumented();
    inline Boolean  SymbsInstrumented();
    
    inline void	    InstrumentingEntryExit();
    inline void	    InstrumentingMesgs();
    inline void	    InstrumentingLoops();
    inline void     InstrumentingSymbs();
    
  private:
    Boolean	entryExitInstrumented;
    Boolean	mesgsInstrumented;
    Boolean	loopsInstrumented;
    Boolean	symbsInstrumented;
};

//----- Inline function definitions for class LibCurrentProcInstrInfo ----

inline Boolean LibCurrentProcInstrInfo::EntryExitInstrumented()
{ return entryExitInstrumented; }

inline Boolean LibCurrentProcInstrInfo::MesgsInstrumented()
{ return mesgsInstrumented; }

inline Boolean LibCurrentProcInstrInfo::LoopsInstrumented()
{ return loopsInstrumented; }

inline Boolean LibCurrentProcInstrInfo::SymbsInstrumented()
{ return symbsInstrumented; }

inline void LibCurrentProcInstrInfo::InstrumentingEntryExit()
{ entryExitInstrumented=true;}

inline void LibCurrentProcInstrInfo::InstrumentingMesgs()
{ mesgsInstrumented = true; }

inline void LibCurrentProcInstrInfo::InstrumentingLoops()
{ loopsInstrumented = true; }

inline void LibCurrentProcInstrInfo::InstrumentingSymbs()
{ symbsInstrumented = true; }

    
//-----------------------------------------------------------------------
// class PabloTraceCaptureLib : Interface to Pablo trace capture library.
//
// This class is derived from base class InstrumentationLibrary
// which is a generic interface for any low level instrumentation library.
// This class implements the interface functions for the Pablo trace
// capture library.
//-----------------------------------------------------------------------

class PabloTraceCaptureLib : public InstrumentationLibrary
{
  public:
    PabloTraceCaptureLib();
    ~PabloTraceCaptureLib();
    
    SystemInfo*	    	  SetupSystemInfo(Fd_Arch_type arch);
    SystemInfo*		  DeleteSystemInfo();
    CurrentProcInstrInfo* SetupCurProcInfo(Dist_Globals *dh);
    CurrentProcInstrInfo* DeleteCurProcInfo();
    
    // ---- Init and wrapup routines specialized for Pablo ----
    AST_ChangeFlags
	InitAtPgmEntry	(AST_INDEX declPoint,
					 AST_INDEX initPoint); 
    AST_ChangeFlags
	WrapupAtPgmExit	(AST_INDEX exitPoint);
    AST_ChangeFlags
	InitAtProcEntry	(AST_INDEX declPoint,
					 AST_INDEX initPoint);
    AST_ChangeFlags
	WrapupAtProcExit(AST_INDEX /* exitPoint */)
						{ return INSTR_INSERTED_NONE; }
					 
    // ---- Actual instrumentation routines specialized for Pablo ----
    AST_ChangeFlags
	InstrProcEntry	(AST_INDEX stmt,
			 StaticInfo static_info);
    AST_ChangeFlags
	InstrProcExit	(AST_INDEX stmt, 
			 StaticInfo static_info);
    AST_ChangeFlags
	InstrLoopEntry	(AST_INDEX stmt, 
			 StaticInfo static_info);
    AST_ChangeFlags
	InstrLoopExit	(AST_INDEX stmt, 
			 StaticInfo static_info);
    AST_ChangeFlags
	InstrMesgCall	(AST_INDEX invocation,
			 AST_INDEX stmt,
			 StaticInfo static_info);
    AST_ChangeFlags
	MarkIntervalStart(AST_INDEX 	/* stmt */,
			  StaticInfo 	/* static_info */)
						{return INSTR_INSERTED_NONE;}
    AST_ChangeFlags
	MarkIntervalEnd	(AST_INDEX 	/* stmt */,
			 StaticInfo 	/* static_info */)
			 			{ return INSTR_INSERTED_NONE; }
    AST_ChangeFlags
    PrintSymbolic	(AST_INDEX 		intExpr,
			 AST_INDEX 		stmt,
			 StaticInfo 		static_info,
			 Boolean		addGuard,
			 int			initialValue);

  private:
    //---- PRIVATE DATA ----
    LibSystemInfo*		systemInfo;
    LibCurrentProcInstrInfo*	curProcInfo;
    
    // Starting values for different sequences of dynamic IDs.
    enum {
	SYM_TAG_BASE_VALUE  = 0,	// Symb must have ID = static_id
	SYM_TAG_MAX_VALUE   = 999,	// Max static_id, else overlap proctags
	PROC_TAG_BASE_VALUE = 1000,	// Assume: #procedures instr <= 999,
	PROC_TAG_MAX_VALUE  = 1999,	// Max proctag, else overlap looptags
	LOOP_TAG_BASE_VALUE = 2000,	// Assume: #loops instr <= 999,
	LOOP_TAG_MAX_VALUE  = 2999	// Max proctag, else overlap looptags
    };
    // ASSUMES: All other tags < 1000,  #loops <= 999
    
    typedef enum FD_TraceLevel_enum {		// Trace level used in Pablo
	TRACE_NONE, TRACE_FULL, TRACE_COUNT,	//   trace routine FDtrace()
	TRACE_INTERVAL_ENTRY, TRACE_INTERVAL_EXIT, TRACE_SYMBOLIC
    } FD_TraceLevel;
    
    Boolean	procsInstrumented;	// Any procedures instrumented? 
    Boolean	loopsInstrumented;	// Any loops instrumented? 
    Boolean	mesgsInstrumented;	// Any messages instrumented? 
    Boolean	symbsInstrumented;	// Any symbolics instrumented? 
    
    char	curLoopName[MAX_NAME];	// "Name" of "current" loop
    char	curProcName[MAX_NAME];	// Name of current procedure
    
    TagTable*	procTagTable;		// procedure entry/exit tag pairs.
    TagTable*	loopTagTable;		// loop entry/exit tag pairs
    TagTable*	loopTagTableForCurProc;	// contents copied to loopTagTable
					// at end of current procedure
    char*	auxTagListFileName;	// File listing external procedures
					//   and their tags
    enum	{ SYM_VAR_TABLE_INIT_SIZE = 16 };
    NameValueTable* SymOldVarTable;	// "Last" value var: name, init. value
    
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
					 char *invocation_name,
					 AST_INDEX stmt,  
					 StaticInfo staticInfo);
    AST_ChangeFlags SymbDeclAndInitAtEntry(AST_INDEX declPoint,
					   AST_INDEX initPoint);
    AST_INDEX	    MakeOldValueAst	(AST_INDEX intExpr,
					 StaticInfo static_info,
					 int initialValue);
    
    inline void	    InstrumentingEntryExit();	// procedure is instrumented
    inline void	    InstrumentingMesgs();	// loops are instrumented
    inline void	    InstrumentingLoops();	// messages are instrumented
    inline void	    InstrumentingSymbs();	// symbolics are instrumented
    
    inline Boolean  EntryExitInstrumented();	// Any procedures instrumented?
    inline Boolean  LoopsInstrumented();	// Any loops instrumented?
    inline Boolean  MesgsInstrumented();	// Any mesgs instrumented?
    inline Boolean  SymbsInstrumented();	// Any symbolics instrumented?
    
    inline Boolean  EntryExitInstrumentedInCurProc(); // Above four flags, but
    inline Boolean  LoopsInstrumentedInCurProc();     // for current procedure
    inline Boolean  MesgsInstrumentedInCurProc();
    inline Boolean  SymbsInstrumentedInCurProc();
    
    AST_ChangeFlags insertFDtrace	(AST_INDEX stmt,
					 FD_TraceLevel traceLevel,
					 char *eventTagName,
					 StaticInfo staticInfo);
    AST_ChangeFlags insertFDtrace	(AST_INDEX stmt,
					 FD_TraceLevel traceLevel,
					 int eventTagName,
					 AST_INDEX intDataVar);
    // int		NextSymEventNum		();
    
    void	getTagsFromAuxFile	(char *aux_tag_list_file_name);
    
    void 	NewLoop			();
};

// Inline member functions for class PabloTraceCaptureLib 

inline void PabloTraceCaptureLib::InstrumentingEntryExit()
{ procsInstrumented = true; curProcInfo->InstrumentingEntryExit(); }

inline void PabloTraceCaptureLib::InstrumentingMesgs()
{ mesgsInstrumented = true; curProcInfo->InstrumentingMesgs(); }

inline void PabloTraceCaptureLib::InstrumentingLoops()
{ loopsInstrumented = true; curProcInfo->InstrumentingLoops(); }

inline void PabloTraceCaptureLib::InstrumentingSymbs()
{ symbsInstrumented = true; curProcInfo->InstrumentingSymbs(); }

inline Boolean  PabloTraceCaptureLib::EntryExitInstrumented()
{ return procsInstrumented; }

inline Boolean  PabloTraceCaptureLib::LoopsInstrumented()
{ return loopsInstrumented; }

inline Boolean  PabloTraceCaptureLib::MesgsInstrumented()
{ return mesgsInstrumented; }

inline Boolean  PabloTraceCaptureLib::SymbsInstrumented()
{ return symbsInstrumented; }

inline Boolean  PabloTraceCaptureLib::EntryExitInstrumentedInCurProc()
{ return curProcInfo->EntryExitInstrumented();}

inline Boolean  PabloTraceCaptureLib::LoopsInstrumentedInCurProc()
{ return curProcInfo->LoopsInstrumented(); }

inline Boolean  PabloTraceCaptureLib::MesgsInstrumentedInCurProc()
{ return curProcInfo->MesgsInstrumented(); }

inline Boolean  PabloTraceCaptureLib::SymbsInstrumentedInCurProc()
{ return curProcInfo->SymbsInstrumented(); }


#endif	/* #ifndef PabloLib_h */

