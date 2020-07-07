/* $Id: Slice2.C,v 1.8 1997/06/24 17:38:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*********************************************************************
 * class Slice member functions
 */

/**********************************************************************
 * Revision History:
 * $Log: Slice2.C,v $
 * Revision 1.8  1997/06/24 17:38:57  carr
 * Support 64-bit Pointers
 *
 * Revision 1.7  1997/03/11  14:28:35  carr
 * newly checked in as revision 1.7
 *
Revision 1.7  94/03/21  13:55:43  patton
fixed comment problem

Revision 1.6  94/02/27  20:15:02  reinhard
Added value-based distributions.
Make CC happy.
See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.

 * Revision 1.10  1994/02/27  19:44:55  reinhard
 * Tweaks to make CC happy.
 *
 * Revision 1.9  1994/01/18  19:51:32  reinhard
 * Updated include paths according to Makefile change.
 * Handle value-based distributions.
 *
 * Revision 1.8  1993/10/04  15:40:25  reinhard
 * Separated counting var's from aux ind var's.
 *
 * Revision 1.7  1993/09/25  23:03:14  reinhard
 * Made counting slices more robust.
 */

#include <assert.h>
#include <iostream>
#include <string.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>
#include <libs/moduleAnalysis/valNum/Val.h>
#include <libs/frontEnd/ast/astlist.h>
#include <include/frontEnd/astnode.h>
#include <include/frontEnd/astrec.h>
#include <libs/fortD/irregAnalysis/Exec.h>
#include <libs/fortD/irregAnalysis/Inspec.h>
#include <libs/fortD/irregAnalysis/IrrGlobals.h>
#include <libs/fortD/irregAnalysis/IrrSymTab.h>
#include <libs/fortD/irregAnalysis/S_desc.h>
#include <libs/fortD/irregAnalysis/Slice.h>
#include <libs/fortD/irregAnalysis/analyse.h>
#include <libs/fortD/irregAnalysis/ValDecomp.h>

using namespace std;
/*------------------- GLOBAL DECLARATIONS -------------------*/

EXTERN(void,      add_count_to_nest, (AST_INDEX loop_node,
				      AST_INDEX body_node,
				      const char *aux_name,
				      AST_INDEX inc_node =
				      pt_gen_int(1)));
EXTERN(AST_INDEX, assert_father_is_subscript, (AST_INDEX node));
EXTERN(void,      cat_loop_triplet,  (AST_INDEX loop_node,
				      ostrstream &buf));
EXTERN(AST_INDEX, copy_loop_nest,    (CfgInstance cfg,
				      AST_INDEX subsc_node,
				      AST_INDEX limit_node,
				      AST_INDEX &body_node,
				      Boolean &need_aux));
EXTERN(AST_INDEX, copy_loop_nest_simple, (CfgInstance cfg,
					  AST_INDEX subsc_node,
					  AST_INDEX limit_node,
					  AST_INDEX &body_node));
EXTERN(int,       count_nest_iters, (CfgInstance cfg,
				     AST_INDEX   subsc_node,
				     AST_INDEX   lim_node));
EXTERN(AST_INDEX, fd_ssa_slice, (CfgInstance cfg, AST_INDEX start_node,
				 AST_INDEX stop_node));
EXTERN(AST_INDEX, find_enclosing_DO, (AST_INDEX node));
EXTERN(AST_INDEX, genLongComment,    (const char *cmt_str));
EXTERN(char *,    prefix_type,       (int type, const char *str));
EXTERN(AST_INDEX, pt_loop_gen_cnt_node, (AST_INDEX loop_node));


/**********************************************************************
 * genStmts_node()  Find the Slice.
 *
 * Assuming an occurrence, OCC, like x(n1), we look for the stmts 
 * contributing to the subscript, like n1.  A nontrivial question seems
 * to be: where should this backwards inspection of the program stop ?
 * For now we might say that this Slice contains
 *
 * a) For an rhs occurrence (USE of x):
 *    the contributing to n1 ("the n1-Slice") from the last definition 
 *    of x up to OCC.
 *
 * b) For an lhs occurrence (DEF of x):
 *    the n1-Slice from the last definition of x, the outermost loop,
 *    or the beginning of the procedure (whatever occurs last) up to 
 *    OCC.
 *
 * However, in both cases, and particularly in b), this heuristic might
 * fail for (may be even not so) bizarre cases, we (ie. Raja) will see.
 * One interesting case is <ref->rank> > 1 (like x(n1,n2,n3)),
 * where each subscript needs its own slice.  For now these slices are
 * lumped together here, since we ultimately need one single schedule
 * for this OCC, but for abstraction purposes we might want to 
 * separate the slice generation for the different subscripts later.
 * DONE!
 */
AST_INDEX
Slice::genStmts_node()
{
  AST_INDEX node, loop_node, count_node;
  AST_INDEX body_node = AST_NIL;

  if (stmts_node != AST_NIL)  // We already have a slice ?
  {
    cerr << "WARNING Slice::genStmts_node(): stmts_node = " <<
      stmts_node << " != AST_NIL; subsc_node = " << subsc_node <<
	".\n";
  }
  else
  {
    // 9/29/93 RvH: Only DO-loops until Raja puts his stuff in
    if (xpnd_flag)             // Subscript expansion ?
    {
      if (is_do(limit_node))
      {
        /*** Raja's slice generation here; for now: smoke + mirrors ***/
        
        // Generate a loop header, starting at <limit_node>
        loop_node = copy_loop_nest(di->getCfg(), subsc_node, limit_node,
                                   body_node, aux_flag);

        // Start a list of stmts
        stmts_node = list_create(loop_node);
        
        // Update ON_HOME information
        // 6/16/93 RvH: currently disabled for mergeStmts
        //di->getOn_home_table()->add_directive(subsc_node, loop_node);

        // Computations within this loop leading to this subscript
        // Delete this when di->getSsa_graph()->preds starts working
        node      = fd_ssa_slice(di->getCfg(), subsc_node, limit_node);
        (void) list_insert_first(body_node, node);
        // Expect SORTED list of AST indices
        //preds      = di->getSsa_graph()->preds(subsc_node);
        //for(AST_Set_Iter iter(*preds); node = iter();) {
        //  body_node  = list_append(body_node, node);
        //}
        //delete preds;
        
        // "n1$arr(i$) = n1" (or "n1$arr(i) = n1")
        node = gen_SUBSCRIPT(pt_gen_ident((char*)getNew_subsc_glob_name()),
                             list_create(pt_gen_ident((char*)getInd_name())));
        node = gen_ASSIGNMENT(AST_NIL, node, tree_copy(subsc_node));
        (void) list_insert_last(body_node, node);

        // Do we need auxiliary induction variable ?
        if (aux_flag)
        {
          // Annotate <node> with "i$cnt = 1", "i$cnt = i$cnt + 1"
          add_count_to_nest(loop_node, body_node, getInd_name());
        }
        
        // Do we need a counting slice ?
        if (new_subsc_size == UNKNOWN_SIZE)
        {
          count_node = gen_count_slice();
          stmts_node = list_insert_first(stmts_node, count_node);  
        }
      }
      else
      {
        stmts_node = genLongComment("--<< WARNING: Could not generate slice for non-DO loop >>--");
      }
    }
    else
    {
      cerr << "WARNING Slice::genStmts_node(): " <<
	"should not expand for this subscript; subsc_node = " <<
	  subsc_node << ".\n";
    }
  }

  return stmts_node;
}


/**********************************************************************
 * gen
 */
AST_INDEX
Slice::gen_count_slice()
{
  AST_INDEX  node, loop_node, body_node, count_node, iter_cnt_node;
  ostrstream cmt_buf;
  char       *cmt_str;
  char       *function_name;
  int        type = TYPE_INTEGER;

  // Generate comment "--<< Counting slice for i$cnt >>--"
  cmt_buf << "--<< Counting slice for " << getCnt_name() << " >>--" << ends;
  cmt_str    = cmt_buf.str();
  count_node = genLongComment(cmt_str);
  delete cmt_str;

  // Find innermost loop enclosing subscript
  loop_node = find_enclosing_DO(subsc_node);

  if (is_do(loop_node))
  {
    // Generate expression for the # of loop iterations; "inb(i)"
    iter_cnt_node = pt_loop_gen_cnt_node(loop_node);
  }
  else
  {
    cerr << "WARNING Slice::gen_count_slice(): could not determine "
      << "# of iterations for non-do loop enclosing \""
	<< ref->getStr() << "\"; subsc_node = " << subsc_node
	  << ", loop_node = " << loop_node << ".\n";
    iter_cnt_node = pt_gen_int(7777);
  }

  // Do we have a single loop ?
  if (loop_node == limit_node)
  {
    // Generate "i$cnt = inb(i)"
    node  = gen_ASSIGNMENT(AST_NIL,
                                pt_gen_ident((char*)getCnt_name()),
                                iter_cnt_node);
    count_node = list_insert_last(count_node, node);
  }
  else
  {
    if (is_do(limit_node))
    {
      // Copy enclosing loop nest, except the innermost loop
      loop_node  = copy_loop_nest_simple(di->getCfg(), loop_node,
					 limit_node, body_node);
      count_node = list_insert_last(count_node, loop_node);

      // Update ON_HOME information
      // 6/16/93 RvH: currently disabled for mergeStmts
      //di->getOn_home_table()->add_directive(subsc_node, loop_node);

      // Annotate <count_node> with "i$cnt = 1", "i$cnt = i$cnt + inb(i)"
      add_count_to_nest(loop_node, body_node, getCnt_name(), iter_cnt_node);
    }
    else
    {
      cerr << "WARNING Slice::gen_count_slice(): could not determine "
	<< "# of iterations for non-do loop enclosing \""
	  << ref->getStr() << "\"; subsc_node = " << subsc_node
	    << ", loop_node = " << limit_node << ".\n";

      // Generate "i$cnt = 7777"
      loop_node  = gen_ASSIGNMENT(AST_NIL,
				  pt_gen_ident((char*)getCnt_name()),
				  pt_gen_int(7777));
      count_node = list_insert_last(count_node, loop_node);
    }
  }

  // Generate comment "--<< ALLOCATE (j$arr, j$cnt) >>--"
  //cmt_buf << "--<< ALLOCATE (" << new_subsc_name << ", " <<
  //  getCnt_name() << ") >>--" << ends;
  //cmt_str = cmt_buf.str();
  //node    = genLongComment(cmt_str);
  //delete cmt_str;

  // Generate "call ialloc(j$glob, j$cnt)"
  function_name = prefix_type(type, "alloc");
  node       = list_create(pt_gen_ident((char*)getNew_subsc_glob_name()));
  node       = list_insert_last(node,
				pt_gen_ident((char*)getCnt_name()));
  node       = pt_gen_call(function_name, node);
  count_node = list_insert_last(count_node, node);

  // Generate "call ialloc(j$loc, j$cnt)"
  node       = list_create(pt_gen_ident((char*)getNew_subsc_loc_name()));
  node       = list_insert_last(node,
				pt_gen_ident((char*)getCnt_name()));
  node       = pt_gen_call(function_name, node);
  count_node = list_insert_last(count_node, node);

  // Generate "--<< END Counting slice >>--"
  node       = genLongComment("--<< END Counting slice >>--");
  count_node = list_insert_last(count_node, node);

  // Note that we now need dynamic integer arrays
  di->setNeed_mall(type);

  return count_node;
}
