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
 * DataTraits.h: Header file for class used to define the characteristics
 *	    of data in the Pablo system.  Contains information on the base
 *	    MachineDataType and the dimesions of the data.
 *
 *	$Header: /home/cs/carr/cvsroot/DSystem/lib/pablo/include/DataTraits.h,v 1.1 1999/09/08 17:45:13 carr Exp $
 *	
 */

#ifndef DataTraits_h
#define DataTraits_h

#include <stdio.h>
#include <stream.h>

#include "Defines.h"

// Global typedef, variable, and routine relating to data types supported:
// If you change these, also modify the initizization of the atomSizeCache
// array in DataTraits.C !

enum MachineDataType {
	CHARACTER = 0,
	INTEGER = 1,
	FLOAT = 2,
	DOUBLE = 3,
	UNDEFINED = 4
};

static const char* typeNameArray[] = 
{
        "char",
        "int",
        "float",
        "double",
        "undefined"
};

// Common data dimension constants 
enum { SCALAR, VECTOR, ARRAY_2D, ARRAY_3D };

class DataTraits {
/*
*
* The DataTraits Class provides an interface to type and dimesion information
* about the data.
*
*/

protected:
	MachineDataType type;
	int		dimension;

public:
	static const int atomSizeCache[ UNDEFINED + 1 ];  // Array to cache
					// sizeof()"s for the MachineDataTypes

 	/* Method loadConfigurationFromFP: Load info from file pointer        */
 	static DataTraits * loadConfigurationFromFP( FILE *fp );
	
	/* Method DataTraits:		Constructor with type and dimension 
	*				specified (with defaults of type =
	*				UNDEFINED and dimension = 0)	      */
	DataTraits( MachineDataType _type = UNDEFINED, int _dimension = 0 );

	/* Method DataTraits:		Constructor with copy initialization  */
	DataTraits( const DataTraits& _traits );

	/* Method clear:		Set type to UNDEFINED and dimension
	*				to zero.                              */
	void clear();

	/* Method getAtomSize:		Returns sizeof() atom of this type    */
	int getAtomSize() const;

	/* Method getDimension:		Returns the dimension                 */
	int getDimension() const;

	/* Method getType:		Returns the type                      */
	MachineDataType getType() const;

	/* Method getTypeName:		Returns the name of the type          */
	const char * getTypeName() const;

	/* Method isArray:		Returns TRUE_ if dimension > 0, 
	*				otherwise returns FALSE_	      */
	Boolean_ isArray() const;

	/* Method isCString:		Returns TRUE_ if dimension == 1 and
	*				type == CHARACTER, otherwise returns
	*				FALSE_				      */
	Boolean_ isCString() const;

	/* Method isDefined:		Returns TRUE_ if type is not 
	*				UNDEFINED			      */
	Boolean_ isDefined() const;

	/* Method isScalar:		Returns TRUE_ if dimension == 0,
	*				otherwise returns FALSE_	      */
	Boolean_ isScalar() const;

	/* Method isUndefined:		Returns TRUE_ if type is UNDEFINED    */
	Boolean_ isUndefined() const;

	/* Method saveConfigurationToFP: Save info to file pointer            */
	Boolean_ saveConfigurationToFP( FILE *fp ) const;

	/* Method setDimension:		Set the dimension 		      */
	void setDimension( int _dimension );	  

	/* Method setTraits:		Set both dimension and type           */
	void setTraits( MachineDataType _type, int _dimension );

	/* Method setType:		Set the type       		      */
	void setType( MachineDataType _type );

	/* Method operator=:		Assignment operator                   */
	DataTraits& operator=( const DataTraits& _traits );

	/* Method operator==: 		Equality operator 		      */
	Boolean_ operator==( const DataTraits& _traits ) const;

	/* Method operator!=:		Inequality operator		      */
	Boolean_ operator!=( const DataTraits& _traits ) const;
						   
	/* Method printOn:		Helper function for output	      */
	void printOn( ostream& strm ) const;

};

inline void
DataTraits::clear() 
{
	type = UNDEFINED;
	dimension = 0;
}

inline int
DataTraits::getAtomSize() const
{
	return atomSizeCache[ type ];
}

inline int 
DataTraits::getDimension() const
{ 
	return dimension; 
}

inline MachineDataType 
DataTraits::getType() const
{ 
	return type;
}

inline const char *
DataTraits::getTypeName() const
{ 
	return typeNameArray[type];
}

inline Boolean_ 
DataTraits::isArray() const 
{ 
    	return ( (dimension != 0) ? TRUE_ : FALSE_ );
}

inline Boolean_ 
DataTraits::isCString() const 
{ 
    	return ( ((dimension == 1) && (type==CHARACTER) ) ? TRUE_ : FALSE_ );
}

inline Boolean_
DataTraits::isDefined() const
{
	return ( type==UNDEFINED ? FALSE_ : TRUE_ );
}

inline Boolean_ 
DataTraits::isScalar() const 
{ 
    	return ( (dimension == 0) ? TRUE_ : FALSE_ );
}

inline Boolean_
DataTraits::isUndefined() const
{
        return ( (type == UNDEFINED) ? TRUE_ : FALSE_ );
}

inline void
DataTraits::setDimension( int _dimension )	  
{ 
	dimension = _dimension; 
}

inline void
DataTraits::setTraits( MachineDataType _type, int _dimension )
{
	type = _type;
	dimension = _dimension;
}

inline void
DataTraits::setType( MachineDataType _type ) 
{ 
	type = _type; 
}

inline DataTraits&
DataTraits::operator= ( const DataTraits& _traits )
{
	type = _traits.type;
	dimension = _traits.dimension;
	return *this;
}

inline ostream& operator<<( ostream& os, DataTraits& traits )
{
	traits.printOn( os );
	return os;
}

inline int atomSize( const MachineDataType& type )
{
	return ( DataTraits::atomSizeCache[ type ] );
}

#endif DataTraits_h
