/* $Id: check.C,v 1.5 1992/12/11 11:22:06 carr Exp $ */
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
#include <general.h>

#include <sr.h>
#include <mh_ast.h>
#include <fort/walk.h>

#ifndef check_h
#include <check.h>
#endif

#ifndef dg_h
#include <dg.h>
#endif

#ifndef dt_h
#include <dt.h>
#endif

#ifndef mem_util_h
#include <mem_util.h>
#endif

static int check_def(AST_INDEX node,
		     check_info_type *check_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX name;
   DG_Edge   *dg;
   int       def,
             edge;
   scalar_info_type *sptr1,*sptr2;
   
     if (is_subscript(node))
       {
	dg = dg_get_edge_structure( PED_DG(check_info->ped));
	name = gen_SUBSCRIPT_get_name(node);
        def = get_info(check_info->ped,name,type_levelv);
	sptr1 = get_scalar_info_ptr(name);
	for (edge = dg_first_sink_ref( PED_DG(check_info->ped),def);
	     edge != END_OF_LIST;
	     edge = dg_next_sink_ref( PED_DG(check_info->ped),edge))
	  if (dg[edge].type == dg_output || dg[edge].type == dg_anti)
	    if (dg[edge].consistent == inconsistent || dg[edge].symbolic)
	      {
	       sptr2 = get_scalar_info_ptr(dg[edge].src);
	       if (dg[edge].level == LOOP_INDEPENDENT &&
		   sptr1->surrounding_do == sptr2->surrounding_do)
		 {
		  if (sptr1->kill_set == NULL)
	            sptr1->kill_set = ut_create_set(check_info->ar,LOOP_ARENA,
						    check_info->size);
		  ut_add_number(sptr1->kill_set,sptr2->table_index);
		  ut_add_number(check_info->LC_kill,sptr2->table_index);
		 }
	       else if (dg[edge].type == dg_output && 
			dg[edge].level == check_info->level)
	         ut_add_number(check_info->LC_kill,sptr2->table_index);
	      }
	for (edge = dg_first_src_ref( PED_DG(check_info->ped),def);
	     edge != END_OF_LIST;
	     edge = dg_next_src_ref( PED_DG(check_info->ped),edge))
	  if (dg[edge].type == dg_output || dg[edge].type == dg_true)
	    if (dg[edge].consistent == inconsistent || dg[edge].symbolic) 
	      {
	       sptr2 = get_scalar_info_ptr(dg[edge].sink);
	       if ((dg[edge].level == LOOP_INDEPENDENT &&
		    sptr1->surrounding_do == sptr2->surrounding_do) ||

		     /* the following condition makes up for the fact that
			loop-independent intra-statement antidependences do
			not exist in the graph. */

		   (dg[edge].type == dg_true && 
		    dg[edge].level == check_info->level &&
		    ut_get_stmt(dg[edge].src) == ut_get_stmt(dg[edge].sink)))
		 {
		  if (sptr1->kill_set == NULL)
		    sptr1->kill_set = ut_create_set(check_info->ar,LOOP_ARENA,
						    check_info->size);
		  ut_add_number(sptr1->kill_set, sptr2->table_index);
		  ut_add_number(check_info->LC_kill,sptr2->table_index);
		 }
	       else if (dg[edge].level == check_info->level)
	         ut_add_number(check_info->LC_kill,sptr2->table_index);
	      }
       }
   return(WALK_CONTINUE);
  }
  
static int check_stmt(AST_INDEX stmt,
		      int level,
		      check_info_type *check_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   if (is_assignment(stmt))
     walk_expression(gen_ASSIGNMENT_get_lvalue(stmt),NOFUNC,check_def,
		     (Generic)check_info);
   else if (is_read_short(stmt))
     walk_expression(gen_READ_SHORT_get_data_vars_LIST(stmt),NOFUNC,check_def,
		     (Generic)check_info);
   return(WALK_CONTINUE);
  }

void sr_check_inconsistent_edges(AST_INDEX root,
				 check_info_type *check_info)
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/


  {
   walk_statements(root,check_info->level,check_stmt,NOFUNC,
		   (Generic)check_info);
  }
