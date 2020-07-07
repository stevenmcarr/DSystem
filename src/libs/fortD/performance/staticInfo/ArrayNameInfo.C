/* $Id: ArrayNameInfo.C,v 1.3 2001/10/12 19:33:01 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// $Id: ArrayNameInfo.C,v 1.3 2001/10/12 19:33:01 carr Exp $ -*-c++-*-
//**************************************************************************
// Definitions for class ArrayNameInfo
//
// Current assumptions:
// 1.  Not supporting aliases, i.e., equivalence not supported, and
//     assume that identical names are used for entries of the same
//     common block in different procedures
//**************************************************************************

//-------------------------------------------------------------------------
// INCLUDES
//
#include <stdio.h>
#include <string.h>
#include <values.h>

#ifndef general_h
#include <libs/support/misc/general.h>
#endif
#ifndef rn_string_h			// For ssave, sfree
#include <libs/support/strings/rn_string.h>
#endif
#ifndef HashTable_h			// For canned string hash functions
#include <libs/support/tables/HashTable.h>
#endif
#ifndef pt_util_h			// For decl of pt_itoa()
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>
#endif

#ifndef fortsym_h			// For symbol table access
#include <libs/frontEnd/fortTree/fortsym.h>
#endif
					// From the performance tool world
#ifndef _SDDF_General_h
#include <libs/fortD/performance/staticInfo/SDDF_General.h>
#endif
#ifndef _SD_SrcInfo_h
#include <libs/fortD/performance/staticInfo/SD_SrcInfo.h>
#endif
#ifndef _SD_DataInfo_h
#include <libs/fortD/performance/staticInfo/SD_DataInfo.h>
#endif
#include <libs/fortD/performance/staticInfo/ArrayNameInfo.h>

using namespace std;

//-------------------------------------------------------------------------
// Declarations of functions private to this file

// This is invoked on each name,value pair from NameValueTable::Destroy()
// Declared as friend of ArrayNameInfo, so InfoTableEntry is accessible.
// This should be private (i.e., static) but that would disallow
// friend declaraiton.

void DestroyEntryCallback(Generic /*name*/, Generic value)
{
    // first, walk the array-name list and delete each entry
    ArrayNameListEntry* curEntry;
    ArrayNameListEntry* nextEntry= ((InfoTableEntry*)value)->equivNameListHead;
    while (nextEntry != (ArrayNameListEntry*) NULL) {
	curEntry = nextEntry;
	nextEntry = curEntry->next;
	delete curEntry;
    }
    
    delete (InfoTableEntry*) value;
}					


//-------------------------------------------------------------------------
// Public member functions

ArrayNameInfo::ArrayNameInfo()
{
    infoTable.Create(INFO_TABLE_INIT_SIZE,
		     (NameHashFunctFunctPtr) StringHashFunct,
		     (NameCompareFunctPtr) StringEntryCompare,
		     (NameValueCleanupFunctPtr) DestroyEntryCallback);
    curProcInfo.procName = (char*) NULL;
}

ArrayNameInfo::~ArrayNameInfo()
{
    infoTable.Destroy();		// Calls DestroyEntryCallback to 
}					// delete each InfoTableEntry
//-------------------------------------------------------------------------

// Call once for each procedure. Record FortTree and procName for future use.
void ArrayNameInfo::SetupForNewProcedure(FortTree _ft, char* _procName)
{
    curProcInfo.ft = _ft;
    assert(curProcInfo.ft != (FortTree) NULL);
    if (curProcInfo.procName != (char*) NULL) sfree(curProcInfo.procName);
    curProcInfo.procName = ssave(_procName);
}

// GetSymTable() : Use the above to obtain a handle to the symbol table.
// NOTE: This must call ft_SymGetTable(..) every time because the
//       symbol table may get recomputed during compilation.

SymDescriptor ArrayNameInfo::GetSymTable()
{
    return ft_SymGetTable(curProcInfo.ft, curProcInfo.procName);
}

//-------------------------------------------------------------------------

// Obtain the unique SDDF_ArrayInfo descriptor for an array:
// 1. Get a unique name key for this array. See GetNameKey() for current impl.
// 2. Hash this key to check if an entry already exists for this array or any
//    of its aliases (i.e., another name in the same position in a common
//    block. Equivalences and formal parameters not tested.)
// 3. If an entry does not exist if the argument createNew == true,
//    create a new one with blank SDDF_ArrayInfo
// 4. Return entry

InfoTableEntry* ArrayNameInfo::GetFullEntry(char* arrayName, Boolean createNew)
{
    // ensure we are within the FortranD compiler context for a procedure:
    assert(thePabloGlobalInfo.currentLocals != (PabloLocalInfo*) NULL);
    
    InfoTableEntry* tableEntry;
    char* arrayNameKey;

    // Create the arrayName, procInfo struct for this name.
    // May not be used if an entry for *this* name exists in the name list for
    // its own entry.
    ArrayNameListEntry* nameEntry = new ArrayNameListEntry;
    strcpy(nameEntry->arrayName, arrayName);
    nameEntry->procInfo = thePabloGlobalInfo.currentLocals->procInfo;
    
    // type returned by gen_get_node_type() for func/subr/pgm ast:
    // nameEntry->procType =
    //	thePabloGlobalInfo.currentLocals->procEntry->type;
    nameEntry->procType = MAXINT;	// V.S.A, 8/12: To avoid UMR error
    nameEntry->next = (ArrayNameListEntry*) NULL;

	    
    // Get a unique key arrayNameKey and check if a table entry exists for it
    arrayNameKey = GetNameKey(arrayName, GetSymTable());
    if (infoTable.QueryNameValue((Generic) arrayNameKey,
				 (Generic*) &tableEntry))
    {
	// Entry already exists. Check if name appears in list for the entry,
	// and if not, append name entry to name list. Return entry.
	Boolean nameExistsInList = false;
	ArrayNameListEntry* nextName = tableEntry->equivNameListHead;
	for ( ; nextName; nextName = nextName->next) {
	    if (strcmp(nextName->arrayName, arrayName) == 0) {
		nameExistsInList = true;
		break;
	    }
	}
	
	if (nameExistsInList)
	    delete nameEntry;
	else {
	    // There is an entry for this array by another name, but this
	    // name is not recorded as an alias. Insert name struct in the list
	    if (tableEntry->equivNameListTail == ((ArrayNameListEntry*) NULL))
		tableEntry->equivNameListHead = nameEntry;
	    else
		tableEntry->equivNameListTail->next = nameEntry;
	    tableEntry->equivNameListTail = nameEntry;
	}
	delete[] arrayNameKey;
	return tableEntry;
    }
    // If you get here, then no entry exists for this array by any name
    
    if (!createNew) {
	delete nameEntry;
	delete[] arrayNameKey;
	return (InfoTableEntry*) NULL;
    }

    // Caller wants to create a new table entry. Create the new entry,
    // including a new blank SDDF_ArrayInfo record.
    // Store key and the name entry in it.
    
    tableEntry = new InfoTableEntry;
    tableEntry->arrayInfo = new SDDF_ArrayInfo; 
    tableEntry->nameKey = arrayNameKey;
    tableEntry->equivNameListHead = nameEntry;
    tableEntry->equivNameListTail = nameEntry;
    tableEntry->beenHere = false;
    
    // Stash away the pointer to the table entry in the hash table
    if (infoTable.AddNameValue((Generic) arrayNameKey,
			       (Generic) tableEntry, (Generic) NULL)) {
	char errbuf[128];
	(void) sprintf(errbuf, "Entry exists for key %s ?\n", arrayNameKey);
	die_with_message(errbuf);
    }
    return tableEntry;
}
//-------------------------------------------------------------------------

// Retrieve the unique SDDF_ArrayInfo descriptor for an array
SDDF_ArrayInfo*	ArrayNameInfo::GetArrayInfo(char* arrayName, Boolean createNew)
{
    InfoTableEntry* entry = GetFullEntry(arrayName, createNew);
    return (entry != (InfoTableEntry*) NULL)? entry->arrayInfo : 
	   (SDDF_ArrayInfo*) NULL;
}


//--------------------------------------------------------------------------
// Private member functions

// char* GlobalArrayNameKey()
//	Computes a search key that is common to aliased versions of a given
// array name throughout the target program. Currently this key is:
// for global variables: "common-blk-name#itoa(offset)#itoa(size)"
// for local  variables: "procedure-name#array-name"
// for formal parameter names: NOT SUPPORTED YET.
//
// CAVEAT: Currently only handles names that are aliased via common blocks.
//         Does not handle procedure formal param names that are bound
//         to a particular distributed array.  Need to propagate info from
//         call sites to do that.
//	   Also does not handle EQUIVALENCE stmts.

char* ArrayNameInfo::GetNameKey(char* arrayName, 		// Given array
				SymDescriptor symTable)		// Symbol table
{
    // Get index of common block name by getting the leader, then get
    // the common block name itself.
    // Much of this copied from: $RN_SRC/execs/a2i_ex/addr.c:BaseAddressName(i)
    
    char* nameKey = new char[NAME_KEY_MAX_LEN];
    int CTi, CToffset;
    unsigned int CTsize;
    int arrayFstIndex = fst_Index(symTable, arrayName);
    
    // Get storage class for array to test if global or local
    int SC = fst_GetFieldByIndex(symTable, arrayFstIndex,SYMTAB_STORAGE_CLASS);
    if (SC & SC_GLOBAL) {
	// For global: get common-blk-index, offset, size, and verify
	fst_Symbol_To_EquivLeader_Offset_Size(symTable, arrayFstIndex,
					      &CTi, &CToffset, &CTsize);
	if ((CTi == SYM_INVALID_INDEX) ||
	    (! (fst_GetFieldByIndex(symTable,
			    CTi,SYMTAB_OBJECT_CLASS) & OC_IS_COMMON_NAME)))
	{
	    char error_buffer[128];
	    (void) sprintf(error_buffer, 
		   "Variable '%s' is GLOBAL but has no COMMON name\n",
		   (char *) fst_GetFieldByIndex(symTable,
						arrayFstIndex, SYMTAB_NAME));
	    die_with_message(error_buffer);
	}
	
	// Get common block name and construct key
	char strtmp[MAX_NAME];
	(void) strcpy(nameKey, (char*) fst_GetFieldByIndex(symTable,
							   CTi,SYMTAB_NAME));
	(void) strcat(nameKey, "#");
	if (CToffset == 0) (void) strcpy(strtmp, "0");
	else pt_itoa(CToffset, strtmp);	// integer arg to pt_itoa must be > 0
	(void) strcat(nameKey, strtmp);
	(void) strcat(nameKey, "#");
	if (CTsize == 0) (void) strcpy(strtmp, "0");
	else pt_itoa(CTsize, strtmp);
	(void) strcat(nameKey, strtmp);
    }
    else {				// No common block => local?
	// Prepend current procedure name# to var name
	(void) strcpy(nameKey, thePabloGlobalInfo.currentLocals->procName);
	(void) strcat(nameKey, "#");
	(void) strcat(nameKey, arrayName);
    }
    
    return nameKey;
}


//-------------------------------------------------------------------------
// Member functions of class ArrayNameIterator 
//-------------------------------------------------------------------------

ArrayNameIterator::ArrayNameIterator(const ArrayNameInfo* arrayNameTable)
{
    infoTableIterator
	= new NameValueTableIterator(&(arrayNameTable->infoTable));
    Reset();
}

ArrayNameIterator::~ArrayNameIterator()
{
    delete infoTableIterator;
}

void ArrayNameIterator::operator ++()
{
    ++(*infoTableIterator);
    SetCurFields(infoTableIterator);
}

void ArrayNameIterator::Reset ()
{
    infoTableIterator->Reset();
    SetCurFields(infoTableIterator);
}

void ArrayNameIterator::SetCurFields(NameValueTableIterator* infoTableIterator)
{
    if ((arrayName = (char*) infoTableIterator->name) != (char*) NULL) {
	fullEntry = (InfoTableEntry*) infoTableIterator->value;
	assert(fullEntry != (InfoTableEntry*) NULL);
	arrayInfo = fullEntry->arrayInfo;
    }
    else {
	fullEntry = (InfoTableEntry*) NULL;
	arrayInfo = (SDDF_ArrayInfo*) NULL;
    }
}


//-------------------------------------------------------------------------
