/* $Id: StaticIDTable.C,v 1.1 1997/03/11 14:29:14 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// $Id: StaticIDTable.C,v 1.1 1997/03/11 14:29:14 carr Exp $
//**************************************************************************
// class StaticIDTable : table matching FD source constructs  with static IDs
//**************************************************************************

#include <assert.h>
#include <string.h>

#ifndef pt_util_h			// For pt_itoa()
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>
#endif
#ifndef HashTable_h			// For canned string hash functions
#include <libs/support/tables/HashTable.h>
#endif

#include <libs/fortD/performance/stats/StaticIDTable.h>

#define DUMMY_STATIC_ID	    -1

//-------------------------------------------------------------------------
// GLOBAL (EXTERN) DECLARATIONS

// Thise should be a member of the class, but that doesn't work for callback:
void  CleanupTableEntryCallback(Generic name, Generic value);
    
//**************************************************************************
// Member functions for class StaticIDTable.
//**************************************************************************

// Public member functions

StaticIDTable::StaticIDTable()
{
    NameValueTable::Create(STATICID_TABLE_INIT_SIZE,
			 (NameHashFunctFunctPtr) StringHashFunct,
			 (NameCompareFunctPtr) StringEntryCompare,
			 (NameValueCleanupFunctPtr) CleanupTableEntryCallback);
    
    procedureIDListNumValues = 0;
    procedureIDListCurIndex  = 0;
    procedureIDListSize = STATICID_TABLE_INIT_SIZE;
    procedureIDList = new int[procedureIDListSize];
}

StaticIDTable::~StaticIDTable()
{
    NameValueTable::Destroy();		// Calls CleanupHashEntryCallback to
    delete procedureIDList;		//   delete stored hash keys.
}

// This is invoked on each name,value pair from NameValueTable::Destroy()
void CleanupTableEntryCallback(Generic name, Generic value)
{
    delete[] (char*) name;
}

Boolean StaticIDTable::StoreStaticID(char *fileName, int lineNum,
				     int charPosNum, int staticID)
{
    char* hashKey = MakeHashKey(fileName, lineNum, charPosNum);
    Boolean retval = NameValueTable::AddNameValue((Generic) hashKey,
					(Generic) staticID, NULL/*oldValue*/);
    //Dont free hashKey: AddNameValue(..) keeps only the pointer, not contents.
    return retval;
}
    
int StaticIDTable::GetStaticID(char *fileName, int lineNum, int charPosNum)
{
    int sid = INVALID_VALUE;
    char* hashKey = MakeHashKey(fileName, lineNum, charPosNum);
    (void) NameValueTable::QueryNameValue((Generic) hashKey, (Generic*) &sid);
    FreeHashKey(hashKey);
    return sid;
}

void StaticIDTable::NoteProcedureId(int procID)
{
    if (procedureIDListNumValues == procedureIDListSize - 1) {
	int* newProcedureIDList = new int[2 * procedureIDListSize];
	memcpy(newProcedureIDList, procedureIDList,
	       procedureIDListSize * sizeof(int));
	procedureIDListSize *= 2;
	delete[] procedureIDList;
	procedureIDList = newProcedureIDList;
    }
    procedureIDList[procedureIDListNumValues++] = procID;
}
    
int StaticIDTable::ProcedureIDList()
{
    if (procedureIDListCurIndex == procedureIDListNumValues) {
	procedureIDListCurIndex = 0;
	return DUMMY_STATIC_ID;
    }
    else
	return procedureIDList[procedureIDListCurIndex++];
}


//--------------------------------------------------------------------------
// Private member functions
//--------------------------------------------------------------------------

char* StaticIDTable::MakeHashKey(char *fileName, int lineNum, int charPosNum)
{
    char *hashKey;
    hashKey = new char[strlen(fileName) + 32 + 1]; // strlen(integer) <= 16
    strcpy(hashKey, fileName);
    
    if (lineNum < 0) {			// Arg to pt_itoa must be non(-)ve
	strcat(hashKey, "-");
	pt_itoa(-lineNum, hashKey + strlen(hashKey));
    }
    else
	pt_itoa(lineNum, hashKey + strlen(hashKey));
    
    if (charPosNum < 0) {
	strcat(hashKey, "-");
	pt_itoa(-charPosNum, hashKey + strlen(hashKey));
    }
    else
	pt_itoa(charPosNum, hashKey + strlen(hashKey));
    
    return hashKey;
}

void StaticIDTable::FreeHashKey(char* hashKey)
{
    assert(hashKey != (char*) NULL);	// Not checking that arg hashKey
    delete[] hashKey;			// was really created by MakeHashKey
}

//**************************************************************************
