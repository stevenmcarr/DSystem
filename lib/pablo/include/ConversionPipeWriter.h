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
* ConversionPipeWriter.h: Handles output to a pipe connected to a binary output
* file whose data characteristics do not match those of the native machine.
*
* $Header: /home/cs/carr/cvsroot/DSystem/lib/pablo/include/ConversionPipeWriter.h,v 1.1 1999/09/08 17:45:13 carr Exp $
*/

#ifndef ConversionPipeWriter_h
#define ConversionPipeWriter_h

#include "DataCharacteristics.h"
#include "PipeWriter.h"

class ConversionPipeWriter : public PipeWriter {
/*
*
* ConversionPipeWriter is the class which writes binary SDDF data with
* characteristics different than those of the native machine to a pipe
* connected to an output file.
*
*/
private:
	static const char *const MY_CLASS;	// My class name

	int 	     bufferSize;	// How big the buffer is
	PacketHeader currentHeader;	// Header for current packet 
	char 	     *packetBuffer;	// buffer to store a single packet

	DataCharacteristics* dataInfo;	// Characteristics of output

        int PACKET_HEADER_SIZE;		// Depends on sizeof(int) for output 

	/* Method _writePacket:		Write the packet constructed in
	*				packetBuffer to the pipe.	     */
	void _writePacket();

protected:
        /* Method _writeSddfHeader:     Write the SDDF header information to 
        *                               the current Stream Pipe.             */
        virtual void _writeSddfHeader() const;

public:
	/* Method ConversionPipeWriter: A constructor with no specification
	*				of the output file's characteristics.
	*				The file will be written with the
	*				same characteristics as the native
	*				machine EXCEPT the byte order will
	*				be reversed.                        */
	ConversionPipeWriter( StreamPipe *thePipe );

	/* Method ConversionPipeWriter:	A constructor with explicit 
	*				specification of the output file's
	*				data characteristics.                */
	ConversionPipeWriter( StreamPipe *thePipe, 
			      const DataCharacteristics& outCharacteristics );

	/* Method ~ConversionPipeWriter: The destructor			     */
        ~ConversionPipeWriter();

        /* Method addPipe:              Add another pipe to the list we
	*                               write to.                            */
	virtual Boolean_ addPipe( StreamPipe *thePipe );

	/* Method putAttributes:	Write pipeAttributes to the pipe     */
	virtual void putAttributes();

	/* Method putAttributes:	Update pipeAttributes and write them
	*				to the pipe.			     */
	virtual void putAttributes( const Attributes& attributes );

	/* Method putCommand:		Write the command to the pipe        */
	virtual void putCommand( int pktTag = 0 );

	/* Method putData:		Write the data associated with a
	*				record to the pipe.		     */
	virtual void putData( const RecordDossier& recordDossier );

	/* Method putDescriptor:	Write the Structure Descriptor to the 
	*				pipe.				     */
	virtual void putDescriptor( const StructureDescriptor& structDescr,
				    int pktTag );

  	/* Method putPacket:            Virtual method that does not make 
	*                               sense for this class and is not 
	*                               implemented.                         */
	virtual void putPacket( const PacketHeader&, const char * ) {
		shouldNotImplement( " ConversionPipeWriter::putPacket" );
		} ;
};

#endif ConversionPipeWriter_h
