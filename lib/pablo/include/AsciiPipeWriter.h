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
 * Contributing Author: Bobby A.A. Nazief (nazief@cs.uiuc.edu)
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
* AsciiPipeWriter.h: Outputs a Ascii data file given this file's internal
* representation.
*
*	$Header: /home/cs/carr/cvsroot/DSystem/lib/pablo/include/AsciiPipeWriter.h,v 1.1 1999/09/08 17:45:13 carr Exp $
*/

#ifndef AsciiPipeWriter_h
#define AsciiPipeWriter_h

#include "PipeWriter.h"

class Array;
class Value;

class AsciiPipeWriter : public PipeWriter {
/*
* An AsciiPipeWriter is the class which produces the Ascii file 
* representation from the internal representation of the data and the data's
* description.
*/
private:
	static const char *const MY_CLASS;	// My class name

	int bufferSize;			// How big the buffer is
	PacketHeader currentHeader;	// Header for current packet 
	char *packetBuffer;		// buffer to store a single packet
	char *bufp;			// pointer to current buffer position

	/* Method _checkBufferSize:	Make sure there is space for cnt
	*		  		characters in packetBuffer and if not
	*				call _increaseBufferSize()	     */
	void _checkBufferSize( int cnt );

	/* Method _increaseBufferSize:	If a newSize is nonzero, increase the
	*				size of packetBuffer to that. Else,
	*				increase it by some fixed value      */
	void _increaseBufferSize( int newSize = 0 );

	/* Method _putArray:	 	Format the array values and write the
	*				to packetBuffer.		     */
	void _putArray( Array *arrayPtr );

	/* Method _putC:		Write the character to packetBuffer
	*				and increment bufp		     */
	void _putC( char c ); 

	/* Method _putValue:		Format the value and write it to 
	*				packetBuffer.			     */
	void _putValue( Value *valuePtr );

	/* Method _writeCString:	Write the CString to the buffer and
	*				enclose it in ""'s.		     */
	void _writeCString( const CString& cstr );

	/* Method _writePacket:		Write the packet constructed in
	*				packetBuffer to the pipe.	     */
	void _writePacket();

protected:
	/* Method _writeSddfHeader:	Write the SDDF header information to 
	*				the current Stream Pipe.             */
	virtual void _writeSddfHeader() const;

public:
	/* Method AsciiPipeWriter:	The constructor			     */
	AsciiPipeWriter( StreamPipe *thePipe );

	/* Method ~AsciiPipeWriter:	The destructor			     */
        ~AsciiPipeWriter();

        /* Method addPipe:              Add another pipe to the list we
        *                               write to.                            */
        virtual Boolean_ addPipe( StreamPipe *thePipe );

	/* Method putAttributes:	Write pipeAttributes to the pipe     */
	virtual void putAttributes();

	/* Method putAttributes:	Update pipeAttributes and write them
	*				to the pipe.			     */
	virtual void putAttributes( const Attributes& attributes );

	/* Method putCommand:		Write a Command packet with the given
	*				tag to the pipe			      */
	virtual void putCommand( int pktTag );

	/* Method putData:		Write the data associated with a
	*				record to the pipe.		     */
	virtual void putData( const RecordDossier& recordDossier );

	/* Method putDescriptor:	Write the structure Descriptor to the 
	*				pipe.				     */
	virtual void putDescriptor( const StructureDescriptor& structDescr,
				    int pktTag );

	/* Method putPacket:		Virtual method that does not make 
	*				sense for this class and is not 
	*				implemented. 			     */
	virtual void putPacket( const PacketHeader&, const char * ) {
		shouldNotImplement(" AsciiPipeWriter::putPacket" );
		} ;

};

inline void 
AsciiPipeWriter::_putC( char c ) 
{ 
	*bufp++ = c; 
}

#endif AsciiPipeWriter_h
