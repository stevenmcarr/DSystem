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
 * Contributing Author: Daniel A. Reed (reed@cs.uiuc.edu)
 * Contributing Author: Brian K. Totty (totty@cs.uiuc.edu)
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
 * Array.h: Class used to represent and manipulate the primitive type for 
 *          variable-length, multidimensional, single-typed arrays.
 *
 *	$Header: /home/cs/carr/cvsroot/DSystem/lib/pablo/include/Array.h,v 1.1 1999/09/08 17:45:13 carr Exp $
 */

#ifndef Array_h
#define Array_h

#include "DataTraits.h"
#include "Obj.h"

class CString;
class DataCharacteristics; 
class Value;

typedef char *DATA_P;

class Array {
/*
* The Array Class is used to represent and manipulate data values
* in variable-length, multidimensional, single-typed arrays.  Once
* an Array instance is created the only way it's type or it's dimension
* can be changed is with the = operator.   IMPORTANT: Most of the error 
* checking is disabled unless ASSERT is defined -- since this class is 
* used heavily, this saves considerable overhead in production mode.  
*/

private:
	static Obj MSG_OBJ;            /* an object to handle our error msgs */

	MachineDataType type;
	int	 dimension;
	int 	 cellCount;
	Boolean_ useDelta;
	int	 deltaCount;
	int	 iterateCount;
	int	 deltaDataSize;
	int	 *deltaIndices;
	int 	 *cellsInDim;
	int	 *strides;
	DATA_P	 data;

//  We use DATA_P to point to our data area, even though the data may
//  contain characters, ints, floats, or doubles.  The pointer arithmetic
//  is all done as if we have char *"s.  We used to use void *, but newer
//  compilers don"t let us do arithmetic on that type and all the conversions
//  were a mess in the code.

public:
	/* Method Array:  	   Constructor with type, dimension, and the
	*			   potential use of array deltas specified. 
	*			   No space is allocated for the array cells  */
	Array( MachineDataType _type, int _dimension,
	       Boolean_ _useDelta = FALSE_ );

	/* Method Array:  	   Constructor with copy initialization	      */
	Array( const Array& array );

	/* Method Array:  	   Constructor with pointer to existing 
	*			   Array instance as an argument.  	      */
	Array( const Array *arrayP );

	/* Method ~Array: 	   The Destructor.			      */
	~Array();

	/* Method atomSize:	   Return the sizeof() for an atom of the
	*			   specified type.  			      */
	int atomSize( MachineDataType atomType ) const;

	/* Method bitsToObj:	   Use information in the binary bitstream to 
	*			   fill in number of cells in each dimension 
	*			   and cell values. Return the number of bytes 
	*			   read or -1 if an error occured.            */
	int bitsToObj( const char *bufPtr );

	/* Method bytesNeeded:	   Return number of bytes needed to represent
	*		           Array as a bitstream                       */
	int bytesNeeded() const;

	/* Method cbitsToObj:	   Use information in the non-native binary 
	*			   bitstream to fill in number of cells in each 
	*			   dimension and cell values. Return the 
	*			   number of bytes read or -1 if an error 
	*	                   occured.                                   */
	int cbitsToObj( const char *bufPtr,
			const DataCharacteristics *cnvInfo );

	/* Method cbytesNeeded:	   Return number of bytes needed to represent
	*		           Array as a non-native binary bitstream     */
	int cbytesNeeded( const DataCharacteristics *cnvInfo ) const;

	/* Method clearDelta:	   Sets the number of delta array elements
	*			   to zero				      */
	void clearDelta();

	/* Method getCellCount:	   Return total number of cells in array.     */
	int getCellCount() const;

	/* Method getCellString:   Return the character string stored in the 
	*			   cells beginning at the specified offset.
	*			   If the Array had dimension N, then there 
	*			   should be N-1 index arguments in the call.  
	*			   If this is not a CHARACTER array or if the 
	*			   index values are bogus, FAILURE_ is returned 
	*			   and cstring is set to an empty string.
	*			   If the array is not populated, cstring is 
	*			   set to an empty string regardless of the 
	*			   index variables and SUCCESS_ is returned.  */
	Boolean_ getCellString( CString& cstring, int dim0Index ... ) const;

	/* Method getCellValue:	   Return value of specified cell. Specialized
	*			   method for vectors to enhance performance. */
	Value getCellValue( int dim0Index ) const;

 	/* Method getCellValue:    Return value of specified cell. Specialized
	*			   method for 2-d arrays to enhance 
	*			   performance.				      */
        Value getCellValue( int dim0Index, int dim1Index ) const;

 	/* Method getCellValue:    Return value of specified cell. General
	*			   method for arrays with >2 dimensions.      */
        Value getCellValue( int dim0Index, int dim1Index, 
			    int dim2Index ... ) const;

	/* Method getCellValue:    Update value parameter with contents of 
	*			   specified cell. Special case for vectors.
	*			   Returns FAILURE_ if error such as index out-
	*			   of-bounds.				      */
        Boolean_ getCellValue( Value& value, int dim0Index ) const;

 	/* Method getCellValue:    Update value parameter with contents of
	*			   of specified cell. Special case for 2-d 
	*			   arrays.  Returns FAILURE_ if error such as
	*			   index out-of-bounds.                       */
        Boolean_ getCellValue( Value& value, int dim0Index, 
			       int dim1Index ) const;

 	/* Method getCellValue:    Update value parameter with contents of
	*			   specified cell. General case for >2-d
	*			   arrays.  Returns FAILURE_ if error such as
	*			   index out-of-bounds.                       */
        Boolean_ getCellValue( Value& value, int dim0Index, int dim1Index, 
			       int dim2Index ... ) const;

	/* Method getDeltaCount:   Return the number of changed cells in the
	*			   delta array				      */
	int getDeltaCount() const;

	/* Method getDeltaValue:   Returns both the next delta value and its
	*			   array indices			      */
	void getDeltaValue( Value& value, int* dimVector );

	/* Method getDimSizes: 	   Return pointer to integer array specifing
	*			   the number of cells in each dimension      */
	const int * getDimSizes() const;

	/* Method getDimension:	   Return the dimension of the array	      */
	int getDimension() const;

	/* Method getType:	   Return type of array values		      */
	MachineDataType getType() const;

	/* Method getTheCellValue: Return value of the cell, where index
	*			   is the absolute index into the array
	*			   rather than an index into each dimension.  */
	Value getTheCellValue( int index ) const;

	/* Method getTheCellValue: Update value parameter with the contents of
	*			   the cell, where index is the absolute index 
	*			   into the array rather than an index into 
	*			   each dimension. Return FAILURE_ if error 
	*			   such as index out-of-bounds.	              */
        Boolean_ getTheCellValue( Value& value, int index ) const;

	/* Method isDeltaArray:	   Returns TRUE_ if the array was constructed
	*			   to use deltas (i.e., shows changes)        */
	Boolean_ isDeltaArray() const;

	/* Method objToBits:	   Represent the dimension sizes and data
	*		           as bits in the buffer pointed to by bufPtr.
	*			   Return the number of bytes used or -1 if
	*			   the buffer was too small.		      */
	int objToBits( char *const bufPtr, int bufLen ) const;

	/* Method objToCbits:	   Represent the dimension sizes and data
	*		           as bits as a non-native binary bitstream
	*			   in the buffer pointed to by bufPtr.
	*			   Return the number of bytes used or -1 if
	*			   the buffer was too small.		      */
	int objToCbits( char *const bufPtr, int bufLen,
			const DataCharacteristics *cnvInfo ) const;

	/* Method resetIterateCount:  Resets the number of iterates fetched   */
	void resetIterateCount();

	/* Method resetDelta:	   Resets the number of changed delta 
	*			   elements to the specified value	      */
	void resetDelta( int countDelta );

	/* Method setCellString:   Set the cells beginning at the specified
	*			   offset to the characters in the string
	*			   argument.   If the Array had dimension N,
	*			   then there should be N-1 index arguments
	*			   in the call.  FAILURE_ is returned if
	*			   this is not a CHARACTER array, if the index
	*			   values are bogus, or if dimSizes have not 
	*			   yet been set.  If the string is longer than 
	*			   those supported by the array - whatever
	*			   dimSizes[N-1] equals - then the string is
	*			   truncated.				      */
	Boolean_ setCellString( const CString& cstring, int dim0Index ... );

	/* Method setCellValue:	   Set specified cell to given value.  The
	*			   type of the array will NOT change - the
	*		           value will be recast as needed.  Returns
	*			   FAILURE_ if index out of bounds or if
	*			   dimension sizes of array not yet set.      */
	Boolean_ setCellValue( const Value& value, int dim0Index ... );

	/* Method setCellValues:   Set all cells without changing the current
	*			   array type.  Data are recast as needed.
	*			   FAILURE_ is returned if the array has not
	*			   yet been sized, or if the new array had 
	*			   different dimension or dimension sizes than
	*			   this array.		                      */
	Boolean_ setCellValues( const Array * array );

	/* Method setDimSizes:     Set number of cells in each dimension from 
	*			   the array of integers pointed to by intArray.
	*			   The data area is resized and initialized 
	*			   to 0.		                      */
	void setDimSizes( const int *intArray );

	/* Method setTheCellValue: Set cell to given value, where the absolute
	*			   index into the array is given rather than
	*			   an index for each dimension. The type of the
	*			   array will NOT change - the value will be 
	*			   recast if needed.  Returns FAILURE_ if 
	*			   index out of bounds or if dimension sizes 
	*			   of array not yet set.                      */
	Boolean_ setTheCellValue( int index, const Value& value );

	/* Method setDeltaIndex:   Updates the list of deltas to indicate
	*			   that the value at the specified index
	*			   has been changed			      */
	void setDeltaIndex( int deltaIndex );

	/* Method operator=:	   Array to Array assignment.  Upon completion,
	*			   both Array instances have copies of the 
	*			   data.  If this Array has different type, 
	*			   dimension, or dimension sizes than the new, 
	*			   we release the old space and alloc the new.
	*			   Else, we reuse the space.                  */
	Array& operator=( const Array& array );

	/* Method printOn:	   Helper function for Array output           */
	void printOn( ostream& strm = cout ) const;

};

inline int
Array::atomSize( MachineDataType atomType ) const
{
	return DataTraits::atomSizeCache[ atomType ];
}

inline void
Array::clearDelta()
{
	deltaCount = 0;
}

inline int 
Array::getCellCount() const
{ 
	return cellCount; 
}

inline int
Array::getDeltaCount() const
{
	return deltaCount;
}

inline const int* 
Array::getDimSizes() const
{ 
	return cellsInDim; 
} 

inline int 
Array::getDimension() const
{ 
	return dimension; 
}

inline MachineDataType 
Array::getType() const
{ 
	return type; 
}

inline Boolean_
Array::isDeltaArray() const
{
	return useDelta;
}

inline void
Array::resetIterateCount() 
{
	iterateCount = 0;
}

inline ostream& operator<<( ostream& os, Array& array)
{
	array.printOn( os );
	return os;
}

#endif

