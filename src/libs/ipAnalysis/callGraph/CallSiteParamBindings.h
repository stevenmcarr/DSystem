/* $Id: CallSiteParamBindings.h,v 1.3 1997/03/11 14:34:34 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef CallSiteParamBindings_h
#define CallSiteParamBindings_h

//***************************************************************************
//    CallSiteParamBindings.h
//
//    define a map of actual parameters <--> formal parameters at a callsite
//
//    Author:
//      John Mellor-Crummey                              December 1992
//         rewrite based on an earlier implementation of parameter 
//         mappings by Mary Hall
//
//    Copyright 1992, Rice University, as part of the ParaScope Programming 
//    Environment Project
//***************************************************************************

#ifndef iptypes_h
#include <libs/ipAnalysis/ipInfo/iptypes.h>
#endif

typedef unsigned ActualParamClass;

#define APC_DataLocal              VTYPE_LOCAL_DATA
#define APC_DataGlobal             VTYPE_COMMON_DATA
#define APC_DataFormal             VTYPE_FORMAL_PARAMETER

#define APC_ProcFormal             (VTYPE_PROCEDURE | VTYPE_FORMAL_PARAMETER)
#define APC_ProcConstant           VTYPE_PROCEDURE
#define APC_ProcIntrinsic          (VTYPE_PROCEDURE | VTYPE_INTRINSIC)

#define APC_Constant               VTYPE_CONSTANT
#define APC_Untyped                VTYPE_NO_ATTRIBUTES


typedef enum { FormalNameSet, ActualNameSet } ParamNameSet;

class FormattedFile;          // external declaration 
class CallSiteParamBindings;  // forward declaration

//--------------------------------------------------------------------------
// info describing an actual <--> formal binding 
//--------------------------------------------------------------------------
typedef struct ParamBindingS {
  const char *actual;   // the name of the actual bound to formal 
  int a_offset;   // the offset of the actual segment bound to formal
  int a_length;   // the length of the actual segment bound to formal
#if 0
  VarScope a_scope; // the scope of the actual
#endif
  ActualParamClass a_class; // role of the actual 

  const char *formal;   // the name of the formal parameter bound
} ParamBinding;


//--------------------------------------------------------------------------
// an opaque type that hides the representation of the set of parameter
// bindings. the only useful thing that can be done with a set of bindings
// is to iterate over the elements.
//--------------------------------------------------------------------------
typedef void *ParamBindingsSet;


//--------------------------------------------------------------------------
// an iterator to enumerate the names of formals or actuals involved in
// parameter bindings 
//--------------------------------------------------------------------------
class ParamNameIterator {
  struct ParamNameIteratorS *hidden;
public:
  ParamNameIterator(CallSiteParamBindings &b, ParamNameSet s);
  ~ParamNameIterator();
  const char *Current();
  void Reset();
  void operator ++();
};


//--------------------------------------------------------------------------
// an iterator that enumerates the set bindings for an actual
//--------------------------------------------------------------------------
class ParamBindingsSetIterator {
  struct ParamBindingsSetIteratorS *hidden;
public:
  ParamBindingsSetIterator(ParamBindingsSet *s);
  ~ParamBindingsSetIterator();
  ParamBinding *Current();
  void Reset();
  void operator ++();
};


//--------------------------------------------------------------------------
// a bidirectional map that relates actual parameters <--> formal parameters
// at for a callsite
//--------------------------------------------------------------------------
class CallSiteParamBindings {
  struct CallSiteParamBindingsS *hidden;
public:
  
  CallSiteParamBindings(); 
  ~CallSiteParamBindings(); 
  
  void Bind(const char *actual, int a_offset, int a_length, 
	    ActualParamClass a_class, // VarScope a_scope,  
	    const char *formal);
  
  // query bindings for an actual 
  ParamBindingsSet *GetForwardBindings(const char *actual);
  
  // query bindings for a formal 
  ParamBinding *GetReverseBinding(const char *formal);
  
  int Read(FormattedFile *file);
  int Write(FormattedFile *file);
  
  void Dump();
  
friend class ParamBindingsIterator;
friend class ParamNameIterator;
};


#endif /* CallSiteParamBindings_h */

