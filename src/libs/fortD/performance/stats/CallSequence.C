/* $Id: CallSequence.C,v 1.1 1997/03/11 14:29:08 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// $Id: CallSequence.C,v 1.1 1997/03/11 14:29:08 carr Exp $
//**************************************************************************
// class CallSequence and associated global functions :
// Save the calling sequence in the program.  Should be part of JC's code.
// For now, only used to find the static ID of the main program.
//**************************************************************************

#include <assert.h>
#include <string.h>

#include <libs/fortD/performance/stats/FDCombine6.h>

#include <libs/fortD/performance/stats/CallSequence.h>
							//  header files
#define DUMMY_STATIC_ID		-1

//--------------------------------------------------------------------------
// STATIC GLOBAL DATA
// Use a global for now. DAMN! Object should instead be created by JC's code.
//--------------------------------------------------------------------------

static CallSequence** callSequence;
					

//--------------------------------------------------------------------------
// MEMBER FUNCTION DEFINITIONS FOR class CallSequence 
//--------------------------------------------------------------------------

// GetCallSequence :	Save the entire calling sequence.
//			For now, only used to find the main program.
 
CallSequence::CallSequence(int _procNum, int size) : procNum(_procNum)
{
    sidList = (int*) NULL;
    callTypeList = (CallType*) NULL;
    GrowArrays(size);
    numValues = 0;
}
CallSequence::~CallSequence()
{
    delete[] sidList;
    delete[] callTypeList;
}

void CallSequence::Enter(int staticID) {
    assert(staticID != DUMMY_STATIC_ID);
    if (numValues == curSize - 1) GrowArrays(curSize * 2);
    sidList[numValues] = staticID;
    callTypeList[numValues] = CS_ENTER;
    ++numValues;
    
    // RecordDossier* staticDossier = staticTable[staticID];
    // for (int i=0; i<procNum; i++) printf("\t");
    // printf("--> %s\n", staticDossier->getCString("Procedure Name").getValue());
}

void CallSequence::Exit (int staticID)
{
    assert(staticID != DUMMY_STATIC_ID);
    if (numValues == curSize - 1) GrowArrays(curSize * 2);
    sidList[numValues] = staticID;
    callTypeList[numValues] = CS_EXIT;
    ++numValues;

    // RecordDossier* staticDossier = staticTable[staticID];
    // for (int i=0; i<procNum; i++) printf("\t");
    // printf("    %s\n", staticDossier->getCString("Procedure Name").getValue());
}

Boolean CallSequence::Verify()
{
    int pgmID = sidList[0];
    int numCalls = 0;
    for (int i=0; i < numValues; i++) {
	if (i > 0 && i < numValues-1 && sidList[i] == pgmID)
	    pgmID = DUMMY_STATIC_ID;
	numCalls += sidList[i] * ((callTypeList[i] == CS_ENTER)? +1 : -1);
    }
    
    assert(numCalls == 0);		// Enter and Exit calls matched?
    if (pgmID == -1 || sidList[0] != sidList[numValues-1]) {
	fprintf(stderr, "WARNING: MAIN PROGRAM NOT TRACED!!\n"
		"         Performance statistics relative to full"
		" program are not available\n");
	return false;
    }
    else return true;
}

inline int CallSequence::ProgramID()	{ return sidList[0]; }

void CallSequence::GrowArrays(int newSize)
{
    int* newsidList = new int[newSize];
    CallType* newCallTypeList = new CallType[newSize];
    if (sidList != (int*) NULL) {
	assert (callTypeList != (CallType*) NULL);
	memcpy(newsidList, sidList, numValues * sizeof(int));
	memcpy(newCallTypeList, callTypeList, numValues * sizeof(CallType));
	delete[] sidList;
	delete[] callTypeList;
    }
    sidList = newsidList;
    callTypeList = newCallTypeList;
    curSize = newSize;
}


//--------------------------------------------------------------------------
// FUNCTIONS DEFINITIONS
//--------------------------------------------------------------------------

Boolean VerifyCallSequence()
{
    Boolean valid = true;
    for (int i = 0; i < GnumNodes; i++)
	if (! callSequence[i]->Verify())
	    valid = false;
    return valid;
}

int ProgramID()
{
    return callSequence[0]->ProgramID();
}

void AddToCallSequence(RecordDossier& origDossier)
{
    if (callSequence == (CallSequence**) NULL) {	// first time: allocate
	callSequence = new CallSequence*[GnumNodes];
	for (int i = 0; i < GnumNodes; i++) {
	    callSequence[i] = new CallSequence(i);
	    assert(callSequence[i] != (CallSequence*) NULL);
	}
    }
    char* nameString = (char*) origDossier.getName().getValue();
    int procNum = origDossier.getValue("Processor Number");
    
    if (!strcmp(nameString, "FDProc1 Entry Trace"))
	callSequence[procNum]->Enter(origDossier.getValue("Static ID"));
    else if (!strcmp(nameString, "FDProc2 Exit Trace"))
	callSequence[procNum]->Exit(origDossier.getValue("Static ID"));
    else
	;				// Entry/Exit Count records ignored
}


//--------------------------------------------------------------------------
