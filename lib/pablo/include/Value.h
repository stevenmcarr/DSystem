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
 * Value.h: Header file for class used to define common methods for
 *	    the various data types in the system.
 *
 *	$Header: /home/cs/carr/cvsroot/DSystem/lib/pablo/include/Value.h,v 1.1 1999/09/08 17:45:14 carr Exp $
 */


#ifndef Value_h
#define Value_h

#include "Array.h"
#include "CString.h"
#include "DataTraits.h"
#include "Obj.h"

class DataCharacteristics;

class Value {
/*
* The Value Class provides a uniform interface to data of different types.  
*/
	union dataUnion {
		char 	c;
		int 	i;
		float 	f;
		double	d;
		Array  	*a;
	    
	    dataUnion() { 
		d = 0.0; 
		}
	} ;


private:
	static Obj MSG_OBJ;		/* an object to handle our error msgs */
	static Value _newValue;		/* used by methods that return Value  */

	DataTraits traits;
	dataUnion  val;

public:
	static Value NOVALUE;		/* A guaranteed UNDEFINED Value       */

	/* Method Value:		Constructor with no arguments. Will
	*				default to UNDEFINED type and dimension
	*				0.                                    */
	Value();

	/* Method Value:		Constructor with type and dimension 
	*				specified 	                      */
	Value( MachineDataType _type, int _dimension );

	/* Method Value:		Constructor with DataTraits specified */
	Value( const DataTraits& _traits );

	/* Method Value:		Constructor with copy initialization  */
	Value( const Value& value );

	/* Method Value:		Constructor with character data       */
	Value( char dataValue );

	/* Method Value:		Constructor with integer data         */
	Value( int dataValue );

	/* Method Value:		Constructor with float data           */
	Value( float dataValue );

	/* Method Value:		Constructor with double data          */
	Value( double dataValue );

	/* Method Value:		Constructor with pointer to Array 
	*				instance.  The ctor will actually
	*				create a new Array instance and the
	*				pointer to that is kept in the Value. 
	*				This insures that Value can free
	*				the Array it references when it is
	*				destroyed without having to worry 
	*				about leaving dangling references.    */
	Value( Array *dataValue );

	/* Method Value:		Constructor with CString reference.
	*				This is a convenience function that
	*				sets up the Value as an ARRAY of
	*				CHARACTERs with dimension = 1 and
	*				initializes the array cells from 
	*				the CString argument.                 */
	Value( const CString& cstring );

	/* Method ~Value:		The Value Destructor		      */
	~Value();

	/* Method bitsToObj:		Set the value from binary bitstream
	*				representation of data. The bitstream
	*				data should match the traits currently 
	*				associated with this value. Return 
	*				number of bytes used or -1 if error   */
	int bitsToObj( const char *bufPtr );

	/* Method bytesNeeded:		Returns number of Bytes needed to 
	*				represent this value as a binary 
	*				bitstream			      */
	int bytesNeeded() const;

	/* Method cbitsToObj:		Set the value from binary bitstream
	*				representation of data - converting to
	*				native representation. The bitstream
	*				data should match the traits currently 
	*				associated with this value. Return 
	*				number of bytes used or -1 if error   */
	int cbitsToObj( const char *bufPtr, 
			const DataCharacteristics *cnvInfo );

	/* Method cbytesNeeded:		Returns number of Bytes needed to 
	*				represent this value as a non-native
	*				binary bitstream 		      */
	int cbytesNeeded( const DataCharacteristics *cnvInfo ) const;

	/* Method getCString:		Set "cstring" to the string value of 
	*				the Value instance if this is indeed
	*				an ARRAY Value of type CHARACTER and
	*				dimension 1.  If it isn't, then return
	*				FAILURE_ and set cstring to the empty
	*				string.                               */
	Boolean_ getCString( CString& cstring ) const;

	/* Method getResultType:	Returns the data type of the Value
	*				resulting from an arithmetic operation
	*				on this Value and another with the 
	*				MachineDataType specified.	      */
	MachineDataType getResultType( MachineDataType operand_type ) const;

	/* Method getTraits:		Returns the data traits of the Value  */
	const DataTraits& getTraits() const;

	/* Method getValueAs:		Returns Value with specified type     */
	Value getValueAs( MachineDataType type ) const;
	
	/* Method getValueAs:		Sets newValue to have specified type
	*				and same data as this value           */
	void getValueAs( MachineDataType type, Value& newValue ) const;

	/* Method isDefined:          	Returns TRUE_ if Value is defined.    */
	Boolean_ isDefined() const;

	/* Method isUndefined:          Returns TRUE_ if Value is undefined.  */
	Boolean_ isUndefined() const;

	/* Method objToBits:		Represent the value as a binary
	*				bitstream.  Return number of bytes
	*				used, or -1 if there was not enough 
	*				space available to hold the data.     */
	int objToBits( char* const bufPtr, int bufSize ) const;

	/* Method objToCbits:		Represent the value as a non-native
	*				binary bitstream.  Return number of 
	*				bytes used, or -1 if there was not 
	*				enough space to hold the data.        */
	int objToCbits( char* const bufPtr, int bufSize,
			const DataCharacteristics *cnvInfo ) const;

	/* Method setCString:		Method which sets this Value to the
	*				cstring argument IF the Value is
	*				already of type CHARACTER and has
	*				dimension = 1.  If the type/dimension 
	*				traits are different, then the Value
	*				is not updated and FAILURE_ is 
	*				returned.  (See also the assignment
	*				operator to change the Value to a
	*				cstring regardless of the current
	*				traits.)                              */
	Boolean_ setCString( const CString& cstring );

	/* Method setQuantity:		Method which sets the quantity of this 
	*				Value to the quantity of newValue
	*				without changing type of this Value.
	*				If the type of the newValue is such 
	*				that it cannot be cast into the type
	*				of this Value without losing precision,
	*				Value is not changed and FAILURE_ is
	*				returned.  This method will work for
	*				Values of any dimension as long as the
	*				dimension of newValue is the same as
	*				the dimension of this Value.( See also 
	*				the assignment operators to change both
	*				quantity and type of this Value.)     */
	Boolean_ setQuantity( const Value& newValue );

	/* Method operator=:		Value to Value assignment (upon 
	*				completion both Value instances have
	*				copies of the data)		      */
	Value& operator=( const Value& value );

	/* Method operator=:		char to Value assignment              */
	Value& operator=( char dataValue );

	/* Method operator=:		integer to Value assignment 	      */
	Value& operator=( int dataValue );

	/* Method operator=:		float to Value assignment             */
	Value& operator=( float dataValue );

	/* Method operator=:		double to Value assignment 	      */
	Value& operator=( double dataValue );

	/* Method operator=:		Array pointer to Value assignment.
	*				Upon completion, this Value instance
	*				points to it's own copy of the Array
	*				data.                                 */
	Value& operator=( const Array *dataValue );

	/* Method operator=:		CString to Value assignment.
	*				Upon completion, this Value instance
	*				will be a CHARACTER array of dimension 
	*				1 with the Array cells initialized
	*				to the CString characters.            */
	Value& operator=( const CString& cstring );

	/* Method operator<:		Value ``less than'' comparison.
	*				Undefined when type == UNDEFINED 
	*				or dimension != 0                     */
	Boolean_ operator<( const Value& v );

	/* Method operator>:		Value ``greater than'' comparison.
	*				Undefined when type == UNDEFINED 
	*				or dimension != 0                     */
	Boolean_ operator>( const Value& v );

	/* Method operator<=:		Value ``less than or equal to'' 
	*				comparison. Undefined when type == 
	*				UNDEFINED or dimension != 0           */
	Boolean_ operator<=( const Value& v );

	/* Method operator>=:		Value ``greater than or equal to'' 
	*				comparison. Undefined when type == 
	*				UNDEFINED or dimension != 0           */
	Boolean_ operator>=( const Value& v );

	/* Method operator==:		Value ``equal to'' comparison.
	*				Undefined when type == UNDEFINED 
	*				or dimension != 0                     */
	Boolean_ operator==( const Value& v );

	/* Method operator!=:		Value ``not equal to'' comparison.
	*				Undefined when type == UNDEFINED 
	*				or dimension != 0                     */
	Boolean_ operator!=( const Value& v );

	/* Method operator-:		Value ``unary minus'' operator.
	*				Undefined when type == UNDEFINED
	*				or dimension != 0		      */
	Value operator-( );

	/* Method operator+:		Value addition - undefined when 
	*				type = UNDEFINED or dimension != 0   */
	Value operator+( const Value& v );

	/* Method operator-:		Value subtraction - undefined when 
	*				type = UNDEFINED or dimension != 0   */
	Value operator-( const Value& v );

	/* Method operator*:		Value multiplication - undefined when 
	*				type = UNDEFINED or dimension != 0   */
	Value operator*( const Value& v );

	/* Method operator/:		Value division - undefined when
	*				type = UNDEFINED or dimension != 0   */
	Value operator/( const Value& v );

	/* Method char:			Conversion to char		      */
	operator char() const;

	/* Method int:			Conversion to int		      */
	operator int() const;

	/* Method float:		Conversion to float		      */
	operator float() const;

	/* Method double:		Conversion to double		      */
	operator double() const;

	/* Method Array*:		Conversion to Array*		      */
	operator Array*() const;

	/* Method CString:		Conversion to CString.  If not a      
	*				CHARACTER with dimension 0 then the
	*				empty string is returned.             */
	operator CString() const;

	/* Method printOn:		Helper function for Value output      */
	void printOn( ostream& strm = cout ) const;
};

#ifndef COERCE_TO_LOWER_PRECISION
/*  
 * Result Type Table Format: entry[i][j]
 * Coerce to Higher Precision of 2 operators
 *
 *
 *  i\j   CHAR     INT     FLOAT   DOUBLE  UNDEF
 * CHAR     C       I        F       D       U
 * INT      I       I        F       D       U
 * FLOAT    F       F        F       D       U
 * DOUBLE   D       D        D       D       U
 * UNDEF    U       U        U       U       U
 */

static const MachineDataType resultTypeTable[5][5] =
{
        { CHARACTER, INTEGER, FLOAT, DOUBLE, UNDEFINED, },
        { INTEGER, INTEGER, FLOAT, DOUBLE, UNDEFINED, },
        { FLOAT, FLOAT, FLOAT, DOUBLE, UNDEFINED, },
        { DOUBLE, DOUBLE, DOUBLE, DOUBLE, UNDEFINED, },
        { UNDEFINED, UNDEFINED, UNDEFINED, UNDEFINED, UNDEFINED, },
};


#else           /* Use coercion to lower precision */

/*  
 * Result Type Table Format: entry[i][j]
 * Coerce to Lower Precision of 2 operators
 *
 *   i\j  CHAR     INT     FLOAT   DOUBLE  UNDEF
 * CHAR     C       C        C       C       U
 * INT      C       I        I       I       U
 * FLOAT    C       I        F       F       U
 * DOUBLE   C       I        F       D       U
 * ARRAY    U       U        U       U       U
 * UNDEF    U       U        U       U       U
 */

static const MachineDataType resultTypeTable[5][5] =
{
        { CHARACTER, CHARACTER, CHARACTER, CHARACTER, UNDEFINED,  },
        { CHARACTER, INTEGER, INTEGER, INTEGER, UNDEFINED, },
        { CHARACTER, INTEGER, FLOAT, FLOAT, UNDEFINED,  },
        { CHARACTER, INTEGER, FLOAT, DOUBLE, UNDEFINED, },
        { UNDEFINED, UNDEFINED, UNDEFINED, UNDEFINED, UNDEFINED, },
};

#endif

/*  
 * Cast Type Table Format: entry[i][j]
 * Cast RHS(j) to type of LHS(i) without any loss of information.
 * If this cannot be done, for example if RHS is DOUBLE and LHS is FLOAT,
 * then result is UNDEFINED.  This is used by the setQuantity method.
 *
 *  i\j   CHAR     INT     FLOAT   DOUBLE  UNDEF
 * CHAR     C       U        U       U       U
 * INT      I       I        U       U       U
 * FLOAT    F       F        F       U       U
 * DOUBLE   D       D        D       D       U
 * UNDEF    U       U        U       U       U
 */

static const MachineDataType castTypeTable[5][5] =
{
        { CHARACTER, UNDEFINED, UNDEFINED, UNDEFINED, UNDEFINED, },
        { INTEGER,   INTEGER,   UNDEFINED, UNDEFINED, UNDEFINED, },
        { FLOAT,     FLOAT,     FLOAT,     UNDEFINED, UNDEFINED, },
        { DOUBLE,    DOUBLE,    DOUBLE,    DOUBLE,    UNDEFINED, },
        { UNDEFINED, UNDEFINED, UNDEFINED, UNDEFINED, UNDEFINED, },
};

inline MachineDataType 
Value::getResultType( MachineDataType operand_type ) const
{
	return ( resultTypeTable[traits.getType()][operand_type] );

}

inline const DataTraits& 
Value::getTraits() const	
{ 
	return traits; 
}

inline Boolean_
Value::isDefined() const
{
        return traits.isDefined();
}

inline Boolean_
Value::isUndefined() const
{
        return traits.isUndefined();
}

inline ostream& operator<< ( ostream& os, Value& v)
{
	v.printOn( os );
	return os;
}

#endif Value.h
