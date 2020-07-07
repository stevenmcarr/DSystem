/* $Id: SD_MsgInfo.h,v 1.2 2001/10/12 19:33:02 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// -*- C++ -*-
/* Explanatory one line comment on file nature */
/* $Header: /home/cs/carr/cvsroot/DSystem/src/libs/fortD/performance/staticInfo/SD_MsgInfo.h,v 1.2 2001/10/12 19:33:02 carr Exp $
//
*/
// The tag should exactly match the filename
#ifndef _SD_MsgInfo_h
#define _SD_MsgInfo_h
// All includes and definitions go here. Include only the minimum set required to include this file. Do not include anything that is only required by the coressponding .c file (if there is one)

#include <iostream>
using namespace std;
#include <libs/support/misc/general.h>
#include <libs/fortD/performance/staticInfo/SD_Base.h>
#include <libs/fortD/performance/staticInfo/SD_List.h>
#include <libs/fortD/performance/staticInfo/SD_Decls.h>


///////////////////////////////////////////////////////////////////////////
// SDDF_MessageSend;
///////////////////////////////////////////////////////////////////////////

enum Message_Send_Oper_Type { MESSAGESEND_INVALID =0,
			      MESSAGESEND_SYNC_SEND_SYNC_RECV = 1, 
			      MESSAGESEND_SYNC_SEND_ASYNC_RECV = 2, 
			      MESSAGESEND_ASYNC_SEND_SYNC_RECV = 3, 
			      MESSAGESEND_ASYNC_SEND_ASYNC_RECV = 4, 
			      MESSAGESEND_GLOBAL_BCAST = 5, 
			      MESSAGESEND_GLOBAL_MCAST = 6, 
			      MESSAGESEND_GLOBAL_SUM = 7, 
			      MESSAGESEND_GLOBAL_PROD = 8, 
			      MESSAGESEND_GLOBAL_MIN = 9, 
			      MESSAGESEND_GLOBAL_MAX = 10 };

class SDDF_MessageSend : public StaticDescriptorBase {
public:
  SDDF_MessageSend();
  virtual ~SDDF_MessageSend();

  void SetSendWait(SDDF_MessageSendWait *p);
  void SetRecv(SDDF_MessageRecv *p);
  void SetRecvWait(SDDF_MessageRecvWait *p);
  void SetEnclosedLoop(SDDF_SrcLoopInfo *p);
  void SetAssocLoop(SDDF_SrcLoopInfo *p);
  
  void SetMesgOperType(Message_Send_Oper_Type t);
  void SetMesgType(SDDF_Symbolic_Value *p);
  void SetMesgPid(SDDF_Symbolic_Value *p);
  void SetMesgSize(SDDF_Symbolic_Value *p);
  
  void SetBufferedFlag(Boolean b);
  Boolean GetBufferedFlag(void) const;

  void AddArray(SDDF_ArrayInfo * p);
  void AddArrayUB(SDDF_Symbolic_Value *p);
  void AddArrayLB(SDDF_Symbolic_Value *p);
  void AddArrayStep(SDDF_Symbolic_Value *p);
  void AddDepend(SDDF_DependInfo *p);
  void AddEnclosedLoop(SDDF_SrcLoopInfo *p);
  
  //IO operations
  friend  ostream & operator << (ostream & o, const SDDF_MessageSend & s);
  virtual void Dump() const ;
  static  void SddfInitDescriptor(PipeWriter & output);
  virtual void SddfDump(PipeWriter & output) const;
  virtual void SddfRead(PipeReader & output);	   

private:
  void ZeroInfo();
  SDDF_MessageSendWait 	*correspondingSendWait;
  SDDF_MessageRecv 	*correspondingRecv;
  SDDF_MessageRecvWait 	*correspondingRecvWait;

  SDDF_SrcLoopInfo      * enclosedLoop;
  SDDF_SrcLoopInfo 	* assocLoop;

  Message_Send_Oper_Type theType;
  
  SDDF_Symbolic_Value   * symMesgType;
  SDDF_Symbolic_Value	* symMesgPid;
  SDDF_Symbolic_Value 	* symMesgSize;

  Boolean isBuffered;

  StaticDescriptorList correspondingArrays;
  
  StaticDescriptorList arrayLB;
  StaticDescriptorList arrayUB;
  StaticDescriptorList arrayStep;
  
  StaticDescriptorList dependID;
  StaticDescriptorList enclosedLoopID;
  static RecordDossier * localDossier;
};

///////////////////////////////////////////////////////////////////////////
// SDDF_MessageRecv;
///////////////////////////////////////////////////////////////////////////

class SDDF_MessageRecv : public StaticDescriptorBase {
public:
  SDDF_MessageRecv();
  virtual ~SDDF_MessageRecv();

  void SetMessageSend(SDDF_MessageSend *p);
  void SetMessageRecvWait(SDDF_MessageRecvWait *p);

  //IO operations
  friend  ostream & operator << (ostream & o, const SDDF_MessageRecv & s);
  virtual void Dump() const ;
  static  void SddfInitDescriptor(PipeWriter & output);
  virtual void SddfDump(PipeWriter & output) const;
  virtual void SddfRead(PipeReader & output);	   
private:
  SDDF_MessageSend * correspondingSend;  
  SDDF_MessageRecvWait    * correspondingRecvWait;
  static RecordDossier * localDossier;
};

///////////////////////////////////////////////////////////////////////////
// SDDF_MessageSendWait;
///////////////////////////////////////////////////////////////////////////
class SDDF_MessageSendWait : public StaticDescriptorBase {
public:
  SDDF_MessageSendWait();
  virtual ~SDDF_MessageSendWait();

  void SetMessageSend(SDDF_MessageSend *p);

  void SetSPMDLoop(SDDF_SPMD_LoopInfo *p);

  //IO operations
  friend  ostream & operator << (ostream & o, const SDDF_MessageSendWait & s);
  virtual void Dump() const ;
  static  void SddfInitDescriptor(PipeWriter & output);
  virtual void SddfDump(PipeWriter & output) const;
  virtual void SddfRead(PipeReader & output);	   
private:
  SDDF_MessageSend * correspondingSend;  
  SDDF_SPMD_LoopInfo    * correspondingLoop;
  static RecordDossier * localDossier;
};

///////////////////////////////////////////////////////////////////////////
// SDDF_MessageRecvWait;
///////////////////////////////////////////////////////////////////////////
class SDDF_MessageRecvWait : public StaticDescriptorBase {
public:
  SDDF_MessageRecvWait();
  virtual ~SDDF_MessageRecvWait();

  void SetMessageSend(SDDF_MessageSend *p);
  void SetMessageRecv(SDDF_MessageRecv *p);
  void SetMessageRecvWaitLoop(SDDF_SPMD_LoopInfo *p);
  void SetMessageSendWaitLoop(SDDF_SPMD_LoopInfo *p);

  //Io operations
  friend  ostream & operator << (ostream & o, const SDDF_MessageRecvWait & s);
  virtual void Dump() const ;
  static  void SddfInitDescriptor(PipeWriter & output);
  virtual void SddfDump(PipeWriter & output) const;
  virtual void SddfRead(PipeReader & output);	   
private:
  SDDF_MessageSend * correspondingSend;  
  SDDF_MessageRecv * correspondingRecv;
  SDDF_SPMD_LoopInfo   * correspondingSendWaitLoop;
  SDDF_SPMD_LoopInfo   * correspondingRecvWaitLoop;

  static RecordDossier * localDossier;
};

// Don't forget terminal semicolon on classes!
#endif




