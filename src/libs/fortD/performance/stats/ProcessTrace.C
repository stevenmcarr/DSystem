/* $Id: ProcessTrace.C,v 1.1 1997/03/11 14:29:13 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * This file is part of the Pablo Performance Analysis Environment
 *
 *                                           TM
 * The Pablo Performance Analysis Environment   software is *not* in
 * the public domain.  However, it is freely available without fee for
 * education, research, and non-profit purposes.  By obtaining copies
 * of this and other files that comprise the Pablo Performance Analysis
 * Environment, you, the Licensee, agree to abide by the following
 * conditions and understandings with respect to the copyrighted software:
 *
 * 1.  The software is copyrighted in the name of the Board of Trustees
 *     of the University of Illinois (UI), and ownership of the software
 *     remains with the UI.
 *
 * 2.  Permission to use, copy, and modify this software and its documentation
 *     for education, research, and non-profit purposes is hereby granted
 *     to Licensee, provided that the copyright notice, the original author's
 *     names and unit identification, and this permission notice appear on
 *     all such copies, and that no charge be made for such copies.  Any
 *     entity desiring permission to incorporate this software into commercial
 *     products should contact:
 *
 *          Professor Daniel A. Reed                 reed@cs.uiuc.edu
 *          University of Illinois
 *          Department of Computer Science
 *          2413 Digital Computer Laboratory
 *          1304 West Springfield Avenue
 *          Urbana, Illinois  61801
 *          USA
 *
 * 3.  Licensee may not use the name, logo, or any other symbol of the UI
 *     nor the names of any of its employees nor any adaptation thereof in
 *     advertizing or publicity pertaining to the software without specific
 *     prior written approval of the UI.
 *
 * 4.  THE UI MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THE
 *     SOFTWARE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS
 *     OR IMPLIED WARRANTY.
 *
 * 5.  The UI shall not be liable for any damages suffered by Licensee from
 *     the use of this software.
 *
 * 6.  The software was developed under agreements between the UI and the
 *     Federal Government which entitle the Government to certain rights.
 *
 **************************************************************************
 *
 * Developed by: The TAPESTRY Parallel Computing Laboratory
 *               University of Illinois at Urbana-Champaign
 *               Department of Computer Science
 *               1304 W. Springfield Avenue
 *               Urbana, IL     61801
 *
 * Copyright (c) 1987-1994
 * The University of Illinois Board of Trustees.
 *      All Rights Reserved.
 *
 * Author: Jhy-Chun Wang (jcwang@cs.uiuc.edu)
 *
 * Project Manager and Principal Investigator:
 *      Daniel A. Reed (reed@cs.uiuc.edu)
 *
 * Funded by: National Science Foundation grants NSF CCR86-57696,
 * NSF CCR87-06653 and NSF CDA87-22836 (Tapestry), NASA ICLASS Contract
 * No. NAG-1-613, DARPA Contract No. DABT63-91-K-0004, by a grant
 * from the Digital Equipment Corporation External Research Program,
 * and by a collaborative research agreement with the Intel Supercomputer
 * Systems Division.
 *
 *
 * Pablo + Fortran D project
 * Date: 04/14/94
 */

/*
 * ProcessTrace.C:
 *    copy from FDCombine4.C, use new classes.
 *
 *    take static/symbolic/dynamic records as input, this program focuses on
 *    data locality of distributed arrays, especially those used in
 *    communication, we want to observe the fraction of off-processor
 *    reference (and local-reference as well, if possible).
 *
 *    V.S.Adve, 8/12/94.
 *    most of the computation part split off from FDCombine6.C to
 *    reorganize as a library that can be linked in either to FDCombine6.C
 *    or the D system.
 */

#include <libs/fortD/performance/stats/FDCombine6.h>
#include <libs/fortD/performance/stats/FDCombine6.i>
#include <libs/fortD/performance/stats/FDCombine6b.h>

// V.S.Adve, 8/20/94 : Added to save the calling sequence.
// Should be a function provided by this code.
extern void AddToCallSequence(RecordDossier& origDossier);

static void NoStaticRecordError();

/* read/process records and prepare output */
int ProcessTraces(char* inFileName, char* outFileName,
		  FileType outputFileType,
		  int _ggranular, int _gpartition)
{
   Ggranular = _ggranular;		// Copy arguments to static globals.
   Gpartition = _gpartition;
   
   tableManager(1);			// Moved here from main()
   
   /****************************************************************
    *    Open the input file
    ****************************************************************/

   InputFileStreamPipe   *In;
   PipeReader            *InPipe;

   In = new InputFileStreamPipe( inFileName );
   if ( In->successfulOpen() ) {
       InPipe = In->createPipeReader();
       if ( InPipe == NULL ) {
	   cerr << "ERROR: Couldn't attach pipe to input file\n";
	   delete In;
	   exit(1);
       }
   } else {
       // cerr << "ERROR: Problem with input file\n\n";
       // delete In;
       // exit(1);
       return 0; // return false to indicate failure.
   }


   /****************************************************************
    *	Open the output file if specified
    ****************************************************************/
   OutputFileStreamPipe *Out;
   PipeWriter           *OutPipe;

   if (outFileName != (char*) NULL) {
       if (outputFileType == UNDEFINED_FILE_TYPE ) {
	   cerr << "ERROR: output file type not specified.\n\n";
	   exit(1);
       }
      Out = new OutputFileStreamPipe(outFileName);
      if ( Out->successfulOpen() ) {
	  if ( outputFileType == ASCII ) { 
	      OutPipe = new AsciiPipeWriter( Out );
	  } else if ( outputFileType == BINARY ) { 
	      OutPipe = new BinaryPipeWriter( Out );
	  } else {
	      OutPipe = new ConversionPipeWriter( Out );
	  }
	  if (OutPipe == (PipeWriter*) NULL) {
	      cerr << "ERROR: Unable to open output PipeWriter.\n\n";
	      delete Out;
	      exit(1);
	  }
      } else {
	  cerr << "ERROR: Unable to open OutputFileStreamPipe.\n\n";
	  delete Out;
	  exit(1);
      }
   }
   else
       OutPipe = (PipeWriter*) NULL;
       

   /****************************************************************
    *    Process the packets in the files.  Read from the input 
    *    pipe and update the profile information.
    ****************************************************************/
   RecordDictionary   RecDict;

   int pktCount       = 0;
   int attrCount      = 0; 
   int descrCount     = 0;
   int dataCount      = 0;
   int cmdCount       = 0;
   int duplicateCount = 0;

   int FDStatCnt      = 0;
   int FDSSymCnt      = 0;
   int FDDSymCnt      = 0;
   int FDProcCnt      = 0;
   int FDLoopCnt      = 0;
   int FDMesgCnt      = 0;

   StructureDescriptor   *origSDp;
   StructureDescriptor   *combSDp;
   RecordDossier         *copyDossier;
   CString               recName;
   CString               combName;
   int                   index;
   int                   symValue;
   int                   theNode;
   int                   i, j;

   RecordID    recordID;
   char*       nameString;

   cout << "Currently processing packet... \n";

   PacketHeader PH = InPipe->getPacketHeader();

   while( PH.type != PIPE_EMPTY ) {
      if ( ( ++pktCount % 1000 ) == 0 ) {
	 cout << pktCount << "... ";
	 cout.flush();
      }

      switch( PH.type ) {
       case PKT_ATTRIBUTE:
	 attrCount++;
	 break;
	 
       case PKT_COMMAND:
	 cmdCount++;
	 break;

       case PKT_DESCRIPTOR:
	 descrCount++;
	 origSDp = new StructureDescriptor();
	 InPipe->getDescriptor( *origSDp );

	 if ( RecDict.insert( PH.tag, *origSDp ) == SUCCESS_ ) {
	    recName = origSDp->getName(); 
	    nameString = (char *) recName.getValue();
	    recordID.setID( nameString );
	    {
	       RecordDossier& inDossier = RecDict.fetch( PH.tag );
	       GfieldID.setIndex( nameString, inDossier, recordID );
	    }

	    switch ( recordID.getFamily() ) {
	     case FDSTAT:  /* for the time being, I don't do anything here */ 
	     case FDSSYM:
	     case FDDSYM:
	     case FDSYSF:
	       break;

	     case FDPROC:
	     case FDLOOP: 
	     case FDMESG:   
	       break;

	     default:
	       if (OutPipe != (PipeWriter*) NULL)
		   OutPipe->putDescriptor( *origSDp, PH.tag );
	       break;
	    }
	 }
	 else {
	    duplicateCount++;
	 }

	 delete origSDp;
	 break;

       case PKT_DATA:
	 {
	    dataCount++;
	    RecordDossier& origDossier = RecDict.fetch( PH.tag );
	    InPipe->getData( origDossier );
	    recName = origDossier.getName(); 
	    nameString = (char *) recName.getValue();
            recordID.setID( nameString );

	    switch ( recordID.getFamily() ) {
	     case FDSTAT: /* static record family */
	       FDStatCnt++;
	       /* store static record in staticTable[] */
	       copyDossier = new RecordDossier( origDossier );
	       index = origDossier.getValue( GfieldID.getST_ID() );

	       if ( index >= currentTableSize ) 
		    /* current table size is not big enough, double it */
		    tableManager( index );

	       if ( staticTable[index] != NULL ) {
		  cerr << "WARNING: duplicated static ID\n";
//		  exit(-1);
	       } else {
		  staticTable[index] = copyDossier; 
	       }
	       break;

	     case FDSSYM: /* compiler symbolic record family */
	       FDSSymCnt++;
	       index = origDossier.getValue( GfieldID.getST_ID() );
	       symValue = origDossier.getValue( GfieldID.getST_sym_value() );
	       if ( index >= currentSymTableSize )
		   /* current table size is not big enough, double it */
		   symTableManager( index );
	       
	       for ( i = 0; i < NNODES; i++ ) {
		  symbolTable[index].value[i] = symValue;
	       }
	       break;

	     case FDDSYM: /* runtime symbolic record family */
	       FDDSymCnt++;
	       index = origDossier.getValue( GfieldID.getDY_ID() );
	       symValue = origDossier.getValue( GfieldID.getDY_sym_value() );
	       theNode = origDossier.getValue( GfieldID.getDY_sym_node() );
               if ( index >= currentSymTableSize )
                   /* current table size is not big enough, double it */
                   symTableManager( index );

	       symbolTable[index].value[theNode] = symValue;
	       break;

	     case FDSYSF: /* runtime system configuration */
	       GnumNodes = origDossier.getValue( GfieldID.getDY_sys_nnode() );
	       break;

	     case FDPROC: /* dynamic proc record family */
	       FDProcCnt++;
	       /* only use exit record */
               if ( ( recordID.getRecord() == DY_TRACEE ) || 
		   ( recordID.getRecord() == DY_COUNTE ) ) {
		  collectLineStatData( origDossier, recordID );
	       }

	       // V.S.Adve, 8/20/94 : Need to find the main program:
	       AddToCallSequence(origDossier);

	       /* prepare stack to calculate in/exclusive mesg time */
	       theNode = origDossier.getValue( GfieldID.getDY_node() );
	       int  staticID;
	       staticID = origDossier.getValue( GfieldID.getDY_staticID() );
	       if ( ( recordID.getRecord() == DY_TRACEB ) ||
                   ( recordID.getRecord() == DY_COUNTB ) ) {
		   GprocStack.push( theNode, staticID );
//		   pushProcStack( GprocStack, theNode, staticID );

	       } else if ( ( recordID.getRecord() == DY_TRACEE ) ||
			  ( recordID.getRecord() == DY_COUNTE ) ) {
		   GprocStack.pop( theNode, staticID );
//		   popProcStack( GprocStack, theNode, staticID );
	       }
	       break;

	     case FDLOOP: /* dynamic loop record family */
	       FDLoopCnt++;
               if ( ( recordID.getRecord() == DY_TRACEE ) || 
		   ( recordID.getRecord() == DY_COUNTE ) ) {
		  collectLineStatData( origDossier, recordID );
	       }
	       break;

	     case FDMESG: /* dynamic mesg record family */
	       FDMesgCnt++;
	       /* collect runtime statistic data based on line number */
	       switch ( recordID.getRecord() ) {
//		case DY_CSENDB:   /* include "begin" records here will */
		case DY_CSENDE:
//		case DY_ISENDB:   /* double the results, that is not right */
		case DY_ISENDE:
		case DY_CRECVE:
		case DY_IRECVE:
		case DY_GLOPE:
		case DY_WAITE:
		  collectLineStatData( origDossier, recordID );
		  break;
	       }

	       /* off-processor reference recorded in csend or isend, those 
		  are the places we look for data locality information */
	       switch ( recordID.getRecord() ) {
                case DY_CSENDB:
		case DY_ISENDB:
		case DY_GLOPB:   /* global oper has no destNode info */
// static records for this purpose are not available yet, 07/13/94
		  
		  // DISABLED BY VSA FOR TESTING:
		  // updateDataLocality( origDossier, recordID );
		  
		  break;
	       }
	       break;
	       
	     default: /* other internal records */
	       if (OutPipe != (PipeWriter*) NULL)
		   OutPipe->putData( origDossier );
	       break;
	    }
         }
	 break;
      }

      PH = InPipe->getPacketHeader();
   }
   
   printf( "\nThere are %d packets in the file %s.\n", 
	  attrCount+descrCount+dataCount+cmdCount, inFileName);
   
   printf( "%d Descriptor; %d Data; %d Attribute; %d Command.\n",
	  descrCount, dataCount, attrCount, cmdCount );

   if ( duplicateCount != 0 ) {
      printf( "%d of the Descriptor packets had duplicate tags.\n", 
	     duplicateCount );
   }

   // printf( "FDStat=%d, FDSSym=%d, FDDSym=%d\n", FDStatCnt, FDSSymCnt, 
   // 	  FDDSymCnt );
   // printf( "FDProc=%d, FDLoop=%d, FDMesg=%d\n", FDProcCnt, FDLoopCnt,
   //     FDMesgCnt );
#ifdef FD_HDF
   /* use NCSA HDF & XDS to disaply array usage */
//   DFSDsetNT( DFNT_INT32 );   /* strange, this guy fail when space */
                                /* requirement is huge */
#endif FD_HDF
   outputLineStatData();
   
   // outputDataLocality( "---Data Locality Based on Array Declaration---" );
   
   delete InPipe;
   delete In;
   if (OutPipe != (PipeWriter*) NULL) {
       delete OutPipe;
       delete Out;
   }
   return 1;				// return true
}   /* end of processFiles() */


/* allocate/initialize and deallocate global tables */
void
tableManager( int index )
{
   int  i;
   int  sizeScale;
   int  newSize;

   static Boolean_  first = TRUE_;
   Boolean_  spaceOK;   /* Defines.h */

   symbolTableStr*    newSymbolTable;
   RecordDossier**    newStaticTable;
   LineStatCell*      newLineStatTable;
   DataLocalityCell*  newDataLocTable;

   if ( first ) {   /* the very first tables allocation */
      first = FALSE_;
      currentTableSize = DEFAULT_TABLE_SIZE;
      currentSymTableSize = DEFAULT_TABLE_SIZE;

      /* tables for static records and symbolic records */
      symbolTable = new symbolTableStr[currentTableSize];
      staticTable = new RecordDossier*[currentTableSize];

      /* tables for lineStatistic and dataLocality */
      lineStatTable = new LineStatCell[currentTableSize];
      dataLocTable = new DataLocalityCell[currentTableSize];

      if ( ( symbolTable == 0 ) || ( staticTable == 0 ) ||
	  ( lineStatTable == 0 ) || ( dataLocTable == 0 ) ) {
	 /* no enough space for init space, exit(-1) */
	 cerr << "ERROR: cannot allocate enough space for (init) tables\n";
	 exit(-1);
      }

      /* initialize staticTable to NULL */
      for ( i = 0; i < currentTableSize; i++ ) {
	 staticTable[i] = NULL;
      }

      /* headers of FileCell listing, to keep output in sorted order */
      lineHeader = NULL;
      dataHeader = NULL;

   } else {   /* enlarge table size */
       // cerr << form( "WARNING: current table size of %d is not enough\n",
       // 	   currentTableSize );

      /* decide how much space we need */
      sizeScale = index / currentTableSize;
      newSize = currentTableSize * ( sizeScale + 1 );

      sizeScale = 1;
      spaceOK = FALSE_;
      do {
	 newStaticTable = new RecordDossier*[newSize];
	 newLineStatTable = new LineStatCell[newSize];
	 newDataLocTable = new DataLocalityCell[newSize];

	 if ( ( newStaticTable == 0 ) ||
	     ( newLineStatTable == 0 ) || (newDataLocTable == 0 ) ) {
	    /* no enough space for newSize, shrink the space and try again */
	    /* first, free allocated space */
	    if ( !newStaticTable )
		 delete[] newStaticTable;
	    if ( !newLineStatTable )
		 delete[] newLineStatTable;
	    if ( !newDataLocTable )
		 delete[] newDataLocTable;

	    /* reduce newSize and try again */
	    sizeScale *= 2;
	    newSize -= ( currentTableSize / sizeScale );
	    if ( newSize <= index ) {
	       cerr << "ERROR: cannot allocate enough space for tables\n";
	       exit(-1);
	    }

	 } else {
	    spaceOK = TRUE_;
	 }
      } while ( ( !spaceOK ) && ( sizeScale < currentTableSize ) );

      if ( !spaceOK ) {
	 cerr << "ERROR: cannot allocate enough space for tables\n";
	 exit(-1);
	 
      } else {
	 // cout << form( "new table space is %d\n", newSize );

	 /* copy existed values to new tables */
	 for ( i = 0; i < currentTableSize; i++ ) {
	    newStaticTable[i] = staticTable[i];   /* array of pointers */
	    newLineStatTable[i] = lineStatTable[i];
	    newDataLocTable[i] = dataLocTable[i];
	 }

	 for ( i = currentTableSize; i < newSize; i++ ) {
	    newStaticTable[i] = NULL;
	 }

	 /* redirect headers and deallocate old space */
	 currentTableSize = newSize;

	 /* free space used by old tables */
	 RecordDossier** oldStaticTable = staticTable;
	 staticTable = newStaticTable;
	 delete[] oldStaticTable;

	 LineStatCell* oldLineStatTable = lineStatTable;
	 lineStatTable = newLineStatTable;
	 delete[] oldLineStatTable;

	 DataLocalityCell* oldDataLocTable = dataLocTable;
	 dataLocTable = newDataLocTable;
	 delete[] oldDataLocTable;
      }
   }

}   /* end of tableManager() */



/* allocate/initialize and deallocate global sym table */
void
symTableManager( int index )
{
   int  i;
   int  sizeScale;
   int  newSize;

   static Boolean_  first = TRUE_;
   Boolean_  spaceOK;   /* Defines.h */

   symbolTableStr*    newSymbolTable;

   cout << "increasing sym table size... ";

   /* decide how much space we need */
   sizeScale = index / currentSymTableSize;
   newSize = currentSymTableSize * ( sizeScale + 1 );

   sizeScale = 1;
   spaceOK = FALSE_;
   do {
       newSymbolTable = new symbolTableStr[newSize];

       if ( newSymbolTable == NULL ) {
           /* no enough space for newSize, shrink the space and try again */
           /* reduce newSize and try again */
           sizeScale *= 2;
           newSize -= ( currentSymTableSize / sizeScale );
           if ( newSize <= index ) {
               cerr << "ERROR: cannot allocate enough space for sym table\n";
               exit(-1);
           }

       } else {
           spaceOK = TRUE_;
       }
   } while ( ( !spaceOK ) && ( sizeScale < currentSymTableSize ) );

   if ( !spaceOK ) {
       cerr << "ERROR: cannot allocate enough space for sym table\n";
       exit(-1);

   } else {
       cout << form( "new table space is %d\n", newSize );

       /* copy existed values to new tables */
       for ( i = 0; i < currentSymTableSize; i++ ) {
           newSymbolTable[i] = symbolTable[i];
       }

       /* redirect headers and deallocate old space */
       currentSymTableSize = newSize;

       /* free space used by old tables */
       symbolTableStr* oldSymbolTable = symbolTable;
       symbolTable = newSymbolTable;
       delete[] oldSymbolTable;
   }
}   /* end of symTableManager() */



/* collect statistic data based on source code's line number */
void
collectLineStatData( const RecordDossier& origDossier, 
		    const RecordID& recordID )
{
   char     fileName[NAME_SIZE];
   char     procName[NAME_SIZE];
   int      lineCount;
   int      lineNums[LINE_SIZE];

   int      myNode;
   int      msgSize;
   int      count;
   int      index;
   int      symIndex;
   TimeStr  second;

   RecordDossier* staticDossier;

   /* get staticID */
   index = origDossier.getValue( GfieldID.getDY_staticID() );
   /* get corresponding static record */
   staticDossier = staticTable[index];
   if ( staticDossier == (RecordDossier *) NULL ) {
       NoStaticRecordError();
   } else {   /* static record is available */
      myNode = origDossier.getValue( GfieldID.getDY_node() );

      retrieveTime( origDossier, recordID, second, count, myNode );

      retrieveLocation( staticDossier, recordID, fileName, procName, 
		      lineCount, lineNums );

//      if ( ( recordID.getRecord() == DY_CSENDB ) || 
//	  ( recordID.getRecord() == DY_ISENDB ) ) {
      if ( recordID.getFamily() == FDMESG ) {
	  if ( ( recordID.getRecord() == DY_CSENDE ) || 
	      ( recordID.getRecord() == DY_ISENDE ) ||
	      ( recordID.getRecord() == DY_GLOPE ) ) {
	      symIndex = staticDossier->getValue( GfieldID.getST_send_size() );
	      msgSize = symbolTable[symIndex].value[myNode];
	      
	  } else {   /* go to corresponding "send" record to get size */
	      int sendID = staticDossier->getValue( GfieldID.getST_wait_sendID() );
	      /* send record, its existence has been checked in lineInfo */
	      /* no need to repeat the check here */
	      RecordDossier* sendDossier = staticTable[sendID];
	      symIndex = sendDossier->getValue( GfieldID.getST_send_size() );
	      msgSize = symbolTable[symIndex].value[myNode];
	  }
      } else {   /* other families have no size info */
	 msgSize = 0;
      }

      lineStatTable[index].addBaseInfo( fileName, procName, lineCount, 
				       lineNums, recordID );

      lineStatTable[index].addStatistic( myNode, second, count, msgSize );

      /* update lineHeader listing */
      lineHeader = updateOutList( lineHeader, fileName, procName, lineNums[0],
				 index );
   }
}   /* end of collectLineStatData() */



/* retrieve time data (in seconds) from dynamic records */
void
retrieveTime( const RecordDossier& origDossier, const RecordID& recordID, 
	     TimeStr& second, int& count, int myNode )
{
   FamilyType  family;
   RecordType  record;

   family = recordID.getFamily();
   record = recordID.getRecord();

   /* for the time being, we don't consider count records */
   count = 1;

   if ( family == FDPROC ) {
      second.procInclude = origDossier.getValue( GfieldID.getDY_proc_inSecond() );
      second.procExclude = origDossier.getValue( GfieldID.getDY_proc_exSecond() );
      second.mesgInclude = 0;
      second.mesgExclude = 0;

   } else {   /* FDMESG & FDLOOP record types */
      second.procInclude = origDossier.getValue( GfieldID.getDY_second() );
      second.procExclude = 0;
      second.mesgInclude = 0;
      second.mesgExclude = 0;

      /* calculate in/exclusive mesg time inside procedure */
      ProcNode*  currentProc;
      currentProc = GprocStack.getTop( myNode );
      Boolean_  topNode;
      topNode = TRUE_;
      int  procID;
      TimeStr  mesgSecond;
      mesgSecond.procInclude = 0;   /* we don't use these two fields here */
      mesgSecond.procExclude = 0;
      while ( currentProc != NULL ) {
	  procID = currentProc->getID();

	  if ( topNode ) {
	      topNode = FALSE_;
	      mesgSecond.mesgInclude = second.procInclude;
	      mesgSecond.mesgExclude = second.procInclude;

	  } else {
	      mesgSecond.mesgInclude = second.procInclude;
              mesgSecond.mesgExclude = 0;
	  }

	  lineStatTable[procID].addTime( myNode, mesgSecond );
	  currentProc = currentProc->getNext();
      }
   }
}   /* end of retrieveTime() */



/* retrieve location info from static records */
void
retrieveLocation( const RecordDossier* staticDossier, const RecordID& recordID,
		 char* fileName, char* procName, int& lineCount, 
		 int*lineNums )
{
   int  index;
   FamilyType  family;
   RecordType  record;
   Value*  nameP;
   RecordDossier*  recordDossier;

   family = recordID.getFamily();
   record = recordID.getRecord();

   switch ( family ) {
    case FDPROC:
      /* until "proc call" record is available, this is the one for proc */
      /* when it becomes available, there is no need to separate "proc call" */
      /* from the others (in term of name retrieving) */
      retrieveName( staticDossier, fileName, procName );

      lineCount = 1;
      lineNums[0] = staticDossier->getValue( GfieldID.getST_line() );
      break;

    default:
      index = staticDossier->getValue( GfieldID.getST_procID() );
      /* get corresponding static record */
      recordDossier = staticTable[index];
      if ( recordDossier == (RecordDossier *) NULL ) {
	NoStaticRecordError();
      } else {   /* static record is available */
         retrieveName( recordDossier, fileName, procName );
      }

      /* retrieve line number(s) */
      switch ( record ) {
       case DY_CSENDB:
       case DY_CSENDE:
       case DY_ISENDB:
       case DY_ISENDE:
       case DY_GLOPB:
       case DY_GLOPE:
	 retrieveMesgLine( staticDossier, lineCount, lineNums );
	 break;

       case DY_CRECVB:
       case DY_CRECVE:
       case DY_IRECVB:
       case DY_IRECVE:
       case DY_WAITB:
       case DY_WAITE:
	 /* go through corresponding "mesg send" record to get line info */
	 index = staticDossier->getValue( GfieldID.getST_wait_sendID() );
         recordDossier = staticTable[index];
         if ( recordDossier == (RecordDossier *) NULL ) {
	    NoStaticRecordError();
         } else {
            retrieveMesgLine( recordDossier, lineCount, lineNums );
         }
	 break;

       default:
	 /* other records that have only one line number */
	 lineCount = 1;
	 lineNums[0] = staticDossier->getValue( GfieldID.getST_line() );
	 break;
      }
   }
}   /* end of retrieveLocation() */



/* retrieve array location info from static records */
void
retrieveArrayLocation( const RecordDossier* arrayDossier, 
		      char* fileName, char* procName, int& lineCount, 
		      int*lineNums )
{
    int  index;
    RecordDossier* recordDossier;

    index = arrayDossier->getValue( GfieldID.getST_procID() );
    /* get corresponding static record */
    recordDossier = staticTable[index];
    if ( recordDossier == (RecordDossier *) NULL ) {
	NoStaticRecordError();
    } else {   /* static record is available */
	
	/* name info are in corresponding proc record */
	retrieveName( recordDossier, fileName, procName );

	lineCount = 1;
	lineNums[0] = arrayDossier->getValue( GfieldID.getST_line() );
    }
}   /* end of retrieveArrayLocation() */



void
retrieveName( const RecordDossier* recordDossier, char* fileName, 
	     char* procName )
{
    CString  nameP;

    nameP = recordDossier->getCString( GfieldID.getST_proc_fileName() );
    strcpy( fileName, (char *) nameP.getValue() );   

    nameP = recordDossier->getCString( GfieldID.getST_proc_procName() );
    strcpy( procName, (char *) nameP.getValue() ); 

};   /* end of retrieveName() */



void
retrieveMesgLine( const RecordDossier* recordDossier, int& lineCount,
		 int* lineNums )
{
   int  i;
   int  numID;
   int  dependID;
   int  lineNum;

   Array*  arrayP;
   RecordDossier*  dependDossier;

   lineCount = 0;
   /* travel through every "depend" record to get line info */
   arrayP = recordDossier->getArrayP( GfieldID.getST_send_dependID() );
   numID = arrayP->getCellCount();
   for ( i = 0; i < numID; i++ ) {
      dependID = arrayP->getCellValue(i);
      dependDossier = staticTable[dependID];
      if ( dependDossier == (RecordDossier *) NULL ) {
	 NoStaticRecordError();
      } else {
	 /* source line */
	 lineNum = dependDossier->getValue( GfieldID.getST_depend_srcLine() );
	 if ( lineNum != -1 ) {
	    lineNums[lineCount] = lineNum;
	    lineCount++;
	 }
	 /* sink line */
	 lineNum = dependDossier->getValue( GfieldID.getST_depend_sinkLine() );
         if ( lineNum != -1 ) {
            lineNums[lineCount] = lineNum;
            lineCount++;
         }
      }
   }
}   /* end of retrieveMesgLine() */



/* output statistic data based on line number for iPablo browser */
void
outputLineStatData( )
{
   int             staticID;
   const char*            fileName;
   const char*            procName;

   char            outputName[80];
   FILE*           ofd;

   FileCell*  currentFile;
   ProcCell*  currentProc;
   LineCell*  currentLine;

   outputName[0] = '\0';
   strcat( outputName, progName );
   strcat( outputName, ".line.da" );
   ofd = fopen( outputName, "w" );
   // printf( "\n Line statistic data is in file: %s\n", outputName);

   fprintf( ofd, "\n nNodes = %d\n\n", GnumNodes );   /* number of nodes */
   fprintf( ofd, " output format: (node0 node1 ...) " );
   fprintf( ofd, " max  min  mean  std-dev\n" );

   currentFile = lineHeader;
   while ( currentFile != (FileCell *) NULL ) {
      fileName = currentFile->getName();
      fprintf( ofd, "\n\n  * fileName = %s\n", fileName );
      currentProc = currentFile->getDown();

      while ( currentProc != (ProcCell *) NULL ) {
	 procName = currentProc->getName();
	 fprintf( ofd, "\n ** procName = %s\n", procName );
	 currentLine = currentProc->getDown();

	 while ( currentLine != (LineCell *) NULL ) {
	    staticID = currentLine->getStaticID();
	    /* line number(s) */
fprintf( ofd, "\n*** static ID = %d", staticID );   /* verify purpose */
	    lineStatTable[staticID].fprintLineNums( ofd );

	    /* count */
	    lineStatTable[staticID].fprintCount( ofd, GnumNodes );

	    /* execution time */
	    lineStatTable[staticID].fprintTime( ofd, GnumNodes );

	    /* mesg size */
	    lineStatTable[staticID].fprintSize( ofd, GnumNodes );

	    currentLine = currentLine->getNext();
	 }
	 currentProc = currentProc->getNext();
      }
      currentFile = currentFile->getNext();
   }
   fclose( ofd );

}   /* end of outputLineStatData() */



/* right now we exclude "global oper" from this calculation */
/* only "csend" and "isend" are calculated */
void
updateDataLocality( const RecordDossier& origDossier, 
		   const RecordID& recordID )
{
   int  mesgID;
   int  arrayID;
   int  myNode;
   int  destNode;
   int  bcast;
   int  count;
   int  nDim;

   int  lineCount;
   int  lineNums[LINE_SIZE];
   char  fileName[NAME_SIZE];
   char  procName[NAME_SIZE];
   char  arrayName[NAME_SIZE];

   Array* arrayP;
   Value* procNameP;

   RecordDossier* staticDossier;
   RecordDossier* arrayDossier;

   UsageInfo*  usageInfo;

   usageInfo = new UsageInfo;

   /* get staticID */
   mesgID = origDossier.getValue( GfieldID.getDY_staticID() );
   /* get corresponding static record */
   staticDossier = staticTable[mesgID];
   if ( staticDossier == (RecordDossier *) NULL ) {
      NoStaticRecordError();
   } else {   /* static record is available */
      /* get src/dest nodes info from dynamic record */
      myNode = origDossier.getValue( GfieldID.getDY_node() );
      if ( recordID.getRecord() == DY_GLOPB )
	  destNode = 0;   /* actually, there is no destNode in global oper */
      else {
	  destNode = origDossier.getValue( GfieldID.getDY_dest() );
	  bcast = staticDossier->getValue( GfieldID.getST_send_opType() );
      }
      if ( destNode == -1 ) {   /* dynamic: broadcast */
	 if ( bcast == ST_BCAST )   /* static: it is a broadcast all right */
	      count = GnumNodes - 1;
	 else {
	    cerr << "ERROR: broadcast: communication type in static record doesn't match dynamic record\n";
	    exit(-1);
	 }
      } else {   /* one-to-one communication */
	 count = 1;
      }

      arrayP = staticDossier->getArrayP( GfieldID.getST_send_arrayID() );
      int numArrays = arrayP->getCellCount();
      int boundIndex = 0;
      for ( int i = 0; i < numArrays; i++ ) {
	 arrayID = arrayP->getCellValue( i );
	 /* -1 means the send only involves scalar variable */
	 /* no distributed array is involved, no data locality */
	 if ( arrayID > -1 ) {
	    /* get corresponding static array record */
	    arrayDossier = staticTable[arrayID];
	    if ( arrayDossier == (RecordDossier *) NULL ) {
		NoStaticRecordError();
	    } else {   /* static array record is available */
		/* check whether it is a non-dist or dist array */
		Array*  alignP;
		alignP = arrayDossier->getArrayP( GfieldID.getST_array_alignID() );
		if ( alignP->getCellCount() > 0 ) {  
		    /* align info is available, a dist array */
		    if ( dataLocTable[arrayID].getLineNum() == INT_MIN ) { 
			/* first use of this entry, fill basic information */

			/* get array location info */
			retrieveArrayLocation( arrayDossier, fileName, procName, 
					      lineCount, lineNums );

			/* array name & dim */
			CString nameP = arrayDossier->getCString( GfieldID.getST_array_name() );
			strcpy( arrayName, (char *) nameP.getValue() );

			nDim = arrayDossier->getValue( GfieldID.getST_array_dim() );
			
			/* there should be only one lineNum for array declaration */
			dataLocTable[arrayID].addBaseInfo( fileName, procName,
							  arrayName, lineNums[0],
							  nDim );

			/* get array boundary & data locality cluster size & */
			/* distType info */
			getArrayCluster( arrayDossier, arrayID, 
					dataLocTable[arrayID] );

		    } else {   /* get array info from dataLocTable */
			strcpy( fileName, dataLocTable[arrayID].getFileName() );
			strcpy( procName, dataLocTable[arrayID].getProcName() );
			lineNums[0] = dataLocTable[arrayID].getLineNum();
		    }

		    /* get runtime usage boundary info */
		    usageInfo->addBaseInfo( arrayID, mesgID, myNode, count );
		    getBoundary( staticDossier, usageInfo, boundIndex );

		    /* part1: array data locality based on array declaration */
		    updateDataTableLoc( usageInfo );

		    /* update dataHeader listing */
		    dataHeader = updateOutList( dataHeader, fileName, procName, 
					       lineNums[0], arrayID );

		    /*part2: array data locality based on mesg statement */
		    /*       this info is stored in lineStatTable */
//		    updateLineTableLoc( usageInfo );
		}
	    }
	 }
      }
   }
   delete usageInfo;
}   /* end of updateDataLocality() */



/* get array boundary & data locality cluster information */
void
getArrayCluster( const RecordDossier* arrayDossier, int arrayID, 
		DataLocalityCell& theArray )
{
   int  i;

   Array*  lowerP;
   Array*  upperP;

   /* first, boundary */
   lowerP = arrayDossier->getArrayP( GfieldID.getST_array_locLB() );
   upperP = arrayDossier->getArrayP( GfieldID.getST_array_locUB() );
   for ( i = 0; i < theArray.getDim(); i++ ) {
      theArray.setLocalLB( i, lowerP->getCellValue( i ) );
      theArray.setLocalUB( i, upperP->getCellValue( i ) );
   }
   /* decide array data locality's cluster size */
   clusterSizeAll( theArray );

   /* retrieve distribution info from relevant static records */
   distributionInfo( arrayDossier, theArray );

}   /* end of getArrayCluster() */



/* retrieve distribution info from relevant static records */
void
distributionInfo( const RecordDossier* arrayDossier, 
		 DataLocalityCell& theArray )
{
   int i;
   int staticID;

   RecordDossier* staticDossier;
   Array* arrayP;

   /* align ID */
   arrayP = arrayDossier->getArrayP( GfieldID.getST_array_alignID() );
   staticID = arrayP->getCellValue( 0 );   /* when static records become more
                                              complicated, this may not be the
                                              case - there may have more than 
                                              one align ID */
   /* decomp ID */
   /* find this align static record */
   staticDossier = staticTable[staticID];
   if ( staticDossier == (RecordDossier *) NULL ) {
       NoStaticRecordError();
   } else {   /* static record is available */
      staticID = staticDossier->getValue( GfieldID.getST_align_decompID() );

      /* dist ID */
      /* find this decomp static record */
      staticDossier = staticTable[staticID];
      if ( staticDossier == (RecordDossier *) NULL ) {
	  NoStaticRecordError();
      } else {   /* static record is available */
	 arrayP = staticDossier->getArrayP( GfieldID.getST_decomp_distID() );
	 staticID = arrayP->getCellValue( 0 );   /* same problem as align */
	 
	 /* dist type */
	 /* find this dist static record */
	 staticDossier = staticTable[staticID];
	 if ( staticDossier == (RecordDossier *) NULL ) {
	     NoStaticRecordError();
	 } else {   /* static record is available */
	    arrayP = staticDossier->getArrayP( GfieldID.getST_dist_distType() );

	    for ( i = 0; i < theArray.getDim(); i++ ) {
	       theArray.setDistType( i, arrayP->getCellValue( i ) );
	    }
	 }
      }
   }

}   /* end of distributionInfo() */



/* get boundary info */
void
getBoundary( const RecordDossier* staticDossier, UsageInfo* usageInfo,
	    int& boundIndex )
{
   int i, j;
   int iStart, iEnd;
   int symIndex;
   int  arrayID;
   int  node;

   Array* lowerP;
   Array* upperP;
   Array* stepP;

   /* there may be more than one array in the boundary fields, so we need
      to keep tracking the right position to start next array */
   iStart = boundIndex;
   arrayID = usageInfo->getArrayID();
   boundIndex += dataLocTable[arrayID].getDim();
   iEnd = boundIndex;

   node = usageInfo->getNode();

   /* first, lower bound */
   lowerP = staticDossier->getArrayP( GfieldID.getST_send_useLB() );
   upperP = staticDossier->getArrayP( GfieldID.getST_send_useUB() );
   stepP = staticDossier->getArrayP( GfieldID.getST_send_useStep() );
   j = 0;
   for ( i = iStart; i < iEnd; i++ ) {
      symIndex = lowerP->getCellValue( i );
      usageInfo->setUseLB( j, symbolTable[symIndex].value[node] );

      symIndex = upperP->getCellValue( i );
      usageInfo->setUseUB( j, symbolTable[symIndex].value[node] );

      symIndex = stepP->getCellValue( i );
      usageInfo->setStep( j, symbolTable[symIndex].value[node] );

      j++;
   }
}   /* end of getBoundary() */



/* calculate cluster size while maintain the right relative size of 
   each dimension */
void
clusterSizeAll( DataLocalityCell& theArray )
{
   int   i;
   int   dimSize, thisSize;
   int   size;
   int  nDim;

   nDim = theArray.getDim();

   if ( Ggranular < 1 ) {   /* each cluster has a size of 1 (detail info) */
      for ( i = 0; i < nDim; i++ ) {
	 theArray.setDataSize( i, 1 );
      }

   } else {
       Ggranular = ( GnumNodes > Ggranular ) ? GnumNodes : Ggranular;
      if ( Gpartition == 0 ) {   /* each dim has approx. same partitions */
	 for ( i = 0; i < nDim; i++ ) {
	     thisSize = theArray.getLocalUB(i) - theArray.getLocalLB(i) + 1;
	     if ( ( thisSize % Ggranular ) == 0 ) {
		 size = thisSize / Ggranular;
	     } else {
		 size = 1 + ( thisSize / Ggranular );
	     }
	    theArray.setDataSize( i, size );
	 }

      } else {   /* each dim has same partition (cluster) size */
	 /* find out the smallest dimension (in turn of size) */
	 dimSize = INT_MAX;
	 for ( i = 0; i < nDim; i++ ) {
	    thisSize = theArray.getLocalUB(i) - theArray.getLocalLB(i) + 1;
	    if ( dimSize > thisSize )  dimSize = thisSize;
	}
	 if ( ( dimSize % Ggranular ) == 0 ) {
	     size = dimSize / Ggranular;
	 } else {
	     size = 1 + ( dimSize / Ggranular );
	 }
	 for ( i = 0; i < nDim; i++ ) {
	    theArray.setDataSize( i, size );
	 }
      }
   }

   /* calculate total number of (cluster-)entries in the array */
   size = 1;
   for ( i = 0; i < nDim; i++ ) {
       if ( ( ( theArray.getLocalUB(i) - theArray.getLocalLB(i) + 1 ) %
	     theArray.getDataSize(i) ) == 0 )
	   size *= ( theArray.getLocalUB(i) - theArray.getLocalLB(i) + 1 ) /
	       theArray.getDataSize(i);
       else
	   size *= ( 1 + ( theArray.getLocalUB(i) - theArray.getLocalLB(i) + 
			  1 ) / theArray.getDataSize(i) );
   }
   theArray.setEntryCount( size );

}   /* end of clusterSizeAll() */



/* convert array index to cluster index, [0..n-1] */
int
indexToCluster( int dim, int arrayIndex, const UsageInfo* usageInfo )
{
   int   clusterIndex;
   int  arrayID;

   arrayID = usageInfo->getArrayID();
   clusterIndex = ( arrayIndex - usageInfo->getUseLB( dim ) ) /
	dataLocTable[arrayID].getDataSize( dim );

   return clusterIndex;

}   /* end of indexToCluster() */



/* convert cluster index back to array index */
void
clusterToIndex( int dim, int clusterIndex, int& lowerIndex, int& upperIndex,
	       const DataLocalityCell& theArray )
{

   lowerIndex = clusterIndex * theArray.getDataSize(dim) +
	theArray.getLocalLB(dim) ;
   upperIndex = ( clusterIndex + 1 ) * theArray.getDataSize(dim) +
	theArray.getLocalLB(dim) - 1;

   if ( upperIndex > theArray.getLocalUB(dim) )
	upperIndex = theArray.getLocalUB(dim);

}   /* end of clusterToIndex() */



/* output data locality information from dataLocTable */
void 
outputDataLocality( const char* title )
{
   int*  entryIDs;
   int   count;
   int   printCount;
   int   i;
   int   staticID;
   int  nDim;
   int  entryCount;
   int  entryUsed;

   Boolean_  haveValue;

   const char*  fileName;
   const char*  procName;
   char   outputName[80];

   FileCell*  currentFile;
   ProcCell*  currentProc;
   LineCell*  currentLine;
   EntryCell*  currentEntry;

   HDFdataStr   HDFdata;
   FILE*        ofd;

   outputName[0] = '\0';
   strcat( outputName, progName );
   strcat( outputName, ".daLoc.da" );
   ofd = fopen( outputName, "w" );
   // printf( "\n Array data locality data is in file: %s\n", outputName );

   fprintf( ofd, "\n %s\n\n", title );
   if ( ( Ggranular > 0 ) && ( Gpartition == 0 ) ) {
       fprintf( ofd, " Using equal number of clusters on every dimension,\n" );
       fprintf( ofd, " number of clusters on every dimension <= %d\n",
               Ggranular );
   } else {
       fprintf( ofd, " Using equal cluster size on every dimension\n" );
   }
   if ( GprintCluster == TRUE_ ) {
       fprintf( ofd, " Index shown in cluster index format\n" );
   } else {
       fprintf( ofd, " Index shown in original array index format\n" );
   }

   currentFile = dataHeader;
   while ( currentFile != (FileCell *) NULL ) {
      fileName = currentFile->getName();
      fprintf( ofd, "\n\n  * fileName = %s\n", fileName );
      currentProc = currentFile->getDown();

      while ( currentProc != (ProcCell *) NULL ) {
         procName = currentProc->getName();
         fprintf( ofd, "\n ** procName = %s\n", procName );
         currentLine = currentProc->getDown();

         while ( currentLine != (LineCell *) NULL ) {
            staticID = currentLine->getStaticID();
            /* line number */
	    fprintf( ofd, "\n*** lineNum = %d\n", 
		    dataLocTable[staticID].getLineNum() );
	    fprintf( ofd, "\n    arrayName = %s, local boundary = ",
		    dataLocTable[staticID].getArrayName() );
	    for ( i = 0; i < dataLocTable[staticID].getDim(); i++ )
		 fprintf( ofd, "[%d:%d]", dataLocTable[staticID].getLocalLB(i),
			 dataLocTable[staticID].getLocalUB(i) );
	    if ( ( Ggranular < 1 ) || ( Gpartition == 1 ) ) {
		fprintf( ofd, ", cluster size = %d\n", 
			dataLocTable[staticID].getDataSize(0) );
	    } else {
		fprintf( ofd, "\n" );
	    }
#ifdef FD_HDF
	    allocHDFArray( HDFdata, dataLocTable[staticID] );
#endif FD_HDF
	    /* go through each node's list */
            for ( i = 0; i < GnumNodes; i++ ) {
	       haveValue = FALSE_;
	       entryCount = 0;   /* total remote-access count */
	       entryUsed = 0;    /* portion of remote-access */
	       currentEntry = dataLocTable[staticID].getDown( i );
	       if ( currentEntry != (EntryCell *) NULL ) {
		  fprintf( ofd, "\n    nodeID = %d\n", i );
		  haveValue = TRUE_;
		  printCount = 0;
	       }
	       while ( currentEntry != (EntryCell *) NULL ) {
                  haveValue = TRUE_;
		  entryUsed++;
		  printCount++;
		  count = currentEntry->getCount();
		  entryCount += count;
		  entryIDs = currentEntry->getID();
#ifdef FD_HDF
		  updateHDFArray( HDFdata, dataLocTable[staticID], entryIDs, 
				 i, count );
#endif FD_HDF
		  nDim= dataLocTable[staticID].getDim();
		  /* choose one (and only one) output format */
		  if ( GprintCluster == TRUE_ ) {
		      /* show in cluster index */
		      printIndex( entryIDs, nDim, haveValue, count, printCount,
				 ofd );
		  } else {
		      /* show in array index range */
		      printRange( entryIDs, nDim, haveValue, count, printCount,
				 dataLocTable[staticID], ofd );
		  }
		  currentEntry = currentEntry->getNext();
	       }

	       if ( haveValue == TRUE_ )
		    fprintf( ofd, "\n" );
	       if ( entryCount ) {
		   fprintf( ofd, "\n Total number of remote-access = %d\n",
			   entryCount );
		   fprintf( ofd, " Total number of (cluster-)entries being" );
		   fprintf( ofd, " remote-accessed = %d (%5.2f%%)\n", 
			   entryUsed, 
			   100.0 * ( (double) entryUsed / 
				    dataLocTable[staticID].getEntryCount() ) );
	       }
	    }
#ifdef FD_HDF
	    /* output HDF data. If data file already exists, data will be
	       appended to the end, be aware of this */
	    DFSDadddata( HDFdata.fileName, nDim,
			(long int *) HDFdata.dimSize,
			(char *) HDFdata.arrayP );

//	    fprintf( ofd, "\n HDF file = %s\n", HDFdata.fileName );

	    /* free allocated space */
            free( HDFdata.arrayP );
#endif FD_HDF
            currentLine = currentLine->getNext();
         }
         currentProc = currentProc->getNext();
      }
      currentFile = currentFile->getNext();
   }
   fclose( ofd );

}   /* end of outputDataLocality() */



void
printIndex( int* entryIDs, int nDim, Boolean_& haveValue, int count, 
	   int printCount, FILE* ofd )
{
   int   j;
   const int   printPerLine = 3;

   fprintf( ofd, " ( " );
   for ( j = 0; j < (nDim - 1); j++ ) {
      fprintf( ofd, "%2d, ", entryIDs[j] );
   }
   fprintf( ofd, "%2d ) = %4d ;", entryIDs[j], count );
   if ( ( printCount % printPerLine ) == 0 ) {
      fprintf( ofd, "\n" );
      haveValue = FALSE_;
   }
}   /* end of printIndex() */



void
printRange( int* entryIDs, int nDim, Boolean_& haveValue, int count,
	   int printCount, const DataLocalityCell& theArray, FILE* ofd )
{
   int   j;
   int   lowerB, upperB;
   const int   printPerLine = 2;

   fprintf( ofd, " ( " );
   for ( j = 0; j < (nDim - 1); j++ ) {
      clusterToIndex( j, entryIDs[j], lowerB, upperB, theArray );
      if ( lowerB == upperB )
	   fprintf( ofd, "%3d, ", lowerB );
      else
	   fprintf( ofd, "%3d..%3d, ", lowerB, upperB );
   }
   clusterToIndex( j, entryIDs[j], lowerB, upperB, theArray );
   if ( lowerB == upperB )
	fprintf( ofd, "%3d ) = %4d ;", lowerB, count );
   else
	fprintf( ofd, "%3d..%3d ) = %4d ;", lowerB, upperB, count );
   if ( ( printCount % printPerLine ) == 0 ) {
      fprintf( ofd, "\n" );
      haveValue = FALSE_;
   }
}   /* end of printRange() */


#ifdef FD_HDF
void
allocHDFArray( HDFdataStr& HDFdata, const DataLocalityCell& theArray )
{
   int   i, j;
   int   size;
   int   nDim;
   int   numEntry;
   int   markID[NDIMS];
   int   offSet[NDIMS];
   char  commandLine[80];

   HDFdata.fileName[0] = '\0';   /* set string to empty */
   strcat( HDFdata.fileName, progName );
   strcat( HDFdata.fileName, ".daLoc." );
   strcat( HDFdata.fileName, theArray.getArrayName() );
   strcat( HDFdata.fileName, ".hdf" );
   // printf( "\n HDF data is in file: %s\n", HDFdata.fileName );

   /* since HDF always appends data to a file, it is better to delete the
      old file first */
   commandLine[0] = '\0';
   strcat( commandLine, "rm -f " );
   strcat( commandLine, HDFdata.fileName );
   system( commandLine );

   numEntry = 1;
   nDim = theArray.getDim();
   for ( i = 0; i < nDim; i++ ) {
      size = theArray.getLocalUB(i) -
	   theArray.getLocalLB(i) + 1;
      if ( ( size % theArray.getDataSize(i) ) == 0 )
	   HDFdata.blockSize[i] = size / theArray.getDataSize(i);
      else
	   HDFdata.blockSize[i] = 1 + size / theArray.getDataSize(i);

      /* is this dim distributed? */
      if ( theArray.getDistType(i) != NOTDIST ) {
	 /* insert a dummy entry between each node's subarray */
	 HDFdata.dimSize[i] = ( HDFdata.blockSize[i] * GnumNodes + 
			       ( GnumNodes - 1 ) );
      } else {  /* not distributed */
	 HDFdata.dimSize[i] = HDFdata.blockSize[i];
      }
      numEntry *= HDFdata.dimSize[i];
   }

   for ( i = 0; i < nDim; i++ ) {
      size = 1;
      for ( j = i+1; j < nDim; j++ ) {
         size *= HDFdata.dimSize[j];
      }
      offSet[i] = size;
   }

   HDFdata.arrayP = (int *) malloc( numEntry * sizeof(int) );
   for ( i = 0; i < numEntry; i++ )
	HDFdata.arrayP[i] = 0;

   /* mark boundary entries */
   for ( i = 0; i < nDim; i++ ) {
      if ( theArray.getDistType(i) != NOTDIST ) {
	 for ( j = 1; j < GnumNodes; j++ ) {
	    markID[i] = j * HDFdata.blockSize[i] + ( j - 1 );
	    markBoundary( 0, i, nDim, markID, HDFdata, offSet );
	 }
      }
   }
}   /* allocHDFArray() */



void
markBoundary( int current, int theOne, int nDim, int* markID, 
	     HDFdataStr& HDFdata, int* offSet )
{
   int   i;
   int   index;

   if ( current < nDim ) {
      if ( current != theOne ) {
	 for ( i = 0; i < HDFdata.dimSize[current]; i++ ) {
	    markID[current] = i;
	    markBoundary( current+1, theOne, nDim, markID, HDFdata, offSet );
	 }

      } else {
	 markBoundary( current+1, theOne, nDim, markID, HDFdata, offSet );
      }

   } else {
      index = 0;
      for ( i = 0; i < nDim; i++ ) {
	 index += ( markID[i] * offSet[i] );
      }
      HDFdata.arrayP[index] = -1;   /* it is not easy to assign good value 
				       that can be distinguished in XDS */
   }
}   /* end of markBoundary() */



void
updateHDFArray( HDFdataStr& HDFdata, const DataLocalityCell& theArray,
	       int* entryIDs, int myID, int count )
{
   int   i, j;
   int   index;
   int   nDim;
   int   size;
   int   globalInx[NDIMS];

   index = 0;
   nDim = theArray.getDim();
   for ( i = 0; i < nDim; i++ ) {
      if ( theArray.getDistType(i) == NOTDIST ) {
	 globalInx[i] = entryIDs[i];
      } else {
	 globalInx[i] = myID * HDFdata.blockSize[i] + myID + entryIDs[i];
      }

      size = 1;
      for ( j = i+1; j < nDim; j++ ) {
	 size *= HDFdata.dimSize[j];
      }
      index += ( size * globalInx[i] );
   }

   HDFdata.arrayP[index] = count;

}   /* updateHDFArray() */
#endif FD_HDF


/* FileCell* lineHeader;
   FileCell* dataHeader;
   ...
   lineHeader = updateOutList( lineHeader, ... );
   dataHeader = updateOutList( dataHeader, ... );
*/
/* there is one outList for lineStatistic and another one for data Locality,
   both use the same structure and are maintained by this procedure 
*/
FileCell*
updateOutList( FileCell* theHeader, const char* fileName,
	      const char* procName, int lineNum, int staticID )
{
   FileCell* newOne;

   if ( theHeader == NULL ) {   /* brand new list */
      newOne = new FileCell( fileName, procName, lineNum, staticID, NULL,
			    NULL );
      return newOne;

   } else {
      newOne = theHeader->updateList( fileName, procName, lineNum, staticID );
      return newOne;   /* it may be the original header, actually */
   }

}   /* end of updateOutList() */



/* update array data locality info in LineStat table */
void
updateLineTableLoc( const UsageInfo* usageInfo )
{
   int  myID;
   int  arrayID;
   int  mesgID;
   int  count;
   int  dim;
   int  entryIDs[NDIMS];

   ArrayCell* arrayHeader;
   ArrayCell* newArrayHeader;
   ArrayCell* thisArray;

   EntryCell* entryHeader;
   EntryCell* newEntryHeader;

   myID = usageInfo->getNode();
   count = usageInfo->getCount();
   mesgID = usageInfo->getMesgID();
   arrayID = usageInfo->getArrayID();
   dim = dataLocTable[arrayID].getDim();

   arrayHeader = lineStatTable[mesgID].getDown( myID );
   if ( arrayHeader == NULL ) {   /* brand new list */
      arrayHeader = new ArrayCell( arrayID );
      lineStatTable[mesgID].linkDown( myID, arrayHeader );
      thisArray = arrayHeader;

   } else {
      newArrayHeader = arrayHeader->updateList( thisArray, arrayID );
      if ( newArrayHeader != arrayHeader ) {
	 arrayHeader = newArrayHeader;
	 lineStatTable[mesgID].linkDown( myID, arrayHeader );
      }
   }

   entryHeader = thisArray->getDown();
   recursiveLineTable( 0, entryIDs, myID, arrayID, count, dim, usageInfo,
                      entryHeader, thisArray );

}   /* end of updateLineTableLoc() */



/* recursively get EntryIDs and update lineStat table */
void
recursiveLineTable( int currentDim, int* entryIDs, const int& myID,
                    const int& arrayID, const int& count, const int& dim,
                    const UsageInfo* usageInfo, EntryCell*& entryHeader,
		   ArrayCell* thisArray )
{
   int  i;

   EntryCell*  newEntryHeader;
   EntryCell*  thisEntry;   /* we don't really use this field right now */

   if ( currentDim < dim-1 ) {
      for ( i = usageInfo->getUseLB( currentDim );
           i <= usageInfo->getUseUB( currentDim );
           i += usageInfo->getStep( currentDim ) ) {
         entryIDs[currentDim] = indexToCluster( currentDim, i, usageInfo );
         recursiveLineTable( currentDim+1, entryIDs, myID, arrayID, count,
                             dim, usageInfo, entryHeader, thisArray );
      }

   } else {   /* currentDim = dim - 1, have complete entryIDs now */

      for ( i = usageInfo->getUseLB( currentDim );
           i <= usageInfo->getUseUB( currentDim );
           i += usageInfo->getStep( currentDim ) ) {
         entryIDs[currentDim] = indexToCluster( currentDim, i, usageInfo );

	 if ( entryHeader == NULL ) {
	    entryHeader = new EntryCell( entryIDs, count );
	    thisArray->linkDown( entryHeader );
	    thisEntry = entryHeader;

	 } else {
	    newEntryHeader = entryHeader->updateList( thisEntry, entryIDs, 
						     count, dim );
	    if ( newEntryHeader != entryHeader ) {
	       entryHeader = newEntryHeader;
	       thisArray->linkDown( entryHeader );
	    }
	 }
      }
   }
}   /* end of recursiveLineTable() */



/* update array data locality info in DataLocality table */
void
updateDataTableLoc( const UsageInfo* usageInfo )
{
   int  myID;
   int  arrayID;
   int  count;
   int  dim;
   int  entryIDs[NDIMS];

   EntryCell*  entryHeader;

   myID = usageInfo->getNode();
   arrayID = usageInfo->getArrayID();
   count = usageInfo->getCount();
   dim = dataLocTable[arrayID].getDim();

   entryHeader = dataLocTable[arrayID].getDown( myID );
   recursiveDataTable( 0, entryIDs, myID, arrayID, count, dim, usageInfo,
		      entryHeader);

}   /* end of updateDataTableLoc() */



/* recursively get EntryIDs and update table */
/* the reason I use "const int& myID" and so on here is to reduce the */
/* number of copies that may be created when this call goes deep */
void
recursiveDataTable( int currentDim, int* entryIDs, const int& myID,
		    const int& arrayID, const int& count, const int& dim,
		    const UsageInfo* usageInfo, EntryCell*& entryHeader )
{
   int  i;

   EntryCell*  newEntryHeader;
   EntryCell*  thisEntry;   /* we don't really use this field right now */

   if ( currentDim < dim-1 ) {
      for ( i = usageInfo->getUseLB( currentDim ); 
	   i <= usageInfo->getUseUB( currentDim );
	   i += usageInfo->getStep( currentDim ) ) {
	 entryIDs[currentDim] = indexToCluster( currentDim, i, usageInfo );
	 recursiveDataTable( currentDim+1, entryIDs, myID, arrayID, count, 
			     dim, usageInfo, entryHeader );
      }

   } else {   /* currentDim = dim - 1, have complete entryIDs now */

      for ( i = usageInfo->getUseLB( currentDim );
           i <= usageInfo->getUseUB( currentDim );
           i += usageInfo->getStep( currentDim ) ) {
         entryIDs[currentDim] = indexToCluster( currentDim, i, usageInfo );

	 if ( entryHeader == NULL ) {
	    entryHeader = new EntryCell( entryIDs, count );
	    dataLocTable[arrayID].linkDown( myID, entryHeader );
	    thisEntry = entryHeader;

	 } else {
	    newEntryHeader = entryHeader->updateList( thisEntry, entryIDs, 
						     count, dim );
	    if ( newEntryHeader != entryHeader ) {
	       entryHeader = newEntryHeader;
	       dataLocTable[arrayID].linkDown( myID, entryHeader );
	    }
	 }
      }
   }
}   /* end of recursiveDataTable() */


void NoStaticRecordError()
{
    cerr << "ERROR:\tno static record was found corresponding to\n"
	 << "\tone of the trace records in the trace file.\n"
	 << "Exiting...\n";
    exit(-1);
}
