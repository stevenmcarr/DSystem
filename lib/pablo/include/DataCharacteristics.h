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
 * DataCharacteristics.h: Class used to define the characteristics
 *	    of data represented in Binary SDDF format.  Such things as
 *	    byte ordering and size of different data types are encapsulated
 *	    here, along with methods to manipulate between native machine
 *	    representation and a non-native format.
 *
 *	$Header: /home/cs/carr/cvsroot/DSystem/lib/pablo/include/DataCharacteristics.h,v 1.1 1999/09/08 17:45:13 carr Exp $
 *	
 */

#ifndef DataCharacteristics_h
#define DataCharacteristics_h

#include "Defines.h"
#include "DataTraits.h"
#include "Obj.h"

class InputFileStreamPipe;

/*
 * These are the currently supported values for the various fields
 * in the SDDF binary header that defines characteristics for data
 * characteristics on a given machine.
 */

enum Sddf_BYTE_ORDERING	{ Sddf_BIG_ENDIAN, 
			  Sddf_LITTLE_ENDIAN 
			};

/* Only TWOS_COMPLEMENT currently supported */
enum Sddf_INT_REPRESENT	{ Sddf_TWOS_COMPLEMENT, 
			  Sddf_ONES_COMPLEMENT, 
			  Sddf_SIGN_MAGNITUDE 
			};

/* Only IEEE_S currently supported */
enum Sddf_SNGLP_REPRESENT { Sddf_IEEE_S,
			    Sddf_VAX_S,
			    Sddf_IBM_S,
			    Sddf_CRAY_S
			  };

/* Only IEEE_D currently supported */
enum Sddf_DBLP_REPRESENT { Sddf_IEEE_D, 
			   Sddf_VAX_D,
			   Sddf_IBM_D,
			   Sddf_CRAY_D
			 };

/* Currently not used */
enum Sddf_EXTRA_REPRESENT { Sddf_IEEE_E, 
			    Sddf_VAX_E,
			    Sddf_IBM_E,
			    Sddf_CRAY_E
			  };

/* Only ASCII currently supported */
enum Sddf_CHAR_REPRESENT { Sddf_ASCII, 
			   Sddf_EBCDIC 
			 };

class DataCharacteristics : public Obj {
/*
*
* The DataCharacteristics Class knows about the various representations of
* data types for different machines.  It reads and writes headers to binary
* SDDF files and is used to convert between other representations and the
* representations on the native machine.
*
*/
private:
	static const char *const MY_CLASS;		// My class Name

	enum { Sddf_BytesInHeader = 19 };

// These static variables are used to keep track of the representations on 
// our Native machine.
	static Sddf_BYTE_ORDERING	NativeByteOrdering;
	static Sddf_INT_REPRESENT	NativeIntegerRepresentation;
	static Sddf_SNGLP_REPRESENT	NativeSingleFloatRepresentation;
	static Sddf_DBLP_REPRESENT	NativeDoubleFloatRepresentation;
	static Sddf_EXTRA_REPRESENT	NativeExtraFloatRepresentation;
	static Sddf_CHAR_REPRESENT	NativeCharRepresentation;
	static int			NativeBytesInChar;
	static int			NativeBytesInShort;
	static int			NativeBytesInInteger;
	static int			NativeBytesInLong;
	static int			NativeBytesInFloat;
	static int			NativeBytesInDouble;
	static int			NativeBytesInLongDouble;


// Each of the following variables use a single byte in the header we read 
// from or write to an SDDF binary file.  These variables track the 
// representations of the data we are reading or writing.  The five bytes 
// "SDDFB" are found at the beginning of every SDDF binary file before these 
// characteristics appear.
	int			bytesInHeader;
	Sddf_BYTE_ORDERING	byteOrdering;
	Sddf_INT_REPRESENT	integerRepresentation;
	Sddf_SNGLP_REPRESENT	singleFloatRepresentation;
	Sddf_DBLP_REPRESENT	doubleFloatRepresentation;
	Sddf_EXTRA_REPRESENT	extraFloatRepresentation;
	Sddf_CHAR_REPRESENT	charRepresentation;
	int			bytesInChar;
	int			bytesInShort;
	int			bytesInInteger;
	int			bytesInLong;
	int			bytesInFloat;
	int			bytesInDouble;
	int			bytesInLongDouble;

	InputFileStreamPipe 	*inputPipe;

public:
	/* Method DataCharacteristics:   Constructor	 	              */
	DataCharacteristics();

	/* Method writeNativeHeader:	 Writes a SDDF binary header with the
	*				 representations native to the machine
	*				 where the program is being executed
	*				 into the buffer.  Returns the number
	*				 of bytes written or -1 if an error
	*				 occured.                             */
	static int writeNativeHeader( char *const bufPtr, int bufLen );

	/* Method functionGet:		 Gets appropriate number of bytes 
	*				 from *buffer into char and returns
	*				 the number of bytes read.            */
	int functionGet( const char *buffer, char *value ) const;

	/* Method functionGet:		 Gets appropriate number of bytes from 
	*				 **bufptr into char, advances *bufptr by
	*				 the number of bytes read, and returns
	*				 the number of bytes read.            */
	int functionGet( char **bufptr, char *value ) const;

	/* Method functionGet:		 Gets appropriate number of bytes 
	*				 from *buffer into int and returns
	*				 the number of bytes read.            */
	int functionGet( const char *buffer, int *value ) const;

	/* Method functionGet:		 Gets appropriate number of bytes from 
	*				 **bufptr into int, advances *bufptr by
	*				 the number of bytes read, and returns
	*				 the number of bytes read.            */
	int functionGet( char **bufptr, int *value ) const;

	/* Method functionGet:		 Gets appropriate number of bytes 
	*				 from *buffer into double and returns
	*				 the number of bytes read.            */
	int functionGet( const char *buffer, double *value ) const;

	/* Method functionGet:		 Gets appropriate number of bytes from 
	*				 **bufptr into double, advances *bufptr
	*				 by the number of bytes read, and 
	*				 returns the number of bytes read.    */
	int functionGet( char **bufptr, double *value ) const;

	/* Method functionGet:		 Gets appropriate number of bytes 
	*				 from *buffer into float and returns
	*				 the number of bytes read.            */
	int functionGet( const char *buffer, float *value ) const;

	/* Method functionGet:		 Gets appropriate number of bytes from 
	*				 **bufptr into float, advances *bufptr 
	*				 by the number of bytes read, and 
	*				 returns the number of bytes read.    */
	int functionGet( char **bufptr, float *value ) const;

	/* Method functionPut:		 Writes the char value as bytes in
	*			         *buffer and returns the number of 
	*				 bytes written 	 	              */
	int functionPut( char *buffer, const char *value ) const;

	/* Method functionPut:		 Writes the char value as bytes in
	*			         **bufptr, advances *bufptr by the 
	*				 number of bytes written and returns 
	*				 the number of bytes written  	      */
	int functionPut( char **bufptr, const char *value ) const;

	/* Method functionPut:		 Writes the int value as bytes in
	*			         *buffer and  returns the number of 
	*				 bytes written 			      */
	int functionPut( char *buffer, const int *value ) const;

	/* Method functionPut:		 Writes the int value as bytes in
	*			         **bufptr, advances *bufptr by the 
	*				 number of bytes written and returns 
	*				 the number of bytes written  	      */
	int functionPut( char **bufptr, const int *value ) const;

	/* Method functionPut:		 Writes the double value as bytes in
	*			         *buffer and returns the number of 
	*				 bytes written 			      */
	int functionPut( char *buffer, const double *value ) const;

	/* Method functionPut:		 Writes the double value as bytes in
	*			         **bufptr, advances *bufptr by the 
	*				 number of bytes written and returns 
	*				 the number of bytes written          */
	int functionPut( char **bufptr, const double *value ) const;

	/* Method functionPut:		 Writes the float value as bytes in
	*			         *buffer and returns the number of 
	*				 bytes written 			      */
	int functionPut( char *buffer, const float *value ) const;

	/* Method functionPut:		 Writes the float value as bytes in
	*			         **bufptr, advances *bufptr by the 
	*				 number of bytes written and returns 
	*				 the number of bytes written	      */
	int functionPut( char **bufptr, const float *value ) const;

	/* Method getAtomSize:		 Returns the size of the given
	*				 MachineDataType on the non-native
	*				 machine.                             */
	int getAtomSize( MachineDataType atomType ) const;

	/* Method initializeFromFile:	 Reads the header information from the
	*				 input file and sets the class variables
	*				 using that information.  Returns
	*				 FALSE_ if for some reason it's unable
	*				 to parse the input or if the needed
	*				 conversion routines don't exist.     */
	Boolean_ initializeFromFile( InputFileStreamPipe * pipe, 
				     int fileDescr );

	/* Method intSize:		 Returns the size of an integer       */
	int intSize() const; 

	/* Method notNative:		 Returns TRUE_ if the representations
	*				 don't match those of the native 
	*				 machine                              */
	Boolean_ notNative() const;

	/* Method setByteOrdering:	 Sets byte ordering    		      */
	Boolean_ setByteOrdering( Sddf_BYTE_ORDERING newByteOrdering );

	/* Method setByteOrderingNonNative:	Sets byte ordering to be the
	*				 opposite of what it is on the native
	*				 machine.                             */
	Boolean_ setByteOrderingNonNative( );

	/* Method setIntegerRepresentation:	Sets integer representation   */
	Boolean_ setIntegerRepresentation( 
			Sddf_INT_REPRESENT newIntegerRepresentation );

	/* Method setSingleFloatRepresentation:	Sets float representation     */
	Boolean_ setSingleFloatRepresentation(
			Sddf_SNGLP_REPRESENT newSingleFloatRepresentation );

	/* Method setDoubleFloatRepresentation:	Sets double representation    */
	Boolean_ setDoubleFloatRepresentation(
			Sddf_DBLP_REPRESENT newDoubleFloatRepresentation );

	/* Method setExtraFloatRepresentation:	Sets long double representation
	*				 (Currently this extra precision
	*				 floating point is not supported in 
	*				 SDDF)                                */
	Boolean_ setExtraFloatRepresentation(
			Sddf_EXTRA_REPRESENT newExtraFloatRepresentation );

	/* Method setCharRepresentation: Sets character representation        */
	Boolean_ setCharRepresentation(
			Sddf_CHAR_REPRESENT newCharRepresentation );

	/* Method setBytesInChar:	 Sets number of bytes in char         */
	Boolean_ setBytesInChar( int newBytesInChar );

	/* Method setBytesInShort:	 Sets number of bytes in short 
	*				 (Currently this type is not supported 
	*				 in SDDF)                             */
	Boolean_ setBytesInShort( int newBytesInShort );

	/* Method setBytesInInteger:	 Sets number of bytes in int          */
	Boolean_ setBytesInInteger( int newBytesInInteger );

	/* Method setBytesInLong:	 Sets number of bytes in long 
	*				 (Currently this type is not supported 
	*				 in SDDF)                             */
	Boolean_ setBytesInLong( int newBytesInLong );

	/* Method setBytesInFloat:	 Sets number of bytes in float        */
	Boolean_ setBytesInFloat( int newBytesInFloat );

	/* Method setBytesInDouble:	 Sets number of bytes in double       */
	Boolean_ setBytesInDouble( int newBytesInDouble );

	/* Method setBytesInLongDouble:	 Sets number of bytes in long double   
	*				 (Currently this type is not supported
	*				 in SDDF)		              */
	Boolean_ setBytesInLongDouble( int newBytesInLongDouble );

	/* Method writeHeader:		 Writes a SDDF binary header with the
	*				 characteristics for the instance
	*				 of the class into the buffer.  
	*				 Returns the number of bytes written 
	*				 or -1 if an error occured.           */
	int writeHeader( char *const bufPtr, int bufLen ) const;

};

inline int
DataCharacteristics::intSize() const
{
	return bytesInInteger;
}

#endif DataCharacteristics_h
