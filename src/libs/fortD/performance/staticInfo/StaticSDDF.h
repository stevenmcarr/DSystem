/* $Id: StaticSDDF.h,v 1.1 1997/03/11 14:29:05 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// This may look like C code, but it is really -*- C++ -*-
// $Id: StaticSDDF.h,v 1.1 1997/03/11 14:29:05 carr Exp $
////////////////////////////////////////////////////////////////////////////
// Declarations and C wrappers for class StaticSDDF
////////////////////////////////////////////////////////////////////////////

#ifndef StaticSDDF_h
#define StaticSDDF_h

#include <libs/fortD/performance/staticInfo/SD_Base.h>

// This code was originally written by Vikram Adve, maimed by Mark Anderson
// There was a class StaticSDDFInfo, which replicated functionality
// provided by StaticDescriptorBase. I removed it and changed
// references to it to references to StaticDescriptorBase

//--------------------------------------------------------------------------
// class ArrayOfStaticSDDFInfo
// 
// Table in which the static SDDF records can be stored.  As each
// procedure finishes compiling, add its static SDDF records to this table.
// Note that the AST will be destroyed after each module completes, so
// cannot leave this stuff in the side arrays.  Also cannot print it till
// the entire program completes because may have pointers to static SDDF
// records from other modules.
// usage of () iterator: while ((staticSDDFInfo = infoArray()) != NULL) { ... }

const int INITSIZE = 16;		// Use 512 for overall program table

class ArrayOfStaticSDDFInfo {
  private:
    StaticDescriptorBase**	array;
    int			arraySize;
    int			growSize;
    int			nextSlot;
  public:
    ArrayOfStaticSDDFInfo(int initSize = INITSIZE);
    ~ArrayOfStaticSDDFInfo();
    int			Size();
    // Add a record
    void		Add(StaticDescriptorBase* staticSDDFInfo); 
    // get ith entry
    StaticDescriptorBase*	operator [] (int i);	
    // iterate over entries 1..Size
//    StaticDescriptorBase*	operator () ();		
};


//--------------------------------------------------------------------------
// I have provided a class SDDF_Symbolic_Value that apparently does
// everything SymbolicInfo does. (See SD_Base.h) I am replacing it. 
// class SymbolicInfo : public StaticSDDFInfo {
//   private:
//     int	 symbolicValue;				// Static ID of this record
//   public:
//     SymbolicInfo(int Id, int Value = NO_SUCH_VALUE);
//     ~SymbolicInfo() {};
// };


//--------------------------------------------------------------------------
#endif  /* StaticSDDF_h */
