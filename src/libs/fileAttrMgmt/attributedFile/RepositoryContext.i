/* $Id: RepositoryContext.i,v 1.1 1997/03/11 14:27:46 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef RepositoryContext_h
#define RepositoryContext_h

//*************************************************************************
// RepositoryContext
//
//   an abstraction that supports access to timestamped attributes of
//   files in the context of other files. (for example, information can 
//   be stored about a source file in the context of a composition.)
//
// Author: John Mellor-Crummey                                 June 1993
//
// Copyright 1993, Rice University
//
//*************************************************************************

#ifndef context_h
#include <libs/support/database/context.h>
#endif

#define REP_CACHE_DIR_NAME     "DSystemCache"

class File;           // external declaration
class FileTimeStamp;  // external declaration


class RepositoryContext {
  struct RepositoryContextS *hidden;
  int InitAttrCache();
  void InitTimeStamp();
  int OpenInternal(char *location, RepositoryContext *parent);
public:
  FileTimeStamp *timeStamp;
  
  RepositoryContext();
  ~RepositoryContext();
  
  //================================================================
  // Open: a RepositoryContext must be opened before accessing attributes
  //================================================================
  
  //----------------------------------------------------------------
  // if a value for parent is specified, this is treated as a 
  // nested context
  //---------------------------------------------------------------
  int Open(char *location, RepositoryContext *parent = 0);
  
  //---------------------------------------------------------------
  // this interface is for backward compatability only. -- JMC 5/93
  //---------------------------------------------------------------
  int Open(Context oldstyle);
  
  //================================================================
  // Close: an open RepositoryContext will automatically be closed 
  // by the destructor if it has not been explicitly closed earlier
  //================================================================
  int Close();
  
  File *CreateFile(char *fileName);
  int DestroyFile(char *fileName);
  File *GetFile(char *fileName);
  char *GetCacheLocation();
};

#endif /* RepositoryContext_h */
