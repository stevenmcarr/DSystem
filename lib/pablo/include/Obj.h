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
 * Obj.h: Base class for all other derived classes
 *
 *	$Header: /home/cs/carr/cvsroot/DSystem/lib/pablo/include/Obj.h,v 1.1 1999/09/08 17:45:13 carr Exp $
 */

#ifndef Obj_h
#define Obj_h

#include <stream.h>

#include "Assert.h"
#include "Defines.h"

/*
 * We define RETURN_ON_FAILURE to check a value.  If the value indicates
 * FAILURE_, then return with the value. Else, no change in flow-of-control.
 * Useful in methods of type Boolean_.
 */
#define RETURN_ON_FAILURE( x )  if ( x == FAILURE_ ) return ( FAILURE_ );

class Obj {
/*
*
* Obj provides support for standard error and warning messages, and 
* maintains the name of the object.
*
*/
private:
	const char *_className;
	
protected:

	/* Method _setClassName:	Set _className			*/
	void _setClassName( const char *cname );

public:
	/* Method Obj:			The Obj constructor		*/
	Obj();

	/* Method Obj:			Obj constructor with class name */
	Obj( const char *cname );

	/* Method ~Obj:			The Obj destructor		*/
	virtual ~Obj();

	/* Method abort: 		Print message and abort system. */
	virtual void abort( const char *s, ... ) const;

	/* Method error: 		This prints an error message.	*/
	virtual void error( const char *s, ... ) const;

 	/* Method gentleWarning:        A warning message to stderr     */
	virtual void gentleWarning( const char *s, ... ) const;

	/* Method getClassName:		Return pointer to class name    */
	const char * getClassName() const;

	/* Method getObjectName:	This returns a name for the 
	*				object. As defined in Obj, it 
	*				returns the same string as the
	*				is returned by getClassName.
	*				However, derived classes may
	*				redefine it to include more 
	*				detailed information.		*/
	virtual const char * getObjectName() const;

	/* Method info:			Prints informational message.	*/
	virtual void info( const char *s, ... ) const;

	/* Method inspect:		Inspect the object.		*/
	virtual void inspect() const;

	/* Method warning:   		This prints a warning message. 	*/
	virtual void warning( const char *s, ... ) const;

	/* Method hash: 		Return a hash value for this 
	*				object  			*/
	int hash() const;

	/* Method notImplemented: 	Print an error message that the 
					argument list is not implemented */
	void notImplemented( const char *s, ... ) const;

	/* Method shouldNotImplement: 	This method prints an error message
	*				that says it should not be
	*				implemented.			*/
	void shouldNotImplement( const char *s, ...  ) const;

	/* Method subclassResponsibility: Method prints an error message
	*				that says it should be implemented
	*				by a subclass.			*/
	void subclassResponsibility( const char *s, ... ) const;

	/* Method printOn: 		Stream output function 		*/
	virtual void printOn( ostream &os = cout ) const {
		os << form( "Obj(0x%x)", this ); 
		} ;
};

inline void
Obj::_setClassName( const char *cname )
{
	_className = cname;
}

inline const char * 
Obj::getClassName() const
{
	return _className;
}

inline int
Obj::hash() const
{
	return (int) this;
}

inline ostream& operator<<( ostream& os, Obj& obj )
{
	obj.printOn( os );
	return os;
}

#endif Obj_h
