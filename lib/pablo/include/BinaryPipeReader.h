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
* BinaryPipeReader.h: A class designed to handle an input from a binary pipe.
*
*	$Header: /home/cs/carr/cvsroot/DSystem/lib/pablo/include/BinaryPipeReader.h,v 1.1 1999/09/08 17:45:13 carr Exp $
*/

#ifndef BinaryPipeReader_h
#define BinaryPipeReader_h

#include "Defines.h"
#include "PipeReader.h"

class StreamPipe;
	
class BinaryPipeReader : public PipeReader {
/*
* The BinaryPipeReader class implements methods which handle reading input
* from a binary pipe.  This is accomplished primarily by calling the
* appropriate parser methods in the classes corresponding to the sort
* of data we are reading ( Descriptors, Attributes, ... ).
*/
private:
	enum{ PACKET_HEADER_SIZE = PACKET_HEADER_FIELD_CNT * sizeof( int ) };

	static const char *const MY_CLASS; // My class name
	static PacketHeader noHeader;	   // a header for PIPE_EMPTY packet.

	int                 bufferSize;	   // how big are we
	char 		    *packetBuffer; // buffer to store a single packet
	PacketHeader 	    currentHeader; // the header for the current packet

	/* Method _readPacket:		Grabs a packet from the pipe, updating
	*				currentHeader and pipeAttributes as
	*				appropriate			      */
	Boolean_ _readPacket( );

public:
	/* Method BinaryPipeReader:	The constructor			      */
	BinaryPipeReader( StreamPipe *initPipe );

	/* Method ~BinaryPipeReader:	The destructor			      */
	~BinaryPipeReader();

	/* Method getData:		Parse the record data in interal buffer
	*				into recordDossier. 		      */
	virtual Boolean_ getData( RecordDossier& recordDossier );

	/* Method getDescriptor:	Parse the structure descriptor in
	*				internal buffer into structDescr      */
	virtual Boolean_ getDescriptor( StructureDescriptor& structDescr );

	/* Method getPacketDataPtr:	Return pointer to data section of 
	*				packetBuffer		              */
	virtual const char * getPacketDataPtr() const;

	/* Method getPacketHeader:	Get header of packet next in pipe. If 
	*				packet is there, it is actually read 
	*				into packetBuffer 		      */
	PacketHeader getPacketHeader( );
};

#endif BinaryPipeReader_h
