/* $Id: SD_Globals.C,v 1.3 1999/06/11 20:47:17 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// -*- C++ -*- 
/* One line expanatory comment */ 
/* $Header: /home/cs/carr/cvsroot/DSystem/src/libs/fortD/performance/staticInfo/SD_Globals.C,v 1.3 1999/06/11 20:47:17 carr Exp $
*/

static const char * RCS_ID = "$Id: SD_Globals.C,v 1.3 1999/06/11 20:47:17 carr Exp $";
#define ASSERT_FILE_VERSION RCS_ID
#define MKASSERT

// Do includes for this code
// Include corresponding .h file last. In general, if these routines
// are called externally, there should be a .h file.

#include <iostream.h>
#include <malloc.h>
#include <string.h>
#include <Attributes.h>
#include <RecordDossier.h>
#include <StructureDescriptor.h>
#include <libs/fortD/performance/staticInfo/StaticSDDF.h>

#include <libs/fortD/performance/staticInfo/MkAssert.h>
#include <libs/fortD/performance/staticInfo/SD_Globals.i>


////////////////////////////////////////////////////////////////////////////
// Misc. Utility Routines.
////////////////////////////////////////////////////////////////////////////

void SetStringInDossier(RecordDossier & d, const char * entryName, 
			const char * theString) {
  int dimSizes[1];
  Array * ProcName = d.getArrayP(entryName);
  dimSizes[0] = strlen(theString) +1;
  ProcName->setDimSizes(dimSizes);
  CString theCString(theString);
  ProcName->setCellString(theCString,0);
}

void AddEntryToStructureDesc(StructureDescriptor & s, 
			     const char * com1, const char * com2, 
			     const char * entryName, 
			     const MachineDataType theType, const int theDim) {
  
  Attributes theAttributes;
  theAttributes.clearEntries();
  theAttributes.insert(com1,com2);
  
  FieldDescriptor * fieldDesc = 
    new FieldDescriptor(entryName, theAttributes, theType, theDim);
  s.insert(*fieldDesc);
  delete fieldDesc;

}

void AddEntryToStructureDesc(StructureDescriptor & s, 
			     const char * com1,
			     const char * entryName, 
			     const MachineDataType theType, const int theDim) {
  AddEntryToStructureDesc(s,com1,entryName,entryName,theType,theDim);
}


