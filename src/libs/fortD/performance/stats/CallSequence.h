/* $Id: CallSequence.h,v 1.1 1997/03/11 14:29:08 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// $Id: CallSequence.h,v 1.1 1997/03/11 14:29:08 carr Exp $ -*-c++-*-
//**************************************************************************
// class CallSequence and associated global functions :
// Save the calling sequence in the program.  Should be part of JC's code.
// For now, only used to find the static ID of the main program.
//**************************************************************************

#ifndef CallSequence_h
#define CallSequence_h

#ifndef general_h			// For enum Boolean 
#include <libs/support/misc/general.h>
#endif
#ifndef RecordDossier_h
#include <RecordDossier.h>
#endif

class CallSequence
{
  private:
    typedef enum CallType_enum
    		{ CS_EXIT=0, CS_ENTER=1 } CallType;
    enum 	{ MAX_PROCS = 256, INIT_LIST_SIZE = 256 };
    int		procNum;
    int		numValues;
    int*	sidList;
    CallType*	callTypeList;
    int		curSize;
    
    void GrowArrays	(int newSize);
    
  public:
    		CallSequence	(int _procNum, int size = INIT_LIST_SIZE);
    		~CallSequence	();
    void	Enter		(int staticID);
    void	Exit		(int staticID);
    Boolean	Verify		();
    int		ProgramID	();
};


//**************************************************************************
// Functions used to obtain informaton about the CallSequence
//**************************************************************************

EXTERN(int,	ProgramID,		());

EXTERN(Boolean,	VerifyCallSequence,	());

extern void	AddToCallSequence	(RecordDossier& origDossier);

#endif /* CallSequence_h */
