/* $Id: ScanFile.C,v 1.2 1997/03/27 20:31:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/frontEnd/fortTextTree/FortTextTree.h>

#include <libs/fileAttrMgmt/fortranModule/ScanFile.h>

int ScanFile(const char *srcFile, ScanFileFileFunc enterFile, 
	      ScanFileFileFunc exitFile, ScanFileLineFunc procFileLine, ...)
{
  va_list args;
  va_start(args, procFileLine);

  ftt_TraverseTextV((char *) srcFile, (PushFuncV) enterFile, 
		    (PopFuncV) exitFile, (EnterFuncV) procFileLine, args);
  va_end(args);
  return 0;
}
