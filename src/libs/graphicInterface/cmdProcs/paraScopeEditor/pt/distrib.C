/* $Id: distrib.C,v 1.1 1997/06/25 13:52:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ped_cp/PEditorCP/pt/distrib.c					*/
/*									*/
/*	distrib.c -- creates the data structures to find a distribution */
/*		     partition.  Includes:				*/
/*									*/
/*		pt_build_do_nodes					*/
/*		pt_build_do_nodes2					*/
/*		pt_build_stmt_nodes					*/
/*		pt_build_do_edges					*/
/*		pt_build_stmt_edges					*/
/*		pt_add_stmt_edges					*/
/*		pt_build_scexpnd_edges					*/
/*		pt_build_stmt_edges2					*/
/*		pt_count_par_stmts					*/
/*		pt_make_adj_list					*/
/*		pt_handle_scexpnd_query2				*/
/*									*/
/*									*/
/************************************************************************/

#include <libs/moduleAnalysis/dependence/utilities/strong.h>
#include <libs/frontEnd/ast/groups.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/pt/private_pt.h>

/* Forward declarations */
STATIC(void, pt_build_stmt_edges,(PedInfo ped, Adj_List *adj_list, AST_INDEX node,
                                  int level, int depth));
STATIC(Boolean, pt_check_user_divide,(PedInfo ped, Adj_List *adj_list, int level));

void pt_build_stmt_nodes (PedInfo ped, Adj_List *adj_list, AST_INDEX node,
                          int level, AST_INDEX do_node);
void pt_add_stmt_edges (PedInfo ped, Adj_List *adj_list, AST_INDEX node,
                        int carry_level, int depth);
void pt_add_stmt_edges2 (PedInfo ped, Adj_List *adj_list, AST_INDEX node,
                         int level, int depth, char *var);


/* pt_build_do_nodes - walks the tree allocating `nodes' for every 
 * 	statement and do node loop (extend for cflow!!)
 */
static void 
pt_build_do_nodes(PedInfo ped, Adj_List *adj_list, AST_INDEX node, int level)
{
    AST_INDEX 	do_node;       
    
    if (!is_loop(node)) 
	return;
    
    for (node = list_first(gen_DO_get_stmt_LIST(node)); 
	 node != AST_NIL; node = list_next(node))
    {
       	if (is_loop(node)) 
	    do_node = node;
       	else
	    do_node = AST_NIL;
	pt_build_stmt_nodes(ped, adj_list, node, level, do_node);
    }
}

static void 
pt_build_do_nodes2(PedInfo ped, Adj_List *adj_list, AST_INDEX node, 
                   int level, AST_INDEX do_node)
{
    if (!is_loop(node))  return;
    
    for (node = list_first(gen_DO_get_stmt_LIST(node)); 
	 node != AST_NIL; node = list_next(node))
    {
       	pt_build_stmt_nodes(ped, adj_list, node, level, do_node);
    }
}

void 
pt_build_stmt_nodes(PedInfo ped, Adj_List *adj_list, AST_INDEX node, 
                    int level, AST_INDEX do_node)
{
    AST_INDEX tnode;
    
    if (!is_executable(node))
	return;

    if (is_loop(node))
	pt_build_do_nodes2(ped, adj_list, node, level, do_node);
    else if (is_compound(node))
    {
	(void) pt_allocnode(adj_list, node, do_node);
	if (gen_get_node_type(node) == GEN_IF)
	{
	    for (node = list_first(gen_IF_get_guard_LIST(node)); 
		 node != AST_NIL; 
		 node = list_next(node))
	    {
		for (tnode = list_first(gen_GUARD_get_stmt_LIST(node)); 
		     tnode != AST_NIL; 
		     tnode = list_next(tnode))
		{
		    pt_build_stmt_nodes(ped, adj_list, tnode, level, do_node);
		}
	    }
	}
    }
    else
	(void) pt_allocnode(adj_list, node, do_node);
}


void 
pt_build_do_edges(PedInfo ped, Adj_List *adj_list, AST_INDEX node, 
                  int level, int depth)
{
    if (!is_loop(node))  return;
    
    node = list_first(gen_DO_get_stmt_LIST(node));
    while(node != AST_NIL)
    {
	pt_build_stmt_edges(ped, adj_list, node, level, depth);
	node = list_next(node);
    }
}

static void 
pt_build_stmt_edges(PedInfo ped, Adj_List *adj_list, AST_INDEX node, 
                    int level, int depth)
{
    AST_INDEX	tnode;
    
    if (is_loop(node))
	pt_build_do_edges(ped, adj_list, node, level, ++depth);
    else if (is_compound(node))
    {
	pt_add_stmt_edges(ped, adj_list, node, level, depth);
	if (gen_get_node_type(node) == GEN_IF)
	{
	    for (node = list_first(gen_IF_get_guard_LIST(node)); 
		 node != AST_NIL; 
		 node = list_next(node))
	    {
		for (tnode = list_first(gen_GUARD_get_stmt_LIST(node)); 
		     tnode != AST_NIL; 
		     tnode = list_next(tnode))
		{
		    pt_add_stmt_edges(ped, adj_list, tnode, level, depth);
		}
	    }
	}
    }
    else
	pt_add_stmt_edges(ped, adj_list, node, level, depth);
}

void 
pt_add_stmt_edges(PedInfo ped, Adj_List *adj_list, AST_INDEX node, 
                  int carry_level, int depth)
{
    int vector;
    int edge;
    DG_Edge *edgeptr;
    AST_INDEX sink_node;
    int level;
    Boolean carried;

    if (!is_executable(node))
	return;

    vector  = get_info(ped, node, type_levelv);
    edgeptr = dg_get_edge_structure( PED_DG(ped));
    
    /*
     *  Get edgeptr                        
     */
    
    for (level = LOOP_INDEPENDENT; level <= depth; level++)
    {
	if ((level > 0 ) && (level < carry_level)) 
	    continue;
	
	for (edge = dg_first_src_stmt( PED_DG(ped),vector,level); edge != NIL; 
	     edge = dg_next_src_stmt( PED_DG(ped),edge))
	{
	    sink_node = edgeptr[edge].sink;
	    if (!is_statement(sink_node))
		sink_node = out(sink_node);
	    
	    if (level == carry_level || level == 0)	/* SCALAR */
		carried = true;
	    else 
		carried = false;
	    pt_add_edge(adj_list, node , sink_node , carried);
	}
    }
}

   


void 
pt_build_scexpnd_edges (PedInfo ped, Adj_List *adj_list, AST_INDEX node,
                        int level, int depth, char *var)
{
    if (!is_loop(node))  return;
    
    node = list_first(gen_DO_get_stmt_LIST(node));
    while(node != AST_NIL)
    {
	pt_build_stmt_edges2 (ped, adj_list, node, level, depth, var);
	node = list_next(node);
    }
}

void 
pt_build_stmt_edges2 (PedInfo ped, Adj_List *adj_list, AST_INDEX node, 
                      int level, int depth, char *var)
{
    if (is_loop(node))
	pt_build_scexpnd_edges (ped, adj_list, node, level, ++depth, var);
    else
	pt_add_stmt_edges2 (ped, adj_list, node, level, depth, var);
}

void 
pt_add_stmt_edges2 (PedInfo ped, Adj_List *adj_list, AST_INDEX node, 
                    int carry_level, int depth, char *var)
{
    int vector;
    int edge;
    DG_Edge *edgeptr;
    AST_INDEX sink_node;
    int level;
    Boolean carried;
    int type;

    if (!is_executable(node))
	return;
  
    vector  = get_info(ped, node, type_levelv);
    edgeptr = dg_get_edge_structure( PED_DG(ped));
    
    for (level = LOOP_INDEPENDENT; level <= depth; level++)
    {
	if ((level > 0 ) && (level < carry_level)) 
	{
	    level++;
	    continue;
	}
	for (edge = dg_first_src_stmt( PED_DG(ped), vector, level); edge != NIL; 
	     edge = dg_next_src_stmt( PED_DG(ped), edge))
	{
	    if  (! se_removable_edge (ped, edge, var))  {
		sink_node = edgeptr[edge].sink;
		if (!is_statement(sink_node))
		    sink_node = out(sink_node);
		
		if (level == carry_level || level == 0)	/* SCALAR */
		    carried = true;
		else 
		    carried = false;
		pt_add_edge (adj_list, node, sink_node, carried);
	    }
	}
    }
}

int  
pt_count_par_stmts (Adj_List *adj_list)
{
   int i, count , stmt;

   count = 0;
   for(i = 0; i <= adj_list->max_region; i++)
   {
      if (adj_list ->region_array[i].parallel == true)
      {
          for( stmt = adj_list -> region_array[i].first_stmt;stmt != NIL;
	       stmt = adj_list-> node_array[stmt].rlink)
	     count++;
      }
   }
   return (count);
}
	 
/*  pt_make_adj_list - top level call to find recurrences.
 */
Adj_List *
pt_make_adj_list (PedInfo ped, AST_INDEX node)
{
    Adj_List *adj_list;
    int       level = loop_level(node);

    adj_list = pt_create_adj_list(); 
    pt_build_do_nodes (ped, adj_list, node, level);
    pt_build_do_edges (ped, adj_list, node, level, level);
    pt_strong_regions (adj_list);
    return(adj_list);
}
int pt_handle_scexpnd_query1 (PedInfo ped, AST_INDEX node, int level, 
                              char *var)
{
    Adj_List   *adj_list;
    int		new_no_of_regions;
    int		old_no_of_regions;
    
    
    adj_list = pt_make_adj_list (ped, node);   
    old_no_of_regions = (adj_list -> max_region) + 1;   
    pt_destroy_adj_list (adj_list);  
    adj_list = pt_create_adj_list();
    pt_build_do_nodes (ped, adj_list, node, level);       
    pt_build_scexpnd_edges (ped, adj_list, node, level, level, var);
    pt_strong_regions (adj_list);
    new_no_of_regions = (adj_list -> max_region) + 1;
    pt_destroy_adj_list (adj_list);
    return (new_no_of_regions - old_no_of_regions);
}


/* Routine to be calleld by scalar expansion routines
 * to check if scalar expansion helps in reducing the
 * no of strongly connected region
 */
int 
pt_handle_scexpnd_query2 (PedInfo ped, AST_INDEX node, int level, char *var)
{
    Adj_List *adj_list;
    int       old_no_of_par_stmts;
    int       new_no_of_par_stmts;
    
    adj_list = pt_make_adj_list (ped, node);
    old_no_of_par_stmts = pt_count_par_stmts (adj_list);
    pt_destroy_adj_list (adj_list);  
    adj_list = pt_create_adj_list();
    pt_build_do_nodes (ped, adj_list, node, level);       
    pt_build_scexpnd_edges (ped, adj_list, node, level, level, var);
    pt_strong_regions (adj_list);
    new_no_of_par_stmts = pt_count_par_stmts (adj_list);
    pt_destroy_adj_list (adj_list);
    return (new_no_of_par_stmts - old_no_of_par_stmts);
}


/* 
 * 
 */
AST_INDEX
pt_do_user_distrib(PedInfo ped, AST_INDEX do_node, ptDivideFunc pt_user_divide_loop, 
                   ptDistribFunc pt_user_post_distrib, Generic user_struct)
{
 Adj_List *adj_list;
 int level;
 AST_INDEX return_node;
 adj_list = pt_make_adj_list(ped, do_node);
 level = loop_level(do_node);
 pt_user_divide_loop(ped,adj_list,level, do_node, user_struct); 
  
/* check if distribution needs to be done */
  if (pt_check_user_divide(ped, adj_list,level)) {
    pt_make_loop_array (adj_list);
   	return_node = pt_rebuild_tree (ped, adj_list, do_node, level); 
   	pt_rebuild_graph (ped, adj_list, level, return_node, do_node);
 
/* user may want to do some patching up of information after distribution
   has been done
*/
    if(pt_user_post_distrib != NULL)
      pt_user_post_distrib(return_node, adj_list, user_struct);
    	return (return_node);
   	}
 else
  return(do_node);
}


/* 
 * 
 */
static Boolean
pt_check_user_divide(PedInfo ped, Adj_List *adj_list, int level)
{
 if (adj_list->max_loop == 0)
   return(false);
 else
   return(true);
}


