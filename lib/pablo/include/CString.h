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
 * CString.h: Character string manipulation
 *
 *	$Header: /home/cs/carr/cvsroot/DSystem/lib/pablo/include/CString.h,v 1.1 1999/09/08 17:45:13 carr Exp $
 */

#ifndef CString_h
#define CString_h

#include "Obj.h"

extern int CStringDEBUG;	/* A global variable we can toggle to enable
				 * or disable debugging within the CString
				 * methods.  It is defined in CString.cc and
				 * initialized to 0.			      */

class CString {
/*
*
* A CString supports the ``standard'' string operations, including
* creation, concatenation (+), comparison (<, <=, >, >=, !=, ==),
* and assignment (=).
*
*/
	/* Method operator<:	Lexiographic character string/CString	
	*			``less than''				*/
	friend int operator<( const char *charstr, const CString& str );

	/* Method operator>:	Lexiographic character string/CString
	*			``greater than''			*/
	friend int operator>( const char *charstr, const CString& str );

	/* Method operator<=:	Lexiographic character string/CString
	*			``less than or equal to''		*/
	friend int operator<=( const char *charstr, const CString& str );

	/* Method operator>=:	Lexiographic character string/CString
	*			``greater than or equal to''		*/
	friend int operator>=( const char *charstr, const CString& str );

	/* Method operator==:	Lexiographic character string/CString
	*			``equal to''				*/
	friend int operator==( const char *charstr, const CString& str );

	/* Method operator!=:	Lexiographic character string/CString
	*			``not equal''				*/
	friend int operator!=( const char *charstr, const CString& str );

	/* Method operator+:	Character string/CString concatenation	*/
	friend CString operator+( const char *charstr, const CString& str );

	/* Method concat:	Character string/CString concatenation	*/
	friend CString concat( const char *charstr, const CString& str );

private:
	int	stringLength;
	char	*stringValue;

public:
	static	CString	NOMATCH;

	/* Method CString:	The simplest CString constructor	*/
	CString();

	/* Method CString:	Initialized CString/character string
	*			 constructor				*/
	CString( const char *stringVal );

	/* Method CString:	Initialized CString/CString constructor	*/
	CString( CString& str );

	/* Method CString:	Initialized CString/CString constructor	*/
	CString( const CString& str );

	/* Method ~CString:	The CString destructor			*/
	virtual ~CString();
	
	/* Method concat:	CString concatenation			*/
	CString concat( const CString& str );

	/* Method concat:	CString and character string concatenation */
	CString concat( const char *charstr );

	/* Method getValue:	Extract C string representation		*/
	const char * getValue() const;

	/* Method length:	Character string length			*/
	int length() const; 

	/* Method operator=:	Character string to CString assignment	*/
	CString& operator=( const char *charstr );

	/* Method operator=: 	CString to CString assignment		*/
	CString& operator=( const CString& str );

	/* Method operator==:	Lexiographic CString equality test	*/
	int operator==( const CString& str ) const;

	/* Method operator==:	Lexiographic char * equality test	*/
	int operator==( const char *str ) const;

	/* Method operator<=:	Lexiographic CString comparison		*/
	int operator<=( const CString& str ) const;

	/* Method operator<=:	Lexicographic CString comparsion	*/
	int operator>=( const CString& str ) const;

	/* Method operator!=:	Lexiographic CString inequality test	*/
	int operator!=( const CString& str ) const;

	/* Method operator!=:	Lexiographic char * inequality test	*/
	int operator!=( const char *str ) const;

	/* Method operator+:	CString concatenation			*/
	CString operator+( const CString& str );

	/* Method operator+:	CString and character string concatenation */
	CString operator+( const char *charstr );

	/* Method operator[]:	CString character extraction		*/
	char& operator[]( int loc ) const; 

        /* Method char*:        Conversion to char*                     */
	operator const char*() const;

	/* Method printOn: 	helper function for CString output	*/ 	
	virtual void printOn( ostream& strm = cout ) const;

};

inline int 
CString::length() const
{
	return( stringLength );
}

inline ostream& operator<<( ostream& os, CString& str )
{
	str.printOn (os);

	return os;
}

#endif CString_h
