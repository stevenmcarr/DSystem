/* $Id: AFormalModAttr.C,v 1.1 1997/03/11 14:27:52 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// AFormalModAttr.C
//
// Author: John Mellor-Crummey                                January 1994
//
// Copyright 1994, Rice University
//***************************************************************************

#include <assert.h>

#if 0
#include <stdio.h>
#include <stdlib.h>
#include <values.h>

#include <libs/support/strings/rn_string.h>
#include <include/ip_info/ProcAFormalInfo.h>

#endif


#include <libs/fileAttrMgmt/attributedFile/AttributedFile.h>

#include <libs/fileAttrMgmt/fortranModule/AFormalModAttr.h>
#include <libs/fileAttrMgmt/fortranModule/CollectLocalInfo.h>
#include <libs/fileAttrMgmt/fortranModule/FortTreeModAttr.h>
#include <libs/fileAttrMgmt/fortranModule/FortranModule.h>

#include <libs/frontEnd/ast/AstIterators.h>
#include <libs/frontEnd/fortTree/fortsym.h>

#include <libs/support/file/FormattedFile.h>



//*******************************************************************
// declarations 
//*******************************************************************


REGISTER_FORTRAN_MODULE_ATTRIBUTE(AFormalModAttr);


//*********************
// forward declarations 
//*********************

static ProcLocalInfo *ComputeProcAFormalInfo(FortTree ft, AST_INDEX node);


//*********************************************************************
// class AFormalModAttr interface operations
//*********************************************************************

AFormalModAttr::AFormalModAttr()
{
}

AFormalModAttr::~AFormalModAttr()
{
}


void AFormalModAttr::DetachUpCall()
{
  uplinkToFile->DetachAttributeIfUnreferenced(CLASS_NAME(FortTreeModAttr));
}


int AFormalModAttr::ComputeUpCall()
{
  FortTreeModAttr *ftAttr = (FortTreeModAttr *) 
    uplinkToFile->AttachAttribute(CLASS_NAME(FortTreeModAttr));

  int code = CollectLocalInfo(ftAttr->ft, this, ComputeProcAFormalInfo);

  uplinkToFile->DetachAttribute(ftAttr);
  return code;
}


ProcLocalInfo *AFormalModAttr::NewProcEntry()
{
  return new ProcAFormalInfo;
}




//***********************************************************************
// class ProcAFormalInfo interface operations
//***********************************************************************

CLASS_NAME_IMPL(ProcAFormalInfo);

ProcAFormalInfo::ProcAFormalInfo(const char *_name) : ProcLocalInfo(_name)
{
}


ProcAFormalInfo::~ProcAFormalInfo()
{
}



//========================================================================
// file I/O operations
//========================================================================


int ProcAFormalInfo::ReadUpCall(FormattedFile *file)
{
  return StringSet::Read(file);
}


int ProcAFormalInfo::WriteUpCall(FormattedFile *file)
{
  return StringSet::Write(file);
}



//*********************************************************************
// private operations
//*********************************************************************


static void AddArrayFormals(ProcAFormalInfo *afi, AST_INDEX formalList, 
			    SymDescriptor symtab)
{
  AstListElementsIterator formals(formalList);
  AST_INDEX formal;
  for (; formal = formals.Current(); formals++) {
    // if formal is an identifier (it could be a * -- an alternate return)
    if (is_identifier(formal)) { 
      char *name = gen_get_text(formal);
      fst_index_t nameIndex = fst_QueryIndex(symtab, name);
      if (fst_GetFieldByIndex(symtab, nameIndex, SYMTAB_NUM_DIMS) != 0) 
	afi->Add(name);
    }
  }
}


static ProcLocalInfo *ComputeProcAFormalInfo(FortTree ft, AST_INDEX node)
{
  
  char *procName = gen_get_text(get_name_in_entry(node));
  ProcAFormalInfo *afi = new ProcAFormalInfo(procName);
  SymDescriptor symtab = ft_SymGetTable(ft, procName);
  assert(symtab != 0); // must have a symbol table

  AddArrayFormals(afi, get_formals_in_entry(node), symtab);

  AST_INDEX stmtList = get_stmts_in_scope(node);
  AstListElementsIterator stmts(stmtList);
  AST_INDEX stmt;
  for (; stmt = stmts.Current(); stmts++) {
    if (is_entry(stmt)) { 
      AddArrayFormals(afi,  get_formals_in_entry(stmt), symtab);
    }
  }
  return afi;
}

