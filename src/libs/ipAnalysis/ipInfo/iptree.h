/* $Id: iptree.h,v 1.18 1997/03/11 14:34:50 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef iptree_h
#define iptree_h

/******************************************************************
 * Procedure IP Information Abstraction        September 1991     *
 * Author: John Mellor-Crummey                                    *
 *                                                                *
 * this file contains definitions that support an external        *
 * representation of summary interprocedural information for an   *
 * entry. this information includes a parameter mapping and       *
 * mod/ref and callsite information at the loop level.            *
 *                                                                *
 * Copyright 1991, Rice University, as part of the ParaScope      *
 * Programming Environment Project                                *
 *                                                                *
 ******************************************************************/


#if 0
#ifndef annot_table_h     /* for AnnotationTable  */
#include <libs/ipAnalysis/ipInfo/annot_table.h>
#endif
#endif

#ifndef CallSite_h      /* for Class CallSites */
#include <libs/ipAnalysis/ipInfo/CallSite.h>
#endif

#ifndef ParameterList_h      
#include <libs/ipAnalysis/ipInfo/ParameterList.h>
#endif

#ifndef ionudtree_h  /* for class NonUniformDegreeTreeNodeWithDBIO */
#include <libs/ipAnalysis/ipInfo/ionudtree.h>
#endif

#ifndef iptypes_h
#include <libs/ipAnalysis/ipInfo/iptypes.h>
#endif

#if 0
#ifndef modref_h    /* for class ModRefInfo */
#include <libs/ipAnalysis/ipInfo/modref.h>
#endif
#endif

#ifndef val_ip_h
#include <libs/moduleAnalysis/valNum/val_ip.h>
#endif

class FormattedFile;
class HashTable;

//-----------------------------------------------------
// class IPinfoTreeNode
//    IP information about a scope (procedure itself
//    or a loop body) including scalar mod/ref info
//    a list of callsites in the scope, and the id
//    of the scope returned by ft_NodeToNumber
//
//    this class reads/writes itself from/to a database
//    port
//-----------------------------------------------------

class IPinfoTreeNode: public NonUniformDegreeTreeNodeWithDBIO {
public:
	CallSites *calls;  // callsites in the scope
#if 0
	ModRefInfo *refs;     // mod/ref list
#endif
  HashTable *fd;           // Fortran D info
#if 0
        AnnotationTable *annot_tab;
#endif
	CodeBlockType type;   // scope type
	int block_id;         // scope id from ft_NodeToNumber

	// constructor -- allocate pointer components
	IPinfoTreeNode(IPinfoTreeNode *parent = 0, int code_block_id = -1,
		CodeBlockType t = BLOCKNONE);

	// destructor -- deallocate pointers to components
	virtual ~IPinfoTreeNode();

#if 0
        // annotations -- designed to deal with null annotation pointer
        Annotation *get_annotation (char *name) {
	  return ((annot_tab == NULL) ? NULL : annot_tab->query_entry(name));
        };
       
        void add_annotation (char *name, Annotation *annotation) {
	  if (annot_tab == NULL) annot_tab = new AnnotationTable();
	  annot_tab->add_entry (name, annotation);
        };
#endif

	// virtual constructor used by the base type to 
	// create nodes 
	virtual NonUniformDegreeTreeNodeWithDBIO 
		*New(NonUniformDegreeTreeNodeWithDBIO *parent);

	// ------------ I/O ------------
	int WriteUpCall(FormattedFile& port);
	int ReadUpCall(FormattedFile& port);
};


//-----------------------------------------------------
// class IPinfoTree
//    IP information about a procedure including the
//    procedure name, formal parameter list, and
//    a scope-based tree of interprocedural information 
//
//    this class reads/writes itself from/to a database
//    port
//-----------------------------------------------------

class IPinfoTree {
public:
  char *name;           // procedure name
  int procedureIsProgram;
  ParameterList *plist; // procedure formal parameter list

  IPinfoTreeNode *tree; // scope tree of IP information
  
  ValIP *val_ip;        // Initial symbolic info, read/written separately

  // used with no args when when reading from file
  IPinfoTree(IPinfoTreeNode *t = 0, char *s = 0, int isProgram = 0) {
    val_ip = 0;
    tree = (t ? t : new IPinfoTreeNode());  // allocate a "tree" if none 
    name = (s ? ssave(s) : 0);              // save if non-null 
    plist = new ParameterList;
	procedureIsProgram = isProgram;
  };
  
  ~IPinfoTree() { delete tree; if (name) sfree(name); 
		  delete plist; if (val_ip) delete val_ip;};
  
  // ------------ I/O ------------
  int Write(FormattedFile& port);
  int Read(FormattedFile& port);
};


#if 0
class IPinfoTreeCallSiteIterator {
  struct IPinfoTreeCallSiteIteratorS *repr;
  void Init(IPinfoTree *t);
  void DeleteList();
public:
  IPinfoTreeCallSiteIterator(IPinfoTree *t);
  IPinfoTreeCallSiteIterator(IPinfoTree &t);
  ~IPinfoTreeCallSiteIterator();
  
  void operator ++();
  CallSite *Current();
  void Reset();
};
#endif

#endif iptree_h
