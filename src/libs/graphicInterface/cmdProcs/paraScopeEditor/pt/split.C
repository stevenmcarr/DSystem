/* $Id: split.C,v 1.1 1997/06/25 13:52:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*********************************************************************/
/*                                                                   */
/* split.c.c - Code to do loop splitting                                 */
/*                                                                   */
/*                                                                   */
/*********************************************************************/

#include <stdlib.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/pt/private_pt.h>


/*
 * pt_split(ped,loop,index_value)
 *  Splits the current loop into two loops.  The first loop runs from
 *  the low bound to index_value and the second runs from the next 
 *  iteration after index_value to the upper bound.
 *
 *  The dependence graph is modified to reflect the addition of the 
 *  second loop.  The loop info nodes must be updated in dp.c after
 *  returning from pt_split for BOTH loops.
 *
 * Inputs:ped - The dependence abstraction
 *        loop- the loop
 *        step - the step size of the loop
 *
 * Outputs: none
 */
void
pt_split(PedInfo ped, AST_INDEX loop, char *index_value)
{
AST_INDEX slist;			/* statement list*/
AST_INDEX control;			/* loop control */
AST_INDEX up_bound;			/* upper bound */
AST_INDEX low_bound;			/* low bound */
AST_INDEX orig_step_size;		/* original ast for step_size */
AST_INDEX step_size;			/* ast for step_size in guards */

AST_INDEX newloop;			/* new loop */
AST_INDEX newlow, newup;		/* new bounds */
AST_INDEX zero, one, arg2,args;	/* part of new upper bound expression */ 
AST_INDEX index_ast; 
AST_INDEX temp, temp2; 			/*  min (index_value + inductvar - 1, up_bound) */ 
AST_INDEX newivar;			/* new induction variable node */ 
AST_INDEX newvarnode;			/* new variable node */
AST_INDEX newcontrol;			/* new control */
AST_INDEX cont,cont_ast;		/* continue */
AST_INDEX ifstmt;
char *newub;

AST_INDEX induct_ast;
char *inductvar;			/* induction variable */
char *cont_lbl;	
char *newcont;				/* new induction variable */
int  len;				/* length of inductvar */
int  level;

DG_Edge   *edgeptr;

Boolean	contnue = false;		/* if the DO is matched with a labeled 
					 * continue, rather than a ENDDO
					 */
int *iptr, value,i,istp;   		/* ptr to simplifed result & simplified result   */
char *char_ptr, char_value[10];
Boolean smplfd = false;			/* assume step_size does not simplify */
Boolean node_sign = true;		/* used by simplify */
Pt_ref_params *old_refs, *new_refs;	/* pointer to all refs in tree */
int int_index_value;


	int_index_value = atoi(index_value);
	iptr = &value;
	char_ptr = &char_value[0];
	node_sign = true;

	/* get the needed loop information */
	slist     = gen_DO_get_stmt_LIST(loop);
	control   = gen_DO_get_control(loop);
	induct_ast = gen_INDUCTIVE_get_name(control);
	inductvar = gen_get_text(induct_ast);
	low_bound  = gen_INDUCTIVE_get_rvalue1(control);
	up_bound  = gen_INDUCTIVE_get_rvalue2(control);
	orig_step_size  = gen_INDUCTIVE_get_rvalue3(control);
	step_size  = orig_step_size;
	newloop = tree_copy(loop);

	one = gen_CONSTANT();
	gen_put_text(one,"1",STR_CONSTANT_INTEGER);
	zero = gen_CONSTANT();
	gen_put_text(zero,"0",STR_CONSTANT_INTEGER);
	
	if (step_size == AST_NIL)
	   {
	   step_size = tree_copy(one);
	   smplfd = true;	
	   istp = 1;
	   }
	else if (NOT(pt_eval(step_size,iptr))) 
	   {
	   istp = value;
	   smplfd = true;	
	   }
	value = 0;
	node_sign = true;
		

	index_ast = gen_CONSTANT();
	gen_put_text(index_ast,index_value,STR_CONSTANT_INTEGER);
	arg2 = list_create(tree_copy(index_ast));
	args = list_append(arg2,list_create(tree_copy(up_bound)));
	if (smplfd && (istp > 0))
	   { /* make upper bound = min (index_value, up_bound) */
	   if (NOT(pt_eval(up_bound,iptr)))
	      {
	      sprintf(char_ptr,"%d",*iptr);
	      newup = gen_CONSTANT();
	      if (value < int_index_value)
		 gen_put_text(newup,char_ptr,STR_CONSTANT_INTEGER);
	      else
		 gen_put_text(newup,index_value,STR_CONSTANT_INTEGER);
	      }
	   else
	      {
	      newup = gen_IDENTIFIER();
	      gen_put_text(newup, "min", STR_IDENTIFIER);
	      newup = gen_INVOCATION(newup,args);
	      }
	   newcontrol = gen_INDUCTIVE(tree_copy(induct_ast),tree_copy(low_bound),newup,tree_copy(orig_step_size));
	   }
	else if (smplfd && (istp < 0))
	   { /* make upper bound = max (index_value, up_bound) */
	   if (NOT(pt_eval(up_bound,iptr)))
	      {
	      sprintf(char_ptr,"%d",*iptr);
	      newup = gen_CONSTANT();
	      if (value > int_index_value)
		 gen_put_text(newup,char_ptr,STR_CONSTANT_INTEGER);
	      else
		 gen_put_text(newup,index_value,STR_CONSTANT_INTEGER);
	      }
	   else
	      {
	      newup = gen_IDENTIFIER();
	      gen_put_text(newup, "max", STR_IDENTIFIER);
	      newup = gen_INVOCATION(newup,args);
	      }
	   newcontrol = gen_INDUCTIVE(tree_copy(induct_ast),tree_copy(low_bound),newup,tree_copy(orig_step_size));
	   }
	else
	   {     /* the step size is not constant or is zero */
	   /* create the new variable for the upper bound of newloop
	    * FUTURE: insure unique new induction variable name
	    */
	   len = strlen(inductvar);
	   newub = (char *) get_mem (sizeof(char *) * (len + 3), "pt_split");
	   (void) strcpy(newub, inductvar);
	   newub[len]    = '$';
	   newub[len +1] = 'h';
	   newub[len +2] = '\0';
	   newvarnode        = gen_IDENTIFIER();
	   gen_put_text (newvarnode, newub, STR_IDENTIFIER);
	   if (NOT(pt_eval(up_bound,iptr)))
	      {
	      sprintf(char_ptr,"%d",*iptr);
	      newup = gen_CONSTANT();
	      if (value < int_index_value)
		 gen_put_text(newup,char_ptr,STR_CONSTANT_INTEGER);
	      else
		 gen_put_text(newup,index_value,STR_CONSTANT_INTEGER);
	      }
	   else
	      {
	      newup = gen_IDENTIFIER();
	      gen_put_text(newup, "min", STR_IDENTIFIER);
	      newup = gen_INVOCATION(tree_copy(newup),tree_copy(args));
	      }
	   newup = gen_ASSIGNMENT(AST_NIL,tree_copy(newvarnode),newup);
	   ifstmt = gen_BINARY_GT(tree_copy(step_size),tree_copy(zero));
	   ifstmt = gen_LOGICAL_IF(AST_NIL,ifstmt,tree_copy(newup));
	   list_insert_before(loop,ifstmt);
	   value = 0;
	   node_sign = true;
	   if (NOT(pt_eval(up_bound,iptr)))
	      {
	      sprintf(char_ptr,"%d",*iptr);
	      newup = gen_CONSTANT();
	      if (value > int_index_value)
		 gen_put_text(newup,char_ptr,STR_CONSTANT_INTEGER);
	      else
		 gen_put_text(newup,index_value,STR_CONSTANT_INTEGER);
	      }
	   else
	      {
	      newup = gen_IDENTIFIER();
	      gen_put_text(newup, "max", STR_IDENTIFIER);
	      newup = gen_INVOCATION(tree_copy(newup),tree_copy(args));
	      }
	   newup = gen_ASSIGNMENT(AST_NIL,tree_copy(newvarnode),newup);
	   ifstmt = gen_BINARY_LT(tree_copy(step_size),tree_copy(zero));
	   ifstmt = gen_LOGICAL_IF(AST_NIL,ifstmt,tree_copy(newup));
	   list_insert_before(loop,ifstmt);
	   newcontrol = gen_INDUCTIVE(tree_copy(induct_ast),tree_copy(low_bound),newvarnode,tree_copy(orig_step_size));
	   }
	tree_replace(control,AST_NIL);
	gen_DO_put_control(loop,newcontrol);

	if (gen_DO_get_lbl_ref(loop) != AST_NIL) contnue = true;

	list_insert_after(loop,newloop);

	/* set up the induction variable for newloop */
	newivar = gen_INDUCTIVE_get_name(gen_DO_get_control(newloop));

	/* compute the lower bound for the new loop */
	temp = gen_BINARY_MINUS(tree_copy(index_ast),tree_copy(low_bound));
	arg2 = list_create(temp);
	args = list_append(arg2,list_create(tree_copy(step_size)));
	value = 0;
	node_sign = true;
	if (smplfd && NOT(pt_eval(temp,iptr)))
	   {
	   value = value % istp;
	   sprintf(char_ptr,"%d",*iptr);
	   temp = gen_CONSTANT();
	   gen_put_text(temp,char_ptr,STR_CONSTANT_INTEGER);
	   }
	else
	   {
	   temp = gen_IDENTIFIER();
	   gen_put_text(temp, "mod", STR_IDENTIFIER);
	   temp = gen_INVOCATION(temp,args);
	   }
	newlow = gen_BINARY_PLUS(tree_copy(index_ast),tree_copy(step_size));
	newlow = gen_BINARY_MINUS(newlow,temp);
	temp = pt_simplify_node(newlow);
	if (temp != newlow)
		{
		tree_free(newlow);
		newlow = temp;
		}

	temp = tree_copy(up_bound);
	temp2 = tree_copy(orig_step_size);
	newcontrol= gen_INDUCTIVE(tree_copy(newivar),newlow,temp,temp2);
	tree_replace(gen_DO_get_control(newloop),AST_NIL);
	gen_DO_put_control(newloop,newcontrol);


	
	/* correct the statement lists for loops with labeled continues */
	if (contnue)
	{
		slist = gen_DO_get_stmt_LIST(newloop);
		cont = list_last(slist);
		cont_ast = gen_CONTINUE_get_lbl_def(cont);
		cont_lbl = gen_get_text(cont_ast);
		len = strlen(cont_lbl);
		newcont = (char *) get_mem (sizeof(char *) * (len + 2), "pt_split");
		(void) strcpy(newcont, cont_lbl);
		newcont[len]    = '9';
		newcont[len +1] = '\0';
		gen_put_text(cont_ast,newcont,STR_CONSTANT_INTEGER);
		cont_ast = gen_DO_get_lbl_def(newloop);
		gen_put_text(cont_ast,newcont,STR_CONSTANT_INTEGER);
		cont_ast = gen_DO_get_lbl_ref(newloop);
		gen_put_text(cont_ast,newcont,STR_CONSTANT_INTEGER);
	}
	
	/*update the meta_type */
	ast_put_meta_type(newloop,ast_get_meta_type(loop));
	
	/* copy the loop information and insert it */
	/* realign the loops */
	level = loop_level(loop);
	el_add_loop( PED_LI(ped), loop, newloop, level);

   

    	/* get pointers to all refs in old and new trees */
    	old_refs = pt_refs(loop, ped);
    	new_refs = pt_refs(newloop, ped);
	
	/* copy the dependences and change the ast indices */
        edgeptr = dg_get_edge_structure( PED_DG(ped));
	pt_copy_dep(ped, edgeptr, loop, level, old_refs, new_refs);

	/* correct the shared and private variable lists */
	el_copy_shared_list( PED_LI(ped),loop,newloop);
	el_copy_private_list( PED_LI(ped),loop,newloop);
}

/*=========================================================================
 * map_ast_ref(old_ast,old_refs,new_refs)
 * Inputs: old_ast - index of ast that must be mapped
 *         old_refs - all old variable references in edge
 *         new_refs - all new variable references in edge
 * Outputs: none
 *=========================================================================
 */
static AST_INDEX
map_ast_ref(AST_INDEX old_ast, Pt_ref_params *old_refs, Pt_ref_params *new_refs)
{
	int i;

	if (old_ast == AST_NIL) return AST_NIL;
	for (i = 0; i < old_refs->defs_num; i++)
	    if (old_refs->defs[i] == old_ast)
		return new_refs->defs[i];
	for (i = 0; i < old_refs->uses_num; i++)
	    if (old_refs->uses[i] == old_ast)
		return new_refs->uses[i];
	for (i = 0; i < old_refs->iv_num; i++)
	    if (old_refs->iv[i] == old_ast)
		return new_refs->iv[i];
	for (i = 0; i < old_refs->subls_num; i++)
	    if (old_refs->subls[i] == old_ast)
		return new_refs->subls[i];
	for (i = 0; i < old_refs->stmts_num; i++)
	    if (old_refs->stmts[i] == old_ast)
		return new_refs->stmts[i];

	printf("AST_INDEX: %d not found in map (map_ast_ref in split.c)\n",old_ast);
	tree_print(old_ast);
	printf("\n");
	return old_ast;  
}



/* =========================================================================
 *
 * pt_copy_dep(ped,edgeptr,loop,level,old_refs,new_refs)
 *
 * Copies the dependence subgraph for "loop" and maps it to create
 * the new dependence subgraph for the new loop represented in
 * the mapping from "old_refs" to "new_refs".
 *
 * Inputs: ped		- dependence abstraction
 *         edgeptr	- pointer to the edge structure
 *	   loop		- loop which to copy dependences from
 *	   level	- level of the loop (MUST be 1)
 * 	   old_refs	- map of old ast indicies
 * 	   new_refs	- map of new ast indicies
 * Outputs: none
 * =========================================================================
 */
void
pt_copy_dep(PedInfo ped, DG_Edge *edgeptr, AST_INDEX loop, int level,
            Pt_ref_params *old_refs, Pt_ref_params *new_refs)
/* old_refs: pointer to all refs in tree */
/* new_refs: pointer to all refs in tree */
{

AST_INDEX  node;
AST_INDEX  src_stmt, sink_stmt;
EDGE_INDEX edge;
EDGE_INDEX newedge;
int	   vector, lvector_size;
int	   src_lvector, sink_lvector;
int	   src_ref, sink_ref;
int	   max;
int	   l;

    for (node = list_first(gen_DO_get_stmt_LIST(loop));
         node!= AST_NIL; 
	 node = list_next(node))
    {
	if (is_do(node))
	   pt_copy_dep(ped,edgeptr,node,level,old_refs,new_refs);
	else
	   {
	   vector  = get_info(ped, node, type_levelv);
	   lvector_size = dg_length_level_vector( PED_DG(ped),vector);
	   max     = dg_length_level_vector( PED_DG(ped), vector);
	   for (l = max; l >= level; l--)
	       {
	   	for(edge = dg_first_src_stmt( PED_DG(ped),vector,l);
	   	    edge != NIL;
	   	    edge = dg_next_src_stmt( PED_DG(ped),edge))
	   	   {
 		   newedge = dg_alloc_edge( PED_DG(ped), &edgeptr);
		   el_copy_edge( PED_DG(ped), PED_DT_INFO(ped), edgeptr, edge, newedge);
		   src_stmt = map_ast_ref(edgeptr[edge].src,old_refs,new_refs);
		   edgeptr[newedge].src = src_stmt;

		   /* does this variable have a reference list different from original? if not allocate one */
		   src_ref = get_info(ped, src_stmt, type_levelv);
		   if (src_ref == edgeptr[edge].src_ref)
		   {
			src_ref = dg_alloc_ref_list( PED_DG(ped));
			put_info(ped, src_stmt, type_levelv, src_ref);
		   }
		   edgeptr[newedge].src_ref = src_ref;

		   while (!is_statement(src_stmt)) src_stmt = tree_out(src_stmt);
		   /* does this stmt have a level vector different from original? if not allocate one */
		   src_lvector = get_info(ped, src_stmt, type_levelv);
		   if (src_lvector == edgeptr[edge].src_vec)
		   {
			src_lvector = dg_alloc_level_vector( PED_DG(ped), lvector_size);
			put_info(ped, src_stmt, type_levelv, src_lvector);
		   }
		   edgeptr[newedge].src_vec = src_lvector;

		   sink_stmt = map_ast_ref(edgeptr[edge].sink,old_refs,new_refs);
		   edgeptr[newedge].sink = sink_stmt;

		   /* does this variable have a reference list different from original? if not allocate one */
		   sink_ref = get_info(ped, sink_stmt, type_levelv);
		   if (sink_ref == edgeptr[edge].sink_ref)
		   {
			sink_ref = dg_alloc_ref_list( PED_DG(ped));
			put_info(ped, sink_stmt, type_levelv, sink_ref);
		   }
		   edgeptr[newedge].sink_ref = sink_ref;

		   while (!is_statement(sink_stmt)) sink_stmt = tree_out(sink_stmt);
		   /* does this stmt have a level vector different from original? if not allocate one */
		   sink_lvector = get_info(ped, sink_stmt, type_levelv);
		   if (sink_lvector == edgeptr[edge].sink_vec)
		   {
			sink_lvector = dg_alloc_level_vector( PED_DG(ped), lvector_size);
			put_info(ped, sink_stmt, type_levelv, sink_lvector);
		   }
		   edgeptr[newedge].sink_vec = sink_lvector;


		   dg_add_edge( PED_DG(ped), newedge);
		   }
		}
	   }
      }
}



