/* $Id: NeedProvSet.C,v 1.3 1997/03/27 20:34:58 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

//******************************************************************
// NeedProvSet.C: 
//
//  set of descriptors for entry points a module needs or provides
//
// Author: 
//   John Mellor-Crummey                              October 1993
//
// Copyright 1993, Rice University
//******************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <libs/support/strings/rn_string.h>
#include <libs/support/tables/HashTable.h>
#include <libs/support/file/FormattedFile.h>
#include <libs/support/strings/StringBuffer.h>
#include <libs/support/strings/StringIO.h>

#include <libs/frontEnd/include/NeedProvSet.h>
#include <libs/frontEnd/ast/gen.h>

//***************************
// local declarations
//***************************

#define UNKNOWN_NARGS -1


//**************************************************
// interface operations for class ProcInterfaceArg
//**************************************************


ProcInterfaceArg::ProcInterfaceArg() : NamedObject(0)
{
}


void ProcInterfaceArg::Init(ProcInterfaceArgUsage _usage, int _type, 
			    const char *_name)
{
  assert(_name != 0);

  *((char **) &name) = ssave(_name);

  usage = _usage;
  type = _type;
}


ProcInterfaceArg::~ProcInterfaceArg()
{
  if (name) sfree((char *)name);
}


//---------------------------------------------------
// I/O operations
//---------------------------------------------------


int ProcInterfaceArg::Read(FormattedFile *file)
{
#if 0
  int tmp;

  int code = file->Read(tmp);
  if (code) return code;

  usage = (ProcInterfaceArgUsage) tmp;
#endif
  
  return ReadString((char **) &name, file) || file->Read(*(int *) &usage) || 
    file->Read(type);
}


int ProcInterfaceArg::Write(FormattedFile *file)
{
  return  WriteString(name, file) || file->Write((int) usage) || 
    file->Write(type);
}



//---------------------------------------------------
// debugging support
//---------------------------------------------------

void ProcInterfaceArg::Dump()
{
  fprintf(stderr,"ProcInterfaceArg: type(%d) usage(%d)\n", name, type, usage);
}

//**************************************************
// interface operations for class ProcInterface
//**************************************************


ProcInterface::ProcInterface() : 
NamedObjectIO(0), procType(PI_ILLEGAL), returnType(0), maxArgs(UNKNOWN_NARGS),
args(0)
{
  nextArg = 0;
}


ProcInterface::ProcInterface
(const char *scope, const char *_name, const ProcInterfaceType _procType, 
 const int _retType):
NamedObjectIO(_name), procType(_procType), returnType(_retType),
enclosingScopeName(ssave(scope)), maxArgs(UNKNOWN_NARGS),
args(0)
{
  nextArg = 0;
}

ProcInterface::ProcInterface(ProcInterface &rhs) : 
NamedObjectIO(rhs.name), procType(rhs.procType), 
returnType(rhs.returnType), maxArgs(rhs.maxArgs),
enclosingScopeName(ssave(rhs.enclosingScopeName)), args(0) 
{
  nextArg = 0;
  AllocateAllArgs(rhs.maxArgs);
  
  for (int i = 0; i < maxArgs; i++) {
    InitNextArg(rhs.args[nextArg].usage, rhs.args[nextArg].type, 
		rhs.args[nextArg].name);
  }
}


ProcInterface::~ProcInterface()
{
  if (maxArgs > 0) delete [] args;
  if (enclosingScopeName) sfree((char *)enclosingScopeName);
}

void ProcInterface::AllocateAllArgs(const int nargs)
{
  *((int *) &maxArgs) = nargs;
  if (maxArgs > 0) args = new ProcInterfaceArg[nargs];
}


const ProcInterfaceArg *ProcInterface::Arg(uint argnum)
{
  assert(maxArgs > 0 && argnum < maxArgs);
  return &args[argnum];
}


void ProcInterface::InitNextArg(ProcInterfaceArgUsage usage,
				int type, const char *name) 
{
  assert(nextArg < maxArgs);
  args[nextArg++].Init(usage,type,name);
}

//---------------------------------------------------
// I/O operations
//---------------------------------------------------


int ProcInterface::Read(FormattedFile *file)
{ 
  return ProcInterface::NamedObjectReadUpCall(file);
}


int ProcInterface::Write(FormattedFile *file)
{ 
  return ProcInterface::NamedObjectWriteUpCall(file);
}


int ProcInterface::NamedObjectReadUpCall(FormattedFile *file)
{
  int code = ReadString((char **) &enclosingScopeName, file);
  if (code) return code;

  code = file->Read(*((int *)&maxArgs)) || 
    file->Read(*((int *)&returnType)) || file->Read(*((int *)&procType));
  if (code) return code;

  if (maxArgs > 0) args = new ProcInterfaceArg[maxArgs];
  while (nextArg < maxArgs) {
    code = args[nextArg++].Read(file);
    if (code) return code;
  }
  return 0;
}

int ProcInterface::NamedObjectWriteUpCall(FormattedFile *file)
{
  int code = WriteString(enclosingScopeName, file);
  if (code) return code;

  code = file->Write(*((int *)&maxArgs)) || 
    file->Write(*((int *)&returnType)) || file->Write(*((int *)&procType));
  if (code) return code;
  
  for(int i = 0; i < maxArgs; i++) {
    code = args[i].Write(file);
    if (code) return code;
  }
  return 0;
}


//---------------------------------------------------
// debugging support
//---------------------------------------------------


void ProcInterface::NamedObjectDumpUpCall()
{
  fprintf(stderr,"ProcInterface: scope(%s) procType(%d) returnType(%d)\n", 
	  enclosingScopeName, procType, returnType);
  for(int i = 0; i < maxArgs; i++) args[i].NamedObjectDump();
}


//---------------------------------------------------
// support for interface consistency checking
//---------------------------------------------------


CompTypeMatch ProcInterface::ReturnTypesConsistent(ProcInterface *rhs)
{
  register int t1 = this->returnType;
  register int t2 = rhs->returnType;
  return ((t1 == TYPE_UNKNOWN) || (t2 == TYPE_UNKNOWN) || (t1 == t2)) ?
    TYPE_MATCH_OK : TYPE_MATCH_ERROR;
}


CompTypeMatch ProcInterface::ArgCountConsistent(ProcInterface *rhs)
{
  if (this->maxArgs == UNKNOWN_NARGS || rhs->maxArgs == UNKNOWN_NARGS ||
      this->maxArgs == rhs->maxArgs) 
    return TYPE_MATCH_OK;
  else 
    return TYPE_MATCH_ERROR;
}

 
uint ProcInterface::NumberArgsToTestForConsistency(ProcInterface *rhs)
{
  if (this->maxArgs != rhs->maxArgs) return 0;
  else return (this->maxArgs == UNKNOWN_NARGS) ? 0 : this->maxArgs;
}


CompTypeMatch  ProcInterface::ArgPairConsistent(ProcInterface *rhs, uint arg)
{
  assert(this->maxArgs == rhs->maxArgs && arg < this->maxArgs);
  return composition_type_match_table[this->args[arg].type][ rhs->args[arg].type];
}


CompTypeMatch ProcInterface::Consistent(ProcInterface *rhs)
{
  int warnings = 0;
  switch(ReturnTypesConsistent(rhs)) {
  case TYPE_MATCH_ERROR: return TYPE_MATCH_ERROR;
  case TYPE_MATCH_WARNING: 
    warnings++;
  case TYPE_MATCH_OK: 
    break;
  }

  switch(ArgCountConsistent(rhs)) {
  case TYPE_MATCH_ERROR: return TYPE_MATCH_ERROR;
  case TYPE_MATCH_WARNING: 
    warnings++;
  case TYPE_MATCH_OK: 
    break;
  }

  uint nargs = NumberArgsToTestForConsistency(rhs);
  for (uint i = 0; i < nargs; i++) {
    switch(ArgPairConsistent(rhs, i)) {
    case TYPE_MATCH_ERROR: return TYPE_MATCH_ERROR;
    case TYPE_MATCH_WARNING: 
      warnings++;
    case TYPE_MATCH_OK: 
      break;
    }
  }
  return (warnings > 0) ? TYPE_MATCH_WARNING : TYPE_MATCH_OK;
}


CompTypeMatch ProcInterface::Consistency
(ProcInterface *rhs, char *lhsUsage, char *rhsUsage,
 char **error_bufptr, char **warning_bufptr)
{
  CompTypeMatch errorKind = TYPE_MATCH_OK;
  int warnings = 0, errors = 0;
  
#define ERROR_BUFFER_INITIAL_LENGTH 100
  StringBuffer error_buffer(ERROR_BUFFER_INITIAL_LENGTH);
  StringBuffer warning_buffer(ERROR_BUFFER_INITIAL_LENGTH);
  
  /* assume ok until proven otherwise */
  errorKind = TYPE_MATCH_OK;
  
  if (ReturnTypesConsistent(rhs) == TYPE_MATCH_ERROR) {

    error_buffer.Append("    Return value type mismatch.\n");
    error_buffer.Append("        In %s, return value type is %s\n",
			lhsUsage, gen_type_get_text(this->returnType));
    error_buffer.Append("        In %s, return value type is %s\n",
			rhsUsage, gen_type_get_text(rhs->returnType));

    if (error_bufptr) *error_bufptr = error_buffer.Finalize();
    return TYPE_MATCH_ERROR;
  }
  
  if (this->maxArgs == UNKNOWN_NARGS || rhs->maxArgs == UNKNOWN_NARGS) 
    return TYPE_MATCH_OK;
  
  if (this->maxArgs != rhs->maxArgs) {

    error_buffer.Append("    Number of arguments inconsistent.\n"); 
    error_buffer.Append("        In %s, number of arguments is %d\n",
			lhsUsage, this->maxArgs);
    error_buffer.Append("        In %s, number of arguments is %d\n",
			rhsUsage, rhs->maxArgs);

    if (error_bufptr) *error_bufptr = error_buffer.Finalize();
    return TYPE_MATCH_ERROR;
  }
  
  for (int i = 0; i < this->maxArgs; i++) {
    if (ArgPairConsistent(rhs,i) == TYPE_MATCH_WARNING) {
      warning_buffer.Append("    Warning: inconsistent argument for parameter %d\n", i + 1);
      warning_buffer.Append("        In %s, argument %d has type %s\n",
			    lhsUsage, i, gen_type_get_text(this->args[i].type));
      warning_buffer.Append("        In %s, argument %d has type %s\n",
			    rhsUsage, i, gen_type_get_text(rhs->args[i].type));
      warnings++;
    }
    
    if (ArgPairConsistent(rhs,i) == TYPE_MATCH_ERROR) {
      error_buffer.Append("    Type error: inconsistent argument for parameter %d\n", i + 1);
      error_buffer.Append("        In %s, argument %d has type %s\n",
			  lhsUsage, i, gen_type_get_text(this->args[i].type));
      error_buffer.Append("        In %s, argument %d has type %s\n",
			  rhsUsage, i, gen_type_get_text(rhs->args[i].type));
      errors++;
    }
  }
  
  if (errors > 0) {
    errorKind = TYPE_MATCH_ERROR;
    if (error_bufptr) *error_bufptr = error_buffer.Finalize();
  } 

  if (warnings > 0) { 
    if (warning_bufptr) *warning_bufptr = warning_buffer.Finalize();
    if (errors == 0) errorKind = TYPE_MATCH_WARNING;
  }
  
  return errorKind;
}


//*********************************************
// interface operations for class NeedProvSet
//*********************************************



NeedProvSet::NeedProvSet()
{
}

NeedProvSet:: NeedProvSet(NeedProvSet &rhs) 
{
  NeedProvSetIterator npi(&rhs);
  ProcInterface *entry;
  for (; entry = npi.Current(); ++npi) AddEntry(new ProcInterface(*entry));
}


NeedProvSet::~NeedProvSet(void)
{
  this->Destroy();
}


ProcInterface *NeedProvSet::GetEntry(const char *_name)
{
  return (ProcInterface *) NamedObjectTable::QueryEntry(_name);
}


NamedObjectIO *NeedProvSet::NewEntry()
{
  return new ProcInterface();
}


//---------------------------------------------------
// I/O operations
//---------------------------------------------------


int NeedProvSet::Read(FormattedFile *file)
{
  return NamedObjectTableRead(file);
}


int NeedProvSet::Write(FormattedFile *file)
{
  return NamedObjectTableWrite(file);
}



//**************************************************
// interface operations for class NeedProvSetIterator
//**************************************************



NeedProvSetIterator::NeedProvSetIterator(NeedProvSet *set) :
NamedObjectTableIterator(set)
{
}


NeedProvSetIterator::~NeedProvSetIterator()
{
}


ProcInterface *NeedProvSetIterator::Current() const
{
  return (ProcInterface *) NamedObjectTableIterator::Current();
}

