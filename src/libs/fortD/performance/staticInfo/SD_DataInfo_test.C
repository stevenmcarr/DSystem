/* $Id: SD_DataInfo_test.C,v 1.1 1997/03/11 14:29:00 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// -*- C++ -*- 
/* One line expanatory comment */ 
/* $Header: /home/cs/carr/cvsroot/DSystem/src/libs/fortD/performance/staticInfo/SD_DataInfo_test.C,v 1.1 1997/03/11 14:29:00 carr Exp $
// $Id:
*/

static const char * RCS_ID = "$Id: SD_DataInfo_test.C,v 1.1 1997/03/11 14:29:00 carr Exp $";
#define ASSERT_FILE_VERSION RCS_ID

// Do includes for this code
// Include corresponding .h file last. In general, if these routines
// are called externally, there should be a .h file.
#include <libs/fortD/performance/staticInfo/SD_DataInfo.h>

#include <AsciiPipeWriter.h>
#include <Attributes.h>
#include <OutputFileStreamPipe.h>
#include <PipeWriter.h>
#include <InitializeStatic.C>

#include <libs/fortD/performance/staticInfo/MkAssert.h>

main () {
  OutputFileStreamPipe * outFile = new OutputFileStreamPipe("Outputfile",16);
  int test = outFile->successfulOpen();
  Assert(test,"File open failed",EXIT);
  AsciiPipeWriter p(outFile);
  
  // Random junk to make file palatable to converter:

  Attributes attributes;
  
  attributes.insert("Creation Date","April 1, 2001");
  attributes.insert("Machine","Timex/Sinclair");

  p.putAttributes(attributes);

  // Set things up and give the new classes a spin:

  int No = 1;


  StaticDescriptorBase::SddfInitDescriptor(p);
  SDDF_DecompInfo::SddfInitDescriptor(p);
  SDDF_DistInfo::SddfInitDescriptor(p);
  SDDF_AlignInfo::SddfInitDescriptor(p);
  SDDF_ArrayInfo::SddfInitDescriptor(p);

  StaticDescriptorBase * SDB = new StaticDescriptorBase;

  SDB->SetPosition(100,"Dork1");
  SDB->SetId(No++);
  SDB->Dump();
  SDB->SddfDump(p);

  SDDF_DecompInfo * SDecomp = new SDDF_DecompInfo;
  
  SDecomp->SetPosition(101,"Dork2");
  SDecomp->SetId(No++); 
  
  SDecomp->SetName("Decomp D");
  SDecomp->SetDimension(12);
  SDecomp->SetDistributedFlag(FALSE);

  SDDF_AlignInfo * SAlign = new SDDF_AlignInfo;
  SDDF_DistInfo * SDist = new SDDF_DistInfo;

  SDecomp->AddAlign(SAlign);
  SDecomp->AddDist(SDist);

  SDecomp->Dump();

  SAlign->SetId(No++);
  SDist->SetId(No++);

  SDecomp->Dump();
  SDecomp->SddfDump(p);

  // Close off output
  attributes.clearEntries();
  attributes.insert("information","end of data");
  p.putAttributes(attributes);

  delete SDB;
  delete SDecomp;
  delete SDist;
  delete SAlign;

  delete outFile;
}
