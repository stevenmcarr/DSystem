/* $Id: StaticIDTable.h,v 1.1 1997/03/11 14:29:14 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// $Id: StaticIDTable.h,v 1.1 1997/03/11 14:29:14 carr Exp $ -*-c++-*-
//**************************************************************************
// class StaticIDTable : table matching FD source constructs  with static IDs
//**************************************************************************

#ifndef StaticIDTable_h
#define StaticIDTable_h

#ifndef general_h			// For enum Boolean; typedef Generic
#include <libs/support/misc/general.h>
#endif
#ifndef NameValueTable_h		// For class NameValueTable
#include <libs/support/tables/NameValueTable.h>
#endif


class StaticIDTable : private NameValueTable {
  public:
    StaticIDTable();
    ~StaticIDTable();
    
    // GetStaticID	Get the staticID corresponding to the source code
    // 			construct (proc/loop/ref) at this location in code
    int		GetStaticID	(char *fileName, int lineNum, int charPosNum);
    
    // StoreStaticID	Store the staticID corresponding to the source code
    // 			construct (proc/loop/ref) at this location in code.
    // 			Returns true if the table already an entry for this.
    Boolean  	StoreStaticID	(char *fileName, int lineNum, int charPosNum,
				 int staticID);
    
    // ProcedureIDList	Returns static ID of procedures in random order.
    // 			Returns -1 when done and resets to repeat.
    int		ProcedureIDList	();
    
    void	NoteProcedureId	(int procID);
    
  private:
    enum StaticIDTable_Constants_enum { STATICID_TABLE_INIT_SIZE = 256,
				    	INVALID_VALUE = -1 };

    int* procedureIDList;
    int  procedureIDListSize;
    int  procedureIDListNumValues;
    int  procedureIDListCurIndex;

    char* MakeHashKey	(char *fileName, int lineNum, int charPosNum);
    void  FreeHashKey	(char* hashKey);
};

#endif /* StaticIDTable_h */
