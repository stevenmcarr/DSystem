/* $Id: FortDPerfTable.h,v 1.1 1997/03/11 14:29:11 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// $Id: FortDPerfTable.h,v 1.1 1997/03/11 14:29:11 carr Exp $ -*-c++-*-
//**************************************************************************
// Interface used by D Editor to obtain dynamic performance metrics
//**************************************************************************

#ifndef FortDPerfTable_h
#define FortDPerfTable_h

#ifndef stdio_h
#include <stdio.h>
#endif

#ifndef general_h			// For enum Boolean
#include <libs/support/misc/general.h>
#endif
#ifndef Metrics_h
#include <libs/fortD/performance/stats/Metrics.h>
#endif


//--------------------------------------------------------------------------
// Declarations of external objects (instead of including header files):

// Declare these instead of #including FDCombine6.h, so as to avoid
// including all the Pablo header files wherever this file is included.
class RecordDossier;
class FileCell;
class LineStatCell;


//--------------------------------------------------------------------------

class FortDPerfTable {
  public:
    FortDPerfTable(Boolean _incl);
    ~FortDPerfTable();
    
    void ConstructPerfTable(RecordDossier** staticTable,
			    FileCell* lineHeader,
			    LineStatCell* lineStatTable);
    
    PerfMetrics* GetPerfInfo(int staticID);

    // The next member function is being made public so an object of
    // this class can also be used to store externally computed metrics,
    // but still indexed by static ID.
    void	 AddPerfInfoEntry   (PerfMetrics* perfEntry, int staticID);
    
  private:
    Boolean	  incl;
    PerfMetrics** perfInfoTable;
    int		  perfInfoTableSize;
    enum FortDPerfTable_Constants_enum { TABLE_INIT_SIZE = 256 };

    PerfMetrics* GetProcPerfInfo    (LineStatCell* procStatCell, int staticID);
    PerfMetrics* GetLoopPerfInfo    (LineStatCell* loopStatCell, int staticID);
    PerfMetrics* GetMesgPerfInfo    (LineStatCell* mesgStatCell, int staticID);
};

inline PerfMetrics* FortDPerfTable::GetPerfInfo(int staticID)
{
    return (staticID >= perfInfoTableSize)? (PerfMetrics*) NULL
					  : perfInfoTable[staticID];
}


//--------------------------------------------------------------------------

#endif /* FortDPerfTable_h */
