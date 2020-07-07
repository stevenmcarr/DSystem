/* $Id: SD_Base.h,v 1.2 2001/10/12 19:33:01 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// This may look like C code, but it is really -*- C++ -*-
// This file covers definition of useful classes. (At least until they
//    are specialized a bit) 
//

#ifndef _SD_Base_h
#define _SD_Base_h

#include <iostream>
using namespace std;
//#include "PipeWriter.h"
//#include "PipeReader.h"
//#include "RecordDossier.h"
//#include "StructureDescriptor.h"

class PipeWriter;
class PipeReader;
class RecordDossier;
class StructureDescriptor;

#include <libs/support/misc/general.h>

// This is intended to provide a hook to hang the standard io
// functionality of the descriptors. Has no data.

typedef int SDDFint;

class SDDF_ProcInfo;
class StaticDescriptorList;

class SddfIoObject {
public:
  SddfIoObject() {};
  virtual ~SddfIoObject() {};
  virtual void Dump() const =0;
//  friend  ostream & operator << (SddfIoObject s, ostream & o);
  // Write an element
  virtual void SddfDossierOut(RecordDossier &) const =0 ;
  virtual void SddfRead(PipeReader &) =0;
};

// Code Position. May be modifed to take a set. Right now is only one value 

typedef long Line_Number;

class CodePosition : public SddfIoObject {
public:
  CodePosition();
  CodePosition(Line_Number n, SDDF_ProcInfo * procedure);
  virtual ~CodePosition();
  
  void  SetPosition(Line_Number n, SDDF_ProcInfo * procedure);
  Line_Number 		GetLine(int elementNo) const;
  SDDF_ProcInfo *  	GetProc(int elementNo) const;

  // Right now, this only replaces the position stored
  void AddPosition(Line_Number n, SDDF_ProcInfo * procedure);
  
  int Num_Elements() const ;
  
  // Input and output operations.
  friend  ostream & operator << (ostream & o, const CodePosition & s);
  virtual void Dump() const ;
  
  // Initialize the Structure Descriptor with the info for code position
  static void SddfElementInit(StructureDescriptor & structDesc) ;
  // Kick out the current info to the Dossier
  virtual void SddfDossierOut(RecordDossier & d) const;
  virtual void SddfRead(PipeReader & p);
private:
  Line_Number lineNumber;
  SDDF_ProcInfo * theProcedure;
};

typedef int Static_Id;

const Static_Id BAD_STATIC_ID_VALUE = -1;

class StaticDescriptorBase : public CodePosition {
public:
  StaticDescriptorBase();
  virtual ~StaticDescriptorBase();
  
  // Accessors for the identifier
  // If static id has not been set, return BAD_STATIC_ID_VALUE
  void 		SetId(Static_Id i);
  Static_Id 	GetId(void) const;
  
  // Input and output operations.
  friend  ostream & operator << (ostream & o, const
				 StaticDescriptorBase & s);
  virtual void Dump() const ;

  static  void SddfElementInit(StructureDescriptor &);
  void SddfDossierOut(RecordDossier &) const;

  static  void SddfInitDescriptor(PipeWriter &);
  virtual void SddfDump(PipeWriter &) const;

  virtual void SddfRead(PipeReader &);

  static void WalkRegistry(PipeWriter & p);
protected:
  virtual const char * GetIdName() const;
private:
  Static_Id myId;
  // Container for my static descriptor format. only one per class
  static RecordDossier  *dossier; 
  // Next id to hand out
  static int nextId;
  // The central registry for all objects
  static StaticDescriptorList * theRegistry;
};

class SDDF_Symbolic_Value : public StaticDescriptorBase {
public: 
  SDDF_Symbolic_Value();
  virtual ~ SDDF_Symbolic_Value();

  void 		SetSymbolicValue(SDDFint v);
  SDDFint 	GetSymbolicValue();

  static  Static_Id ReserveSymbolicId();

  friend  ostream & operator << (ostream & o, const SDDF_Symbolic_Value & s);
  virtual void Dump() const ;
  static  void SddfInitDescriptor(PipeWriter & output);
  virtual void SddfDump(PipeWriter & output) const;
  virtual void SddfRead(PipeReader & output);	   
private:
  Boolean wasSet;
  SDDFint symValue;
  static RecordDossier  *localDossier; 
};

#endif
