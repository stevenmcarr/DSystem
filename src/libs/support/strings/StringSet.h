/* $Id: StringSet.h,v 1.1 1997/03/11 14:37:31 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef StringSet_h
#define StringSet_h

//*************************************************************************** 
// StringSet.h                                                January 1994  
//
// Author: John Mellor-Crummey                                             
//                                                                        
// Each entry in the set contains a pointer to a string. Strings will be
// freed with sfree (see rn_string.h) if deallocation on cleanup is requested. 
//                                                  
// Copyright 1994, Rice University
//***************************************************************************


#ifndef NamedObjectTable_h
#include <libs/support/tables/namedObject/NamedObjectTable.h>
#endif


class FormattedFile; // minimal external declaration


//-------------------------------------------------------------
// class StringSet
//-------------------------------------------------------------
class StringSet: private NamedObjectTableIO {
private:
  NamedObjectIO *NewEntry();
public:
  StringSet();
  StringSet(StringSet &rhs);
  virtual ~StringSet();
  
  NamedObjectTableIO::NumberOfEntries;
  
  void Add(const char *string);
  void Delete(const char *string);

  int IsMember(const char *string);
  const char *GetMember(unsigned int canonicalIndex); // 0 --> invalid index
  int GetMemberIndex(const char *name); // -1 --> not present

  int operator ==(StringSet &rhs);
  int operator !=(StringSet &rhs);
  void operator |=(StringSet &rhs);

  int Read(FormattedFile *file);
  int Write(FormattedFile *file);
  
  void Dump();
  void DumpContents();

friend class StringSetIterator;
};


//-------------------------------------------------------------
// class StringSetIterator
//-------------------------------------------------------------
class StringSetIterator: private NamedObjectTableIterator {
public:
  StringSetIterator(const StringSet *set);
  ~StringSetIterator();

  const char *Current() const;
  NamedObjectTableIterator::operator++;
  NamedObjectTableIterator::Reset;
};

#endif
