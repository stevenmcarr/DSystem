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
 * Contributing Author: Ruth Aydt (aydt@cs.uicu.edu)
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
 * OutputFileStreamPipe.h: A StreamPipe that writes to a file.
 *
 *	$Header: /home/cs/carr/cvsroot/DSystem/lib/pablo/include/OutputFileStreamPipe.h,v 1.1 1999/09/08 17:45:13 carr Exp $
 */

#ifndef OutputFileStreamPipe_h
#define OutputFileStreamPipe_h

#include "FileStreamPipe.h"

static const int DEFAULTwritePoolSize = 204800;    // Default size of write pool

class OutputFileStreamPipe : public FileStreamPipe {
/*
 * An OutputFileStreamPipe is a pipe that writes it output to a disk file.
 */

private: 
 	static const char *const MY_CLASS;      // My class name

	char     *theWritePool; 	        // The write pool array.
	int      writePoolSize;		        // Size of write pool
	int      writePoolOffset;	        // Offset within the write pool.

	/* Method _flushPool:			Flushes data in the buffer pool
	*					to the output file.           */
	void _flushPool();

public:
	/* Method OutputFileStreamPipe: 	The constructor 	      */
	OutputFileStreamPipe( const char *fileName, 
			      int bytesInWritePool = DEFAULTwritePoolSize );

	/* Method ~OutputFileStreamPipe: 	The destructor 		      */
	~OutputFileStreamPipe();

	/* Method bytesFree: 			Returns the number of bytes 
	*					that can be written           */
	virtual int bytesFree() const;

	/* Method bytesReady: 			Returns the number of bytes 
	*					ready to read                 */
	virtual int bytesReady() const;

        /* Method createPipeReader:     	Not implemented for class     */
        virtual PipeReader * createPipeReader() const {
                shouldNotImplement( "createPipeReader()" );
                return NULL;
                } ;

	/* Method get: 				It is an error to read from 
	*					an output file 		      */
	virtual int get( char *data, int lengthRequested );

        /* Method getByteOffset:        	Returns the current byte 
	*					offset in the file associated 
	*					with this pipe.               */
	virtual off_t getByteOffset();

        /* Method getFileSize:        		Returns the length of the 
	*					file. Does not change the 
	*					current file pointer.        */
	virtual off_t getFileSize();

	/* Method getSkippingPeek: 	        It is an error to read from 
	*					an output file 		      */
	virtual int getSkippingPeek( char *data, int lengthRequested );

	/* Method isEmpty: 			Returns TRUE_ if pipe empty   */
	virtual Boolean_ isEmpty() const;

	/* Method isFull: 			Returns TRUE_ if pipe is full */
	virtual Boolean_ isFull() const;

        /* Method isOutputPipe:           	Always returns TRUE_          */
	virtual Boolean_ isOutputPipe() const { 
		return TRUE_; 
		} ;

	/* Method peek: 			It is an error to peek into 
	*					an output file 		      */
	virtual int peek( char *data, int lengthRequested );

	/* Method put: 				Write data to the buffer,
	*					flushing to the file when the
	*					buffer fills up.              */
	virtual int put( const char *data, int length );

        /* Method setByteOffset:        	Sets the current byte offset 
	*					in the file associated with 
	*					this pipe.                    */
        virtual void setByteOffset( off_t newByteOffset );

	/* Method truncateAfterByte:            Truncates the data in the file
	*					associated with this pipe after
	*					the specified byte, leaving
	*					the seek pointer at the end of
	*					the file.	              */
	virtual void truncateAfterByte( off_t byteOffset );

	/* Method printOn: 			Stream output function        */
	virtual void printOn( ostream& os ) const;

};

inline ostream & operator<<( ostream& os, OutputFileStreamPipe& p )
{
	p.printOn( os );
	return os;
}

#endif


