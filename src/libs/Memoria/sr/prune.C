/* $Id: prune.C,v 1.15 1998/06/01 15:40:39 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
#include <libs/support/misc/general.h>
#include <libs/Memoria/include/sr.h>
#include <libs/Memoria/include/mh_ast.h>
#include <libs/frontEnd/include/walk.h>
#include <libs/Memoria/sr/prune.h>

#ifndef Arena_h
#include <libs/support/memMgmt/Arena.h>
#endif

#ifndef dt_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dt.h>
#endif

#ifndef dg_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dg.h>
#endif

#ifndef mh_config_h
#include <libs/Memoria/include/mh_config.h>
#endif

#include <libs/Memoria/include/mem_util.h>

static void MarkAllSinksAsNotScalar(PedInfo ped,
				    AST_INDEX node)

  {
   EDGE_INDEX edge;
   int        ref;
   DG_Edge    *dg;

    dg = dg_get_edge_structure(PED_DG(ped));
    ref = get_info(ped,node,type_levelv);
    for (edge = dg_first_src_ref( PED_DG(ped),ref);
	 edge != END_OF_LIST;
	 edge = dg_next_src_ref( PED_DG(ped),edge))
      {
       get_scalar_info_ptr(dg[edge].sink)->scalar = false;
       get_scalar_info_ptr(dg[edge].sink)->prevent_slr = true;
      }
  }
   
static void prune_dependence_edges(AST_INDEX     node,
				   int           distance,
				   gen_info_type *gen_info,
				   Boolean&      Invariant,
				   Boolean&      Recurrence)

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
                    sink_stmt,
                    name;

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
		       scalar_src->recurrence = true;
		     else if (edge_dist != 0)
		       {
			scalar_src->recurrence = false;
			scalar_src->prevent_rec = true;
		       }
		     if (edge_dist > distance && 
			 NOT(scalar_src->recurrence && ReplaceLevel > 1))
		       dg_delete_free_edge( PED_DG(gen_info->ped),edge);
		    }
		  else
		    {
		     dg_delete_free_edge( PED_DG(gen_info->ped),edge);
		    }
		  break;
		case dg_output: 
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
     if (scalar_sink->generator != -1)
       {
	name=gen_SUBSCRIPT_get_name(gen_info->array_table[scalar_sink->generator].node);
	Invariant = BOOL(scalar_sink->scalar || get_scalar_info_ptr(name)->scalar);
	Recurrence = get_scalar_info_ptr(name)->recurrence;
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
   Boolean Invariant = false;
   Boolean Recurrence = false;

     if (is_subscript(node))
       {
	name = gen_SUBSCRIPT_get_name(node);
	scalar_info = get_scalar_info_ptr(name);
	if (scalar_info->generator == -1) 
	 
	/* no generator */

	  {
	   prune_dependence_edges(name,-2,gen_info,Invariant,Recurrence);
	   if (Invariant && (ReplaceLevel < 2 || ReplaceLevel == 5))
	     scalar_info->scalar = false;
	  }
	else if (!scalar_info->is_consistent || !scalar_info->constant)
	  {
	   scalar_info->generator = -1;
	   prune_dependence_edges(name,-2,gen_info,Invariant,Recurrence);
	  }
	else if (scalar_info->gen_type == LIAV && ReplaceLevel > 0)

	/* loop-independent available generator */

	  prune_dependence_edges(name,0,gen_info,Invariant,Recurrence);
	else if (scalar_info->gen_type == LIPAV && ReplaceLevel > 6)
	  prune_dependence_edges(name,0,gen_info,Invariant,Recurrence);
	else if (scalar_info->gen_type == LCAV &&
		 ((scalar_info->gen_distance == 1 && ReplaceLevel > 1 &&
		   ReplaceLevel != 5) ||
		  (scalar_info->gen_distance > 1 && (ReplaceLevel == 4 ||
						     ReplaceLevel == 8))))
	
	/* loop-carried available generator */

	  {
	   prune_dependence_edges(name,scalar_info->gen_distance,
				  gen_info,Invariant,Recurrence);
	   if (ReplaceLevel < 3 && NOT(Invariant) && NOT(Recurrence))
	     {
	      scalar_info->generator = -1;
	      prune_dependence_edges(name,-2,gen_info,Invariant,Recurrence);
	     }
	  }
	else if (scalar_info->gen_type == LCPAV && ReplaceLevel == 8)
	  {
	    /* loop-carried partially available generator */

	    prune_dependence_edges(name,scalar_info->gen_distance,gen_info,Invariant,
				   Recurrence);
	    
	    if (scalar_info->gen_type != LCPAV ||
		(!ut_member_number(gen_info->entry->LC_antic_in,
				   scalar_info->generator) &&
		 (NOT(((config_type*)PED_MH_CONFIG(gen_info->ped))->aggressive)
		  || NOT(scalar_info->scalar) || scalar_info->prevent_slr)))


	      /* partially availiable but not anticipated on all paths */

	      {
		scalar_info->generator = -1;
		prune_dependence_edges(name,-2,gen_info,Invariant,Recurrence);
		scalar_info->scalar = false;  /* invariant code motion
						    is unsafe if PAV */
	      }
	  }
	else
	  {
	   scalar_info->generator = -1;
	   prune_dependence_edges(name,-2,gen_info,Invariant,Recurrence);
	  }
       }
   return(WALK_CONTINUE);
  }

static int CheckForInv(AST_INDEX     AstNode,
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
                    sink_stmt,
                    node;

     if (is_subscript(AstNode))
       {
	node = gen_SUBSCRIPT_get_name(AstNode);
	scalar_sink = get_scalar_info_ptr(node);
	dg = dg_get_edge_structure( PED_DG(gen_info->ped));
	sink_ref= get_info(gen_info->ped,node,type_levelv);
	for (edge = dg_first_sink_ref( PED_DG(gen_info->ped),sink_ref);
	     edge != END_OF_LIST;
	     edge = dg_next_sink_ref( PED_DG(gen_info->ped),edge))
	  {
	   if (dg[edge].type == dg_true || dg[edge].type == dg_anti ||
	       dg[edge].type == dg_input || dg[edge].type == dg_output)
	     {
	      scalar_src = get_scalar_info_ptr(dg[edge].src);
	      if (scalar_src->surrounding_do == scalar_sink->surrounding_do)
		{
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
			 if (dg[edge].src == dg[edge].sink && 
			     dg[edge].type == dg_input && !scalar_src->prevent_slr)
			   scalar_src->scalar = true;
			}
		      else
			{
			 if (dg[edge].type == dg_true)
			   {
			    scalar_src->prevent_slr = true;
			    scalar_src->scalar = false;
			    MarkAllSinksAsNotScalar(gen_info->ped,dg[edge].src);
			   }
			}
		      break;
		    case dg_output: 
		      if (dg[edge].consistent == consistent_SIV && 
			  !dg[edge].symbolic && dg[edge].src == dg[edge].sink &&
			  !scalar_src->prevent_slr)
			 scalar_src->scalar = true;
		      break;
		    default:
		      break;
	          }
		}
	     }
	  }
       }
     return(WALK_CONTINUE);
  }


static int mark_invariant(AST_INDEX       stmt,
			  int             level,
			  gen_info_type   *gen_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   if (is_guard(stmt))
     walk_expression(gen_GUARD_get_rvalue(stmt),NOFUNC,(WK_EXPR_CLBACK)CheckForInv,
		     (Generic)gen_info);
   else if (is_logical_if(stmt))
     walk_expression(gen_LOGICAL_IF_get_rvalue(stmt),NOFUNC,
		     (WK_EXPR_CLBACK)CheckForInv,
		     (Generic)gen_info);
   else if (is_assignment(stmt))
     {
      walk_expression(gen_ASSIGNMENT_get_lvalue(stmt),NOFUNC,
		     (WK_EXPR_CLBACK)CheckForInv,
		      (Generic)gen_info);
      walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),NOFUNC,
		     (WK_EXPR_CLBACK)CheckForInv,
		      (Generic)gen_info);
     }
   else if (is_write(stmt))
     walk_expression(gen_WRITE_get_data_vars_LIST(stmt),NOFUNC,
		     (WK_EXPR_CLBACK)CheckForInv,
		     (Generic)gen_info);
   else if (is_arithmetic_if(stmt))
     walk_expression(gen_ARITHMETIC_IF_get_rvalue(stmt),NOFUNC,
		     (WK_EXPR_CLBACK)CheckForInv,
		     (Generic)gen_info);
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
     walk_expression(gen_GUARD_get_rvalue(stmt),NOFUNC,(WK_EXPR_CLBACK)check_gen,
		     (Generic)gen_info);
   else if (is_logical_if(stmt))
     walk_expression(gen_LOGICAL_IF_get_rvalue(stmt),NOFUNC,
		     (WK_EXPR_CLBACK)check_gen,
		     (Generic)gen_info);
   else if (is_assignment(stmt))
     {
      walk_expression(gen_ASSIGNMENT_get_lvalue(stmt),NOFUNC,
		     (WK_EXPR_CLBACK)check_gen,
		      (Generic)gen_info);
      walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),NOFUNC,
		     (WK_EXPR_CLBACK)check_gen,
		      (Generic)gen_info);
     }
   else if (is_write(stmt))
     walk_expression(gen_WRITE_get_data_vars_LIST(stmt),NOFUNC,
		     (WK_EXPR_CLBACK)check_gen,
		     (Generic)gen_info);
   else if (is_arithmetic_if(stmt))
     walk_expression(gen_ARITHMETIC_IF_get_rvalue(stmt),NOFUNC,
		     (WK_EXPR_CLBACK)check_gen,
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
   walk_statements(root,level,(WK_STMT_CLBACK)mark_invariant,NOFUNC,(Generic)gen_info);
   walk_statements(root,level,(WK_STMT_CLBACK)check_stmts,NOFUNC,(Generic)gen_info);
  }
