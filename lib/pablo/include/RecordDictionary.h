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
 * RecordDictionary.h: Class used to keep track of the record dossiers
 *		       that are valid on a given input or output port.
 *
 *	$Header: /home/cs/carr/cvsroot/DSystem/lib/pablo/include/RecordDictionary.h,v 1.1 1999/09/08 17:45:13 carr Exp $
 */

#ifndef RecordDictionary_h
#define RecordDictionary_h

#include "Defines.h"
#include "Obj.h"
#include "RecordDossier.h"
#include "StructureDescriptor.h"

class RecordDictionary : public Obj {
/*
* There is a single instance of the RecordDictionary class for each input or
* output port.  It is used to maintain a list of the RecordDossiers 
* encountered by the system at that point in the processing.
*/
	friend class RecordDictionaryIterator;
	friend class RecordDictionaryPIterator;

	class RecordDictEntry {
	    friend class RecordDictionary;
	    friend class RecordDictionaryIterator;
	    friend class RecordDictionaryPIterator;

	private:
	    RecordDossier	dossier;
	    RecordDictEntry	*nextEntry;

	    RecordDictEntry( int tag, const StructureDescriptor& structDescr ) 
		: dossier( tag, structDescr )
		{ nextEntry = NULL; }
	} ;

private:
	static const char *const MY_CLASS;	// My class name
	static RecordDossier   	noDossier; 

	RecordDictEntry		*headEntry;
	RecordDictEntry		*tailEntry;

public:
	/* Method RecordDictionary:	The RecordDictionary constructor with
	*				no initialization		      */
	RecordDictionary();

	/* Method ~RecordDictionary:	The RecordDictionary destructor       */
	 ~RecordDictionary();

	/* Method contains:		Membership test on match with tag     */
	Boolean_ contains( int dossierTag ) const;
	 
	/* Method entryCount:           Returns the number of dossiers in 
	*                               the dictionary.                       */
	int entryCount() const;

	/* Method fetch:		Lookup the named tag and return the
	*				RecordDossier with that systemTag
	*				or NOMATCH RecordDossier if no 
	*				match was found.		      */
	RecordDossier& fetch( int dossierTag ) const;

	/* Method getArrayP:		Get pointer to array of named field in 
	 *				RecordDossier with specified tag. If 
	 *				value is not of type array, then NULL
	 *				pointer returned.                     */
	Array * getArrayP( int dossierTag, const CString& fieldName ) const;

	/* Method getArrayP:		Get pointer to array of field with 
	 *				given ID in RecordDossier with 
	 *				specified tag.  If value is not of type
	 *				array, then NULL is returned.         */
	Array * getArrayP( int dossierTag, int fieldID ) const;


        /* Method getCString:           Get string value of named field in
	 *				Record Dossier with specified tag. 
         *                              If field is not type = CHARACTER with
         *                              dimension = 1 then CString::NOMATCH
         *                              is returned                           */
        CString& getCString( int dossierTag, const CString& fieldName ) const; 

        /* Method getCString:           Get string value field with given ID
	 *				in Record Dossier with specified tag.
         *                              If field is not type = CHARACTER with
         *                              dimension = 1 then CString::NOMATCH
         *                              is returned */
        CString& getCString( int dossierTag, int fieldID ) const;
  
	/* Method getValue:		Get ref to value of named field in 
	 *				RecordDossier with specified tag      */
	Value& getValue( int dossierTag, const CString& fieldName ) const;

	/* Method getValue:		Get ref to value of field with given 
	 *				ID in RecordDossier with specified 
	 *				tag      			      */
	Value& getValue( int dossierTag, int fieldID ) const;

	/* Method getValueP:		Get pointer to value of named field in 
	 *				RecordDossier with specified tag      */
	Value * getValueP( int dossierTag, const CString& fieldName ) const;

	/* Method getValueP:		Get pointer to value of field with 
	 *				given ID in RecordDossier with 
	 *				specified tag      		      */
	Value * getValueP( int dossierTag, int fieldID ) const;

	/* Method insert:		Create a new RecordDossier in the
	 *				RecordDictionary. Returns Success or
	 *				Failure if this is a duplicate entry  */
	Boolean_ insert( int dossierTag, 
			 const StructureDescriptor& structDescr );

        /* Method setCString:           Set value of named field in Record
	 *				Dossier with given tag to the
         *                              cstring argument. Returns FAILURE_ 
         *                              if field not found or if the field
         *                              is not a CHARACTER of dimension 1.    */
        Boolean_ setCString( int dossierTag, 
			     const CString& fieldName, 
                             const CString& cstring ); 
  
        /* Method setCString:           Set value of field with named ID in
	 *				Record Dossier with the given tag to 
         *                              the cstring argument. Returns FAILURE_ 
         *                              if fieldID invalid or if the field
         *                              is not a CHARACTER of dimension 1.    */
        Boolean_ setCString( int dossierTag,
			     int fieldID, 
			     const CString& cstring ); 

	/* Method setValue:		Set value of named field in 
	 *				RecordDossier with specified tag.     */
	Boolean_ setValue( int dossierTag, 
			   const CString& fieldName,
			   const Value& fieldValue);

	/* Method setValue:		Set value of field with given ID in 
	 *				RecordDossier with specified tag      */
	Boolean_ setValue( int dossierTag, 
			   int fieldID, 
			   const Value& fieldValue );

	/* Method printOn: 		Helper function for RecordDictionary
	*				output. 			      */
	virtual void printOn( ostream& strm = cout ) const;

};

inline ostream& operator<<( ostream& os, RecordDictionary& structDict )
{
	structDict.printOn (os);

	return os;
}

#endif RecordDictionary_h
