/* $Id: SDDF_SetupCleanup.C,v 1.1 1997/03/11 14:28:58 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

// -*- C++ -*- 
/* Contains the C code for the basic interface routines for fortran D */ 
/* $Header: /home/cs/carr/cvsroot/DSystem/src/libs/fortD/performance/staticInfo/SDDF_SetupCleanup.C,v 1.1 1997/03/11 14:28:58 carr Exp $
//
*/

static const char * RCS_ID = "$Id: SDDF_SetupCleanup.C,v 1.1 1997/03/11 14:28:58 carr Exp $";
#define ASSERT_FILE_VERSION RCS_ID
#define MKASSERT

// Do includes for this code
// Include corresponding .h file last. In general, if these routines
// are called externally, there should be a .h file.

#include <stream.h>

#include <Attributes.h>
#include <OutputFileStreamPipe.h>
#include <PipeReader.h>
#include <PipeWriter.h>
#include <RecordDossier.h>
#include <StructureDescriptor.h>
 

#include <libs/fileAttrMgmt/attributedFile/AttributedFile.h>

#include <libs/fortD/performance/staticInfo/SDDF_General.h>
#include <libs/fortD/performance/staticInfo/ArrayNameInfo.h>
#include <libs/fortD/performance/staticInfo/SD_Map.h>
#include <libs/fortD/performance/staticInfo/SD_Base.h>
#include <libs/fortD/performance/staticInfo/SD_SrcInfo.h>
#include <libs/fortD/performance/staticInfo/SD_DataInfo.h>
#include <libs/fortD/performance/staticInfo/SD_MsgInfo.h>
#include <libs/fortD/performance/staticInfo/SDDF_IO.h>
#include <libs/fortD/performance/staticInfo/utility.h>
//#include "VPDlist.h"
#include <libs/fortD/performance/staticInfo/MkAssert.h>

#include <libs/fortD/performance/staticInfo/SDDF_Instrumentation.h>
#include <libs/fortD/performance/staticInfo/FD_ProcEntry.h>
#include <libs/fortD/performance/instr/InstrumentSPMD.h>

// Def. of class FD_ProcEntry
// #ifndef fort_d_driver_h		// since driver.h cannot be included
// #include <include/fort_d/driver.h>	// after Pablo include files.
// #endif


//**************************************************************************
// Private inline functions and forward declarations.
//**************************************************************************

STATIC(void, CreateProcRecord, (AST_INDEX root, char * procName, char * fileName));
		 

//**************************************************************************
// The root of all evil.
//**************************************************************************

PabloGlobalInfo thePabloGlobalInfo;


//**************************************************************************
// Functions to initialize and cleanup our data structures.
// These functions called from the Fortran D compiler code-gen phase
// The EXTERN declartions of these functions are in SDDF_Instrumentation.h
//**************************************************************************

void SD_Build_Tree_Annot(Context modContext,	 // context used for file name
			 char* procName,	 // name of the procedure
			 AST_INDEX root,	 // root of the proceudre AST
			 FortTextTree ftt,	 // fort text tree and fort
			 FortTree ft)		 // tree for line# info
{
  if (! thePabloGlobalInfo.wantPabloInfo) return;
  
  // Initialize local info
  PabloLocalInfo * myLocals = new PabloLocalInfo;
  thePabloGlobalInfo.currentLocals = myLocals;
  
  myLocals->SetProcName(procName);           // Procedure name
  
  // TEMPORARY FIX for inconsistency between old and new repository file names:
  // Set file name as the tail component of the path name.
  char* startchar = strrchr(modContext->ReferenceFilePathName(), (int)'/');
  if (startchar == (char*) NULL)	// i.e., no '/' in path name
      startchar = (char*)modContext->ReferenceFilePathName();
  else
      startchar++;			// char following last '/'
  myLocals->SetFileName(startchar); // File name
  
  // Build line number map and assoc info
  myLocals->InitSideArray(root,ft,ftt);
  // myLocals->sideArray->Dump();

  // Create the SDDF static record for this procedure
  CreateProcRecord(root,procName, myLocals->moduleFileName);

  // need to build the symboltable.
  thePabloGlobalInfo.arrayInfo->SetupForNewProcedure(ft, procName);
}

			 
///////////////////////////////////////////////////////////////////////////
// CreateProcRecord -- Create the SDDF static record for this procedure
// Private to this file.
///////////////////////////////////////////////////////////////////////////

static void CreateProcRecord(AST_INDEX root, char * procName, char * fileName)
{
    SDDF_ProcInfo* procInfo = new SDDF_ProcInfo;
    int lineNum = thePabloGlobalInfo.currentLocals->sideArray->getLine(root);
		
    procInfo->SetPosition(lineNum, 0);
    procInfo->SetProcName(procName);
    procInfo->SetFileName(fileName);
    procInfo->SetRecursiveFlag(false);

    thePabloGlobalInfo.currentLocals->SetProcInfo(procInfo);
}
			 
///////////////////////////////////////////////////////////////////////////
// SD_InitLocalAndGatherInfo - Not used right now. Most function moved
// to SD_Build_Tree_Annot
///////////////////////////////////////////////////////////////////////////
void  SD_InitLocalAndGatherInfo( FortTextTree, FortTree) {
  if (! thePabloGlobalInfo.wantPabloInfo) return;
  
  // Right now keep things simple, and work from there
  PabloLocalInfo * myLocals = thePabloGlobalInfo.currentLocals;
// myLocals->sideArray->Dump();
}

///////////////////////////////////////////////////////////////////////////
// Cleanup the instrumentation for this procedure, if any
///////////////////////////////////////////////////////////////////////////

void SD_CleanupInstr(Dist_Globals* dh)
{
    SPMDInstrumentation* instr = thePabloGlobalInfo.instr;
    
    if (! thePabloGlobalInfo.wantPabloInfo) {
	MkAssert(! instr->InstrumentationOn(),
		 "SDDF info not generated though instrumentation requested?\n",
		 EXIT); 
	return;
    }
    
    // For now, static info is printed only when instrumentation is on,
    // but later make static info available separately.  For now, ensure 
    // instrumentation was requested, call the instr wrapup routines.
    
    MkAssert(instr->InstrumentationOn(),
	     "Trying to cleanup instrumentation though it wasn't requested!\n",
	     EXIT);
    
    // Instrument procedure here because dh is not accessible elsewhere
    // in the SDDF setup and cleanup routines
    if (instr->GetInstrOpts()->InstrProcedures()) {
	SDDF_ProcInfo *procInfo =
	    thePabloGlobalInfo.currentLocals->GetProcInfo();
	int procId = procInfo->GetId();
	(void) instr->instrumentProcedureEntryExit(dh,dh->root,procId,procId);
    }
    
    // Similarly, instrument program execution time here:
    if (instr->GetInstrOpts()->InstrPgmExecTime())
	(void) instr->measureProgramTime(dh, dh->root);
    
    // instrumentation wrapup for the current procedure
    (void) instr->wrapupProgramUnit(dh);
}

///////////////////////////////////////////////////////////////////////////
// Cleanup the local info
///////////////////////////////////////////////////////////////////////////
void SD_CleanupLocal() {
  if (! thePabloGlobalInfo.wantPabloInfo) return;
  
  delete thePabloGlobalInfo.currentLocals;
  thePabloGlobalInfo.currentLocals = 0;
}

///////////////////////////////////////////////////////////////////////////
// Does all the global initializations.
///////////////////////////////////////////////////////////////////////////
void SD_InitialSetup(SPMDInstrumentation *instr) {
  // Compute and print static info only if some instrumentation is requested.
  thePabloGlobalInfo.instr = instr; // but keep this pointer in any case
  if (instr->InstrumentationOn()) {
      thePabloGlobalInfo.wantPabloInfo = true;
      thePabloGlobalInfo.arrayInfo = new ArrayNameInfo;
      thePabloGlobalInfo.currentLocals = 0;
  }
}

///////////////////////////////////////////////////////////////////////////
// Kick out all the sddf records and clean up
///////////////////////////////////////////////////////////////////////////
void SD_FinalCleanupAndOutput() {
    if (! thePabloGlobalInfo.wantPabloInfo) {
	return;
    }
    // Else do cleanup...
    
    // Do final work with arrays
    SD_ProcessArrays();

    // Initialize the output pipe. Someday we should set a filename for
    // this, but not right now 

    const int SDDF_OUTPIPE_BUFFER_SIZE = 16;
    OutputFileStreamPipe * outFile = 
	new OutputFileStreamPipe("FortD_SDDF.A", SDDF_OUTPIPE_BUFFER_SIZE);
  
    PipeWriter * p = SD_InitIo(outFile);
  
    // walk that registry and kick everything out
    StaticDescriptorBase::WalkRegistry(*p);
  
    Attributes attributes;
    // Close off output
    attributes.clearEntries();
    attributes.insert("information","end of data");
    p->putAttributes(attributes);
  
    //  p->close();
    //  delete p;
    delete outFile;
}


