/* $Id: SD_DataInfo.C,v 1.1 1997/03/11 14:29:00 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// -*- C++ -*- 
/* One line expanatory comment */ 
/* $Header: /home/cs/carr/cvsroot/DSystem/src/libs/fortD/performance/staticInfo/SD_DataInfo.C,v 1.1 1997/03/11 14:29:00 carr Exp $
//
*/

static const char * RCS_ID = "$Id: SD_DataInfo.C,v 1.1 1997/03/11 14:29:00 carr Exp $";
#define ASSERT_FILE_VERSION RCS_ID
#define MKASSERT

// Do includes for this code
// Include corresponding .h file last. In general, if these routines
// are called externally, there should be a .h file.
#include <malloc.h>
#include <string.h>
#include <libs/fortD/performance/staticInfo/SD_Base.h>
#include <libs/fortD/performance/staticInfo/SD_DataInfo.h>
#include <libs/fortD/performance/staticInfo/SD_SrcInfo.h>
#include <libs/fortD/performance/staticInfo/SD_MsgInfo.h>
#include <libs/fortD/performance/staticInfo/MkAssert.h>
#include <libs/fortD/performance/staticInfo/utility.h>
#include <libs/fortD/performance/staticInfo/SD_Globals.i>

#include <Attributes.h>
#include <RecordDossier.h>
#include <StructureDescriptor.h>
#include <PipeReader.h>
#include <PipeWriter.h>
#include <libs/fortD/performance/staticInfo/StaticSDDF.h>

///////////////////////////////////////////////////////////////////////////
// SDDF_DecompInfo
///////////////////////////////////////////////////////////////////////////

RecordDossier * SDDF_DecompInfo::localDossier = 0;
static const char * DECOMP_NAME_NAME = "Decomp Name";
static const char * DECOMP_DIMENSIONS_NAME = "Dimensions";
static const char * DECOMP_ALIGN_ID_NAME = "Align ID";
static const char * DECOMP_DYNAMIC_DIST_NAME = "Dynamic Distributed";
static const char * DECOMP_DIST_ID_NAME = "Dist ID";

SDDF_DecompInfo::SDDF_DecompInfo() : StaticDescriptorBase() {
  decompName = dupstr("Unassigned");
  decompDim = 0;
  isDynamicDist = false;
};

SDDF_DecompInfo::~SDDF_DecompInfo() {
  delete[] decompName;
};

void SDDF_DecompInfo::SetName(char * Name) {
  SetString(decompName,Name);
}

const char * SDDF_DecompInfo::GetName(void) const {
  return decompName;
}

void SDDF_DecompInfo::SetDimension(int Dim) {
  decompDim = Dim;
}

int SDDF_DecompInfo::GetDimension(void) const {
  return decompDim;
}
  
void SDDF_DecompInfo::SetDistributedFlag(Boolean d) {
  isDynamicDist = d;
}
Boolean SDDF_DecompInfo::GetDistributedFlag(void) const {
  return isDynamicDist;
}
  
void SDDF_DecompInfo::AddAlign(SDDF_AlignInfo * p) {
  correspondingAligns.AddElement(p);
}
void SDDF_DecompInfo::AddDist(SDDF_DistInfo * p) {
  correspondingDists.AddElement(p);
}
  
//IO operations
ostream & operator << (ostream & o, const SDDF_DecompInfo & s) {
  o << "SDDF_DecompInfo\n";
  o << "Name: " << s.decompName << " Dimensions: " << s.decompDim;
  o << "Is Dynamic " << s.isDynamicDist << endl;
  o << "Aligns  " << s.correspondingAligns << endl;
  o << "Decomps " << s.correspondingDists << endl;
  return o;
};

void SDDF_DecompInfo::Dump() const {
  cout << *this;
}

void SDDF_DecompInfo::SddfInitDescriptor(PipeWriter & output) {
  if(localDossier ==0) {
    // Only do this once
    Attributes 		theAttributes;
    theAttributes.clearEntries();
    theAttributes.insert("description","FD Static Decomposition Record");
    StructureDescriptor structDesc("FDStat Decomp",theAttributes);

    // Do standard attributes
    StaticDescriptorBase::SddfElementInit(structDesc);

    // Do attributes specific to Decomp
    AddEntryToStructureDesc(structDesc,"Name",DECOMP_NAME_NAME,CHARACTER, 1);

    AddEntryToStructureDesc(structDesc,"Dimension", DECOMP_DIMENSIONS_NAME,
			    INTEGER, 0);
    
    AddEntryToStructureDesc(structDesc,"Pointer",DECOMP_ALIGN_ID_NAME,
			    INTEGER, 1);

    AddEntryToStructureDesc(structDesc,"Boolean",DECOMP_DYNAMIC_DIST_NAME,
			    INTEGER, 0);

    AddEntryToStructureDesc(structDesc,"Pointer",DECOMP_DIST_ID_NAME,
			    INTEGER, 1);

    output.putDescriptor(structDesc,SD_TAG_DECOMP);
    localDossier = new RecordDossier(SD_TAG_DECOMP,structDesc);
  }
}

void SDDF_DecompInfo::SddfDump(PipeWriter & output) const {
  MkAssert(localDossier,"Hey, no dossier!",EXIT);
  // Output standard attributes
  StaticDescriptorBase::SddfDossierOut(*localDossier);

  SetStringInDossier(*localDossier,DECOMP_NAME_NAME,decompName);

  localDossier->setValue(DECOMP_DIMENSIONS_NAME,decompDim);

  Array * alignArray = localDossier->getArrayP(DECOMP_ALIGN_ID_NAME);
  correspondingAligns.ResolveToArray(alignArray);

  localDossier->setValue(DECOMP_DYNAMIC_DIST_NAME,int(isDynamicDist));

  Array * distArray = localDossier->getArrayP(DECOMP_DIST_ID_NAME);
  correspondingDists.ResolveToArray(distArray);

  output.putData(*localDossier);
}

void SDDF_DecompInfo::SddfRead(PipeReader &) {
  ShouldNotGetHere;
}


///////////////////////////////////////////////////////////////////////////
// SDDF_DistInfo
///////////////////////////////////////////////////////////////////////////

RecordDossier * SDDF_DistInfo::localDossier = 0;
static const char * DIST_DECOMP_ID_NAME = "Decomp ID";
static const char * DIST_DIST_TYPE_NAME = "Dist Type";
static const char * DIST_PROCESSOR_NUM_NAME = "Processors";
static const char * DIST_BLOCK_SIZE_NAME = "Block Size";
static const char * DIST_INT_LOCAL_SIZE_NAME = "Internal Local Array Size";
static const char * DIST_BOUND_LOCAL_SIZE_NAME = "Boundrary Local Array Size";
static const char * DIST_MIN_GLOBAL_INDEX_NAME = "Minimum Global Index";
static const char * DIST_MAX_GLOBAL_INDEX_NAME = "Maximum Global Index";
static const char * DIST_STEP_GLOBAL_INDEX_NAME = "Step of Global Index";
static const char * DIST_MESG_ID_NAME = "Mesg ID";

ostream & operator << (ostream & o, const DistInfoDimensionDeclInfo & d) {
  o << "Type " << d.theType;
  o << "\tBlockSize " << d.blockSize;
  o << "\tProcessors" << d.numProc << endl;
  o << "\tILAS " << d.internalLocalArraySize;
  o << "\tBLAS " << d.boundaryLocalArraySize;
  o << "\tmGI " << d.minGlobalIndex;
  o << "\tMGI " << d.maxGlobalIndex;
  o << "\tsGI" << d.stepGlobalIndex << endl;
  return o;
}


SDDF_DistInfo::SDDF_DistInfo() : StaticDescriptorBase() {
  correspondingDecomp = 0;
}

SDDF_DistInfo::~SDDF_DistInfo() {};

void SDDF_DistInfo::SetDecomp(SDDF_DecompInfo * d) {
  correspondingDecomp = d;
}

void SDDF_DistInfo::AddDimensionInfo(DistInfoDimensionDeclInfo & d) {
  DistInfoDimensionDeclInfo * dimInfo = new DistInfoDimensionDeclInfo(d);
  dimensionInfo.Push(dimInfo);
}

void SDDF_DistInfo::AddMessage(SDDF_MessageSend * s) {
  correspondingMessages.AddElement(s);
}

//IO operations
ostream & operator << (ostream & o, const SDDF_DistInfo & s) {
  o <<  "SDDF_DistInfo\n";
  o << "Decomp   " << s.correspondingDecomp;
  
  // This casts to a not const VPDlist without invoking a constructor
  VPDlist_iter i(CAST_NO_CTOR(VPDlist,s.dimensionInfo));
  int pos = 0;
  while(!i.Eol()) {
    o << *(DistInfoDimensionDeclInfo *)(i.Current());
    i.Next();
  }  
  o << "Messages " <<  s.correspondingMessages;
  return o;
}

void SDDF_DistInfo::Dump() const {
  cout << *this;
}

void SDDF_DistInfo::SddfInitDescriptor(PipeWriter & output) {
  if(localDossier ==0) {
    // Only do this once
    Attributes 		theAttributes;
    theAttributes.clearEntries();
    theAttributes.insert("description","FD Static Distribution Record");
    StructureDescriptor structDesc("FDStat Dist",theAttributes);

    // Do standard attributes
    StaticDescriptorBase::SddfElementInit(structDesc);
  
    // Do attributes specific to Dist
    AddEntryToStructureDesc(structDesc,"Pointer",DIST_DECOMP_ID_NAME,  
			    INTEGER,0);

    // all of these should be same size
    AddEntryToStructureDesc(structDesc,"Dist Info",DIST_DIST_TYPE_NAME,  
			    INTEGER,1);
    AddEntryToStructureDesc(structDesc,"Dist Info",DIST_PROCESSOR_NUM_NAME,
			    INTEGER,1);
    AddEntryToStructureDesc(structDesc,"Dist Info",DIST_BLOCK_SIZE_NAME,
			    INTEGER,1);
    AddEntryToStructureDesc(structDesc,"Dist Info",DIST_INT_LOCAL_SIZE_NAME,
			    INTEGER,1);
    AddEntryToStructureDesc(structDesc,"Dist Info",DIST_BOUND_LOCAL_SIZE_NAME,
			    INTEGER,1);
    AddEntryToStructureDesc(structDesc,"Dist Info",DIST_MIN_GLOBAL_INDEX_NAME,
			    INTEGER,1);
    AddEntryToStructureDesc(structDesc,"Dist Info",DIST_MAX_GLOBAL_INDEX_NAME,
			    INTEGER,1);
    AddEntryToStructureDesc(structDesc,"Dist Info",DIST_STEP_GLOBAL_INDEX_NAME,
			    INTEGER,1);
    AddEntryToStructureDesc(structDesc,"Pointer",DIST_MESG_ID_NAME,INTEGER,1);

    output.putDescriptor(structDesc,SD_TAG_DIST);
    localDossier = new RecordDossier(SD_TAG_DIST,structDesc);
  }
}

void SDDF_DistInfo::SddfDump(PipeWriter & output) const {
  MkAssert(localDossier,"Hey, no dossier!",EXIT);
  // Output standard attributes
  StaticDescriptorBase::SddfDossierOut(*localDossier);
  
  localDossier->setValue(DIST_DECOMP_ID_NAME,
			 ResolveStaticDescriptorId(correspondingDecomp));

  // Get arrays
  Array *theTypeArray   = localDossier->getArrayP(DIST_DIST_TYPE_NAME);
  Array *blockSizeArray = localDossier->getArrayP(DIST_PROCESSOR_NUM_NAME);
  Array *numProcArray   = localDossier->getArrayP(DIST_BLOCK_SIZE_NAME);
  Array *internalLocalArraySizeArray = 
    localDossier->getArrayP(DIST_INT_LOCAL_SIZE_NAME);
  Array *boundaryLocalArraySizeArray = 
    localDossier->getArrayP(DIST_BOUND_LOCAL_SIZE_NAME);
  Array *minGlobalIndexArray = 
    localDossier->getArrayP(DIST_MIN_GLOBAL_INDEX_NAME);
  Array *maxGlobalIndexArray = 
    localDossier->getArrayP(DIST_MAX_GLOBAL_INDEX_NAME);
  Array *stepGlobalIndexArray = 
    localDossier->getArrayP(DIST_STEP_GLOBAL_INDEX_NAME);

  // Set Sizes:
  int dimSize[1];
  dimSize[0] = dimensionInfo.size();
  theTypeArray->setDimSizes(dimSize);
  blockSizeArray->setDimSizes(dimSize);
  numProcArray->setDimSizes(dimSize);
  internalLocalArraySizeArray->setDimSizes(dimSize);
  boundaryLocalArraySizeArray->setDimSizes(dimSize);
  minGlobalIndexArray->setDimSizes(dimSize);
  maxGlobalIndexArray->setDimSizes(dimSize);
  stepGlobalIndexArray->setDimSizes(dimSize);

  // Scan through and set items:
  int pos = 0;
  // This casts to a not const VPDlist without invoking a constructor
  VPDlist_iter i(CAST_NO_CTOR(VPDlist,dimensionInfo));
  while(!i.Eol()) {
    DistInfoDimensionDeclInfo * dimDecl =
      (DistInfoDimensionDeclInfo *)(i.Current());

    // Set them arrays
    theTypeArray->setCellValue(dimDecl->theType,pos);
    blockSizeArray->setCellValue(dimDecl->blockSize,pos);
    numProcArray->setCellValue(dimDecl->numProc,pos);
    internalLocalArraySizeArray->setCellValue(dimDecl->internalLocalArraySize,pos);
    boundaryLocalArraySizeArray->setCellValue(dimDecl->boundaryLocalArraySize,pos);
    minGlobalIndexArray->setCellValue(dimDecl->minGlobalIndex,pos);
    maxGlobalIndexArray->setCellValue(dimDecl->maxGlobalIndex,pos);
    stepGlobalIndexArray->setCellValue(dimDecl->stepGlobalIndex,pos);
    
    pos++; i.Next();
  }
  
  Array *mesgArray = localDossier->getArrayP(DIST_MESG_ID_NAME);
  correspondingMessages.ResolveToArray(mesgArray);

  output.putData(*localDossier);
}

void SDDF_DistInfo::SddfRead(PipeReader & ) { ShouldNotGetHere;}

///////////////////////////////////////////////////////////////////////////
// SDDF_AlignInfo
///////////////////////////////////////////////////////////////////////////

RecordDossier * SDDF_AlignInfo::localDossier = 0;
static const char * ALIGN_ARRAY_ID_NAME = "Array ID";
static const char * ALIGN_DECOMP_ID_NAME = "Decomp ID";
static const char * ALIGN_ALIGN_TYPE_NAME = "Align Type";
static const char * ALIGN_ALIGN_OFFSET_NAME = "Align Offset";
static const char * ALIGN_ALIGN_COEFF_NAME = "Align Coeff";
static const char * ALIGN_MESG_ID_NAME = "Mesg ID";

ostream & operator << (ostream & o, const AlignInfoDimensionInfo & d) {
  o << "Type " << d.theType;
  o << "\tOffset " << d.offset;
  o << "\tCoeff" << d.coeff << endl;
  return o;
}

SDDF_AlignInfo::SDDF_AlignInfo() : StaticDescriptorBase() {
  correspondingArray  =0;
  correspondingDecomp =0;
} 

SDDF_AlignInfo::~SDDF_AlignInfo() {}

void SDDF_AlignInfo::SetArray(SDDF_ArrayInfo * a) {
  correspondingArray =a;
}

void SDDF_AlignInfo::SetDecomp(SDDF_DecompInfo * d) {
  correspondingDecomp = d;
}
  
void SDDF_AlignInfo::AddDimensionInfo(AlignInfoDimensionInfo & a) {
  AlignInfoDimensionInfo * dimInfo = new AlignInfoDimensionInfo(a);
  dimensionInfo.Push(dimInfo);
}

void SDDF_AlignInfo::AddMessage(SDDF_MessageSend * s ) {
  correspondingMessages.AddElement(s);
}
  
//IO operations
ostream & operator << (ostream & o, const SDDF_AlignInfo & s) {
  o <<  "SDDF_AlignInfo\n";
  o << "Array   " << s.correspondingArray;
  o << "Decomp  " << s.correspondingDecomp;

  // This casts to a not const VPDlist without invoking a constructor
  VPDlist_iter i(CAST_NO_CTOR(VPDlist,s.dimensionInfo));
  int pos = 0;
  while(!i.Eol()) {
    o << *(DistInfoDimensionDeclInfo *)(i.Current());
    i.Next();
  }  
  o << "Messages " <<  s.correspondingMessages;
  return o;
}

void SDDF_AlignInfo::Dump() const  {
  cout << *this;
}

void SDDF_AlignInfo::SddfInitDescriptor(PipeWriter & output) {
  if(localDossier ==0) {
    // Only do this once
    Attributes 		theAttributes;
    theAttributes.clearEntries();
    theAttributes.insert("description","FD Static Alignment Record");
    StructureDescriptor structDesc("FDStat Align",theAttributes);

    // Do standard attributes
    StaticDescriptorBase::SddfElementInit(structDesc);
  
    // Do attributes specific to Dist
    AddEntryToStructureDesc(structDesc,"Pointer",ALIGN_ARRAY_ID_NAME,  
			    INTEGER,0);

    // all of these should be same size
    AddEntryToStructureDesc(structDesc,"Dist Info",ALIGN_DECOMP_ID_NAME,  
			    INTEGER,0);

    AddEntryToStructureDesc(structDesc,"Dist Info",ALIGN_ALIGN_TYPE_NAME,
			    INTEGER,1);
    AddEntryToStructureDesc(structDesc,"Dist Info",ALIGN_ALIGN_OFFSET_NAME,
			    INTEGER,1);
    AddEntryToStructureDesc(structDesc,"Dist Info",ALIGN_ALIGN_COEFF_NAME,
			    INTEGER,1);
    AddEntryToStructureDesc(structDesc,"Pointer",ALIGN_MESG_ID_NAME,INTEGER,1);

    output.putDescriptor(structDesc,SD_TAG_ALIGN);
    localDossier = new RecordDossier(SD_TAG_ALIGN,structDesc);
  }
}

void SDDF_AlignInfo::SddfDump(PipeWriter & output) const {	
  MkAssert(localDossier,"Hey, no dossier!",EXIT);
  // Output standard attributes
  StaticDescriptorBase::SddfDossierOut(*localDossier);
  
  localDossier->setValue(ALIGN_ARRAY_ID_NAME,
			 ResolveStaticDescriptorId(correspondingArray));
  localDossier->setValue(ALIGN_DECOMP_ID_NAME,
			 ResolveStaticDescriptorId(correspondingDecomp));

  // Get arrays
  Array *theTypeArray =   localDossier->getArrayP(ALIGN_ALIGN_TYPE_NAME);
  Array *offsetArray = localDossier->getArrayP(ALIGN_ALIGN_OFFSET_NAME);
  Array *coeffArray     = localDossier->getArrayP(ALIGN_ALIGN_COEFF_NAME);

    // Set Sizes:
  int dimSize[1];
  dimSize[0] = dimensionInfo.size();
  theTypeArray->setDimSizes(dimSize);
  offsetArray->setDimSizes(dimSize);
  coeffArray->setDimSizes(dimSize);

  int pos = 0;
  // This casts to a not const VPDlist without invoking a constructor
  VPDlist_iter i(CAST_NO_CTOR(VPDlist,dimensionInfo));
  while(!i.Eol()) {
    AlignInfoDimensionInfo * dimDecl =
      (AlignInfoDimensionInfo *)(i.Current());

    // Set them arrays
    theTypeArray->setCellValue(dimDecl->theType,pos);
    offsetArray->setCellValue(dimDecl->offset,pos);
    coeffArray->setCellValue(dimDecl->coeff,pos);
    pos++; i.Next();
  }

  Array * mesgArray = localDossier->getArrayP(DIST_MESG_ID_NAME);
  correspondingMessages.ResolveToArray(mesgArray);

  output.putData(*localDossier);
}

void SDDF_AlignInfo::SddfRead(PipeReader & ) { ShouldNotGetHere;}

///////////////////////////////////////////////////////////////////////////
// SDDF_ArrayInfo
///////////////////////////////////////////////////////////////////////////

RecordDossier * SDDF_ArrayInfo::localDossier = 0;
static const char * ARRAY_NAME_NAME        = "Array Name";
static const char * ARRAY_DIMENSION_NAME   = "Dimensions";
static const char * ARRAY_LOWER_BOUND_NAME = "Local Lower Bound";
static const char * ARRAY_UPPER_BOUND_NAME = "Local Upper Bound";
static const char * ARRAY_ALIGN_ID_NAME    = "Align ID";
static const char * ARRAY_MESG_ID_NAME     = "Mesg ID";

SDDF_ArrayInfo::SDDF_ArrayInfo() : StaticDescriptorBase() {
  arrayName = dupstr("Unassigned");
  arrayDim  = 0;
  numProcs  = 0;
}

SDDF_ArrayInfo::~SDDF_ArrayInfo(){
  delete[] arrayName;
  arrayDim =0;
  numProcs =0;
}

void SDDF_ArrayInfo::SetName(char * Name) {
  MkAssert(Name !=0,"Must provide a real name!",EXIT);
  SetString(arrayName,Name);
}

const char * 	SDDF_ArrayInfo::GetName(void) const{ 
  return arrayName;
}

void SDDF_ArrayInfo::SetDimension(int Dim) { 
  MkAssert(Dim>0,"An array with no or negative dimensions?",EXIT);
  arrayDim = Dim;
}

int SDDF_ArrayInfo::GetDimension(void) const {
  return arrayDim;
}
 
void SDDF_ArrayInfo::AddLocalLB(SDDFint bound) {
  lowerBounds.Append((void *)(bound));
}

void SDDF_ArrayInfo::AddLocalUB(SDDFint bound) {
  upperBounds.Append((void *)(bound));
}

 
void SDDF_ArrayInfo::AddAlign(SDDF_AlignInfo *p) {
  correspondingAligns.AddElement(p);
}

void SDDF_ArrayInfo::AddMessage(SDDF_MessageSend *p) {
  correspondingMessages.AddElement(p);
}
  
ostream & operator << (ostream & o, const SDDF_ArrayInfo & s) {
  o << "SDDF_ArrayInfo\n";
  o << "Name: " << s.arrayName << " Dimensions: " << s.arrayDim;
  o << "Processors " << s.numProcs << endl;
  o << "Dim\tLower\tUpper\n";
  for (int i = 0; i < s.lowerBounds.NumberOfEntries(); i++) {
    o << i << "\t" << int(s.lowerBounds[i]) << "\t" << int(s.upperBounds[i]) << endl;
  }
  o << "Aligns   " << s.correspondingAligns << endl;
  o << "Messages " << s.correspondingMessages << endl;
  return o;
}

void SDDF_ArrayInfo::Dump() const { 
  cout << *this;
}

void SDDF_ArrayInfo::SddfInitDescriptor(PipeWriter & output) {
  if (localDossier == 0) {
    Attributes 		theAttributes;
    theAttributes.clearEntries();
    theAttributes.insert("description","FD Static Distributed Array Record");
    StructureDescriptor structDesc("FDStat Array",theAttributes);

    // Do standard attributes
    StaticDescriptorBase::SddfElementInit(structDesc);

    AddEntryToStructureDesc(structDesc,"Dist Info",ARRAY_NAME_NAME,  
			    CHARACTER,1);

    AddEntryToStructureDesc(structDesc,"Dist Info",ARRAY_DIMENSION_NAME,  
			    INTEGER,0);

    AddEntryToStructureDesc(structDesc,"Dist Info",ARRAY_LOWER_BOUND_NAME,  
			    INTEGER,2);
    AddEntryToStructureDesc(structDesc,"Dist Info",ARRAY_UPPER_BOUND_NAME,  
			    INTEGER,2);
    
    AddEntryToStructureDesc(structDesc,"Dist Info",ARRAY_ALIGN_ID_NAME,  
			    INTEGER,1);
    AddEntryToStructureDesc(structDesc,"Dist Info",ARRAY_MESG_ID_NAME,  
			    INTEGER,1);

    output.putDescriptor(structDesc,SD_TAG_ARRAY);
    localDossier = new RecordDossier(SD_TAG_ARRAY,structDesc);
  }
}

void SDDF_ArrayInfo::SddfDump(PipeWriter & output) const{
  MkAssert(localDossier,"Hey, no dossier!",EXIT);
  // Output standard attributes
  StaticDescriptorBase::SddfDossierOut(*localDossier);

  SetStringInDossier(*localDossier,ARRAY_NAME_NAME,arrayName);
  
  localDossier->setValue(ARRAY_DIMENSION_NAME,arrayDim);
  
  // Get arrays:
  Array * lowerBoundArray = localDossier->getArrayP(ARRAY_LOWER_BOUND_NAME);
  Array * upperBoundArray = localDossier->getArrayP(ARRAY_UPPER_BOUND_NAME);

  // hack to compensate for the fact that lowerBounds and upperBounds
  // are declared as 2D in the sddf spec.

  int procsToOutput = numProcs;

  if (procsToOutput ==0) {
    procsToOutput =1;
  } 
  
  int dimension = lowerBounds.NumberOfEntries();

  int dimSize[2];
  dimSize[0] = procsToOutput;
  dimSize[1] = dimension;
  lowerBoundArray->setDimSizes(dimSize);
  upperBoundArray->setDimSizes(dimSize);

  for (int proc = 0; proc < procsToOutput; proc++) {
    for (int dim = 0; dim < dimension; dim++) {
      lowerBoundArray->setCellValue(int(lowerBounds[dim]),proc,dim);
      upperBoundArray->setCellValue(int(upperBounds[dim]),proc,dim);
    }
  }

  Array * alignArray = localDossier->getArrayP(ARRAY_ALIGN_ID_NAME);
  Array * mesgArray = localDossier->getArrayP(ARRAY_MESG_ID_NAME);

  correspondingAligns.ResolveToArray(alignArray);
  correspondingMessages.ResolveToArray(mesgArray);

  output.putData(*localDossier);
}
void SDDF_ArrayInfo::SddfRead(PipeReader &){
  ShouldNotGetHere;
}	   
