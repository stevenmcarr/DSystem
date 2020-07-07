/* $Id: SD_SrcInfo.h,v 1.2 2001/10/12 19:33:02 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// -*- C++ -*-
/* Explanatory one line comment on file nature */
/* $Header: /home/cs/carr/cvsroot/DSystem/src/libs/fortD/performance/staticInfo/SD_SrcInfo.h,v 1.2 2001/10/12 19:33:02 carr Exp $
//
*/
// The tag should exactly match the filename
#ifndef _SD_SrcInfo_h
#define _SD_SrcInfo_h
// All includes and definitions go here. Include only the minimum set required to include this file. Do not include anything that is only required by the coressponding .c file (if there is one)

#include <iostream>
using namespace std;
#include <libs/fortD/performance/staticInfo/SD_Base.h>
#include <libs/fortD/performance/staticInfo/SD_List.h>
#include <libs/support/misc/general.h>

#include <libs/fortD/performance/staticInfo/SD_Decls.h>

///////////////////////////////////////////////////////////////////////////
// SDDF_ProcInfo
///////////////////////////////////////////////////////////////////////////
class SDDF_ProcInfo : public StaticDescriptorBase {
 public:
  SDDF_ProcInfo();
  virtual ~SDDF_ProcInfo();

  void SetProcName(char * Name);
  const char * GetProcName(void) const;

  void SetFileName(char * Name);
  const char * GetFileName(void) const;

  void AddArray(SDDF_ArrayInfo * p);
  void AddAlign(SDDF_AlignInfo * p);
  void AddDist(SDDF_DistInfo * p);

  void SetRecursiveFlag(Boolean r);
  Boolean GetRecursiveFlag(void) const;
  
  //IO operations
  friend  ostream & operator << (ostream & o, const
				 SDDF_ProcInfo & s);
  virtual void Dump() const ;
  static  void SddfInitDescriptor(PipeWriter & output);
  virtual void SddfDump(PipeWriter & output) const;
  virtual void SddfRead(PipeReader & output);

 private:
  
  char * procName;
  char * fileName;

  StaticDescriptorList correspondingArrays;
  StaticDescriptorList correspondingAligns;
  StaticDescriptorList correspondingDists;

  Boolean isRecursive;

  static RecordDossier * localDossier;
};

///////////////////////////////////////////////////////////////////////////
// SDDF_SrcLoopInfo
///////////////////////////////////////////////////////////////////////////

class SDDF_SrcLoopInfo : public StaticDescriptorBase {
 public:	
 
  SDDF_SrcLoopInfo();
  virtual ~SDDF_SrcLoopInfo();

  void AddDepends(SDDF_DependInfo *p);
  void AddSPMDLoops(SDDF_SPMD_LoopInfo *p);
  void AddMessages(SDDF_MessageSend *p);

  //IO operations
  friend  ostream & operator << (ostream & o, const
				 SDDF_SrcLoopInfo & s);
  virtual void Dump() const ;
  static  void SddfInitDescriptor(PipeWriter & output);
  virtual void SddfDump(PipeWriter & output) const;
  virtual void SddfRead(PipeReader & output);


 private:
  StaticDescriptorList correspondingDepends;
  StaticDescriptorList correspondingSPMDLoops;
  StaticDescriptorList correspondingMessages;

  static RecordDossier * localDossier;
};



///////////////////////////////////////////////////////////////////////////
// SDDF_DependInfo
///////////////////////////////////////////////////////////////////////////

// I need to find the fortran D structure and copy it:
enum DependInfoDependType { DEPEND_NONE,DEPEND_DEPENDENCE, DEPEND_OUTPUT, DEPEND_ANTI} ;

class SDDF_DependInfo : public StaticDescriptorBase {
public:
  SDDF_DependInfo();
  virtual ~SDDF_DependInfo();

  void SetSrcPos(Line_Number l, int pos);
  void SetSinkPos(Line_Number l, int pos);
  void SetDependType(DependInfoDependType d);

  //IO operations
  friend  ostream & operator << (ostream & o, const
				 SDDF_DependInfo & s);
  virtual void Dump() const ;
  static  void SddfInitDescriptor(PipeWriter & output);
  virtual void SddfDump(PipeWriter & output) const;
  virtual void SddfRead(PipeReader & output);

private:
  Line_Number srcLine, sinkLine;
  int         srcPos,  sinkPos;
  DependInfoDependType  theType;
  static RecordDossier * localDossier;
};

///////////////////////////////////////////////////////////////////////////
// SDDF_SPMD_LoopInfo
///////////////////////////////////////////////////////////////////////////

class SDDF_SPMD_LoopInfo : public StaticDescriptorBase {
  // Unimplemented at this time;
  static RecordDossier * localDossier;
};

// Don't forget terminal semicolon on classes!
#endif




