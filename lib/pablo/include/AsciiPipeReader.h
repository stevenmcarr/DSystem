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
 * Project Manager and Pincipal Investigator:
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
 * AsciiPipeReader.h: class dealing with input from an ascii pipe
 *
 * $Header: /home/cs/carr/cvsroot/DSystem/lib/pablo/include/AsciiPipeReader.h,v 1.1 1999/09/08 17:45:13 carr Exp $
 */

#ifndef AsciiPipeReader_h
#define AsciiPipeReader_h

#include "DataTraits.h"
#include "PipeReader.h"

class Array;
class BaseDescriptor;
class FieldDescriptor;
class StreamPipe;
class Value;

class AsciiPipeReader : public PipeReader {
/*
* The AsciiPipeReader class contains methods designed to accept input from
* an ascii pipe.
*/
private:
	static const char *const MY_CLASS;	// My class name
	static PacketHeader noHeader;		// a header for PIPE_EMPTY pkt

	int 		    bufferSize;		// how big we are
	char		    *packetBuffer;	// buffer to store a single pkt
	char		    *shadowBuffer;	// shadow of packetBuffer
	char		    *bufp;		// pointer used in parsing data
	Attributes 	    nameToTag;		// data name to descriptor table
	PacketHeader	    currentHeader;	// header for the current packet

	/* Method _badInput:		Call abort() with a message indicating
	* 				what was expected and where the
	*				parsing failed.  You never return
	*				from this method.                     */
	void _badInput( const char *message, const char *parseString ) const;

	/* Method _getArray:		Parse an Array data record picking out
	*			        dimension and cell values.	      */
	void _getArray( Array *array, MachineDataType type, int dimension );

	/* Method _getAttributes:	Parse data and fill in attributes with
	*			        key/value pairs found.		      */
	void _getAttributes( BaseDescriptor & descriptor );

	/* Method _getCommandTag:	Parse data and return integer tag for
	*				the command packet.		      */
	int _getCommandTag();

	/* Method _getDataTag:		Parse data for the name and look up
	*				tag in the nameToTag table. Tag is 
	*				returned.		              */
	int _getDataTag();

	/* Method _getDescriptorTag:	Parse data and return the integer tag
	*				for the descriptor.  Also insert the
	*				name/tag in the nameToTag table.      */
	int _getDescriptorTag();

	/* Method _getField:		Parse data for the field information 
	*				and set fieldDescr values.	      */
	void _getField( FieldDescriptor& fieldDescr );

	/* Method _getPipeAttributes:	Parse data for the attributes and
	*				insert key/value pairs in 
	*				pipeAttributes.			      */
	void _getPipeAttributes();

	/* Method _getString:		Parse data and string.  	      */
	void _getString( char **string );

	/* Method _getType:		Parse data and set type.	      */
	void _getType( MachineDataType& type );
		
	/* Method _getValue:		Parse data for a value.               */
	void _getValue( Value& val, MachineDataType type );

	/* Method _readPacket:		Read the next packet from the pipe. 
	*				Update currentHeader & pipeAttributes */
	Boolean_ _readPacket();

	/* Method _skipWhiteSpace:	Skip over white space in data.        */
	void _skipWhiteSpace();

public:
	/* Method AsciiPipeReader:	The constructor.		      */
	AsciiPipeReader( StreamPipe *initPipe );
	
	/* Method ~AsciiPipeReader:	The destructor.			      */
	~AsciiPipeReader();

	/* Method getData:		Parse the record data in internal 
	*				buffer into recordDossier.	      */
	virtual Boolean_ getData( RecordDossier& recordDossier );

	/* Method getDescriptor:	Parse the structure descriptor in
	*				internal buffer into structDescr.     */
	virtual Boolean_ getDescriptor( StructureDescriptor& structDescr );

	/* Method getPacketDataPtr:	Virtual method that does not make    
	*				sense for this class & is not
	*			        implemented.			      */
	virtual const char * getPacketDataPtr() const {
		shouldNotImplement(" AsciiPipeReader::getPacketDataPtr" ); 
		return NULL;
		} ;
	
	/* Method getPacketHeader:	Get header of packet next in pipe.  If
	*				packet is there it is actually read 
	*				into packetBuffer.		      */
	virtual PacketHeader getPacketHeader( );
};

#endif AsciiPipeReader_h

