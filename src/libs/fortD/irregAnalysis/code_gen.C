/* $Id: code_gen.C,v 1.18 1997/03/11 14:28:38 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/***********************************************************************
 * The functions in this file contain the code generation routines
 * for handling irregular problems (inspectors, executors etc.)
 *
 * This file contains ONLY functions which actually manipulate the AST.
 * Whenever the AST is modified by a function in any of the "irreg"
 * files, one should STRONGLY consider abstracting the modification
 * action into a separate function and putting that function into this
 * file.
 * Before writing a new function for doing low level manipulations, you
 * might check in pt_util.c that a similar function does not exist yet.
 */

/**********************************************************************
 * Revision History:
 * $Log: code_gen.C,v $
 * Revision 1.18  1997/03/11 14:28:38  carr
 * newly checked in as revision 1.18
 *
Revision 1.18  94/03/21  13:35:37  patton
fixed comment problem

Revision 1.17  94/02/27  20:15:32  reinhard
Added value-based distributions.
Make CC happy.
See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.

 * Revision 1.22  1994/02/27  19:47:27  reinhard
 * Added parameter to fd_ssa_init().
 *
 */

#include <assert.h>
#include <iostream.h>
#include <strstream.h>
#include <string.h>
#include <stdlib.h>

#include <libs/fortD/irregAnalysis/analyse.h>
#include <include/frontEnd/astcons.h>
#include <libs/frontEnd/ast/astlist.h>
#include <include/frontEnd/astnode.h>
#include <include/frontEnd/astrec.h>
#include <libs/frontEnd/include/walk.h>
#include <libs/moduleAnalysis/dependence/utilities/strong.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>
#include <libs/moduleAnalysis/cfg/Cfg.h>
#include <libs/fortD/localInfo/fd_symtable.h>
#include <libs/fortD/misc/fd_types.h>

/*------------------- GLOBAL DECLARATIONS -------------------*/

EXTERN(AST_INDEX, begin_ast_stmt,   (AST_INDEX node));
EXTERN(AST_INDEX, cfg_node_to_ast_func, (CfgInstance    cfg,
					 CfgNodeId      cn,
					 Boolean        prepend,
					 ListInsertFunc &insert_func,
					 Boolean skip_comments = true));
EXTERN(AST_INDEX, cfg_node_to_nearest_ast, (CfgInstance cfg,
					    CfgNodeId   cn,
					    Boolean     postdom));
EXTERN(AST_INDEX, cfg_node_to_predom_ast, (CfgInstance cfg,
					  CfgNodeId   cn));
EXTERN(AST_INDEX, cfg_node_to_postdom_ast, (CfgInstance cfg,
					   CfgNodeId   cn));
EXTERN(AST_INDEX, find_stmt_list, (AST_INDEX node));
EXTERN(Boolean,   is_list_or_NIL, (AST_INDEX node));
EXTERN(const char *,    loop_gen_ivar,  (AST_INDEX node));
EXTERN(AST_INDEX, pt_gen_comment, (char *str));
EXTERN(int,       tarj_level2,     (TarjTree tarjans, CfgNodeId cn));


/*------------------ LOCAL DECLARATIONS ---------------------*/

EXTERN(AST_INDEX, mergeStmts, (AST_INDEX node1, AST_INDEX node2));
EXTERN(void,      try_fuse_loop_list, (AST_INDEX node));
EXTERN(Boolean,   test_fuse, (AST_INDEX loop1_node,
			      AST_INDEX loop2_node));
EXTERN(AST_INDEX, copy_loop, (AST_INDEX loop_node, const char *aux_name,
			      const char *&iter_name, AST_INDEX &body_node));
EXTERN(AST_INDEX, copy_loop_nest, (CfgInstance cfg,
				   AST_INDEX subsc_node,
				   AST_INDEX limit_node,
				   AST_INDEX &body_node,
				   Boolean &need_aux));
EXTERN(AST_INDEX, copy_loop_nest_simple, (CfgInstance cfg,
					  AST_INDEX subsc_node,
					  AST_INDEX limit_node,
					  AST_INDEX &body_node));
EXTERN(AST_INDEX, add_else,    (AST_INDEX node));
EXTERN(AST_INDEX, if2if_endif, (AST_INDEX node));
EXTERN(void,      add_aux, (AST_INDEX loop_node, const char* aux_name));
EXTERN(void,      add_count_to_nest, (AST_INDEX loop_node,
				      AST_INDEX body_node,
				      const char *aux_name,
				      AST_INDEX inc_node =
				      pt_gen_int(1)));
EXTERN(AST_INDEX, genCall, (const char *func_name, AST_INDEX args_list));
EXTERN(AST_INDEX, pt_gen_empty_comment, ());
EXTERN(AST_INDEX, prepend_blank_line, (AST_INDEX node));
EXTERN(AST_INDEX, append_blank_line, (AST_INDEX node));
EXTERN(AST_INDEX, genComment, (const char *cmt_str));
EXTERN(AST_INDEX, genLongComment, (const char *cmt_str));
EXTERN(AST_INDEX, prependLongComment, (const char *cmt_str,
				       AST_INDEX node));
EXTERN(void,      appendLongComment, (const char *cmt_str,
				      AST_INDEX node));
EXTERN(void,      attach_cfg_node, (CfgInstance cfg,
				    CfgNodeId   cn,
				    AST_INDEX   new_node,
				    Boolean     prepend,
				    Boolean skip_comments = true));
EXTERN(void,      prepend_cfg_node, (CfgInstance cfg,
				     CfgNodeId   cn,
				     AST_INDEX   new_node));
EXTERN(void,      append_cfg_node, (CfgInstance cfg,
				    CfgNodeId   cn,
				    AST_INDEX   new_node));
EXTERN(void,      insert_flags,    (AST_INDEX root_node,
				    Boolean *flags));


/*------------------ LOCAL CONSTANTS ------------------------*/

// Max length of single comment line
static const int MAX_COMMENT_LINE = 60; 


/**********************************************************************
 * mergeStmts()   Given two lists of stmts, merge them together.
 *                This should include loop fusion, redundant stmt
 *                elimination, etc.
 *                This is used both for merging the <rank> slices of
 *                a single reference and for merging the slices
 *                of different references.
 *                We assume that there are no loop carried dependences.
 */
AST_INDEX
mergeStmts(AST_INDEX node1, AST_INDEX node2)
{
  AST_INDEX merged_node;
  
  assert(is_list_or_NIL(node1) && is_list_or_NIL(node2));
  
  merged_node = list_append(node1, node2);
  try_fuse_loop_list(merged_node);          // Try simple loop fusion
  
  return merged_node;
}


/**********************************************************************
 * try_fuse_loop_list()  Given list of stmts, determine whether it
 *                       contains 2 loops; if so, try to fuse them.
 */
void
try_fuse_loop_list(AST_INDEX node)
{
  AST_INDEX loop1_node, loop2_node, body1_node, body2_node;

  if (is_list(node) && (list_length(node) == 2)) {
    loop1_node = list_retrieve(node, 1);
    loop2_node = list_retrieve(node, 2);
    if (test_fuse(loop1_node, loop2_node)) {
      body1_node = gen_DO_get_stmt_LIST(loop1_node);
      body2_node = tree_copy(gen_DO_get_stmt_LIST(loop2_node));
      body1_node = list_append(body1_node, body2_node);
      gen_DO_put_stmt_LIST(loop1_node, body1_node);
      (void) list_remove_node(loop2_node);
      tree_free(loop2_node);
    }
  }
}


/**********************************************************************
 * test_fuse()  Test whether these are two loops which can be fused.
 *              For now, just compare whether the headers are identical.
 * NOTE: this does NOT test for dependences.
 */
Boolean
test_fuse(AST_INDEX loop1_node, AST_INDEX loop2_node)
{
  Boolean result;

  result = (is_do(loop1_node) && is_do(loop2_node)) ? 
    (ast_equiv(gen_DO_get_control(loop1_node),
	       gen_DO_get_control(loop2_node))) : false;

  return result;
}


/**********************************************************************
 * copy_loop()  Make a copy of <orig_loop_node> loop.
 *              If not in normal form: generate auxiliary induction
 *              variable (AIV).
 *              020493RvH: This function should probably go away, but 
 *              some if its guts may be reused in the slice generation.
 *             
 */
AST_INDEX                             // The resulting AST
copy_loop(AST_INDEX  loop_node,       // The given loop
	  const char *aux_name,       // Name of AIV (if needed)
	  const char *&iterator_name, // Normal form loop iterator
	  AST_INDEX  &body_node)      // Body of generated loop
{
  AST_INDEX node, stmts_node, control_node, new_loop_node;
  Boolean   is_normal;

  stmts_node    = list_create(AST_NIL); // Start a list of stmts
  is_normal     = pt_loop_is_normal(loop_node);
  iterator_name = is_normal ? loop_gen_ivar(loop_node) : aux_name;

  if (!is_normal) {                       // Generate "i$ = 0"
    node       = gen_ASSIGNMENT(AST_NIL, pt_gen_ident((char*)aux_name),
				pt_gen_int(0));
    stmts_node = list_insert_last(stmts_node, node); 
  }

  // Generate identical loop with given body
  control_node  = tree_copy(gen_DO_get_control(loop_node));

  // Start w/ empty loop body
  body_node     = list_create(AST_NIL);
  new_loop_node = gen_DO(AST_NIL, AST_NIL, AST_NIL, control_node,
			 body_node);
  stmts_node    = list_insert_last(stmts_node, new_loop_node); 

  if (!is_normal) {                       // Generate "i$ = i$ + 1"
    node = gen_ASSIGNMENT(AST_NIL, pt_gen_ident((char*)aux_name),
			  pt_gen_add(pt_gen_ident((char*)aux_name),
				     pt_gen_int(1)));
    body_node = list_insert_first(body_node, node);
  }

  return stmts_node;
}


/**********************************************************************
 * copy_loop_nest()  Copy the loop nest headers surrounding
 *                   <subsc_node>, up to <lim_node>.
 */
AST_INDEX                               // Generated copy
copy_loop_nest(CfgInstance cfg,         // Control flow graph
	       AST_INDEX   subsc_node,  // Subscript within loop nest
	       AST_INDEX   lim_node,    // Header of loop nest
	       AST_INDEX   &body_node,  // Body of loop surrounding <subsc_node>
	       Boolean     &need_aux)   // Do we need an aux ind var ?

{
  AST_INDEX control_node;
  Boolean   loop_normal_flag = true;
  AST_INDEX stmts_node = AST_NIL;
  AST_INDEX loop_node  = lim_node;
  TarjTree  tarjans    = cfg_get_intervals(cfg);
  AST_INDEX stmt_node  = begin_ast_stmt(subsc_node);
  CfgNodeId cur_id     = cfg_node_from_ast(cfg, stmt_node);
  int       cur_level  = tarj_level2(tarjans, cur_id);
  CfgNodeId lim_id     = cfg_node_from_ast(cfg, lim_node);
  int       lim_level  = tarj_level2(tarjans, lim_id);

  body_node = AST_NIL;
  need_aux  = false;

  if (!is_do(lim_node))   // Make sure we are handed a loop to copy
  {
    cerr << "WARNING copy_loop_nest(): lim_node " << lim_node
      << " is not a do loop !\n";
  }
  else if (subsc_node == lim_node)
  {
    cerr << "WARNING copy_loop_nest(): subsc_node == lim_node == "
      << lim_node << "!\n";
  }
  else
  {
    // We need an auxiliary induction var if we have multiple loops
    // or one loop that is not in normal form.
    need_aux = (Boolean) (lim_level < cur_level - 1);

    // Start a list of stmts
    body_node  = list_create(AST_NIL);
    stmts_node = body_node;

    while (cur_level > lim_level)
    {
      // Walk outward on the interval tree
      cur_id = tarj_outer(tarjans, cur_id);
      cur_level--;
      
      // Retrieve the AST_INDEX corresponding to cfg node <cur_id>
      loop_node = cfg_node_to_ast(cfg, cur_id);

      // Make sure that our loop is a DO loop
      if (is_do(loop_node))
      {
        // Need aux induction var if loop is not in normal form
        need_aux     = (Boolean) (need_aux
				  && pt_loop_is_normal(loop_node));
        
        // Generate identical loop with given body
        control_node = tree_copy(gen_DO_get_control(loop_node));

        // Wrap loop around <stmts_node>
        stmts_node   = gen_DO(AST_NIL, AST_NIL, AST_NIL, control_node,
                              stmts_node);
      }
      else
      {
        cerr << "WARNING copy_loop_nest(): " <<
          "Attempt to copy non-DO loop, AST_INDEX = " << loop_node <<
            ".\n";
      }
    }

    // Make sure we ended up at the right loop level
    if (lim_node != loop_node)
    {
      cerr << "WARNING copy_loop_nest(): lim_node = " <<
        lim_node << " != loop_node = " << loop_node << ".\n";
    }
  }

  return stmts_node;
}


/**********************************************************************
 * copy_loop_nest_simple()  Copy the loop nest headers surrounding
 *                          <subsc_node>, up to <lim_node>,
 *                          do not return <need_aux>
 */
AST_INDEX
copy_loop_nest_simple(CfgInstance cfg,  // Control flow graph
	       AST_INDEX   subsc_node,  // Subscript within loop nest
	       AST_INDEX   lim_node,    // Header of loop nest
	       AST_INDEX   &body_node)  // Body of loop surrounding <subsc_node>
{
  Boolean   dummy_bool;
  AST_INDEX node;

  node = copy_loop_nest(cfg, subsc_node, lim_node, body_node,
			dummy_bool);

  return node;
}


/**********************************************************************
 * add_else()  Add else branch to if-endif stmt
 */
AST_INDEX
add_else(AST_INDEX node)
{
  AST_INDEX else_node, guard_node;
  
  else_node  = list_create(AST_NIL);      // No stmts yet
  //guard_node = gen_GUARD(AST_NIL,        // No label
	//		 AST_NIL,        // No test
	//		 else_node);
  guard_node = gen_GUARD(AST_NIL, AST_NIL, else_node);
  list_insert_after(node, guard_node);

  return else_node;
}


/**********************************************************************
 * if2if_endif()  Convert a logical if to if-endif block
 */
AST_INDEX
if2if_endif(AST_INDEX node)
{
  AST_INDEX stmt_list, rval_node, guard_node, label_node;
  AST_INDEX new_stmt_list, new_node;

  if (is_logical_if(node))
  {
    // Copy whatever we need from the old LOGICAL_IF
    rval_node  = tree_copy(gen_LOGICAL_IF_get_rvalue(node));
    stmt_list  = tree_copy(gen_LOGICAL_IF_get_stmt_LIST(node));
    label_node = tree_copy(gen_LOGICAL_IF_get_lbl_def(node));
    
    // Construct new IF
    guard_node    = gen_GUARD(AST_NIL, rval_node, stmt_list);
    new_stmt_list = list_create(guard_node);
    new_node      = gen_IF(label_node, AST_NIL, new_stmt_list);

    // Swap LOGICAL_IF, IF in source text (this deletes node)
    pt_tree_replace(node, new_node);
  }
  else
  {
    cerr << "WARNING if2if_endif(): " <<
      "expected a logical if; <node> = " << node << ".\n";

    new_node = AST_NIL;
  }

  return new_node;
}


/**********************************************************************
 * add_aux()  Add auxiliary induction variable <aux_name> to <loop>.
 */
void
add_aux(AST_INDEX loop_node, const char *aux_name)
{
  AST_INDEX node, body_node;

  // Generate "i$ = 0"
  node = gen_ASSIGNMENT(AST_NIL, pt_gen_ident((char*)aux_name),
			pt_gen_int(0));
  (void) list_insert_before(loop_node, node);

  // Generate "i$ = i$ + 1"
  body_node = gen_DO_get_stmt_LIST(loop_node);
  node      = gen_ASSIGNMENT(AST_NIL, pt_gen_ident((char*)aux_name),
			     pt_gen_add(pt_gen_ident((char*)aux_name),
					pt_gen_int(1)));
  (void) list_insert_first(body_node, node);
}


/**********************************************************************
 * add_count_to_nest()  Add an auxiliary induction variable to a loop
 *                      nest.
 */
void
add_count_to_nest(AST_INDEX loop_node, AST_INDEX body_node,
		  const char *aux_name, AST_INDEX inc_node)
{
  AST_INDEX node;

  // Generate "i$ = 0"
  node = gen_ASSIGNMENT(AST_NIL, pt_gen_ident((char*)aux_name),
			pt_gen_int(0));
  if (loop_node == AST_NIL)
  {
    cerr << "WARNING add_count_to_nest(): loop_node = AST_NIL.\n";
  }
  else
  {
    (void) list_insert_before(loop_node, node);
  }

  // Generate "i$ = i$ + 1"
  node = gen_ASSIGNMENT(AST_NIL, pt_gen_ident((char*)aux_name),
			pt_gen_add(pt_gen_ident((char*)aux_name),
				   inc_node));
  (void) list_insert_first(body_node, node);
}


/**********************************************************************
 * genCall()   Given a function name and a list of arguments,
 *             generate a call node.
 */
AST_INDEX
genCall(const char *func_name, AST_INDEX args_list)
{
  AST_INDEX func_name_node;

  func_name_node = gen_IDENTIFIER();
  gen_put_text(func_name_node, (char*)func_name, STR_IDENTIFIER);
  return gen_CALL(AST_NIL , gen_INVOCATION(func_name_node , args_list));
}


/**********************************************************************
 * pt_gen_empty_comment()  Generate an empty comment.
 */
AST_INDEX
pt_gen_empty_comment()
{
  //AST_INDEX result = pt_gen_comment("");
  AST_INDEX result = gen_COMMENT(AST_NIL);

  return result;
}


/**********************************************************************
 * prepend_blank_line()  Prepend blank line before stmt if needed.
 */
AST_INDEX
prepend_blank_line(AST_INDEX node)
{
  AST_INDEX result;

  result = (!is_comment(node) ||
	    (gen_COMMENT_get_text(node) != AST_NIL)) ?
	      list_insert_before(node, pt_gen_empty_comment()) : node;

  return result;
}


/**********************************************************************
 * append_blank_line()  Append blank line before stmt if needed.
 */
AST_INDEX
append_blank_line(AST_INDEX node)
{
  AST_INDEX result;

  result = (!is_comment(node) ||
	    (gen_COMMENT_get_text(node) != AST_NIL)) ?
	      list_insert_after(node, pt_gen_empty_comment()) : node;

  return result;
}


/**********************************************************************
 * genComment()  Generate comment from string
 */
AST_INDEX
genComment(const char *cmt_str)
{
  AST_INDEX cmt_node;
  char *line_str;
  int line_len;

  line_len = strlen(cmt_str);
  line_str = new char[line_len+1];
  strcpy(line_str, cmt_str);
  cmt_node = pt_gen_comment(line_str);

  return cmt_node;
}


/**********************************************************************
 * genLongComment()  Generate comment from string, break into multiple
 *                   lines if necessary
 *                   Note: This always returns a LIST of nodes
 */
AST_INDEX
genLongComment(const char *cmt_str)
{
  AST_INDEX cmt_node, cmt_list;
  char      *line_str;
  int       line_len;

  cmt_list = list_create(AST_NIL);
  do {
    line_len = min(strlen(cmt_str), MAX_COMMENT_LINE);   // One line
    line_str = new char[line_len+1];                     // of comment
    strncpy(line_str, cmt_str, line_len);
    line_str[line_len] = '\0';                           // Terminator
    cmt_node = pt_gen_comment(line_str);
    list_insert_last(cmt_list, cmt_node);
    cmt_str = &cmt_str[line_len];
  } while (strlen(cmt_str) > 0);

  return cmt_list;
}


/**********************************************************************
 * prependLongComment()  Generate comment from string,
 *                       break it into multiple lines if necessary,
 *                       and prepend it to <node>
 */
AST_INDEX
prependLongComment(const char *cmt_str, AST_INDEX node)
{
  AST_INDEX cmt_list;
  AST_INDEX prependee = AST_NIL;

  if (node == AST_NIL) {
    cerr << "WARNING prependLongComment(): " <<
      "Attempt to prepend \"" << cmt_str << "\" to AST_NIL.";
  } else {
    cmt_list = genLongComment(cmt_str);
    prependee = list_insert_before(node, cmt_list);
  }

  return prependee;
}


/**********************************************************************
 * appendLongComment()  Generate comment from string,
 *                      break it into multiple lines if necessary,
 *                      and append it to <node>
 */
void
appendLongComment(const char *cmt_str, AST_INDEX node)
{
  AST_INDEX cmt_list;

  if (node == AST_NIL) {
    cerr << "WARNING appendLongComment(): " <<
      "Attempt to append \"" << cmt_str << "\" to AST_NIL.";
  } else {
    cmt_list = genLongComment(cmt_str);
    list_insert_after(node, cmt_list);
  }
}


/**********************************************************************
 * attach_cfg_node()  Prepend or append <node> to the AST
 *                    corresponding to <cn>
 */
void
attach_cfg_node(CfgInstance cfg,
		CfgNodeId   cn,
		AST_INDEX   new_node,
		Boolean     prepend,
		Boolean     skip_comments)
{
  ListInsertFunc insert_func;
  AST_INDEX      node;

  
  node = cfg_node_to_ast_func(cfg, cn, prepend, insert_func,
			      skip_comments);

  if (node == AST_NIL)
  {
    cerr << "WARNING attach_cfg_node(): found no AST node, cn = " <<
      cn << ", prepend = " << prepend << ".\n";
  }
  else
  {
    (void) insert_func(node, new_node);
  }
}


/**********************************************************************
 * prepend_cfg_node()  Prepend <node> to the AST corresponding to <cn>
 */
void
prepend_cfg_node(CfgInstance cfg,
		 CfgNodeId   cn,
		 AST_INDEX   new_node)
{
  AST_INDEX node;

  node = cfg_node_to_postdom_ast(cfg, cn);  // Try to map <cn> to <node>

  if (node == AST_NIL) {
    cerr << "WARNING prepend_cfg_node(): found no AST node.\n";
  }
  else
  {
    // Handle if-branch-guards correctly
    if (is_guard(node)) {
      
      // Is it a guarded branch (ie, no ELSE-header) ?
      if (gen_GUARD_get_rvalue(node) != AST_NIL)
      {
	// Prepend to the surrounding IF
	node = ast_get_father(ast_get_father(node));
	if (!is_if(node)) {
	  cerr << "WARNING prepend_cfg_node(): " <<
	    "found GUARD but no IF; cn = " << cn << ".\n";
	}
	else
	{
	  (void) list_insert_before(node, new_node);
	}
      }
      else
      {
	// Make it first stmt in branch
	node = gen_GUARD_get_stmt_LIST(node);
	(void) list_insert_first(node, new_node);
      }
    }
    else
    {
      // No branch => insert before this node
      (void) list_insert_before(node, new_node);
    }
  }
}


/**********************************************************************
 * append_cfg_node()  Append <node> to the AST corresponding to <cn>
 */
void
append_cfg_node(CfgInstance cfg,
		CfgNodeId   cn,
		AST_INDEX   new_node)
{
  AST_INDEX node;

  node = cfg_node_to_predom_ast(cfg, cn);  // Try to map <cn> to <node>

  if (node == AST_NIL) {
    cerr << "WARNING append_cfg_node(): found no AST node.\n";
  }
  else
  {
    // Handle if-branch-guards correctly
    if (is_guard(node)) {
      
      // Is it a guarded branch (ie, no ELSE-header) ?
      if (gen_GUARD_get_rvalue(node) != AST_NIL)                             {
	// Prepend to the surrounding IF
	node = ast_get_father(ast_get_father(node));
	if (!is_if(node)) {
	  cerr << "WARNING append_cfg_node(): " <<
	    "found GUARD but no IF; cn = " << cn << ".\n";
	}
	else
	{
	  (void) list_insert_before(node, new_node);
	}
      }
      else
      {
	// Make it first stmt in branch
	node = gen_GUARD_get_stmt_LIST(node);
	(void) list_insert_first(node, new_node);
      }
    }
    else
    {
      (void) list_insert_after(node, new_node);
    }
  }
}


/**********************************************************************
 * insert_flags()  Insert a comment of the form
 *
 *            C     --<< do_all_arrays: true, split_comm: true >>--
 */
void
insert_flags(AST_INDEX root_node, Boolean *flags)
{
  ostrstream cmt_buf;
  char       *cmt_str;
  AST_INDEX  cmt_list;
  int        i;
  int        first_flag = (int) Skip_irreg;
  int        last_flag  = (int) Gen_high_level;

  cmt_buf << "--<< OPTIONS: ";
  for (i = first_flag; i <= last_flag; i++)
  {
    if (i > 0) {
      cmt_buf << ", ";
    }
    cmt_buf << fd_flags_names[i] << ": " << flags[i];
  }
  cmt_buf << " >>--" << ends;
  cmt_str  = cmt_buf.str();
  cmt_list = genLongComment(cmt_str);
  delete cmt_str;
  list_insert_first(find_stmt_list(root_node), cmt_list);
}
