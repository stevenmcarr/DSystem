/* $Id: FDCombine6.h,v 1.3 2001/10/12 19:33:25 carr Exp $ */
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
 *
 * Date: 06/27/94
 */

/*
 * Subject: header file for new line statistic (and data locality) modules
 *
 * FDCombine6.h:
 *    structure (class) used by FDCombine6.C, new modules for line statistic
 *    data and array data locality.
 */

#ifndef  FDCombine6_h
#define  FDCombine6_h

#include <iostream.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <values.h>
#include <math.h>

// #include "InitializeStatic.C"

#include <PipeReader.h>
#include <AsciiPipeWriter.h>
#include <BinaryPipeWriter.h>
#include <ConversionPipeWriter.h>
#include <InputFileStreamPipe.h>
#include <OutputFileStreamPipe.h>

#include <Attributes.h>
#include <PacketHeader.h>
#include <RecordDictionary.h>
#include <RecordDictionaryPIterator.h>
#include <StructureDescriptor.h>
#include <StructureDescriptorIterator.h>

#ifdef FD_HDF
#include <hdf.h>
#endif FD_HDF

#ifndef  NAME_SIZE
#define  NAME_SIZE  40
#endif

#ifndef  LINE_SIZE
#define  LINE_SIZE  10
#endif

#ifndef NDIMS
#define NDIMS  4
#endif

#ifndef NNODES
#define NNODES 32
#endif

/* Added by VSA because these values are not found in the Rice
 * version of <values.h>
 */
#ifndef INT_MAX
#define INT_MAX  MAXINT         /* MAXINT is available in our <values.h> */
#endif
#ifndef INT_MIN
#define INT_MIN  -(MAXINT - 1)
#endif

#ifndef DEFAULT_TABLE_SIZE
#define DEFAULT_TABLE_SIZE  256   /* start with small size so I can verify */
#endif                            /* the correctness of tableManager() */

#define DUMMY_STATIC_ID	    -1

#define ST_BCAST  5   /* commType in static mesg send, 5 - broadcast */

enum FileType         { UNDEFINED_FILE_TYPE, ASCII, BINARY, CONVERT };

enum CompStatus  { EQ, LT, GT };

enum DistType    { NOTDIST, BLOCK, CYCLIC, BLOCKCYCLIC };

enum FamilyType  { FDSTAT, FDSSYM, FDDSYM, FDSYSF, FDPROC, FDLOOP,
		   FDMESG, FDNULL };

enum RecordType  { DY_CSENDB, DY_CSENDE, DY_ISENDB, DY_ISENDE, 
		   DY_CRECVB, DY_CRECVE, DY_IRECVB, DY_IRECVE, 
		   DY_GLOPB, DY_GLOPE, DY_WAITB, DY_WAITE,
                   DY_TRACEB, DY_TRACEE, DY_COUNTB, DY_COUNTE, 
		   ST_DECOMP, ST_DIST, ST_ALIGN, ST_ARRAY, ST_PROC, ST_DEPEND, 
		   ST_SEND, ST_RECV, ST_SENDW, ST_RECVW, RECNULL };


/* HDF info */
struct HDFdataStr {
   char        fileName[80];
   int         blockSize[NDIMS];
   long int    dimSize[NDIMS];
   int*        arrayP;
};


/* symbolic record table */
struct symbolTableStr {
   int value[NNODES];

   symbolTableStr() {
      for ( int i = 0; i < NNODES; i++ )
	   value[i] = INT_MIN;
   }

   ~symbolTableStr() { }

   symbolTableStr& operator=( const symbolTableStr& symbolTableEntry ) {
      int  i;
      for ( i = 0; i < NNODES; i++ )
	   value[i] = symbolTableEntry.value[i];

      return *this;
   }
};


struct TimeStr {          /* only proc trace will use all og them */
   double  procInclude;   /* loop trace will use the first one only */
   double  procExclude;
   double  mesgInclude;   /* mesg trace will use this one alone */
   double  mesgExclude;

   TimeStr() {
      procInclude = 0;
      procExclude = 0;
      mesgInclude = 0;
      mesgExclude = 0;
   }

   ~TimeStr() { }

   TimeStr& operator=( const TimeStr& timeStr ) {
      procInclude = timeStr.procInclude;
      procExclude = timeStr.procExclude;
      mesgInclude = timeStr.mesgInclude;
      mesgExclude = timeStr.mesgExclude;

      return *this;
   }

   TimeStr& operator=( double initValue ) {
      procInclude = initValue;
      procExclude = initValue;
      mesgInclude = initValue;
      mesgExclude = initValue;

      return *this;
   }

   TimeStr& operator+=( const TimeStr& timeStr ) {
      procInclude += timeStr.procInclude;
      procExclude += timeStr.procExclude;
      mesgInclude += timeStr.mesgInclude;
      mesgExclude += timeStr.mesgExclude;

      return *this;
   }
};


//**************** pre-declare all classes **********************************//

class LineStatCell;
class DataLocalityCell;
class UsageInfo;
class ArrayCell;
class EntryCell;
class FileCell;
class ProcCell;
class LineCell;
class RecordID;
class FieldID;


/* record identifier */
class RecordID {
   FamilyType  family;
   RecordType  record;

   void setFamily( const char* recName );

   void setRecord( const char* recName );


 public:
   RecordID() {
      
      family = FDNULL;
      record = RECNULL;
   }

   
   RecordID( const char* recName ) {

      setFamily( recName );
      setRecord( recName );
   }


   ~RecordID() { }


   void setID( const char* recName ) {

      setFamily( recName );
      setRecord( recName );
   }


   FamilyType getFamily() const {
      return family;
   }


   RecordType getRecord() const {
      return record;
   }
};



class FieldID {
   /* common static fields */
   int  ST_ID;   /* static ID */
   int  ST_procID;   /* location - procedure record ID */
   int  ST_line;   /* location - line number */

   /* proc */
   int  ST_proc_fileName;   /* location - file name */
   int  ST_proc_procName;   /* location - proc name */

   /* decomp */
   int  ST_decomp_distID;   /* dist ID */

   /* dist */
   int  ST_dist_distType;   /* distribution type */

   /* align */
   int  ST_align_decompID;   /* decomp ID */

   /* dist array */
   int  ST_array_name;  /* array name */
   int  ST_array_dim;   /* array dimension */
   int  ST_array_locLB;   /* local lower bound */
   int  ST_array_locUB;   /* local upper bound */
   int  ST_array_alignID;   /* align ID */

   /* mesg send */
   int  ST_send_opType;   /* communication type */
   int  ST_send_size;   /* message size */
   int  ST_send_arrayID;   /* dist array ID */
   int  ST_send_useLB;   /* portion of array used in a communicaiton */
   int  ST_send_useUB;
   int  ST_send_useStep;   /* array stride (step) */
   int  ST_send_dependID;   /* depend record ID */

   /* recv & send wait & recv wait */
   int  ST_wait_sendID;   /* mesg send ID */

   /* depend */
   int  ST_depend_srcLine;   /* depend source line */
   int  ST_depend_sinkLine;   /* depend sink line */

   /* static symbolic record */
   int  ST_sym_value;   /* static symbolic value */

   /* common dynamic fields */
   int  DY_ID;   /* dynamic record ID */
   int  DY_staticID;   /* static ID */
   int  DY_node;   /* processor ID */
   int  DY_dest;   /* destination node */
   int  DY_count;   /* event count */
   int  DY_second;   /* seconds */

   /* procedure */
   int  DY_proc_inSecond;   /* inclusive seconds */
   int  DY_proc_exSecond;   /* exclusive seconds */

   /* dynamic symbolic record */
   int  DY_sym_node;   /* processor ID */
   int  DY_sym_value;   /* runtime symbolic value */

   /* runtime system configuration */
   int  DY_sys_nnode;   /* number of processors used in this application */

   void testAndSetIndex( const char* recName, int* theField, int index );


 public:
   FieldID() {
      /* default values */
      /* common static fields */
      ST_ID = INT_MIN;
      ST_procID = INT_MIN;   
      ST_line = INT_MIN;   

      /* proc */
      ST_proc_fileName = INT_MIN;
      ST_proc_procName = INT_MIN;

      /* decomp */
      ST_decomp_distID = INT_MIN;

      /* dist */
      ST_dist_distType = INT_MIN; 

      /* align */
      ST_align_decompID = INT_MIN; 

      /* dist array */
      ST_array_name = INT_MIN;
      ST_array_dim = INT_MIN;   
      ST_array_locLB = INT_MIN;   
      ST_array_locUB = INT_MIN;   
      ST_array_alignID = INT_MIN;   

      /* mesg send */
      ST_send_opType = INT_MIN;
      ST_send_size = INT_MIN;   
      ST_send_arrayID = INT_MIN;   
      ST_send_useLB = INT_MIN;   
      ST_send_useUB = INT_MIN;
      ST_send_useStep = INT_MIN; 
      ST_send_dependID = INT_MIN;

      /* recv & send wait & recv wait */
      ST_wait_sendID = INT_MIN;

      /* depend */
      ST_depend_srcLine = INT_MIN;
      ST_depend_sinkLine = INT_MIN;

      /* static symbolic record */
      ST_sym_value = INT_MIN;  

      /* common dynamic fields */
      DY_ID = INT_MIN;   
      DY_staticID = INT_MIN;   
      DY_node = INT_MIN;   
      DY_dest = INT_MIN;   
      DY_count = INT_MIN;
      DY_second = INT_MIN;   

      /* procedure */
      DY_proc_inSecond = INT_MIN;
      DY_proc_exSecond = INT_MIN;   

      /* dynamic symbolic record */
      DY_sym_node = INT_MIN;  
      DY_sym_value = INT_MIN;  

      /* runtime system configuration */
      DY_sys_nnode = INT_MIN;   
   }

   ~FieldID() { }

   void setIndex( const char* recName, const RecordDossier& inDossier,
		 const RecordID& inRecord );

   int getST_ID() const {
      return ST_ID;
   }

   int getST_procID() const {
      return ST_procID;
   }

   int getST_line() const {
      return ST_line;
   }

   int getST_proc_fileName() const {
      return ST_proc_fileName;
   }

   int getST_proc_procName() const {
      return ST_proc_procName;
   }

   int getST_decomp_distID() const {
      return ST_decomp_distID;
   }

   int getST_dist_distType() const {
      return ST_dist_distType;
   }

   int getST_align_decompID() const {
      return ST_align_decompID;
   }

   int getST_array_name() const {
      return ST_array_name;
   }

   int getST_array_dim() const {
      return ST_array_dim;
   }

   int getST_array_locLB() const {
      return ST_array_locLB;
   }

   int getST_array_locUB() const {
      return ST_array_locUB;
   }

   int getST_array_alignID() const {
      return ST_array_alignID;
   }

   int getST_send_opType() const {
      return ST_send_opType;
   }

   int getST_send_size() const {
      return ST_send_size;
   }

   int getST_send_arrayID() const {
      return ST_send_arrayID;
   }

   int getST_send_useLB() const {
      return ST_send_useLB;
   }

   int getST_send_useUB() const {
      return ST_send_useUB;
   }

   int getST_send_useStep() const {
      return ST_send_useStep;
   }

   int getST_send_dependID() const {
      return ST_send_dependID;
   }

   int getST_wait_sendID() const {
      return ST_wait_sendID;
   }

   int getST_depend_srcLine() const {
      return ST_depend_srcLine;
   }

   int getST_depend_sinkLine() const {
      return ST_depend_sinkLine;
   }

   int getST_sym_value() const {
      return ST_sym_value;
   }

   int getDY_ID() const {
      return DY_ID;
   }

   int getDY_staticID() const {
      return DY_staticID;
   }

   int getDY_node() const {
      return DY_node;
   }

   int getDY_dest() const {
      return DY_dest;
   }

   int getDY_count() const {
      return DY_count;
   }

   int getDY_second() const {
      return DY_second;
   }

   int getDY_proc_inSecond() const {
      return DY_proc_inSecond;
   }

   int getDY_proc_exSecond() const {
      return DY_proc_exSecond;
   }

   int getDY_sym_node() const {
      return DY_sym_node;
   }

   int getDY_sym_value() const {
      return DY_sym_value;
   }

   int getDY_sys_nnode() const {
      return DY_sys_nnode;
   }
};


/*          up
             ^
             |
   prec <- this -> next
             |
             V
           down
*/

/* entry of line statistic table */
class LineStatCell {
   char  fileName[NAME_SIZE];
   char  procName[NAME_SIZE];
   int   lineCount;             /* number of entries in lineNums[] */
   int   lineNums[LINE_SIZE];
   FamilyType  family;

   /* statistic data */
   struct {
      int         count;
      int         size;
      TimeStr     second;
      ArrayCell*  down;   /* for mesg: array data locality information */
   } nodes[NNODES];

 public:
   LineStatCell() {

      lineCount = 0;
      for ( int i = 0; i < NNODES; i++ ) {
	 nodes[i].count = 0;
	 nodes[i].size = 0;
//	 nodes[i].second.procInclude = 0;
//	 nodes[i].second.procExclude = 0;
//	 nodes[i].second.mesgInclude = 0;
//	 nodes[i].second.mesgExclude = 0;
	 nodes[i].down = NULL;
      }
   }


   ~LineStatCell() { }


   void addBaseInfo( const char* inFileName, const char* inProcName, 
		    int inLineCount, const int* inLineNums,
		    const RecordID& recordID );


   void addStatistic( int myID, const TimeStr& inSecond, int inCount = 1,
		     int inSize = 0 );
	    

   char* getFileName() const {
      return (char*) fileName;
   }


   char* getProcName() const {
      return (char*) procName;
   }


   int getLineCount() const {
      return lineCount;
   }


   const int* getLineNums() const {
      return lineNums;
   }


   void fprintLineNums( FILE* ofd ) const {

      fprintf( ofd, "\n*** lineNums = " );
      for ( int i = 0; i < lineCount; i++ )
	   fprintf( ofd, "%d  ", lineNums[i] );
      fprintf( ofd, "\n" );
   }


   FamilyType getFamily() const {
      return family;
   }


   void addCount( int myID, int inCount ) {
      nodes[myID].count += inCount;
   }


   int getCount( int myID ) const {
      return nodes[myID].count;
   }


   void fprintCount( FILE* ofd, int nnode ) const {

      int  max, min, thisCount;
      double  sum, var, mean;

      fprintf( ofd, "    count: (" );
      max = INT_MIN; min = INT_MAX; 
      sum = 0; var = 0;
      for ( int i = 0; i < nnode; i++ ) {
	 thisCount = nodes[i].count;
	 fprintf( ofd, "  %d", thisCount );
	 if ( max < thisCount )
	      max = thisCount;
	 if ( min > thisCount )
	      min = thisCount;
	 sum += thisCount;
	 var += pow( thisCount, 2.0 );   /* double sqrt( double ) */
      }
      mean = sum / (double) nnode;
      var = sqrt( ( var / (double) nnode ) - pow( mean, 2.0 ) );
      fprintf( ofd, "  )  %d  %d  %6.2f  %6.2f\n", max, min, mean, var );
   }


   void addSize( int myID, int inSize ) {
      nodes[myID].size += inSize;
   }


   int getSize( int myID ) const {
      return nodes[myID].size;
   }


   void fprintSize( FILE* ofd, int nnode ) const {

      int  max, min, thisSize;
      double  sum, var, mean;

      if ( family != FDMESG )   /* only FDMESG has valid mesg size data */
	   return;

      fprintf( ofd, "    mesg size: (" );
      max = INT_MIN; min = INT_MAX; 
      sum = 0; var = 0;
      for ( int i = 0; i < nnode; i++ ) {
         thisSize = nodes[i].size;
         fprintf( ofd, "  %d", thisSize );
         if ( max < thisSize )
              max = thisSize;
         if ( min > thisSize )
              min = thisSize;
         sum += thisSize;
	 var += pow( thisSize, 2.0 );
      }
      mean = sum / (double) nnode;
      var = sqrt( ( var / (double) nnode ) - pow( mean, 2.0 ) );
      fprintf( ofd, "  )  %d  %d  %6.2f  %6.2f\n", max, min, mean, var );
   }


   void addTime( int myID, const TimeStr& inSecond ) {

      nodes[myID].second += inSecond;
//      nodes[myID].second.procInclude += inSecond.procInclude;
//      nodes[myID].second.procExclude += inSecond.procExclude;
//      nodes[myID].second.mesgInclude += inSecond.mesgInclude;
//      nodes[myID].second.mesgExclude += inSecond.mesgExclude;
   }


   const TimeStr& getTime( int myID ) const {
      return nodes[myID].second;
   }


   void fprintTime( FILE* ofd, int nnode ) const {

      double  max, min, thisTime;
      double  sum, mean, var;

      /* (proc) inclusive time */
      if ( family == FDPROC )
	  fprintf( ofd, "    incProc: (" );
      else
	  fprintf( ofd, "    seconds: (" );

      max = INT_MIN; min = INT_MAX; 
      sum = 0; var = 0;
      for ( int i = 0; i < nnode; i++ ) {
         thisTime = nodes[i].second.procInclude;
         fprintf( ofd, "  %8.6f", thisTime );
         if ( max < thisTime )
              max = thisTime;
         if ( min > thisTime )
              min = thisTime;
         sum += thisTime;
	 var += pow( thisTime, 2.0 );
      }
      mean = sum / (double) nnode;
      var = sqrt( ( var / (double) nnode ) - pow( mean, 2.0 ) );
      fprintf( ofd, "  )  %8.6f  %8.6f  %8.6f  %8.6f\n", max, min, mean, var );

      if ( family != FDPROC )   /* only FDPROC has other second fields */
           return;

      /* proc exclusive time */
      fprintf( ofd, "    excProc: (" );
      max = INT_MIN; min = INT_MAX; 
      sum = 0; var = 0;
      for ( int i = 0; i < nnode; i++ ) {
         thisTime = nodes[i].second.procExclude;
         fprintf( ofd, "  %8.6f", thisTime );
         if ( max < thisTime )
              max = thisTime;
         if ( min > thisTime )
              min = thisTime;
         sum += thisTime;
	 var += pow( thisTime, 2.0 );
      }
      mean = sum / (double) nnode;
      var = sqrt( ( var / (double) nnode ) - pow( mean, 2.0 ) );
      fprintf( ofd, "  )  %8.6f  %8.6f  %8.6f  %8.6f\n", max, min, mean, var );


      /* mesg inclusive time */
      fprintf( ofd, "    incMesg: (" );
      max = INT_MIN; min = INT_MAX; 
      sum = 0; var = 0;
      for ( int i = 0; i < nnode; i++ ) {
         thisTime = nodes[i].second.mesgInclude;
         fprintf( ofd, "  %8.6f", thisTime );
         if ( max < thisTime )
              max = thisTime;
         if ( min > thisTime )
              min = thisTime;
         sum += thisTime;
         var += pow( thisTime, 2.0 );
     }
      mean = sum / (double) nnode;
      var = sqrt( ( var / (double) nnode ) - pow( mean, 2.0 ) );
      fprintf( ofd, "  )  %8.6f  %8.6f  %8.6f  %8.6f\n", max, min, mean, var );


      /* mesg exclusive time */
      fprintf( ofd, "    excMesg: (" );
      max = INT_MIN; min = INT_MAX; 
      sum = 0; var = 0;
      for ( int i = 0; i < nnode; i++ ) {
         thisTime = nodes[i].second.mesgExclude;
         fprintf( ofd, "  %8.6f", thisTime );
         if ( max < thisTime )
              max = thisTime;
         if ( min > thisTime )
              min = thisTime;
         sum += thisTime;
         var += pow( thisTime, 2.0 );
     }
      mean = sum / (double) nnode;
      var = sqrt( ( var / (double) nnode ) - pow( mean, 2.0 ) );
      fprintf( ofd, "  )  %8.6f  %8.6f  %8.6f  %8.6f\n", max, min, mean, var );
   }


   void linkDown( int myID, ArrayCell* downCell ) {
      nodes[myID].down = downCell;
   }


   ArrayCell* getDown( int myID ) const {
      return nodes[myID].down;
   }


   LineStatCell& operator=( const LineStatCell& lineStatCell ) {
      
      if ( lineStatCell.lineCount == 0 )   
	   /* no valid fields in this entry, no action is needed */
	   return *this;

      /* valid entry, copy everything over */
      strcpy( fileName, lineStatCell.fileName );
      strcpy( procName, lineStatCell.procName );

      lineCount = lineStatCell.lineCount;
      int i;
      for ( i = 0; i < lineCount; i++ )
	   lineNums[i] = lineStatCell.lineNums[i];
      
      family = lineStatCell.family;

      for ( i = 0; i < NNODES; i++ ) {
	 nodes[i].count = lineStatCell.nodes[i].count;
	 nodes[i].size = lineStatCell.nodes[i].size;
	 nodes[i].second = lineStatCell.nodes[i].second;
	 nodes[i].down = lineStatCell.nodes[i].down;
      }

      return *this;
   }
};


/* entry of array data locality table */
class DataLocalityCell {
   char  fileName[NAME_SIZE];
   char  procName[NAME_SIZE];
   char  arrayName[NAME_SIZE];
   int   lineNum;   /* array declaration should come from one line only */
   int   nDim;
   int   nEntry;    /* total number of (cluster-)entries */
   struct {
      DistType  distType;   /* re-arranged according to "Align"[Dim List] */
      int       localLB;
      int       localUB;
      int       dataSize;   /* data locality cluster size */
   }  dims[NDIMS];

   EntryCell*  downs[NNODES];

 public:
   DataLocalityCell() {

      lineNum = INT_MIN;
      for ( int i = 0; i < NNODES; i++ )
	   downs[i] = NULL;
   }


   ~DataLocalityCell() { }


   void addBaseInfo( const char* inFileName, const char* inProcName, 
                    const char* inArrayName, int inLineNum, int inDim );


   const char* getFileName() const {
      return fileName;
   }


   const char* getProcName() const {
      return procName;
   }


   const char* getArrayName() const {
      return arrayName;
   }


   int getLineNum() const {
      return lineNum;
   }


   int getDim() const {
      return nDim;
   }


   void setEntryCount( int inEntry ) {
      nEntry = inEntry;
   }


   int getEntryCount() const {
      return nEntry;
   }


   void setDistType( int dim, int type ) {

      switch ( type ) {
       case 1:
	 dims[dim].distType = BLOCK;
	 break;

       case 2:
	 dims[dim].distType = CYCLIC;
	 break;

       case 3:
	 dims[dim].distType = BLOCKCYCLIC;
	 break;

       default:
	 dims[dim].distType = NOTDIST;
         break;
      }
   }


   DistType getDistType( int dim ) const {
      return dims[dim].distType;
   }


   void setLocalLB( int dim, int localLB ) {
      dims[dim].localLB = localLB;
   }


   int getLocalLB( int dim ) const {
      return dims[dim].localLB;
   }


   void setLocalUB( int dim, int localUB ) {
      dims[dim].localUB = localUB;
   }


   int getLocalUB( int dim ) const {
      return dims[dim].localUB;
   }


   void setDataSize( int dim, int dataSize ) {
      dims[dim].dataSize = dataSize;
   }


   int getDataSize( int dim ) const {
      return dims[dim].dataSize;
   }


   void linkDown( int myID, EntryCell* downCell ) {
      downs[myID] = downCell;
   }


   EntryCell* getDown( int myID ) const {
      return downs[myID];
   }


   DataLocalityCell& operator=( const DataLocalityCell& dataLocalityCell ) {
      
      if ( lineNum == INT_MIN )
	   /* dummy entry, skip it */
	   return *this;

      /* valid entry, copy everything over */
      strcpy( fileName, dataLocalityCell.fileName );
      strcpy( procName, dataLocalityCell.procName );
      strcpy( arrayName, dataLocalityCell.arrayName );

      lineNum = dataLocalityCell.lineNum;
      nDim = dataLocalityCell.nDim;

      int  i;
      for ( i = 0; i < nDim; i++ ) {
	 dims[i].distType = dataLocalityCell.dims[i].distType;
	 dims[i].localLB = dataLocalityCell.dims[i].localLB;
	 dims[i].localUB = dataLocalityCell.dims[i].localUB;
	 dims[i].dataSize = dataLocalityCell.dims[i].dataSize;
      }

      return *this;
   }
};


class UsageInfo {
   int  arrayID;   /* static dist array record, locality by each array */
   int  mesgID;   /* static mesg send record, locality by each statement */
   int  node;
   int  count;   /* number of use of each entry in this mesg send */
   struct {
      int  useLB;
      int  useUB;
      int  step;
   } dims[NDIMS];

 public:
   UsageInfo() {
      arrayID = INT_MIN;
   }

   UsageInfo( int inArrayID, int inMesgID, int inNode, int inCount ) {
      arrayID = inArrayID;
      mesgID = inMesgID;
      node = inNode;
      count = inCount;
   }

   ~UsageInfo() { }


   void addBaseInfo( int inArrayID, int inMesgID, int inNode, int inCount ) {
      arrayID = inArrayID;
      mesgID = inMesgID;
      node = inNode;
      count = inCount;
   }


   int getArrayID() const {
      return arrayID;
   }

   
   int getMesgID() const {
      return mesgID;
   }


   int getNode() const {
      return node;
   }


   int getCount() const {
      return count;
   }


   void setUseLB( int dim, int useLB ) {
      dims[dim].useLB = useLB;
   }


   int getUseLB( int dim ) const {
      return dims[dim].useLB;
   }


   void setUseUB( int dim, int useUB ) {
      dims[dim].useUB = useUB;
   }


   int getUseUB( int dim ) const {
      return dims[dim].useUB;
   }


   void setStep( int dim, int step ) {
      dims[dim].step = step;
   }


   int getStep( int dim ) const {
      return dims[dim].step;
   }
};



class ArrayCell {
   int  staticID;
   ArrayCell*  next;
   EntryCell*  down;

 public:
   ArrayCell() {

      staticID = INT_MIN;
      next = NULL;
      down = NULL;
   }


   // ArrayCell( int inID ) {		 * V.S.Adve: This constructor
   //					 * has the same functionality
   //    staticID = inID;		 * as the next one, because
   //    next = NULL;			 * of the chosen defaults.
   //    down = NULL;			 * Also, both cannot be used under
   // }					 * Solaric CC, because they can
   //					 * have the same default signature.

   ArrayCell( int inID, ArrayCell* prevCell = NULL,
	     ArrayCell* nextCell = NULL,
	     EntryCell* downCell = NULL ) {

      staticID = inID;
      if ( prevCell != NULL ) 
	   prevCell->linkNext( this );
      next = nextCell;
      down = downCell;
   }


   ~ArrayCell() { }


   void addID( int inID ) {
      staticID = inID;
   }


   int getID() const {
      return staticID;
   }


   void linkNext( ArrayCell* nextCell ) {
      next = nextCell;
   }


   ArrayCell* getNext() const {
      return next;
   }


   void linkDown( EntryCell* downCell ) {
      down = downCell;
   }


   EntryCell* getDown() const {
      return down;
   }


   ArrayCell* updateList( ArrayCell*& thisArray, int inID );

};


/* array entry, used to collect array data locality information */
class EntryCell {
   int  entryIDs[NDIMS];   /* entry index */
   int  count;            /* # of usage of this entry */
   EntryCell*  next;      /* pointer to next EntryCell */

 public:
   EntryCell( ) { 

      for ( int i = 0; i < NDIMS; i++ ) {
	 entryIDs[i] = INT_MIN;
      }
      count = 0;
      next = NULL; 
   }

   // V.S.Adve:  The next two definitions of EntryCell::EntryCell() have
   //     a common usage, and are ambiguous. Solaris CC doesn't allow this.
   //     But the two are identical in function anyway.
   //
   // EntryCell( int* inEntryIDs, int increaseBy = 1 ) { 
   // 
   //    for ( int i = 0; i < NDIMS; i++ ) {
   // 	 entryIDs[i] = inEntryIDs[i];
   //    }
   //    /* as long as the first dim is not a dummy value, this is a valid 
   // 	 entry */
   //    count = ( entryIDs[0] == INT_MIN ) ? 0 : increaseBy;
   //    next = NULL; 
   // }


   /* insert a new EntryCell into listing */
   EntryCell( int* inEntryIDs, int increaseBy = 1, EntryCell* prevCell = NULL,
	     EntryCell* nextCell = NULL ) {

      for ( int i = 0; i < NDIMS; i++ ) {
         entryIDs[i] = inEntryIDs[i];
      }
      count = ( entryIDs[0] == INT_MIN ) ? 0 : increaseBy;
      next = nextCell;
      if ( prevCell != NULL )
	   prevCell->linkNext( this );
   }


   ~EntryCell() { }


   void addID( int inDim, int* inEntryIDs, int increaseBy = 1 ) {

      for ( int i = 0; i < inDim; i++ ) {
         entryIDs[i] = inEntryIDs[i];
      }
      count = ( entryIDs[0] == INT_MIN ) ? 0 : increaseBy;
   }


   int* getID() const {
      return  (int*) entryIDs;
   }


   void addCount( int increaseBy = 1 ) {
      count += increaseBy;
   }


   int getCount() const {
      return count;
   }


   void linkNext( EntryCell* nextCell ) {
      next = nextCell;
   }


   EntryCell* getNext() const {
      return next;
   }



   EntryCell* updateList( EntryCell*& thisEntry, int* inEntryIDs, int count,
			 int dim );

};



/* line cell, part of a listing used to keep line statistic data and
   array data locality data in order
*/
class LineCell {
   int  lineNum;
   int  staticID;
   LineCell*  next;

 public:
   LineCell() {

      lineNum = INT_MIN;
      staticID = INT_MIN;
      next = NULL;
   }


   LineCell( int inLineNum, int inStaticID, LineCell* prevCell = NULL,
	    LineCell* nextCell = NULL ) {

      lineNum = inLineNum;
      staticID = inStaticID;
      if ( prevCell != NULL )
	   prevCell->linkNext( this );
      next = nextCell;
   }


   ~LineCell() { }


   int getLineNum() const {
      return lineNum;
   }


   int getStaticID() const {
      return staticID;
   }


   void linkNext( LineCell* nextCell ) {
      next = nextCell;
   }


   LineCell* getNext() const {
      return next;
   }


   void updateList( ProcCell* thisProc, int inLineNum, int inStaticID );

};



/* proc name cell, part of a listing used to keep line statistic data and
   array data locality data in order. It is not really necessary to have
   this as part of the listing, because line numbers in a (physical) file
   are already in (procedure) order - it is just that, within one file, they 
   are not in (procedure) alphabetic order.
*/
class ProcCell {
   char       name[NAME_SIZE];
   ProcCell*  next;
   LineCell*  down;

 public:
   ProcCell() {

      name[0] = '\0';
      next = NULL;
      down = NULL;
   }


   ProcCell( const char* inProcName, int inLineNum, int inStaticID, 
	    ProcCell* prevCell = NULL, ProcCell* nextCell = NULL );


   ~ProcCell() { }


   const char* getName() const {
      return name;
   }


   void linkNext( ProcCell* nextCell ) {
      next = nextCell;
   }


   ProcCell* getNext() const {
      return next;
   }


   void linkDown( LineCell* downCell ) {
      down = downCell;
   }


   LineCell* getDown() const {
      return down;
   }


   void updateList( FileCell* thisFile, const char* inProcName, int inLineNum, 
		   int inStaticID );

};



/* file name cell, part of a listing used to keep line statistic data and
   array data locality data in order
*/
class FileCell {
   char  name[NAME_SIZE];
   FileCell*  next;
   ProcCell*  down;

 public:
   FileCell() {

      name[0] = '\0';
      next = NULL;
      down = NULL;
   }


   FileCell( const char* inFileName, const char* inProcName, int inLineNum, 
	    int inStaticID, FileCell* prevCell = NULL,
	    FileCell* nextCell = NULL );


   ~FileCell() { }


   const char* getName() const {
      return name;
   }

   
   void linkNext( FileCell* nextCell ) {
      next = nextCell;
   }


   FileCell* getNext() const {
      return next;
   }


   void linkDown( ProcCell* downCell ) {
      down = downCell;
   }


   ProcCell* getDown() const {
      return down;
   }


   FileCell* updateList( const char* inFileName, const char* inProcName, 
			int inLineNum, int inStaticID );

};



/* entry used in ProcStack */
class ProcNode {
    int  ID;
    ProcNode*  next;

  public:
    ProcNode() {
	ID = INT_MIN;
	next = (ProcNode *) NULL;
    }


    ProcNode( int inID, ProcNode* inNext ) {
	ID = inID;
	next = inNext;
    }


    ~ProcNode() { }


    int getID() const {
	return ID;
    }


    ProcNode* getNext() const {
	return next;
    }
};



class ProcStack {
    ProcNode*  top[NNODES];

  public:
    ProcStack() {
	int  i;
	for ( i = 0; i < NNODES; i++ )
	    top[i] = (ProcNode *) NULL;
    }


    ~ProcStack() { }


    ProcNode* getTop( int node ) const {
	return top[node];
    }


    void push( int node, int ID ) {
	top[node] = new ProcNode( ID, top[node] );
    }

    
    void pop( int node, int ID ) {
	if ( top[node] == NULL ) {
	    cerr << "ERROR: empty ProcStack\n";
//	    exit(-1);

	} else if ( ID != top[node]->getID() ) {
	    cerr << "ERROR: ID inconsistent in ProcStack.pop()\n";
//	    exit(-1);

	} else {
	    ProcNode* temp = top[node];
	    top[node] = top[node]->getNext();
	    delete temp;
	}
    }
};


//**************** external declarations **********************************//
// Added by V.S.Adve, 8/12/94.
// These are needed because this code is called from different places,

//**** Globals from FDCombine6b.h ****

extern FileCell*  lineHeader;
extern FileCell*  dataHeader;

extern symbolTableStr*    symbolTable;
extern RecordDossier**    staticTable;
extern LineStatCell*      lineStatTable;
extern DataLocalityCell*  dataLocTable;

extern int   GnumNodes;		// number of processors used the application
extern int   currentTableSize;  // size of each table indexed by staticID

extern FieldID     GfieldID;	// field index
extern ProcStack   GprocStack;
extern char*       progName;

extern Boolean_    GprintCluster;   // output index in cluster format


//**** Externally accessible function from ProcessTrace.C ****

extern int ProcessTraces(char* inFileName,
			 char* outFileName = (char*) NULL,
			 FileType outFileType = UNDEFINED_FILE_TYPE,
			 int Ggranular = 0,
			 int Gpartition = 1);

#endif  FDCombine6_h
