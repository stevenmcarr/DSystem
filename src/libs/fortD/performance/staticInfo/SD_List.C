/* $Id: SD_List.C,v 1.1 1997/03/11 14:29:02 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// -*- C++ -*- 
/* One line expanatory comment */ 
/* $Header: /home/cs/carr/cvsroot/DSystem/src/libs/fortD/performance/staticInfo/SD_List.C,v 1.1 1997/03/11 14:29:02 carr Exp $
//
*/

static const char * RCS_ID = "$Id: SD_List.C,v 1.1 1997/03/11 14:29:02 carr Exp $";
#define MKASSERT
#define ASSERT_FILE_VERSION RCS_ID

// Do includes for this code
// Include corresponding .h file last. In general, if these routines
// are called externally, there should be a .h file.
#include <libs/fortD/performance/staticInfo/SD_List.h>
#include <libs/fortD/performance/staticInfo/MkAssert.h>
#include <libs/fortD/performance/staticInfo/SD_Globals.i>
#include <Attributes.h>
#include <PipeReader.h>
#include <PipeWriter.h>
#include <RecordDossier.h>
#include <libs/fortD/performance/staticInfo/StaticSDDF.h>
#include <StructureDescriptor.h>

Static_Id ResolveStaticDescriptorId(const StaticDescriptorBase *s) {
  Static_Id id;
  if (s ==0) {
    id = SDDF_UNDEF_VALUE;
  } else {
    id = s->GetId();
    // Right now handle unresolved ids gracelessly
    MkAssert(id != BAD_STATIC_ID_VALUE, "Unresolvable static id",EXIT);
  }
  return id;
}

StaticDescriptorList::StaticDescriptorList() {};


StaticDescriptorList::~StaticDescriptorList() {};

void StaticDescriptorList::AddElement(StaticDescriptorBase * s) {
  list.Push((void *)(s));
}

void StaticDescriptorList::ResolveToArray(Array * r) const {
  // Array must be of type INTEGER:
  MkAssert((r->getType() == INTEGER),"Must provide Array of type INTEGER",EXIT);
  int size = list.size();
  
  // Handle size 
  int dimSize[1];
  dimSize[0]=size;
  r->setDimSizes(dimSize);

  // This casts to a not const VPDlist without invoking a constructor
  VPDlist_iter i(CAST_NO_CTOR(VPDlist,list));
  int pos = 0;
  while(!i.Eol()) {
    StaticDescriptorBase * staticDescriptor = 
      (StaticDescriptorBase *)(i.Current());
    
    Static_Id id = ResolveStaticDescriptorId(staticDescriptor);

    r->setCellValue(id,pos);
    i.Next();
    pos++;
  }
}

ostream & operator << (ostream & o, const StaticDescriptorBase *s) {
  Static_Id id = s->GetId();

  // Handle unresolved ids:
  if (id != BAD_STATIC_ID_VALUE) {
    o << '<' << id << '>';
  } else {
    o << '[' << (void *)(s) << ']';
  }
  return o;
}


ostream & operator << (ostream & o, const StaticDescriptorList & l) {
  VPDlist_iter i(CAST_NO_CTOR(VPDlist,l.list));
  int pos = 0;
  while(!i.Eol()) {
    StaticDescriptorBase * staticDescriptor = 
      (StaticDescriptorBase *)(i.Current());

    o << staticDescriptor;

    i.Next();
    if (!i.Eol()) { 
      o << ", ";
    }
  }
  return o;
}

void StaticDescriptorList::Dump() const {
  cout << *this;
}

void StaticDescriptorList::SddfDumpList(PipeWriter & p) const {
  VPDlist_iter i(CAST_NO_CTOR(VPDlist,list));
  int pos = 0;
  while(!i.Eol()) {
    StaticDescriptorBase * staticDescriptor = 
      (StaticDescriptorBase *)(i.Current());
    staticDescriptor->SddfDump(p);
    i.Next();
  }
}




