/* $Id: SD_DataInfo.h,v 1.1 1997/03/11 14:29:00 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// -*- C++ -*-
/* SDDF descriptors for data items. */
/* $Header: /home/cs/carr/cvsroot/DSystem/src/libs/fortD/performance/staticInfo/SD_DataInfo.h,v 1.1 1997/03/11 14:29:00 carr Exp $
//
*/
// The tag should exactly match the filename
#ifndef _SD_DataInfo_h
#define _SD_DataInfo_h
// All includes and definitions go here. Include only the minimum set required to include this file. Do not include anything that is only required by the coressponding .c file (if there is one)

#include <stream.h>
#include <libs/fortD/performance/staticInfo/SD_Base.h>
#include <libs/fortD/performance/staticInfo/SD_List.h>
#include <libs/fortD/performance/staticInfo/SD_Decls.h>
#include <libs/support/vectors/PointerVector.h>
#include <libs/support/misc/general.h>
 
///////////////////////////////////////////////////////////////////////////
// SDDF_DecompInfo
///////////////////////////////////////////////////////////////////////////
 
class SDDF_DecompInfo : public StaticDescriptorBase {
 public:
  SDDF_DecompInfo();
  virtual ~SDDF_DecompInfo();
 
  void          SetName(char * Name);
  const char *  GetName(void) const;
 
  void          SetDimension(int Dim);
  int           GetDimension(void) const;
  
  void          SetDistributedFlag(Boolean d);
  Boolean          GetDistributedFlag(void) const;
  
  void AddAlign(SDDF_AlignInfo * p);
  void AddDist(SDDF_DistInfo * p);
  
 
  //IO operations
  friend  ostream & operator << (ostream & o, const
                                 SDDF_DecompInfo & s);
  virtual void Dump() const ;
  static  void SddfInitDescriptor(PipeWriter & output);
  virtual void SddfDump(PipeWriter & output) const;
  virtual void SddfRead(PipeReader & output);
 
 private:
  
  char * decompName;
  int    decompDim;
  
  StaticDescriptorList correspondingAligns;
  StaticDescriptorList correspondingDists;
  
  Boolean isDynamicDist;
 
  static RecordDossier * localDossier;
};
 
///////////////////////////////////////////////////////////////////////////
// SDDF_DistInfo
///////////////////////////////////////////////////////////////////////////
 
enum Dist_Info_Dist_Type { DISTINFO_NOT_DISTRIBUTED, DISTINFO_BLOCK,
                             DISTINFO_CYCLIC, DISTINFO_BLOCK_CYCLIC};
 
enum DistInfoDimensionDeclInfoIndex { DISTD_THE_TYPE, DISTD_BLOCK_SIZE, DISTD_NUM_PROC, DISTD_INTERNAL_LOCAL_ARRAY_SIZE, DISTD_BOUNDARY_LOCAL_ARRAY_SIZE, DISTD_MIN_GLOBAL_INDEX, DISTD_MAX_GLOBAL_INDEX, DISTD_STEP_GLOBAL_INDEX};
 
struct DistInfoDimensionDeclInfo {
  DistInfoDimensionDeclInfo();
  ~DistInfoDimensionDeclInfo();
  
  Dist_Info_Dist_Type theType;
  int blockSize;
  int numProc;
  int internalLocalArraySize;
  int boundaryLocalArraySize;
  int minGlobalIndex;
  int maxGlobalIndex;
  int stepGlobalIndex;
 
};
 
ostream & operator << (ostream & o, const DistInfoDimensionDeclInfo & d);
 
class SDDF_DistInfo : public StaticDescriptorBase {
public:
  SDDF_DistInfo();
  virtual ~SDDF_DistInfo();
 
  void SetDecomp(SDDF_DecompInfo * d);
  void AddDimensionInfo(DistInfoDimensionDeclInfo & d);
  void AddMessage(SDDF_MessageSend * s);
 
  //IO operations
  friend  ostream & operator << (ostream & o, const
                                 SDDF_DistInfo & s);
  virtual void Dump() const ;
  static  void SddfInitDescriptor(PipeWriter & output);
  virtual void SddfDump(PipeWriter & output) const;
  virtual void SddfRead(PipeReader & output);      
  
private:
  
  
  SDDF_DecompInfo * correspondingDecomp;
  StaticDescriptorList correspondingMessages; 
  VPDlist dimensionInfo;
 
  static RecordDossier * localDossier;
};
 
///////////////////////////////////////////////////////////////////////////
// SDDF_AlignInfo
///////////////////////////////////////////////////////////////////////////
 
enum Align_Info_Align_Type {ALIGNINFO_UNKNOWN, ALIGNINFO_PERFECT,
                              ALIGNINFO_OFFSET, ALIGNINFO_COEFF,
                              ALIGNINFO_CONST };
 
struct AlignInfoDimensionInfo {
  Align_Info_Align_Type theType;
  int offset;
  int coeff;
};
 
class SDDF_AlignInfo : public StaticDescriptorBase {
 public:
  SDDF_AlignInfo();   
  virtual ~SDDF_AlignInfo();
 
  void SetArray(SDDF_ArrayInfo * a);
  void SetDecomp(SDDF_DecompInfo * d);
  
  void AddDimensionInfo(AlignInfoDimensionInfo & a);
  void AddMessage(SDDF_MessageSend * s);
  
  //IO operations
  friend  ostream & operator << (ostream & o, const SDDF_AlignInfo & s);
  virtual void Dump() const ;
  static  void SddfInitDescriptor(PipeWriter & output);
  virtual void SddfDump(PipeWriter & output) const;
  virtual void SddfRead(PipeReader & output);      
 private:
  SDDF_ArrayInfo * correspondingArray;
  SDDF_DecompInfo * correspondingDecomp;
  
  VPDlist dimensionInfo;
 
  StaticDescriptorList correspondingMessages; 
 
  static RecordDossier * localDossier;
};
 
 
///////////////////////////////////////////////////////////////////////////
// SDDF_ArrayInfo
///////////////////////////////////////////////////////////////////////////
 
class SDDF_ArrayInfo : public StaticDescriptorBase {
 public:
  SDDF_ArrayInfo();   
  virtual ~SDDF_ArrayInfo();
 
  void          SetName(char * Name);
  const char *  GetName(void) const;
 
  // Set dimension must be called before any SetLocalLB or SetLocalUB calls
  void          SetDimension(int Dim);
  int           GetDimension(void) const;
 
  void          SetNumProcs(int i);
 
  void          AddLocalLB(SDDFint bound);
  void          AddLocalUB(SDDFint bound);
 
  // Need to do something about local LB's and UB's
 
  void AddAlign(SDDF_AlignInfo *p);
  void AddMessage(SDDF_MessageSend *p);
  
  //IO operations
  friend  ostream & operator << (ostream & o, const
                                 SDDF_ArrayInfo & s);
  virtual void Dump() const ;
  static  void SddfInitDescriptor(PipeWriter & output);
  virtual void SddfDump(PipeWriter & output) const;
  virtual void SddfRead(PipeReader & input);       
 private:
  
  char * arrayName;
  SDDFint    arrayDim;
  int    numProcs;
 
  PointerVector lowerBounds;
  PointerVector upperBounds;
 
  StaticDescriptorList correspondingAligns;
  StaticDescriptorList correspondingMessages; 
 
  static RecordDossier * localDossier;
};
 
 
 
 
 
// Don't forget terminal semicolon on classes!
#endif
