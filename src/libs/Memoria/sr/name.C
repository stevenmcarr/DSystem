/* $Id: name.C,v 1.5 1992/12/11 11:22:21 carr Exp $ */
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
#include <general.h>
#include <sr.h>
#include <mh_ast.h>
#include <fort/walk.h>
#include <name.h>

#ifndef dt_h
#include <dt.h>
#endif

#include <mem_util.h>

static void do_partition(AST_INDEX name,
			 UtilList  *nlist,
			 DG_Edge   *dg,
			 PedInfo   ped)
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
 

  {
   scalar_info_type *sptr;
   int              refl;
   EDGE_INDEX       edge;

     sptr = get_scalar_info_ptr(name);
     sptr->list_index = util_node_alloc(name,"name"); 
     util_append(nlist,sptr->list_index);
     sptr->visited = true;
     refl = get_info(ped,name,type_levelv);
     for (edge = dg_first_src_ref( PED_DG(ped),refl);
	  edge != END_OF_LIST;
	  edge = dg_next_src_ref( PED_DG(ped),edge))
       if (dg[edge].type == dg_true || dg[edge].type == dg_input)
	 {
	  sptr = get_scalar_info_ptr(dg[edge].sink);
	  if(!sptr->visited)
	    do_partition(dg[edge].sink,nlist,dg,ped);
	 }
     for (edge = dg_first_sink_ref( PED_DG(ped),refl);
	  edge != END_OF_LIST;
	  edge = dg_next_sink_ref( PED_DG(ped),edge))
       if (dg[edge].type == dg_true || dg[edge].type == dg_input)
	 {
	  sptr = get_scalar_info_ptr(dg[edge].src);
	  if(!sptr->visited)
	    do_partition(dg[edge].src,nlist,dg,ped);
	 }
  }

static int partition_names(AST_INDEX      node,
			   name_info_type *name_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   scalar_info_type *sptr;
   AST_INDEX        name;
   name_node_type   *name_node;

     if (is_subscript(node))
       {
	name = gen_SUBSCRIPT_get_name(node);
	sptr = get_scalar_info_ptr(name);
	if (!sptr->visited && (sptr->generator != -1 || sptr->is_generator))
	  {
	   name_node = (name_node_type *)name_info->
	                  ar->arena_alloc_mem_clear(LOOP_ARENA,
						    sizeof(name_node_type));
	   util_append(name_info->glist,util_node_alloc((Generic)name_node,
							"generator"));
	   name_node->nlist = util_list_alloc((Generic)NULL,"name-list");
	   do_partition(name,name_node->nlist,name_info->dg,name_info->ped);
	  }
	else
	  sptr->visited = true;
       }
     return(WALK_CONTINUE);
  }

static int calc_distances(AST_INDEX node,
			  int       distance,
			  DG_Edge   *dg,
			  PedInfo   ped)
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
 

  {
   scalar_info_type *sptr;
   int              refl,
                    edist,
                    total,
                    max_dist = 0;
   EDGE_INDEX       edge;

     sptr = get_scalar_info_ptr(node);
     if (sptr->visited)
       {
	sptr->gen_distance = distance;
	sptr->visited = false;
	refl = get_info(ped,node,type_levelv);
	for (edge = dg_first_sink_ref( PED_DG(ped),refl);
	     edge != END_OF_LIST;
	     edge = dg_next_sink_ref( PED_DG(ped),edge))
	if (dg[edge].type == dg_true || dg[edge].type == dg_input)
	  if (dg[edge].level == LOOP_INDEPENDENT)
	    (void) calc_distances(dg[edge].src,distance,dg,ped);
	  else if ((edist = gen_get_dt_DIS(&dg[edge],dg[edge].level)) > 0)
	    (void) calc_distances(dg[edge].src,distance-edist,dg,ped);
	  else
	    (void) calc_distances(dg[edge].src,distance-1,dg,ped);
	for (edge = dg_first_src_ref( PED_DG(ped),refl);
	     edge != END_OF_LIST;
	     edge = dg_next_src_ref( PED_DG(ped),edge))
	  if (dg[edge].type == dg_true || dg[edge].type == dg_input)
	    if (dg[edge].level == LOOP_INDEPENDENT)
	      {
	       total = calc_distances(dg[edge].sink,distance,dg,ped);
	       if (total > max_dist)
	         max_dist = total;
	      }
	    else
	      { 
	       if ((edist = gen_get_dt_DIS(&dg[edge],dg[edge].level)) < 0)
	         edist = 1;
	       total =  edist + calc_distances(dg[edge].sink,
					       distance + edist,
					       dg,ped);
	       if (total > max_dist)
	         max_dist = total;
	      }
       }
   return(max_dist);
  }

static void check_if_oldest_value(AST_INDEX node,
				  UtilList  *nlist,
				  Boolean   *gen_not_found,
				  DG_Edge   *dg,
				  PedInfo   ped)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   EDGE_INDEX edge;
   int        sink_ref;
   scalar_info_type *sptr;

     *gen_not_found = false;
     sink_ref = get_info(ped,node,type_levelv);
     for (edge = dg_first_sink_ref( PED_DG(ped),sink_ref);
	  edge != END_OF_LIST && !*gen_not_found;
	  edge = dg_next_sink_ref( PED_DG(ped),edge))
       if(dg[edge].consistent != inconsistent && !dg[edge].symbolic)
         if (dg[edge].type == dg_output && dg[edge].src != dg[edge].sink)
	   {
	    sptr = get_scalar_info_ptr(dg[edge].src);
	    if (sptr->list_index != NULL)
	      if (util_in_list(sptr->list_index,nlist))
	        *gen_not_found = true;
	   }
	 else if (dg[edge].type == dg_true || 
		  (dg[edge].type == dg_input && gen_is_dt_DIS(&dg[edge])))
		   
	   *gen_not_found = true;
  }

void sr_find_generator(UtilNode *lnode,
		       DG_Edge  *dg,
		       PedInfo  ped)

  {
   UtilNode         *node;
   AST_INDEX        astnode;
   scalar_info_type *sptr,*gptr;
   Boolean          gen_not_found,
                    recurrence_ok;
   name_node_type   *name_node;

     gen_not_found = true;
     recurrence_ok = true;
     name_node = (name_node_type *)UTIL_NODE_ATOM(lnode);
     name_node->opt_will_allocate = false;
     for (node = UTIL_HEAD(name_node->nlist);
	  node != NULLNODE;
	  node = UTIL_NEXT(node))
       {
	astnode = (AST_INDEX)UTIL_NODE_ATOM(node);
	sptr = get_scalar_info_ptr(astnode);
	if (sptr->is_generator)
	  if (gen_not_found)
	    {
	     check_if_oldest_value(astnode,name_node->nlist,&gen_not_found,
				   dg,ped);
	     if (gen_not_found)
	       sptr->is_generator = false;
	     else
	       {
		sptr->num_regs = calc_distances(astnode,0,dg,ped) + 1;
		name_node->gen = astnode;
	       }
	    }
	  else if (sptr->generator != -1)
	    sptr->is_generator = false;
	  else if (sptr->prevent_rec)
	    recurrence_ok = false;
	if (sptr->gen_type == LIAV)
	  name_node->opt_will_allocate = true;
       }
     gptr = get_scalar_info_ptr(name_node->gen);
     gptr->recurrence = BOOL(gptr->recurrence & recurrence_ok);
     for (node = UTIL_HEAD(name_node->nlist);
	  node != NULLNODE;
	  node = UTIL_NEXT(node))
       {
	astnode = (AST_INDEX)UTIL_NODE_ATOM(node);
	sptr = get_scalar_info_ptr(astnode);
	sptr->recurrence = gptr->recurrence;
       }
  }

void sr_generate_names(AST_INDEX        root,
		       name_info_type   *name_info)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   UtilNode         *lnode;
   
     walk_expression(gen_DO_get_stmt_LIST(root),partition_names,NOFUNC,
		     (Generic)name_info);
     for (lnode = UTIL_HEAD(name_info->glist);
	  lnode != NULLNODE;
	  lnode = UTIL_NEXT(lnode))
       sr_find_generator(lnode,name_info->dg,name_info->ped);
  }
   
