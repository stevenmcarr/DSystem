/* $Id: FortTextTreeModAttr.h,v 1.1 1997/03/11 14:27:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef FortTextTreeModAttr_h
#define FortTextTreeModAttr_h

#ifdef __cplusplus

// ***************************************************************************
// FortTextTreeModAttr.h
//
// Author: John Mellor-Crummey                                August 1993
//
// Copyright 1993, Rice University
// ***************************************************************************

#ifndef FortTextTree_h
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#endif


#ifndef Attribute_h
#include <libs/fileAttrMgmt/attributedFile/Attribute.h>
#endif


class FortTreeModAttr; // minimal external definition

class FortTextTreeModAttr : public Attribute {
private:
  FortTreeModAttr *ftAttr;
public:
  FortTextTree ftt;

  FortTextTreeModAttr();
  ~FortTextTreeModAttr();

  CLASS_NAME_FDEF(FortTextTreeModAttr);

  virtual int Create();
  virtual void Destroy();

  virtual int ReComputeUpCall();
  virtual int ComputeUpCall();

  virtual int ReadUpCall(File *file);
  virtual int WriteUpCall(File *file);

  void NoteChange(Object *ob, int kind, void *change);
};

CLASS_NAME_EDEF(FortTextTreeModAttr);

#else

typedef void *FortTextTreeModAttr;

#endif

#endif /* FortTextTreeModAttr_h */
