/* $Id: FortDPerfTable.C,v 1.2 2001/09/17 00:12:21 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// $Id: FortDPerfTable.C,v 1.2 2001/09/17 00:12:21 carr Exp $
//**************************************************************************
// Implementation of class FortDPerfTable:
//**************************************************************************


//--------------------------------------------------------------------------
// INCLUDE FILES
//--------------------------------------------------------------------------

#include <assert.h>

#ifndef FDCombine6_h
#include <libs/fortD/performance/stats/FDCombine6.h>
#endif

#ifndef FortDPerfTable
#include <libs/fortD/performance/stats/FortDPerfTable.h>
#endif

#ifndef SD_MsgInfo_h                    // For symbolic names of message types
#include <libs/fortD/performance/staticInfo/SD_MsgInfo.h>
#endif


// OVERFLOW is #defined to 3 in math.h (which is in FDCombine6.h) but 
// I cannot move PerfInfo.h after the fortran D header files because
// of another such #define problem in the reverse direction. So:
// #undef OVERFLOW

// #ifndef groups_h			// From the Fortran D world
// #include <include/fort/groups.h>
// #endif
// #ifndef AstIterators_h
// #include <include/fort/AstIterators.h>
// #endif
// #ifndef FD_CODEGEN
// #include <libs/fort_d/dc/private_dc.h>
// #endif


//--------------------------------------------------------------------------
// EXTERNAL VARIABLES AND FUNCTIONS
//--------------------------------------------------------------------------



//**************************************************************************
// MEMBER FUNCTION DEFINITIONS FOR class FortDPerfTable
//**************************************************************************

FortDPerfTable::FortDPerfTable(Boolean _incl) : incl(_incl)
{
    perfInfoTableSize = TABLE_INIT_SIZE;
    perfInfoTable = new PerfMetrics*[perfInfoTableSize];
    for (int i=0; i < perfInfoTableSize; i++)
	perfInfoTable[i] = (PerfMetrics*) NULL;
}
FortDPerfTable::~FortDPerfTable()
{
    for (int i=0; i < perfInfoTableSize; i++)
	if (perfInfoTable[i] != (PerfMetrics*) NULL) // save call to delete
	    delete perfInfoTable[i];
    delete[] perfInfoTable;
}

void FortDPerfTable::ConstructPerfTable(RecordDossier** staticTable,
					FileCell* lineHeader,
					LineStatCell* lineStatTable)
{
    PerfMetrics	*perfInfo, *perfInfoForCurProc;
    int		staticID, sendWaitID, recvWaitID;
    FileCell*	currentFile;
    ProcCell*	currentProc;
    LineCell*	currentLine;
    LineStatCell* lineStatCell;
    RecordDossier* mesgDossier;

    for (currentFile = lineHeader;
	 currentFile != (FileCell *) NULL;
	 currentFile = currentFile->getNext())
    {
	for (currentProc = currentFile->getDown();
	     currentProc != (ProcCell *) NULL;
	     currentProc = currentProc->getNext())
	{
	    // First, get the individual perf stats for loops and messages
	    // within this procedure, and incl. and excl. costs for this proc
	    
	    for (currentLine = currentProc->getDown();
		 currentLine != (LineCell *) NULL;
		 currentLine = currentLine->getNext())
	    {
		staticID = currentLine->getStaticID();
		lineStatCell = &lineStatTable[staticID];
		switch (lineStatCell->getFamily()) {
		  case FDPROC:
		    perfInfo = GetProcPerfInfo(lineStatCell, staticID); break;
		  case FDLOOP:
		    perfInfo = GetLoopPerfInfo(lineStatCell, staticID); break;
		  case FDMESG:
		    perfInfo = GetMesgPerfInfo(lineStatCell, staticID); break;
		  default: assert(false); break;
		}
		AddPerfInfoEntry(perfInfo, staticID);
		
		// Remember perf info entry for current procedure
		if (lineStatCell->getFamily() == FDPROC)
		    perfInfoForCurProc = perfInfo;
	    }
	    
	    // Then, loop again to compute:
	    // (1) send and recv costs separately for current procedure
	    //     (only possible for exclusive costs, since procedure call
	    //     instance specific metrics not available.)
	    // (2) total send cost and total recv cost (i.e., including
	    //     msgwait() if any in each case); these totals are then
	    //     stored under the Send ID and Recv ID respectively.
	    
	    if (!incl) {
		int numProcs = perfInfoForCurProc->NumProcs();
		perfInfoForCurProc->SendOvhdIdleTime().SetZero(numProcs);
		perfInfoForCurProc->RecvOvhdIdleTime().SetZero(numProcs);
		
		for (currentLine = currentProc->getDown();
		     currentLine != (LineCell *) NULL;
		     currentLine = currentLine->getNext())
		{
		    staticID = currentLine->getStaticID();
		    lineStatCell = &lineStatTable[staticID];
		    if (lineStatCell->getFamily() == FDMESG) {
			mesgDossier = staticTable[staticID];
			char* nameString =
			    (char*)mesgDossier->getName().getValue();

			// First, add message volume sent in this procedure:
			if (!strcmp(nameString, "FDStat Mesg Send"))
			    perfInfoForCurProc->AddTotalMesgVolume(
				GetPerfInfo(staticID)->TotalMesgVolume());

			// Then, add send/recv cost for this procedure:
			if (!strcmp(nameString, "FDStat Mesg Send") ||
			    !strcmp(nameString, "FDStat Mesg Send Wait"))
			{
			    perfInfoForCurProc->AddSendOvhdIdleTime(
				GetPerfInfo(staticID)->IdleTime());
			}
			else if (!strcmp(nameString, "FDStat Mesg Recv") ||
				 !strcmp(nameString, "FDStat Mesg Recv Wait"))
			{
			    perfInfoForCurProc->AddRecvOvhdIdleTime(
				GetPerfInfo(staticID)->IdleTime());
			}
			else {assert(false);} // Should be a mesg static record

			// Then incorporate send wait cost into Send msgwait()
			// and recv msgwait() cost into Recv:
			if (!strcmp(nameString, "FDStat Mesg Send")) {
			    sendWaitID = mesgDossier->getValue("Send Wait ID");
			    if (sendWaitID != DUMMY_STATIC_ID)
				*GetPerfInfo(staticID) += *GetPerfInfo(sendWaitID);
			}
			else if (!strcmp(nameString, "FDStat Mesg Recv")) {
			    recvWaitID = mesgDossier->getValue("Recv Wait ID");
			    if (recvWaitID != DUMMY_STATIC_ID)
				*GetPerfInfo(staticID) += *GetPerfInfo(recvWaitID);
			}
		    }
		}
	    }
	}
    }
}

void FortDPerfTable::AddPerfInfoEntry(PerfMetrics* perfEntry, int staticID)
{
    if (staticID >= perfInfoTableSize) {	// Must grow table
	int newPerfInfoTableSize = staticID + TABLE_INIT_SIZE;
	PerfMetrics** tmpPerfInfoTable =new PerfMetrics*[newPerfInfoTableSize];
	int i;
	for (i = 0; i < perfInfoTableSize; i++)
	    tmpPerfInfoTable[i] = perfInfoTable[i];
	for ( ; i < newPerfInfoTableSize; i++)
	    tmpPerfInfoTable[i] = (PerfMetrics*) NULL;
	
	delete[] perfInfoTable;
	perfInfoTable = tmpPerfInfoTable;
	perfInfoTableSize = newPerfInfoTableSize;
    }
    assert (perfInfoTable[staticID] == (PerfMetrics*) NULL);
    perfInfoTable[staticID] = perfEntry;
}

PerfMetrics* FortDPerfTable::GetProcPerfInfo(LineStatCell* procStatCell,
					     int /* staticID */)
{
    int i;
    PerfMetrics* perfInfo = new PerfMetrics(GnumNodes);
    PerfStats* perfStats;
    TimeStr timeStr;

    // Compute numInvocations
    perfStats = &perfInfo->NumInvocations();
    perfStats->SetSize(GnumNodes);
    for (i = 0; i < GnumNodes; i++)
	perfStats->SetValueOnProcessor(i, procStatCell->getCount(i));
    perfStats->ComputeStats();
    
    // Compute total time
    perfStats = &perfInfo->TotalTime();
    perfStats->SetSize(GnumNodes);
    for (i = 0; i < GnumNodes; i++) {
	timeStr = procStatCell->getTime(i);
	perfStats->SetValueOnProcessor(i, ((incl)? timeStr.procInclude
					         : timeStr.procExclude));
    }
    perfStats->ComputeStats();
    
    // Compute idle time = message time
    perfStats = &perfInfo->IdleTime();
    perfStats->SetSize(GnumNodes);
    for (i = 0; i < GnumNodes; i++) {
	timeStr = procStatCell->getTime(i);
	perfStats->SetValueOnProcessor(i, ((incl)? timeStr.mesgInclude
					         : timeStr.mesgExclude));
    }
    perfStats->ComputeStats();
    
    // Compute busy time = total time - idle time
    perfInfo->BusyTime() = perfInfo->TotalTime() - perfInfo->IdleTime();
    
    return perfInfo;
}

PerfMetrics* FortDPerfTable::GetLoopPerfInfo(LineStatCell* loopStatCell,
					     int /* staticID */)
{
    int i;
    PerfMetrics* perfInfo = new PerfMetrics(GnumNodes);
    PerfStats* perfStats;
    TimeStr timeStr;

    // Compute numInvocations
    perfStats = &perfInfo->NumInvocations();
    perfStats->SetSize(GnumNodes);
    for (i = 0; i < GnumNodes; i++)
	perfStats->SetValueOnProcessor(i, loopStatCell->getCount(i));
    perfStats->ComputeStats();
    
    if (incl) {				// No exclusive loop times available.
	// Compute total time.
	perfStats = &perfInfo->TotalTime();
	perfStats->SetSize(GnumNodes);
	for (i = 0; i < GnumNodes; i++) {
	    timeStr = loopStatCell->getTime(i);
	    perfStats->SetValueOnProcessor(i, ((incl)? timeStr.mesgInclude
					             : timeStr.mesgExclude));;
	}
	perfStats->ComputeStats();

	// No message times available for loops, yet.
	// // Compute idle time = message time
	// perfStats = &perfInfo->IdleTime();
	// perfStats->SetSize(GnumNodes);
	// for (i = 0; i < GnumNodes; i++) {
	//     timeStr = loopStatCell->getTime(i);
	//     perfStats->SetValueOnProcessor(i, ((incl)?timeStr.mesgInclude
	//     				                :timeStr.mesgExclude));
	// }
	// perfStats->ComputeStats();
	//
	// // Compute busy time = total time - idle time
	// perfInfo->BusyTime() = perfInfo->TotalTime() - perfInfo->IdleTime();
    }
    
    return perfInfo;
}

PerfMetrics* FortDPerfTable::GetMesgPerfInfo(LineStatCell* mesgStatCell,
					     int staticID)
{
    int i, mesgOperType, numBytesSent;
    PerfMetrics* perfInfo = new PerfMetrics(GnumNodes);
    PerfStats* perfStats;
    TimeStr timeStr;
    RecordDossier* mesgDossier;
    
    mesgDossier = staticTable[staticID];
    char* nameString = (char*) mesgDossier->getName().getValue();
    
    // Compute numInvocations 
    perfStats = &perfInfo->NumInvocations();
    perfStats->SetSize(GnumNodes);
    for (i = 0; i < GnumNodes; i++)
	perfStats->SetValueOnProcessor(i, mesgStatCell->getCount(i));
    perfStats->ComputeStats();
    
    // Compute total #bytes *sent*, i.e., ignore receives
    // Broadcasts are counted as P-1 messages, all on one processor.
    // Global reductions are counted as 2*(P-1) messages, counted as
    // 2*(P-1) / P ~= 2 messages per processor.
    if (!strcmp(nameString, "FDStat Mesg Send")) {
	int factorPerProc;
	perfStats = &perfInfo->TotalMesgVolume();
	perfStats->SetSize(GnumNodes);
	
	mesgOperType = mesgDossier->getValue("Mesg Oper Type");
	if (mesgOperType == MESSAGESEND_GLOBAL_BCAST)
	    factorPerProc = GnumNodes - 1; // ok to use for all procs!
	else if (mesgOperType == MESSAGESEND_GLOBAL_SUM   ||
		 mesgOperType == MESSAGESEND_GLOBAL_PROD  ||
		 mesgOperType == MESSAGESEND_GLOBAL_MIN   ||
		 mesgOperType == MESSAGESEND_GLOBAL_MAX)
	    factorPerProc = 2;
	else
	    factorPerProc = 1;
	
	for (i = 0; i < GnumNodes; i++) {
	    numBytesSent = mesgStatCell->getSize(i);
	    perfStats->SetValueOnProcessor(i,  factorPerProc * numBytesSent);
	}
	
	perfStats->ComputeStats();
    }
    
    // Compute idleTime. totalTime is the same as idleTime. busyTime is 0.
    perfStats = &perfInfo->IdleTime();
    perfStats->SetSize(GnumNodes);
    for (i = 0; i < GnumNodes; i++) {
	timeStr = mesgStatCell->getTime(i);
	perfStats->SetValueOnProcessor(i, timeStr.procInclude);	// mesg time is
    }						         // not in mesgInclude!
    perfStats->ComputeStats();
    
    perfInfo->SetTotalTime(perfInfo->IdleTime());
    perfInfo->BusyTime().SetZero(GnumNodes);
    
    // idleTime is due to either send overhead or recv overhead
    
    if (!strcmp(nameString, "FDStat Mesg Send") ||
	!strcmp(nameString, "FDStat Mesg Send Wait"))
    {
	perfInfo->SetSendOvhdIdleTime(perfInfo->IdleTime());
	perfInfo->RecvOvhdIdleTime().SetZero(GnumNodes);
    }
    else if (!strcmp(nameString, "FDStat Mesg Recv") ||
	     !strcmp(nameString, "FDStat Mesg Recv Wait"))
    {
	perfInfo->SetRecvOvhdIdleTime(perfInfo->IdleTime());
	perfInfo->SendOvhdIdleTime().SetZero(GnumNodes);
    }
    else { assert(false); }		// Should be some mesg static record

    return perfInfo;
}

//**************************************************************************

