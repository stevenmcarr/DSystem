/* $Id: FortTreeModAttr.h,v 1.1 1997/03/11 14:27:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef FortTreeModAttr_h
#define FortTreeModAttr_h

#ifdef __cplusplus

// ***************************************************************************
// FortTreeModAttr.h
//
// Author: John Mellor-Crummey                                August 1993
//
// Copyright 1993, Rice University
// ***************************************************************************


#ifndef Attribute_h
#include <libs/fileAttrMgmt/attributedFile/Attribute.h>
#endif

#ifndef FortTree_h
#include <libs/frontEnd/fortTree/FortTree.h>
#endif


class FortTreeModAttr : public Attribute {
public:
  FortTree ft;
  
  FortTreeModAttr();
  ~FortTreeModAttr();
  
  CLASS_NAME_FDEF(FortTreeModAttr);
  
  virtual int Create();
  virtual void Destroy();
  
  virtual int ComputeUpCall();
  virtual void DetachUpCall();
  
  AST_INDEX Root();
  
  ft_States GetState();
  ft_States GetCheckedState();
  void ResetStateToInitialized();
  
  NeedProvSet *GetNeeds();
  NeedProvSet *GetProvs();
  
  virtual int ReadUpCall(File *file);
  virtual int WriteUpCall(File *file);
  
  void NoteChange(Object *ob, int kind, void *change);
};

CLASS_NAME_EDEF(FortTreeModAttr);


#else

typedef void *FortTreeModAttr;

#endif

#endif /* FortTreeModAttr_h */

