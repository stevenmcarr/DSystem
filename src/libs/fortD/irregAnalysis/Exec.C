/* $Id: Exec.C,v 1.13 1997/03/11 14:28:27 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*********************************************************************
 * class Inspec member functions
 */

/**********************************************************************
 * Revision History:
 * $Log: Exec.C,v $
 * Revision 1.13  1997/03/11 14:28:27  carr
 * newly checked in as revision 1.13
 *
Revision 1.13  94/03/21  14:04:57  patton
fixed comment problem
,

Revision 1.12  94/02/27  20:14:23  reinhard
Added value-based distributions.
Make CC happy.
See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.

 * Revision 1.19  1994/02/27  19:40:11  reinhard
 * Moved from Str_ht to NamedGenericTable.
 *
 * Revision 1.18  1994/01/18  19:46:22  reinhard
 * Updated include paths according to Makefile change.
 *
 * Revision 1.17  1993/10/06  18:26:04  reinhard
 * Made exec's based on Slices instead of S_desc's.
 *
 */

#include <assert.h>

#include <libs/frontEnd/ast/AstIter.h>
#include <libs/frontEnd/ast/groups.h>
#include <libs/support/tables/NamedGeneric.h>
#include <libs/support/sets/Sparse_Set.h>
#include <libs/fortD/irregAnalysis/Exec.h>
#include <libs/fortD/irregAnalysis/IrrGlobals.h>
#include <libs/fortD/irregAnalysis/IrrSymTab.h>
#include <libs/fortD/irregAnalysis/OnHomeTable.h>
#include <libs/fortD/irregAnalysis/S_desc.h>
#include <libs/fortD/irregAnalysis/Slice.h>
#include <libs/fortD/irregAnalysis/analyse.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>


/*------------------- GLOBAL DECLARATIONS -------------------*/

EXTERN(void,      add_count_to_nest, (AST_INDEX loop_node,
				      AST_INDEX body_node,
				      const char *aux_name,
				      AST_INDEX inc_node =
				      pt_gen_const("1")));
EXTERN(AST_INDEX, begin_ast_stmt, (AST_INDEX node));
EXTERN(AST_INDEX, genLongComment, (const char *cmt_str));
EXTERN(AST_INDEX, loop_gen_body_last,  (CfgInstance cfg,
                                        AST_INDEX loop_node));
EXTERN(const char *, loop_gen_ivar,  (AST_INDEX node));
//EXTERN(int,       tarj_level2,    (TarjTree tarjans, CfgNodeId cn));


/*------------------- LOCAL DECLARATIONS --------------------*/

EXTERN(Generic,   genExec, (AST_INDEX target_node, Generic gen_arg));


/**********************************************************************
 * Constructor
 */
Exec::Exec(AST_INDEX my_loop_node, IrrGlobals *my_di)
: di               (my_di),
  iter_cnt_node    (AST_NIL),
  loop_cnst_flag   (false),
  loop_node        (my_loop_node),
  loop_normal_flag (false),
  slices           (new Sparse_Set()),
  stmts_node       (my_loop_node)
{
  if (is_do(loop_node))
  {
    loop_cnst_flag = pt_loop_get_count(loop_node, &iter_cnt,
                                       &iter_cnt_node,
                                       &loop_normal_flag);
    if (!loop_cnst_flag)
    {
      iter_cnt = UNKNOWN_SIZE;
    }
  }
}


/**********************************************************************
 * Destructor
 * No deallocation of AST's yet
 */
Exec::~Exec()
{
  delete slices;
}


/**********************************************************************
 * 012993RvH: wait until templates are supported ...
 */
Generic
genExec(AST_INDEX target_node, Generic gen_arg)
{
  IrrGlobals *di   = (IrrGlobals*) gen_arg;
  Exec       *exec = new Exec(target_node, di);

  return (Generic) exec;
}


/**********************************************************************
 * add_Slice()  Note <slice> as irregular.
 */
void 
Exec::add_Slice(Slice *sl)
{
  // Make sure there are no conflicts w/ where this exec should go
  assert(loop_node == sl->getLimit_node());

  *slices += (Element) sl;
}


/**********************************************************************
 * install()  Convert loop into executor.
 */
void 
Exec::install()
{
  AST_INDEX node;

  // Make sure we have a place to put executor
  assert(loop_node != AST_NIL);

  // Sort out the given slices
  //sort_slices();

  // Generate a comment identifying executor
  node       = gen_exec_cmt();
  stmts_node = list_insert_before(stmts_node, node);

  // Convert subscripts of slices
  convert_subs();

  // Make sure we did not remove any ON_HOME directive
  di->getOn_home_table()->fixup_loop(stmts_node, loop_node);
  
  // Register the loops in this executor
  // (for example, to disable loop interchange in the reg. compiler).
  di->register_loops(loop_node);
}


/**********************************************************************
 * sort_slices()  Sort out the given slices.
 */
/*
void 
Exec::sort_slices()
{
  // Determine how many slices this insp is resonsible for
  slice_cnt = slice_set.count();

  // Allocate space for all slices
  slices = new Slice_ptr[slice_cnt];

  // Store slice in (fast) array
  for (int i = 0; i < slice_cnt; i++)
  {
    slices[i] = (Slice *) slice_set.get_entry_by_index(i);
  }
}
*/


/**********************************************************************
 * gen_exec_cmt()  Generate a comment
 *                   "--<< Executor for x(n1), ... , y(n2) >>--"
 */
AST_INDEX
Exec::gen_exec_cmt()
{
  AST_INDEX  cmt_list;
  ostrstream cmt_buf;
  char       *cmt_str;
  
  cmt_buf << "--<< Executor for ";
  catSlicesBuf(cmt_buf);
  cmt_buf << " >>--" << ends;
  cmt_str = cmt_buf.str();
  cmt_list = genLongComment(cmt_str);
  delete cmt_str;
  
  return cmt_list;
}


/**********************************************************************
 * convert_subs()  Convert subscripts of references.
 */
void 
Exec::convert_subs()
{
  AST_ht          loop2name;
  Slice           *sl;
  AST_INDEX       subsc_node, stmt_node, cur_loop_node, body_node;
  CfgNodeId       id, cur_loop_id;
  const char      *subsubsc_name;
  Sparse_Set_Iter slices_iter(*slices);
  CfgInstance     cfg     = di->getCfg();
  TarjTree        tarjans = cfg_get_intervals(cfg);

  // Iterate over slices
  //for (i = 0; i < slice_cnt; i++)
  while (sl = (Slice*) slices_iter())
  {
    // Is this an irregular slice ?
    if (sl->getXpnd_flag())
    {
      subsc_node  = sl->getSubsc_node();    // Get AST of slice
      stmt_node   = begin_ast_stmt(subsc_node);  // Find stmt of slice
      if (stmt_node == AST_NIL)
      {
        cerr << "WARNING Exec::convert_subs(): stmt_node == AST_NIL, "
          << " apparently subscript \"" << sl->getRef()->getStr()
            << "\" already got converted.\n";
      }
      else
      {
        id = cfg_node_from_ast(cfg, stmt_node);   // Find cfg node
        cur_loop_id = tarj_is_header(tarjans, id) ? id
	    : tarj_outer(tarjans, id);            // Find enclosing loop
        cur_loop_node = cfg_node_to_ast(cfg, cur_loop_id);

        // Nested loop  OR  Loop not normal ?
        if ((cur_loop_node != loop_node)
            || (!is_do(cur_loop_node))
            || (!pt_loop_is_normal(cur_loop_node)))
        {
          // We need an auxiliary induction variable
	  
          // Find out at which level we need it
          //level = tarj_level2(tarjans, cur_loop_id);

          // No aux declared for this loop yet ?
          //if (!need_aux[level])
          if (!loop2name.query_entry(cur_loop_node))
          {
            //need_aux[level] = true;
            subsubsc_name = di->getSt()->gen_fresh_int_name();
	    loop2name.add_entry(cur_loop_node, (Generic) subsubsc_name);
	    assert(di->getSt()->declInt(subsubsc_name));
            body_node = loop_gen_body_last(cfg, cur_loop_node);
            add_count_to_nest(loop_node, body_node, subsubsc_name);
          }
	  else
	  {
	    subsubsc_name = (const char*)
	      loop2name.get_entry_by_AST(cur_loop_node);
	  }
        }
        else
        {
          // Use loop var of this level as subscript of this slice
          subsubsc_name = loop_gen_ivar(loop_node);
        }

        // Update the subscript of the slice
        sl->update_subsc(subsubsc_name);
      }
    }
  }
}


/**********************************************************************
 * catSlicesBuf()  Append a string of the references of this Exec
 *               to a given buffer
 */
void
Exec::catSlicesBuf(ostrstream &slices_buf)    // Append to this buffer
{
  //Str_ht str_cnts;             // Matches string -> # of occurences of string
  NamedGenericTable str_cnts;  // Matches string -> # occurences of string
  NamedGenericTableIterator str_cntsI(&str_cnts);
  int  cnt;
  //int  str_cnt;  // # of different slice strings
  int  *data;
  Boolean    first;
  Slice      *sl;
  const char *str;
  Sparse_Set_Iter slices_iter(*slices);

  // Create set of strings for our ref
  while (sl = (Slice*) slices_iter())
  {
    //string_ht.add_entry((char *) slices[i]->getStr());
    str  = sl->getRef()->getStr();

    // Check whether we encountered <str> before
    //if (str_cnts.query_entry(str))
    if (str_cnts.QueryEntry(str))
    {
      data = (int*) str_cnts.QueryEntry(str);
      (*data)++;
    }
    else
    {
      data  = new int;
      *data = 1;
      //str_cnts.add_entry(str, (Generic) data);
      str_cnts.AddEntry(str, (Generic) data);
    }
  }

  // See how many different reference strings this executor contains
  //str_cnt = string_ht.count();

  // Allocate space for counting the refs w/ common string
  //refs_per_str = new int[str_cnt];
  //for (i = 0; i < str_cnt; refs_per_str[i++] = 0);

  // Count the refs w/ common string
  //for (i = 0; i < slice_cnt; i++)
  //{
  //  str_index = string_ht.get_entry_index((char *) slices[i]->getStr());
  //  refs_per_str[str_index]++;
  //}

  // Iterate over strings
  //str_cnt = str_cnts.count();
  first   = true;
  for (;
       str = str_cntsI.CurrentName();
       str_cntsI++)
  {
    if (!first) {
      slices_buf << ", ";
    }
    else
    {
      first = false;
    }

    //str = str_cnts.get_str_by_index(i);
    //cnt = *(int*) str_cnts.get_entry_by_index(i);
    cnt = *(int*) str_cntsI.Current();
    // Make sure there is at least one slice w/ this string
    assert(cnt > 0);

    // Indicate if more then one ref w/ this string; eg, "[7*]"
    if (cnt > 1)
    {
      slices_buf << "[" << cnt << "*]";
    }

    // Print string of ref
    slices_buf << str;
  }
}
  

/**********************************************************************
 * Exec_apply()  Apply method to class member.
 */
/*
void
Exec_apply(AST_htEntry *entry, va_list ap)
{
  Forall_ftype forall_func;  // The function to apply
  Exec_ftype   action;       // The function to apply (typecasted)
  Exec         *insp;        // The argument of the function
    
  forall_func = va_arg(ap, Forall_ftype);
  action      = (Exec_ftype) forall_func;
  insp        = (Exec *)(entry->getData());
  insp->apply(action);
  va_end(ap);
}
*/
