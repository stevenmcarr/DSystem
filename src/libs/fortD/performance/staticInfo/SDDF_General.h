/* $Id: SDDF_General.h,v 1.1 1997/03/11 14:28:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// -*- C++ -*-
/* All generally used declarations that only need to be seen by the
// sddf library 
// $Header: /home/cs/carr/cvsroot/DSystem/src/libs/fortD/performance/staticInfo/SDDF_General.h,v 1.1 1997/03/11 14:28:56 carr Exp $
//
*/
#ifndef _SDDF_General_h
#define _SDDF_General_h
// All includes and definitions go here. Include only the minimum set required to include this file. Do not include anything that is only required by the coressponding .c file (if there is one)

#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/fortD/misc/FortD.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dg.h>
#include <libs/fortD/performance/staticInfo/SDDF_Instrumentation.h>
#include <libs/fortD/performance/staticInfo/SD_Base.h>
#include <libs/fortD/performance/staticInfo/SD_Decls.h>
#include <libs/fortD/performance/staticInfo/VPDlist.h>
#include <libs/support/vectors/PointerVector.h>

class SDDF_SideArray;
class ArrayNameInfo;
class SPMDInstrumentation;


struct PabloLocalInfo {
 PabloLocalInfo();
 ~PabloLocalInfo();
 void SetProcInfo( SDDF_ProcInfo * proc);
 void SetProcName(const char * name);
 void SetFileName(const char * name);
 void PabloLocalInfo::InitSideArray(AST_INDEX root, FortTree ft, 
				    FortTextTree ftt);
 SDDF_ProcInfo * GetProcInfo() const;

 SDDF_SideArray * sideArray;
 SDDF_ProcInfo * procInfo;
 char * procName;
 char * moduleFileName;

 // These three are redundant, I think.
 FortTree curFt;
 FortTextTree curFtt;
 AST_INDEX theRoot;
};

// This contains all the SDDF/Pablo information that needs to persist
// over the lifetime of the program. 

struct PabloGlobalInfo {
  // Calling SD_InitialSetup() sets this flag and enables Pablo info:
  Boolean wantPabloInfo;
  // The current mess of local info.
  PabloLocalInfo * currentLocals;
  ArrayNameInfo* arrayInfo;
  SPMDInstrumentation *instr;
  PointerVector arrayNameList;		// For list of array names seen. 
                                        // Used to sort out aliasings later
};

extern PabloGlobalInfo thePabloGlobalInfo;

// This is the annotation that is added to the side array


class PabloSideArrayInfo {
public:
  PabloSideArrayInfo();
  PabloSideArrayInfo(int SLine, int ELine);
  ~PabloSideArrayInfo();
  void SetSDDFInfo(StaticDescriptorBase * rec, SDDF_RECORD_TYPE t,
		   FortranDHashTableEntry * ht);
  
  StaticDescriptorBase * Get(int i = 0);
  StaticDescriptorBase * GetSafe(SDDF_RECORD_TYPE t,int i =0);
  Boolean AddSafe(SDDF_RECORD_TYPE t, StaticDescriptorBase *p);
  void Add(StaticDescriptorBase * p);
  
  int Size();
  // Obsolete interface:
  //  StaticDescriptorBase * theRec;
  // This allows me to do a lot of stuff much more easily later, but
  // may not be as persistient as I like.
  FortranDHashTableEntry * htEnt;
  
  void SetStartLine(int s);
  void SetEndLine(int s);
  void SetStartChar(int s);
  void SetEndChar(int s);
  void SetTextPtr(char * c);
  void putDGEdge(DG_Edge* edge);
  
  int GetStartLine();
  int GetEndLine();
  int GetStartChar();
  int GetEndChar();
  char * GetTextPtr();
  DG_Edge* getDGEdge();
  
private:
  PointerVector a;
  SDDF_RECORD_TYPE theType;
  int startLine, endLine, startChar, endChar;
  char * theText;
  DG_Edge* DGedge;
};

// Don't forget terminal semicolon on classes!
#endif
