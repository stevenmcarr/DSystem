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
 * Author: Robert Olson (olson@cs.uiuc.edu)
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
 * BasePipe.h: Base class for pipes
 *
 *	$Header: /home/cs/carr/cvsroot/DSystem/lib/pablo/include/BasePipe.h,v 1.1 1999/09/08 17:45:13 carr Exp $
 */

#ifndef BasePipe_h
#define BasePipe_h

#include "Obj.h"

class Wrapper;

class BasePipe : public Obj {
/*
 *	BasePipe is the class from which all Pipes are derived. It has
 *	support for connections to wrappers.
 *	The methods for reading and writing from Pipes are defined
 *	in the two major subclasses, StreamPipe and BlockPipe.
 *	The object name is set to be the name in the constructor
 */
private:
	char *_objectName;			/* Name used in constructor */

protected:
	Wrapper *inputWrapper;
	Wrapper *outputWrapper;

	/* Method _setObjectName:		Give detailed name to Pipe  */
	void _setObjectName( const char *detailName );

public:
	/* Method BasePipe: 			The BasePipe constructor */
	BasePipe();

	/* Method ~BasePipe: 			The BasePipe destructor  */
	~BasePipe();

	/* Method getObjectName:		Overloads Obj::getObjectName()
	*					to return _objectName    */
	virtual const char * getObjectName() const;

	/* Method isFilePipe: 			Returns TRUE_ if this 
	*					is a file pipe           */
	virtual Boolean_ isFilePipe() const { 
		return FALSE_; 
		} ;

	/* Method isInputPipe: 			Returns TRUE_ if this 
	*					is an input pipe         */
	virtual Boolean_ isInputPipe() const { 
		return FALSE_; 
		} ;

	/* Method isOutputPipe: 		Returns TRUE_ if this 
	*					is an output pipe        */
	virtual Boolean_ isOutputPipe() const { 
		return FALSE_; 
		} ;

	/* Method getInputWrapper: 		Get the input wrapper    */
	Wrapper * getInputWrapper() const;

	/* Method getOutputWrapper: 		Get the output wrapper   */
	Wrapper * getOutputWrapper() const;

	/* Method setInputWrapper: 		Set the input wrapper    */
	void setInputWrapper( Wrapper *iw );

	/* Method setOutputWrapper: 		Set the output wrapper   */
	void setOutputWrapper( Wrapper *ow );

	/* Method printOn: 			Stream output function   */
	virtual void printOn( ostream& os) const;
};

inline ostream & operator<<( ostream &os, BasePipe& p )
{
	p.printOn( os );
	return os;
}

#endif BasePipe_h

