/*
 * This file is part of the Picasso Visualization Environment
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
 * CONFIDENTIAL INFORMATION. Distribution
 * restricted under license agreement.
 *
 * Author: Ruth A. Aydt (aydt@cs.uiuc.edu)
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
 * InitializeStatic.C: Initializes static data in proper order.  
 *			Include this file in programs built with the SDDF 
 *			library.
 *
 */
#ifndef InitializeStatic_C 
#define	InitializeStatic_C 

#include "CString.h"
#include "StructureDescriptor.h"
#include "RecordDictionary.h"


Obj Array::MSG_OBJ( "Array" );
Obj Value::MSG_OBJ( "Value" );
CString CString::NOMATCH( "_NO_MTCH" );
RecordDossier RecordDictionary::noDossier( CString::NOMATCH );
RecordDossier RecordDossier::NODOSSIER( CString::NOMATCH );
FieldDescriptor StructureDescriptor::noField( CString::NOMATCH );

#endif InitializeStatic_C 
