/* $Id: MkAssert.C,v 1.1 1997/03/11 14:28:55 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// -*- C++ -*- 
/* Error system */ 
/* $Header: /home/cs/carr/cvsroot/DSystem/src/libs/fortD/performance/staticInfo/MkAssert.C,v 1.1 1997/03/11 14:28:55 carr Exp $
*/

static const char * RCS_ID = "$Id: MkAssert.C,v 1.1 1997/03/11 14:28:55 carr Exp $";

#include <stdlib.h>
#include <strstream.h>
#include <iostream.h>
#include <fstream.h>
#include <string.h>
#include <libs/support/misc/general.h>
#include <libs/fortD/performance/staticInfo/MkAssert.h>

static ofstream logfile;
static Boolean externlog = false;

const int TMP_BUFFER_SIZE = 1000;

void OpenErrorMsg(const char * filename) {
  cerr << "Error opening file " << filename << endl;
  cerr << flush;
  exit(101);
}  // OpenErrorMsg

void ReadErrorMsg(const char * filename, int lineno) {
  cerr << "Error reading file " << filename << ": line " << lineno << endl;
  cerr << flush;
  exit(101);
}  // ReadErrorMsg

void InitializeErrorChannel(const char * logfilename) {
  if ((logfilename != 0) && (strlen(logfilename) >0)) {
    logfile.open(logfilename,ios::out,filebuf::openprot);
    if (logfile.good()) {
      externlog = true;      
      return;
    }	
  }
  externlog = false;
} // Initialize_Error_Channel


void NoteFailureAndDie(const char * ex, const char * errorDesc, const char * fileVersion, 
		       const char * fileName, int lineNo, ExitType quitAction, 
		       int status) {
  NoteFailedAssertion(ex,fileVersion,fileName,lineNo);

  if (errorDesc != 0 && *errorDesc != 0) {
    cerr << errorDesc << flush;
    logfile << errorDesc << flush;
  }	

  Quit(quitAction,status);

} // NoteFailureAndDie

void NoteFailureAndDieS(const char * ex, const char * errorDesc, const char * section, 
			const char * fileVersion, 
			const char * fileName, int lineNo, ExitType quitAction, 
			int status) {
  NoteFailedAssertion(ex,fileVersion,fileName,lineNo);

  if (errorDesc != 0 && *errorDesc != 0) {
    cerr << errorDesc << flush;
    logfile << errorDesc << flush;
  }	

  if (section != 0 && *section != 0) {
    cerr << section << flush;
    logfile << section << flush;
  }	

  Quit(quitAction,status);

} // NoteFailureAndDie

void NoteFailedAssertion(const char * ex, const char * fileVersion, const char *fileName, 
			   int lineNo) {
  char Buf[TMP_BUFFER_SIZE];
  ostrstream o(Buf,TMP_BUFFER_SIZE);

  o << "INTERNAL ERROR\n Assertion failed: file \"" << fileName << "\", line ";
  o << lineNo << endl;
  o << "Assertion: " << ex << endl;
  o << "File Version: " << fileVersion << endl;

  cerr << Buf;
  cerr.flush();
  if (externlog) {
    logfile << o;
    logfile.flush();
  }
}  // NoteFailedAssertion


void Quit(ExitType action, int status) {
  switch (action) {
  case ABORT:	
    abort();
    break;  
  case EXIT:	
    exit(status);
    break;
  case CONTINUE:      
    break;
  case NOTIFY:
  default:   
    cerr << "Programming error: Quit: unimplemented action\n" << flush;
    if (externlog) {
      logfile <<  "Programming error: Quit: unknown action\n" << flush;
    }
    break;
  }
}  // end Quit
