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
 * Contributing Author: Daniel A. Reed (reed@cs.uiuc.edu)
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
 * FieldDescriptor.h: Basic class used to describe a field of data in the 
 *		      self-documenting data format.
 *
 *	$Header: /home/cs/carr/cvsroot/DSystem/lib/pablo/include/FieldDescriptor.h,v 1.1 1999/09/08 17:45:13 carr Exp $
 */

#ifndef FieldDescriptor_h
#define FieldDescriptor_h

#include "BaseDescriptor.h"
#include "DataTraits.h"
#include "Defines.h"

class DataCharacteristics;

class FieldDescriptor : public BaseDescriptor {
/*
*
* A FieldDescriptor is the base component of a self documenting data
* format.  It defines a one field of data.
*
*/

private:
	static const char *const MY_CLASS;	// My class name

protected:
	DataTraits	traits;

public:
	/* Method FieldDescriptor:	A FieldDescriptor constructor with
	*				no initialization		*/
	FieldDescriptor();

	/* Method FieldDescriptor:	A FieldDescriptor constructor with
	*				initial value for name		*/
	FieldDescriptor( const CString& _name );

	/* Method FieldDescriptor:	A FieldDescriptor constructor with
	*				initial values for all elements */
	FieldDescriptor( const CString& _name, 
			 const Attributes& _attributes,
			 const MachineDataType _type,
		 	 const int _dimension );

	/* Method FieldDescriptor:	FieldDescriptor constructor with
	*			 	copy initialization		*/
	FieldDescriptor( const FieldDescriptor& field );

  	/* Method bitsToObj:    	Method used to translate from binary
        *                       	bitstream representation of data to 
	*				FieldDescriptor instance. Returns 
	*				number of bytes read or -1 if some 
	*				error occured. 			*/
        int bitsToObj( const char *bufPtr );

     	/* Method bytesNeeded:  	Returns the number of bytes needed
        *                       	to represent the instance as a 
	*				binary bitstream                */
        int bytesNeeded( ) const;

  	/* Method cbitsToObj:    	Method used to translate from 
	*				non-native binary bitstream 
	*				representation of data to 
	*				FieldDescriptor instance. Returns 
	*				number of bytes read or -1 if some 
	*				error occured. 			*/
        int cbitsToObj( const char *bufPtr,
			const DataCharacteristics *cnvInfo );

     	/* Method cbytesNeeded:  	Returns the number of bytes needed
        *                       	to represent the instance as a 
	*				non-native binary bitstream     */
        int cbytesNeeded( const DataCharacteristics *cnvInfo ) const;


	/* Method getDimension:		Returns the dimension of the 
	*				field (zero for scalars)	*/
	int getDimension() const;

	/* Method getTraits:		Returns the DataTraits for this
	*				field.				*/
	const DataTraits& getTraits() const;

 	/* Method getType:		Returns the builtin data type of
	*				this field (e.g., int)		*/
	MachineDataType getType() const;

 	/* Method objToBits:		Used to translate instance to binary
        *                       	bitstream representation of data. 
	*				Returns number of bytes used to write 
	*				bitstream data, or -1 if bufLen < number
	*				of bytes needed. 		*/
        int objToBits( char* const bufPtr, int bufLen ) const;

 	/* Method objToCbits:		Used to translate instance to non-native
	*				binary bitstream representation of data.
	*				Returns number of bytes used to write 
	*				bitstream data, or -1 if bufLen < number
	*				of bytes needed. 		*/
        int objToCbits( char* const bufPtr, int bufLen,
			const DataCharacteristics *cnvInfo ) const;

	/* Method setDimension:		Set the dimension of the field 
	*				(zero for scalars)	        */
	void setDimension( const int fieldDimension );

	/* Method setTraits:		Set the DataTraits for field    */
	void setTraits( const DataTraits& _traits );

	/* Method setType:		Set the builtin data type of
	*				this field (e.g., int)		*/
	void setType( const MachineDataType _type );

	/* Method operator=:		FieldDescriptor assigment	*/
	FieldDescriptor& operator=( const FieldDescriptor& field );

	/* Method operator==:		FieldDescriptor equivalence	*/
	Boolean_ operator==( const FieldDescriptor& field ) const;

	/* Method operator!=:		FieldDescriptor non-equivalence	*/
	Boolean_ operator!=( const FieldDescriptor& field ) const;

	/* Method printOn: 		Helper function for FieldDescriptor
	*				output. 			*/ 	
	virtual void printOn( ostream& strm = cout ) const;
};

#endif FieldDescriptor_h
