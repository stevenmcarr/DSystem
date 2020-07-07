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
 * Author: Ruth A. Aydt (aydt.cs.uiuc.edu)
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
 * StructureDescriptor.h: Class used to describe a structure made up of one
 *		          or more fields of data in the self-documenting data 
 *			  format.
 *
 *	$Header: /home/cs/carr/cvsroot/DSystem/lib/pablo/include/StructureDescriptor.h,v 1.1 1999/09/08 17:45:13 carr Exp $
 */

#ifndef StructureDescriptor_h
#define StructureDescriptor_h

#include <iostream>
using namespace std;

#include "BaseDescriptor.h"
#include "Defines.h"
#include "FieldDescriptor.h"

class DataCharacteristics;

class StructureDescriptor : public BaseDescriptor {
/*
*
* A StructureDescriptor defines the smallest unit of data which can be
* passed through the system.  Each StructureDescriptor groups one or more
* FieldDescriptors into a logical element of the self-documenting data format.
* Within a StructureDescriptor instance, each field.name must be unique.
*
*/
	friend class StructureDescriptorIterator;

	class StructureEntry {
		friend class StructureDescriptor;
		friend class StructureDescriptorIterator;
	private:
	    FieldDescriptor	field;
	    StructureEntry      *nextEntry;

	    StructureEntry()    { nextEntry = NULL; }
	} ;

private:
	static const char *const MY_CLASS;	// My class name
	static FieldDescriptor 	noField;

	StructureEntry	        *headEntry;
	StructureEntry		*tailEntry;

public:
	/* Method StructureDescriptor:	A StructureDescriptor constructor with
	*				no initialization		      */
	StructureDescriptor();

	/* Method StructureDescriptor:	A StructureDescriptor constructor with
	*				initial value for name 		      */
	StructureDescriptor( const CString& structureName );

	/* Method StructureDescriptor:	A StructureDescriptor constructor with
	*				initial values for name and 
	*				attributes 			      */
	StructureDescriptor( const CString& structureName, 
			     const Attributes& structureAttributes );

	/* Method StructureDescriptor:	StructureDescriptor constructor with
	*			 	copy initialization		      */
	StructureDescriptor( const StructureDescriptor& structureDescr );

	/* Method ~StructureDescriptor:	The StructureDescriptor destructor    */
	 ~StructureDescriptor();

        /* Method bitsToObj:    	Translates from binary bitstream
        *                       	representation of data to 
        *                       	StructureDescriptor instance. Any 
	*				fields in the instance when the call is
	*				made are deleted.  Returns number of 
	*				bytes read or -1 if error occured.    */
        int bitsToObj( const char *bufPtr );

        /* Method bytesNeeded:  	Returns the number of bytes needed
        *                       	to represent instance as a binary 
	*				bitstream.               	      */
        int bytesNeeded( ) const;

        /* Method cbitsToObj:    	Translates from non-native binary 
        *                       	bitstream representation of data to 
        *                       	StructureDescriptor instance. Any 
	*				fields in the instance when the call is
	*				made are deleted.  Returns number of 
	*				bytes read or -1 if error occured.    */
        int cbitsToObj( const char *bufPtr, 
			const DataCharacteristics *cnvInfo );

        /* Method cbytesNeeded:  	Returns the number of bytes needed
        *                       	to represent instance as a non-native
	*				binary bitstream.            	      */
        int cbytesNeeded( const DataCharacteristics *cnvInfo ) const;

	/* Method contains:		Field membership test		      */
	Boolean_ contains( const CString& fieldName ) const;

	/* Method entryCount:		Returns the number of field entries
	*				in this StructureDescriptor           */
	int entryCount() const;

	/* Method fetch:		Lookup the named field and return the
	*				FieldDescriptor of that name, or a
	*				NOMATCH FieldDescriptor if no match
	*				was found.			      */
	const FieldDescriptor& fetch( const CString& fieldName ) const;

	/* Method insert:		Insert a FieldDescriptor into the
	*				StructureDescriptor. Returns SUCCESS or
	*				FAILURE (if duplicate field.name)     */
	Boolean_ insert( const FieldDescriptor& fieldDescr );

	/* Method isMatchFor:		Checks to see if structureDescr matches
	*				this StructureDescriptor at the level
	*				specified by matchOn {Name, Attributes,
	*				Fields}				      */
	Boolean_ isMatchFor( const StructureDescriptor& structureDescr,
			     Boolean_ matchOnName, 
			     Boolean_ matchOnAttibutes, 
			     Boolean_ matchOnFields ) const;

        /* Method objToBits:    	Translates StructureDescriptor instance
	*				to binary bitstream representation of 
	*				data. Returns number of bytes used to 
	*				write bitstream, or -1 if bufLen < 
	*				bytes needed to represent data.	      */
        int objToBits( char* const bufPtr, int bufLen ) const;

        /* Method objToCbits:    	Translates StructureDescriptor instance
	*				to non-native binary bitstream 
	*				representation of data. Returns number 
	*				of bytes used to write bitstream, or -1
	*				if bufLen < bytes needed to represent 
	*				data.	      			      */
        int objToCbits( char* const bufPtr, int bufLen, 
			const DataCharacteristics *cnvInfo ) const;

	/* Method remove:		Remove the named field from the 
	*				StructureDescriptor.		      */
	void remove( const CString& fieldName ); 

	/* Method operator=:		StructureDescriptor assigment	      */
	StructureDescriptor& operator=( const StructureDescriptor& 
							structureDescr);

	/* Method printOn: 		Helper function for StructureDescriptor
	*				output. 			      */
	virtual void printOn( ostream& strm = cout ) const;

};

#endif StructureDescriptor_h
