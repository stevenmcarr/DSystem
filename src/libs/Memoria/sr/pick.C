/* $Id: pick.C,v 1.3 1992/10/03 15:48:59 rn Exp $ */
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
#include <sr.h>
#include <Arena.h>
#include <scalar.h>
#include <pick.h>

static int update_avail(AST_INDEX       node,
			pick_info_type  *pick_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   scalar_info_type *sptr;
   AST_INDEX        name;
   DG_Edge          *dg;
   int              vector;
   EDGE_INDEX       edge;

     if (is_subscript(node))
       {
	name = gen_SUBSCRIPT_get_name(node);
	sptr = get_scalar_info_ptr(name);
	ut_add_number(pick_info->LI_avail,sptr->table_index);
	ut_add_number(pick_info->LI_pavail,sptr->table_index);
	ut_add_number(pick_info->LI_rgen,sptr->array_num);
	if (pick_info->def)
	  {
	   dg = dg_get_edge_structure( PED_DG(pick_info->ped));
	   vector = get_info(pick_info->ped,name,type_levelv);
	   for (edge = dg_first_sink_ref( PED_DG(pick_info->ped),vector);
		edge != END_OF_LIST;
		edge = dg_next_sink_ref( PED_DG(pick_info->ped),edge))
	   if (pick_info->dg[edge].level == LOOP_INDEPENDENT)
	     {
	      ut_delete_number(pick_info->LI_rgen,
			       get_scalar_info_ptr(dg[edge].src)->array_num);
	      ut_delete_number(pick_info->LC_rgen_if_1,
			       get_scalar_info_ptr(dg[edge].src)->array_num);
	     }
	   if (sptr->kill_set != NULL)
	     {
	      ut_difference121(pick_info->LI_avail,sptr->kill_set);
	      ut_difference121(pick_info->LI_pavail,sptr->kill_set);
	      ut_difference121(pick_info->LC_avail_if_1,sptr->kill_set);
	      ut_difference121(pick_info->LC_pavail_if_1,sptr->kill_set);
	     }
	  }
       }
   return(WALK_CONTINUE);
  }

static int kill_rhs(AST_INDEX      node,
		    pick_info_type *pick_info)

  {
   scalar_info_type *sptr;
   
     if (is_subscript(node))
       if (pt_expr_equal(node,pick_info->lhs))
	 {
	  sptr = get_scalar_info_ptr(gen_SUBSCRIPT_get_name(node));
	  ut_delete_number(pick_info->LI_rgen,sptr->array_num);
	  ut_delete_number(pick_info->LC_rgen_if_1,sptr->array_num);
	 }
     return(WALK_CONTINUE);
  }

static int chk_store(AST_INDEX         node,
		     pick_info_type    *pick_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   EDGE_INDEX       edge;
   int              sink_ref;
   stmt_info_type   *stmt_ptr1,*stmt_ptr2;
   scalar_info_type *sptr1, *sptr2;
   AST_INDEX        name;

     if (is_subscript(node))
       {
	name = gen_SUBSCRIPT_get_name(node);
	sptr1 = get_scalar_info_ptr(name);
	stmt_ptr1 = get_stmt_info_ptr(ut_get_stmt(node));
	sink_ref = get_info(pick_info->ped,name,type_levelv);
	for (edge = dg_first_sink_ref( PED_DG(pick_info->ped),sink_ref);
	     edge != END_OF_LIST;
	     edge = dg_next_sink_ref( PED_DG(pick_info->ped),edge))
	  {
	   sptr2 = get_scalar_info_ptr(pick_info->dg[edge].src);
	   if (pick_info->dg[edge].type == dg_output &&
	       pick_info->dg[edge].level == LOOP_INDEPENDENT &&
	       sptr1->surrounding_do == sptr2->surrounding_do)
	    if (pick_info->dg[edge].consistent != inconsistent && 
		!pick_info->dg[edge].symbolic)
	      {
	       stmt_ptr2 = get_stmt_info_ptr(ut_get_stmt(pick_info->dg[edge].
							 src));
	       if (stmt_ptr1->block == stmt_ptr2->block &&
		   ut_member_number(pick_info->LI_avail,sptr1->table_index) &&
		   ut_member_number(pick_info->LI_rgen,sptr2->array_num))
	         sptr2->no_store = true;
	      }
	  }
       }
     return(WALK_CONTINUE);
  }
	    

static int get_gen(AST_INDEX         node,
		   pick_info_type    *pick_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   int              edge,
                    next_edge,
                    sink_ref,
                    dist;
   scalar_info_type *psrc,
                    *psink;

     if (is_subscript(node))
       {
	get_scalar_info_ptr(gen_SUBSCRIPT_get_name(node))->generator = -1;
	sink_ref = get_info(pick_info->ped,gen_SUBSCRIPT_get_name(node),
			    type_levelv);
	edge = dg_first_sink_ref( PED_DG(pick_info->ped),sink_ref);

	/*  consistent edges can provide a value only if they are not 
	    killed.  Thus, their availability means their value reaches
	    the sink.  Available consistent edges will always be chosen
	    over inconsistent edges. */

	if (edge != END_OF_LIST)
	  psink = get_scalar_info_ptr(pick_info->dg[edge].sink);
	while (edge != END_OF_LIST)
	  {
	   next_edge = dg_next_sink_ref( PED_DG(pick_info->ped),edge);
	   if (pick_info->dg[edge].type == dg_true ||
	       (pick_info->dg[edge].type == dg_input && 
		pick_info->dg[edge].consistent != inconsistent && 
		!pick_info->dg[edge].symbolic)) 
	     {
	      psrc = get_scalar_info_ptr(pick_info->dg[edge].src);
	      if  (pick_info->dg[edge].level == pick_info->level)

	            /* loop-carried edge */

		{
		 if ((dist = gen_get_dt_DIS(&pick_info->dg[edge],
					    pick_info->dg[edge].level)) < 0)
		   dist = 1;
		 if ((!ut_member_number(pick_info->exit_block->LC_rgen_out,
					psrc->array_num)) &&
		     (!ut_member_number(pick_info->LC_rgen_if_1,
					 psrc->array_num) ||
		     !ut_member_number(pick_info->exit_block->LC_rgen_out_if_1,
				       psrc->array_num) || dist != 1))
		   dg_delete_free_edge( PED_DG(pick_info->ped),edge);
		 else
		   {
		    if (ut_member_number(pick_info->exit_block->LC_avail_out,
					 psrc->table_index) ||
			(ut_member_number(pick_info->LC_avail_if_1,
					  psrc->table_index) && 
			 ut_member_number(pick_info->exit_block->
					  LC_avail_out_if_1,psrc->table_index)
			 && dist == 1))

		    /* we have a loop-carried available edge */

		      if (psink->gen_type == LIAV && psink->is_consistent &&
			  psink->constant)
		        dg_delete_free_edge( PED_DG(pick_info->ped),edge);
		      else if (psink->generator == -1)
			{
			 psink->generator = psrc->table_index;
			 psink->gen_distance = dist;
			 psink->gen_type = LCAV;
			 psink->constant = !pick_info->dg[edge].symbolic;
			 psink->is_consistent = 
			   (pick_info->dg[edge].consistent != inconsistent);
			}
		      else if (psink->gen_type == LCAV)
		        if (dist < psink->gen_distance ||
			    (dist == psink->gen_distance && 
			     (!psink->is_consistent || !psink->constant) &&
			     pick_info->dg[edge].consistent != inconsistent
			     && !pick_info->dg[edge].symbolic))
			  {
			   psink->generator = psrc->table_index;
			   psink->gen_distance = dist;
			   psink->gen_type = LCAV;
			   psink->constant = !pick_info->dg[edge].symbolic;
			   psink->is_consistent =
			    (pick_info->dg[edge].consistent != inconsistent);
			  }
			else;
		      else if (pick_info->dg[edge].consistent != 
			          inconsistent &&
			       !pick_info->dg[edge].symbolic)
			{
			 psink->generator = psrc->table_index;
			 psink->gen_distance = dist;
			 psink->gen_type = LCAV;
			 psink->constant = !pick_info->dg[edge].symbolic;
			 psink->is_consistent = 
			    (pick_info->dg[edge].consistent != inconsistent);
			}
		      else;
		    else if (ut_member_number(pick_info->exit_block->
					      LC_pavail_out,psrc->table_index)
			     || (ut_member_number(pick_info->LC_pavail_if_1,
						  psrc->table_index) && 
				 dist == 1))

		    /* we have a loop-carried partially available edge */

		      if (psink->gen_type == LIAV && psink->is_consistent &&
			  psink->constant)
		        dg_delete_free_edge( PED_DG(pick_info->ped),edge);
		      else if (psink->generator == -1)
			{
			 psink->generator = psrc->table_index;
			 psink->gen_distance = dist;
			 psink->gen_type = LCPAV;
			 psink->constant = !pick_info->dg[edge].symbolic;
			 psink->is_consistent = 
			    (pick_info->dg[edge].consistent != inconsistent);
			}
		      else if (psink->gen_type == LCPAV)
		      
		      /* current generator is LCPAV pick one with largest 
			 distance */
		      
		        if (pick_info->dg[edge].consistent != inconsistent 
			    && !pick_info->dg[edge].symbolic &&
			    (dist > psink->gen_distance || 
			     !psink->is_consistent || !psink->constant))
			  {
			   psink->generator = psrc->table_index;
			   psink->gen_distance = dist;
			   psink->gen_type = LCPAV;
			   psink->constant = !pick_info->dg[edge].symbolic;
			   psink->is_consistent=
			    (pick_info->dg[edge].consistent != inconsistent);
			  }
			else;
		      else if ((!psink->is_consistent || !psink->constant) && 
			       pick_info->dg[edge].consistent != inconsistent
			       && !pick_info->dg[edge].symbolic)
			{
			 psink->generator = psrc->table_index;
			 psink->gen_distance = dist;
			 psink->gen_type = LCPAV;
			 psink->constant = !pick_info->dg[edge].symbolic;
			 psink->is_consistent = 
			    (pick_info->dg[edge].consistent != inconsistent);
			}
		   }
		}
	      else if (psrc->surrounding_do == psink->surrounding_do)
		{
		   /* loop-independent edge */

		 if (!ut_member_number(pick_info->LI_rgen,psrc->array_num))
		   dg_delete_free_edge( PED_DG(pick_info->ped),edge);
		 else if (ut_member_number(pick_info->LI_avail,
					   psrc->table_index))
		   if (psink->gen_type == LIAV && psink->is_consistent &&
		       psink->constant && 
		       (pick_info->dg[edge].consistent == inconsistent ||
			pick_info->dg[edge].symbolic))
		     dg_delete_free_edge( PED_DG(pick_info->ped),edge);
		   else if (psink->generator == -1 ||
			    !psink->is_consistent || 
			    !psink->constant || 
			    (psink->gen_type != LIAV && 
			     pick_info->dg[edge].consistent != inconsistent 
			     && !pick_info->dg[edge].symbolic))
		     {
		      psink->generator = psrc->table_index;
		      psink->gen_distance = 0;
		      psink->gen_type = LIAV;
		      psink->constant = !pick_info->dg[edge].symbolic;
		      psink->is_consistent = 
			    (pick_info->dg[edge].consistent != inconsistent);
		     }
		   else;
		 else if (ut_member_number(pick_info->LI_pavail,
					   psrc->table_index))
		 
		 /* a loop-independent partially available */

		   if (psink->gen_type == LIAV && psink->is_consistent &&
		       psink->constant)
		     dg_delete_free_edge( PED_DG(pick_info->ped),edge);
		   else if (psink->generator == -1 || 
			    ((!psink->is_consistent  || !psink->constant) &&
			     pick_info->dg[edge].consistent != inconsistent 
			     && !pick_info->dg[edge].symbolic))
		     {
		      psink->generator = psrc->table_index;
		      psink->gen_distance = 0;
		      psink->gen_type = LIPAV;
		      psink->constant = !pick_info->dg[edge].symbolic;
		      psink->is_consistent = 
			    (pick_info->dg[edge].consistent != inconsistent);
		     }
		   else if (pick_info->dg[edge].consistent != inconsistent &&
			    !pick_info->dg[edge].symbolic &&
			    psink->gen_type != LCAV)
		  
		  /* current generator is only partially available, so
		     a LI pavail is chosen over LC */
		   
		     {
		      psink->generator = psrc->table_index;
		      psink->gen_distance = 0;
		      psink->gen_type = LIPAV;
		      psink->constant = !pick_info->dg[edge].symbolic;
		      psink->is_consistent =
			    (pick_info->dg[edge].consistent != inconsistent);
		     }
		}
	     }
	   edge = next_edge;
	  }
       }
   return(WALK_CONTINUE);
  }

static void look_for_gens(pick_info_type   *pick_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   AST_INDEX  stmt;
   Boolean    in_block = true,
              contains_div = false;


    ut_copy12(pick_info->LI_avail,pick_info->block->LI_avail_in);
    ut_copy12(pick_info->LI_rgen,pick_info->block->LI_rgen_in);
    ut_copy12(pick_info->LC_rgen_if_1,pick_info->block->LC_rgen_in_if_1);
    ut_copy12(pick_info->LI_pavail,pick_info->block->LI_pavail_in);
    ut_copy12(pick_info->LC_avail_if_1,pick_info->block->LC_avail_in_if_1);
    ut_copy12(pick_info->LC_pavail_if_1,pick_info->block->LC_pavail_in_if_1);
    stmt = pick_info->block->first;
    do
      {
       if (is_if(stmt))
         stmt = list_first(gen_IF_get_guard_LIST(stmt));
       else if (is_guard(stmt))
	 {
	  pick_info->def = false;
	  walk_expression(gen_GUARD_get_rvalue(stmt),get_gen,update_avail,
			  (Generic)pick_info);
	  stmt = list_first(gen_GUARD_get_stmt_LIST(stmt));
	 }
       else if (is_logical_if(stmt))
	 {
	  pick_info->def = false;
	  walk_expression(gen_LOGICAL_IF_get_rvalue(stmt),get_gen,update_avail,
			  (Generic)pick_info);
	  stmt = list_first(gen_LOGICAL_IF_get_stmt_LIST(stmt));
	 }
       else
	 {
	  if (is_assignment(stmt))
	    {
	     if (((config_type *)PED_MH_CONFIG(pick_info->ped))->soft_div)
	       walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),NOFUNC,
			       ut_check_div,(Generic)&contains_div);
	     if (!contains_div)
	       {
		pick_info->def = false;
		walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),get_gen,
				update_avail,(Generic)pick_info);
		pick_info->def = true;
		walk_expression(gen_ASSIGNMENT_get_lvalue(stmt),chk_store,
				update_avail,(Generic)pick_info);
		pick_info->lhs = gen_ASSIGNMENT_get_lvalue(stmt);
		walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),NOFUNC,
				kill_rhs,(Generic)pick_info);
	       }
	     else
	       {
		contains_div = false;
		ut_clear_set(pick_info->LI_avail);
		ut_clear_set(pick_info->LI_pavail);
		ut_clear_set(pick_info->LC_avail_if_1);
		ut_clear_set(pick_info->LC_pavail_if_1);
		ut_clear_set(pick_info->LI_rgen);
		ut_clear_set(pick_info->LC_rgen_if_1);
	       }
	    }
	  else if (is_write(stmt))
	    {
	     pick_info->def = false;
	     walk_expression(gen_WRITE_get_data_vars_LIST(stmt),NOFUNC,get_gen,
			     (Generic)pick_info);
	     walk_expression(gen_WRITE_get_data_vars_LIST(stmt),NOFUNC,
			     update_avail,(Generic)pick_info);
	    }
	  else if (is_read_short(stmt))
	    {
	     pick_info->def = true;
	     walk_expression(gen_READ_SHORT_get_data_vars_LIST(stmt),chk_store,
			     update_avail,(Generic)pick_info);
	    }
	  else if (is_arithmetic_if(stmt))
	    {
	     pick_info->def = false;
	     walk_expression(gen_ARITHMETIC_IF_get_rvalue(stmt),get_gen,
			     update_avail,(Generic)pick_info);
	    }
	  else if (is_call(stmt))
	    {
	     contains_div = false;
	     ut_clear_set(pick_info->LI_avail);
	     ut_clear_set(pick_info->LI_pavail);
	     ut_clear_set(pick_info->LC_avail_if_1);
	     ut_clear_set(pick_info->LC_pavail_if_1);
	     ut_clear_set(pick_info->LI_rgen);
	     ut_clear_set(pick_info->LC_rgen_if_1);
	    }
	  stmt = list_next(stmt);
	 }
       if (is_null_node(stmt))
         in_block = false;
       else if(get_stmt_info_ptr(stmt)->block != pick_info->block)
         in_block = false;
      } while(in_block);
  }

void sr_pick_possible_generators(flow_graph_type   flow_graph,
				 int               level,
				 prelim_info_type  *prelim_info,
				 PedInfo           ped)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   pick_info_type pick_info;

     pick_info.contains_cf = prelim_info->contains_cf;
     pick_info.dg = dg_get_edge_structure( PED_DG(ped));
     pick_info.ped = ped;
     pick_info.LI_avail = ut_create_set(prelim_info->ar,LOOP_ARENA,
					prelim_info->array_refs);
     pick_info.LI_rgen = ut_create_set(prelim_info->ar,LOOP_ARENA,
				       prelim_info->array_refs);
     pick_info.LC_rgen_if_1 = ut_create_set(prelim_info->ar,LOOP_ARENA,
					    prelim_info->array_refs);
     pick_info.LI_pavail = ut_create_set(prelim_info->ar,LOOP_ARENA,
					 prelim_info->array_refs);
     pick_info.LC_avail_if_1 = ut_create_set(prelim_info->ar,LOOP_ARENA,
					     prelim_info->array_refs);
     pick_info.LC_pavail_if_1 = ut_create_set(prelim_info->ar,LOOP_ARENA,
					      prelim_info->array_refs);
     pick_info.exit_block = flow_graph.exit;
     pick_info.level = level;
     for (pick_info.block = flow_graph.entry;
	  pick_info.block != flow_graph.exit;
	  pick_info.block = pick_info.block->next)
       look_for_gens(&pick_info);
  }
  
