/* $Id: align.C,v 1.1 1997/06/25 13:52:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	align.c - contains code for the alignment of dependences	*/
/*									*/
/*	Feb 1991 TSM	Created						*/
/*									*/
/************************************************************************/

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/pt/private_pt.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dt.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/align.h>

/*****************************************************/
/* pt_align_degree - gets the length of the selected */
/* dependence and returns it as the desired aligning */
/* degree.  It also checks for cases where alignment */
/* may be undesirable.				     */
/*****************************************************/

align_info
pt_align_degree(PedInfo ped)
  //PedInfo	ped;
{
AST_INDEX	loop, list, source, sink, loop_list, step, control;
EDGE_INDEX	edge;
int		level, current_dep, vector, k, dist, S;
DG_Edge		*dg;
align_info	al_info;
Loop_list *linfo;

/* return zero if nothing is found */
al_info.degree = 0;

/* initialize flags as false */
al_info.danger = false;
al_info.other_dep = false;
al_info.loop_depth = 0;

/* initialize source and sink */
source = AST_NIL;
sink = AST_NIL;

/* get loop and dependence information */
loop = PED_SELECTED_LOOP(ped);
level = loop_level(loop);

/* check depth of loop nest */
/* only alignment of deepest loop in nest allowed for now */

linfo = (Loop_list *) get_info(ped, loop, type_ref);
if ((linfo == (Loop_list *) NO_REF) || (linfo->level != level))
{
  al_info.loop_depth = linfo->level;
  return(al_info);
}

current_dep = PED_CURRENT_DEPENDENCE(ped);
dg = dg_get_edge_structure( PED_DG(ped));
loop_list = gen_DO_get_stmt_LIST(loop);
list = list_first(loop_list);

/* go through the dependences and get the length of the selected
   dependence	*/
while (list != AST_NIL)
	{
	vector = get_info(ped, list, type_levelv);
	for (edge = dg_first_src_stmt( PED_DG(ped),vector,level); edge !=NIL;
				edge = dg_next_src_stmt( PED_DG(ped),edge))
		{

	/* make sure edge is valid */
		if (dg[edge].dt_type != DT_UNKNOWN)
			{
			dist = gen_get_dt_DIS(dg + edge, level);

		/* make sure this is meant to be a distance */
			if (!gen_is_dt_DIR(dist))

			/* return the distance if this is the selected 
			   dependence */
				if (!current_dep--)
					{
					al_info.degree = dist;
					source = dg[edge].src;
					sink = dg[edge].sink;
					}
			}
		}

	list = list_next(list);
	}

/* see if other dependencies share the source or sink of the current dep */
/* i.e. possible dependence conflict */
list = list_first(loop_list);
current_dep = PED_CURRENT_DEPENDENCE(ped);
while (list != AST_NIL)
	{
	vector = get_info(ped, list, type_levelv);
	for (edge = dg_first_src_stmt( PED_DG(ped),vector,level); edge !=NIL;
				edge = dg_next_src_stmt( PED_DG(ped),edge))
		{
		if (dg[edge].dt_type != DT_UNKNOWN)
			{
			if ((current_dep-- != 0)&&((dg[edge].src == source)||
			(dg[edge].sink == sink)||(dg[edge].src == sink)||
			(dg[edge].sink == source)))
				al_info.other_dep = true;
			}
		}
	list = list_next(list);
	}

/* make sure the source occurs before the sink */

/* make sure that both source and sink exist, if not al_info.danger is false */
if ((source != AST_NIL) && (sink != AST_NIL))
	{
	/* get actual source and sink */
	while (tree_out(sink) != loop)
		sink = tree_out(sink);
	while (tree_out(source) != loop)
		source = tree_out(source);

	/* set danger to be true,  if sink is found after source,
		danger will be reset to false again */
	al_info.danger = true;

	/* go through the statements until the source is found */
	list = list_first(loop_list);
	while(list != source)
		list = list_next(list);

	/* go through list after source to find sink, if found then no danger */
	while(list != AST_NIL)
		{
		if (list == sink)
			al_info.danger = false;
		list = list_next(list);
		}
	}

return(al_info);
}


/*****************************************************/
/* pt_align - the function which does the aligning   */
/*						     */
/* inputs:  ped and the alignment degree	     */
/*****************************************************/

void
pt_align(PedInfo ped, char *arg)
  //PedInfo	ped;
  //char	*arg;
{
int		align_int, current_dep, vector, level, sink_index, k, length;
AST_INDEX	loop, list, sink, control, up_bound, low_bound, temp, temp2, step, 
		src_loop, sink_loop, loop_list, src_loop_list, sink_loop_list, 
		src_low_bound, sink_up_bound, src_list, sink_list;
DG_Edge		*dg;
EDGE_INDEX	edge;
char		*var;
Boolean		lin;

align_int = pt_convert_to_int(arg);

if (align_int <= 0)
	return;

/* get loop and dependence information */
loop = PED_SELECTED_LOOP(ped);
current_dep = PED_CURRENT_DEPENDENCE(ped);
dg = dg_get_edge_structure( PED_DG(ped));
loop_list = gen_DO_get_stmt_LIST(loop);
list = list_first(loop_list);
level = loop_level(loop);

/* go through the dependences in search of the sink statement */
while (list != AST_NIL)
	{
	vector = get_info(ped, list, type_levelv);
	for (edge = dg_first_src_stmt( PED_DG(ped),vector,level); edge !=NIL;
				edge = dg_next_src_stmt( PED_DG(ped),edge))
		{

	/* make sure edge is valid */
		if (dg[edge].dt_type != DT_UNKNOWN)

	/* check if this is the desired sink */
		if (!current_dep--)
			sink = dg[edge].sink;
		}

	list = list_next(list);
	}

/* find the expression containing the sink */
while (tree_out(sink) != loop)
	sink = tree_out(sink);

/* get index to the sink statement */
sink_index = list_element(sink);

/* get the loop counter variable */
control = gen_DO_get_control(loop);
var = gen_get_text(gen_INDUCTIVE_get_name(control));

/* get upper bound, lower bound, and step of loop */
low_bound = gen_INDUCTIVE_get_rvalue1(control);
up_bound = gen_INDUCTIVE_get_rvalue2(control);
step = gen_INDUCTIVE_get_rvalue3(control);

/* peel off boundry iterations from the loop */
/* if only aligning by one, then only need to peel one iteration */
/* if aligning by more than one, then need to create loops */
/* creates a src and sink list to be inserted later on */
if (align_int == 1)
{
	/* peel off the non-sink and non-comment statements */
	list = list_first(loop_list);
	src_list = AST_NIL;
	while (list != AST_NIL)
		{
		if ((list != sink)&&(!is_comment(list)))
			{
			src_list = (src_list == AST_NIL) ?
				list_create(tree_copy(list)) :
				list_append(src_list, list_create(tree_copy(list)));
			}
		list = list_next(list);
		}

	pt_clear_info(ped, src_list);

	/* find the value for i in the last iteration */
	if (step == AST_NIL)
		temp = tree_copy(up_bound);
	else
		{
		temp = pt_gen_sub(tree_copy(up_bound), tree_copy(low_bound));
		temp = pt_gen_mod(temp, tree_copy(step));
		temp = pt_gen_sub(tree_copy(up_bound), temp);
		temp = pt_simplify_expr(temp);
		}

	/* replace i with this value and insert*/
	pt_var_replace(src_list, var, temp);

	/* peel off the sink statement */
	temp = list_create(tree_copy(sink));
	pt_clear_info(ped, temp);
	pt_var_replace(temp, var, tree_copy(low_bound));
	sink_list = temp;

}
else
{
	/* make copies of the original loop */
	src_loop = tree_copy(loop);
	sink_loop = tree_copy(loop);
	pt_clear_info(ped, src_loop);
	pt_clear_info(ped, sink_loop);

	/* remove the sink statement and comments from the source loop */
	src_loop_list = gen_DO_get_stmt_LIST(src_loop);
	list_remove(src_loop_list, sink_index);
	list = list_first(src_loop_list);
	while(list != AST_NIL)
		{
		if is_comment(list)
			{
			temp = list_next(list);
			list_remove_node(list);
			list = temp;
			}
		else
			list = list_next(list);
		}

	/* remove all statements but the sink from the sink statement */
	sink_loop_list = gen_DO_get_stmt_LIST(sink_loop);
	length = list_length(sink_loop_list);
	for (k=1; k < sink_index; k++)
		list_remove_first(sink_loop_list);
	for (k = sink_index + 1; k <= length; k++)
		list_remove_last(sink_loop_list);

	/* adjust bounds of the source loop */
	/* should reflect the last align_int iterations of */
	/* the original loop */
	control = gen_DO_get_control(src_loop);
	src_low_bound = gen_INDUCTIVE_get_rvalue1(control);
	if (step == AST_NIL)
		temp = pt_gen_sub(tree_copy(up_bound), pt_gen_int(align_int - 1));
	else
		{
		temp = pt_gen_mul(pt_gen_int(align_int - 1), tree_copy(step));
		temp = pt_gen_sub(tree_copy(up_bound), temp);
		temp2 = pt_gen_sub(tree_copy(up_bound), tree_copy(low_bound));
		temp2 = pt_gen_mod(temp2, tree_copy(step));
		temp = pt_gen_sub(temp, temp2);
		}
	temp = pt_simplify_expr(temp);
	pt_tree_replace(src_low_bound, temp);

	/* adjust bounds of the sink loop */
	/* should reflect the first align_int iterations of */
	/* the original loop */
	control = gen_DO_get_control(sink_loop);
	sink_up_bound = gen_INDUCTIVE_get_rvalue2(control);
	if (step == AST_NIL)
		temp = pt_gen_add(tree_copy(low_bound), pt_gen_int(align_int - 1));
	else
		{
		temp = pt_gen_mul(pt_gen_int(align_int - 1), tree_copy(step));
		temp = pt_gen_add(tree_copy(low_bound), temp);
		}
	temp = pt_simplify_expr(temp);
	pt_tree_replace(sink_up_bound, temp);

	/* make lists of source and sink loops */
	src_list = list_create(src_loop);
	sink_list = list_create(sink_loop);
}

/* add alignment degree to the sink statement */
if (step == AST_NIL)
	temp = pt_gen_int(align_int);
else
	temp = pt_gen_mul(pt_gen_int(align_int), tree_copy(step));
temp = pt_gen_add(pt_gen_ident(var), temp);
temp = pt_simplify_expr(temp);
pt_var_replace(sink, var, temp);

/* adjust the upper bound of the loop */
if (step == AST_NIL)
	temp = pt_gen_int(align_int);
else
	temp = pt_gen_mul(pt_gen_int(align_int), tree_copy(step));
temp = pt_gen_sub(tree_copy(up_bound), temp);
temp = pt_simplify_expr(temp);
pt_tree_replace(up_bound, temp);

/* insert peeled off source and sink lists */
pt_insert_after(loop, src_list);
pt_insert_before(loop, sink_list);

return;
}
