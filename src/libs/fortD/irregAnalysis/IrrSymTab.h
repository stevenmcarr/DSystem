/* $Id: IrrSymTab.h,v 1.8 1997/03/11 14:28:31 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef IrrSymTab_h
#define IrrSymTab_h

/**********************************************************************
 * This class is the interface from the Irregular Part of the
 * Distribted Memory Compiler to symbol table information.
 *
 * 6/3/93 RvH:
 * This class should go away as soon as we have a sane symbol table
 * mechanism in the Fortran D compiler; for now, it serves just for
 * hiding the messy details of the current world.
 */

/**********************************************************************
 * Revision History:
 * $Log: IrrSymTab.h,v $
 * Revision 1.8  1997/03/11 14:28:31  carr
 * newly checked in as revision 1.8
 *
 * Revision 1.8  94/03/21  13:00:09  patton
 * fixed comment problem
 * 
 * Revision 1.7  94/02/27  20:14:35  reinhard
 * Added value-based distributions.
 * Make CC happy.
 * See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.
 * 
 * Revision 1.8  1994/02/27  19:43:41  reinhard
 * Tweaks to make CC happy.
 * Added is_distributed(), get_rank().
 *
 */

#ifndef fortranD_h
#include <libs/fortD/misc/FortD.h>
#endif

#ifndef dg_instance_h
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#endif

class StringHashTable;                   // Minimal external declaration
//#ifndef string_ht_h
//#include <include/misc/string_ht.h>      // StringHashTable
//#endif

#ifndef Mall_h
#include <libs/fortD/irregAnalysis/Mall.h>
#endif

#ifndef cfgval_h
#include <libs/moduleAnalysis/cfgValNum/cfgval.h>
#endif

#ifndef Cfg_h
#include <libs/moduleAnalysis/cfg/Cfg.h>
#endif

/*-------------------- TYPES ----------------------------------------*/

typedef char       *Name_type;    // Identifier name
typedef Name_type  *Names_type;   // Array of names

typedef int        Dim_type[MAXDIM];    // Ranks of an array
typedef Dim_type   *Dims_type;          // Array of ranks

/**********************************************************************
 * class IrrSymTab declaration
 */
class IrrSymTab
{
public:
  IrrSymTab();        // Constructor 
  ~IrrSymTab();                          // Destructor 

  // Access functions
  SymDescriptor getSymtab() const { return symtab; }
  void          putFd(FortranDInfo *my_fd) { fd = my_fd; }

  void    init(FortTree my_ft);                   // Initialization
  Boolean is_local(const char *name);             // Local variable ?
  Boolean is_irreg_and_distrib(AST_INDEX   node,
			       CfgInstance cfg);
  Boolean is_distributed(AST_INDEX node,
			 int dim = -1);           // Ref to dist var ?
  AST_INDEX get_size_node(const char *name,
			  int dim = 0);           // Get subscript size
  SNODE     *findName(const char *name) const;    // Find variable from name
  int       get_type(const char *name);           // Get type
  int       get_elmt_size(const char *name);      // Size of one elmt
  int       get_rank(const char *name);           // Num of dims
  //char    *getInt(int index);                   // Declare int
  const char *gen_fresh_int_name();               // Generate "i$" etc.
  const char *gen_fresh_name(const char *id_name, // Generate fresh var
			     const char *ext_name = "");
  Boolean declInt(const char *name);               // Declare scalar int
  Boolean decl1dArr(const char *name, int size,    // 1-D array,
                    int type = TYPE_INTEGER);      // by default integer
  Boolean decl1dArr(AST_INDEX name_node,           // 1-D array,
                    AST_INDEX size_node,           // by default integer
                    int type = TYPE_INTEGER);
  Boolean declIntParam(const char *name, AST_INDEX val); // Parameter

private:
  AST_INDEX find_decls_node(AST_INDEX root_node);
  void      addDecl(AST_INDEX decl_node);   // Add declaration to AST 
  Boolean   sp_is_array(SNODE *sp);
  void      store_sym_table   (AST_INDEX node, enum FORM type);
  AST_INDEX gen1dArrDecl      (const char *name, int size);
  AST_INDEX genVarDecl        (const char *name, int rank, int *dims);
  AST_INDEX gen_decls_comment (AST_INDEX decls_node);
  void      break_line_if_needed(AST_INDEX &node, int &len);

  // Private fields
  int           arrs_len[MallType_cnt];  // Lengths of decls
  AST_INDEX     arrs_node[MallType_cnt]; // Add array decls here
  Boolean       decls_exist;             // Any declarations yet ?
  AST_INDEX     decls_node;              // New decls after this node
  FortranDInfo  *fd;                     // General Fortran D info
  FortTree      ft;                      // Fortran tree
  SNODE         *ihash[NIHASH];          // From symbolt.c
  int           ints_len;                // Lengths of decls
  AST_INDEX     ints_node;               // Add scalar int decls here
  AST_INDEX     params_node;             // Add parameters here
  StringHashTable *string_ht;            // Names generated so far
  SymDescriptor   symtab;                // Access to symbol table
};

typedef IrrSymTab *IrrSymTab_ptr;

#endif
