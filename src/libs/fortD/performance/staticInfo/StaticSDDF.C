/* $Id: StaticSDDF.C,v 1.1 1997/03/11 14:29:05 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// This may look like C code, but it is really -*- C++ -*-
// $Id: StaticSDDF.C,v 1.1 1997/03/11 14:29:05 carr Exp $ 
////////////////////////////////////////////////////////////////////////////
// Declarations and C wrappers for class StaticSDDF
////////////////////////////////////////////////////////////////////////////

// This code was originally written by Vikram Adve, maimed by Mark Anderson

// There was a class StaticSDDFInfo, which replicated functionality
// provided by StaticDescriptorBase. I removed it and changed
// references to it to references to StaticDescriptorBase

static const char * RCS_ID = "$Id: StaticSDDF.C,v 1.1 1997/03/11 14:29:05 carr Exp $";
#define MKASSERT
#define ASSERT_FILE_VERSION RCS_ID

#include <libs/fortD/performance/staticInfo/SD_Base.h>
#include <libs/fortD/performance/staticInfo/StaticSDDF.h>
#include <libs/fortD/performance/staticInfo/MkAssert.h>

//--------------------------------------------------------------------------
// class ArrayOfStaticSDDFInfo

ArrayOfStaticSDDFInfo::ArrayOfStaticSDDFInfo(int initSize) {
  arraySize = initSize; 
  growSize  = initSize;
  nextSlot  = 0;
  array = new  StaticDescriptorBase * [initSize];
}	

ArrayOfStaticSDDFInfo::~ArrayOfStaticSDDFInfo() {
  delete[] array;
}

int ArrayOfStaticSDDFInfo::Size() { 
  return arraySize; 
}

void ArrayOfStaticSDDFInfo::Add(StaticDescriptorBase * staticSDDFInfo) {
  if (nextSlot == arraySize - 1) {	// then grow array by growSize
    int newSize = arraySize + growSize;
    StaticDescriptorBase** newArray = new StaticDescriptorBase * [newSize];
    for (int i=0; i < newSize; i++)
      newArray[i] = array[i];
    delete[] array;
    array = newArray;
    arraySize = newSize;
  }
  array[nextSlot++] = staticSDDFInfo;
}

StaticDescriptorBase* ArrayOfStaticSDDFInfo::operator [] (int i) { 
  ProgrammingErrorIfNot(i < arraySize); 
  return array[i]; 
}

//StaticSDDFInfo*	ArrayOfStaticSDDFInfo::operator () () {}

