/* $Id: SD_Base_test.C,v 1.1 1997/03/11 14:28:59 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// -*- C++ -*- 
/* One line expanatory comment */ 
/* $Header: /home/cs/carr/cvsroot/DSystem/src/libs/fortD/performance/staticInfo/SD_Base_test.C,v 1.1 1997/03/11 14:28:59 carr Exp $
// $Id:
*/

static const char * RCS_ID = "$Id: SD_Base_test.C,v 1.1 1997/03/11 14:28:59 carr Exp $";
#define ASSERT_FILE_VERSION RCS_ID

// Do includes for this code
// Include corresponding .h file last. In general, if these routines
// are called externally, there should be a .h file.
#include <libs/fortD/performance/staticInfo/SD_Base.h>

#include <AsciiPipeWriter.h>
#include <Attributes.h>
#include <OutputFileStreamPipe.h>
#include <PipeWriter.h>
#include <InitializeStatic.C>

main () {
  OutputFileStreamPipe * outFile = new OutputFileStreamPipe("Outputfile");
  int test = outFile->successfulOpen();
  Assert(test,"File open failed",EXIT);
  AsciiPipeWriter p(outFile);
  
  // Random junk to make file palatable to converter:

  Attributes attributes;
  
  attributes.insert("Creation Date","April 1, 2001");
  attributes.insert("Machine","Timex/Sinclair");

  p.putAttributes(attributes);

  // Set things up and give the new classes a spin:


  StaticDescriptorBase::SddfInitDescriptor(p);

  StaticDescriptorBase * SDB = new StaticDescriptorBase;

  SDB->SetPosition(100,"Dork1");
  SDB->SetId(1);

  SDB->Dump();

  SDB->SddfDump(p);

  // Close off output
  attributes.clearEntries();
  attributes.insert("information","end of data");
  p.putAttributes(attributes);

  delete outFile;


}
