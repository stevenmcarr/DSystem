/* $Id: Mall.h,v 1.3 1997/03/11 14:28:32 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef Mall_h
#define Mall_h

/**********************************************************************
 * class Mall
 */

/**********************************************************************
 * Revision History:
 * $Log: Mall.h,v $
 * Revision 1.3  1997/03/11 14:28:32  carr
 * newly checked in as revision 1.3
 *
 * Revision 1.3  94/03/21  13:02:26  patton
 * fixed comment problem
 * 
 * Revision 1.2  94/02/27  20:14:37  reinhard
 * Added value-based distributions.
 * Make CC happy.
 * See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.
 * 
 * Revision 1.3  1994/02/27  19:44:05  reinhard
 * Tweaks to make CC happy.
 *
 * Revision 1.2  1994/01/18  19:49:45  reinhard
 * Updated include paths according to Makefile change.
 * Handle value-based distributions.
 *
 * Revision 1.1  1993/09/25  15:39:09  reinhard
 * Initial revision
 *
 */

#ifndef context_h
#include <libs/support/database/context.h>
#endif

#ifndef FortTextTree_h
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#endif

//#ifndef Str_ht_h
//#include <Str_ht.h>
//#endif

#ifndef forttypes_h
#include <libs/frontEnd/ast/forttypes.h>
#endif

/*-------------------- EXTERNAL DECLARATIONS ----------------*/

class IrrSymTab;
class NamedGenericTable;

/*------------------- TYPES ---------------------------------*/

// The different types to be allocated dynamically
typedef int MallType;

// The different extensions for newly created variables
enum ExtType  { TypeIndex,      // "i$type", "f$type"
		WorkArray,      // "i$wrk"
		WorkArraySize,  // "i$wrk_size"
		Index,          // "x$ind"
		Size,           // "x$size"
		NewSizeTemp     // "$newsize"
		};

/*------------------- CONSTANTS -----------------------------*/

// Based on include/fort/forttypes.h
static const int  MallType_cnt = TYPE_LOGICAL + 2;
extern const char *MallType_names[];
extern const char *MallType_prefix_strs[];
extern const char MallType_prefixes[];
extern const int  MallType_asts[];
extern const char *Ext_names[];


/**********************************************************************
 * class Mall_entry declaration
 */
class Mall_entry
{
public:
  Mall_entry(const char *my_name,              // Constructor 
	     const char *my_index_id,
	     const char *my_size_id,
	     int         my_type);
  ~Mall_entry();                               // Destructor 

  // Access functions
  const char *getName()     const { return name; }
  const char *getIndex_id() const { return index_id; }
  const char *getSize_id()  const { return size_id; }
  int  getType()      const { return type; }

private:
  const char *index_id;  // "x$ind"
  const char *name;      // "x"
  const char *size_id;   // "x$size"
  int        type;       // "1" = Integer
};


/**********************************************************************
 * class Mall declaration
 */
class Mall
{
public:
  Mall();      // Constructor 
  ~Mall();     // Destructor 

  // Access functions
  AST_INDEX getRoot_node()     const { return root_node; }

  void      convert(FortTree my_ft, FortTextTree my_ftt,
		    IrrSymTab *my_st = NULL);

private:
  Boolean    check_module ();              // (Type-)check module
  void       find_init_calls();            // Find initialization calls
  void       find_alloc_calls();           // Find allocation calls
  Mall_entry *gen_alloc(int        type,  // Generate alloc
			const char *name,
			AST_INDEX  size_node,
			AST_INDEX  &stmt_node);
  void       find_resize_calls();           // Find resizing calls
  void       convert_refs();                // Convert references
  void       delete_decls();                // Delete extra declarations
  int        prefix2type(char prefix);      // Map prefix to type
  Boolean    is_mall_call(AST_INDEX node,
			  const char *pattern, int arg_cnt,
			  int &type, AST_INDEX &args_list);

  // Private fields
  NamedGenericTable *dyn_arrs;     // Set of dynamic arrays
  AST_INDEX         free_node;     // Node for deallocation calls
  FortTree          ft;            // Fortran tree
  FortTextTree      ftt;           // Text/structure of FortTree
  AST_INDEX         init_node[MallType_cnt]; // Node for init calls
  const char        *new_size_temp_name;     // "$newsize"
  Boolean           own_st;        // We have our own st
  AST_INDEX         root_node;     // Root of program AST
  IrrSymTab         *st;           // Access to symbol table info
  const char        *type_ident_name[MallType_cnt]; // "i$type"
  const char        *wrk_ident_name[MallType_cnt];  // "i$wrk"
  AST_INDEX         wrk_ident_node[MallType_cnt];   // "i$wrk"
  const char        *wrk_size_name[MallType_cnt];   // "i$wrk_size"
  AST_INDEX         wrk_size_node[MallType_cnt];    // "i$wrk_size"
};

typedef Mall *Mall_ptr;

// IrrSymTab.h included at end (instead of beginning) to avoid dep cycle
//#ifndef IrrSymTab_h
//#include <libs/fort_d/irreg/IrrSymTab.h>
//#endif

#endif
