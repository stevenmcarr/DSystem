/* $Id: Iterator.h,v 1.1 1997/03/11 14:36:45 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// Iterator.h
//
//   a base s
//
// Author: John Mellor-Crummey                                October 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#ifndef Iterator_h
#define Iterator_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

class Iterator {
protected:
  virtual void *CurrentUpCall() const = 0;
public:
  virtual ~Iterator();
  virtual void operator++() = 0;
  virtual void Reset() = 0;
  virtual Boolean IsValid();
};

#endif
