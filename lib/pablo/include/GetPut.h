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
 * Author: Taed Nelson (nelson@cs.uiuc.edu)
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
 * GetPut.h: Basic routines used to read and write data in bitstream format.
 *
 *	$Header: /home/cs/carr/cvsroot/DSystem/lib/pablo/include/GetPut.h,v 1.1 1999/09/08 17:45:13 carr Exp $
 */
#ifndef GetPut_h
#define GetPut_h

	/* Routine functionGet:		Gets appropriate number of bytes 
	*				from *buffer into char and returns
	*				the number of bytes read.            */
	int functionGet( const char *buffer, char *value );

	/* Routine functionGet:		Gets appropriate number of bytes from 
	*				**bufptr into char, advances *bufptr by
	*				the number of bytes read, and returns
	*				the number of bytes read.            */
	int functionGet( char **bufptr, char *value );

	/* Routine functionGet:		Gets appropriate number of bytes 
	*				from *buffer into int and returns
	*				the number of bytes read.            */
	int functionGet( const char *buffer, int *value );

	/* Routine functionGet:		Gets appropriate number of bytes from 
	*				**bufptr into int, advances *bufptr by
	*				the number of bytes read, and returns
	*				the number of bytes read.            */
	int functionGet( char **bufptr, int *value );

	/* Routine functionGet:		Gets appropriate number of bytes 
	*				from *buffer into double and returns
	*				the number of bytes read.            */
	int functionGet( const char *buffer, double *value );

	/* Routine functionGet:		Gets appropriate number of bytes from 
	*				**bufptr into double, advances *bufptr
	*				by the number of bytes read, and returns
	*				the number of bytes read.            */
	int functionGet( char **bufptr, double *value );

	/* Routine functionGet:		Gets appropriate number of bytes 
	*				from *buffer into float and returns
	*				the number of bytes read.            */
	int functionGet( const char *buffer, float *value );

	/* Routine functionGet:		Gets appropriate number of bytes from 
	*				**bufptr into float, advances *bufptr by
	*				the number of bytes read, and returns
	*				the number of bytes read.            */
	int functionGet( char **bufptr, float *value );

	/* Routine functionPut:		Writes the char value as bytes in
	*			        *buffer & returns the number of bytes 
	*				written 			     */
	int functionPut( char *buffer, const char *value );

	/* Routine functionPut:		Writes the char value as bytes in
	*			        **bufptr, advances *bufptr by the number
	*				of bytes written & returns the number 
	*				of bytes written 		     */
	int functionPut( char **bufptr, const char *value );

	/* Routine functionPut:		Writes the int value as bytes in
	*			        *buffer & returns the number of bytes 
	*				written 			     */
	int functionPut( char *buffer, const int *value );

	/* Routine functionPut:		Writes the int value as bytes in
	*			        **bufptr, advances *bufptr by the number
	*				of bytes written & returns the number 
	*				of bytes written 		     */
	int functionPut( char **bufptr, const int *value );

	/* Routine functionPut:		Writes the double value as bytes in
	*			        *buffer & returns the number of bytes 
	*				written 			     */
	int functionPut( char *buffer, const double *value );

	/* Routine functionPut:		Writes the double value as bytes in
	*			        **bufptr, advances *bufptr by the number
	*				of bytes written & returns the number 
	*				of bytes written 		     */
	int functionPut( char **bufptr, const double *value );

	/* Routine functionPut:		Writes the float value as bytes in
	*			        *buffer & returns the number of bytes 
	*				written 			     */
	int functionPut( char *buffer, const float *value );

	/* Routine functionPut:		Writes the float value as bytes in
	*			        **bufptr, advances *bufptr by the number
	*				of bytes written & returns the number 
	*				of bytes written 		     */
	int functionPut( char **bufptr, const float *value );


/*
 * NOTE: There appears to be a bug in the resolution of function calls when
 * the signature contains a const char **. For example, 
 *	int functionGet( const char **bufptr, int *value )		<A>
 * With this declaration, the code segment 
 *  	char *bufp; float v; functionGet( &bufp, &v) 
 * will call functionGet( const char *buffer, int *v) and NOT the expected
 * functionGet declared in <A>.  This is why the Get functions do not include
 * the const in the signatures.	-RAA
 */

/****	Character Values	****/

inline int 
functionGet( const char *buffer, char *value ) 
{
	*value = *buffer;
	return ( 1 );
}

inline int
functionGet( char **bufptr, char *value ) 
{
	*value = *( *bufptr )++;
	return ( 1 );
}

inline int 
functionPut( char *buffer, const char *value ) 
{
	*buffer = *value;
	return ( 1 );
}

inline int 
functionPut( char **bufptr, const char *value ) 
{
	*( *bufptr )++ = *value;
	return ( 1 );
}

/****	Integer Values		****/

inline int 
functionGet( const char *buffer, int *value ) 
{
	char *vBytes = (char *)value;
	int bytes = 0;
	while( bytes < sizeof( int ) ) {
	    *vBytes++ = *buffer++;
	    bytes++;
	}
	return ( bytes );
}

inline int 
functionGet( char **bufptr, int *value ) 
{
	char *vBytes = (char *)value;
	int bytes = 0;
	while( bytes < sizeof( int ) ) {
	    *vBytes++ = *( *bufptr )++;
	    bytes++;
	}
	return ( bytes );
}

inline int 
functionPut( char *buffer, const int *value ) 
{
	char *vBytes = (char *)value;
	int bytes = 0;
	while( bytes < sizeof( int ) ) {
	    *buffer++ = *vBytes++;
	    bytes++;
	}
	return ( bytes );
}

inline int 
functionPut( char **bufptr, const int *value ) 
{
	char *vBytes = (char *)value;
	int bytes = 0;
	while( bytes < sizeof( int ) ) {
	    *( *bufptr )++ = *vBytes++;
	    bytes++;
	}
	return ( bytes );
}

/****	Float Values	    ****/

inline int 
functionGet( const char *buffer, float *value ) 
{
	char *vBytes = (char *)value;
	int bytes = 0;
	while( bytes < sizeof( float ) ) {
	    *vBytes++ = *buffer++;
	    bytes++;
	}
	return ( bytes );
}

inline int 
functionGet( char **bufptr, float *value ) 
{
	char *vBytes = (char *)value;
	int bytes = 0;
	while( bytes < sizeof( float ) ) {
	    *vBytes++ = *( *bufptr )++;
	    bytes++;
	}
	return ( bytes );
}

inline int 
functionPut( char *buffer, const float *value ) 
{
	char *vBytes = (char *)value;
	int bytes = 0;
	while( bytes < sizeof( float ) ) {
	   *buffer++ = *vBytes++;
	    bytes++;
	}
	return ( bytes );
}

inline int 
functionPut( char **bufptr, const float *value ) 
{
	char *vBytes = (char *)value;
	int bytes = 0;
	while( bytes < sizeof( float ) ) {
	    *( *bufptr )++ = *vBytes++;
	    bytes++;
	}
	return ( bytes );
}

/****	Double Values	   ****/

inline int 
functionGet( const char *buffer, double *value ) 
{
	char *vBytes = (char *)value;
	int bytes = 0;
	while( bytes < sizeof( double ) ) {
	    *vBytes++ = *buffer++;
	    bytes++;
	}
	return ( bytes );
}

inline int 
functionGet( char **bufptr, double *value ) 
{
	char *vBytes = (char *)value;
	int bytes = 0;
	while( bytes < sizeof( double ) ) {
	    *vBytes++ = *( *bufptr )++;
	    bytes++;
	}
	return ( bytes );
}

inline int 
functionPut( char *buffer, const double *value ) 
{
	char *vBytes = (char *)value;
	int bytes = 0;
	while( bytes < sizeof( double ) ) {
	    *buffer++ = *vBytes++;
	    bytes++;
	}
	return ( bytes );
}

inline int 
functionPut( char **bufptr, const double *value ) 
{
	char *vBytes = (char *)value;
	int bytes = 0;
	while( bytes < sizeof( double ) ) {
	    *( *bufptr )++ = *vBytes++;
	    bytes++;
	}
	return ( bytes );
}

#endif GetPut_h
