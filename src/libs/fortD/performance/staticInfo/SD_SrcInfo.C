/* $Id: SD_SrcInfo.C,v 1.1 1997/03/11 14:29:04 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// -*- C++ -*- 
/* One line expanatory comment */ 
/* $Header: /home/cs/carr/cvsroot/DSystem/src/libs/fortD/performance/staticInfo/SD_SrcInfo.C,v 1.1 1997/03/11 14:29:04 carr Exp $
//
*/

static const char * RCS_ID = "$Id: SD_SrcInfo.C,v 1.1 1997/03/11 14:29:04 carr Exp $";
#define ASSERT_FILE_VERSION RCS_ID
#define MKASSERT

// Do includes for this code
// Include corresponding .h file last. In general, if these routines
// are called externally, there should be a .h file.

#include <libs/fortD/performance/staticInfo/MkAssert.h>
#include <libs/fortD/performance/staticInfo/utility.h>
#include <libs/fortD/performance/staticInfo/SD_Base.h>
#include <libs/fortD/performance/staticInfo/SD_SrcInfo.h>
#include <libs/fortD/performance/staticInfo/SD_DataInfo.h>
#include <libs/fortD/performance/staticInfo/SD_Globals.i>

#include <Attributes.h>
#include <RecordDossier.h>
#include <StructureDescriptor.h>
#include <PipeReader.h>
#include <PipeWriter.h>
#include <libs/fortD/performance/staticInfo/StaticSDDF.h>
#include <libs/fortD/performance/staticInfo/utility.h>

///////////////////////////////////////////////////////////////////////////
// SDDF_ProcInfo
///////////////////////////////////////////////////////////////////////////

RecordDossier * SDDF_ProcInfo::localDossier = 0;
static const char * PROC_PROCNAME_NAME = "Procedure Name";
static const char * PROC_FILENAME_NAME = "File Name";
static const char * PROC_ARRAY_ID_NAME = "Array ID";
static const char * PROC_ALIGN_ID_NAME = "Align ID";
static const char * PROC_DIST_ID_NAME  = "Dist ID";
static const char * PROC_RECURSIVE_NAME ="Recursive";

SDDF_ProcInfo:: SDDF_ProcInfo() {
  isRecursive = false;
  procName = dupstr("Undefined");
  fileName = dupstr("Undefined");
}

SDDF_ProcInfo::~SDDF_ProcInfo(){
  isRecursive = false;	
  delete procName;	
  delete fileName;
  procName = 0;
  fileName = 0;
}

void SDDF_ProcInfo::SetProcName(char * Name) {
  SetString(procName,Name);
}

const char * SDDF_ProcInfo::GetProcName(void) const {
  return procName;
}

void SDDF_ProcInfo::SetFileName(char * Name) {
  SetString(fileName,Name);
}

const char * SDDF_ProcInfo::GetFileName(void) const {
  return fileName;
}

void SDDF_ProcInfo::AddArray(SDDF_ArrayInfo * p) {
  correspondingArrays.AddElement(p);
}

void SDDF_ProcInfo::AddAlign(SDDF_AlignInfo * p) {
  correspondingAligns.AddElement(p);	
}

void SDDF_ProcInfo::AddDist(SDDF_DistInfo * p) {
  correspondingDists.AddElement(p);
}

void SDDF_ProcInfo::SetRecursiveFlag(Boolean r){
  isRecursive = r;
}

Boolean SDDF_ProcInfo::GetRecursiveFlag(void) const {
  return isRecursive;
}
  
//IO operations
ostream & operator << (ostream & o, const SDDF_ProcInfo & s) {
  o << "SDDF_ProcInfo\n";
  o << "FileName: " << s.fileName << endl;
  o << "ProcName: " << s.procName << endl;
  o << "Is Recursive" << s.isRecursive << endl;
  o << "Arrays " << s.correspondingArrays << endl;
  o << "Aligns " << s.correspondingAligns << endl;
  o << "Dists " << s.correspondingDists << endl;
  return o;
}

void SDDF_ProcInfo::Dump() const {
  cout << *this;
}

void SDDF_ProcInfo::SddfInitDescriptor(PipeWriter & output) {
  if(localDossier ==0) {
    // Only do this once
    Attributes 		theAttributes;
    theAttributes.clearEntries();
    theAttributes.insert("description","FD Static Procedure Record");
    StructureDescriptor structDesc("FDStat Proc",theAttributes);
    
    // Do standard attributes
    StaticDescriptorBase::SddfElementInit(structDesc);
    
    // Do attributes specific to Decomp	
    AddEntryToStructureDesc(structDesc,"Location",PROC_FILENAME_NAME,CHARACTER,1);
    AddEntryToStructureDesc(structDesc,"Name",PROC_PROCNAME_NAME,CHARACTER,1);

    AddEntryToStructureDesc(structDesc,"Pointer",PROC_ARRAY_ID_NAME,INTEGER,1);
    AddEntryToStructureDesc(structDesc,"Pointer",PROC_ALIGN_ID_NAME,INTEGER,1);
    AddEntryToStructureDesc(structDesc,"Pointer",PROC_DIST_ID_NAME,INTEGER,1);
    
    AddEntryToStructureDesc(structDesc,"Boolean",PROC_RECURSIVE_NAME,INTEGER,0);
    
    output.putDescriptor(structDesc,SD_TAG_PROC);
    localDossier = new RecordDossier(SD_TAG_PROC,structDesc);
  }
}

void SDDF_ProcInfo::SddfDump(PipeWriter & output) const {
  MkAssert(localDossier,"Hey, no dossier!",EXIT);
  StaticDescriptorBase::SddfDossierOut(*localDossier);

  SetStringInDossier(*localDossier,PROC_FILENAME_NAME,fileName);
  SetStringInDossier(*localDossier,PROC_PROCNAME_NAME,procName);

  Array * arrayArray = localDossier->getArrayP(PROC_ARRAY_ID_NAME);
  correspondingArrays.ResolveToArray(arrayArray);

  Array * alignArray = localDossier->getArrayP(PROC_ALIGN_ID_NAME);
  correspondingAligns.ResolveToArray(alignArray);

  Array * distArray = localDossier->getArrayP(PROC_DIST_ID_NAME);
  correspondingDists.ResolveToArray(distArray);

  localDossier->setValue(PROC_RECURSIVE_NAME,int(isRecursive));

  output.putData(*localDossier);
}

void SDDF_ProcInfo::SddfRead(PipeReader & output) {
  ShouldNotGetHere;
}

///////////////////////////////////////////////////////////////////////////
// SDDF_SrcLoopInfo
///////////////////////////////////////////////////////////////////////////


SDDF_SrcLoopInfo::SDDF_SrcLoopInfo(){}
SDDF_SrcLoopInfo::~SDDF_SrcLoopInfo(){}

void SDDF_SrcLoopInfo::AddDepends(SDDF_DependInfo *p){}
void SDDF_SrcLoopInfo::AddSPMDLoops(SDDF_SPMD_LoopInfo *p){}
void SDDF_SrcLoopInfo::AddMessages(SDDF_MessageSend *p){}

  //IO operations
ostream & operator << (ostream & o, const SDDF_SrcLoopInfo & s){return o;}
void SDDF_SrcLoopInfo::Dump() const {}
void SDDF_SrcLoopInfo::SddfInitDescriptor(PipeWriter & output){}
void SDDF_SrcLoopInfo::SddfDump(PipeWriter & output) const{}
void SDDF_SrcLoopInfo::SddfRead(PipeReader & output){}

///////////////////////////////////////////////////////////////////////////
// SDDF_DependInfo
///////////////////////////////////////////////////////////////////////////

RecordDossier * SDDF_DependInfo::localDossier = 0;
static const char * DEPEND_SRC_LINE_NAME  = "Dependence Source Line Number";
static const char * DEPEND_SINK_LINE_NAME = "Dependence Sink Line Number";
static const char * DEPEND_SRC_POS_NAME   = "Dependence Source Reference Position";
static const char * DEPEND_SINK_POS_NAME  = "Dependence Sink Reference Position";
static const char * DEPEND_TYPE_NAME      = "Depend Type";

SDDF_DependInfo::SDDF_DependInfo() {
  srcLine  = -1; 
  sinkLine = -1;
  srcPos   = -1;
  sinkPos  = -1;
  theType  = DEPEND_NONE;
}

SDDF_DependInfo::~SDDF_DependInfo() {
  srcLine  = 0; 
  sinkLine = 0;
  srcPos   = -1;
  sinkPos  = -1;
  theType  = DEPEND_NONE; 
}

void SDDF_DependInfo::SetSrcPos(Line_Number l, int pos) {
  srcLine = l;
  srcPos  = pos;
}

void SDDF_DependInfo::SetSinkPos(Line_Number l, int pos) {
  sinkLine = l;
  sinkPos  = pos;
}

void SDDF_DependInfo::SetDependType(DependInfoDependType d) {
  theType = d;
}

//IO operations
ostream & operator << (ostream & o, const SDDF_DependInfo & s) {
  o << "SDDF_DependInfo\n";
  o << "Source Line, Character Pos: "<< s.srcLine << ',' << s.srcPos;
  o << "Sink Line, Character Pos: "<< s.sinkLine << ',' << s.sinkPos;
  o << endl;
  o << "Depend Type: " << s.theType << endl;
  return o;
}

void SDDF_DependInfo::Dump() const {
  cout << *this;
}

void SDDF_DependInfo::SddfInitDescriptor(PipeWriter & output) {
  if(localDossier ==0) {
    // Only do this once
    Attributes 		theAttributes;
    theAttributes.clearEntries();
    theAttributes.insert("description","FD Static Depend Record");
    StructureDescriptor structDesc("FDStat Depend",theAttributes);
    
    // Do standard attributes
    StaticDescriptorBase::SddfElementInit(structDesc);
    
    // Do attributes specific to Depend
    AddEntryToStructureDesc(structDesc,"Location",DEPEND_SRC_LINE_NAME,INTEGER,0);
    AddEntryToStructureDesc(structDesc,"Character Offset",DEPEND_SRC_POS_NAME,INTEGER,0);
    AddEntryToStructureDesc(structDesc,"Location",DEPEND_SINK_LINE_NAME,INTEGER,0);
    AddEntryToStructureDesc(structDesc,"Character Offset",DEPEND_SINK_POS_NAME,INTEGER,0);

    AddEntryToStructureDesc(structDesc,"Dependence",DEPEND_TYPE_NAME,INTEGER,0);

    output.putDescriptor(structDesc,SD_TAG_DEPEND);
    localDossier = new RecordDossier(SD_TAG_DEPEND,structDesc);
  }  
}
void SDDF_DependInfo::SddfDump(PipeWriter & output) const {
  MkAssert(localDossier,"Hey, no dossier!",EXIT);
  // Output standard attributes
  StaticDescriptorBase::SddfDossierOut(*localDossier);
  
  localDossier->setValue(DEPEND_SRC_LINE_NAME,(int)srcLine);
  localDossier->setValue(DEPEND_SRC_POS_NAME,srcPos);
  localDossier->setValue(DEPEND_SINK_LINE_NAME,(int)sinkLine);
  localDossier->setValue(DEPEND_SINK_POS_NAME,sinkPos);
  
  localDossier->setValue(DEPEND_TYPE_NAME,(int)theType);
  
  output.putData(*localDossier); 
}

void SDDF_DependInfo::SddfRead(PipeReader & output) {
  ShouldNotGetHere;
}


///////////////////////////////////////////////////////////////////////////
// SDDF_SPMD_LoopInfo
///////////////////////////////////////////////////////////////////////////
