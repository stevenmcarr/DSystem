/* $Id: InstrumentSPMD.h,v 1.10 1997/03/11 14:28:50 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// $Id: InstrumentSPMD.h,v 1.10 1997/03/11 14:28:50 carr Exp $ -*-c++-*-
//**************************************************************************
// Declarations for InstrumentSPMD.C
//**************************************************************************

#ifndef instrument_spmd_h
#define instrument_spmd_h

#include <assert.h>

#ifndef general_h
#include <libs/support/misc/general.h>
#endif
#ifndef fd_types_h
#include <libs/fortD/misc/fd_types.h>
#endif
#ifndef FD_CODEGEN
#include <libs/fortD/codeGen/private_dc.h>
#endif

#ifndef DEBUG_INSTRUMENTATION
#define DEBUG_INSTRUMENTATION	(false)
#endif

//-----------------------------------------------------------------------
// Common enums and structures for instrumentation

typedef enum ProgramUnitType_enum {
    INSTR_MAIN_PROGRAM = 1,
    INSTR_SUBROUTINE   = 2,
    INSTR_FUNCTION     = 3
} ProgramUnitType;

typedef short AST_ChangeFlags;		// Cannot use an enum because
#define    INSTR_INSERTED_NONE	   (0)	// I need to take bit-or on these
#define    INSTR_INSERTED_BEFORE   (1)	// values.
#define    INSTR_INSERTED_AFTER    (2)
#define    INSTR_INSERTED_REPLACED (4)

typedef int StaticInfo;


//-----------------------------------------------------------------------
// Insrumentation options info

class InstrumentationOptions
{
  public:
    InstrumentationOptions(Fd_opts* fd_opts);
    ~InstrumentationOptions();
    void	   UpdateOptions(Fd_opts* fd_opts);
    
    inline Boolean InstrPgmExecTime()	{ return opts.instrPgmExecTime; }
    inline Boolean InstrProcedures()	{ return opts.instrProcedures; }
    inline Boolean InstrMessages()	{ return opts.instrMessages; }
    inline Boolean InstrLoops()		{ return opts.instrLoops; }
    inline Boolean InstrFullSymbolics()	{ return opts.instrFullSymbolics; }
    inline Boolean InstrRequested()	{ return (Boolean) (InstrPgmExecTime()
						  || InstrProcedures()
						  || InstrMessages()
						  || InstrLoops()
						  || InstrFullSymbolics()); }
    inline int	   InstrNprocs()	{ return opts.nprocs; }
    inline Fd_Arch_type InstrArch() 	{ return opts.arch; }

  private:
    typedef struct InstrOpts_struct {
	Boolean instrPgmExecTime;
	Boolean instrProcedures;
	Boolean instrMessages;
	Boolean instrLoops;
	Boolean instrFullSymbolics;
	int	nprocs;                 //
	Fd_Arch_type arch;		// target architecture
    } InstrOpts;
    InstrOpts opts;
};


//-----------------------------------------------------------------------
// Book-keeping info for each procedure

class CurrentProcInstrInfo
{
  public:
    CurrentProcInstrInfo(Dist_Globals *dh);
    virtual ~CurrentProcInstrInfo();
    
    char *	get_proc_name			(); // Name of cur. procedure
    Dist_Globals* get_dh_handle			();
    
    Boolean	is_main_program			();
    Boolean	is_first_stmt			(AST_INDEX stmt);
    Boolean	is_program_exit_point		(AST_INDEX stmt);
    Boolean	is_subprogram_exit_point	(AST_INDEX stmt);
    Boolean	is_last_stmt_in_program_unit	(AST_INDEX stmt);
    Boolean	is_program_unit_exit_stmt	(AST_INDEX stmt);
    Boolean	is_loop_exit_stmt		(AST_INDEX stmt);
    
    AST_INDEX 	get_orig_first_stmt		();
    AST_INDEX 	get_first_stmt_after_decl	();
    
    AST_INDEX 	placeHolderAfter		(AST_INDEX stmt);
    void 	deletePlaceHolder		(AST_INDEX ph,
						 Boolean deleteAll = false);
    Boolean	checkAndMoveLabel		(AST_INDEX stmt);
    
    inline AST_INDEX	GetNewNode		()
	{ return repr.newReplaceNode; }
    inline void		saveReplaceNode		(AST_INDEX newNode)
	{ repr.newReplaceNode = newNode; }
  
  private:
    struct CurrentProcInfo_S		// Info about current program unit
    {
	Dist_Globals	*dh;			// Compiler data
	AST_INDEX	root;			// Root of program unit AST
	char		program_unit_name[MAX_NAME];
	ProgramUnitType program_unit_type;	// Subr/func/main pgm?

	AST_INDEX	first_executable_stmt;	// Marks proc. entry point
	AST_INDEX	first_stmt_after_decl;	// Where declarations go
	AST_INDEX	instrPh;		// Where entry instr. goes

	enum CurProcConstants_enum { MAX_PLACE_HOLDERS = 64 };
	AST_INDEX phList[MAX_PLACE_HOLDERS];
	int  numPh; 
	
	AST_INDEX		 	newReplaceNode;
    } repr;

    void	mark_first_executable_stmt(AST_INDEX stmt);
    Boolean	find_name_and_type(AST_INDEX stmt);
};


/************************************************************************
 * class SystemInfo
 * 
 * Generic interface for system-specific information.
 * Includes some information that can be used by any instrumentation library.
 * Can be specialized by deriving a new class for a specific library.
 ***********************************************************************/

// ----- List of message-passing routines that can be traced ----
static const int IPSC_NUM_MESG_CALLS_TO_TRACE = 27;
static char* IPSC_MESG_CALL_LIST[] =
	{ "csend", "crecv",
	  "isend", "irecv", "msgwait",
	  "gcol", "gcolx", "gdhigh", "gdlow", "gdprod", "gdsum",
	  "giand", "gihigh", "gilow", "gior", "giprod", "gisum", "gixor",
	  "gland", "glor", "glxor",
	  "gopf", "gsendx", "gshigh", "gslow", "gsprod", "gssum"
	};

class SystemInfo
{
  public:
    SystemInfo(Fd_Arch_type arch);
    virtual ~SystemInfo();

    // ---- Members describing message library routines for given arch ----
    virtual Boolean	InstrumentableMesgCall	(char* callName);

    virtual int		NumMesgCallsToTrace	();
    virtual int		MessageNumber		(char *msgCallName);
    virtual char*	MessageName		(int msgCallNum);

    virtual void	recordInvocation	(char *msgCallName);
    virtual int		NumInvocations		(int msgCallNum);
    virtual int		NumInvocationsInCurProc	(int msgCallNum);
    virtual void	ResetInvocationsInCurProc(int msgCallNum);

    // ---- Members describing timing library routines for given arch ----
    virtual AST_INDEX	InitTimerCall		();
    virtual AST_INDEX	Timer			(char* timerVarName);
    virtual AST_INDEX	TimerPrint		(char* myIndexVarName,
						 char* printMesg,
						 char* startVarName,
						 char* endVarName);
  protected:
    Fd_Arch_type archType;		// Mesg-passing architecture
    int		numMesgCallsToTrace;	// Number of traceable mesg calls
    char**	mesgCallList;		// List   of traceable mesg calls
    int*  	numInvocations;		// #invocations of each mesg call
    int* 	numInvocationsInCurrentProc; // '' in current procedure
};
 


/************************************************************************
 * class InstrumentationLibrary
 * 
 * Generic interface to the low-level instrumentation library routines.
 * A derived class must implement some or all of these interface routines
 * for each library.  Here each of these routines is just a NOP.
 *
 * Usage:
 * (1) SetupSystemInfo(arch) is called once at the start of compilation.
 *     DeleteSystemInfo() is called once at the end of compilation.
 * (2) SetupCurProcInfo(dh) called once at the start of compiling each subr.
 *     DeleteCurProcInfo() called once at the end of compiling each subr.
 *
 * Constraints on the library:
 * (1) each routine only inserts code before and/or after the
 *     statement node(s) passed in as parameters, and may replace the
 *     node itself, but does not insert code elsewhere in or otherwise modify
 *     the tree.
 *
 * Assumptions that the library is allowed to make:
 * (1) the node(s) passed in is a statement
 * (2) the statement(s) passed in to each lib routine cannot be
 *     jumped to directly, i.e., it is ok to insert instrumentation before
 *     and/or after the statement and guarantee that control passing through
 *     the statement also passes through the inserted instrumentation code.
 * (3) InstrProcEntry() will be called before InstrProcExit().
 *     InstrLoopEntry() will be called before InstrLoopExit().
 ***********************************************************************/

class InstrumentationLibrary
{
  public:
    InstrumentationLibrary() {}
    virtual ~InstrumentationLibrary() {}
    
    // ---- Allocate classes encapsulating system-specific information
    // and current procedure information. These *must* be defined by
    // derived class for each library.
    virtual SystemInfo*	    	  SetupSystemInfo(Fd_Arch_type arch) = 0;
    virtual SystemInfo*		  DeleteSystemInfo() = 0;
    virtual CurrentProcInstrInfo* SetupCurProcInfo(Dist_Globals *dh) = 0;
    virtual CurrentProcInstrInfo* DeleteCurProcInfo() = 0;
    
    // ---- Init and wrapup routines ----
    virtual AST_ChangeFlags
	InitAtPgmEntry(AST_INDEX	 	/* declPoint */,
		       AST_INDEX	 	/* initPoint */)
					{ return INSTR_INSERTED_NONE; }
    virtual AST_ChangeFlags
	WrapupAtPgmExit(AST_INDEX 		/* exitPoint */)
					{ return INSTR_INSERTED_NONE; }
    virtual AST_ChangeFlags
	InitAtProcEntry(AST_INDEX 		/* declPoint */,
			AST_INDEX 		/* initPoint */) 
					{ return INSTR_INSERTED_NONE; }
    virtual AST_ChangeFlags
	WrapupAtProcExit(AST_INDEX 		/* exitPoint */)
					{ return INSTR_INSERTED_NONE; }
					 
    // ---- Actual instrumentation routines ----
    virtual AST_ChangeFlags
	InstrProcEntry(AST_INDEX 		/* stmt */,
		       StaticInfo 		/* static_info */)
					{ return INSTR_INSERTED_NONE; }
    virtual AST_ChangeFlags
	InstrProcExit(AST_INDEX 		/* stmt */, 
		      StaticInfo 		/* static_info */)
					{ return INSTR_INSERTED_NONE; }
    virtual AST_ChangeFlags
	InstrLoopEntry(AST_INDEX 		/* stmt */, 
		       StaticInfo 		/* static_info */)
					{ return INSTR_INSERTED_NONE; }
    virtual AST_ChangeFlags
	InstrLoopExit(AST_INDEX 		/* stmt */, 
		      StaticInfo 		/* static_info */)
					{ return INSTR_INSERTED_NONE; }
    virtual AST_ChangeFlags
	InstrMesgCall(AST_INDEX 		/* invocation */,
		      AST_INDEX 		/* stmt */,
		      StaticInfo 		/* static_info */)
					{ return INSTR_INSERTED_NONE; }
    virtual AST_ChangeFlags
	MarkIntervalStart(AST_INDEX 		/* stmt */,
			  StaticInfo 		/* static_info */)
					{ return INSTR_INSERTED_NONE; }
    virtual AST_ChangeFlags
	MarkIntervalEnd(AST_INDEX 		/* stmt */,
			StaticInfo 		/* static_info */)
					{ return INSTR_INSERTED_NONE; }
    virtual AST_ChangeFlags
	PrintSymbolic(AST_INDEX 		/* expr */,
		      AST_INDEX 		/* stmt */,
		      StaticInfo 		/* static_info */,
		      Boolean			/* addGuard */,
		      int			/* initialValue */)
					{ return INSTR_INSERTED_NONE; }
};


/************************************************************************
 * Class:
 *    SPMDInstrumentation
 * Description:
 *    This instrumentation object provides a package of
 *    low-level, demand-driven instrumentation primitives for:
 *		-- individual messages
 *		-- procedures (execution time)
 *		-- loops (execution time)
 *		-- full program (execution time)
 *		-- an arbitrary interval of statements (execution time)
 *    Actual code insertion is done by the instrumentation library specific
 *    services provided by the appropriate derived class of the base class
 *    InstrumentationLibrary.
 *
 *    Usage:
 *	1. Allocate this object (SPMDInstrumentation)
 *	2. For each program unit (i.e., subroutine, function or main program):
 *		(a) Repeatedly call one of the instrumentation routines
 *                  to instrument one of the above choices
 *		(b) Call wrapupProgramUnit(dh) to insert the initialization
 *                  and wrap-up code for this procedure.
 *         The main program *must* be instrumented last.
 *      3. Delete the object. This object must not be deleted until the
 *         end of compilation.
 *
 * Original Author:
 *    Vikram Adve
 * Creation Date:
 *    May 1994
 * Change History:
 *    05/08/94 (adve):  Initial.
 ***********************************************************************/

class SPMDInstrumentation
{
  public:
    SPMDInstrumentation(Fd_opts* fd_opts);
    virtual ~SPMDInstrumentation();

    // Instr requested and possible?
    Boolean	InstrumentationOn	  ();

    AST_ChangeFlags 	instrumentMesgCall(Dist_Globals *dh,
					   AST_INDEX invocation,
					   AST_INDEX stmt,
					   int staticId);
    AST_ChangeFlags 	instrumentProcedureEntryExit(Dist_Globals *dh,
						     AST_INDEX header,
						     int enterId,
						     int exitId);
    AST_ChangeFlags 	measureLoopTime(Dist_Globals *dh,
					AST_INDEX stmt,
					int enterId,
					int exitId);
    AST_ChangeFlags 	measureProgramTime(Dist_Globals *dh,
					   AST_INDEX header);
    AST_ChangeFlags 	measureIntervalTime(Dist_Globals *dh,
					    AST_INDEX start_stmt,
					    AST_INDEX end_stmt,
					    int enterId,
					    int exitId);
    AST_ChangeFlags	PrintSymbolic(Dist_Globals *dh,
				      AST_INDEX	 intExpr,
				      AST_INDEX  stmt,
				      StaticInfo static_info,
				      Boolean	 addGuard,
				      int	 initialValue);
    AST_ChangeFlags	wrapupProgramUnit(Dist_Globals *dh);

    inline AST_INDEX	GetNewNode()	{return curProcInfo->GetNewNode();}

    inline SystemInfo*		 GetSystemInfo()  { return systemInfo; }
    inline CurrentProcInstrInfo* GetCurProcInfo() { return curProcInfo; }
    inline InstrumentationOptions* GetInstrOpts() { return instrOpts; }

  private:
    Boolean			doInstr;	// Instr requested and possible
    InstrumentationOptions*	instrOpts;	// Instrumentation options
    InstrumentationLibrary*	iLib;		// Low-level library interface
    SystemInfo*			systemInfo;	// System-specific info.
    CurrentProcInstrInfo*	curProcInfo;	// Data for cur node in CG
    
    void doSetupIfRequired(Dist_Globals *dh);	// Initialization on the 1st
						// call for a program unit.

    // Data and member routines to measure pgm execution time.
    char	startTimeVarName[MAX_NAME];
    char	endTimeVarName[MAX_NAME];
    
    AST_ChangeFlags InstrPgmEntry	(Dist_Globals* dh, AST_INDEX stmt);
    AST_ChangeFlags InstrPgmExit	(Dist_Globals* dh, AST_INDEX stmt);
    AST_ChangeFlags InitPgmTiming	(AST_INDEX declPoint,
					 AST_INDEX initPoint);
};


//**************************************************************************
// Minor helper functions for generating/inserting specific kinds of stmts

EXTERN(AST_INDEX, int_decl_stmt,		(char *int_name));
EXTERN(AST_INDEX, int_parameter_stmt,		(char *int_name,
						 int int_value));
EXTERN(void,	add_blank_line_before,	(AST_INDEX node));
EXTERN(void,	insert_commented_cpp_dir,	(AST_INDEX stmt,
						 char *cpp_dir_text));
EXTERN(Boolean,	is_subprogram_exit_stmt,	(AST_INDEX stmt));
EXTERN(Boolean,	is_program_unit_exit_stmt,	(AST_INDEX stmt));
EXTERN(Boolean,	is_last_stmt_in_program_unit,	(AST_INDEX stmt));
 
/* Added to prevent warning about Pablo macro of the same name: */
#undef Assert

#ifdef  NDEBUG
#   define Assert(Expr) ((void)0)
#else
#   if defined(__STDC__)
#	define Assert(Expr)\
	    do {\
		if (!(Expr)) {\
		    puts("\n*** Fatal internal instrumentation error:");\
		    puts("*** !( " #Expr " )\n");\
		    fflush(stdout);\
		    assert(false);\
	       }\
	    } while (0)
#    else	/* __STDC__ */
#	define Assert(Expr)\
	    do {\
		if (!(Expr)) {\
		    puts("\n*** Fatal internal instrumentation error:");\
		    puts("*** !( Expr )\n");\
		    fflush(stdout);\
		    assert(false);\
	       }\
	    } while (0)
#endif /* __STDC__ */
#endif

//--------------------------------------------------------------------------
#endif	/* #ifndef instrument_spmd_h */
