/* $Id: MakeStaticIDTable.C,v 1.1 1997/03/11 14:29:11 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// $Id: MakeStaticIDTable.C,v 1.1 1997/03/11 14:29:11 carr Exp $
//**************************************************************************
// MakeStaticIDTable :	Create a table pairing staticIDs with code-locations 
//		 	of corresponding Fortran D source constructs.
//
// Thus, the static ID for a procedure, loop or message can be obtained
// by indexing the table with (fileName,lineNum,charPos) where charPos = -1
// for proc and loop.  For messages, the location of any non-local reference
// satisfied by the message can be used as the index.
//**************************************************************************

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <libs/fortD/performance/stats/FDCombine6.h>
						      //  any Pablo header files
#include <libs/fortD/performance/stats/StaticIDTable.h>

//--------------------------------------------------------------------------
// EXTERNAL VARIABLES AND FUNCTIONS AND FORWARD DECLARATIONS
//--------------------------------------------------------------------------

// All the variables and functions used are declared in FDCombine6.h:
// NAME_SIZE
// LINE_SIZE
// RecordDossier**	staticTable;
// int			currentTableSize;
// void			retrieveName(...);

EXTERN(void, MakeStaticIDTable, (StaticIDTable* sidTable));

STATIC(void, MakeOneTableEntry, (RecordDossier* dossier,
				 StaticIDTable* sidTable));


//--------------------------------------------------------------------------
// FUNCTIONS DEFINITIONS
//--------------------------------------------------------------------------

// MakeStaticIDTable :	Create a table pairing staticIDs with code-locations 
// 			of corresponding Fortran D source constructs.

void MakeStaticIDTable(StaticIDTable* sidTable)
{
    RecordDossier *dossier;
    char *nameString;
    for (int i=0; i < currentTableSize; i++)
	if ((dossier = staticTable[i]) != (RecordDossier*) NULL) {
	    // Check its not a static symbolic record and make table entry
	    nameString = (char*) dossier->getName().getValue();
	    assert(nameString != (char*) NULL);
	    if ( strcmp(nameString, "FDSSym Value") != 0)
		MakeOneTableEntry(dossier, sidTable);
	}
}

void MakeOneTableEntry(RecordDossier* dossier, StaticIDTable* sidTable)
{
    assert(dossier != (RecordDossier*) NULL);
    char   *fileName, *nameString;
    Array  *dependIDArray;		// Pablo's Array class
    int	   dependIDArraySize;
    int    lineNum, charPos;
    int    staticID, index;
    RecordDossier *procDossier, *dependDossier;
    
    staticID = dossier->getValue("Static ID");
    nameString = (char*) dossier->getName().getValue();
    assert(nameString != (char*) NULL);

    // Get Procedure record and retrieve fileName
    if (!strcmp(nameString, "FDStat Proc")) {
	procDossier = dossier;
	sidTable->NoteProcedureId(staticID);
    }
    else {
	index = dossier->getValue("Procedure ID");
	assert(index != DUMMY_STATIC_ID);
	procDossier = staticTable[index];
    }
    fileName = (char*) procDossier->getCString("File Name").getValue();

    // Process Message Send records and Depend records differently from rest:
    //
    // If this is a message send record: get the location information for 
    //     all Fortran D non-local references satisfied by this message.
    // If this is a depend record: Dont add static IDs to table, since
    //     depend (sink) location info is used to store static ID of messages.
    // Otherwise, add staticID to table, directly indexed by
    //     filename,lineNum,charPos (charPos = -1)
    
    if ( !strcmp(nameString, "FDStat Mesg Send")) {
	
	dependIDArray = dossier->getArrayP("Depend ID");
	dependIDArraySize = dependIDArray->getCellCount();
	for (int i = 0; i < dependIDArraySize; i++) {
	    index = dependIDArray->getCellValue(i);
	    assert(index != DUMMY_STATIC_ID);
	    dependDossier = staticTable[index];
	    lineNum = dependDossier->getValue("Dependence Sink Line Number");
	    charPos = dependDossier->getValue("Dependence Sink Reference Position");
	    sidTable->StoreStaticID(fileName, lineNum, charPos, staticID);
	}
    }
    else if (!strcmp(nameString, "FDStat Depend")) {
	;
    }
    else {
	lineNum = dossier->getValue("Line Number");
	charPos = -1;
	sidTable->StoreStaticID(fileName, lineNum, charPos, staticID);
    }
}

//--------------------------------------------------------------------------
