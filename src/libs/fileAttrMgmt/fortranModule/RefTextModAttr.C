/* $Id: RefTextModAttr.C,v 1.1 1997/03/11 14:28:02 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// RefTextModAttr.C
//
// Author: John Mellor-Crummey                                August 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>

#include <libs/support/file/UnixFile.h>
#include <libs/support/file/File.h>

#include <libs/fileAttrMgmt/fortranModule/FortranModule.h>
#include <libs/fileAttrMgmt/fortranModule/FortTextTreeModAttr.h>
#include <libs/fileAttrMgmt/fortranModule/FortTreeModAttr.h>
#include <libs/fileAttrMgmt/fortranModule/RefTextModAttr.h>



//*******************************************************************
// declarations 
//*******************************************************************

REGISTER_FORTRAN_MODULE_ATTRIBUTE(RefTextModAttr);


//**********************
// forward declarations
//**********************

static int exportToFile(int lineno, char *line, va_list args);



//*******************************************************************
// class RefTextModAttr interface operations
//*******************************************************************

RefTextModAttr::RefTextModAttr()
{
}


RefTextModAttr::~RefTextModAttr()
{
  Destroy();
}


int RefTextModAttr::Create()
{
  fttAttr = (FortTextTreeModAttr *)
    uplinkToFile->AttachAttribute(CLASS_NAME(FortTextTreeModAttr));
  return 0;
}

void RefTextModAttr::Destroy()
{
  uplinkToFile->DetachAttribute(fttAttr);
}


int RefTextModAttr::ComputeUpCall()
{
  return 0;
}


Boolean RefTextModAttr::Export(MessageFunction message_func, 
			       YesNoFunction yes_no_func)
{
  const char *const filename = uplinkToFile->ReferenceFilePathName();
  Boolean doSave = true;

  //-----------------------------------------------------------------
  // create a backup if the original file is not empty
  //-----------------------------------------------------------------
  if (!file_is_empty(filename)) {
    char *backupFilename = nssave(2, filename, "~");
    
    //-----------------------------------------------------------------
    // if backupFilename exists and is writable, then remove it and link
    // filename and backupFilename
    //-----------------------------------------------------------------
    if (((unlink(backupFilename) == -1) && errno != ENOENT) ||
	(link(filename, backupFilename) == -1)) {

      char *prompt = nssave(7, "Could not backup ", filename, "\n", " to ", 
			    backupFilename, ".\n", " Save anyway?\n");

      //-----------------------------------------------------------------
      // prompt here for save using callback 
      //-----------------------------------------------------------------
      (void) yes_no_func(prompt, &doSave, false);

      sfree(prompt);
    } else {

      //-----------------------------------------------------------------
      // remove the original file, we're about to recreate it
      //-----------------------------------------------------------------
      if (unlink(filename) == -1) {

	//-----------------------------------------------------------------
	// notify user of failure, then remove the backup file since 
	// save failed, and original file is still in place 
	//-----------------------------------------------------------------
	 message_func("Could not unlink %s prior to save. Save failed.", 
		      filename);

	(void) unlink(backupFilename); // ignore errors here
	doSave = false;
      }
    }
    sfree(backupFilename);
  }
  
  if (doSave == true) {
    File refText;
    if (refText.Open(filename, "w") != EOF) { 
      ftt_Export(fttAttr->ftt, '*', DEFAULT_SIG_COMMENT_HANDLERS,
	       exportToFile, &refText);
      refText.Close();	       
      uplinkToFile->ResetTimeStamp();
      return true;
    } else {
      //-----------------------------------------------------------------
      //  notify user of failure
      //-----------------------------------------------------------------
      message_func("Could not open %s. Save failed.", filename);
    }
  }

  return false;
}



//*******************************************************************
// private operations
//*******************************************************************


static int exportToFile(int, char *line, va_list args)
{
  File *file = va_arg(args, File*);
  return (file->Printf("%s\n", line) == EOF ? EOF : 0);
}



