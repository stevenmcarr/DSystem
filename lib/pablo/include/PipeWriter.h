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
 *		 University of Illinois at Urbana-Champaign
 *		 Department of Computer Science
 *		 1304 W. Springfield Avenue
 *		 Urbana, IL	61801
 *
 * Copyright (c) 1987-1994
 * The University of Illinois Board of Trustees.
 *	All Rights Reserved.
 *
 * Author: Ruth Aydt (aydt@cs.uiuc.edu)
 * Contributing Author: Tom Birkett (birkett@cs.uiuc.edu)
 * Contributing Author: Taed Nelson (nelson@cs.uiuc.edu)
 *
 * Project Manager and Principal Investigator:
 *	Daniel A. Reed (reed@cs.uiuc.edu)
 *
 * Funded by: National Science Foundation grants NSF CCR86-57696,
 * NSF CCR87-06653 and NSF CDA87-22836 (Tapestry), NASA ICLASS Contract
 * No. NAG-1-613, DARPA Contract No. DABT63-91-K-0004, by a grant
 * from the Digital Equipment Corporation External Research Program,
 * and by a collaborative research agreement with the Intel Supercomputer
 * Systems Division.
 *
 */
/*
* PipeWriter.h: Abstract Class defining general methods for outputting
* pipe data.
*
*	$Header: /home/cs/carr/cvsroot/DSystem/lib/pablo/include/PipeWriter.h,v 1.1 1999/09/08 17:45:13 carr Exp $
*/
    
#ifndef PipeWriter_h
#define PipeWriter_h

#include <sys/types.h>

#include "Attributes.h"
#include "PacketHeader.h"
#include "Obj.h"

class RecordDossier;
class StreamPipe;
class StructureDescriptor;

class PipeWriter : public Obj {
/*
* A PipeWriter is an abstract class defining the methods needed to
* output data to the pipe. 
*/

#define PIPES_SUPPORTED 16
protected:
	StreamPipe	*pipe[PIPES_SUPPORTED];
	int		activePipes;
	Attributes	pipeAttributes;
	off_t		startOffset;

        /* Method _writeSddfHeader:     Write the SDDF header information to 
        *                               the current Stream Pipe.             */
        virtual void _writeSddfHeader() const = 0;

public:
	/* Method PipeWriter:		The constructor			     */
	PipeWriter( StreamPipe *thePipe );

	/* Method ~PipeWriter:		The destructor			     */
	~PipeWriter( );

	/* Method addPipe:		Add another pipe to the list we 
	*				write to. 			     */
	virtual Boolean_ addPipe( StreamPipe *thePipe );

	/* Method deletePipe:		Delete the pipe from the list we
	*				write to.			     */
	void deletePipe( StreamPipe *thePipe );

	/* Method putAttributes:	Write pipeAttributes to the pipe.    */
	virtual void putAttributes() = 0;

	/* Method putAttributes:	Update pipeAttributes and write it
	*				to the pipe.			     */
	virtual void putAttributes( const Attributes& attributes ) = 0;

	/* Method putCommand:		Write the command to the pipe        */
	virtual void putCommand( int pktTag = 0 ) = 0;

	/* Method putData:		Write the data associated with
	*				a record to the pipe		     */
	virtual void putData( const RecordDossier& recordDossier ) = 0;

	/* Method putDescriptor:	Write the Structure Descriptor to 
	*				the pipe.                            */
	virtual void putDescriptor( const StructureDescriptor& structDescr,
				    int pktTag ) = 0;

	/* Method putPacket:		Write a Packet to the pipe.	     */
	virtual void putPacket( const PacketHeader& pktHeader, 
				const char *pktData ) = 0;


        /* Method getFileSize:          Returns the size of the file the pipe
        *                               is connected to.  Not valid for all
        *                               types of pipes - if call made and 
        *                               invalid, program aborts with error 
        *                               message.                              */
        off_t getFileSize();

        /* Method getOffset:            Returns the current offset into the
        *                               pipe being written to.  Not valid for
        *                               all types of pipes - if call made and
        *                               invalid, program aborts with error
        *                               message.                              */
        off_t getOffset();

        /* Method rewind:               Rewind the offset to what it was when
        *                               the PipeWriter was created and the
	*				header written (file is truncated to
	*				the shorter length).  Not valid for all 
	*				types of pipes - if call made
        *                               and invalid, program aborts with error
        *                               message.                              */
        void rewind();

        /* Method setOffset:            Sets the current offset into the pipe
        *                               being read from (no change in data
	*				takes place).  Not valid for all
        *                               types of pipes - if call made and
        *                               invalid, program aborts with error
        *                               message.                              */
        void setOffset( off_t offset );

        /* Method successfulConnection: Returns TRUE_ if the connection was
        *                               successful in the constructor.        */
        Boolean_ successfulConnection() const;
};

inline Boolean_
PipeWriter::successfulConnection() const
{
        return( CnvToBoolean_( activePipes != 0 ) );
}

#endif PipeWriter_h
