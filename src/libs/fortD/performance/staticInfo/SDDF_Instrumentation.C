/* $Id: SDDF_Instrumentation.C,v 1.2 1997/03/27 20:34:10 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// -*- C++ -*- xterm -geometry 80x40+495+0 -name "`hostname`_2" &

/* Contains the C code for the basic interface routines for fortran D */ 
/* $Header: /home/cs/carr/cvsroot/DSystem/src/libs/fortD/performance/staticInfo/SDDF_Instrumentation.C,v 1.2 1997/03/27 20:34:10 carr Exp $
//
*/

static const char * RCS_ID = "$Id: SDDF_Instrumentation.C,v 1.2 1997/03/27 20:34:10 carr Exp $";
#define ASSERT_FILE_VERSION RCS_ID
#define MKASSERT

// Do includes for this code
// Include corresponding .h file last. In general, if these routines
// are called externally, there should be a .h file.

#include <stream.h>

#include <libs/fortD/performance/staticInfo/SD_Map.h>
#include <libs/fortD/performance/staticInfo/SD_Base.h>
#include <libs/fortD/performance/staticInfo/SD_DataInfo.h>
#include <libs/fortD/performance/staticInfo/SD_MsgInfo.h>
#include <libs/fortD/performance/staticInfo/SD_SrcInfo.h>
#include <libs/fortD/performance/staticInfo/SDDF_IO.h>
#include <libs/fortD/performance/staticInfo/utility.h>
//#include "VPDlist.h"
#include <libs/fortD/performance/staticInfo/MkAssert.h>

#include <libs/fortD/misc/fd_types.h>
#include <libs/fortD/driver/driver.h>
#include <libs/fortD/performance/staticInfo/SDDF_General.h>
#include <libs/fortD/performance/staticInfo/ArrayNameInfo.h>

#include <PipeWriter.h>
#include <OutputFileStreamPipe.h>

#include <libs/fortD/performance/staticInfo/SDDF_Instrumentation.h>
#include <libs/fortD/performance/staticInfo/FD_ProcEntry.h>

///////////////////////////////////////////////////////////////////////////
// This gathers info that fortran d has collected for decomps.
///////////////////////////////////////////////////////////////////////////
void SD_Get_Decomp_Info(FortTree /*ft*/,AST_INDEX cur, 
			FortranDHashTableEntry * htEnt) {
  
  if (! thePabloGlobalInfo.wantPabloInfo) return;
  
  PabloLocalInfo * myLocals = thePabloGlobalInfo.currentLocals;

  // These routines are called twice, once before whole program
  // analysis, once after. myLocals is nil the first time, which is
  // when we do not want to execute. Relys on this behavior.
  if (myLocals ==0) return;

  // Create a static decomp record, and store it into the structure:
  SDDF_DecompInfo * decomp = new SDDF_DecompInfo();

  // grab node for side array:
  PabloSideArrayInfo * sideNode =   myLocals->sideArray->getInfo(cur);
  sideNode->SetSDDFInfo(decomp, SDDF_DECOMPINFO, htEnt);

  // Figure out line number and procedure name info
  int lineNo = myLocals ->sideArray->getLine(cur);
  SDDF_ProcInfo * proc = myLocals->GetProcInfo();
  decomp->SetPosition(lineNo,proc);

  // Stuff it with what we know
  decomp->SetName(htEnt->name());
  decomp->SetDimension(htEnt->numdim);
  decomp->SetDistributedFlag(false);

  // I cannot do aligns and dists yet.

}

void SD_Get_Distrib_Info(AST_INDEX cur, 
			FortranDHashTableEntry * htEnt) {
  
  if (! thePabloGlobalInfo.wantPabloInfo) return;
  
  PabloLocalInfo * myLocals = thePabloGlobalInfo.currentLocals;

  // These routines are called twice, once before whole program
  // analysis, once after. myLocals is nil the first time, which is
  // when we do not want to execute. Relys on this behavior.
  if (myLocals ==0) return;

  // Create a static distribution record, and store it into the structure:
  SDDF_DistInfo * dist = new SDDF_DistInfo;

  // Create new node for side array:
  PabloSideArrayInfo * sideNode =   myLocals->sideArray->getInfo(cur);
  sideNode->SetSDDFInfo(dist, SDDF_DISTINFO, htEnt);


  // Figure out line number and procedure name info
  int lineNo = myLocals ->sideArray->getLine(cur);
  SDDF_ProcInfo * proc = myLocals->GetProcInfo();
  dist->SetPosition(lineNo,proc);

  // Add info to procedure record:
  proc->AddDist(dist);

  // Discover the corresponding decomp statement
  AST_INDEX decompNode = ft_NumberToNode(myLocals->curFt,
					 htEnt->d->decomp_id_number);

  // Get side array for decomp, with check
  PabloSideArrayInfo * decompInfo =
    myLocals->sideArray->getInfo(decompNode);
  MkAssert(decompInfo!=0,"No Side array for Node!",EXIT);

  SDDF_DecompInfo * decomp;
  decomp = (SDDF_DecompInfo *) decompInfo->GetSafe(SDDF_DECOMPINFO);
  // Check to see if we actually got a decomp back
  MkAssert(decomp!=0,"No decomp stored at this node!",EXIT);

  // Make cross pointers
  dist->SetDecomp(decomp);
  decomp->AddDist(dist);

  // Leave the rest for another day.
}

//===========================================================================
// Align processing
//===========================================================================

/////////////////////////////////////////////////////////////////////////////
// Start squirming through array dimemsions and sort out record data
// Right now, we are really stupid and only understand perfect alignments. 
// Otherwise, we record unknown.
/////////////////////////////////////////////////////////////////////////////
void SD_AddAlignTypeInfo(SDDF_AlignInfo * align, 
			 FortranDHashTableEntry *arrayHt) {
  
  MkAssert(thePabloGlobalInfo.wantPabloInfo, "SD_AddAlignTypeInfo: "
	   "Called without first calling SD_InitialSetup?", EXIT);
	   
  int Dim = arrayHt->numdim;
  
  AlignInfoDimensionInfo dimInfo;
  // Offset by one, since after the parse routines index is incremented.
  int curIndex = arrayHt->d->a_index-1;

  // Align information is stored in the array record
  // the ith align is in align_info[i]
  // there is a linked list of index and mapping info, one per dimension
  // in AlignList (align_info)

  // However, in the case of perfect alignment, we do not have this
  // linked list. Need to check for this and handle appropriately:
  if (arrayHt->d->align_info[curIndex]->perfect_align) {
    // Perfect alignments, just stuff away:
    dimInfo.coeff  = 1;
    dimInfo.offset = 0;
    dimInfo.theType = ALIGNINFO_PERFECT;
    for (int i = 0; i < arrayHt->d->align_info[curIndex]->ndim; i++) {
      align->AddDimensionInfo(dimInfo);
    }
  } else {
    // We have the more complex case of non-perfect alignments
    AlignEntry * alignEntry = arrayHt->d->align_info[curIndex]->first_entry();
    while (alignEntry !=0) {
      subscr_info * thisSubscript = alignEntry->sub_info();
      dimInfo.coeff = thisSubscript->coeff;
      dimInfo.offset = thisSubscript->offset;
      // The two enums used here are just different enough to screw you up
      switch (thisSubscript->stype) {
      case ALIGN_UNKNOWN: 
	dimInfo.theType = ALIGNINFO_UNKNOWN;
	break;
      case ALIGN_PERFECT:
	dimInfo.theType = ALIGNINFO_PERFECT;
	break;
      case ALIGN_OFFSET:
	dimInfo.theType = ALIGNINFO_OFFSET;
	break;
      case ALIGN_COEFF:
	dimInfo.theType = ALIGNINFO_COEFF;
	break;
      case ALIGN_CONST:
	dimInfo.theType = ALIGNINFO_CONST;
	break;
      default:
	ShouldNotGetHere;
      }
      align->AddDimensionInfo(dimInfo);
      alignEntry = arrayHt->d->align_info[curIndex]->next_entry();
    }	
  }
}

////////////////////////////////////////////////////////////////////////
// Find and cross link with appropriate decomp
////////////////////////////////////////////////////////////////////////
void SD_Link_Decomp_to_Align(FortranDHashTableEntry *decompHt, 
			     SDDF_AlignInfo * align ) {  
  
  MkAssert(thePabloGlobalInfo.wantPabloInfo, "SD_Link_Decomp_to_Align: "
	   "Called without first calling SD_InitialSetup?", EXIT);
	   
  PabloLocalInfo * myLocals = thePabloGlobalInfo.currentLocals;
  
  // Discover the corresponding decompHt statement
  AST_INDEX decompNode = ft_NumberToNode(myLocals->curFt,decompHt->d->decomp_id_number);

  // Get side array for decomp, with check
  PabloSideArrayInfo * decompInfo =
    myLocals->sideArray->getInfo(decompNode);
  MkAssert(decompInfo!=0,"No Side array for Node!",EXIT);

  SDDF_DecompInfo * decomp;
  decomp = (SDDF_DecompInfo *) decompInfo->GetSafe(SDDF_DECOMPINFO);
  // Check to see if we actually got a decomp back
  MkAssert(decomp!=0,"No decomp stored at this node!",EXIT);

  // Make cross pointers
  align->SetDecomp(decomp);
  decomp->AddAlign(align);
}

////////////////////////////////////////////////////////////////////////
// Find and cross link with appropriate array
////////////////////////////////////////////////////////////////////////
void SD_Link_Array_to_Align(AST_INDEX cur, FortranDHashTableEntry *arrayHt,
			    SDDF_AlignInfo * align) {
  
  MkAssert(thePabloGlobalInfo.wantPabloInfo, "SD_Link_Array_to_Align: "
	   "Called without first calling SD_InitialSetup?", EXIT);
	   
  PabloLocalInfo * myLocals = thePabloGlobalInfo.currentLocals;

    // Add this align to array info.
  SDDF_ArrayInfo * arrayInfo;
  // This gets one of the many possible names for the array
  char * arrayName = arrayHt->name();
  
  // Add this to the list of arrays I know about:
  // This is an ugly hack which I should be ashamed of.
  thePabloGlobalInfo.arrayNameList.Append(arrayName);

  // Really need to do something to prevent multiple calls for same array. 
  // Right now the arrayinfostuff does not provide easy access
  // Get the table entry, and create if it does not exist.
  // also get/create the SDDF_ArrayInfo record
  InfoTableEntry * iTable = 
    thePabloGlobalInfo.arrayInfo->GetFullEntry(arrayName);
  arrayInfo = iTable->arrayInfo;

  // Add back links
  SDDF_ProcInfo * proc = myLocals->GetProcInfo();
  proc->AddArray(arrayInfo);

  // This is incorrect, but I wanted at least some minimal debug info.
  // I really need to decide on a cannonical name for the array and
  // use that. This will rename the array for each instance it is
  // aligned.
  arrayInfo->SetName(arrayName);

  // Dance through the array structure and grab what one can for the
  // array record:
  arrayInfo->SetDimension(arrayHt->getdim());

  // Add the ast of the align to the table
 iTable->correspAligns.Append((void*)(cur));

  // Add the forward and back pointers to the array record:
  arrayInfo->AddAlign(align);
  align->SetArray(arrayInfo);
}
  
////////////////////////////////////////////////////////////////////////
// Grab the needed items for aligns out of the array record.
// Also generates cross links to decomp and arrays
// This is called once for each array in align.
////////////////////////////////////////////////////////////////////////
void SD_Get_Align_Info(AST_INDEX cur, FortranDHashTableEntry *decompHt, 
		       FortranDHashTableEntry *arrayHt) {
  
  if (! thePabloGlobalInfo.wantPabloInfo) return;
  
  PabloLocalInfo * myLocals = thePabloGlobalInfo.currentLocals;
  
  // These routines are called twice, once before whole program
  // analysis, once after. myLocals is nil the first time, which is
  // when we do not want to execute. Relys on this behavior.
  if (myLocals ==0) return;

  // Create a static distribution record, and store it into the structure:
  SDDF_AlignInfo * align = new SDDF_AlignInfo;

  PabloSideArrayInfo * sideNode =   myLocals->sideArray->getInfo(cur);
  
  // Now, we may have already seen another part of this align.
  // This version of the code does not need to do anything here.
  sideNode->AddSafe(SDDF_ALIGNINFO, align);

  // Figure out line number and procedure name info
  int lineNo = myLocals ->sideArray->getLine(cur);
  SDDF_ProcInfo * proc = myLocals->GetProcInfo();
  align->SetPosition(lineNo,myLocals->GetProcInfo());
  proc->AddAlign(align);

  //////////////////////////////////////////////////////
  // Find the decomp, and do cross links
  SD_Link_Decomp_to_Align(decompHt,align);

  //////////////////////////////////////////////////////
  // Add the alignment type info to the align record
  SD_AddAlignTypeInfo(align,arrayHt);

  //////////////////////////////////////////////////////
  // Add this align to array info.
  SD_Link_Array_to_Align(cur, arrayHt, align);

  // I think we are done.
  // !!!! don't forget fixup stage for array info!
}

//===========================================================================
// Fix up array name info after program compilation.
//===========================================================================

////////////////////////////////////////////////////////////////////////
// Find a cannonical name from the itable
////////////////////////////////////////////////////////////////////////
ArrayNameListEntry * findCannonicalName(InfoTableEntry * i) {
  // Walk the array
  ArrayNameListEntry * curEntry = i->equivNameListHead;
  while ((curEntry  !=0) && (curEntry->procType != GEN_PROGRAM)) {
    curEntry = curEntry->next;
  }	
  // Use the program declaration if available
  if (curEntry ==0) {
    // not avail, so use first on list:
    return i->equivNameListHead;
  } else if (curEntry->procType == GEN_PROGRAM) {
    // return decl in program body.
    return curEntry;
  }
  else {		// 7/11/94, VSA: Final else case was missing.
    assert(false);	//		 Needed for return value.
    return (ArrayNameListEntry*) NULL;
  }
}

////////////////////////////////////////////////////////////////////////
// Do the array processing
////////////////////////////////////////////////////////////////////////
void SD_ProcessArrays() {
  MkAssert(thePabloGlobalInfo.wantPabloInfo, "SD_ProcessArrays: "
	   "Called without first calling SD_InitialSetup?", EXIT);
  
  // I need to go through the arrayNameList, get the array info for it,
  // and stuff the array record appropriately

  char*               name;
  SDDF_ArrayInfo*     array;
  InfoTableEntry*     iTableEnt;
  ArrayNameListEntry *canonicalEntry;
  ArrayNameIterator arrayNameSet(thePabloGlobalInfo.arrayInfo);
  
  for ( ; (name = arrayNameSet.arrayName) != (char*) NULL; ++arrayNameSet) {
    array     = arrayNameSet.arrayInfo;
    iTableEnt = arrayNameSet.fullEntry;
    MkAssert(iTableEnt!=0,"We should have an entry for this array name",ABORT);
    MkAssert(array    !=0,"We should have an info rec. for this array", ABORT);
    
    assert (iTableEnt->beenHere == false);
    iTableEnt->beenHere = true;
    
    canonicalEntry = findCannonicalName(iTableEnt);
    MkAssert(canonicalEntry !=0,
	     "We should have at least one entry for this array name",ABORT);
    
    array->SetName(canonicalEntry->arrayName);
    array->SetPosition(/*lineNo*/ -1, canonicalEntry->procInfo);
    // char * fileName = "Unknown";
  }
}
