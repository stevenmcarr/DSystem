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
 * Author: Robert Olson (olson@cs.uiuc.edu)
 * Contributing Author: Ruth A. Aydt (aydt@cs.uiuc.edu)
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
 * StreamPipe.h: Abstract base class for stream-based pipes.
 *
 *	$Header: /home/cs/carr/cvsroot/DSystem/lib/pablo/include/StreamPipe.h,v 1.1 1999/09/08 17:45:13 carr Exp $
 */

#ifndef StreamPipe_h
#define StreamPipe_h

#include <sys/types.h>

#include "BasePipe.h"

class DataCharacteristics;
class PipeReader;

class StreamPipe : public BasePipe {
/*
 *	A StreamPipe supports stream-based communication (ie the passing
 *	of blocks of data without explicit record boundaries).
 */
public:
	/* Method StreamPipe: 			The StreamPipe constructor    */
	StreamPipe( );

	/* Method ~StreamPipe: 			The StreamPipe destructor     */
	~StreamPipe();

	/* Method bytesFree: 			Return the number of bytes 
	*					that can be written 	      */
	virtual int bytesFree() const = 0;

	/* Method bytesReady: 			Return the number of bytes 
	*					ready to be read	      */
	virtual int bytesReady() const = 0;

	/* Method createPipeReader:		Implemented by derived classes
	*					to create appropriate type of
	*				        PipeReader for the StreamPipe.
	*					Returns NULL if can't create
	*					PipeReader.                   */
	virtual PipeReader * createPipeReader() const = 0;

	/* Method get: 				Get data from the pipe	      */
	virtual int get( char *data, int lengthRequested ) = 0;

	/* Method getByteOffset:		Returns the current byte offset
	*					in the pipe.  This is not
	*					valid for all types of pipes.
	*					Implemented in appropriate
	*					derived classes.              */
	virtual off_t getByteOffset() {
		abort( "getByteOffset() is not valid for this class" );
		return -1;	
		} ;

	/* Method getFileSize:	        	Returns the length of the file. 
	*					Does not change the current 
	*					file pointer.  This is not 
	*					valid for all types of pipes.
	*					Implemented in appropriate
	*					derived classes.             */
	virtual off_t getFileSize() {
		abort( "getFileSize() is not valid for this class" );
		return -1;	
		} ;

	/* Method getInputCharacteristics:	Return pointer to the data
	*					characteristics for the input
	*					associated with this pipe.
	*					Only valid if inputType is
	*					CONVERT_BINARY_SDDF.  
	*					Implemented in appropriate
	*					derived classes.              */
	virtual const DataCharacteristics * getInputCharacteristics() const {
		shouldNotImplement( "getInputCharacteristics()" );
		return NULL;
		} ;

	/* Method getInputType:			Return the type of the input
	*					associated with this pipe.    
	*					Returns valid type in 
	*					appropriate derived classes.  */
	virtual PabloSddfType getInputType() const {
		return INVALID_TYPE;
		} ;

	/* Method getSkippingPeek: 		Get data from the pipe, 
	*					skipping over data in the peek
	*					buffer.  Also clear peek 
	*					buffer.                       */
	virtual int getSkippingPeek( char *data, int lengthRequested ) = 0;

	/* Method isEmpty: 			Returns TRUE_ if the pipe is 
	*					empty 			      */
	virtual Boolean_ isEmpty() const = 0;

	/* Method peek: 			Peek at data in the pipe      */
	virtual int peek( char *data, int lengthRequested ) = 0;

	/* Method put: 				Write data into the pipe      */
	virtual int put( const char *data, int length ) = 0;

	/* Method isFull: 			Returns TRUE_ if the pipe is 
	*					full	 		      */
	virtual Boolean_ isFull() const = 0;

	/* Method setByteOffset:		Sets the current byte offset
	*					in the pipe.  This is not
	*					valid for all types of pipes.
	*					Implemented in appropriate
	*					derived classes.              */
	virtual void setByteOffset( off_t ) {
		abort( "setByteOffset() is not valid for this class" );
		} ;

	/* Method truncateAfterByte:		Truncates the data in the pipe
	*					after the specified byte. This 
	*					is only valid for pipes 
	*					associated with output data 
	*					files.                        */
	virtual void truncateAfterByte( off_t ) {
		abort( "truncateAfterByte() is not valid for this class" );
		} ;

	/* Method printOn: 			Stream output function	      */
	virtual void printOn( ostream& os ) const;
};

inline ostream & operator<<( ostream& os, StreamPipe& p )
{
	p.printOn( os );
	return os;
}

#endif StreamPipe_h
