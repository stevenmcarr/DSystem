/* $Id: MkAssert.h,v 1.1 1997/03/11 14:28:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// -*- C++ -*-
/* Explanatory one line comment on file nature */
/* $Header: /home/cs/carr/cvsroot/DSystem/src/libs/fortD/performance/staticInfo/MkAssert.h,v 1.1 1997/03/11 14:28:56 carr Exp $
*/
// This code is from Loni Granston. I've ported it to c++
// This implements an Assert macro that tests a condition, and if the
// condition is false, prints the file, line number and condition
// tested to std err and possible a logfile.
// 
#ifndef _MkAssert_h
#define _MkAssert_h

#include <iostream.h>
#include <libs/support/misc/general.h>

#ifndef ASSERT_FILE_VERSION 
#define ASSERT_FILE_VERSION "No file version info provided"
#endif


enum ExitType {EXIT,ABORT,CONTINUE,NOTIFY};
void OpenErrorMsg(const char * filename);
void ReadErrorMsg(const char * filename, int lineno);
void Quit(ExitType action, int status);
void InitializeErrorChannel(const char * logfile);
void NoteFailedAssertion(const char * ex, const char * fileVersion, const char *fileName, 
			   int lineNo);

void NoteFailureAndDie(const char * ex, const char * errorDesc, const char * fileVersion, 
		       const char * fileName, int lineNo, ExitType quitAction, 
		       int status);

void NoteFailureAndDieS(const char * ex, const char * errorDesc, const char * section,  
			const char * fileVersion, 
			const char * fileName, int lineNo, ExitType quitAction, 
			int status);

# ifdef MKASSERT

#define _MkAssert(ex, errorDesc, quitAction)	{if (!(ex)){NoteFailureAndDie(#ex, errorDesc,ASSERT_FILE_VERSION, __FILE__, __LINE__, quitAction, 101); }}
#define MkAssert(ex, msg, action)	_MkAssert(ex, msg, action)

#define _MkAssertS(ex, errorDesc, section, quitAction) {if (!(ex)){NoteFailureAndDieS(#ex, errorDesc, section, ASSERT_FILE_VERSION, __FILE__, __LINE__, quitAction, 101);}}
#define MkAssertS(ex, msg, section, action) _MkAssertS(ex,msg,section,action)

# else

#define _MkAssert(ex, s, action)
#define MkAssert(ex, s, action)
#define _MkAssertI(ex, msg, section, action)
#define MkAssertI(ex, msg, section, action)
# endif

#define ProgrammingErrorIfNot(cond) MkAssert((cond), "Programming error!!", ABORT)
#define ShouldNotGetHere	MkAssert(false, "Should not get here; Programming error!!", ABORT)

#endif
