/* $Id: ScanFile.C,v 1.1 1997/03/11 14:28:03 carr Exp $ */
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

  ftt_TraverseText((char *) srcFile, (PushFunc) enterFile, 
		   (PopFunc) exitFile, (EnterFunc) procFileLine, (Generic) args);
  va_end(args);
  return 0;
}
