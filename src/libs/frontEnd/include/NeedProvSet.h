/* $Id: NeedProvSet.h,v 1.1 1997/03/11 14:29:58 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//******************************************************************
// NeedProvSet.h: 
//
//  set of descriptors for entry points a module needs or provides
//
// Author: 
//   John Mellor-Crummey                              October 1993
//
// Copyright 1993, Rice University
//******************************************************************

#ifndef NeedProvSet_h
#define NeedProvSet_h


#include <sys/types.h>

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef forttypes_h
#include <libs/frontEnd/ast/forttypes.h>
#endif

#ifndef NamedObject_h
#include <libs/support/tables/namedObject/NamedObject.h>
#endif


#ifndef NamedObjectTable_h
#include <libs/support/tables/namedObject/NamedObjectTable.h>
#endif


class FormattedFile;  // minimal external definition


#define UNKNOWN_MODULE_NAME "_unknown_module_name_"


typedef enum { 
  PI_PROGRAM, PI_SUBROUTINE, PI_FUNCTION, PI_BLOCK_DATA, PI_UNKNOWN_ROUTINE,
  PI_ILLEGAL
} ProcInterfaceType;
       

typedef enum { 
  PI_LABEL, PI_VARIABLE, PI_PROCEDURE, PI_EXPRESSION 
} ProcInterfaceArgUsage;


class ProcInterfaceArg : public NamedObject {
public:
  NamedObject::name;
  ProcInterfaceArgUsage usage;
  int type;
  
  ProcInterfaceArg();
  ~ProcInterfaceArg();

  int Write(FormattedFile *file);
  int Read(FormattedFile *file);
  
  void Init(ProcInterfaceArgUsage usage, int type, const char *name);

  void Dump();
};


class ProcInterface : public NamedObjectIO {
private:
  int nextArg;
  ProcInterfaceArg *args;

  void NamedObjectDumpUpCall();
  int NamedObjectReadUpCall(FormattedFile *file);
  int NamedObjectWriteUpCall(FormattedFile *file);

public:
  int maxArgs;
  
  //-----------------------------------------------------------------------------
  // for call sites: the name of the callee
  // for entries: the name of the entry
  // for procedures: the name of the procedure
  //-----------------------------------------------------------------------------
  NamedObjectIO::name;
  
  const ProcInterfaceType procType; // type of named thing
  const int returnType;             // return type if any
  
  //-----------------------------------------------------------------------------
  // for entry points or call sites, this is the name of the enclosing procedure
  // for procedures, enclosingScopeName is the name of the procedure itself
  //-----------------------------------------------------------------------------
  const char* enclosingScopeName; 
  
  
  ProcInterface();
  ProcInterface(const char *enclosingScopeName, const char *name,
		const ProcInterfaceType procType = PI_ILLEGAL, 
		const int retType = 0);
  ProcInterface(ProcInterface &rhs);
  ~ProcInterface();
  
  CompTypeMatch ReturnTypesConsistent(ProcInterface *rhs);
  CompTypeMatch ArgCountConsistent(ProcInterface *rhs);

  uint NumberArgsToTestForConsistency(ProcInterface *rhs);
  CompTypeMatch  ArgPairConsistent(ProcInterface *rhs, uint argIndex);

  CompTypeMatch Consistent(ProcInterface *rhs);
  CompTypeMatch Consistency(ProcInterface *rhs,
			    char *lhsUsage, char *rhsUsage,
			    char **error_bufptr = 0, 
			    char **warning_bufptr = 0);
  
  // nargs == -1  signifies unknown number of arguments
  void AllocateAllArgs(const int nargs);
  void InitNextArg(ProcInterfaceArgUsage usage, int type, 
		   const char *argName);
  
  const ProcInterfaceArg *Arg(uint argnum);

  // I/O operations
  int Read(FormattedFile *file);
  int Write(FormattedFile *file);
  
};



class NeedProvSet : public NamedObjectTableIO {
private:
  NamedObjectIO *NewEntry();
  
public:
  NeedProvSet();
  NeedProvSet(NeedProvSet &rhs);
  ~NeedProvSet(void);
  
  ProcInterface *GetEntry(const char *name);
  NamedObjectTableIO::AddEntry;

  // I/O support
  int Write(FormattedFile *file);
  int Read(FormattedFile *file);

  void Dump(void);
  
friend class NeedProvSetIterator;
};



class NeedProvSetIterator : private NamedObjectTableIterator {
public:
    NeedProvSetIterator(NeedProvSet *set);
   ~NeedProvSetIterator();

    NamedObjectTableIterator::operator++;
    NamedObjectTableIterator::Reset;
    ProcInterface* Current() const;
};


#endif
