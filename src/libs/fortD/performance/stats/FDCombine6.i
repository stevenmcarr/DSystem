/* $Id: FDCombine6.i,v 1.1 1997/03/11 14:29:09 carr Exp $ */
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
 * Subject: function definitions for new line statistic (and data
 *          locality) modules
 *
 * FDCombine6.i:
 *    definitions of some member functions; this code used to be in
 *    FDCombine6.h
 */

#include <libs/fortD/performance/stats/FDCombine6.h>

//******************************** procedures *******************************//

void
RecordID::setFamily( const char* recName )
{
   const int  COMP_SIZE = 6;

      if ( strncmp( recName, "FDMesg", COMP_SIZE ) == 0 )
           family = FDMESG;
      else if ( strncmp( recName, "FDProc", COMP_SIZE ) == 0 )
           family = FDPROC;
      else if ( strncmp( recName, "FDLoop", COMP_SIZE ) == 0 )
           family = FDLOOP;
      else if ( strncmp( recName, "FDDSym", COMP_SIZE ) == 0 )
           family = FDDSYM;
      else if ( strncmp( recName, "FDStat", COMP_SIZE ) == 0 )
           family = FDSTAT;
      else if ( strncmp( recName, "FDSSym", COMP_SIZE ) == 0 )
           family = FDSSYM;
      else if ( strncmp( recName, "FDSysF", COMP_SIZE ) == 0 )
           family = FDSYSF;
      else
           family = FDNULL;

}   /* end of RecordID::getFamily() */



void
RecordID::setRecord( const char* recName )
{
   char* nameString = strchr( recName, ' ' );   /* first blank space */

      if ( nameString ) {   /* not a NULL */
         switch( family ) {
          case FDSTAT:
            if ( !strcmp( nameString, " Decomp" ) )
                 record = ST_DECOMP;
            else if ( !strcmp( nameString, " Dist" ) )
                 record = ST_DIST;
            else if ( !strcmp( nameString, " Align" ) )
                 record = ST_ALIGN;
            else if ( !strcmp( nameString, " Array" ) )
                 record = ST_ARRAY;
            else if ( !strcmp( nameString, " Proc" ) )
                 record = ST_PROC;
            else if ( !strcmp( nameString, " Depend" ) )
                 record = ST_DEPEND;
            else if ( !strcmp( nameString, " Mesg Send" ) )
                 record = ST_SEND;
            else if ( !strcmp( nameString, " Mesg Recv" ) )
                 record = ST_RECV;
            else if ( !strcmp( nameString, " Mesg Send Wait" ) )
                 record = ST_SENDW;
            else if ( !strcmp( nameString, " Mesg Recv Wait" ) )
                 record = ST_RECVW;
            else
                 record = RECNULL;
            break;

          case FDPROC:
          case FDLOOP:
            if ( !strcmp( nameString, " Entry Trace" ) )
                 record = DY_TRACEB;                      /* trace entry */
            else if ( !strcmp( nameString, " Exit Trace" ) )
                 record = DY_TRACEE;                      /* trace exit */
            else if ( !strcmp( nameString, " Entry Count" ) )
                 record = DY_COUNTB;                      /* count entry */
            else if ( !strcmp( nameString, " Exit Count" ) )
                 record = DY_COUNTE;                      /* count exit */
            else
                 record = RECNULL;
            break;

          case FDMESG:
            if ( !strcmp( nameString, " Blk Send Begin" ) )
                 record = DY_CSENDB;
            else if ( !strcmp( nameString, " Blk Send End" ) )
                 record = DY_CSENDE;
            else if ( !strcmp( nameString, " Blk Recv Begin" ) )
                 record = DY_CRECVB;
            else if ( !strcmp( nameString, " Blk Recv End" ) )
                 record = DY_CRECVE;
            else if ( !strcmp( nameString, " Nonblk Send Begin" ) )
                 record = DY_ISENDB;
            else if ( !strcmp( nameString, " Nonblk Send End" ) )
                 record = DY_ISENDE;
            else if ( !strcmp( nameString, " Nonblk Recv Begin" ) )
                 record = DY_IRECVB;
            else if ( !strcmp( nameString, " Nonblk Recv End" ) )
                 record = DY_IRECVE;
            else if ( !strcmp( nameString, " Oper Begin" ) )
                 record = DY_GLOPB;
            else if ( !strcmp( nameString, " Oper End" ) )
                 record = DY_GLOPE;
            else if ( !strcmp( nameString, " Wait Begin" ) )
                 record = DY_WAITB;
            else if ( !strcmp( nameString, " Wait End" ) )
                 record = DY_WAITE;
            else
                 record = RECNULL;
            break;

          default:
            record = RECNULL;
            break;
         }

      } else {
         record = RECNULL;
      }
}   /* end of RecordID::getRecord() */



void 
FieldID::testAndSetIndex( const char* recName, int* theField, int index ) 
{
   if ( *theField == INT_MIN ) {   /* no valid value yet */
      *theField = index;
      
   } else if ( *theField != index ) {
      cerr << form( "WARNING: unmatched field IDs in %s\n", recName );
   }
}   /* end of FieldID::testAndSetIndex() */



void 
FieldID::setIndex( const char* recName, const RecordDossier& inDossier,
		  const RecordID& inRecord ) 
{
   int  index;

   char*  nameString = strchr( recName, ' ' );   /* first blank space */
   FamilyType  family = inRecord.getFamily();
   RecordType  record = inRecord.getRecord();

   switch ( family ) {
    case FDSTAT:
      /* generic index */
      if ( ( index = inDossier.getFieldID( "Static ID" ) ) != -1 )
	   testAndSetIndex( recName, &ST_ID, index );
      if ( ( index = inDossier.getFieldID( "Procedure ID" ) ) != -1 )
	   testAndSetIndex( recName, &ST_procID, index );
      if ( ( index = inDossier.getFieldID( "Line Number" ) ) != -1 )
	   testAndSetIndex( recName, &ST_line, index );

      switch ( record ) {
       case ST_PROC:
	 if ( ( index = inDossier.getFieldID( "File Name" ) ) != -1 )
	      testAndSetIndex( recName, &ST_proc_fileName, index );
	 if ( ( index = inDossier.getFieldID( "Procedure Name" ) ) != -1 )
	      testAndSetIndex( recName, &ST_proc_procName, index );
	 break;

       case ST_SEND:
	 /* mesg send index */
	 if ( ( index = inDossier.getFieldID( "Mesg Oper Type" ) ) != -1 )
	      testAndSetIndex( recName, &ST_send_opType, index );
	 if ( ( index = inDossier.getFieldID( "Sym Mesg Size" ) ) != -1 )
	      testAndSetIndex( recName, &ST_send_size, index );
	 if ( ( index = inDossier.getFieldID( "Array ID" ) ) != -1 )
              testAndSetIndex( recName, &ST_send_arrayID, index );
	 if ( ( index = inDossier.getFieldID( "Sym Array LB" ) ) != -1 )
	      testAndSetIndex( recName, &ST_send_useLB, index );
	 if ( ( index = inDossier.getFieldID( "Sym Array UB" ) ) != -1 )
	      testAndSetIndex( recName, &ST_send_useUB, index );
	 if ( ( index = inDossier.getFieldID( "Sym Array Step" ) ) != -1 )
	      testAndSetIndex( recName, &ST_send_useStep, index );
	 if ( ( index = inDossier.getFieldID( "Depend ID" ) ) != -1 )
	      testAndSetIndex( recName, &ST_send_dependID, index );
	 break;

       case ST_RECV:
       case ST_SENDW:
       case ST_RECVW:
	 if ( ( index = inDossier.getFieldID( "Mesg ID" ) ) != -1 )
	      testAndSetIndex( recName, &ST_wait_sendID, index );
	 break;

       case ST_ARRAY:
	 /* array index */
	 if ( ( index = inDossier.getFieldID( "Array Name" ) ) != -1 )
	      testAndSetIndex( recName, &ST_array_name, index );
	 if ( ( index = inDossier.getFieldID( "Dimensions" ) ) != -1 )
	      testAndSetIndex( recName, &ST_array_dim, index );
	 if ( ( index = inDossier.getFieldID( "Local Lower Bound" ) ) != -1 )
	      testAndSetIndex( recName, &ST_array_locLB, index );
	 if ( ( index = inDossier.getFieldID( "Local Upper Bound" ) ) != -1 )
	      testAndSetIndex( recName, &ST_array_locUB, index );
	 if ( ( index = inDossier.getFieldID( "Align ID" ) ) != -1 )
	      testAndSetIndex( recName, &ST_array_alignID, index );
	 break;

       case ST_DECOMP:
	 /* decomposition */
	 if ( ( index = inDossier.getFieldID( "Dist ID" ) ) != -1 )
	      testAndSetIndex( recName, &ST_decomp_distID, index );
	 break;

       case ST_DIST:
	 /* distribution */
	 if ( ( index = inDossier.getFieldID( "Dist Type" ) ) != -1 )
	      testAndSetIndex( recName, &ST_dist_distType, index );
	 break;

       case ST_ALIGN:
	 /* alignment */
	 if ( ( index = inDossier.getFieldID( "Decomp ID" ) ) != -1 )
	      testAndSetIndex( recName, &ST_align_decompID, index );
	 break;

       case ST_DEPEND:
	 /* depend */
	 if ( ( index = inDossier.getFieldID( "Dependence Source Line Number" ) ) != -1 )
	      testAndSetIndex( recName, &ST_depend_srcLine, index );
	 if ( ( index = inDossier.getFieldID( "Dependence Sink Line Number" ) ) != -1 )
	      testAndSetIndex( recName, &ST_depend_sinkLine, index );
	 break;
      }
      break;

    case FDSSYM:
      /* make sure the staticID has same fieldID as other static records */
      if ( ( index = inDossier.getFieldID( "Static ID" ) ) != -1 )
	   testAndSetIndex( recName, &ST_ID, index );
      if ( ( index = inDossier.getFieldID( "Symbolic Value" ) ) != -1 )
	   testAndSetIndex( recName, &ST_sym_value, index );
      break;

    case FDDSYM:
      /* make sure the DY_ID has same fieldID as other dynamic records */
      if ( ( index = inDossier.getFieldID( "Event ID" ) ) != -1 )
	   testAndSetIndex( recName, &DY_ID, index );
      if ( ( index = inDossier.getFieldID( "Processor Number" ) ) != -1 )
	   testAndSetIndex( recName, &DY_sym_node, index );
      if ( ( index = inDossier.getFieldID( "Symbolic Value" ) ) != -1 )
           testAndSetIndex( recName, &DY_sym_value, index );
      break;

    case FDSYSF:
      if ( ( index = inDossier.getFieldID( "Number of Processors" ) ) != -1 )
	   testAndSetIndex( recName, &DY_sys_nnode, index );
      break;

    case FDPROC:
    case FDLOOP:
    case FDMESG:
      /* common fields in dynamic records */
      if ( ( index = inDossier.getFieldID( "Event ID" ) ) != -1 )
	   testAndSetIndex( recName, &DY_ID, index );
      if ( ( index = inDossier.getFieldID( "Static ID" ) ) != -1 )
	   testAndSetIndex( recName, &DY_staticID, index );
      if ( ( index = inDossier.getFieldID( "Processor Number" ) ) != -1 )
	   testAndSetIndex( recName, &DY_node, index );
      
      /* seconds field */
      if ( family == FDMESG ) {
	 if ( ( index = inDossier.getFieldID( "Message Seconds" ) ) != -1 )
	      testAndSetIndex( recName, &DY_second, index );
	    
      } else if ( family == FDLOOP ) {
	 if ( ( index = inDossier.getFieldID( "Loop Seconds" ) ) != -1 )
	      testAndSetIndex( recName, &DY_second, index );

      } else {
	 if ( ( index = inDossier.getFieldID( "Inclusive Seconds" ) ) != -1 )
	      testAndSetIndex( recName, &DY_proc_inSecond, index );
	 if ( ( index = inDossier.getFieldID( "Exclusive Seconds" ) ) != -1 )
	      testAndSetIndex( recName, &DY_proc_exSecond, index );
      }

      /* proc/loop event count */
      if ( !strcmp( nameString, " Entry Count" ) ) {
	 if ( ( index = inDossier.getFieldID( "Event Count" ) ) != -1 )
	      testAndSetIndex( recName, &DY_count, index );
      }

      if ( family == FDMESG ) {
	 /* for csendb and isendb only */
	 if ( ( index = inDossier.getFieldID( "Destination Node" ) ) != -1 )
	      testAndSetIndex( recName, &DY_dest, index );
      }
      break;

    default:
      break;
   }
}   /* end of FieldID::setIndex() */



void 
LineStatCell::addBaseInfo( const char* inFileName, const char* inProcName, 
			  int inLineCount, const int* inLineNums,
			  const RecordID& recordID ) 
{
   if ( lineCount != 0 )   /* base info is already available */
	return;

   if ( strlen( inFileName ) >= NAME_SIZE ) 
	cerr << form( "WARNING: fileName length larger than %d\n", NAME_SIZE );
   strcpy( fileName, inFileName );

   if ( strlen( inProcName ) >= NAME_SIZE )
	cerr << form( "WARNING: procName length larger than %d\n", NAME_SIZE );
   strcpy( procName, inProcName );

   /* it is better to have line number(s) in ascending order */
   lineNums[0] = inLineNums[0];   /* asign the first value */
   lineCount = 1;
   for ( int i = 1; i < inLineCount; i++ ) {
      int j = lineCount - 1;
      while ( ( inLineNums[i] < lineNums[j] ) && ( j >= 0 ) ) 
	   j--;

      if ( j < 0 ) {   /* insert at the first place */
	 for ( int k = lineCount; k > 0; k-- ) {
	    lineNums[k] = lineNums[k-1];
	 }
	 lineNums[0] = inLineNums[i];
	 lineCount++;

      } else {
	 if ( inLineNums[i] != lineNums[j] ) {  /* skip duplicated line # */
	    if ( j != ( lineCount - 1 ) ) {    /* not add to tail */
	       for ( int k = lineCount; k > j+1; k-- ) {   /* move data */
		  lineNums[k] = lineNums[k-1];
	       }
	    }
	    lineNums[j+1] = inLineNums[i];
	    lineCount++;
	 }
      }
      }
   family = recordID.getFamily();

}   /* end of LineStatCell::addBaseInfo() */



void 
LineStatCell::addStatistic( int myID, const TimeStr& inSecond, int inCount,
			   int inSize) 
{
   nodes[myID].count += inCount;
   nodes[myID].size += inSize;
   
   nodes[myID].second += inSecond;
//      nodes[myID].second.procInclude += inSecond.procInclude;
//      nodes[myID].second.procExclude += inSecond.procExclude;
//      nodes[myID].second.mesgInclude += inSecond.mesgInclude;
//      nodes[myID].second.mesgExclude += inSecond.mesgExclude;

}   /* end of LineStatCell::addStatisitc() */



void 
DataLocalityCell::addBaseInfo( const char* inFileName, const char* inProcName, 
			      const char* inArrayName, int inLineNum, 
			      int inDim ) 
{
   if ( strlen( inFileName ) >= NAME_SIZE ) 
	cerr << form( "WARNING: fileName length larger than %d\n", NAME_SIZE );
   strcpy( fileName, inFileName );

   if ( strlen( inProcName ) >= NAME_SIZE )
	cerr << form( "WARNING: procName length larger than %d\n", NAME_SIZE );
   strcpy( procName, inProcName );

   if ( strlen( inArrayName ) >= NAME_SIZE )
	cerr << form( "WARNING: arrayName length larger than %d\n", NAME_SIZE );
   strcpy( arrayName, inArrayName );

   lineNum = inLineNum;
   nDim = inDim;

}   /* end of DataLocalityCell::addBaseInfo() */



ArrayCell* 
ArrayCell::updateList( ArrayCell*& thisArray, int inID ) 
{
   int  currentID;

   ArrayCell*  prev;
   ArrayCell*  current;
   ArrayCell*  header;
   ArrayCell*  newOne;

   prev = NULL;
   /* this procedure assume the update starting at the beginning */
   header = this;
   current = this;

   while ( current != NULL ) {
      currentID = current->getID();
      if ( inID < currentID ) {
	 thisArray = new ArrayCell( inID, prev, current );
	 if ( prev == NULL ) {   /* insert to the first place */
	    return thisArray;    /* return new header */
	 } else {                /* insert between two nodes */
	    return header;       /* return original header */
	 }

      } else if ( inID == currentID ) {
	 thisArray = current;
	 return header;

      } else {
	 prev = current;
	 current = current->getNext();
      }
   }

   /* append to the end */
   thisArray = new ArrayCell( inID, prev, current );
   if ( prev == NULL )     /* empty listing */
	return thisArray;
   else
	return header;
   
}   /* end of ArrayCell::updateList() */



EntryCell* 
EntryCell::updateList( EntryCell*& thisEntry, int* inEntryIDs, 
				 int count, int dim ) 
{
   EntryCell*  prev;
   EntryCell*  current;
   EntryCell*  theHeader;
   CompStatus  status;
   int*        currentIDs;

   /* thisEntry will point to the entry we just updated, we do not use
      this info at the moment, but just in case we will need it later */

   theHeader = this;   /* update should begin from current cell */
   prev = NULL;
   current = theHeader;

   status = EQ;
   while ( current != NULL ) {
      currentIDs = current->getID();
      int i = 0;
      while ( ( i < dim ) && ( status == EQ ) ) {
	 if ( inEntryIDs[i] == currentIDs[i] ) 
	      status = EQ;
	 else if ( inEntryIDs[i] < currentIDs[i] )
	      status = LT;
	 else
	      status = GT;
	 i++;
      }

      switch ( status ) {
       case EQ:
	 current->addCount( count );
	 thisEntry = current;
	 return theHeader;
	 break;

       case LT:
	 thisEntry =  new EntryCell( inEntryIDs, count, prev, current );
	 if ( prev == NULL ) {   /* insert to the first place */
	    return thisEntry;
	 } else {   /* insert between two nodes */
	    return theHeader;
	 }
	 break;
	       
       case GT:
	 prev = current;
	 current = current->getNext();
	 status = EQ;
	 break;
      }
   }

   /* append to the end */
   thisEntry = new EntryCell( inEntryIDs, count, prev, current );
   if ( prev == NULL )   /* empty listing */
	return thisEntry;
   else
	return theHeader;
   
}   /* end of EntryCell::updateList() */



void 
LineCell::updateList( ProcCell* thisProc, int inLineNum, int inStaticID ) 
{
   LineCell*  theHeader;
   LineCell*  prev;
   LineCell*  current;
   LineCell*  newOne;
   int        currentLine;
   int        currentID;

   prev = NULL;
   current = thisProc->getDown();

   if ( current == NULL ) {
      /* it should not happen, but just in case */
      newOne = new LineCell( inLineNum, inStaticID );
      thisProc->linkDown( newOne );
      return;
   }

   while ( current != NULL ) {
      currentLine = current->getLineNum();
      currentID = current->getStaticID();
      if ( inLineNum < currentLine ) {
	 if ( prev == NULL ) {   /* insert into the first place */
	    newOne = new LineCell( inLineNum, inStaticID, prev, current );
	    thisProc->linkDown( newOne );
	       
	 } else {   /* insert between two nodes */
	    newOne = new LineCell( inLineNum, inStaticID, prev, current );
	 }
	 return;

      } else if ( inLineNum == currentLine ) {   /* same line number */
	 if ( inStaticID < currentID ) {
	    if ( prev == NULL ) {   /* insert into the first place */
	       newOne = new LineCell( inLineNum, inStaticID, prev, current );
	       thisProc->linkDown( newOne );
		  
	    } else {   /* insert between two nodes */
	       newOne = new LineCell( inLineNum, inStaticID, prev, current );
	    }
	    return;

	 } else if ( inStaticID == currentID ) {
	    /* node already exists, do nothing */
	    return;
	    
	 } else {   /* check next node */
	    prev = current;
	    current = current->getNext();
	 }

      } else {   /* check next node */
	 prev = current;
	 current = current->getNext();
      }
   }
	    
   /* append to the end */
   newOne = new LineCell( inLineNum, inStaticID, prev, current );
   return;

}   /* end of LineCell::updateList() */



ProcCell::ProcCell( const char* inProcName, int inLineNum, int inStaticID, 
		   ProcCell* prevCell, 
		   ProcCell* nextCell) 
{
   if ( strlen( inProcName ) >= NAME_SIZE )
	cerr << form( "WARNING: procName length larger than %d\n",
		     NAME_SIZE );
   strcpy( name, inProcName );

   if ( prevCell != NULL )
	prevCell->linkNext( this );
   next = nextCell;
   down = new LineCell( inLineNum, inStaticID );

}   /* end of ProcCell::ProcCell() */



void 
ProcCell::updateList( FileCell* thisFile, const char* inProcName, 
		     int inLineNum, int inStaticID ) 
{
   int  status;

   ProcCell*  prev;
   ProcCell*  current;
   ProcCell*  newOne;
   const char*      currentName;

   LineCell*  theLine;
   LineCell*  newLine;

   prev = NULL;
   current = thisFile->getDown();

   if ( current == NULL ) {
      /* it should not happen, but just in case */
      newOne = new ProcCell( inProcName, inLineNum, inStaticID,
			    prev, current );
      thisFile->linkDown( newOne );
      return;
   }

   while ( current != NULL ) {
      currentName = current->getName();
      status = strcmp( inProcName, currentName );
      if ( status < 0 ) {
	 if ( prev == NULL ) {   /* insert into the first place */
	    newOne = new ProcCell( inProcName, inLineNum, inStaticID, 
				  prev, current );
	    thisFile->linkDown( newOne );

	 } else {   /* insert between two nodes */
	    newOne = new ProcCell( inProcName, inLineNum, inStaticID,
				  prev, current );
	 }
	 return;

      } else if ( status == 0 ) {   /* node already exists */
	 theLine = current->getDown();
	 if ( theLine == NULL ) {
	    /* it should not happen, but just in case */
	    newLine = new LineCell( inLineNum, inStaticID );
	    current->linkDown( newLine );

	 } else {
	    theLine->updateList( current, inLineNum, inStaticID );
	 }
	 return;
	 
      } else {   /* check next node */
	 prev = current;
	 current = current->getNext();
      }
   }

   /* append to the end */
   newOne = new ProcCell( inProcName, inLineNum, inStaticID, prev, 
			 current );
   return;

}   /* end of ProcCell::updateList() */



FileCell::FileCell( const char* inFileName, const char* inProcName, 
		   int inLineNum, 
		   int inStaticID, FileCell* prevCell,
		   FileCell* nextCell) 
{
   if ( strlen( inFileName ) >= NAME_SIZE )
	cerr << form( "WARNING: fileName length larger than %d\n",
		     NAME_SIZE );
   strcpy( name, inFileName );

   if ( prevCell != NULL )
	prevCell->linkNext( this );
   next = nextCell;
   down = new ProcCell( inProcName, inLineNum, inStaticID, NULL, NULL );

}   /* end of FileCell::FileCell() */



FileCell* 
FileCell::updateList( const char* inFileName, const char* inProcName, 
		     int inLineNum, int inStaticID ) 
{
   int  status;

   FileCell*  theHeader;
   FileCell*  prev;
   FileCell*  current;
   FileCell*  newOne;
   const char*      currentName;

   ProcCell*  theProc;
   ProcCell*  newProc;

   theHeader = this;   /* the first cell in fileName list */
   prev = NULL;
   current = this;
      
   while ( current != NULL ) {
      currentName = current->getName();
      int status = strcmp( inFileName, currentName );
      if ( status < 0 ) {
	 if ( prev == NULL ) {   /* insert into the first place */
	    theHeader = new FileCell( inFileName, inProcName, inLineNum, 
				     inStaticID, prev, current );

	 } else {   /* insert between two nodes */
	    newOne = new FileCell( inFileName, inProcName, inLineNum,
				  inStaticID, prev, current );
	 }
	 return theHeader;

      } else if ( status == 0 ) {   /* node already exists */
	 theProc = current->getDown();
	 if ( theProc == NULL ) {
	    /* it should not happen, but just in case */
	    newProc = new ProcCell( inProcName, inLineNum, inStaticID,
				   NULL, NULL );
	    current->linkDown( newProc );

	 } else {
	    theProc->updateList( current, inProcName, inLineNum, 
				inStaticID );
	 }
	 return theHeader;

      } else {   /* check next node */
	 prev = current;
	 current = current->getNext();
      }
   }

   /* append to the end */
   newOne = new FileCell( inFileName, inProcName, inLineNum,
			    inStaticID, prev, current );
   return theHeader;

}   /* end of FileCell::updateList() */


