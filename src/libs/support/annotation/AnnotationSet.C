/* $Id: AnnotationSet.C,v 1.1 1997/03/11 14:36:27 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//*************************************************************************** 
// AnnotationSet.C
//                                                                          
//   can be used to store (by reference) anything derived from the 
//   Annotation base class.
//                                                                          
// Author:  John Mellor-Crummey                            July 1994
//
// Copyright 1994, Rice University
//                                                                          
//***************************************************************************


#include <stdio.h>

#include <libs/support/file/FormattedFile.h>
#include <libs/support/annotation/AnnotationSet.h>
#include <libs/support/registry/ClassInstanceRegistry.h>
#include <libs/support/annotation/AnnotationMgr.h>
#include <libs/support/annotation/Annotation.h>
#include <libs/support/strings/rn_string.h>
#include <libs/support/strings/StringIO.h>



//***************************
// forward declarations
//***************************

static int ReadAnnotation(FormattedFile *file, ClassInstanceRegistry *registry, 
			  AnnotationSet *table);

static int WriteAnnotation(FormattedFile *file, ClassInstanceRegistry *registry, 
			   Annotation *annot);



//**********************************************************************
// class AnnotationSet interface operations
//**********************************************************************

CLASS_NAME_IMPL(AnnotationSet);

AnnotationSet::AnnotationSet()
{
}


AnnotationSet::~AnnotationSet()
{
}


Annotation *AnnotationSet::QueryEntry(const char *name)
{
  return (Annotation *) NamedObjectTable::QueryEntry(name);
}


int AnnotationSet::Write(FormattedFile *file, ClassInstanceRegistry *registry) 
{ 
  int code = file->Write(NumberOfEntries());
  if (code) return code;

  AnnotationSetIterator annots(this);
  for(Annotation *annot; annot = annots.Current(); annots++) {
    int code = WriteAnnotation(file, registry, annot);
    if (code < 0) return code;
  }

  return 0;
}


int AnnotationSet::Read(FormattedFile *file, ClassInstanceRegistry *registry)
{ 
  int count;
  file->Read(count);

  for (int i = 0; i < count; i++) {
    int code = ReadAnnotation(file, registry, this);
    if (code < 0) return EOF;
  }

  return 0;
}


void AnnotationSet::Dump() 
{
  NamedObjectTableDump();
}
 
 

//**********************************************************************
// implementation of class AnnotationSetIterator
//**********************************************************************

AnnotationSetIterator::AnnotationSetIterator
(AnnotationSet *theTable) : NamedObjectTableIterator(theTable)
{
}


Annotation *AnnotationSetIterator::Current()
{
  return (Annotation *) NamedObjectTableIterator::Current();
}



//**********************************************************************
// private operations
//**********************************************************************

static int ReadAnnotation(FormattedFile *file, ClassInstanceRegistry *registry, 
			  AnnotationSet *table)
{
  char *tname; 
  if (ReadString(&tname, file)) return EOF;
  
  // if name is empty, this was a placeholder only and we are done
  if (tname[0] == 0) {
    sfree(tname);
    return 0;
  }
  
  AnnotationMgr *mgr = LOOKUP_STATIC_CLASS_INSTANCE(tname, AnnotationMgr, registry);
  Annotation *annot = mgr->New();
  
  sfree(tname);
  
  // non-zero return code indicates failure which means the
  // annotation is thrown away. the return code is propagated to the caller.
  int code = annot->ReadUpCall(file);
  
  if (code == 0) table->AddEntry(annot);
  else delete annot;
  
  return code;
}


static int WriteAnnotation(FormattedFile *file, ClassInstanceRegistry *registry, 
			   Annotation *annot)
{
  int code;
  AnnotationMgr *mgr = 
    LOOKUP_STATIC_CLASS_INSTANCE(annot->name, AnnotationMgr, registry);

  // an annotation is writable only if it has a manager
  // if not, write an empty placeholder string for the annotation to avoid
  // confusing the read routine since the count of items has already been written
  if (mgr) { 
    code = WriteString(annot->name, file) || annot->WriteUpCall(file);
  } else {
    code = WriteString("", file);
  }
  return code;
}
