/* $Id: SD_List.h,v 1.2 2001/10/12 19:33:01 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// -*- C++ -*-
/* Explanatory one line comment on file nature */
/* $Header: /home/cs/carr/cvsroot/DSystem/src/libs/fortD/performance/staticInfo/SD_List.h,v 1.2 2001/10/12 19:33:01 carr Exp $
//
*/
// The tag should exactly match the filename
#ifndef _SD_List_h
#define _SD_List_h
// All includes and definitions go here. Include only the minimum set required to include this file. Do not include anything that is only required by the coressponding .c file (if there is one)

#include <iostream.h>
//#include "Array.h" Avoid need for messy conflicts.
#include <libs/fortD/performance/staticInfo/SD_Base.h>

#include <libs/fortD/performance/staticInfo/VPDlist.h>

class Array; // All I need is a pointer, not the whole definition.

ostream & operator << (ostream & o, const StaticDescriptorBase *s);
Static_Id ResolveStaticDescriptorId(const StaticDescriptorBase *s);

// Holds pointers to StaticDescriptorBase, and then when the time
// comes forces them to ids, and stuffs SDDF arrays.
// Limitations:
// Assumes that array is of type INTEGER
// Currently does not insure that items in list are unique. 
// If Id has not been assigned will misbehave.
// Note: Will fix all of these eventually.
class StaticDescriptorList  {
public:
  StaticDescriptorList();
  ~StaticDescriptorList();

// AddElement adds a new item to the list
  void AddElement(StaticDescriptorBase * s);
// ResolveToArray takes all of these pointers, resolves them to Static
// ids, and then stuffs them into the array.
  void ResolveToArray(Array * r) const;

  void SddfDumpList(PipeWriter & p) const;

  void Dump() const;
  friend  ostream & operator << (ostream & o, const StaticDescriptorList & l);
private:
  VPDlist list;
};

// Don't forget terminal semicolon on classes!
#endif




