/* $Id: SDDF_IO.h,v 1.1 1997/03/11 14:28:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// -*- C++ -*-
/* Explanatory one line comment on file nature */
/* $Header: /home/cs/carr/cvsroot/DSystem/src/libs/fortD/performance/staticInfo/SDDF_IO.h,v 1.1 1997/03/11 14:28:57 carr Exp $
//
*/
// The tag should exactly match the filename
#ifndef _stdform_h
#define _stdform_h
// All includes and definitions go here. Include only the minimum set
// required to include this file. Do not include anything that is only
// required by the coressponding .c file (if there is one)   

class PipeWriter;
class OutputFileStreamPipe;

PipeWriter * SD_InitIo(OutputFileStreamPipe * op);

// Don't forget terminal semicolon on classes!
#endif




