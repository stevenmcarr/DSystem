/* $Id: CallSite.h,v 1.15 1997/03/11 14:34:39 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef CallSite_h
#define CallSite_h

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#ifndef FormattedFile_h
#include <libs/support/file/FormattedFile.h>
#endif

#ifndef SinglyLinkedListIO_h
#include <libs/support/lists/IOSinglyLinkedList.h>
#endif

#ifndef iptypes_h
#include <libs/ipAnalysis/ipInfo/iptypes.h>
#endif

#ifndef rn_string_h
#include <libs/support/strings/rn_string.h>
#endif

/******************************************************************
 * CallSite Abstraction                        September 1991     *
 * Author: John Mellor-Crummey                                    *
 *                                                                *
 * this file contains definitions that supports an external       *
 * representation of callsite summary information used for        *
 * interprocedural dataflow analysis                              *
 *                                                                *
 * Copyright 1991, Rice University, as part of the ParaScope      *
 * Programming Environment Project                                *
 *                                                                *
 ******************************************************************/

//-----------------------------------------------------
// class ActualListEntry
//    information about an actual parameter used at a
//    procedure callsite
//
//    type                     field[0]    field[1]  Note
//    VTYPE_CONSTANT           0           0
//    VTYPE_PROCEDURE          0           0          1
//    VTYPE_FORMAL_PARAMETER   0           0          2
//    VTYPE_COMMON_DATA        offset      size       3
//    VTYPE_LOCAL_DATA         offset      size       3
//    VTYPE_STAR               0           0          4
//    VTYPE_CONSTANT_EXPR      0           0          5
//    VTYPE_COMPILER_TEMPORARY 0           0
// 
//    1. VTYPE_PROCEDURE can be overlaid with 
//       VTYPE_INTRINSIC 
//    2. VTYPE_FORMAL_PARAMETER can be overlaid with 
//       VTYPE_PROCEDURE
//    3. pname is equivalence class leader, fields are
//       offset from class leader, and size of data 
//       object
//    4. pname is "*"
//    5. pname is "const_expr"
//
//    this class reads/writes itself from/to a database
//    port
//-----------------------------------------------------

class ActualListEntry : public SinglyLinkedListEntryIO {
	int ptype;    // vtype information 
	char *pname;  // actual name (if any applies) 
	int field[2]; // type specific info about the actual
public:
  SinglyLinkedListIO *fortd_set;
	ActualListEntry() {;}; // used by ActualList::NewEntry()
	ActualListEntry(char *p_name, int p_type, int f0, int f1) { 
		pname = ssave(p_name);
		ptype = p_type;
		field[0] = f0; field[1] = f1;
		fortd_set = 0;
	};

	// destructor
	virtual ~ActualListEntry(); 

	int Type() { return ptype; };
	char *Name() { return pname; };
	int GetField(unsigned int indx) { 
		assert(indx <= 1);
		return field[indx]; 
	};

	int ReadUpCall(FormattedFile& port);
	int WriteUpCall(FormattedFile& port);
};


//-----------------------------------------------------
// class ActualList
//    a list of actual parameters for a callsite 
//
//    (see documentation about ActualListEntry for
//    permissible arguments to Append)
//
//    this class reads/writes itself from/to a database
//    port
//-----------------------------------------------------

class ActualList : public SinglyLinkedListIO {
	ActualListEntry *current;
public:
	ActualList() { current = 0; };

	SinglyLinkedListEntryIO *NewEntry();

	void Append(char *name, int type, int f0, int f1) {
		ActualListEntry *e = 
			new ActualListEntry(name, type, f0, f1);
		SinglyLinkedList::Append((SinglyLinkedListEntry *) e);
	};

	ActualListEntry *First() { 
		return (current = (ActualListEntry *) 
			SinglyLinkedList::First()); 
	};
	ActualListEntry *Next() { 
		return current = 
			(current ? (ActualListEntry *) current->Next() : 0); 
	};
};


//-----------------------------------------------------
// class CallSite
//    information about a procedure callsite including 
//    the procedure name, its id from ft_NodeToNumber,
//    a boolean that notes if it is a procedure 
//    parameter, and a list of actuals passed
//
//    this class reads/writes itself from/to a database
//    port
//-----------------------------------------------------

class CallSite : public SinglyLinkedListEntryIO {
	char *entry_name;            // the routine invoked
	int callsite_id;             // from ft_NodeToNumber
	Boolean entry_is_proc_param; // procedure parameter?
	ActualList *alist;           // actual parameters
public:
  SinglyLinkedListIO *fortd_set[100]; // fortran d annotations
	CallSite(char *name, int site_id,  Boolean ipp) { 
		entry_name = ssave(name); 
		callsite_id = site_id;
		entry_is_proc_param = ipp;
		alist = new ActualList;
                for(int i =0; i< 100; ++i)
                 fortd_set[i] = 0;
	};
	CallSite() {  // used by CallSiteList::NewEntry
		entry_name = 0; 
		entry_is_proc_param = false;
		alist = new ActualList; 
                for(int i= 0; i< 100; ++i)
                 fortd_set[i] = 0;
 	};

	// destructor
	virtual ~CallSite(); 

	const char *Name() { return entry_name; };
  	int  Id()    { return callsite_id; };
	Boolean CalleeIsProcParameter() { return entry_is_proc_param; };
	ActualList *GetActuals() { return alist; };

	int ReadUpCall(FormattedFile& p);
	int WriteUpCall(FormattedFile& p);
};


//-----------------------------------------------------
// class CallSiteList
//    a list of callsites 
//
//    this class reads/writes itself from/to a database
//    port
//-----------------------------------------------------

class CallSites : public SinglyLinkedListIO {
private:
  SinglyLinkedListEntryIO *NewEntry();
public:
  void Add(CallSite *cs);
  SinglyLinkedListIO::Read;
  SinglyLinkedListIO::Write;
friend class CallSitesIterator;
};

class CallSitesIterator : private SinglyLinkedListIterator {
public:
  CallSitesIterator(CallSites *callSites);
  ~CallSitesIterator();
  CallSite *Current();
  SinglyLinkedListIterator::operator++;
  SinglyLinkedListIterator::Reset;
};

#endif callsite_h
