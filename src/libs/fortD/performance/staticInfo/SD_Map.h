/* $Id: SD_Map.h,v 1.2 1997/03/11 14:29:03 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// -*- C++ -*-
#ifndef SD_Map_h
#define SD_Map_h

/* $Header $ */

#ifndef rn_varargs_h
#include <include/rn_varargs.h>
#endif


#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef FortTree_h
#include <libs/frontEnd/fortTree/FortTree.h>
#endif
#ifndef FortTextTree_h
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#endif
#ifndef ft2text_h
#include <libs/frontEnd/prettyPrinter/ft2text.h>
#endif

#include <libs/support/vectors/PointerVector.h>
#include <libs/fortD/performance/staticInfo/SDDF_General.h>


class SDDF_SideArray {
public:
  // Maptext toggles whether or not line text info is added.
  SDDF_SideArray(char* moduleFileName,
		 AST_INDEX root,
		 FortTree ft,
		 FortTextTree ftt, 
		 Boolean MapText);
  ~SDDF_SideArray();

  // Dump the side array to stdout, with line nos and text.
  void Dump();

  // get the info record
  PabloSideArrayInfo * getInfo(AST_INDEX node);
  void setInfo(AST_INDEX node, PabloSideArrayInfo * p);
   
  // Short cut for common action of getting a copy of theRec from the
  // side array  
  StaticDescriptorBase* getSDDFDescriptor(AST_INDEX node);

  // the line number of this statement.
  int getLine(AST_INDEX node);

private:
  //--- Data ----
  void SD_MapNode(FortTreeNode node);
  AST_INDEX theRoot;
  FortTree myFt;
  FortTextTree myFtt;
  FortTreeSideArray md;
  Boolean mapText;
  PointerVector saRecPtrVector;		// List of allocated side-array records
  
  char* normFileName ;
  FILE* normOutFile;

  //--- Members ----
  
  // Functions to remember and free allocated side-array records
  void	KeepSideArrayRecPointer	(PabloSideArrayInfo* sideArrayRec);
  void	FreeSideArrayRecords	();
  
  // Functions to write out the module file in normalized form.
  int  WriteNormalizedFile	(char* moduleFileName);
  void OpenNormalizedOutFile	(char* moduleFileName);
  inline FILE* NormalizedFile()	{ return normOutFile; }
  void CloseNormalizedOutFile	();
};


#endif /* map_h */
