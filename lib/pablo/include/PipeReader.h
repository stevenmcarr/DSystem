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
 * Author: Ruth A. Aydt (aydt@cs.uiuc.edu)
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
* PipeReader.h: Definition of an Abstract Class for parsing pipe data.
*
*	$Header: /home/cs/carr/cvsroot/DSystem/lib/pablo/include/PipeReader.h,v 1.1 1999/09/08 17:45:13 carr Exp $
*/

#ifndef PipeReader_h
#define PipeReader_h

#include  <sys/types.h>

#include "Attributes.h"
#include "PacketHeader.h"
#include "Obj.h"
#include "StreamPipe.h"

class RecordDossier;
class StructureDescriptor;

class PipeReader : public Obj {
/*
* A PipeReader is an abstract class defining the general methods for 
* parsing what comes off the pipe.
*/

protected:
	StreamPipe 	*pipe;
	Attributes 	pipeAttributes;
	off_t		startOffset; 

public:
	/* Method PipeReader:		The constructor */
	PipeReader( StreamPipe *thePipe );

	/* Method ~PipeReader:		The destructor */
	~PipeReader();

	/* Method getData:		Parse the record data from the interal
	*				buffer into the recordDossier. 	Fail if
	*				not enough data or if type is not 
	*				PKT_DATA			      */
	virtual Boolean_ getData( RecordDossier& recordDossier ) = 0;

	/* Method getDescriptor:	Parse the structure descriptor from
	*				the internal buffer into structDescr.
	*				Fail if not enough data or if type is
	*				not PKT_DESCRIPTOR		      */
	virtual Boolean_ getDescriptor( StructureDescriptor& structDescr ) = 0;

	/* Method getPacketDataPtr:	Return pointer to the data section of 
	*				the cached packet.  Beware that this
	*				pointer  will become invalid after the
	*				next getPacketHeader() call           */
	virtual const char * getPacketDataPtr() const = 0;

	/* Method getPacketHeader:	Returns the header of the next packet
	*				in the pipe, or a header with type of
	*				PIPE_EMPTY if there is no packet.  If
	*				a packet DOES exist, then it is read
	*				into the internal buffer of the Pipe
	*				Reader.  If it's type PKT_ATTRIBUTES
	*				then pipeAttributes is updated.       */
	virtual PacketHeader getPacketHeader() = 0;


	/* Method getAttributes:	Copy pipeAttributes to attributes     */
	void getAttributes( Attributes& attributes ) const;

	/* Method getFileSize:		Returns the size of the file the pipe
	*				is connected to.  Not valid for all
	*				types of pipes - if call made and 
	*				invalid, program aborts with error 
	*				message.			      */
	off_t getFileSize();

	/* Method getOffset:		Returns the current offset into the
	*				pipe being read from.  Not valid for
	*				all types of pipes - if call made and
	*				invalid, program aborts with error
	*				message.                   	      */
	off_t getOffset();

	/* Method rewind:		Rewind the offset to what it was when
	*				the PipeReader was created.  Not valid
	*				for all types of pipes - if call made
	*				and invalid, program aborts with error
	*				message.                              */
	void rewind();

	/* Method setOffset:		Sets the current offset into the pipe
	* 				being read from.  Not valid for all
	*				types of pipes - if call made and 
	*				invalid, program aborts with error
	*				message.                              */
	void setOffset( off_t offset );

	/* Method successfulConnection:	Returns TRUE_ if the connection was
	*				successful in the constructor.        */
	Boolean_ successfulConnection() const;

	/* Method printOn:		Helper function for output	      */
	virtual void printOn( ostream& os = cout ) const;

};

inline off_t
PipeReader::getFileSize()
{
	return( pipe->getFileSize() );
}

inline off_t
PipeReader::getOffset()
{
	return( pipe->getByteOffset() );
}

inline void
PipeReader::rewind()
{
	pipe->setByteOffset( startOffset );
}

inline void
PipeReader::setOffset( off_t offset )
{
	pipe->setByteOffset( offset );
}

inline Boolean_
PipeReader::successfulConnection() const
{
	return( CnvToBoolean_(pipe != NULL) );
}

inline ostream& operator<<( ostream& os, PipeReader &r )
{
	r.printOn( os );
	return os;
}

#endif PipeReader_h
