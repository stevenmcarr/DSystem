/* $Id: ScanFile.C,v 1.7 2001/10/12 19:36:21 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/frontEnd/fortTextTree/FortTextTree.h>

#include <libs/fileAttrMgmt/fortranModule/ScanFile.h>
#include <stdarg.h>

int ScanFile(const char *srcFile, ScanFileFileFunc enterFile, 
	      ScanFileFileFunc exitFile, ScanFileLineFunc procFileLine, ...)
{
  va_list args;
  va_start(args, procFileLine);

#if defined(OSF1) || defined(LINUX_ALPHA)
  ftt_TraverseTextV((char *) srcFile, (PushFuncV) enterFile, 
		    (PopFuncV) exitFile, (EnterFuncV) procFileLine, args);
#else
  ftt_TraverseText((char *) srcFile, (PushFunc) enterFile, 
		    (PopFunc) exitFile, (EnterFunc) procFileLine, (Generic)args);
#endif
  va_end(args);
  return 0;
}
