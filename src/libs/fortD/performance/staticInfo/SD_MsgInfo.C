/* $Id: SD_MsgInfo.C,v 1.1 1997/03/11 14:29:04 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// -*- C++ -*- 
/* One line expanatory comment */ 
/* $Header: /home/cs/carr/cvsroot/DSystem/src/libs/fortD/performance/staticInfo/SD_MsgInfo.C,v 1.1 1997/03/11 14:29:04 carr Exp $
   //
   */

static const char * RCS_ID = "$Id: SD_MsgInfo.C,v 1.1 1997/03/11 14:29:04 carr Exp $";
#define MKASSERT
#define ASSERT_FILE_VERSION RCS_ID


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
#include <libs/fortD/performance/staticInfo/SD_Globals.i>

#include <Attributes.h>
#include <RecordDossier.h>
#include <StructureDescriptor.h>
#include <PipeReader.h>
#include <PipeWriter.h>
#include <libs/fortD/performance/staticInfo/StaticSDDF.h>

///////////////////////////////////////////////////////////////////////////
// SDDF_MessageSend
///////////////////////////////////////////////////////////////////////////

RecordDossier * SDDF_MessageSend::localDossier = 0;

static const char * MSGSEND_SEND_WAIT_ID_NAME = "Send Wait ID";
static const char * MSGSEND_RECV_ID_NAME = "Recv ID";
static const char * MSGSEND_RECV_WAIT_ID_NAME = "Recv Wait ID";

static const char * MSGSEND_ENCL_LOOP_ID_NAME = "Enclosed Loop ID";
static const char * MSGSEND_ASSOC_LOOP_ID_NAME = "Assoc Loop ID";

static const char * MSGSEND_MESG_OPER_TYPE_NAME = "Mesg Oper Type";
static const char * MSGSEND_SYM_MESG_TYPE_NAME = "Sym Mesg Type";
static const char * MSGSEND_SYM_MESG_PID_NAME = "Sym Mesg Pid";
static const char * MSGSEND_SYM_MESG_SIZE_NAME = "Sym Mesg Size";

static const char * MSGSEND_BUFFERED_FLAG_NAME = "Buffered";

static const char * MSGSEND_ARRAY_ID_NAME = "Array ID";
static const char * MSGSEND_SYM_ARRAY_LB_NAME = "Sym Array LB";
static const char * MSGSEND_SYM_ARRAY_UB_NAME = "Sym Array UB";
static const char * MSGSEND_SYM_ARRAY_STEP_NAME = "Sym Array Step";
static const char * MSGSEND_DEPEND_ID_NAME = "Depend ID";
static const char * MSGSEND_ENCLOSED_LOOP_ID_NAME = "Enclosed Loop ID2";

void SDDF_MessageSend::ZeroInfo() {
  correspondingSendWait =0;
  correspondingRecv	=0;
  correspondingRecvWait	=0;

  enclosedLoop		=0;
  assocLoop		=0;

  theType =MESSAGESEND_INVALID;

  isBuffered   		=false;
  
  symMesgType		=0;
  symMesgPid		=0;
  symMesgSize		=0;
}

SDDF_MessageSend::SDDF_MessageSend() {
  ZeroInfo();
}

SDDF_MessageSend::~SDDF_MessageSend() {
  ZeroInfo();
}

void SDDF_MessageSend::SetSendWait(SDDF_MessageSendWait *p) {
  correspondingSendWait =p;
}

void SDDF_MessageSend::SetRecv(SDDF_MessageRecv *p) {
  correspondingRecv = p;
}
void SDDF_MessageSend::SetRecvWait(SDDF_MessageRecvWait *p) {
  correspondingRecvWait	=p;
}

void SDDF_MessageSend::SetEnclosedLoop(SDDF_SrcLoopInfo *p) {
  enclosedLoop=p;
}
void SDDF_MessageSend::SetAssocLoop(SDDF_SrcLoopInfo *p) {
  assocLoop=p;
}

void SDDF_MessageSend::SetMesgOperType(Message_Send_Oper_Type t) {
  theType =t;
}

void SDDF_MessageSend::SetMesgType(SDDF_Symbolic_Value *p) {
  symMesgType =p;
}
void SDDF_MessageSend::SetMesgPid(SDDF_Symbolic_Value *p) {
  symMesgPid =p;
}

void SDDF_MessageSend::SetMesgSize(SDDF_Symbolic_Value *p) {
  symMesgSize =p;
}

void SDDF_MessageSend::SetBufferedFlag(Boolean b) {
  isBuffered =b;
}

Boolean SDDF_MessageSend::GetBufferedFlag(void) const {
  return isBuffered;
}

void SDDF_MessageSend::AddArray(SDDF_ArrayInfo * p) {
  correspondingArrays.AddElement(p);
}	
void SDDF_MessageSend::AddArrayUB(SDDF_Symbolic_Value *p) {
  arrayUB.AddElement(p);
}
void SDDF_MessageSend::AddArrayLB(SDDF_Symbolic_Value *p)  {
  arrayLB.AddElement(p);
}
void SDDF_MessageSend::AddArrayStep(SDDF_Symbolic_Value *p)  {
  arrayStep.AddElement(p);
}

void SDDF_MessageSend::AddDepend(SDDF_DependInfo *p)  {
  dependID.AddElement(p);
}

void SDDF_MessageSend::AddEnclosedLoop(SDDF_SrcLoopInfo *p) {
  enclosedLoopID.AddElement(p);
}

ostream & operator << (ostream & o, const SDDF_MessageSend & s) {
  o << "SDDF_MessageSend\n";
  o << "SendWait " << ResolveStaticDescriptorId(s.correspondingSendWait) << endl;
  o << "Recv " << ResolveStaticDescriptorId(s.correspondingRecv) << endl;
  o << "RecvWait " << ResolveStaticDescriptorId(s.correspondingRecvWait) << endl;
  o << "EnclLoop " << ResolveStaticDescriptorId(s.enclosedLoop) << endl;
  o << "AssocLoop "  << s.assocLoop << endl;
  return o;
}

void SDDF_MessageSend::Dump() const { 
  cout << *this;
};

void SDDF_MessageSend::SddfInitDescriptor(PipeWriter & output) {
  if(localDossier ==0) {
    // Only do this once
    Attributes 		theAttributes;
    theAttributes.clearEntries();
    theAttributes.insert("description","FD Static Message Send Record");
    StructureDescriptor structDesc("FDStat Mesg Send",theAttributes);

    // Do standard attributes
    StaticDescriptorBase::SddfElementInit(structDesc);

    // Insert msgsend attributes
    AddEntryToStructureDesc(structDesc,"Pointer",
			    MSGSEND_SEND_WAIT_ID_NAME,INTEGER,0);
    AddEntryToStructureDesc(structDesc,"Pointer",
			    MSGSEND_RECV_ID_NAME,INTEGER,0);
    AddEntryToStructureDesc(structDesc,"Pointer",MSGSEND_RECV_WAIT_ID_NAME,INTEGER,0);

    AddEntryToStructureDesc(structDesc,"Pointer",MSGSEND_ENCL_LOOP_ID_NAME,INTEGER,0);
    AddEntryToStructureDesc(structDesc,"Pointer",MSGSEND_ASSOC_LOOP_ID_NAME,INTEGER,0);

    AddEntryToStructureDesc(structDesc,"Type",MSGSEND_MESG_OPER_TYPE_NAME,INTEGER,0);
    AddEntryToStructureDesc(structDesc,"SymPtr",MSGSEND_SYM_MESG_TYPE_NAME,INTEGER,0);
    AddEntryToStructureDesc(structDesc,"SymPtr",MSGSEND_SYM_MESG_PID_NAME,INTEGER,0);
    AddEntryToStructureDesc(structDesc,"SymPtr",MSGSEND_SYM_MESG_SIZE_NAME,INTEGER,0);
    AddEntryToStructureDesc(structDesc,"Boolean",MSGSEND_BUFFERED_FLAG_NAME,INTEGER,0);
   
    // Array Info
    AddEntryToStructureDesc(structDesc,"Pointer",MSGSEND_ARRAY_ID_NAME,INTEGER,1);
    AddEntryToStructureDesc(structDesc,"SymPtr",MSGSEND_SYM_ARRAY_LB_NAME,INTEGER,1);
    AddEntryToStructureDesc(structDesc,"SymPtr",MSGSEND_SYM_ARRAY_UB_NAME,INTEGER,1);
    AddEntryToStructureDesc(structDesc,"SymPtr",MSGSEND_SYM_ARRAY_STEP_NAME,INTEGER,1);
    AddEntryToStructureDesc(structDesc,"Pointer",MSGSEND_DEPEND_ID_NAME,INTEGER,1);
    AddEntryToStructureDesc(structDesc,"Pointer",MSGSEND_ENCLOSED_LOOP_ID_NAME,INTEGER,1);
    
    output.putDescriptor(structDesc,SD_TAG_SEND);
    localDossier = new RecordDossier(SD_TAG_SEND,structDesc);
  }
}

void SDDF_MessageSend::SddfDump(PipeWriter & output) const{
  MkAssert(localDossier,"Hey, no dossier!",EXIT);
  // Output standard attributes
  StaticDescriptorBase::SddfDossierOut(*localDossier);
  
  localDossier->setValue(MSGSEND_SEND_WAIT_ID_NAME,
			 ResolveStaticDescriptorId(correspondingSendWait));
  localDossier->setValue(MSGSEND_RECV_ID_NAME,
			 ResolveStaticDescriptorId(correspondingRecv));
  localDossier->setValue(MSGSEND_RECV_WAIT_ID_NAME,
			 ResolveStaticDescriptorId(correspondingRecvWait));
  localDossier->setValue(MSGSEND_ENCL_LOOP_ID_NAME,
			 ResolveStaticDescriptorId(enclosedLoop));
  localDossier->setValue(MSGSEND_ASSOC_LOOP_ID_NAME,
			 ResolveStaticDescriptorId(assocLoop));

  localDossier->setValue(MSGSEND_MESG_OPER_TYPE_NAME,
			 int(theType));
  localDossier->setValue(MSGSEND_SYM_MESG_TYPE_NAME,
			 ResolveStaticDescriptorId(symMesgType));
  localDossier->setValue(MSGSEND_SYM_MESG_PID_NAME,
			 ResolveStaticDescriptorId(symMesgPid));
  localDossier->setValue(MSGSEND_SYM_MESG_SIZE_NAME,
			 ResolveStaticDescriptorId(symMesgSize));
  localDossier->setValue(MSGSEND_BUFFERED_FLAG_NAME,
			 int(isBuffered));
  
  Array *arrayIDArray = localDossier->getArrayP(MSGSEND_ARRAY_ID_NAME);
  correspondingArrays.ResolveToArray(arrayIDArray);

  Array *arrayLBArray = localDossier->getArrayP(MSGSEND_SYM_ARRAY_LB_NAME);
  arrayLB.ResolveToArray(arrayLBArray);
  Array *arrayUBArray = localDossier->getArrayP(MSGSEND_SYM_ARRAY_UB_NAME);
  arrayUB.ResolveToArray(arrayUBArray);
  Array *arrayStepArray = localDossier->getArrayP(MSGSEND_SYM_ARRAY_STEP_NAME);
  arrayStep.ResolveToArray(arrayStepArray);

  Array *dependIDArray = localDossier->getArrayP(MSGSEND_DEPEND_ID_NAME);
  dependID.ResolveToArray(dependIDArray);
  Array *enclosedLoopIDArray = localDossier->getArrayP(MSGSEND_ENCLOSED_LOOP_ID_NAME);
  enclosedLoopID.ResolveToArray(enclosedLoopIDArray);

  output.putData(*localDossier);
}

void SDDF_MessageSend::SddfRead(PipeReader &) { 
  ShouldNotGetHere 
}

	
///////////////////////////////////////////////////////////////////////////
// SDDF_MessageRecv;
///////////////////////////////////////////////////////////////////////////   

RecordDossier * SDDF_MessageRecv::localDossier = 0;

SDDF_MessageRecv::SDDF_MessageRecv(){
  correspondingSend =0;
  correspondingRecvWait =0;
}

SDDF_MessageRecv::~SDDF_MessageRecv(){
  correspondingSend =0;
  correspondingRecvWait =0;
}

void SDDF_MessageRecv::SetMessageSend(SDDF_MessageSend *p){
  correspondingSend = p;
}

void SDDF_MessageRecv::SetMessageRecvWait(SDDF_MessageRecvWait *p){
  correspondingRecvWait = p;
}

//IO operations
ostream & operator << (ostream & o, const SDDF_MessageRecv & s) {
  o << "SDDF_MessageRecv\n";
  o << "Send " << s.correspondingSend << endl;
  o << "RecvWait " << s.correspondingRecvWait << endl;
  return o;
}

void SDDF_MessageRecv::Dump() const {
  cout << *this;
}

static const char * MSGRECV_SEND_ID_NAME 	= "Mesg ID";
static const char * MSGRECV_RECV_WAIT_ID_NAME	= "Recv Wait ID";

void SDDF_MessageRecv::SddfInitDescriptor(PipeWriter & output){
  if(localDossier ==0) {
    // Only do this once
    Attributes 		theAttributes;
    theAttributes.clearEntries();
    theAttributes.insert("description","FD Static Message Receive Record");
    StructureDescriptor structDesc("FDStat Mesg Recv",theAttributes);

    // Do standard attributes
    StaticDescriptorBase::SddfElementInit(structDesc);

    AddEntryToStructureDesc(structDesc,"Pointer",
			    MSGRECV_SEND_ID_NAME,INTEGER,0);
    AddEntryToStructureDesc(structDesc,"Pointer",
			    MSGRECV_RECV_WAIT_ID_NAME,INTEGER,0);

    output.putDescriptor(structDesc,SD_TAG_RECV);
    localDossier = new RecordDossier(SD_TAG_RECV,structDesc);
  }
}

void SDDF_MessageRecv::SddfDump(PipeWriter & output) const { 
  MkAssert(localDossier,"Hey, no dossier!",EXIT);
  // Output standard attributes
  StaticDescriptorBase::SddfDossierOut(*localDossier);

  localDossier->setValue(MSGRECV_SEND_ID_NAME,
			 ResolveStaticDescriptorId(correspondingSend));
  localDossier->setValue(MSGRECV_RECV_WAIT_ID_NAME,
			 ResolveStaticDescriptorId(correspondingRecvWait));

  output.putData(*localDossier);
}

void SDDF_MessageRecv::SddfRead(PipeReader &){
  ShouldNotGetHere;
}	   

///////////////////////////////////////////////////////////////////////////
// SDDF_MessageSendWait{}
///////////////////////////////////////////////////////////////////////////

RecordDossier * SDDF_MessageSendWait::localDossier = 0;

SDDF_MessageSendWait::SDDF_MessageSendWait() {
  correspondingSend =0;
  correspondingLoop =0;
}

SDDF_MessageSendWait::~SDDF_MessageSendWait(){
  correspondingSend =0;
  correspondingLoop =0;
}

void SDDF_MessageSendWait::SetMessageSend(SDDF_MessageSend *p){
  correspondingSend =p;
}
void SDDF_MessageSendWait::SetSPMDLoop(SDDF_SPMD_LoopInfo *p){
  correspondingLoop =p;
}

//IO operations
ostream & operator << (ostream & o, const SDDF_MessageSendWait & s){
  o << "SDDF_MessageSendWait\n";
  o << "Send " << s.correspondingSend << endl;
  o << "Loop " << s.correspondingLoop << endl;
  return o;
}

void SDDF_MessageSendWait::Dump() const {
  cout << *this;
}

static const char * MESGSENDWAIT_SEND_ID_NAME = "Mesg ID";
static const char * MESGSENDWAIT_SPMDLOOP_ID_NAME = "SPMDLoop ID";

void SDDF_MessageSendWait::SddfInitDescriptor(PipeWriter & output) {
  if(localDossier ==0) {
    // Only do this once
    Attributes 		theAttributes;
    theAttributes.clearEntries();
    theAttributes.insert("description","FD Static Message Send Wait Record");
    StructureDescriptor structDesc("FDStat Mesg Send Wait",theAttributes);

    // Do standard attributes
    StaticDescriptorBase::SddfElementInit(structDesc);

    AddEntryToStructureDesc(structDesc,"Pointer",
			    MESGSENDWAIT_SEND_ID_NAME,INTEGER,0);
    AddEntryToStructureDesc(structDesc,"Pointer",
			    MESGSENDWAIT_SPMDLOOP_ID_NAME,INTEGER,0);

    output.putDescriptor(structDesc,SD_TAG_SEND_WAIT);
    localDossier = new RecordDossier(SD_TAG_SEND_WAIT,structDesc);
  }
}

void SDDF_MessageSendWait::SddfDump(PipeWriter & output) const {
  // Output standard attributes
  StaticDescriptorBase::SddfDossierOut(*localDossier);

  localDossier->setValue(MESGSENDWAIT_SEND_ID_NAME,
			 ResolveStaticDescriptorId(correspondingSend));
  localDossier->setValue(MESGSENDWAIT_SPMDLOOP_ID_NAME,
			 ResolveStaticDescriptorId(correspondingLoop));

  output.putData(*localDossier);
}

void SDDF_MessageSendWait::SddfRead(PipeReader &) {
  ShouldNotGetHere
}	   

///////////////////////////////////////////////////////////////////////////
// SDDF_MessageRecvWait
///////////////////////////////////////////////////////////////////////////

RecordDossier * SDDF_MessageRecvWait::localDossier = 0;

SDDF_MessageRecvWait:: SDDF_MessageRecvWait() {
  correspondingSend =0;
  correspondingRecv =0;
  correspondingSendWaitLoop =0;
  correspondingRecvWaitLoop =0;
};

SDDF_MessageRecvWait::~SDDF_MessageRecvWait() {
  correspondingSend =0;
  correspondingRecv =0;
  correspondingSendWaitLoop =0;
  correspondingRecvWaitLoop =0;
};

void SDDF_MessageRecvWait::SetMessageSend(SDDF_MessageSend *p) {
  correspondingSend =p;
}

void SDDF_MessageRecvWait::SetMessageRecv(SDDF_MessageRecv *p) {
  correspondingRecv =p;
}
void SDDF_MessageRecvWait::SetMessageSendWaitLoop(SDDF_SPMD_LoopInfo *p){
  correspondingSendWaitLoop =p;
}
void SDDF_MessageRecvWait::SetMessageRecvWaitLoop(SDDF_SPMD_LoopInfo *p){
  correspondingRecvWaitLoop =p;
}

  //Io operations
ostream & operator << (ostream & o, const SDDF_MessageRecvWait & s){
  o << "SDDF_MessageRecvWait\n";
  o << "Send " << s.correspondingSend << endl;
  o << "Recv " << s.correspondingRecv << endl;
  o << "SendWait " << s.correspondingSendWaitLoop << endl;
  o << "RecvWait " << s.correspondingRecvWaitLoop << endl;
  return o;
}

void SDDF_MessageRecvWait::Dump() const {
  cout << *this;
}

static const char * MESGRECVWAIT_SEND_ID_NAME = "Mesg ID";
static const char * MESGRECVWAIT_RECV_ID_NAME = "Recv ID";
static const char * MESGRECVWAIT_SPMDLOOP_SEND_ID_NAME = "SPMDLoop SendWait ID";
static const char * MESGRECVWAIT_SPMDLOOP_RECV_ID_NAME = "SPMDLoop RecvWait ID";

void SDDF_MessageRecvWait::SddfInitDescriptor(PipeWriter & output) {
  if(localDossier ==0) {
    // Only do this once
    Attributes 		theAttributes;
    theAttributes.clearEntries();
    theAttributes.insert("description","FD Static Message Receive Wait Record");
    StructureDescriptor structDesc("FDStat Mesg Recv Wait",theAttributes);

    // Do standard attributes
    StaticDescriptorBase::SddfElementInit(structDesc);

    AddEntryToStructureDesc(structDesc,"Pointer",
			    MESGRECVWAIT_SEND_ID_NAME,INTEGER,0);
    AddEntryToStructureDesc(structDesc,"Pointer",
			    MESGRECVWAIT_RECV_ID_NAME,INTEGER,0);
    AddEntryToStructureDesc(structDesc,"Pointer",
			    MESGRECVWAIT_SPMDLOOP_SEND_ID_NAME,INTEGER,0);
    AddEntryToStructureDesc(structDesc,"Pointer",
			    MESGRECVWAIT_SPMDLOOP_RECV_ID_NAME,INTEGER,0);

    output.putDescriptor(structDesc,SD_TAG_RECV_WAIT);
    localDossier = new RecordDossier(SD_TAG_RECV_WAIT,structDesc);
  }
}

void SDDF_MessageRecvWait::SddfDump(PipeWriter & output) const {
  // Output standard attributes
  StaticDescriptorBase::SddfDossierOut(*localDossier);

  localDossier->setValue(MESGRECVWAIT_SEND_ID_NAME,
			 ResolveStaticDescriptorId(correspondingSend));
  localDossier->setValue(MESGRECVWAIT_RECV_ID_NAME,
			 ResolveStaticDescriptorId(correspondingRecv));
  localDossier->setValue(MESGRECVWAIT_SPMDLOOP_SEND_ID_NAME,
			 ResolveStaticDescriptorId(correspondingSendWaitLoop));
  localDossier->setValue(MESGRECVWAIT_SPMDLOOP_RECV_ID_NAME,
			 ResolveStaticDescriptorId(correspondingRecvWaitLoop));

  output.putData(*localDossier);
}

void SDDF_MessageRecvWait::SddfRead(PipeReader &){
  ShouldNotGetHere;
}	   
