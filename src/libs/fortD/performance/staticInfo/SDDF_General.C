/* $Id: SDDF_General.C,v 1.1 1997/03/11 14:28:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// -*- C++ -*- 
// One line expanatory comment 
// $Header: /home/cs/carr/cvsroot/DSystem/src/libs/fortD/performance/staticInfo/SDDF_General.C,v 1.1 1997/03/11 14:28:56 carr Exp $
//
static const char * RCS_ID = "$Id: SDDF_General.C,v 1.1 1997/03/11 14:28:56 carr Exp $";
#define MKASSERT
#define ASSERT_FILE_VERSION RCS_ID

// Do includes for this code
// Include corresponding .h file last. In general, if these routines
// are called externally, there should be a .h file.

#include <libs/fortD/performance/staticInfo/SDDF_General.h>
#include <libs/fortD/performance/staticInfo/SD_Map.h>
#include <libs/fortD/performance/staticInfo/MkAssert.h>
#include <libs/fortD/performance/staticInfo/utility.h>
#include <libs/fortD/performance/staticInfo/SD_SrcInfo.h>

//////////////////////////////////////////////
// Class PabloSideArrayInfo
/////////////////////////////////////////////

StaticDescriptorBase * PabloSideArrayInfo::Get(int i) {
  MkAssert(i>=0 && i < Size(), "Out of range query",ABORT);
  return (StaticDescriptorBase *) a[i];
}

StaticDescriptorBase * PabloSideArrayInfo::GetSafe(SDDF_RECORD_TYPE t, int i) {
  StaticDescriptorBase * b =0;
  if (i>=0 && i < Size()) {
    b = Get(i);
  }
  return(theType == t) ? b : 0;
}

Boolean PabloSideArrayInfo::AddSafe(SDDF_RECORD_TYPE t, StaticDescriptorBase *p) {
  if (Size() ==0) {
    Add(p);
    theType =t;
    return true;
  } else {
    if (theType == t) {
      Add(p);
      return true;
    } else {
      return false;
    }
  }
}

void PabloSideArrayInfo::Add(StaticDescriptorBase * p) {
  a.Append(p);
};

PabloSideArrayInfo::PabloSideArrayInfo() {
  htEnt  = 0;
  theType= SDDF_UNUSED;

  startLine = -1;
  endLine   = -1;
  startChar = -1;
  endChar   = -1;
  theText   = 0;
  DGedge    = (DG_Edge*) NULL;
}


PabloSideArrayInfo::PabloSideArrayInfo(int SLine, int ELine) {
  htEnt  = 0;
  theType= SDDF_UNUSED;

  startLine = SLine;
  endLine   = ELine;
  startChar = -1;
  endChar   = -1;
  theText   = 0;
  DGedge    = (DG_Edge*) NULL;
}


void PabloSideArrayInfo::SetSDDFInfo(StaticDescriptorBase * rec, 
				       SDDF_RECORD_TYPE t,
				       FortranDHashTableEntry * ht) {
  htEnt  = ht;
  a.Append(rec);
  theType= t;
}

PabloSideArrayInfo::~PabloSideArrayInfo() {

  htEnt  = 0;
  theType= SDDF_UNUSED;
}

int PabloSideArrayInfo::Size() { 
  return a.NumberOfEntries();
}

void PabloSideArrayInfo::SetStartLine(int s) {
  startLine = s;
}
void PabloSideArrayInfo::SetEndLine(int s) {
  endLine   = s;
}
void PabloSideArrayInfo::SetStartChar(int s) {
  startChar = s;
}
void PabloSideArrayInfo::SetEndChar(int s) {
  endChar   = s;
}
void PabloSideArrayInfo::SetTextPtr(char * c) {
  theText   = c;
}
void PabloSideArrayInfo::putDGEdge(DG_Edge* edge) {
  DGedge    = edge;
}

int PabloSideArrayInfo::GetStartLine() {
  return startLine;
}
int PabloSideArrayInfo::GetEndLine() {
  return endLine;
}
int PabloSideArrayInfo::GetStartChar() {
  return startChar;
}
int PabloSideArrayInfo::GetEndChar() {
  return endChar;
}
char * PabloSideArrayInfo::GetTextPtr() {
  return theText;
}
DG_Edge* PabloSideArrayInfo::getDGEdge() {
  return DGedge;
}


//////////////////////////////////////////////
// Class PabloLocalInfo
/////////////////////////////////////////////

PabloLocalInfo::PabloLocalInfo() {
  sideArray 	= 0;
  theRoot 	= 0;
  curFt    	= 0;
  curFtt	= 0;
  procInfo	= 0;
  procName      = dupstr("No Procedure Name Set");
  moduleFileName= dupstr("No File Name Set");

}

PabloLocalInfo::~PabloLocalInfo() {
  delete sideArray;
  delete procName;
  delete moduleFileName;
  sideArray 	= 0;
  theRoot 	= 0;
  curFt    	= 0;
  curFtt	= 0;
  procInfo      = 0;
}


void PabloLocalInfo::SetProcInfo(SDDF_ProcInfo * proc) {
    procInfo = proc;
}

void PabloLocalInfo::SetProcName(const char * name) {
  SetString(procName,name);
}
 
void PabloLocalInfo::SetFileName(const char * name) {
  SetString(moduleFileName,name);
}

void PabloLocalInfo::InitSideArray(AST_INDEX root,
				   FortTree ft, FortTextTree ftt) {
  theRoot = root;
  curFt   = ft;
  curFtt  = ftt;

  // Annotate the ast with a side array for line no info and such.
  sideArray = new SDDF_SideArray(moduleFileName,root,ft,ftt,true);
}

SDDF_ProcInfo * PabloLocalInfo::GetProcInfo() const {
  return procInfo;
}

