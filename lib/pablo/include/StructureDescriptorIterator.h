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
 * StructureDescriptorIterator.h: Class used to step through fields in a 
 *			  StructureDescriptor.
 *
 *	$Header: /home/cs/carr/cvsroot/DSystem/lib/pablo/include/StructureDescriptorIterator.h,v 1.1 1999/09/08 17:45:14 carr Exp $
 */

#ifndef StructureDescriptorIterator_h
#define StructureDescriptorIterator_h

#include "FieldDescriptor.h"
#include "Obj.h"
#include "StructureDescriptor.h"

class StructureDescriptorIterator : public Obj {
/*
*
* A StructureDescriptorIterator is used to step through the fields in a
* StructureDesciptor.  It's member variables maintain a sense of the "current"
* StructureEntry in the Descriptor.
*
*/

private:
	Boolean_				firstTime;
	const StructureDescriptor&		myStructureDescriptor;
	StructureDescriptor::StructureEntry	*currentStructureEntry;

public:
	/* Method StructureDescriptorIterator:	The Iterator constructor 
	*		  with reference to StructureDescriptor.              */
	StructureDescriptorIterator( const StructureDescriptor& structDescr );

	/* Method ~StructureDescriptorIterator:	The Iterator destructor       */
	~StructureDescriptorIterator();

	/* Method first:  Returns reference to first FieldDescriptor in the 
	*		  StructureDescriptor or to "noField" if there isn't 
	*		  one. 						      */
	const FieldDescriptor& first();

	/* Method next:   Returns reference to next FieldDescriptor in the
	*		  StructureDescriptpor or to "noField" if there isn't
	*		  one. If called before first() for a particular 
	*		  instance of the iterator it behaves the same as
	*		  first().				              */
	const FieldDescriptor& next();

};

#endif StructureDescriptorIterator_h
