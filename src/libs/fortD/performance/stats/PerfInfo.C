/* $Id: PerfInfo.C,v 1.3 2001/10/12 19:33:25 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// $Id: PerfInfo.C,v 1.3 2001/10/12 19:33:25 carr Exp $
//**************************************************************************
// Implementation of class FortDPerfInfo: Wrapper class for accessing runtime
//					  performance data from Pablo traces.
//**************************************************************************

/***************************************************************************
NOTES:

lineStatTable[], the main table of statistics, is indexed by StaticID.
Thus, in order to obtain perf info for a particular procedure/loop/mesg,
we have to first find the staticID allocated to it when the program was
first compiled for instrumenting.  Do this as follows:

In Ded:
    1) For a given procedure: get fileName, lineNum (of procedure declaration)
    2) For a given loop     : get fileName, lineNum (of loop header)
    3) For a non-local ref  : get fileName, lineNum, charPos (flc)
    4) For a given mesg: choose any non-local ref in its rset and do 3)
In my code:    
    5) Use flc info passed in from Ded to index into hash table; get StaticID

Step (5) implemented as follows:
When processing trace file (merge of static file and dynamic file),
create a hash-table of pairs: [flc, staticID]
where:
	flc	 == file-name,line-num,char-pos for a particular proc/loop/ref
		    (use c = -1 for procedures and loops)
	staticID == static ID of the corresponding proc/loop/mesg

***************************************************************************/



//--------------------------------------------------------------------------
// INCLUDE FILES
//--------------------------------------------------------------------------

#include <assert.h>

#ifndef FDCombine6_h
#include <libs/fortD/performance/stats/FDCombine6.h>
#endif

// OVERFLOW is #defined to 3 in math.h (which is in FDCombine6.h) but 
// I cannot move PerfInfo.h after the fortran D header files because
// of another such #define problem in the reverse direction. So:
#undef OVERFLOW

#ifndef CallSequence_h
#include <libs/fortD/performance/stats/CallSequence.h>
#endif

#include <libs/fortD/performance/stats/PerfInfo.h>

#ifndef groups_h			// From the Fortran D world
#include <libs/frontEnd/ast/groups.h>
#endif
#ifndef AstIterators_h
#include <libs/frontEnd/ast/AstIterators.h>
#endif


//--------------------------------------------------------------------------
// EXTERNAL VARIABLES AND FUNCTIONS
//--------------------------------------------------------------------------

EXTERN(void, tree_print,(AST_INDEX node));
EXTERN(void, TestPrint,	(SetOfNamedFractions& snf));

// Next two are defined here but EXTERN'ed to get naming right for linkage.
EXTERN(void, MakeStaticIDTable,	(StaticIDTable* sidTable));
EXTERN(void, printPerfStatistics, (Dist_Globals* dh, Generic PerfInfoObjectP));



//--------------------------------------------------------------------------
// INTERNAL VARIABLES AND FUNCTIONS
//--------------------------------------------------------------------------

uint MyIntegerHashFunct(const void* const value, const uint size);
int MyIntegerEntryCompare(const void* const value1, const void* const value2);


//**************************************************************************
// MEMBER FUNCTION DEFINITIONS FOR class FortDPerfInfo
//**************************************************************************

FortDPerfInfo::FortDPerfInfo(char* TraceFileName)
{
  doIt = (ProcessTraces(TraceFileName) == 1)? true : false;
  if (!doIt) return;
    
  repr.sidTable = new StaticIDTable;
  MakeStaticIDTable(repr.sidTable);
    
  repr.inclPerfTable = new FortDPerfTable(true /*incl*/);
  repr.inclPerfTable->ConstructPerfTable(staticTable,lineHeader,lineStatTable);
  
  repr.exclPerfTable = new FortDPerfTable(false /*incl*/);
  repr.exclPerfTable->ConstructPerfTable(staticTable,lineHeader,lineStatTable);
  
  repr.mesgTotalPerfInfoTable = new FortDPerfTable(false /*incl*/);
  repr.loopCommPerfInfoTable  = new FortDPerfTable(true  /*incl*/);
  
  repr.loopIndexTable = new HashTable();
  repr.loopIndexTable->Create(sizeof(int),		// entrySize
			      (uint) 64,		// initialSize
			      (HashFunctFunctPtr)    MyIntegerHashFunct,
			      (RehashFunctFunctPtr)  IntegerRehashHashFunct,
			      (EntryCompareFunctPtr) MyIntegerEntryCompare,
			      (EntryCleanupFunctPtr) NULL);
  
  if (VerifyCallSequence()) {		// Perf info for main pgm available?
      int pgmStaticID = ProgramID(); 
      assert(pgmStaticID != DUMMY_STATIC_ID);
      repr.pgmPerfInfo = repr.inclPerfTable->GetPerfInfo(pgmStaticID);
      assert(repr.pgmPerfInfo != (PerfMetrics*) NULL);

      // FOR IMMEDIATE TESTING ONLY
      printf("\n\tPERFORMANCE STATISTICS FOR MAIN PROGRAM\n\n");
      repr.pgmPerfInfo->Print();

      printf("\n\n\tPROGRAM TIME BY COMPONENT\n\n");
      TestPrint(PgmTimeByComponent());
      
      printf("\n\n\tPROGRAM TIME BY PROCEDURE\n\n");
      TestPrint(PgmTimeByProcedure());
      
      printf("\n\n\tPROGRAM COMMUNICATION TIME BY PROCEDURE\n\n");
      TestPrint(PgmCommTimeByProcedure());
      printf("\n\n");
  }
  else
      repr.pgmPerfInfo = (PerfMetrics*) NULL;
}

FortDPerfInfo::~FortDPerfInfo()
{
    if (!doIt) return;
    repr.loopIndexTable->Destroy();
    delete repr.loopIndexTable;
    delete repr.loopCommPerfInfoTable;
    delete repr.mesgTotalPerfInfoTable;
    delete repr.exclPerfTable;
    delete repr.inclPerfTable;
    delete repr.sidTable;
}

void FortDPerfInfo::ModuleInit(char* moduleFileName, AST_INDEX root,
			       FortTextTree ftt, FortTree ft)
{
    if (!doIt) return;
  
    // TEMPORARY FIX for inconsistency between old and new repository names:
    // Set file name as the tail component of the path name.
    char* startchar = strrchr(moduleFileName, (int)'/');
    if (startchar == (char*) NULL)	// i.e., no '/' in path name
	startchar = moduleFileName;
    else
	startchar++;			// char following last '/'
    repr.moduleFileName = startchar;
    
    repr.moduleRoot = root;
    repr.ftt = ftt;
    repr.ft  = ft;
}

//--------------------------------------------------------------------------

inline PerfMetrics* FortDPerfInfo::PgmPerfInfo()
{ return ((doIt)? repr.pgmPerfInfo : NULL); }

//--------------------------------------------------------------------------

PerfMetrics* FortDPerfInfo::ProcPerfInfo(AST_INDEX procStmt, Boolean incl)
{
    if (!doIt) return NULL;
    int staticID = GetStaticID(procStmt);
    if (staticID == DUMMY_STATIC_ID)	// No perf info for this code object
	return (PerfMetrics*) NULL;
    
    return (incl)? repr.inclPerfTable->GetPerfInfo(staticID)
		 : repr.exclPerfTable->GetPerfInfo(staticID);
}
//--------------------------------------------------------------------------

PerfMetrics* FortDPerfInfo::LoopPerfInfo(AST_INDEX loopHdr, Boolean incl)
{
    if (!doIt) return NULL;
    return LoopCommPerfInfo((Dist_Globals*)NULL, loopHdr, incl);
    
    //int staticID = GetStaticID(loopHdr);
    //if (staticID == DUMMY_STATIC_ID)	// No perf info for this code object
    //	return (PerfMetrics*) NULL;
    //
    //return (incl)? repr.inclPerfTable->GetPerfInfo(staticID)
    //		 : repr.exclPerfTable->GetPerfInfo(staticID);
}
//--------------------------------------------------------------------------

PerfMetrics* FortDPerfInfo::MesgPerfInfo(AST_INDEX remoteRef)
{
    if (!doIt) return NULL;
    PerfMetrics *totalPerfMetrics, *sendPerfInfo, *recvPerfInfo;
    int staticID = GetStaticID(remoteRef); 
    if (staticID == DUMMY_STATIC_ID)
	return (PerfMetrics*) NULL;
    
    totalPerfMetrics = repr.mesgTotalPerfInfoTable->GetPerfInfo(staticID);
    if (totalPerfMetrics == (PerfMetrics*) NULL) {
	// Total for this message was not computed previously; compute and save
	sendPerfInfo = MesgSendPerfInfo(staticID);
	if (sendPerfInfo != (PerfMetrics*) NULL) {
	    totalPerfMetrics = new PerfMetrics(sendPerfInfo->NumProcs());
	    *totalPerfMetrics = *sendPerfInfo;
	    recvPerfInfo = MesgRecvPerfInfo(staticID);
	    if (recvPerfInfo != (PerfMetrics*) NULL) // reductions have no recv
		*totalPerfMetrics += *recvPerfInfo;
	    repr.mesgTotalPerfInfoTable->
	    			AddPerfInfoEntry(totalPerfMetrics, staticID);
	}
    }
    return totalPerfMetrics;
}
//--------------------------------------------------------------------------

SetOfNamedFractions& FortDPerfInfo::PgmTimeByComponent()
{
    SetOfNamedFractions* setNF = new SetOfNamedFractions(false, 2);
    if (!doIt) return *setNF;
    double pgmTime;
    PerfMetrics* pgmPerfInfo;
    
    if ((pgmPerfInfo = PgmPerfInfo()) != (PerfMetrics*) NULL) {
	setNF->SetTotal(pgmTime = pgmPerfInfo->TotalTime().Mean());
	setNF->AddToList(PgmPerfInfo()->BusyTime().Mean() / pgmTime, "Busy");
	setNF->AddToList(PgmPerfInfo()->IdleTime().Mean() / pgmTime, "Comm");
    }
    return *setNF;
}
//--------------------------------------------------------------------------

SetOfNamedFractions& FortDPerfInfo::PgmTimeByProcedure()
{
    SetOfNamedFractions* setNF = new SetOfNamedFractions(true);
    if (!doIt) return *setNF;
    double pgmTime;
    int procID;
    PerfMetrics *procPerfInfo, *pgmPerfInfo;

    if ((pgmPerfInfo = PgmPerfInfo()) != (PerfMetrics*) NULL) {
	setNF->SetTotal(pgmTime = pgmPerfInfo->TotalTime().Mean());
    
	while ((procID = repr.sidTable->ProcedureIDList()) !=DUMMY_STATIC_ID) {
	    procPerfInfo = repr.exclPerfTable->GetPerfInfo(procID);
	    if (procPerfInfo != NULL)
		setNF->AddToList(procPerfInfo->TotalTime().Mean() / pgmTime,
		 staticTable[procID]->getCString("Procedure Name").getValue());
	}
    }
    
    return *setNF;
}
//--------------------------------------------------------------------------

SetOfNamedFractions& FortDPerfInfo::PgmCommTimeByProcedure()
{
    SetOfNamedFractions* setNF = new SetOfNamedFractions(true);
    if (!doIt) return *setNF;
    double pgmTime;
    int procID;
    PerfMetrics *procPerfInfo, *pgmPerfInfo;

    if ((pgmPerfInfo = PgmPerfInfo()) != (PerfMetrics*) NULL) {
	setNF->SetTotal(pgmTime = PgmPerfInfo()->IdleTime().Mean());
    
	while ((procID = repr.sidTable->ProcedureIDList()) !=DUMMY_STATIC_ID) {
	    procPerfInfo = repr.exclPerfTable->GetPerfInfo(procID);
	    if (procPerfInfo != NULL)
		setNF->AddToList(procPerfInfo->IdleTime().Mean() / pgmTime,
		 staticTable[procID]->getCString("Procedure Name").getValue());
	}
    }
    
    return *setNF;
}
//--------------------------------------------------------------------------

SetOfNamedFractions& FortDPerfInfo::ProcedureTimeByComponent(AST_INDEX procStmt) {
    SetOfNamedFractions* setNF = new SetOfNamedFractions(false, 2);
    if (!doIt) return *setNF;
    PerfMetrics* procPerfInfo = ProcPerfInfo(procStmt, /*incl*/ false);
    
    if (procPerfInfo != (PerfMetrics*) NULL) {
	double procTime;
	setNF->SetTotal(procTime = procPerfInfo->TotalTime().Mean());
	setNF->AddToList(procPerfInfo->BusyTime().Mean() / procTime, "Busy");
	setNF->AddToList(procPerfInfo->IdleTime().Mean() / procTime, "Comm");
    }
    
    return *setNF;
}
//--------------------------------------------------------------------------

SetOfNamedFractions& FortDPerfInfo::ProcedureCommTimeByStmt(AST_INDEX /* procStmt */)
{
    SetOfNamedFractions* setNF = new SetOfNamedFractions(false);
    if (!doIt) return *setNF;
    return *setNF;
}
//--------------------------------------------------------------------------

SetOfNamedFractions& FortDPerfInfo::LoopTimeByComponent(AST_INDEX loopHdr)
{
    SetOfNamedFractions* setNF = new SetOfNamedFractions(false, 2);
    if (!doIt) return *setNF;
    PerfMetrics* loopPerfInfo = LoopPerfInfo(loopHdr, /*incl*/ false);
    
    if (loopPerfInfo != (PerfMetrics*) NULL) {
	double loopTime;
	setNF->SetTotal(loopTime = loopPerfInfo->TotalTime().Mean());
	setNF->AddToList(loopPerfInfo->BusyTime().Mean() / loopTime, "Busy");
	setNF->AddToList(loopPerfInfo->IdleTime().Mean() / loopTime, "Comm");
    }
    
    return *setNF;
}
//--------------------------------------------------------------------------

double FortDPerfInfo::LoopTimeAsFrnOfPgmTime(AST_INDEX loopHdr)
{
    assert(false); 		// Disabled until loop exec time available
    if (!doIt) return 0.0;
    PerfMetrics* loopMetrics = LoopPerfInfo(loopHdr, false /*incl*/);
    return (loopMetrics == (PerfMetrics*) NULL)? 0.0 :
	loopMetrics->TotalTime().Mean() / PgmPerfInfo()->TotalTime().Mean();
}
//--------------------------------------------------------------------------

double FortDPerfInfo::LoopCommTimeAsFrnOfPgmCommTime(Dist_Globals *dh,
						     AST_INDEX loopHdr)
{
    if (!doIt) return 0.0;
    PerfMetrics* loopMetrics = LoopCommPerfInfo(dh, loopHdr, false /*incl*/);
    return (loopMetrics == (PerfMetrics*) NULL)? 0.0 :
	loopMetrics->IdleTime().Mean() / PgmPerfInfo()->IdleTime().Mean();
}
//--------------------------------------------------------------------------

double FortDPerfInfo::MesgTimeAsFrnOfPgmTime(AST_INDEX remoteRef)
{
    if (!doIt) return 0.0;
    PerfMetrics* mesgMetrics = MesgPerfInfo(remoteRef);
    return (mesgMetrics == (PerfMetrics*) NULL)? 0.0 :
	mesgMetrics->TotalTime().Mean() / PgmPerfInfo()->TotalTime().Mean();
}
//--------------------------------------------------------------------------

double FortDPerfInfo::MesgTimeAsFrnOfPgmCommTime(AST_INDEX remoteRef)
{
    if (!doIt) return 0.0;
    PerfMetrics* mesgMetrics = MesgPerfInfo(remoteRef);
    return (mesgMetrics == (PerfMetrics*) NULL)? 0.0 :
	mesgMetrics->TotalTime().Mean() / PgmPerfInfo()->IdleTime().Mean();
}


//--------------------------------------------------------------------------
// Private member functions for class FortDPerfInfo
//--------------------------------------------------------------------------

// GetStaticID(AST_INDEX node)	:  Obtain the static ID for proc/loop/mesg
// 
// Get the line number and (for non-local ref corresponding to a mesg)
// character position information for the given AST node, and use that
// as index into StaticIDTable sidTable to obtain the static ID. 
//
// TEMPORARY special case for loops:
// Loops dont yet have static IDs, so just create an arbitrary numbering
// of them.  This can be used to index into a separate table for loop info.

int FortDPerfInfo::GetStaticID(AST_INDEX node)
{
    if (is_do(node))
	return GetLoopStaticID(node);
    
    int l1, l2, c1, c2;
    AST_INDEX nodeToLookup = (is_subscript(node))?
					gen_SUBSCRIPT_get_name(node) : node;
    ftt_NodeToText(repr.ftt, nodeToLookup, &l1, &c1, &l2, &c2);
    if (! is_subscript(node)) c1 = -1;	// For proc. and loop, ignore char pos
    
    return repr.sidTable->GetStaticID(repr.moduleFileName, l1, c1);
}

int FortDPerfInfo::GetLoopStaticID(AST_INDEX loopHdr)
{
    int oldIndex;
    AST_INDEX oldEntry;
    
    if ((oldIndex = repr.loopIndexTable->GetEntryIndex(&loopHdr))
		 == HASH_TABLE_EMPTY)
    {
	repr.loopIndexTable->AddEntry(&loopHdr, (AddEntryFunctPtr) NULL);
	oldIndex = repr.loopIndexTable->GetEntryIndex(&loopHdr);
    }
    else {
	oldEntry = *(AST_INDEX*)repr.loopIndexTable->GetEntryByIndex(oldIndex);
	assert(oldEntry == loopHdr); // Just a sanity check
    }
    assert(oldIndex != HASH_TABLE_EMPTY);
    return oldIndex;
}

// My corrected versions of the integer hash functions:
uint MyIntegerHashFunct(const void* const value, const uint size)
{ return IntegerHashFunct(*(int*)value, size); }

int MyIntegerEntryCompare(const void* const value1, const void* const value2)
{ return IntegerEntryCompare(*(int*)value1, *(int*)value2); }

//--------------------------------------------------------------------------

PerfMetrics* FortDPerfInfo::MesgSendPerfInfo(int staticID)
{
    return (staticID == DUMMY_STATIC_ID)? // No send perf info for this ref.
		NULL : repr.inclPerfTable->GetPerfInfo(staticID);
}    

//--------------------------------------------------------------------------

PerfMetrics* FortDPerfInfo::MesgRecvPerfInfo(int staticID)
{
    RecordDossier* sendDossier = staticTable[staticID];
    int recvID = sendDossier->getValue("Recv ID"); // the static ID we need
    return (recvID == DUMMY_STATIC_ID)? // No recv perf info for this ref.
		NULL : repr.inclPerfTable->GetPerfInfo(recvID);
}
//--------------------------------------------------------------------------

// Temporarily compute loop communication information indirectly by adding 
// the costs of all enclosed messages, rather than directly from traces.

PerfMetrics*
FortDPerfInfo::LoopCommPerfInfo(Dist_Globals *dh,
				AST_INDEX loopHdr,
				Boolean incl)
{
    if (!doIt || !is_do(loopHdr) || incl || dh == NULL)
	return (PerfMetrics*) NULL;
    
    int staticID = GetStaticID(loopHdr);
    PerfMetrics* loopInfo = repr.loopCommPerfInfoTable->GetPerfInfo(staticID);
    if (loopInfo != (PerfMetrics*) NULL)
	return loopInfo;
    
    // If you reach here, the perf. info for this loop has not been computed.
    loopInfo = new PerfMetrics(GnumNodes);
    loopInfo->ClearAllValues();
    
    // First: messages other than reductions: loop through all RSD sets.
    // RSD sets are stored in side-array for the loop stmt
    Rsd_set_info *rsd_loop_list;
    Rsd_set *rset;
    PerfMetrics *mesgInfo;
    
    rsd_loop_list = (Rsd_set_info *) get_info(dh->ped, loopHdr, type_dc);
    if (rsd_loop_list != (Rsd_set_info *) NO_DC_INFO) {
	/* loop through RSDs for all variables */
	for (int j = rsd_loop_list->num_ref - 1; j >= 0; j--) {
	    /* loop through all RSDs for one variable */
	    for (rset = rsd_loop_list->rsd_s[j]; rset; rset = rset->rsd_next) {
		/* do only any one subscripted reference for that RSD */
		assert(rset->num_subs > 0);
		mesgInfo = MesgPerfInfo(rset->subs[0]);
		if (mesgInfo != (PerfMetrics*) NULL)
		    *loopInfo += *mesgInfo; // only numInvocations wrong
	    }
	}
    }
    
    // Then: reductions. Walk the AST looking for reductions (in side-array
    // for assignment stmts) just as the message generation code does.
    // When found, the reduc_set identifies the loop with which the reduction
    // is associated.  The communication cost should be assigned to that loop.
    
    AstIterator tree_walk(loopHdr, PreOrder, AST_ITER_STMTS_ONLY);
    AST_INDEX stmt;
    Iter_set *iset;
    for (; (stmt = tree_walk.Current()) != AST_NIL; ++tree_walk)
	if (is_assignment(stmt)) {
	    iset = (Iter_set *) get_info(dh->ped, stmt, type_dc);
	    if (iset != (Iter_set *) NO_DC_INFO  	  // info available
		&& iset->reduc_set != (Reduc_set*) NULL   // its a reduction
		&& ! iset->reduc_set->local		  // not owner-computes
		&& iset->reduc_set->loop == loopHdr)	  // "on" this loop
	    {
		mesgInfo = MesgPerfInfo(iset->reduc_set->lhs);
		if (mesgInfo != (PerfMetrics*) NULL)
		    *loopInfo += *mesgInfo; // only numInvocations wrong
	    }
	}
    
    // Finally, reset busyTime and numInvocations, which would have got
    // set to the sum of numInvocations for all the messages, but the real
    // values are unknown until loop SDDF records are supported
    loopInfo->TotalTime().SetZero(loopInfo->NumProcs());
    loopInfo->NumInvocations().SetZero(loopInfo->NumProcs());
    
    // And store the object ptr away for future use
    repr.loopCommPerfInfoTable->AddPerfInfoEntry(loopInfo, staticID);
    
    return loopInfo;
}


//**************************************************************************
// EXTERNAL FUNCTION FOR TESTING
//**************************************************************************

void PrintProc(AST_INDEX stmt)
{
    AST_INDEX unitNameAST;
    if (is_program(stmt)) unitNameAST = gen_PROGRAM_get_name(stmt);
    else if (is_subroutine(stmt)) unitNameAST = gen_SUBROUTINE_get_name(stmt);
    else if (is_function(stmt)) unitNameAST = gen_FUNCTION_get_name(stmt);
    else { assert(false); }

    printf("\nPROGRAM %s\n", gen_get_text(unitNameAST));
}

void PrintLoop(AST_INDEX stmt)
{ printf("\nAGGREGATE COMMUNICATION COST FOR LOOP:\n\n"); }

void PrintRefInLine(AST_INDEX subscriptedRef)
{
    AST_INDEX stmt = subscriptedRef;
    while (!is_statement(stmt))
	stmt = tree_out(stmt);
    printf("\nMESSAGE FOR non-local ref: %s\n", gen_get_text(subscriptedRef));
    printf("	    in stmt: %s\n", gen_get_text(stmt));
}

void printPerfStatistics(Dist_Globals* dh, Generic PerfInfoObjectP)
{
    FortDPerfInfo* perfInfo = (FortDPerfInfo*) PerfInfoObjectP;
    if (!perfInfo->DoIt()) return;
    
    printf("\tTEST PERFORMANCE STATISTICS INTERFACE: FILE %s\n", perfInfo->GetModuleFileName());
    
    AstIterator tree_walk(dh->root, PreOrder, AST_ITER_STMTS_ONLY);
    AST_INDEX stmt;
    PerfMetrics *metrics;
    
    // For procedures and loops, loop through all stmts:
    for (; (stmt = tree_walk.Current()) != AST_NIL; ++tree_walk) {
	if (is_f77_subprogram_stmt(stmt)) {
	    PrintProc(stmt);
	    
	    printf("\nProcedure Time By Component:\n");
	    perfInfo->ProcedureTimeByComponent(stmt).Print();
	    
	    printf("\nInclusive Metrics:\n");
	    metrics = perfInfo->ProcPerfInfo(stmt, true );
	    if (metrics != (PerfMetrics*) NULL) metrics->Print();
	    
	    printf("\nExclusive Metrics:\n");
	    metrics = perfInfo->ProcPerfInfo(stmt, false);
	    if (metrics != (PerfMetrics*) NULL) metrics->Print();
	}
	else if (is_do(stmt)) {
	    metrics = perfInfo->LoopPerfInfo(stmt, false /*incl*/);
	    if (metrics != (PerfMetrics*) NULL) {
		PrintLoop(stmt);
		metrics->Print();
	    }
	}
    }
    
    // For messages (except reductions), loop through all RSD sets
    // RSD sets are stored in side-array for loops stmts
    Rsd_set_info *rsd_loop_list;
    Rsd_set *rset;
    AST_INDEX loop;
    
    for (int i = dh->numdoloops - 1; i >= 0; i--) {
	loop = dh->doloops[i];
	
	metrics = perfInfo->LoopCommPerfInfo(dh, loop, false);
	if (metrics != (PerfMetrics*) NULL) {
	    PrintLoop(loop);
	    printf("LoopCommTimeAsFrnOfPgmCommTime = %g\n\n",
		   perfInfo->LoopCommTimeAsFrnOfPgmCommTime(dh, loop));
	    metrics->Print();
	}
	else printf("No communication performance info available for loop\n");
	
	rsd_loop_list = (Rsd_set_info *) get_info(dh->ped, loop, type_dc);
	if (rsd_loop_list != (Rsd_set_info *) NO_DC_INFO) {
	    /* loop through RSDs for all variables */
	    for (int j = rsd_loop_list->num_ref - 1; j >= 0; j--) {
		/* loop through all RSDs for one variable */
		for (rset = rsd_loop_list->rsd_s[j]; rset;
		     				rset = rset->rsd_next) {
		    for (int k = rset->num_subs - 1; k >= 0; k--) {
			PrintRefInLine(rset->subs[k]);
			metrics = perfInfo->MesgPerfInfo(rset->subs[k]);
			if (metrics != (PerfMetrics*) NULL) {
			    printf("Message time / Program time : %f\n\n", perfInfo->MesgTimeAsFrnOfPgmTime(rset->subs[k]));
			    printf("Message time / Program comm time : %f\n\n", perfInfo->MesgTimeAsFrnOfPgmCommTime(rset->subs[k]));
			    printf("Message costs:\n");
			    metrics->Print();
			}
			else printf("No perf. info available for message\n");
		    }
		}
	    }
	}
    }
    
    // Walk the AST looking for reductions (in side-array for assignment stmts)
    Iter_set *iset;
    tree_walk.Reset();
    for (; (stmt = tree_walk.Current()) != AST_NIL; ++tree_walk)
	if (is_assignment(stmt)) {
	    iset = (Iter_set *) get_info(dh->ped, stmt, type_dc);
	    if (iset != (Iter_set *) NO_DC_INFO  	  // info available
		&& iset->reduc_set != (Reduc_set*) NULL   // its a reduction
		&& ! iset->reduc_set->local)		  // not owner-computes
	    {
		// Dont print loop info: it gets printed above.
		PrintRefInLine(iset->reduc_set->lhs);
		metrics = perfInfo->MesgPerfInfo(iset->reduc_set->lhs);
		if (metrics != (PerfMetrics*) NULL) {
		    printf("Message time / Program time : %f\n\n", perfInfo->MesgTimeAsFrnOfPgmTime(iset->reduc_set->lhs));
		    printf("Message time / Program comm time : %f\n\n", perfInfo->MesgTimeAsFrnOfPgmCommTime(iset->reduc_set->lhs));
		    printf("Message (reduction) costs:\n");
		    metrics->Print();
		}
		else printf("No perf. info available for reduction\n");
	    }
	}
}


//--------------------------------------------------------------------------
