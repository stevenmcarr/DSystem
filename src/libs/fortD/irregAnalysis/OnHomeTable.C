/* $Id: OnHomeTable.C,v 1.7 1997/03/11 14:28:32 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/**********************************************************************
 * This class processes all ON_HOME directives and creates a table for
 * fast lookup on whether/how a stmt is affected by such directives.
 *
 * The format of the ON_HOME directive is adapted to HPF:
 *
 * C   EXECUTE (<ivar>) ON_HOME <array>(<ivar>)
 *     DO <ivar> = ...
 * 
 * For now it is assumed that <array>(<ivar>) occurs at the lhs or rhs
 * of some stmt within the loop.
 */

/**********************************************************************
 * Revision History:
 * $Log: OnHomeTable.C,v $
 * Revision 1.7  1997/03/11 14:28:32  carr
 * newly checked in as revision 1.7
 *
Revision 1.7  94/03/21  13:11:18  patton
fixed comment problem

Revision 1.6  94/02/27  20:14:38  reinhard
Added value-based distributions.
Make CC happy.
See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.

 * Revision 1.6  1994/02/27  19:44:10  reinhard
 * Tweaks to make CC happy.
 *
 * Revision 1.5  1994/01/18  19:49:56  reinhard
 * Updated include paths according to Makefile change.
 *
 * Revision 1.4  1993/10/04  15:38:27  reinhard
 * Const'd params.
 *
 * Revision 1.3  1993/06/22  23:08:50  reinhard
 * Moved include file to fort.
 *
 * Revision 1.2  1993/06/16  19:47:01  reinhard
 * Added support for rhs matching (not only lhs any more).
 *
 * Revision 1.1  1993/06/15  16:14:37  reinhard
 * Initial revision
 *
 */

#include <assert.h>
#include <stream.h>
#include <libs/fortD/irregAnalysis/OnHomeTable.h>
#include <libs/frontEnd/include/walk.h>
#include <libs/frontEnd/ast/AstIter.h>
#include <libs/frontEnd/ast/AST_ht.h>
#include <libs/support/strings/rn_string.h>
#include <libs/frontEnd/ast/groups.h>


/*----------------------- GLOBAL DECLARATIONS -----------------------*/

EXTERN(AST_INDEX, begin_ast_stmt,  (AST_INDEX node));
EXTERN(AST_INDEX, find_next_loop,  (AST_INDEX start_node));
EXTERN(const char *, ftt_ast2str,  (FortTextTree ftt, AST_INDEX node));
EXTERN(const char *, loop_gen_ivar, (AST_INDEX node));
EXTERN(Boolean,   match_pattern,   (const char *&str, 
				    const char *pattern));
EXTERN(char *,    str_tolower,     (const char *str));


/**********************************************************************
 * Constructor 
 */
OnHomeTable::OnHomeTable(FortTextTree my_ftt)
: ftt        (my_ftt),
  owners     (new AST_ht()),
  directives (new AST_ht())
{
  scan_program();
}


/**********************************************************************
 * Destructor 
 */
OnHomeTable::~OnHomeTable()
{
  delete owners;
  delete directives;
}


/**********************************************************************
 * scan_program()  Scan the program for directives.
 */
void
OnHomeTable::scan_program()
{
  AST_INDEX root_node = ftt_Root(ftt);
  AST_INDEX node, prev_node;
  AstIter   iter(root_node);
  char      *home_name;
  Boolean   found_lhs;

  // Scan all loops for preceeding "ON_HOME" directives
  while ((node = iter()) != AST_NIL)
  {
    // Are we at a loop node, preceeded by a on_home directive ?
    if ((is_loop(node))
	&& ((prev_node = list_prev(node)) != AST_NIL)
	&& (is_comment(prev_node))
	&& (home_name = match_on_home_directive(prev_node, node)))
    {
      found_lhs = process_home(prev_node, node, home_name);

      if (!found_lhs) {
	//add_dummy_lhs
      }
    }
  }
}


/**********************************************************************
 * fixup_program()  Push back any directives that were removed from
 *                  their loops by previous transformations.
 */
void
OnHomeTable::fixup_program()
{
  char      *home_name;
  AST_INDEX node;
  AST_INDEX root_node = ftt_Root(ftt);
  AST_INDEX loop_node = find_next_loop(root_node);
  AstIter   iter(root_node);

  // Update ftt
  ftt_TreeChanged(ftt, root_node);

  // Scan all loops for preceeding "ON_HOME" directives
  while (((node = iter()) != AST_NIL) && (loop_node != AST_NIL))
  {
    if (node == loop_node)
    {
      loop_node = find_next_loop(loop_node);
    }
    else
    {
      // Are we at a on_home directive for <loop_node> ?
      if ((is_comment(node))
	  && (home_name = match_on_home_directive(node, loop_node))
	  && (list_next(node) != loop_node))
      {
	// Push directive back in front of <loop_node>
	node = list_remove_node(node);
	(void) list_insert_before(loop_node, node);
      }
    }
  }

  scan_program();
}


/**********************************************************************
 * fixup_loop()  Push back any directives that were removed from
 *               a loops by previous transformations.
 */
void
OnHomeTable::fixup_loop(AST_INDEX stmts_node, AST_INDEX loop_node)
{
  char      *home_name;
  AST_INDEX root_node = ftt_Root(ftt);
  AST_INDEX prev_node = list_prev(stmts_node);

  // Update ftt
  ftt_TreeChanged(ftt, root_node);

  // Are we at a on_home directive for <loop_node> ?
  if ((is_comment(prev_node))
      && (home_name = match_on_home_directive(prev_node, loop_node))
      && (list_next(prev_node) != loop_node))
  {
    // Push directive back in front of <loop_node>
    prev_node = list_remove_node(prev_node);
    (void) list_insert_before(loop_node, prev_node);
  }
}


/**********************************************************************
 * match_on_home_directive()  Try to match an ON_HOME directive in cmt.
 */
char *
OnHomeTable::match_on_home_directive(AST_INDEX cmt_node,
				     AST_INDEX loop_node)
{
  char    *home_name = NULL;   // The home we are interested in ("x(i)")
  AstIter iter(loop_node);     // Iterator for walking the loop
  char    *orig_cmt, *cmt, *cmt_start;

  // Try to match text of comment against "execute (i) on_home"
  orig_cmt  = gen_get_text(gen_COMMENT_get_text(cmt_node));
  cmt       = str_tolower(orig_cmt);
  cmt_start = cmt;    // Save this for later delete

  if (match_pattern(cmt, "execute ("))
  {
    if ((match_pattern(cmt, loop_gen_ivar(loop_node)))
	&& (match_pattern(cmt, ") on_home ")))
    {
      // <cmt> now contains the tail ("x(i)") of the ON_HOME directive
      // Make a copy so that we can delete <cmt_start> properly
      home_name = ssave(cmt);
    }
    else
    {
      cerr << "WARNING OnHomeTable::match_on_home_directive(): " <<
	"could not match \"" << cmt_start << "\".\n";
    }
  }
  delete cmt_start;

  return home_name;
}


/**********************************************************************
 *  match_home_name()  Try to match <home_name> against some lhs
 *                     in the body of <loop_node>.
 */
AST_INDEX
OnHomeTable::match_home_name(AST_INDEX loop_node, char *home_name,
			     FortTextTree ftt)
{
  AstIter    iter(loop_node, false);  // Iterate not <only_stmts>
  AST_INDEX  matched_node = AST_NIL;  // Found, matching lhs
  AST_INDEX  node;
  const char *text;

  while (((node = iter()) != AST_NIL) && (matched_node == AST_NIL))
  {
    if (is_subscript(node))
    {
      text = ftt_ast2str(ftt, node);
	      
      // Did we find a match ?
      if (match_pattern(text, home_name))
      {
	matched_node = node;
      }
    }
  }

  return matched_node;
}


/**********************************************************************
 * process_home()
 */
Boolean                                      // Did we find lhs ?
OnHomeTable::process_home(AST_INDEX prev_node,
			  AST_INDEX loop_node, char* home_name)
{
  AST_INDEX matched_node;
  Boolean   found_lhs;

  // Try to match remaining text of comment against some lhs
  // occuring in the body of the loop
  matched_node = match_home_name(loop_node, home_name, ftt);
  found_lhs    = (Boolean) (matched_node != AST_NIL);

  // Did we find a matching lhs ?
  if (found_lhs)
  {
    // Record the directive for the matched node
    directives->add_entry(matched_node, (Generic) prev_node);

    // Record the matched node in all stmts of current loop
    record_owner(loop_node, matched_node);
  }
  else
  {
    cerr << "WARNING OnHomeTable::process_home((): " <<
      "Did not find lhs \"" << home_name <<
	"\" in body of loop, AST_INDEX = " << loop_node << ".\n";
  }

  delete home_name;

  return found_lhs;
}


/**********************************************************************
 * record_owner() Record the matched node in all stmts of current loop.
 */
void
OnHomeTable::record_owner(AST_INDEX loop_node, AST_INDEX matched_node)
{
  AstIter   iter(loop_node);    // Iterator for walking the loop
  AST_INDEX node, lhs_node;

  while ((node = iter()) != AST_NIL)
  {
    if (is_assignment(node))     
    {
      lhs_node = gen_ASSIGNMENT_get_lvalue(node);
      owners->add_entry(lhs_node, (Generic) matched_node);
    }
  }
}
 
 
/**********************************************************************
 * copy_info()  Copy ON_HOME info from <node> to <new_node>.
 *              <new_node> is assumed to at stmt level, eg, a loop
 *              header. The info is copied to the nodes in <new_node>
 *              recursivly.
 */
void
OnHomeTable::copy_info(AST_INDEX node, AST_INDEX new_node)
{
  AST_INDEX stmt_node, lhs_node, owner_node;
 
  // <node> has to be the lhs of an assignment in order to carry info
  stmt_node = begin_ast_stmt(node);
  if (is_assignment(stmt_node))
  {
    lhs_node   = gen_ASSIGNMENT_get_lvalue(stmt_node);
    owner_node = get_home_node(lhs_node);
 
    // Is there any ownership info for <lhs_node> available ?
    if (owner_node != AST_NIL)
    {
      record_owner(new_node, owner_node);
    }
  }
  else
  {
    cerr << "WARNING OnHomeTable::copy_info(): node = " << node <<
      " is not within an assignment statement.\n";
  }
}


/**********************************************************************
 * add_directive()  Copy ON_HOME directive from <node> to <new_node>.
 *              <new_node> is assumed to at stmt level, eg, a loop
 *              header. The info is copied to the nodes in <new_node>
 *              recursivly.
 */
void
OnHomeTable::add_directive(AST_INDEX node, AST_INDEX loop_node)
{
  AST_INDEX stmt_node, lhs_node, directive_node, new_directive_node;

  // <node> has to be the lhs of an assignment in order to carry info
  stmt_node = begin_ast_stmt(node);
  if (is_assignment(stmt_node))
  {
    lhs_node   = gen_ASSIGNMENT_get_lvalue(stmt_node);
    directive_node = get_directive(lhs_node);

    // Is there any ownership info for <lhs_node> available ?
    if (directive_node != AST_NIL)
    {
      new_directive_node = tree_copy(directive_node);
      (void) list_insert_before(loop_node, new_directive_node);
    }
  }
  else
  {
    cerr << "WARNING OnHomeTable::add_directive(): node = " << node <<
      " is not within an assignment statement.\n";
  }
}


/**********************************************************************
 * get_home_node()  Return lhs corresponding to enclosing ON_HOME
 *                 directive, or original lhs.
 */
AST_INDEX
OnHomeTable::get_home_node(AST_INDEX lhs_node)
{
  Boolean   other_owner = owners->query_entry(lhs_node);
  AST_INDEX owner_node  = other_owner ?
    (AST_INDEX) owners->get_entry_by_AST(lhs_node) : lhs_node;

  return owner_node;
}


/**********************************************************************
 * get_directive()
 */
AST_INDEX
OnHomeTable::get_directive(AST_INDEX lhs_node)
{
  AST_INDEX owner_node, directive_node;

  owner_node     = (AST_INDEX) owners->get_entry_by_AST(lhs_node);
  directive_node = (owner_node == AST_NIL) ? AST_NIL
    : (AST_INDEX) directives->get_entry_by_AST(owner_node);

  return directive_node;
}
