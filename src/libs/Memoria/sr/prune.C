/* $Id: prune.C,v 1.6 1993/09/06 14:55:42 carr Exp $ */
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
#include <general.h>
#include <sr.h>
#include <mh_ast.h>
#include <fort/walk.h>
#include <prune.h>

#ifndef Arena_h
#include <misc/Arena.h>
#endif

#ifndef dt_h
#include <dt.h>
#endif

#ifndef dg_h
#include <dg.h>
#endif

#ifndef mh_config_h
#include <mh_config.h>
#endif

#include <mem_util.h>

static void prune_dependence_edges(AST_INDEX     node,
				   int           distance,
				   gen_info_type *gen_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   int      sink_ref,
            edge_dist,
            edge,
            next_edge;
   DG_Edge  *dg;
   scalar_info_type *scalar_src,
                    *scalar_sink;
   AST_INDEX        src_stmt,
                    sink_stmt;

     scalar_sink = get_scalar_info_ptr(node);
     dg = dg_get_edge_structure( PED_DG(gen_info->ped));
     sink_ref= get_info(gen_info->ped,node,type_levelv);
     for (edge = dg_first_sink_ref( PED_DG(gen_info->ped),sink_ref);
	  edge != END_OF_LIST;
	  edge = next_edge)
       {
	next_edge = dg_next_sink_ref( PED_DG(gen_info->ped),edge);
	if (dg[edge].type == dg_true || dg[edge].type == dg_anti ||
	    dg[edge].type == dg_input || dg[edge].type == dg_output)
	  {
	   scalar_src = get_scalar_info_ptr(dg[edge].src);
	   if (scalar_src->surrounding_do != scalar_sink->surrounding_do)
	     dg_delete_free_edge( PED_DG(gen_info->ped),edge);
	   else 
	     switch(dg[edge].type)
	       {
		case dg_input:
		case dg_true: 
		  if (dg[edge].consistent != inconsistent && 
		      !dg[edge].symbolic)
		    {
		     if (dg[edge].level != LOOP_INDEPENDENT)
		       if ((edge_dist = 
			    gen_get_dt_DIS(&dg[edge],dg[edge].level)) < 0)
		         edge_dist = 1;
		       else;
		     else
		       edge_dist = 0;
		     src_stmt = ut_get_stmt(dg[edge].src);
		     sink_stmt = ut_get_stmt(node);
		     if (((src_stmt == sink_stmt && dg[edge].type == dg_true)
			  || get_stmt_info_ptr(src_stmt)->stmt_num >
			  get_stmt_info_ptr(sink_stmt)->stmt_num) &&
			  edge_dist == 1 && !scalar_sink->prevent_rec &&
			 !scalar_src->prevent_rec)

		       /* set to false for now, most of the time this can
			  be true, but there are cases that fail, see
			  psmoo_i.f in {carr flo52} in the database. 
			  When loop independent intrastatement 
			  anti dependences are added this can be changed 
			  back to true.*/

			scalar_src->recurrence = false;
		     else if (edge_dist != 0)
		       {
			scalar_src->recurrence = false;
			scalar_src->prevent_rec = true;
		       }
		     if (dg[edge].src == dg[edge].sink && 
			 dg[edge].type == dg_input && !scalar_src->prevent_slr)
		       scalar_src->scalar = true;
		     if (edge_dist > distance)
		       dg_delete_free_edge( PED_DG(gen_info->ped),edge);
		   /*  else if (edge_dist == distance && 
			      (scalar_src->generator == -1 || 
			       dg[edge].src == dg[edge].sink))
		       scalar_src->is_generator = true; */
		    }
		  else
		    {
		     if (dg[edge].type == dg_true)
		       {
			scalar_src->prevent_slr = true;
			scalar_src->scalar = false;
			scalar_sink->prevent_slr = true;
			scalar_sink->scalar = false;
		       }
		     dg_delete_free_edge( PED_DG(gen_info->ped),edge);
		    }
		  break;
		case dg_output: 
		    if (dg[edge].consistent == consistent_SIV && 
			!dg[edge].symbolic && dg[edge].src == dg[edge].sink &&
			!scalar_src->prevent_slr)
		      scalar_src->scalar = true;
		  break;
		case dg_anti:
		  if (dg[edge].consistent != inconsistent &&
		      dg[edge].level == LOOP_INDEPENDENT)
		    {
		     scalar_src->recurrence = false;
		     scalar_src->prevent_rec = true;
		    }
		default:
		  dg_delete_free_edge( PED_DG(gen_info->ped),edge);
	       }
	  }
       }
  }

static int check_gen(AST_INDEX       node,
		     gen_info_type   *gen_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX        name;
   scalar_info_type *scalar_info;

     if (is_subscript(node))
       {
	name = gen_SUBSCRIPT_get_name(node);
	scalar_info = get_scalar_info_ptr(name);
	if (scalar_info->generator == -1) 
	 
	/* no generator */

	  prune_dependence_edges(name,-2,gen_info);
	else if (!scalar_info->is_consistent || !scalar_info->constant)
	  {
	   scalar_info->generator = -1;
	   prune_dependence_edges(name,-2,gen_info);
	  }
	else if (scalar_info->gen_distance == 0)

	/* loop-independent generator */

	  prune_dependence_edges(name,0,gen_info);
	else if (scalar_info->gen_type == LCAV)
	
	/* loop-carried available generator */

	  {
	   prune_dependence_edges(name,scalar_info->gen_distance,
				  gen_info);
	  }
	else if (scalar_info->gen_type == LCPAV &&
		 (ut_member_number(gen_info->entry->LC_antic_in,
				   scalar_info->generator) ||
		  (((config_type *)PED_MH_CONFIG(gen_info->ped))->aggressive &&
		 gen_info->array_table[scalar_info->generator].profit > 0.0)))

	/* loop-carried partially available generator */

	  {
	   prune_dependence_edges(name,scalar_info->gen_distance,
				  gen_info);
	  }
	else

	/* partially availiable but not anticipated on all paths */

	  {
	   scalar_info->generator = -1;
	   prune_dependence_edges(name,-2,gen_info);
	  }
       }
   return(WALK_CONTINUE);
  }

static int check_stmts(AST_INDEX       stmt,
		       int             level,
		       gen_info_type   *gen_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   if (is_guard(stmt))
     walk_expression(gen_GUARD_get_rvalue(stmt),NOFUNC,check_gen,
		     (Generic)gen_info);
   else if (is_logical_if(stmt))
     walk_expression(gen_LOGICAL_IF_get_rvalue(stmt),NOFUNC,check_gen,
		     (Generic)gen_info);
   else if (is_assignment(stmt))
     {
      walk_expression(gen_ASSIGNMENT_get_lvalue(stmt),NOFUNC,check_gen,
		      (Generic)gen_info);
      walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),NOFUNC,check_gen,
		      (Generic)gen_info);
     }
   else if (is_write(stmt))
     walk_expression(gen_WRITE_get_data_vars_LIST(stmt),NOFUNC,check_gen,
		     (Generic)gen_info);
   else if (is_arithmetic_if(stmt))
     walk_expression(gen_ARITHMETIC_IF_get_rvalue(stmt),NOFUNC,check_gen,
		     (Generic)gen_info);
   return(WALK_CONTINUE);
  }

void sr_prune_graph(AST_INDEX       root,
		    int             level,
		    gen_info_type   *gen_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   walk_statements(root,level,check_stmts,NOFUNC,(Generic)gen_info);
  }
