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
 * Author: Daniel A. Reed (reed@cs.uiuc.edu)
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
 * Attributes.h: Linked list implementation class for key/value pairs.
 *
 *	$Header: /home/cs/carr/cvsroot/DSystem/lib/pablo/include/Attributes.h,v 1.1 1999/09/08 17:45:13 carr Exp $
 */

#ifndef Attributes_h
#define Attributes_h

#include "CString.h"
#include "Obj.h"

class DataCharacteristics;

class Attributes : public Obj {
/*
*
* Attributes is the implementation class for associative data
* structures (i.e., item retrieval by key rather than index).
*
*/
	friend class AttributesIterator;

	class AttributeEntry {
	   friend class Attributes;
	   friend class AttributesIterator;
	private:
	   CString	   key;
	   CString	   value;
	   AttributeEntry  *nextEntry;

	   AttributeEntry() { nextEntry = NULL; }
	} ;

private:
	AttributeEntry     *headEntry;
	AttributeEntry     *tailEntry;

public:
	/* Method Attributes:	The simplest Attributes constructor	      */
	Attributes();

	/* Method Attributes:	Attributes constructor with copy
	*			initialization				      */
	Attributes( const Attributes& attributes );

	/* Method ~Attributes:	The Attributes destructor		      */
	 ~Attributes();

        /* Method bitsToObj:    Method used to translate from binary bitstream
        *                       representation of data to Attributes instance. 
	*			Any key/value pairs existing in the Attributes
	*			list when the call is made are removed.
	*			Returns number of bytes read or -1 if some 
	*			error occured.                      	      */
        int bitsToObj( const char *bufPtr );

        /* Method bytesNeeded:  Method which returns the number of bytes needed
        *                       to represent the Attributes Object instance
        *                       as a binary bitstream.                        */
        int bytesNeeded() const;

        /* Method cbitsToObj:   Method used to translate from non-native
	*			binary bitstream representation of data to 
	*			Attributes instance. Any key/value pairs 
	*			existing in the Attributes list when the 
	*			call is made are removed.
	*			Returns number of bytes read or -1 if some 
	*			error occured.                      	      */
        int cbitsToObj( const char *bufPtr,
			const DataCharacteristics *cnvInfo );

        /* Method cbytesNeeded: Method which returns the number of bytes needed
        *                       to represent the Attributes Object instance
        *                       as a non-native binary bitstream.             */
        int cbytesNeeded( const DataCharacteristics *cnvInfo ) const;

	/* Method clearEntries:	Method to remove all key/value entries from 
	*			the Attributes list			      */
	void clearEntries();

	/* Method contains:	Attributes membership test		      */
	Boolean_ contains( const CString& key ) const;

	/* Method entryCount:	Number of key/value entries		      */
	int entryCount() const;

	/* Method fetch:	Value extraction based on key.  Returns 
	*			value or NOMATCH if no match.                 */
	const CString& fetch( const CString& key ) const;

	/* Method insert:	Add key/value pair to list. Returns
	*			SUCCESS_ or FAILURE_ (if duplicate key).      */
	Boolean_ insert( const CString& key, const CString& value );

        /* Method objToBits:    Method used to translate internal Attributes
        *                       instance to bitstream representation of data. 
	*			Returns number of bytes used to write bitstream
	* 			data, or -1 if bufLen was less than the number 
	*			of bytes needed to represent data in binary 
	*			bitstream format.                   	      */
        int objToBits( char *const bufPtr, int bufLen ) const;

        /* Method objToCbits:   Method used to translate internal Attributes
        *                       instance to non-native bitstream representation
	*			of data. Returns number of bytes used to 
	*			write bitstream data, or -1 if bufLen was 
	*			less than the number of bytes needed to 
	*			represent data in bitstream format.   	      */
        int objToCbits( char *const bufPtr, int bufLen,
			const DataCharacteristics *cnvInfo ) const;

	/* Method remove:	Remove key/value pair from list		      */
	void remove( const CString& key );

	/* Method operator[]:	Attributes extraction based on key	      */
	const CString& operator[]( const CString& key ) const; 

	/* Method operator=:	Attributes assignment (with copies)	      */
	Attributes& operator=( const Attributes& attributes );

	/* Method operator==:	Attributes equivalence test. Attributes are
	*		 	considered equivalent if they have identical
	*			key/value pairs appearing in the Attributes 
	*		 	list IN THE SAME ORDER			      */
	Boolean_ operator==( const Attributes& attributes ) const;

	/* Method operator!=:	Attributes non-equivalence test               */
	Boolean_ operator!=( const Attributes& attributes ) const;

	/* Method printOn: 	helper function for Attributes output	      */
	virtual void printOn( ostream& strm = cout ) const;
};

inline ostream& operator<<( ostream& os, Attributes& attributes )
{
	attributes.printOn (os);

	return os;
}

#endif Attributes_h
