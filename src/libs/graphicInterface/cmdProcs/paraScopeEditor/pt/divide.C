/* $Id: divide.C,v 1.1 1997/06/25 13:52:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ped_cp/pt/divide.c						*/
/*									*/
/*	divide.c -- divide the statements of the loop for distributions	*/
/*	Includes the following routines:				*/
/*									*/
/*		pt_divide_loop						*/
/*		pt_look_back						*/
/*		pt_look_front						*/
/*		pt_make_loop_array					*/
/*		pt_rebuild_tree						*/
/*		pt_rebuild_graph					*/
/*		pt_find_distribution					*/
/*		pt_print_distribution					*/
/*		pt_do_distribution					*/
/*									*/
/*  Loop distribution shows exactly what it is going to do. First 	*/
/*  to preview the distribution do 1, 2, and 3:				*/
/*    1) Call pt_make_adj_list first, followed by			*/
/*    2) Call pt_find_distribution 					*/
/*    3) call pt_print_distribution to get the message to be output.	*/
/*  To accomplish the distribution in the code :			*/
/*    4) call pt_do_distribution 					*/
/*									*/
/*  9/90 - Edited by kats to add vector as well as maximal 		*/
/*	distribution and for control flow.				*/
/************************************************************************/

#include <libs/moduleAnalysis/dependence/utilities/strong.h>
#include <libs/moduleAnalysis/dependence/controlDependence/cd_graph.h>
					/* dstr_cd_graph(); and ...();	*/
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/pt/private_pt.h>

#include <libs/support/stacks/xstack.h>


/* forward declarations */
Boolean pt_look_front();
AST_INDEX find_new_loop ();


void pt_divide_loop (PedInfo ped, Adj_List *adj_list, int level, int type)
{
    AST_INDEX    do_node;
    int          section_number;    
    Boolean      parallel;
    Adj_Node    *node_array;
    int          current_node;
    int          i;
    int          region;
    int	         old_last_node;
    int          last_node;
    int          stmt;
    Region_Type *region_array;
    
    section_number = -1;  
    current_node   = 0;
    node_array 	  = adj_list -> node_array;
    region_array   = adj_list -> region_array;
    while (current_node <= adj_list -> last_used_node)
    {
	section_number++;
	parallel = true;
	/* build a new section of the distributed loop */
	last_node = current_node;
	do
	{
	    if (last_node > current_node)  current_node++;
	    
	    /* include everything in the strongly connected region */
	    region = node_array[current_node].region;
	    if (region_array[region].parallel == false) parallel = false; 
	    if (region_array[region].visited == false)
	    {
		region_array[region].visited = true;
		stmt = region_array[region].first_stmt;
		while(stmt!= NIL)
		{
		    node_array[stmt].loop = section_number;
		    if (stmt > last_node)  last_node = stmt;
		    /* look for backward dependences */
		    old_last_node = last_node; 
		    last_node = pt_look_back(ped,last_node, adj_list, 
					     node_array[stmt].statement, level);
		    
		    /* Backward carried dependence = not parallel */
		    if (last_node != old_last_node)
		    {
			parallel = false;
			region_array[region].parallel = false; 
		    }
		    stmt = node_array[stmt].rlink;
		}
	    }
	    /* include everything that is nested at the same level */
	    do_node = node_array[current_node].do_node;
	    if (do_node != AST_NIL && current_node == last_node)
	    {
		for(i=current_node; 
		    (node_array[i].do_node == do_node) && (i <= adj_list ->last_used_node); 
		    i++);
		last_node = i - 1;
	    }
	    
	    /* include the next statement if it has the same parallelism status
	       as the last one */
	    if ((type == PG_PARALLEL) && (current_node == last_node) && 
		(current_node < adj_list -> last_used_node)) 
	    {
		region = node_array[current_node + 1].region;
		if (region_array[region].parallel == parallel)
		{
		    /*  add if its a single stmt region              */
		    /*  and there are no forward carried dependences */
		    stmt = region_array[region].first_stmt;
		    if ((node_array[stmt].rlink == NIL) &&
			(stmt == pt_look_back(ped, stmt, adj_list, 
					      node_array[stmt].statement, level))) 
		    {		     
			if ((parallel == false) || 
			    (pt_look_front(ped, stmt, adj_list, level) == false))
			    last_node++;
		    } 
		}
	    }        
	} while(last_node > current_node);
	current_node++;
    }   
    adj_list -> max_loop = section_number;
}

/* This looks if backward dependences cause additional stmts to be
   included in the new loop region being constructed   */

int 
pt_look_back (PedInfo ped, int last_node, Adj_List *adj_list, 
              AST_INDEX stmt, int level)
{
    int        edge,vector;
    int        src_index;
    DG_Edge    *edgeptr;
    AST_INDEX  src_node;
    
    vector  = get_info (ped, stmt, type_levelv);
    edgeptr = dg_get_edge_structure( PED_DG(ped));
    edge    = dg_first_sink_stmt( PED_DG(ped), vector, level);
    
    while(edge != NIL)
    {
	src_node = edgeptr[edge].src;        
	if (!is_statement(src_node))
	    src_node = out (src_node);
	src_index = pt_getindex(adj_list, src_node);
	if (src_index > last_node)  last_node = src_index ;
	edge = dg_next_sink_stmt( PED_DG(ped),edge);
    }
    return(last_node);
}

/* check if this stmt is the sink of a carried forward edge  */

Boolean 
pt_look_front (PedInfo ped, int stmt_index, Adj_List *adj_list, int level)
{
    int        edge,vector;
    int        src_index;
    DG_Edge    *edgeptr;
    AST_INDEX  stmt;
    AST_INDEX  src_node;
    
    stmt = (adj_list -> node_array)[stmt_index].statement;
    vector  = get_info(ped, stmt, type_levelv);
    edgeptr = dg_get_edge_structure( PED_DG(ped));
    
    edge    = dg_first_sink_stmt( PED_DG(ped), vector, level);
    
    while(edge != NIL)
    {
	src_node = edgeptr[edge].src;        
	if (!is_statement(src_node))
	    src_node = out (src_node);
	src_index = pt_getindex(adj_list, src_node);
	if (src_index < stmt_index)  return(true);
	edge = dg_next_sink_stmt( PED_DG(ped),edge);
    }
    return(false);
}



   


/* Put the endpoints of all distributed loops in an array     */

void 
pt_make_loop_array (Adj_List *adj_list)
{
    int          node;
    int          region;
    Adj_Node    *node_array;
    Region_Type *region_array;
    int          total_loops;
    Loop_Type   *loop_list;
    int          loop;
    Boolean      parallel;
    UtilNode	*unode;
    
    node_array  = adj_list -> node_array;
    total_loops = adj_list -> max_loop + 1;
    loop_list = (Loop_Type *) get_mem(total_loops * sizeof(Loop_Type), 
                                      "pt_make_loop_array" );
    region_array = adj_list -> region_array;
    loop = -1;
    for (node = 0 ; node <= adj_list -> last_used_node; node++)
    {
	/* make one loop  */      
	loop++;
	loop_list[loop].stmts = util_list_alloc (loop, "pt_make_loop_array");
	parallel = true;
	
	if (node_array[node].do_node != AST_NIL)
	{
	    unode = util_node_alloc (node_array[node].do_node, "pt_make_loop_array");
	    util_append (loop_list[loop].stmts, unode);
	}
	for ( ; node <= adj_list -> last_used_node && node_array[node].loop == loop ; node++) 
	{
	    region = node_array[node].region;
	    if (region_array[region].parallel == false)
		parallel = false;
	    if (node_array[node].do_node == AST_NIL)
	    {
	 	unode = util_node_alloc (node_array[node].statement, "pt_make_loop_array");
	 	util_append (loop_list[loop].stmts, unode);
	    }
	}	  
	node--;
	loop_list[loop].parallel = parallel;        
    }
    adj_list -> loop_array = loop_list;
}

AST_INDEX 
pt_rebuild_tree(PedInfo ped, Adj_List *adj_list, AST_INDEX old_do, int level)
{
    AST_INDEX    return_node;
    AST_INDEX    new_do;
    AST_INDEX    prev_loop;
    AST_INDEX    new_lbl_def;
    AST_INDEX    new_lbl_ref;
    AST_INDEX    new_control;
    AST_INDEX    new_close_lbl_def;
    AST_INDEX    new_stmt_list;
    
    AST_INDEX    old_stmt_list;
    AST_INDEX    stmt;
    int          new_loop;
    Loop_Type    *loop_array;
    META_TYPE    meta_type;
    
    UtilNode	*unode;
    Stack	ifstmts;
    AST_INDEX	ifstmt;
    
    loop_array = adj_list -> loop_array;
    
    prev_loop = old_do;
    meta_type = ast_get_meta_type(old_do);
    old_stmt_list = gen_DO_get_stmt_LIST(old_do);
    
    
    ifstmts = stack_create (sizeof(AST_INDEX));
    /* Create a loop and stick it just after the last added old loop */
    for (new_loop = 0; new_loop <= adj_list -> max_loop ; new_loop++)
    {
	
	/* delete the label, if any on the statement  */
	new_lbl_def = AST_NIL;
	
	/* generate a new label reference              */
	new_lbl_ref = AST_NIL;
	new_control = tree_copy(gen_DO_get_control(old_do)); 
	new_close_lbl_def = AST_NIL;
	
	
	/* find and copy the stmt subtree corresponding to the new loop */
	new_stmt_list = list_create(AST_NIL);
	ifstmt = AST_NIL;
	stack_push(ifstmts, (Generic*)&ifstmt);
	
	for (unode = UTIL_HEAD(loop_array[new_loop].stmts); 
	     unode != NULLNODE; 
	     unode = UTIL_NEXT(unode)) 
	{
	    stmt = UTIL_NODE_ATOM(unode);
	    if (ifstmt == AST_NIL)
	    {
		list_remove_node(stmt);
		list_insert_last(new_stmt_list,stmt);
	    }
	    else if (ifstmt != out(out(stmt)))
	    {
		if (ifstmt != out(stmt))
		{
		    for ( stack_pop(ifstmts, (Generic*)&ifstmt); 
			 (ifstmt != AST_NIL) && (ifstmt != out(out(stmt)));
			 stack_pop(ifstmts, (Generic*)&ifstmt) )
		    {
			if (ifstmt == out(stmt) || (ifstmt == AST_NIL))
			    break;
		    }
		}
		if (ifstmt == AST_NIL)
		{
		    list_remove_node(stmt);
		    list_insert_last(new_stmt_list,stmt);
		}
	    }		
	    if (is_if(stmt))
	    {
		ifstmt = stmt;
		stack_push(ifstmts, (Generic*)&ifstmt);
	    }
	}
	for ( ; ifstmt != AST_NIL; stack_pop(ifstmts, (Generic*)&ifstmt))
	    ;
	
	new_do = gen_DO(new_lbl_def, new_close_lbl_def, 
			new_lbl_ref, new_control, new_stmt_list);   
	ast_put_meta_type (new_do, meta_type);
	el_add_loop( PED_LI(ped), prev_loop, new_do, level);		      
	el_copy_shared_list( PED_LI(ped), old_do, new_do);
	el_copy_private_list( PED_LI(ped), old_do, new_do);  
	list_insert_before (old_do, new_do);
	prev_loop = new_do;

	if(new_loop == 0)  
	    return_node = new_do;   
    }
    stack_destroy(ifstmts);
    
    /* this part may be shifted elsewhere                */
    el_remove_loop( PED_LI(ped), old_do); 
    list_remove_node (old_do);    
    return (return_node);
}

/************************************************************************/
/*  pt_rebuild_graph 							*/
/*	Deletes data dependences between the new loops.  Moves any	*/
/*	control dependences that have been modelled.  Updates		*/
/*	cds to & from new loop headers and new statements.		*/
/************************************************************************/

void
pt_rebuild_graph (PedInfo ped, Adj_List *adj_list, int level, 
                  AST_INDEX first_do, AST_INDEX old_do)
{
    int		edge, next, new_edge, vector, lvector;
    int		src_index, sink_index;
    int		cur_level, i;
    DG_Edge    *edgeptr;
    AST_INDEX	stmt, sink_node, src, cdSrc, new_do;
    Adj_Node   *node_array;
    
    node_array = adj_list -> node_array;
    edgeptr    = dg_get_edge_structure( PED_DG(ped));

    /* Looks at and correct all the control and data dependences for
     * non-loop statements, based on their position in the newly created
     * loop nests.
     */

    /* Correct all the control dependences with their source at old_do.
     */
    vector = get_info (ped, old_do, type_levelv);
    for (cur_level = LOOP_INDEPENDENT; cur_level <= level; cur_level++)
    {	
	for (edge = dg_first_src_stmt( PED_DG(ped), vector, cur_level); edge != NIL;
	     edge = next)
	{
	    next = dg_next_src_stmt( PED_DG(ped), edge);

	    if (edgeptr[edge].type != dg_control)
		continue;
	    
	    if (edgeptr[edge].src == edgeptr[edge].sink)
		continue;
	    
	    if ((new_do = find_new_loop (level, edgeptr[edge].sink))
		!= AST_NIL)
	    {
		dg_delete_edge( PED_DG(ped), edge);
		if ((lvector = get_info (ped, new_do, type_levelv)) == -1)
		{
		    lvector = dg_alloc_level_vector( PED_DG(ped), MAXLOOP);
		    put_info (ped, new_do, type_levelv, lvector);
		}
		edgeptr[edge].src_vec = lvector;
		edgeptr[edge].src     = new_do;
		dg_add_edge( PED_DG(ped), edge);
	    }
	    else
		printf("Confusion in cd update in pt_rebuild_graph\n");
	}
    }
    /* Copy the control dependences with their sink at the old_do, to each
     * of the new_do loops.
     */
    for (cur_level = LOOP_INDEPENDENT; cur_level <= level; cur_level++)
    {	
	for (edge = dg_first_sink_stmt( PED_DG(ped), vector, cur_level); edge != NIL;
	     edge = next)
	{
	    next = dg_next_sink_stmt( PED_DG(ped), edge);
	    
	    if (edgeptr[edge].type != dg_control)
		continue;

	    dg_delete_edge( PED_DG(ped), edge);
	    
	    /* All the new do's */
	    new_do = first_do;
	    for (i = 0; i <= adj_list->max_loop; i++)
	    {
		new_edge = dg_alloc_edge( PED_DG(ped), &edgeptr);
		el_copy_edge( PED_DG(ped), PED_DT_INFO(ped), edgeptr, edge, new_edge);
		if ((lvector = get_info (ped, new_do, type_levelv)) == -1)
		{
		    lvector = dg_alloc_level_vector( PED_DG(ped), MAXLOOP);
		    put_info (ped, new_do, type_levelv, lvector);
		}
		if (edgeptr[new_edge].sink == edgeptr[new_edge].src)
		{
		    edgeptr[new_edge].src_vec = lvector;
		    edgeptr[new_edge].src     = new_do;
		}
		edgeptr[new_edge].sink_vec    = lvector;
		edgeptr[new_edge].sink        = new_do;

		dg_add_edge( PED_DG(ped), new_edge);
		
		new_do = list_next(new_do);
	    }
	    dg_free_edge( PED_DG(ped), edgeptr, edge);
	}
    }

    /* Update data & control dependences which cross between regions
     */
    for (src_index = 0 ; src_index <= adj_list -> last_used_node ; src_index++)
    {
	stmt    = node_array[src_index].statement;
	vector  = get_info(ped, stmt, type_levelv);
	
	for (cur_level = LOOP_INDEPENDENT; cur_level <= level; cur_level++)
	{
	    for (edge = dg_first_src_stmt( PED_DG(ped), vector, cur_level); 
		 edge != NIL; edge = next)
	    {
		next = dg_next_src_stmt( PED_DG(ped), edge);
	
            	sink_node = edgeptr[edge].sink;  
            	if (!is_statement(sink_node))
		    sink_node = out (sink_node);
	    	sink_index = pt_getindex(adj_list, sink_node); 
		
	    	if (node_array[sink_index].loop != node_array[src_index].loop)
	    	{
		    /* delete dependence */
		    dg_delete_edge( PED_DG(ped), edge);
		    
		    if (edgeptr[edge].type == dg_control)
		    { /* add it back with new src */
			for (src = out(sink_node); !is_if(src); src = out(src))
			{
			    if (src == AST_NIL)
				break;
			}
			edgeptr[edge].src = src;
			if ((lvector = get_info (ped, src, type_levelv)) == -1)
			{
			    lvector = dg_alloc_level_vector( PED_DG(ped), MAXLOOP);
			    put_info (ped, src, type_levelv, lvector);
			}
			edgeptr[edge].src_vec = lvector;
			dg_add_edge( PED_DG(ped), edge);
		    }
		    else		/* dg_free_edge, 910604, mpal	*/
			dg_free_edge( PED_DG(ped), edgeptr, edge );
	    	}
	    }
	}
    }
}
  
AST_INDEX
find_new_loop (int level, AST_INDEX stmt)
{
    AST_INDEX loop = stmt;

    for ( ; loop != AST_NIL; loop = out(loop) )
    {
	if (!is_do(loop))
	    continue;
	if (level == loop_level(loop))
	    return loop;
    }
    return AST_NIL;
}

/*  The next three routines were written so that loop distribution
    shows exactly what it does - a popular demand. The way it works 
    now is :

    1) Call pt_make_adj_list as always
    2) Call pt_find_distribution 
    3) Do not call pt_print_regions, call pt_print_distribution
       instead to get the message to be output.
    4) When the user wants the actual distribution to be done,
       call pt_do_distribution.

      jss  Sep 88
 */
int       
pt_find_distribution (PedInfo ped, AST_INDEX loop, Generic PT, int type)
{
    Adj_List   *adj_list;   
    int        level = loop_level(loop);    

    adj_list = (Adj_List *) PT;
    pt_divide_loop (ped, adj_list, level, type);
    pt_make_loop_array (adj_list);
    if (adj_list -> max_loop == 0)
	return(CANNOT_CHANGE);
    else return (CAN_CHANGE);
}
 

 
char *
pt_print_distribution(Generic AL, Generic handle)
{      
    static char  s[20];
    static char  msg[2000];   /* temporary HACK */
    Adj_List *adj_list;
    Adj_Node *node_array;
    int    node;
    AST_INDEX  stmt;
    int   this_loop;
    
    adj_list = (Adj_List *) AL;
    msg[0] = '\0';
    node_array = adj_list -> node_array;
    
    for(node=0; node<= adj_list->last_used_node; )
    {
	this_loop = node_array[node].loop;
	sprintf(s, "\nLoop %d", this_loop);
	strcat(msg, s);
	if (adj_list->loop_array[this_loop].parallel)
	    strcat(msg, ("(parallel):                     \n"));
	else
	    strcat(msg, ("(serial):                       \n"));
	
	while ((node_array[node].loop == this_loop) &&     
	       (node <= adj_list -> last_used_node))
	{
            stmt = node_array[node].statement;
            strcat(msg, pt_get_stmt_text((PedInfo)handle, stmt));
            strcat(msg, "\n");
	    node++;
	    
	}
    }
    if(adj_list -> max_loop == 0)
    {
	if (adj_list -> max_region == 0)
	{
	    strcpy(msg, "All the statements in the loop form a strongly\nconnected region of the dependence graph.\nThe loop cannot be distributed.\n");
	}
	else
	{
	    strcpy(msg, "The loop distribution algorithm will put\nall the statements in a single loop.\nThis may be for optimization purposes \nor to honor the constraints imposed on \nthe algorithm.\n");
	}
    }                    
    
    return(msg);
}
       
   
AST_INDEX
pt_do_distribution (PedInfo ped, AST_INDEX do_node, int type)
{
    AST_INDEX    return_node;
    Adj_List    *adj_list;
    ControlDep  *cd;
    int          level = loop_level (do_node);

    adj_list =  pt_make_adj_list (ped, do_node);

    pt_divide_loop (ped, adj_list, level, type);
    pt_make_loop_array (adj_list);
    cd = dstr_cd_graph (PED_DG(ped), PED_LI(ped), PED_INFO(ped),
			adj_list, do_node); 
    if(   (cd == NULL) 
       || (!dstr_cd_restructure(PED_DG(ped), PED_LI(ped), PED_INFO(ped),
				cd, adj_list, do_node)) )
    {
   	return_node = pt_rebuild_tree (ped, adj_list, do_node, level); 
   	pt_rebuild_graph (ped, adj_list, level, return_node, do_node);
   	return (return_node);
    }
    return_node = dstr_cd_rebuild_tree( PED_LI(ped), cd, adj_list, do_node, level);
    pt_rebuild_graph (ped, adj_list, level, return_node, do_node);   
    return (return_node);
}
            
      
      
   
