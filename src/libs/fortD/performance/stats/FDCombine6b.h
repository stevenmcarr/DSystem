/* $Id: FDCombine6b.h,v 1.1 1997/03/11 14:29:10 carr Exp $ */
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

#ifndef  FDCombine6b_h
#define  FDCombine6b_h


//********************** function declarations ******************************//
extern void  processFiles();

extern void  tableManager( int index );

extern void  symTableManager( int index );

extern void  collectLineStatData( const RecordDossier& origDossier,
				 const RecordID& recordID );

extern void  retrieveTime( const RecordDossier& origDossier, 
			  const RecordID& recordID, 
			  TimeStr& second, 
			  int& count,
			  int myNode );

extern void  retrieveLocation( const RecordDossier* staticDossier, 
			      const RecordID& recordID,
			      char* fileName, 
			      char* procName, 
			      int& lineCount,
			      int*lineNums );

extern void retrieveArrayLocation( const RecordDossier* arrayDossier,
				  char* fileName, 
				  char* procName, 
				  int& lineCount,
				  int*lineNums );

extern void  retrieveName( const RecordDossier* recordDossier, 
			  char* fileName,
			  char* procName );

extern void  retrieveMesgLine( const RecordDossier* recordDossier, 
			      int& lineCount,
			      int* lineNums );

extern void  outputLineStatData();

extern void  updateDataLocality( const RecordDossier& origDossier,
				const RecordID& recordID );

extern void  getArrayCluster( const RecordDossier* arrayDossier, 
			     int arrayID,
			     DataLocalityCell& theArray );

extern void  distributionInfo( const RecordDossier* arrayDossier,
			      DataLocalityCell& theArray );

extern void  getBoundary( const RecordDossier* staticDossier, 
			 UsageInfo* usageInfo,
			 int& boundIndex );

extern void  clusterSizeAll( DataLocalityCell& theArray );

extern int  indexToCluster( int dim, 
			   int arrayIndex, 
			   const UsageInfo* usageInfo );

extern void  clusterToIndex( int dim, 
			    int clusterIndex, 
			    int& lowerIndex, 
			    int& upperIndex,
			    const DataLocalityCell& theArray );

extern void  outputDataLocality( const char* title );

extern void  printIndex( int* entryIDs, 
			int nDim, 
			Boolean_& haveValue, 
			int count,
			int printCount, 
			FILE* ofd );

extern void  printRange( int* entryIDs, 
			int nDim, 
			Boolean_& haveValue, 
			int count,
			int printCount, 
			const DataLocalityCell& theArray, 
			FILE* ofd );

extern void  allocHDFArray( HDFdataStr& HDFdata, 
			   const DataLocalityCell& theArray );

extern void  markBoundary( int current, 
			  int theOne, 
			  int nDim, 
			  int* markID,
			  HDFdataStr& HDFdata, 
			  int* offSet );

extern void  updateHDFArray( HDFdataStr& HDFdata, 
			    const DataLocalityCell& theArray,
			    int* entryIDs, 
			    int myID, 
			    int count );

extern FileCell*  updateOutList( FileCell* theHeader, 
				const char* fileName,
				const char* procName, 
				int lineNum, 
				int staticID );

extern void  updateLineTableLoc( const UsageInfo* usageInfo );

extern void  recursiveLineTable( int currentDim, 
				int* entryIDs, 
				const int& myID,
				const int& arrayID, 
				const int& count, 
				const int& dim,
				const UsageInfo* usageInfo, 
				EntryCell*& entryHeader,
				ArrayCell* thisArray );

extern void  updateDataTableLoc( const UsageInfo* usageInfo );

extern void  recursiveDataTable( int currentDim, 
				int* entryIDs, 
				const int& myID,
				const int& arrayID, 
				const int& count, 
				const int& dim,
				const UsageInfo* usageInfo, 
				EntryCell*& entryHeader );


//*********************** global variables **********************************//

/* global headers */
FileCell*  lineHeader;
FileCell*  dataHeader;

symbolTableStr*    symbolTable;
RecordDossier**    staticTable;
LineStatCell*      lineStatTable;
DataLocalityCell*  dataLocTable;

/* global variables */
			    /* V.S.Adve: removed static keyword on GnumNodes */
int   	     GnumNodes;     /* number of processors used the application */

static int   Ggranular;     /* the granularity of cluster size */
static int   Gpartition;    /* partition scheme: equal partition or size */

int  	    currentTableSize;	/* V.S.Adve: removed static keyword */
int  	    currentSymTableSize;	/* V.S.Adve: removed static keyword */

FieldID     GfieldID;   /* field index */
ProcStack   GprocStack;
char*       progName = "dPablo";

Boolean_    GprintCluster = TRUE_;   /* output index in cluster format */

#endif  FDCombine6b_h
