/* $Id: FDCombine6.C,v 1.1 1997/03/11 14:29:09 carr Exp $ */
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
 * FDCombine6.C:
 *    copy from FDCombine4.C, use new classes.
 *
 *    take static/symbolic/dynamic records as input, this program focuses on
 *    data locality of distributed arrays, especially those used in
 *    communication, we want to observe the fraction of off-processor
 *    reference (and local-reference as well, if possible).
 */

#include <libs/fortD/performance/stats/FDCombine6.h>
#include <InitializeStatic.C>


main( int argc, char **argv )
{
   void processFiles( );
   // tableManager(1);  V.S.Adve: This moved to beginning of ProcessTraces()
   processFiles();
}



/* read/process records and prepare output */
void
processFiles( )
{
   /****************************************************************
    *    Get the name of the input file
    ****************************************************************/
   char                  inFileName[512];
   Status                inputFileStatus;
   InputFileStreamPipe   *In;
   PipeReader            *InPipe;

   cout << "Please enter name of the input SSDF file: ";
   cin >> inFileName;
      
   /****************************************************************
    *    Ask if output file is ascii, binary, or conversion format
    ****************************************************************/
   FileType        outputFileType = UNDEFINED_FILE_TYPE;

   while ( outputFileType == UNDEFINED_FILE_TYPE ) {
      cout << "\nOutput file in Ascii, Binary or Converted "
	   << "(reverse byte order)  [A, B, or C]: ";
      cin >> BUF;

      if ( BUF[0] == 'a' || BUF[0] == 'A' ) {
	 outputFileType = ASCII;
      } else if ( BUF[0] == 'b' || BUF[0] == 'B' ) {
	 outputFileType = BINARY;
      } else if ( BUF[0] == 'c' || BUF[0] == 'C' ) {
	 outputFileType = CONVERT;
      } else {
	 cerr << "ERROR: invalid format type.\n";
      }
   }


   /****************************************************************
    *    Get the name of the output file
    ****************************************************************/
   char                  outFileName[512];

   cout << "Please enter name of the output SDDF file: ";
   cin >> outFileName;

   /****************************************************************
    *    Get the value of globalGranu and globalPart
    ****************************************************************/

   int 			Ggranular, Gpartition;
   char                 BUF[512];

   /* cluster index is the basic unit in counting data locality, */
   /* array indices fall in the same cluster index will be considered */
   /* as the same event */
   /* e.g, array index (1..32) with granu = 1 and 4 processors will */
   /*      have a cluster index of (1..8), anything happens in array index */
   /*      (5..8) will be counted as happening to cluter index (2). */

   cout << "Please enter cluster granularity (int n) for data locality,\n";
   cout << "   n <  1 -- detail data locality, cluster index size = 1, or\n";
   cout << "   n >= 1 -- higher value means finer grain: ";
   cin >> BUF;
   Ggranular = atoi( BUF );

   Gpartition = -1;
   while ( ( Gpartition != 0 ) && ( Gpartition != 1 ) ) {
      cout << "Please enter partition scheme for data locality (0/1):\n";
      cout << "   0 -- equal number of clusters for every dimension, or\n";
      cout << "   1 -- equal cluster size for every dimension: ";
      cin >> BUF;
      Gpartition = atoi( BUF );
   }

   /****************************************************************
    *    Call ProcessTrace() to process the packets in the files.
    *	 Reads from the input pipe and updates the profile information.
    ****************************************************************/

   (void) ProcessTraces(inFileName, outFileName, FileType, Ggranular, Gpartition);
   
}   /* end of processFiles() */
