/* $Id: walk_dg.C,v 1.1 1997/06/25 15:06:11 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	File:	dep/dg/walk_dg.c					*/
/*	Author:	Kathryn McKinley					*/
/*									*/
/*									*/
/************************************************************************/

#include <libs/moduleAnalysis/dependence/dependenceGraph/dep_dg.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>

#include <libs/moduleAnalysis/dependence/dependenceGraph/private_dg.h>

/***********************************************************/
/*  To walk the level vectors and the expression pointers. */
/***********************************************************/

EDGE_INDEX dg_first_src_stmt(DG_Instance* dg, int src_vec, int level)
{
	level += 1;
	
	if (src_vec < 0)
		return -1;
	if (!dg->vmd[src_vec].used)
		return -1;
	if (dg->vmd[src_vec].size < level)
		return -1;

	return (dg->vmd[src_vec + level].src.next);
}

EDGE_INDEX dg_first_sink_stmt(DG_Instance* dg, int sink_vec, int level)
{
	level += 1;

	if (sink_vec < 0)
		return -1;
	if (!dg->vmd[sink_vec].used)
		return -1;
	if (dg->vmd[sink_vec].size < level)
		return -1;

	return (dg->vmd[sink_vec + level].sink.next);
}

EDGE_INDEX dg_first_src_ref(DG_Instance* dg, int src_ref)
{
	if (src_ref < 0)
		return -1;
	if (!dg->ref[src_ref].used)
		return -1;

	return (dg->ref[src_ref].src.next);

}

EDGE_INDEX dg_first_sink_ref(DG_Instance* dg, int sink_ref)
{
	if (sink_ref < 0)
		return -1;
	if (!dg->ref[sink_ref].used)
		return -1;

	return (dg->ref[sink_ref].sink.next);
}

EDGE_INDEX dg_next_src_stmt(DG_Instance* dg, int edge)
{
	if (edge < 0)
		return -1;
	if (!dg->edgeptr[edge].used)
		return -1;

	return (dg->vec_list[edge].src.next);
}

EDGE_INDEX dg_next_src_ref(DG_Instance* dg, int edge)
{
	if (edge < 0)
		return -1;
	if (!dg->edgeptr[edge].used)
		return -1;

	return (dg->ref_list[edge].src.next);
}

EDGE_INDEX dg_next_sink_stmt(DG_Instance* dg, int edge)
{
	if (edge < 0)
		return -1;
	if (!dg->edgeptr[edge].used)
		return -1;

	return (dg->vec_list[edge].sink.next);
}

EDGE_INDEX dg_next_sink_ref(DG_Instance* dg, int edge)
{
	if (edge < 0)
		return -1;
	if (!dg->edgeptr[edge].used)
		return -1;

	return (dg->ref_list[edge].sink.next);
}

EDGE_INDEX dg_prev_src_stmt(DG_Instance* dg, int edge)
{
	if (edge < 0)
		return -1;
	if (!dg->edgeptr[edge].used)
		return -1;

	return (dg->vec_list[edge].src.prev);
}

EDGE_INDEX dg_prev_src_ref(DG_Instance* dg, int edge)
{
	if (edge < 0)
		return -1;
	if (!dg->edgeptr[edge].used)
		return -1;

	return (dg->ref_list[edge].src.prev);
}

EDGE_INDEX dg_prev_sink_stmt(DG_Instance* dg, int edge)
{
	if (edge < 0)
		return -1;
	if (!dg->edgeptr[edge].used)
		return -1;

	return (dg->vec_list[edge].sink.prev);
}

EDGE_INDEX dg_prev_sink_ref(DG_Instance* dg, int edge)
{
	if (edge < 0)
		return -1;
	if (!dg->edgeptr[edge].used)
		return -1;

	return (dg->ref_list[edge].sink.prev);
}


int
dg_length_level_vector(DG_Instance* dg, int vec)
{
	return (dg->vmd[vec].size - 2);
}
