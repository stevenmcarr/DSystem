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
 * RecordDossier.h: Class used to cache structure information gleaned from the
 *		    StructureDescriptor and provide space and methods for
 *		    associating data packets with their StructureDescriptors.
 *
 * $Header: /home/cs/carr/cvsroot/DSystem/lib/pablo/include/RecordDossier.h,v 1.1 1999/09/08 17:45:13 carr Exp $
 */

#ifndef RecordDossier_h
#define RecordDossier_h

#include "BaseDescriptor.h"
#include "Defines.h"
#include "FieldDescriptor.h"
#include "Value.h"

class DataCharacteristics;
class StructureDescriptor;

class RecordDossier : public BaseDescriptor {
/*
* A RecordDossier is used within the system to store and manipulate the
* actual data.  It caches the structure descriptor information and record
* data.  The record data fields will initially be empty. The RecordDossier
* actually contains redundant information that could be derived from elsewhere
* in the system.  It exists to simplify and speed up processing.
*/

	class DossierField {
	    friend class RecordDossier;

	private:
	    FieldDescriptor	*descriptor;
	    Value		*value;
	} ;

private:
	static const char *const MY_CLASS;	// My class name

	int		tag;
	int		fieldCount;
	DossierField* 	field;

public:
	static RecordDossier NODOSSIER; /* A guaranteed NOMATCH RecordDossier */

	/* Method RecordDossier:	This constructor takes one argument
	*				- the dossier name. It is used to 
	*				build a "dummy" RecordDossier with 
	*				tag = 0 and fieldCount = 0.	      */
	RecordDossier( const CString& dossierName );

        /* Method RecordDossier:        This constructor provides copy
        *                               semantics for RecordDossier objects.
        *                               That is, it creates a RecordDossier
        *                               which contains the same information
        *                               as its argument, but shares no
        *                               pointers.                             */
        RecordDossier( const RecordDossier& originalDossier );

	/* Method RecordDossier:	The constructor specifying a tag and
	*			 	StructureDescriptor. NOTE: This is 
	*				usually called by RecordDictionary::
	*				insert()       			      */
	RecordDossier( int sysTag, const StructureDescriptor& structureDescr );

	/* Method ~RecordDossier:	The RecordDossier destructor          */
	~RecordDossier();

	/* Method bitsToObj:		Set the field values using data in the 
	*				bitstream pointed to by dataPtr. Return
	*				the number of bytes that were needed to 
	*				set values or -1 if an error occured. */
	int bitsToObj( const char *dataPtr );

	/* Method bytesNeeded:		Bytes necessary to represent the field 
	*				values in binary bitstream format     */
	int bytesNeeded() const;

	/* Method cbitsToObj:		Set the field values using data in the 
	*				non-native bitstream pointed to by 
	*				dataPtr. Return	the number of bytes 
	*				that were needed to set values or -1 
	*				if an error occured.                  */
	int cbitsToObj( const char *dataPtr, 
			const DataCharacteristics *cnvInfo );

	/* Method cbytesNeeded:		Bytes necessary to represent the field 
	*				values in non-native binary bitstream 
	*				format     			      */
	int cbytesNeeded( const DataCharacteristics *cnvInfo ) const;

	/* Method entryCount:		Return the number of fields in the 
	*				Dossier				      */
	int entryCount() const;

	/* Method getArrayP:		Get pointer to Array of named field.
	*				Return NULL if value is not an array. */
	Array * getArrayP( const CString& fieldName ) const; 

	/* Method getArrayP:		Get pointer to array of field with 
	*				given ID. Return NULL if value is not
	*				an array			      */
	Array * getArrayP( int fieldID ) const;

	/* Method getField:		Get pointer to named FieldDescriptor  */
	FieldDescriptor * getField( const CString& fieldName ) const; 

	/* Method getField:		Get pointer to FieldDescriptor with
	*				given ID    			      */
	FieldDescriptor * getField( int fieldID ) const;

	/* Method getFieldID:		Return the ID of the field with the
	*				given name. Returns -1 if no match.   */
	int getFieldID( const CString& fieldName ) const;

	/* Method getTag:		Get the tag 			      */
	int getTag() const;

	/* Method getCString:		Get string value of named field.  If
	*				field is not type = CHARACTER with
	*				dimension = 1 then CString::NOMATCH
	*				is returned                           */
	CString& getCString( const CString& fieldName ) const; 

	/* Method getCString:		Get string value field with given ID.  
	*			        If field is not type = CHARACTER with
	*				dimension = 1 then CString::NOMATCH
	*				is returned */
	CString& getCString( int fieldID ) const;

	/* Method getValue:		Get value of named field	      */
	Value& getValue( const CString& fieldName ) const; 

	/* Method getValue:		Get value of field with given ID      */
	Value& getValue( int fieldID ) const;

	/* Method getValueP:		Get pointer to Value of named field   */
	Value * getValueP( const CString& fieldName ) const; 

	/* Method getValueP:		Get pointer to Value of field with 
	*				given ID      			      */
	Value * getValueP( int fieldID ) const;

	/* Method objToBits:		Use the field values to populate 
	*				the binary bitstream pointed to by 
	*				bufPtr. Return the number of bytes 
	*				actually used or -1 if bufLen was 
	*				too small to hold values.  	      */
	int objToBits( char *const bufPtr, int bufLen ) const;

	/* Method objToCbits:		Use the field values to populate 
	*				the non-native binary bitstream pointed
	*				to by bufPtr. Return the number of 
	*				bytes actually used or -1 if bufLen was 
	*				too small to hold values.  	      */
	int objToCbits( char *const bufPtr, int bufLen,
			const DataCharacteristics *cnvInfo ) const;

	/* Method setCString:		Set value of named field to the
	*				cstring argument. Returns FAILURE_ 
	*				if field not found or if the field
	*				is not a CHARACTER of dimension 1.    */
	Boolean_ setCString( const CString& fieldName, 
			     const CString& cstring ); 

	/* Method setCString:		Set value of field with named ID to the
	*				cstring argument. Returns FAILURE_ 
	*				if fieldID invalid or if the field
	*				is not a CHARACTER of dimension 1.    */
	Boolean_ setCString( int fieldID, const CString& cstring ); 

	/* Method setValue:		Set value of named field. Returns
	*   				FAILURE_ if field not found or if
	*				fieldValue.traits cannot sensibly be
	*				converted to match descriptor->traits */
	Boolean_ setValue( const CString& fieldName, const Value& fieldValue ); 

	/* Method setValue:		Set value of field with given ID. 
	*				Returns FAILURE_ if fieldID invalid or
	*				fieldValue.traits cannot sensibly be
	*				converted to match descriptor->traits */
	Boolean_ setValue( int fieldID, const Value& fieldValue );

	/* Method operator=:		Assigment with copying of data	      */
	RecordDossier& operator=( const RecordDossier& recordDossier );

	/* Method printOn: 		Helper function for RecordDossier
	*				output. 			      */
	virtual void printOn( ostream& strm = cout ) const;

};

inline int 
RecordDossier::entryCount() const
{
	return fieldCount;
}

inline int
RecordDossier::getTag() const
{
	return tag;
}

#endif RecordDossier_h
