/* $Id: OnHomeTable.h,v 1.7 1997/03/11 14:28:33 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef OnHomeTable_h
#define OnHomeTable_h

/**********************************************************************
 * This class processes all ON_HOME directives and creates a table for
 * fast lookup on whether/how a stmt is affected by such directives.
 */

/**********************************************************************
 * Revision History:
 * $Log: OnHomeTable.h,v $
 * Revision 1.7  1997/03/11 14:28:33  carr
 * newly checked in as revision 1.7
 *
 * Revision 1.7  94/03/21  13:13:08  patton
 * fixed comment problem
 * 
 * Revision 1.6  94/02/27  20:14:39  reinhard
 * Added value-based distributions.
 * Make CC happy.
 * See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.
 * 
 * Revision 1.5  1994/02/27  19:44:24  reinhard
 * Tweaks to make CC happy.
 *
 * Revision 1.4  1994/01/18  19:50:10  reinhard
 * Updated include paths according to Makefile change.
 *
 * Revision 1.3  1993/06/22  23:09:10  reinhard
 * Moved include file to fort.
 *
 * Revision 1.2  1993/06/16  19:47:38  reinhard
 * Added support for rhs matching (not only lhs any more).
 *
 * Revision 1.1  1993/06/15  16:14:57  reinhard
 * Initial revision
 *
 */

#ifndef context_h
#include <libs/support/database/context.h>
#endif

#ifndef FortTextTree_h
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#endif

/*-------------------- GLOBAL DECLARATIONS --------------------------*/

class AST_ht;

/*-------------------- FORWARD DECLARATIONS -------------------------*/

class OnHomeTable;

/*-------------------- TYPES ----------------------------------------*/

// Type of method to be called by walk routine for every node
typedef int (OnHomeTable::*WalkMethod)(AST_INDEX node);

class OnHomeTable
{
public:
  OnHomeTable(FortTextTree my_ftt);       // Constructor 
  ~OnHomeTable();                         // Destructor 

  void      scan_program();               // Scan for directives
  void      copy_info(AST_INDEX node, AST_INDEX new_node);
  void      add_directive(AST_INDEX node, AST_INDEX loop_node);
  void      fixup_program();
  void      fixup_loop(AST_INDEX stmts_node, AST_INDEX loop_node);
  AST_INDEX get_home_node(AST_INDEX lhs_node);  // Map lhs to owner

private:
  FortTextTree ftt;
  AST_ht  *owners;       // Hashes stmts to lhs's that "own" them
  AST_ht  *directives;   // Hashes "owners" to their directives

  // Private methods
  char      *match_on_home_directive(AST_INDEX cmt_node,
				     AST_INDEX loop_node);
  AST_INDEX match_home_name         (AST_INDEX loop_node,
				     char *home_name,
				     FortTextTree ftt);
  Boolean   process_home            (AST_INDEX prev_node,
				     AST_INDEX loop_node,
				     char* home_name);
  void      record_owner            (AST_INDEX loop_node,
				     AST_INDEX matched_node);
  AST_INDEX get_directive           (AST_INDEX lhs_node);

};

#endif
