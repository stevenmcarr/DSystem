/* $Id: SD_Globals.i,v 1.1 1997/03/11 14:29:01 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// -*- C++ -*-
/* Explanatory one line comment on file nature */
/* $Header: /home/cs/carr/cvsroot/DSystem/src/libs/fortD/performance/staticInfo/SD_Globals.i,v 1.1 1997/03/11 14:29:01 carr Exp $
//
*/
// The tag should exactly match the filename
#ifndef _stdform_h
#define _stdform_h
// All includes and definitions go here. Include only the minimum set required to include this file. Do not include anything that is only required by the coressponding .c file (if there is one)

#include <Attributes.h>
#include <RecordDossier.h>
#include <StructureDescriptor.h>
#include <libs/fortD/performance/staticInfo/StaticSDDF.h>

// Define standard tags and such for static descriptors 

static const int SDDF_UNDEF_VALUE = -1;

static const int SD_TAG_BASE = 1;
static const int SD_TAG_DECOMP 			= 897;
static const int SD_TAG_DIST   			= 898;
static const int SD_TAG_ALIGN  			= 899;
static const int SD_TAG_ARRAY  			= 900;
static const int SD_TAG_PROC   			= 901;
static const int SD_TAG_STATIC_SRC_LOOP 	= 902;
static const int SD_TAG_DEPEND 			= 903;
static const int SD_TAG_SPMD_LOOP 		= 904;
static const int SD_TAG_SEND   			= 905;
static const int SD_TAG_RECV 			= 906;
static const int SD_TAG_SEND_WAIT 		= 907;
static const int SD_TAG_RECV_WAIT 		= 908;
static const int SD_TAG_SYMBOLIC_VALUE 		= 909;

// A handy macro to help eliminate a common error in my code.

#define CAST_NO_CTOR(type,value) (*(type *)(&value))

// Helper function for the very common task of setting a character
// Array entry in the RecordDossier to a char *
void SetStringInDossier(RecordDossier & d, const char * entryName, 
			const char * theString);

// Helper functions for the common case insertion into a structure descriptor
void AddEntryToStructureDesc(StructureDescriptor & s, 
			     const char * com1, const char * com2, 
			     const char * entryName, 
			     const MachineDataType theType, const int theDim);

// Shorter form of above (for when com2 == entryName)
void AddEntryToStructureDesc(StructureDescriptor & s, 
			     const char * com1, 
			     const char * entryName, 
			     const MachineDataType theType, const int theDim);


// Don't forget terminal semicolon on classes!
#endif




