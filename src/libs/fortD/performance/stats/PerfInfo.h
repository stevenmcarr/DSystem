/* $Id: PerfInfo.h,v 1.1 1997/03/11 14:29:13 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// $Id: PerfInfo.h,v 1.1 1997/03/11 14:29:13 carr Exp $ -*-c++-*-
//**************************************************************************
// Interface used by D Editor to obtain dynamic performance metrics
//**************************************************************************

#ifndef PerfInfo_h
#define PerfInfo_h

#ifndef stdio_h
#include <stdio.h>
#endif

#ifndef FortDPerfTable_h
#include <libs/fortD/performance/stats/FortDPerfTable.h>
#endif
#ifndef StaticIDTable_h
#include <libs/fortD/performance/stats/StaticIDTable.h>
#endif
#ifndef Metrics_h
#include <libs/fortD/performance/stats/Metrics.h>
#endif

// Include the above files before the D system header files.
// is_open() has been #defined into something else in the D system,
// but is_open() is declared in fstream.h causing a syntax error.
// (PerfInfo.h -> FDCombine.h -> stream.h -> fstream.h)

#ifndef FD_CODEGEN			// For Dist_Globals
#include <libs/fortD/codeGen/private_dc.h>
#endif
#ifndef gen_h				// For AST_INDEX
#include <libs/frontEnd/ast/gen.h>
#endif
#ifndef FortTree_h
#include <libs/frontEnd/fortTree/FortTree.h>
#endif
#ifndef FortTextTree_h
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#endif
#ifndef HashTable_h
#include <libs/support/tables/HashTable.h>
#endif
const int HASH_TABLE_EMPTY = -1;	// HashTable doesn't export EMPTY


//--------------------------------------------------------------------------
// Declarations of external objects (instead of including header files):

// Declare these instead of #including FDCombine6.h, so as to avoid
// including all the Pablo header files wherever this file is included.
class RecordDossier;
class FileCell;
class LineStatCell;


//--------------------------------------------------------------------------

struct FortDPerfInfo_struct {		// Move to .i file later
    StaticIDTable*	sidTable;	// Pairs of location <--> static IDs
    FortDPerfTable*	inclPerfTable;	// Inclusive perf metrics for all IDs
    FortDPerfTable*	exclPerfTable;	// Exclusive perf metrics for all IDs
    FortDPerfTable*	mesgTotalPerfInfoTable;	// Total perf info for mesgs
    FortDPerfTable*	loopCommPerfInfoTable;	// Comm. perf info for loops
    PerfMetrics*	pgmPerfInfo;	// Inclusive perf info for full program

    //Temporarily used to create a unique ID for loops (no static IDs exist)
    HashTable*	  	loopIndexTable;
    
    // Handles to Fortran D compiler information
    char*		moduleFileName;
    AST_INDEX		moduleRoot;
    FortTree		ft;
    FortTextTree	ftt;
};

class FortDPerfInfo
{
  public:
    FortDPerfInfo(char* TraceFileName = "trace.bin");
    ~FortDPerfInfo();
    
    // ModuleInit must be called before any performance info
    // for the current module is accessed
    void	ModuleInit(char* moduleFileName, AST_INDEX root,
			   FortTextTree ftt, FortTree ft);
    
    char*	GetModuleFileName();
    Boolean	DoIt() {return doIt;}	// If false, public members do nothing
    
    
    // Aggregate performance metrics for display
    
    SetOfNamedFractions&  PgmTimeByComponent		();
    SetOfNamedFractions&  PgmTimeByProcedure		();
    SetOfNamedFractions&  PgmCommTimeByProcedure	();
    
    SetOfNamedFractions&  ProcedureTimeByComponent	(AST_INDEX procStmt);
    SetOfNamedFractions&  ProcedureCommTimeByStmt	(AST_INDEX procStmt);
    
    SetOfNamedFractions&  LoopTimeByComponent		(AST_INDEX loopHdr);
    double		  LoopTimeAsFrnOfPgmTime	(AST_INDEX loopHdr);
    double		  LoopCommTimeAsFrnOfPgmCommTime(Dist_Globals *dh,
							 AST_INDEX loopHdr);
    
    double		  MesgTimeAsFrnOfPgmTime	(AST_INDEX remoteRef);
    double		  MesgTimeAsFrnOfPgmCommTime	(AST_INDEX remoteRef);
    
    
    // Individual performance info for program, procedure, loop or message:
    //
    // Arguments identifying the required proc, loop or mesg:
    // For procedure : AST_INDEX of the procedure/function/program stmt
    // For loop      : AST_INDEX of the loop header
    // For message   : AST_INDEX of *any* non-local array reference
    // 			satisfied by that message.
    
    PerfMetrics*  PgmPerfInfo		();
    PerfMetrics*  ProcPerfInfo		(AST_INDEX procStmt, Boolean incl);
    PerfMetrics*  LoopPerfInfo		(AST_INDEX loopHdr,  Boolean incl);
    PerfMetrics*  MesgPerfInfo		(AST_INDEX remoteRef);
    
    // Temporarily added to compute loop communication information indirectly
    // from nested messages, rather than directly from traces.
    PerfMetrics*  LoopCommPerfInfo	(Dist_Globals *dh,
					 AST_INDEX loopHdr,
					 Boolean incl);
  private:
    Boolean	  doIt;			// If false, public members do nothing.
    struct FortDPerfInfo_struct repr;
    
    int		  GetStaticID		(AST_INDEX node);
    PerfMetrics*  MesgSendPerfInfo	(int staticID);
    PerfMetrics*  MesgRecvPerfInfo	(int staticID);
    
    int		  GetLoopStaticID	(AST_INDEX loopHdr);
};

inline char* FortDPerfInfo::GetModuleFileName()
{
    return repr.moduleFileName;
}


//--------------------------------------------------------------------------

#endif /* PerfInfo_h */
