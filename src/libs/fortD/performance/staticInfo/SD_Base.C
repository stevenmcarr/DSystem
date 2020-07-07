/* $Id: SD_Base.C,v 1.1 1997/03/11 14:28:59 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// -*- C++ -*- 
/* One line expanatory comment */ 
/* $Header: /home/cs/carr/cvsroot/DSystem/src/libs/fortD/performance/staticInfo/SD_Base.C,v 1.1 1997/03/11 14:28:59 carr Exp $
*/

static const char * RCS_ID = "$Id: SD_Base.C,v 1.1 1997/03/11 14:28:59 carr Exp $";
#define ASSERT_FILE_VERSION RCS_ID
#define MKASSERT

// Do includes for this code
// Include corresponding .h file last. In general, if these routines
// are called externally, there should be a .h file.

#include <iostream>
#include <malloc.h>
#include <string.h>
#include <Attributes.h>
#include <RecordDossier.h>
#include <StructureDescriptor.h>
#include <PipeReader.h>
#include <PipeWriter.h>
#include <libs/fortD/performance/staticInfo/StaticSDDF.h>

#include <libs/fortD/performance/staticInfo/MkAssert.h>
#include <libs/fortD/performance/staticInfo/utility.h>

#include <libs/fortD/performance/staticInfo/SD_Base.h>
#include <libs/fortD/performance/staticInfo/SD_List.h>
#include <libs/fortD/performance/staticInfo/SD_SrcInfo.h>
#include <libs/fortD/performance/staticInfo/SD_Globals.i>

////////////////////////////////////////////////////////////////////////////
// CodePosition Code
////////////////////////////////////////////////////////////////////////////

static const char * LINE_NO_NAME         = "Line Number";
static const char * PROCEDURE_ID_NAME  = "Procedure ID";

CodePosition::CodePosition() {
  lineNumber = 0;
  theProcedure =0;
}

CodePosition::CodePosition(Line_Number n, SDDF_ProcInfo * procedure) {
  lineNumber = n;
  theProcedure = procedure;
}


CodePosition::~CodePosition() {
  lineNumber = -1;
  theProcedure =0;
}

void CodePosition::SetPosition(Line_Number n, SDDF_ProcInfo * procedure) {
  theProcedure = procedure;
  lineNumber = n;
}

Line_Number CodePosition::GetLine(int) const {
  return lineNumber;
}

SDDF_ProcInfo* CodePosition::GetProc(int) const {
  return theProcedure;
}
 
void CodePosition::AddPosition(Line_Number n, SDDF_ProcInfo * procedure) {
  SetPosition(n,procedure);
}
  
int CodePosition::Num_Elements() const {
  return 1;
}

// Input and output operations.

void CodePosition::SddfElementInit(StructureDescriptor & structDesc) {
  AddEntryToStructureDesc(structDesc, "Pointer", PROCEDURE_ID_NAME, INTEGER, 0);
  AddEntryToStructureDesc(structDesc,"Location", LINE_NO_NAME, INTEGER, 0);
}


void CodePosition::SddfDossierOut(RecordDossier & d) const {
  d.setValue(LINE_NO_NAME,(int)lineNumber);
  d.setValue(PROCEDURE_ID_NAME, ResolveStaticDescriptorId(theProcedure));
}

void CodePosition::SddfRead(PipeReader &) {
  ShouldNotGetHere;
}

ostream & operator << (ostream & o, const CodePosition & s) {
  o << "Line " << s.lineNumber << " in procedure " << s.theProcedure;
  o << endl << flush;
  return o;
}

void CodePosition::Dump() const {
  cout << *this;
}


////////////////////////////////////////////////////////////////////////////
// Static Descriptor Base Code
////////////////////////////////////////////////////////////////////////////

// Declare the registry statically, and place a pointer as a static
// class member
static StaticDescriptorList theSddfRegistry;
StaticDescriptorList * StaticDescriptorBase::theRegistry = &theSddfRegistry;

static const char * ID_NAME = "Static ID";
int StaticDescriptorBase::nextId = 1;

RecordDossier * StaticDescriptorBase::dossier = 0;

StaticDescriptorBase::StaticDescriptorBase() : CodePosition() {
  // Set my id
  myId = nextId++;
  // Register myself
  theRegistry->AddElement(this);
}

StaticDescriptorBase::~StaticDescriptorBase() {
  myId = BAD_STATIC_ID_VALUE;
}

void StaticDescriptorBase::SetId(Static_Id i) {
  // Id should be set in constructor
  MkAssert(true,"Should not be setting Static_ID",ABORT);
  myId = i;
}

Static_Id StaticDescriptorBase::GetId() const {
  return myId;
}

ostream & operator << (ostream & o, const StaticDescriptorBase & s) {
  o << *(CodePosition *)(&s);
  
//  operator <<(o,*(CodePosition*)(&s));
  o << "Id " << s.myId << endl;
  return o;
}

void StaticDescriptorBase::Dump() const {
  cout << *this;
}

// These functions put out the local part of the staticdescriptor info
void StaticDescriptorBase::SddfElementInit(StructureDescriptor & structDesc) {

  AddEntryToStructureDesc(structDesc,"ID",ID_NAME,ID_NAME, INTEGER, 0);

  // Kick out Code position info
  CodePosition::SddfElementInit(structDesc);
}

void StaticDescriptorBase::SddfDossierOut(RecordDossier & dossier) const {
  dossier.setValue(ID_NAME,myId);
  CodePosition::SddfDossierOut(dossier);
}

void StaticDescriptorBase::SddfInitDescriptor(PipeWriter & output) {
  if (dossier == 0) {
    // Do the initialization only if it hasn't already been done
    Attributes 		theAttributes;
    theAttributes.clearEntries();
    theAttributes.insert("description","Base Type for SDDF static classes");
    StructureDescriptor structDesc("StaticDescriptorBase",theAttributes);

    StaticDescriptorBase::SddfElementInit(structDesc);

    output.putDescriptor(structDesc,SD_TAG_BASE);
    dossier = new RecordDossier(SD_TAG_BASE,structDesc);
  }
}

void StaticDescriptorBase::SddfDump(PipeWriter & output) const {
  MkAssert(dossier,"Hey, no dossier!",EXIT);
  SddfDossierOut(*dossier);
  output.putData(*dossier);
}

void StaticDescriptorBase::SddfRead(PipeReader &) {
  ShouldNotGetHere;
}

const char * StaticDescriptorBase::GetIdName() const { 
  return ID_NAME;
}


void StaticDescriptorBase::WalkRegistry(PipeWriter & p) {
  theRegistry->SddfDumpList(p);
}

////////////////////////////////////////////////////////////////////////////
// SDDF_Symbolic_Value
////////////////////////////////////////////////////////////////////////////

RecordDossier * SDDF_Symbolic_Value::localDossier = 0;
static const char * SYMBOLIC_VALUE_VALUE_NAME = "Symbolic Value";

SDDF_Symbolic_Value::SDDF_Symbolic_Value() {
  symValue = -1;
  wasSet   = false;
}

SDDF_Symbolic_Value::~SDDF_Symbolic_Value() {
  symValue = -1;  
  wasSet   = false;
}
 
void SDDF_Symbolic_Value::SetSymbolicValue(SDDFint v) {
  symValue = v;
  wasSet = true;
}

SDDFint SDDF_Symbolic_Value::GetSymbolicValue() {
  return symValue;
}

ostream & operator << (ostream & o, const SDDF_Symbolic_Value & s) {
  o << "SDDF_Symbolic_Value\n";
  o << "Value: " << s.symValue << endl;
  return o;
}
 
void SDDF_Symbolic_Value::Dump() const {
  cout << *this;
}

void SDDF_Symbolic_Value::SddfInitDescriptor(PipeWriter & output) {
  if(localDossier ==0) {
    // Only do this once
    Attributes 		theAttributes;
    theAttributes.clearEntries();
    theAttributes.insert("description","FD Static Symbolic Value Record");
    StructureDescriptor structDesc("FDSSym Value",theAttributes);

    // No standard attributes!
    AddEntryToStructureDesc(structDesc,"ID",ID_NAME,ID_NAME, INTEGER, 0);
 
    // Do attributes specific to Decomp
    AddEntryToStructureDesc(structDesc,"Value", SYMBOLIC_VALUE_VALUE_NAME,
			    INTEGER, 0);

    output.putDescriptor(structDesc,SD_TAG_SYMBOLIC_VALUE);
    localDossier = new RecordDossier(SD_TAG_SYMBOLIC_VALUE,structDesc);
  }
}

void SDDF_Symbolic_Value::SddfDump(PipeWriter & output) const {
  // Only output a record if we set a value for it.
  if (wasSet) {
    MkAssert(localDossier,"Hey, no dossier!",EXIT);
    // Do not Output most standard attributes
    Static_Id myId = SDDF_Symbolic_Value::GetId();
    localDossier->setValue(ID_NAME,myId);
    
    localDossier->setValue(SYMBOLIC_VALUE_VALUE_NAME,symValue); 

    output.putData(*localDossier);
  }
}

void SDDF_Symbolic_Value::SddfRead(PipeReader &) {
   ShouldNotGetHere;
}
