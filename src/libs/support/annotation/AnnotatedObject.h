/* $Id: AnnotatedObject.h,v 1.1 1997/03/11 14:36:26 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef AnnotatedObject_h
#define AnnotatedObject_h

//*********************************************************************
// AnnotatedObject.h
//
// Author: John Mellor-Crummey                             July 1994
//
// Copyright 1994, Rice University
//*********************************************************************


class Annotation;
class AnnotationSet;
class FormattedFile;
class ClassInstanceRegistry;


//*********************************************************************
// class AnnotatedObject
//*********************************************************************

class AnnotatedObject {
public:
  AnnotatedObject();
  virtual ~AnnotatedObject();
  
  Annotation *QueryAnnot(const char *aname);
  void AddAnnot(Annotation *annot);
  void DeleteAnnot(const char *name);

  void DumpAnnotations();

protected:
  virtual ClassInstanceRegistry *GetAnnotMgrRegistry(); 

  int ReadAnnotations(FormattedFile *file);
  int WriteAnnotations(FormattedFile *file);

private:
  AnnotationSet *annotations;
};

#endif
