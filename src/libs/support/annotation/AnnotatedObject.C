/* $Id: AnnotatedObject.C,v 1.1 1997/03/11 14:36:26 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//*********************************************************************
// AnnotatedObject.C
//
// Author: John Mellor-Crummey                             July 1994
//
// Copyright 1994, Rice University
//*********************************************************************

#include <stdio.h>

#include <libs/support/annotation/Annotation.h>
#include <libs/support/annotation/AnnotatedObject.h>
#include <libs/support/annotation/AnnotationSet.h>



//*********************************************************************
// class AnnotatedObject interface operations
//*********************************************************************

AnnotatedObject::AnnotatedObject()
{
  annotations = 0;
}


AnnotatedObject::~AnnotatedObject()
{
  delete annotations;
}


Annotation *AnnotatedObject::QueryAnnot(const char *name) 
{
  return (annotations ? annotations->QueryEntry(name) : 0);
}


void AnnotatedObject::AddAnnot(Annotation *annot)
{
  if (annotations == 0) annotations = new AnnotationSet;
  annotations->AddEntry(annot);
}


void AnnotatedObject::DeleteAnnot(const char *name)
{
  annotations->DeleteEntry(name);
}


ClassInstanceRegistry *AnnotatedObject::GetAnnotMgrRegistry() 
{
  return 0;
}


int AnnotatedObject::ReadAnnotations(FormattedFile *file)
{
  ClassInstanceRegistry *registry = GetAnnotMgrRegistry();
  if (registry) {
    if (annotations == 0) annotations = new AnnotationSet;
    return annotations->Read(file, registry);
  } return 0;
}


int AnnotatedObject::WriteAnnotations(FormattedFile *file)
{ 
  ClassInstanceRegistry *registry = GetAnnotMgrRegistry();
  if (registry) {
    if (annotations) return annotations->Write(file, registry);
    else {
      annotations = new AnnotationSet;
      int code = annotations->Write(file, registry);

      delete annotations;
      annotations = 0;
    }
  } else return 0;
}


void AnnotatedObject::DumpAnnotations()
{ 
  if (annotations) annotations->Dump();
}
