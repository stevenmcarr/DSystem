/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
#include <sr.h>
#include <Arena.h>
#include "insert.h"
#include "dfantic.h"

static void hoist_inserts(block_type *entry,
			  int        size,
			  arena_type *ar)
  
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   edge_type  *edge;
   block_type *block;
   Set        temp;

     temp = ut_create_set(ar,LOOP_ARENA,size);
     ut_complement(temp);
     for (block = entry;
	  block != NULL;
	  block = block->next)
       {
	for (edge = block->succ;
	     edge != NULL;
	     edge = edge->next_succ)
	  ut_intersect121(temp,edge->Insert);
	if (!ut_set_is_empty(temp))
	  {
	   ut_union121(block->Insert,temp);
	   for (edge = block->succ;
		edge != NULL;
		edge = edge->next_succ)
	   ut_difference121(edge->Insert,temp);	   
	  }
       }
  }

static void analyze_block(block_type *block,
			  Boolean    first,
			  int        size,
			  arena_type *ar,
			  PedInfo    ped)
  {
   edge_type  *edge;
   Set         temp,
               temp1;

     if (first)
       sr_calc_local_antic(block,size,ped);
     temp = ut_create_set(ar,LOOP_ARENA,size);
     ut_clear_set(block->PP_out);
     ut_complement(block->PP_out);
     for (edge = block->succ;
	  edge != NULL;
	  edge = edge->next_succ)
       ut_intersect121(block->PP_out,edge->to->PP_in);
     ut_copy12(temp,block->PP_out);
     ut_union121(temp,block->gen);
     temp1 = ut_create_set(ar,LOOP_ARENA,size);
     ut_copy12(temp1,block->LC_pavail_in);
     ut_union121(temp1,block->LI_pavail_in);
     ut_intersect121(temp,temp1);
     ut_copy12(temp1,block->LC_antic_in);
     ut_union121(temp1,block->LI_antic_in);
     ut_intersect121(temp,temp1);
     ut_copy12(block->PP_in,temp);
  }

static void walk_flow_graph(block_type *block,
			    Boolean    unvisited_mark,
			    Boolean    first,
			    int        size,
			    arena_type *ar,
			    PedInfo    ped)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   edge_type *edge;

     if (block->mark == unvisited_mark)
       {
	block->mark = (Boolean)!unvisited_mark;
	for (edge = block->succ;
	     edge != NULL;
	     edge = edge->next_succ)
	  walk_flow_graph(edge->to,unvisited_mark,first,size,ar,ped);
	analyze_block(block,first,size,ar,ped);
       }
  }

static void compute_edge_inserts(block_type *block,
				 Boolean    unvisited_mark,
				 block_type *exit,
				 int        size,
				 arena_type *ar)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   edge_type *edge;
   Set         temp1,temp2;

     temp1 = ut_create_set(ar,LOOP_ARENA,size);
     temp2 = ut_create_set(ar,LOOP_ARENA,size);
     if (block->mark == unvisited_mark)
       {
	block->mark = (Boolean)!unvisited_mark;
	if (block != exit)
	  for (edge = block->succ;
	       edge != NULL;
	       edge = edge->next_succ)
	    {
	     ut_copy12(temp1,edge->from->LI_avail_out);
	     ut_complement(temp1);
	     ut_copy12(temp2,edge->from->PP_out);
	     ut_complement(temp2);
	     ut_intersect121(temp1,temp2);
	     ut_intersect121(temp1,edge->to->PP_in);
	     ut_copy12(edge->Insert,temp1);
	     compute_edge_inserts(edge->to,unvisited_mark,exit,size,ar);
	  }
       }
  }

void sr_perform_insert_analysis(flow_graph_type  flow_graph,
				int              size,
				arena_type       *ar,
				PedInfo          ped)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   Set_info   Insert_sets;
     
     walk_flow_graph(flow_graph.entry,flow_graph.entry->mark,true,size,ar,ped);
     walk_flow_graph(flow_graph.entry,flow_graph.entry->mark,false,size,ar,
		     ped);
     compute_edge_inserts(flow_graph.entry,flow_graph.entry->mark,
			  flow_graph.exit,size,ar);
     hoist_inserts(flow_graph.entry,size,ar);
  }
