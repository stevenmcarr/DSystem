/* $Id: SDDF_IO.C,v 1.1 1997/03/11 14:28:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// -*- C++ -*- 
/* One line expanatory comment */ 
/* $Header: /home/cs/carr/cvsroot/DSystem/src/libs/fortD/performance/staticInfo/SDDF_IO.C,v 1.1 1997/03/11 14:28:57 carr Exp $
//
*/

static const char * RCS_ID = "$Id: SDDF_IO.C,v 1.1 1997/03/11 14:28:57 carr Exp $";
#define ASSERT_FILE_VERSION RCS_ID
#define MKASSERT

// Do includes for this code
// Include corresponding .h file last. In general, if these routines
// are called externally, there should be a .h file.

#include <libs/fortD/performance/staticInfo/SD_DataInfo.h>
#include <libs/fortD/performance/staticInfo/SD_MsgInfo.h>
#include <libs/fortD/performance/staticInfo/SD_SrcInfo.h>

#include <AsciiPipeWriter.h>
#include <Attributes.h>
#include <OutputFileStreamPipe.h>
#include <PipeWriter.h>
#include <InitializeStatic.C>
 
#include <libs/fortD/performance/staticInfo/MkAssert.h>

// Set this small to ease debugging.
const int SDDF_OUTPIPE_BUFFER_SIZE = 16;


PipeWriter * SD_InitIo(OutputFileStreamPipe * op) {

  int test = op->successfulOpen();
  MkAssert(test,"File open failed",EXIT);
  AsciiPipeWriter * p = new AsciiPipeWriter(op);
  
  // Random junk to make file palatable to converter:
  
  Attributes attributes;
  
  attributes.insert("Creation Date","April 1, 2001");
  attributes.insert("Machine","Timex/Sinclair");
  
  p->putAttributes(attributes);
  
  // Set things up and give the new classes a spin:
  
  SDDF_DecompInfo::SddfInitDescriptor(*p);
  SDDF_DistInfo::SddfInitDescriptor(*p);
  SDDF_AlignInfo::SddfInitDescriptor(*p);
  SDDF_ArrayInfo::SddfInitDescriptor(*p);
  SDDF_ProcInfo::SddfInitDescriptor(*p);  
  SDDF_DependInfo::SddfInitDescriptor(*p);
  SDDF_MessageSend::SddfInitDescriptor(*p);
  SDDF_MessageRecv::SddfInitDescriptor(*p);
  SDDF_MessageSendWait::SddfInitDescriptor(*p);
  SDDF_MessageRecvWait::SddfInitDescriptor(*p);
  SDDF_Symbolic_Value::SddfInitDescriptor(*p);

  return p;
  
}
