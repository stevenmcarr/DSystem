/* $Id: statement.C,v 1.1 1997/06/25 13:52:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ped_cp/pt/statement.c						*/
/*									*/
/*	Statement transformation routines				*/
/*									*/
/*	Delete Statement						*/
/*	Add Statement							*/
/*									*/
/************************************************************************/

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/pt/private_pt.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>


/*-----------------------------------------------------------------

	Delete statement

*/

/*
 * pt_test_delete_stmt(): check if statement can be deleted.
 *	Restrictions: Only assignment statements in loops may be deleted.
 */
int
pt_test_delete_stmt(PedInfo ped, AST_INDEX loop, AST_INDEX stmt, char **msg)
{

    if (!is_assignment(stmt))
    {
	*msg = ssave("Only assignment statements in loops may be deleted");
	return CANNOT_CHANGE;
    }

    return CAN_CHANGE;
}

/*
 * pt_delete_stmt(): delete specified statement and update the dependence
 * graph. 
 *
 *	- Vas, May 1988.
 */
void
pt_delete_stmt(PedInfo ped, AST_INDEX stmt)
{
    AST_INDEX	lhs;

    /* First consider the LHS var */
    lhs = gen_ASSIGNMENT_get_lvalue(stmt);
    
    walk_lhs(ped, lhs);

    /* delete all remaining edges on all levels that involve references in
       the RHS of the statement */
    walk_rhs(ped, gen_ASSIGNMENT_get_rvalue(stmt));

    /* remove all highlighting from the deleted statement */
    if(PED_PREV_SRC(ped) != AST_NIL){
        ped->TreeWillChange(PED_ED_HANDLE(ped), PED_PREV_SRC(ped));
        ped->TreeChanged(PED_ED_HANDLE(ped), PED_PREV_SRC(ped));
	PED_PREV_SRC(ped) = AST_NIL;
    }

    if (PED_PREV_SINK(ped) != AST_NIL){
        ped->TreeWillChange(PED_ED_HANDLE(ped), PED_PREV_SINK(ped));
        ped->TreeChanged(PED_ED_HANDLE(ped), PED_PREV_SINK(ped));
	PED_PREV_SINK(ped) = AST_NIL;
    }
       
    /* finally delete the statement itself */
    list_remove_node(stmt);
    
    /* Check if any variable in the statement has no other occurances in this loop.
     * If so remove it from either the shared or the private list.
     * 
     * --- Not yet implemented --- Vas.
     */

    return;

}

/*
 * walk_lhs(): walk the lhs expression, updating the dependence edges
 * involving any variables in the expression.
 */
void
walk_lhs(PedInfo ped, AST_INDEX n)
{
    AST_INDEX	list;
    EDGE_INDEX	inedge, outedge, newedge;
    int		ref;
    DepType 	ntype;
    DG_Edge	*Elist, *in, *out, *New;

    if(is_subscript(n)) {
        ref = get_info(ped, gen_SUBSCRIPT_get_name(n), type_levelv);

        /* since this is an array variable, simply delete all incoming and
           outgoing deps on it at all levels (this works because we don't 
           compute array kills yet) */

       if(ref > 0) {
           for(inedge=dg_first_sink_ref( PED_DG(ped), ref); inedge >= 0;
	       inedge=dg_first_sink_ref( PED_DG(ped), ref))
	   { 
  	       Elist = dg_get_edge_structure( PED_DG(ped));
	       in = &Elist[inedge];
	       dg_delete_free_edge( PED_DG(ped), inedge);
           }
	   for(outedge=dg_first_src_ref( PED_DG(ped), ref); outedge >= 0;
	       outedge=dg_first_src_ref( PED_DG(ped), ref))
	   {
  	       Elist = dg_get_edge_structure( PED_DG(ped));
	       out = &Elist[outedge];
	       dg_delete_free_edge( PED_DG(ped), outedge);	      
           }
	}
	list = gen_SUBSCRIPT_get_rvalue_LIST(n);
	for(n = list_first(list); n!=AST_NIL; n=list_next(n))
	    walk_rhs(ped, n); /* a subscript is a USE not a DEF. So
	    			treat it like an rhs expression */
    }
    else if(is_identifier(n)) {
          /* for each incoming dep edge to the LHS scalar, 
      	     check all outgoing dep edges from the LHS scalar */

	  ref = get_info(ped, n, type_levelv);
	  if(ref > 0){
  	     Elist = dg_get_edge_structure( PED_DG(ped));

             for(inedge=dg_first_sink_ref( PED_DG(ped), ref); 
	         inedge>0; inedge=dg_next_sink_ref( PED_DG(ped), inedge)){ 
		 in = &Elist[inedge];
		 /* ignore edges that have src and sink in the same stmt */
		 if(tree_out(in->src) == tree_out(in->sink))
		    continue;

	        for(outedge=dg_first_src_ref( PED_DG(ped), ref); 
		    outedge>0; outedge=dg_next_src_ref( PED_DG(ped), outedge)){
          
		   out = &Elist[outedge];
		   
		   /* only compare edges at the same level */
	           if((in->level != out->level) && 
				((in->level != LOOP_INDEPENDENT) && 
				(out->level != LOOP_INDEPENDENT))) {
		        continue;
		   }
		   /* ignore edges that have src and sink in the same stmt */
		   if(tree_out(out->src) == tree_out(out->sink))
	              continue;

	          /* Add a new edge, depending on which one of the following 
	             cases it corresponds to: */

	          /* case (1) output in, true out => true */  	
     		   if(in->type == dg_output
		      && out->type == dg_true)
		        ntype = dg_true;

	          /* case (2) output in, output out => output */
     		   if(in->type == dg_output
		      && out->type == dg_output)
		        ntype = dg_output;
     
	          /* case (3) anti in, output out => anti */
     		   if(in->type == dg_anti
		      && out->type == dg_output)
		        ntype = dg_anti;

	          /* case (4) anti in, true out => do nothing */
		   if(in->type == dg_anti
		      && out->type == dg_true){
		      /* do nothing */
	   	   }
		   else{

 	             /* add a new edge */

  	             Elist = dg_get_edge_structure( PED_DG(ped));
	             newedge = dg_alloc_edge( PED_DG(ped), &Elist);
		     New = &Elist[newedge];
	             New->src = in->src;
	             New->sink = out->sink;
	             New->type = ntype;
	             New->level = (in->level > out->level)? out->level: in->level;
	             New->src_ref = in->src_ref;
	             New->sink_ref = out->sink_ref;
	             New->src_vec = in->src_vec;
	             New->sink_vec = out->sink_vec;
	             New->ic_preventing = false;
	             New->ic_sensitive = false;
	             New->used = true;
	             New->label = in->label;
	       
	             dg_add_edge( PED_DG(ped), newedge);
    	           }
	       }
             }

	     /* delete all incoming and outgoing dependence edges */
             for(inedge = dg_first_sink_ref( PED_DG(ped), ref); inedge >= 0;
	         inedge = dg_first_sink_ref( PED_DG(ped), ref))
	     { 
		   Elist = dg_get_edge_structure( PED_DG(ped));
	           in = &Elist[inedge];
	           dg_delete_free_edge( PED_DG(ped), inedge);
	     }
             for(outedge = dg_first_src_ref( PED_DG(ped), ref); outedge >= 0;
	         outedge = dg_first_src_ref( PED_DG(ped), ref))
	     { 
		   Elist = dg_get_edge_structure( PED_DG(ped));
	     	   out = &Elist[outedge];
	           dg_delete_free_edge( PED_DG(ped), outedge);
	     }
	}
    }
    else{
       printf("Confused in walk_lhs()\n");
    }
    
    return;

}

/*  walk_rhs(): walk rhs expression, deleting all dependences involving 
 *  variables in the expression.
 */
void
walk_rhs(PedInfo ped, AST_INDEX n)
{
    AST_INDEX	list;
    EDGE_INDEX	inedge, outedge;
    int		ref;

    if(is_identifier(n)) {
       /* delete all dep edges involving this identifier */
       ref = get_info(ped, n, type_levelv);
       if(ref > 0) {
           for(inedge=dg_first_sink_ref( PED_DG(ped), ref); inedge >= 0;
	       inedge=dg_first_sink_ref( PED_DG(ped), ref))
	   { 
	       dg_delete_free_edge( PED_DG(ped), inedge);
           }
	   for(outedge=dg_first_src_ref( PED_DG(ped), ref); outedge >= 0;
	       outedge=dg_first_src_ref( PED_DG(ped), ref))
	   {
	       dg_delete_free_edge( PED_DG(ped), outedge);	      
           }
	}
    }
    else if(is_subscript(n)) {
        walk_rhs(ped, gen_SUBSCRIPT_get_name(n));
	list = gen_SUBSCRIPT_get_rvalue_LIST(n);
	for(n = list_first(list); n!=AST_NIL; n=list_next(n))
	    walk_rhs(ped, n);
    }
    else if(is_unary_minus(n)) {
       walk_rhs(ped, gen_UNARY_MINUS_get_rvalue(n));
    }
    else if(is_binary_exponent(n)) {
       walk_rhs(ped, gen_BINARY_EXPONENT_get_rvalue1(n));
       walk_rhs(ped, gen_BINARY_EXPONENT_get_rvalue2(n));
    }
    else if(is_binary_times(n)) {
       walk_rhs(ped, gen_BINARY_TIMES_get_rvalue1(n));
       walk_rhs(ped, gen_BINARY_TIMES_get_rvalue2(n));
    }
    else if(is_binary_divide(n)) {
       walk_rhs(ped, gen_BINARY_DIVIDE_get_rvalue1(n));
       walk_rhs(ped, gen_BINARY_DIVIDE_get_rvalue2(n));
    }
    else if(is_binary_plus(n)) {
       walk_rhs(ped, gen_BINARY_PLUS_get_rvalue1(n));
       walk_rhs(ped, gen_BINARY_PLUS_get_rvalue2(n));
    }
    else if(is_binary_minus(n)) {
       walk_rhs(ped, gen_BINARY_MINUS_get_rvalue1(n));
       walk_rhs(ped, gen_BINARY_MINUS_get_rvalue2(n));
    }
    else if(is_function(n)) {
       list = gen_FUNCTION_get_formal_arg_LIST(n);
       for(n = list_first(list); n!=AST_NIL; n=list_next(n))
	   walk_rhs(ped, n);
    }
    else return;
    
    return;
}


/*-----------------------------------------------------------------

	Add statement

*/

/*-----------------------------------------------------------------

	pt_add_stmt()

	This stupid version reanalyzes the program completely :-(

*/

void
pt_add_stmt(PedInfo ped, char *stmt)
{
	AST_INDEX selection;
	AST_INDEX node;
	int line1;
	int char1;
	int line2;
	int char2;

	/* insert stmt into tree */
	ftt_InsertTextLine(PED_FTT(ped), pedLine(ped), stmt);
}

#ifdef NEW_CODE

/* 
=====================================================================
    This code is useless until NED's bug with tree rebuilding
    is fixed.  Currently massive AST changes up to the root
    are possible, making it impossible to limit the scope
    of the reanalysis :-(
=====================================================================
*/


/*-----------------------------------------------------------------

	find_scope2()	Gets closest scope, including loops/if stmts

*/

AST_INDEX
find_scope2(AST_INDEX node)
{
	AST_INDEX ret;

	ret = node;

	while (true)
	{
		ret = tree_out(ret);

		if (ret == AST_NIL)
			return find_scope(node);
		else if (is_do(ret) || is_parallelloop(ret) || is_if(ret))
			return ret;
	}
}

/*-----------------------------------------------------------------

*/

pt_add_stmt(PedInfo ped, char *stmt)
{
	AST_INDEX scope;
	AST_INDEX dummy;
	AST_INDEX place_holder;
	AST_INDEX node;
	int line1;
	int char1;
	int line2;
	int char2;
	AST_INDEX selection;

	/* get selection & scope	*/
	selection = PED_SELECTION(ped);
	scope = find_scope2(selection);

	/* get line # of selection	*/
	(void) ftt_NodeToText(PED_FTT(ped), selection, 
		&line1, &char1, &line2, &char2);

	/* save copy of original tree	*/
	dummy = tree_copy(scope);

	/* insert place holder for new statement	*/
	place_holder = gen_PLACE_HOLDER();
	if (in_list(selection))
		list_insert_before(selection, place_holder);
	else
	{
		message("pt_add_stmt(): Not list!");
		return;
	}

	/* replace orig tree with dummy	*/
	tree_replace(scope, dummy);

	/* insert stmt into dummy tree */
	ftt_InsertTextLine(PED_FTT(ped), line1, stmt);

	/* get node in dummy tree	*/
	(void) ftt_TextToNode(PED_FTT(ped), line1, 0, line1, 100, &node);
	list_insert_before(node, place_holder);

	/* put original tree back	*/
	tree_replace(dummy, scope);
	tree_replace(place_holder, node);

	/* insert stmt in front of original selection	*/
	if (is_list(node))
		list_insert_before(place_holder, node);
	else
		list_insert_before(place_holder, list_create(node));
	list_remove_node(place_holder);
}


#endif
