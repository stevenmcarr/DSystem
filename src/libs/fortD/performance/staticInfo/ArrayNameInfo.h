/* $Id: ArrayNameInfo.h,v 1.1 1997/03/11 14:28:54 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// $Id: ArrayNameInfo.h,v 1.1 1997/03/11 14:28:54 carr Exp $ -*-c++-*-
// **************************************************************************
// Declarations for class ArrayNameInfo
// The class is used to resolve global/local array names and aliases
// (equivalences and commons) and obtain access to the unique SDDF_ArrayInfo
// descriptor for a particular local or global array by any of its
// equivalent names.
//
// Current assumptions:
// 1.  Not supporting aliases, i.e., equivalence not supported, and
//     assume that identical names are used for entries of the same
//     common block in different procedures
// **************************************************************************

#ifndef ArrayNameInfo_h
#define ArrayNameInfo_h

#ifndef NameValueTable_h
#include <libs/support/tables/NameValueTable.h>
#endif

#ifndef PointerVector_h
#include <libs/support/vectors/PointerVector.h>
#endif
#ifndef _SD_Decls_h
#include <libs/fortD/performance/staticInfo/SD_Decls.h>
#endif
#ifndef _SD_DataInfo_h
#include <libs/fortD/performance/staticInfo/SD_DataInfo.h>
#endif

typedef struct ArrayNameListEntry_struct {
    char arrayName[MAX_NAME];		// array name
    SDDF_ProcInfo* procInfo;		// name was entered from this procedure
    unsigned int procType;	 //GEN_FUNCTION / GEN_SUBROUTINE / GEN_PROGRAM
    ArrayNameListEntry_struct* next;	// pointer to next entry in list
} ArrayNameListEntry;

struct InfoTableEntry {
  char *nameKey;
  ArrayNameListEntry *equivNameListHead; // list of names for same array with
  ArrayNameListEntry *equivNameListTail; // list of names for same array with
  SDDF_ArrayInfo* arrayInfo;		 // common array info for all names
  Boolean beenHere;
  PointerVector correspAligns;		// Contains AST_Indexes.
};


//--------------------------------------------------------------------------

class ArrayNameInfo {				// A permanent data structure
  public:
    ArrayNameInfo();
    ~ArrayNameInfo();
  
    // Call once for each procedure to record symTable pointer.
    void 		SetupForNewProcedure	(FortTree _ft, char* procName);
    
    SDDF_ArrayInfo*	GetArrayInfo		(char* arrayName,
						 Boolean createNew = true);
    
    InfoTableEntry *    GetFullEntry		(char* arrayName,
						 Boolean createNew = true);
    
    friend class ArrayNameIterator;

  private:
    friend void DestroyEntryCallback(Generic name, Generic value);
    enum ArrayNameInfoConstants { INFO_TABLE_INIT_SIZE = 64,
			          NAME_KEY_MAX_LEN = 2 * MAX_NAME };
    NameValueTable infoTable;
    
    struct curProcInfo_struct {
	char* procName;
	FortTree ft;			// Must save ft, not symTable, because
	// SymDescriptor symTable;	// this is freed and recreated by
    };					// dc_irreg_analyze()
    struct curProcInfo_struct curProcInfo;
    
    // Computes a key for a given array name. The key is the same for all
    // names used for the same array in the program.
    // Caveat: Does not yet support:
    // 1. Formal parameter names for arrays passed in as arguments
    // 2. Equivalenced arrays
    char* GetNameKey(char* arrayName,			// Given array
		     SymDescriptor symTable);		// Symbol table

    SymDescriptor GetSymTable();
};


//--------------------------------------------------------------------------

class ArrayNameIterator {
  public:
    char*		arrayName;
    SDDF_ArrayInfo*	arrayInfo;
    InfoTableEntry *    fullEntry;
    
    ArrayNameIterator(const ArrayNameInfo* arrayNameTable);
    ~ArrayNameIterator();

    void operator ++();
    void Reset();
  
  private:
    NameValueTableIterator* infoTableIterator; 
    void SetCurFields(NameValueTableIterator* infoTableIterator);
};


//--------------------------------------------------------------------------
#endif	/* ArrayNameInfo_h */
