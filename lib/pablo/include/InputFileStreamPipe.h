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
 * Contributing Author: Ruth Aydt (aydt@cs.uiuc.edu)
 * Contributing Author: Bill Whitehouse (whitehou@cs.uiuc.edu)
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
 * InputFileStreamPipe.h: A StreamPipe that reads from a file.
 *
 *	$Header: /home/cs/carr/cvsroot/DSystem/lib/pablo/include/InputFileStreamPipe.h,v 1.1 1999/09/08 17:45:13 carr Exp $
 */

#ifndef InputFileStreamPipe_h
#define InputFileStreamPipe_h

#include "FileStreamPipe.h"

static const int DEFAULTreadPoolSize = 204800;  // Default size of the read pool

class InputFileStreamPipe : public FileStreamPipe {
/*
 * An InputFileStreamPipe is a pipe that takes its input from a disk file.
 */

private:
	static const char *const MY_CLASS;         // My class name

        PabloSddfType       myFileType;            // Type of file
	DataCharacteristics *fileCharacteristics;  // Details if Binary SDDF

	Boolean_ endOfFileReached;         // Set to flag end-of-file.
	off_t	 nextByteInFile;           // Next byte to be read from file

	char	 *peekBuffer;	  	   // Buffer for peeking
	int 	 peekBufferOffset;	   // Current position in peek Buffer
	int	 peekBufferSize;           // Size of peek Buffer
	int	 peekLength;		   // Bytes unread in peek buffer

	char 	 *theReadPool;  	   // Pointer to the read pool.
	int  	 readPoolSize;		   // Size of the read pool.
	int  	 readPoolOffset;	   // Current offset in read pool.
	int  	 bytesReadIntoPool;	   // Number of valid bytes in read pool

	/* Method _readPool:   		Reads from beginning of the pre-fetch 
         *                              pool                                  */
	int _readPool( char* theBuffer, int amountToRead );


public:
	/* Method InputFileStreamPipe: 	The constructor 		      */
	InputFileStreamPipe( const char *fileName, 
			     int bytesInReadPool = DEFAULTreadPoolSize );

	/* Method ~InputFileStreamPipe: The destructor 			      */
	~InputFileStreamPipe();

	/* Method bytesFree: 		Returns the number of bytes that can 
	*				be written 			      */
	virtual int bytesFree() const;

	/* Method bytesReady: 		Returns the number of bytes ready to 
	*				read 				      */
	virtual int bytesReady() const;

	/* Method createPipeReader:	Creates the appropriate type of Pipe
	*				Reader for the input file we're 
	*			 	processing and return it to the caller.
	*				Returns NULL if unable to create the 
	*				reader for some reason.               */
	virtual PipeReader * createPipeReader() const;

	/* Method get: 			Read data from the file, returning 
	*				number of bytes read.                 */
	virtual int get( char *data, int lengthRequested );

	/* Method getInputCharacteristics: Returns pointer to the information
	*				on the data characteristics for the
	*				input file. This will be NULL if the
	*				file isn't in Binary SDDF format.     */
	virtual const DataCharacteristics * getInputCharacteristics() const;

        /* Method getByteOffset:        Returns the current byte offset in the 
	*				file associated with this pipe. Note 
	*				that calls to peek() do not advance the 
	*				offset.      			      */
        virtual off_t getByteOffset();

        /* Method getFileSize:          Returns the length of the file. Does
	*				not change the current offset.        */
        virtual off_t getFileSize();

	/* Method getInputType:		Returns SDDF type of the input file.  */
	virtual PabloSddfType getInputType() const;

	/* Method getSkippingPeek:	Read data from the file, skipping any
	*				data that is in the peek buffer. Also
	*				clears peek buffer.                   */
	virtual int getSkippingPeek( char *data, int lengthRequested );

	/* Method isEmpty: 		Returns TRUE_ if the pipe is empty    */
	virtual Boolean_ isEmpty() const;

	/* Method isFull: 		Returns TRUE_ if the pipe is full     */
	virtual Boolean_ isFull() const;

	/* Method isInputPipe:          Always returns TRUE_	              */
	virtual Boolean_ isInputPipe() const { 
		return TRUE_; 
		} ;

	/* Method peek: 		Peek from the file  		      */
	virtual int peek( char *data, int lengthRequested );

	/* Method put: 			It is an error to put to a read-file  */
	virtual int put( const char *, int );

        /* Method setByteOffset:        Sets the current byte offset in the 
	*				file associated with this pipe.  Any
	*				bytes in the peekBuffer at the time
	*				this call is made are cleared.        */
        virtual void setByteOffset( off_t newByteOffset );

	/* Method printOn: 		Stream output function 		      */
	virtual void printOn( ostream& os ) const;
};

inline off_t
InputFileStreamPipe::getByteOffset() 
{
	return nextByteInFile;
}

inline const DataCharacteristics* 
InputFileStreamPipe::getInputCharacteristics() const
{
	return fileCharacteristics;
}

inline PabloSddfType 
InputFileStreamPipe::getInputType() const
{
	return myFileType;
}

inline ostream & operator<<( ostream &os, InputFileStreamPipe& p )
{
	p.printOn( os );
	return os;
}

#endif

