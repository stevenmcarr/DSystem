/* $Id: ujam.C,v 1.10 1997/03/27 20:28:01 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
#include <libs/support/misc/general.h>
#include <libs/Memoria/include/mh.h>
#include <libs/Memoria/include/mh_ast.h>
#include <libs/frontEnd/include/walk.h>
#include <libs/Memoria/uj/ujam.h>
#include <libs/Memoria/uj/log.h>

#include <libs/Memoria/include/mark.h>
#include <libs/Memoria/include/mem_util.h>
#include <libs/Memoria/include/analyze.h>
#include <libs/Memoria/include/shape.h>
#include <libs/Memoria/uj/compute_uj.h>
#include <libs/Memoria/uj/do_unroll.h>
#include <libs/Memoria/uj/do_dist.h>

#ifndef dg_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dg.h>
#endif

#ifndef dp_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#endif

#ifndef dt_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dt.h>
#endif

static int RemoveInterloopDependences(AST_INDEX node,
				      PedInfo   ped)

  {
   DG_Edge    *dg;
   int        vector;
   EDGE_INDEX edge,
              next_edge;
   AST_INDEX  name;

     if (is_subscript(node))
       {
	dg = dg_get_edge_structure( PED_DG(ped));
	name = gen_SUBSCRIPT_get_name(node);
	vector = get_info(ped,name,type_levelv);
	for (edge = dg_first_src_ref( PED_DG(ped),vector);
	     edge != END_OF_LIST;
	     edge = next_edge)
	  {
	   next_edge = dg_next_src_ref( PED_DG(ped),edge);
	   if (get_subscript_ptr(dg[edge].src)->surrounding_do !=
	       get_subscript_ptr(dg[edge].sink)->surrounding_do)
	     dg_delete_free_edge( PED_DG(ped),edge);
	  }
	for (edge = dg_first_sink_ref( PED_DG(ped),vector);
	     edge != END_OF_LIST;
	     edge = next_edge)
	  {
	   next_edge = dg_next_sink_ref( PED_DG(ped),edge);
	   if (get_subscript_ptr(dg[edge].src)->surrounding_do !=
	       get_subscript_ptr(dg[edge].sink)->surrounding_do)
	   dg_delete_free_edge( PED_DG(ped),edge);
	  }
       } 
     return(WALK_CONTINUE);
  }


static int remove_edges(AST_INDEX      stmt,
			int            level,
			loop_info_type *loop_info)

  {
   DG_Edge    *dg;
   int        vector,lvl;
   EDGE_INDEX edge,
              next_edge;
   stmt_info_type *sptr;

     if (is_assignment(stmt))
       if (is_subscript(gen_ASSIGNMENT_get_lvalue(stmt)))
         get_subscript_ptr(gen_SUBSCRIPT_get_name(gen_ASSIGNMENT_get_lvalue(
                           stmt)))->store = true;
     dg = dg_get_edge_structure( PED_DG(loop_info->ped));
     vector = get_info(loop_info->ped,stmt,type_levelv);
     for (lvl = 1;lvl < loop_info->unroll_level;lvl++)
       {
	for (edge = dg_first_src_stmt( PED_DG(loop_info->ped),vector,lvl);
	     edge != END_OF_LIST;
	     edge = next_edge)
	  {
	   next_edge = dg_next_src_stmt( PED_DG(loop_info->ped),edge);
	   dg_delete_free_edge( PED_DG(loop_info->ped),edge);
	  }
	for (edge = dg_first_sink_stmt( PED_DG(loop_info->ped),vector,lvl);
	     edge != END_OF_LIST;
	     edge = next_edge)
	  {
	   next_edge = dg_next_sink_stmt( PED_DG(loop_info->ped),edge);
	   dg_delete_free_edge( PED_DG(loop_info->ped),edge);
	  }
       }
     for (lvl = loop_info->unroll_level; lvl < level;lvl++)
       {
	for (edge = dg_first_src_stmt( PED_DG(loop_info->ped),vector,lvl);
	     edge != END_OF_LIST;
	     edge = next_edge)
	  {
	   next_edge = dg_next_src_stmt( PED_DG(loop_info->ped),edge);
	   if (dg[edge].type == dg_exit || dg[edge].type == dg_io ||
	       dg[edge].type == dg_call || dg[edge].type == dg_control)
	     dg_delete_free_edge( PED_DG(loop_info->ped),edge);
	   else 
	     if ((sptr=get_stmt_info_ptr(ut_get_stmt(dg[edge].sink))) == NULL)
	       dg_delete_free_edge( PED_DG(loop_info->ped),edge);
	  }
	for (edge = dg_first_sink_stmt( PED_DG(loop_info->ped),vector,lvl);
	     edge != END_OF_LIST;
	     edge = next_edge)
	  {
	   next_edge = dg_next_sink_stmt( PED_DG(loop_info->ped),edge);
	   if (dg[edge].type == dg_exit || dg[edge].type == dg_io ||
	       dg[edge].type == dg_call || dg[edge].type == dg_control)
	     dg_delete_free_edge( PED_DG(loop_info->ped),edge);
	   else 
	     if ((sptr = get_stmt_info_ptr(ut_get_stmt(dg[edge].src))) == NULL)
	       dg_delete_free_edge( PED_DG(loop_info->ped),edge);
	  }
       }
     for (edge = dg_first_src_stmt( PED_DG(loop_info->ped),vector,
				   LOOP_INDEPENDENT);
	  edge != END_OF_LIST;
	  edge = next_edge)
       {
	next_edge = dg_next_src_stmt( PED_DG(loop_info->ped),edge);
	if (dg[edge].type == dg_exit || dg[edge].type == dg_io ||
	    dg[edge].type == dg_call || dg[edge].type == dg_control)
	  dg_delete_free_edge( PED_DG(loop_info->ped),edge);
	else 
	  if ((sptr = get_stmt_info_ptr(ut_get_stmt(dg[edge].sink))) == NULL)
	    dg_delete_free_edge( PED_DG(loop_info->ped),edge);
       }
     for (edge = dg_first_sink_stmt( PED_DG(loop_info->ped),vector,
				    LOOP_INDEPENDENT);
	  edge != END_OF_LIST;
	  edge = next_edge)
       {
	next_edge = dg_next_sink_stmt( PED_DG(loop_info->ped),edge);
	if (dg[edge].type == dg_exit || dg[edge].type == dg_io ||
	    dg[edge].type == dg_call || dg[edge].type == dg_control)
	  dg_delete_free_edge( PED_DG(loop_info->ped),edge);
	else 
	  if ((sptr = get_stmt_info_ptr(ut_get_stmt(dg[edge].src))) == NULL)
	    dg_delete_free_edge( PED_DG(loop_info->ped),edge);
       }
     return(WALK_CONTINUE);
  }

static int check_loop(AST_INDEX      stmt,
		      int            level,
		      loop_info_type *loop_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   if (is_call(stmt) || is_assigned_goto(stmt) || is_computed_goto(stmt) ||
       is_goto(stmt) || is_arithmetic_if(stmt) || is_return(stmt))
     loop_info->unroll_level = level;
   else if (is_do(stmt))
     loop_info->num_do++;
   return(WALK_CONTINUE);
  }

static void walk_to_free_split(model_loop *loop_data,
			       int        loop)

  {
   int next;
   UtilNode *listnode;
   
     if (!util_list_empty(loop_data[loop].split_list))
       {
	for (listnode = UTIL_HEAD(loop_data[loop].split_list);
	     listnode != NULLNODE;
	     listnode = UTIL_NEXT(listnode))
	  walk_to_free_split((model_loop *)UTIL_NODE_ATOM(listnode),0);
	util_free_nodes(loop_data[loop].split_list);
	util_list_free(loop_data[loop].split_list);
       }
     for (next = loop_data[loop].inner_loop;
	  next != -1;
	  next = loop_data[next].next_loop)
       walk_to_free_split(loop_data,next);
  }

static int check_unroll(AST_INDEX      stmt,
			int            level,
			loop_info_type *loop_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   model_loop *loop_data;
   pre_info_type  pre_info;

   if (is_do(stmt))
     if (loop_info->unroll_level == level)
       {
	pre_info.stmt_num = 0;
	pre_info.loop_num = 0;
	pre_info.surrounding_do = -1;
	pre_info.surround_node = AST_NIL;
	pre_info.abort = false;
	pre_info.ped = loop_info->ped;
	pre_info.symtab = loop_info->symtab;
	pre_info.ar = loop_info->ar;
	walk_statements(stmt,level,(WK_STMT_CLBACK)ut_mark_do_pre,
			(WK_STMT_CLBACK)ut_mark_do_post,
			(Generic)&pre_info);
	walk_statements(stmt,level,(WK_STMT_CLBACK)remove_edges,
			(WK_STMT_CLBACK)NOFUNC,(Generic)loop_info);
	if (pre_info.abort)
	  return(WALK_ABORT);
	loop_data = (model_loop *)loop_info->ar->arena_alloc_mem_clear
	                  (LOOP_ARENA,loop_info->num_do*sizeof(model_loop)*4);
	ut_analyze_loop(stmt,loop_data,level,loop_info->ped,loop_info->symtab);
	ut_check_shape(loop_data,0);
	mh_compute_unroll_amounts(loop_data,loop_info->num_do,
				  loop_info->num_loops,loop_info->ped,
				  loop_info->symtab,loop_info->ar);
	if (((config_type *)PED_MH_CONFIG(loop_info->ped))->logging)
	  {

	   /* removing interloop dependences is necessary to ensure that
	      scalar replacement won't fail when called to gather unroll
	      statistics */

	   walk_expression(stmt,(WK_EXPR_CLBACK)RemoveInterloopDependences,
			   (WK_EXPR_CLBACK)NOFUNC,(Generic)loop_info->ped);
	   mh_log_data(loop_data,
		       ((config_type *)PED_MH_CONFIG(loop_info->ped))->logfile,
		       loop_info->ped,loop_info->symtab,loop_info->ar,
		       loop_info->LoopStats);
	  }
	else
	  {

	    // no distribution is necessary for partitioned unrolling as
	    // amounts do not differ (currently)

	    if (PartitionUnrollAmount == 0)
	      mh_do_distribution(loop_data,&loop_info->num_do);

	   mh_do_unroll_and_jam(loop_data,loop_info->ped,loop_info->symtab,
				loop_info->num_loops,loop_info->ar,loop_info->LoopStats);
	   walk_to_free_split(loop_data,0);
	  }
       }
     else if (loop_info->unroll_level > level)
       loop_info->unroll_level = level;
   return(WALK_CONTINUE);
  }


AST_INDEX memory_unroll_and_jam(PedInfo       ped,
				AST_INDEX     root,
				int           level,
				int           num_loops,
				SymDescriptor symtab,
				arena_type    *ar,
				LoopStatsType *LoopStats)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   loop_info_type loop_info;
   model_loop     *loop_data;
   stmt_info_type *stptr;
   AST_INDEX      prev;

     prev = list_prev(root);
     loop_info.unroll_level = level;
     loop_info.num_do = 0;
     loop_info.ped = ped;
     loop_info.num_loops = num_loops;
     loop_info.symtab = symtab;
     loop_info.ar = ar;
     loop_info.LoopStats = LoopStats;
     walk_statements(root,level,(WK_STMT_CLBACK)check_loop,(WK_STMT_CLBACK)check_unroll,(Generic)&loop_info);
     while (list_prev(root) != prev)
       root = list_prev(root);
     return(root);
  }
