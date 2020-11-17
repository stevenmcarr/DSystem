/* $Id: vectorize.C,v 1.17 1997/06/25 15:24:09 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/****************************************************************/
/*                                                              */
/*   File:        vectorize.C                                 */
/*                                                              */
/*   Description: Apply the AdancedVectorization algorithm from */
/*                Allen and Kennedy book to a loop nest         */
/*                                                              */
/****************************************************************/

#include <sstream>

#include <libs/support/misc/general.h>
#include <libs/support/strings/rn_string.h>

#include <libs/Memoria/include/mh.h>
#include <libs/Memoria/include/mh_ast.h>
#include <libs/frontEnd/include/walk.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>
#include <libs/Memoria/include/mh_config.h>

#ifndef header_h
#include <libs/Memoria/include/header.h>
#endif

#ifndef dt_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dt.h>
#endif

#ifndef gi_h
#include <libs/frontEnd/include/gi.h>
#endif

#ifndef interchange_h
#include <libs/Memoria/li/interchange.h>
#endif

#ifndef analyze_h
#include <libs/Memoria/include/analyze.h>
#endif

#ifndef shape_h
#include <libs/Memoria/include/shape.h>
#endif

#ifndef mem_util_h
#include <libs/Memoria/include/mem_util.h>
#endif

#ifndef mark_h
#include <libs/Memoria/include/mark.h>
#endif

#ifndef dg_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dg.h>
#endif

#include <libs/Memoria/li/MemoryOrder.h>
#include <libs/frontEnd/ast/cd_branch.h>

#include <libs/Memoria/vec/depGraph.h>
#include <libs/Memoria/vec/piDepGraph.h>
#include <libs/Memoria/vec/vectorize.h>

/****************************************************************************/
/*                                                                          */
/*   Function:   set_scratch                                                */
/*                                                                          */
/*   Input:      node - a node in the AST                                   */
/*               dummy - anything                                           */
/*                                                                          */
/*   Description: This function is called by walk_expression on each AST    */
/*                node.  It sets the scratch field to NULL so no spurrious  */
/*                pointers to space exist.                                  */
/*                                                                          */
/****************************************************************************/

static int set_scratch(AST_INDEX node,
					   Generic dummy)

{
	set_scratch_to_NULL(node);
	return (WALK_CONTINUE);
}

static model_loop *buildLoopInformation(PedInfo ped,
										AST_INDEX root,
										int level,
										SymDescriptor symtab,
										arena_type *ar,
										pre_info_type *pre_info)

{
	model_loop *loop_data;

	pre_info->stmt_num = 0;
	pre_info->loop_num = 0;
	pre_info->surrounding_do = -1;
	pre_info->abort = false;
	pre_info->ped = ped;
	pre_info->symtab = symtab;
	pre_info->ar = ar;

	walk_expression(root, set_scratch, NOFUNC, (Generic)NULL);

	/* prepare nest for analyzing, record surrounding do information */

	walk_statements(root, level, (WK_STMT_CLBACK)ut_mark_do_pre,
					(WK_STMT_CLBACK)ut_mark_do_post, (Generic)pre_info);
	if (pre_info->abort)
		return NULL;

	loop_data = (model_loop *)ar->arena_alloc_mem_clear(LOOP_ARENA,
														pre_info->loop_num * sizeof(model_loop));
	/* create loop structure */

	ut_analyze_loop(root, loop_data, level, ped, symtab);
	ut_check_shape(loop_data, 0);

	return loop_data;
}

static PiDependenceGraph *buildDPi(std::list<SCC*>& sccs, int k, PedInfo ped)
{
	int n = sccs.size();
	PiDependenceGraph *d_pi = new PiDependenceGraph(n, ped);
	int i = 0;
	for (std::list<SCC*>::iterator it = sccs.begin();
		 it != sccs.end();
		 it++)
	{
		SCC *scc = *it;
		RegionNode *R = new RegionNode(scc,k);
		d_pi->addRegionNode(R);
	}
	d_pi->buildGraph(k);

	return d_pi;
}

static DependenceGraph *buildIntraRegionGraph(RegionNode *R, PedInfo ped, int k) {

	DependenceGraph *dgraph = new DependenceGraph(R->getNumStmts(),ped);
	std::list<AST_INDEX>& stmts = R->getStmts();
	for (std::list<AST_INDEX>::iterator it = stmts.begin();
	     it != stmts.end();
		 it++)
    {
		dgraph->addNodeToRegion(*it);
	}
	dgraph->buildGraph(k);

	return dgraph;
}

static string *getLevelsString(int k, int max) {
	if (max < k)
		return new string("none");
	else
	{
		string *s = new string("");
		for (int i = k; i < max-1; i++)
		{
		    stringstream ss;
			ss << i;
			s->append(ss.str()+", ");
		}
		stringstream ss;
		ss << (max-1);
		s->append(ss.str());

		return s;
	}

}

int getLevelKLoopIndex(model_loop *loop_data,int index, int k) {

	if (loop_data[index].level == k)
		return index;
	else 
		getLevelKLoopIndex(loop_data,loop_data[index].parent,k);
}

typedef struct genvecinfo {
	AST_INDEX expr;
	char *index_var;
} GenVecInfo;

static int replace_index(AST_INDEX node,
						 Generic vecinfo)
{
	GenVecInfo *gen_vec_info = (GenVecInfo *)vecinfo;
	if (is_identifier(node) && !strcmp(gen_get_text(node),gen_vec_info->index_var)) 
	{
		pt_tree_replace(node,tree_copy_with_type(gen_vec_info->expr));
		return WALK_FROM_OLD_NEXT;
	}
	return WALK_CONTINUE;
}

static int update_index(AST_INDEX node,
					   	 	  Generic loop_info)
{
	model_loop * loop_data = (model_loop*)loop_info;
	AST_INDEX control = gen_DO_get_control(loop_data->node);
	AST_INDEX lwb = gen_INDUCTIVE_get_rvalue1(control);
	AST_INDEX upb = gen_INDUCTIVE_get_rvalue2(control);
	AST_INDEX step = gen_INDUCTIVE_get_rvalue3(control);

	char *index_var = gen_get_text(gen_INDUCTIVE_get_name(control));

	if (is_identifier(node) && !strcmp(gen_get_text(node),index_var)) 
	{
		if (!is_subscript(tree_out(node))) 
		{
			GenVecInfo vecinfo;

			vecinfo.index_var = index_var;
			vecinfo.expr = lwb;
			AST_INDEX triplet_lwb = tree_copy_with_type(tree_out(node));
			walk_expression(triplet_lwb,NOFUNC,replace_index,(Generic)&vecinfo);
			triplet_lwb = pt_simplify_expr(triplet_lwb);

			vecinfo.expr = upb;
			AST_INDEX triplet_upb = tree_copy_with_type(tree_out(node));
			walk_expression(triplet_upb,NOFUNC,replace_index,(Generic)&vecinfo);
			triplet_upb = pt_simplify_expr(triplet_upb);

			pt_tree_replace(tree_out(node),gen_TRIPLET(triplet_lwb,triplet_upb,tree_copy_with_type(step)));

		    return WALK_ABORT;
		} else 
			pt_tree_replace(node,gen_TRIPLET(tree_copy_with_type(lwb),tree_copy_with_type(upb),
											 tree_copy_with_type(step)));
        
		return WALK_ABORT;
	}

	return WALK_CONTINUE;
	
}

static int find_subscript(AST_INDEX node,
						  Generic loop_info)
{
	if (is_subscript(node)) {
		AST_INDEX next_subscript = AST_NIL;
		for (AST_INDEX subscript = list_first(gen_SUBSCRIPT_get_rvalue_LIST(node));
			 subscript != AST_NIL;
			 subscript = next_subscript) 
		{
			next_subscript = list_next(subscript);
			walk_expression(subscript,NOFUNC,update_index,loop_info);
		}
	}
	return WALK_CONTINUE;
}

AST_INDEX makeVectorStatement(AST_INDEX stmt, model_loop *loop_data, int k, int max) {
	
    AST_INDEX new_stmt = tree_copy_with_type(stmt);
	for (int i = k; i < max; i++)
	{
		int index = getLevelKLoopIndex(loop_data,get_stmt_info_ptr(stmt)->surrounding_do,i);
		walk_expression(new_stmt, NOFUNC, find_subscript, (Generic)&loop_data[index]);
	}
	return new_stmt;
}

AST_INDEX advancedVectorization(model_loop *loop_data,PedInfo ped, DependenceGraph *dgraph, int k)
{
	AST_INDEX levelKStmtList = gen_LIST_OF_NODES();
	dgraph->computeSCC();
	std::list<SCC*>& sccs = dgraph->getSCCS();

	PiDependenceGraph *d_pi = buildDPi(sccs, k, ped);

	std::list<RegionNode*> *regionOrder = d_pi->topSort();

	for (std::list<RegionNode*>::iterator it = regionOrder->begin();
		 it != regionOrder->end();
		 it++)
    {
		RegionNode *R = *it;
		if (dgraph->isRegionCyclic(*R))
		{
			if (R->getNumStmts() > 0) 
			{
				///generate a level-k DO
				AST_INDEX first_stmt = *R->getStmts().begin();
				AST_INDEX loop_stmt = loop_data[getLevelKLoopIndex(loop_data,get_stmt_info_ptr(first_stmt)->surrounding_do,k)].node;
				AST_INDEX levelKDo = gen_DO(tree_copy_with_type(gen_DO_get_lbl_def(loop_stmt)),
											tree_copy_with_type(gen_DO_get_close_lbl_def(loop_stmt)),
											tree_copy_with_type(gen_DO_get_lbl_ref(loop_stmt)),
											tree_copy_with_type(gen_DO_get_control(loop_stmt)),
											AST_NIL);
				AST_INDEX loop_stmt_list = gen_LIST_OF_NODES();
				
				while(R->getNumStmts() > 0) {
					std::list<AST_INDEX> *stmt_group = R->getStatementGroup(k+1);

					first_stmt = *stmt_group->begin();

					if (get_stmt_info_ptr(first_stmt)->level == k+1)
						for (std::list<AST_INDEX>::iterator it2 = stmt_group->begin();
							 it2 != stmt_group->end();
							 it2++)
							loop_stmt_list = list_insert_last(loop_stmt_list,*it2);
					else 
					{
						RegionNode *R2 = new RegionNode();
						R2->addStmts(*stmt_group);
						DependenceGraph *dgraph2 = buildIntraRegionGraph(R2,ped,k+1);
						loop_stmt_list = list_append(loop_stmt_list,advancedVectorization(loop_data,ped,dgraph2,k+1));
					}
				}
				gen_DO_put_stmt_LIST(levelKDo,loop_stmt_list);

				//generate a level-k ENDDO

				levelKStmtList = list_insert_last(levelKStmtList,levelKDo);
			}
		} else
		{
			std::list<AST_INDEX>& regionStmts = R->getStmts();
			for (std::list<AST_INDEX>::iterator it2 = regionStmts.begin();
				 it2 != regionStmts.end();
				 it2++)
			{
				AST_INDEX stmt = *it2;
				stringstream ss;
				ss << get_stmt_info_ptr(stmt)->stmt_num ;
				levelKStmtList = list_insert_last(levelKStmtList,
												  makeVectorStatement(stmt,loop_data,k,get_stmt_info_ptr(stmt)->level));
			}
		}
		
	}

	return levelKStmtList;

}

int buildRegion(AST_INDEX stmt,
				int level,
				Generic vgraph)
{
	if (!is_loop_stmt(stmt))
		((DependenceGraph *)vgraph)->addNodeToRegion(stmt);

	return (WALK_CONTINUE);
}
/****************************************************************/
/*                                                              */
/*   Function:     memory_advanced_vectorization                */
/*                                                              */
/*   Input:        ped - dependence graph handle                */
/*                 root - AST index of an outermost loop        */
/*                 level - nesting level of root                */
/*                 symtab - symbol table                        */
/*                 ar - arena for memory allocation             */
/*                                                              */
/*   Description:  Order a loop nest based upon memory          */
/*                 performance                                  */
/*                                                              */
/****************************************************************/

void memory_advanced_vectorization(PedInfo ped,
								   AST_INDEX root,
								   int level,
								   SymDescriptor symtab,
								   arena_type *ar)

{
	model_loop *loop_data;

	pre_info_type pre_info;
	pre_info.stmt_num = 0;
    pre_info.loop_num = 0;
    pre_info.surrounding_do = 0;
	pre_info.surround_node = AST_NIL;
    pre_info.abort = false;
    pre_info.ped = ped;
    pre_info.symtab = symtab;
    pre_info.ar = ar;


	if ((loop_data = buildLoopInformation(ped, root, level, symtab, ar, &pre_info)) == NULL)
		return;


	DependenceGraph *vecDepGraph = new DependenceGraph(pre_info.stmt_num, ped);
	walk_statements(root, level, (WK_STMT_CLBACK)buildRegion,
					(WK_STMT_CLBACK)NOFUNC, (Generic)vecDepGraph);

	vecDepGraph->buildGraph(1);

	AST_INDEX vecStmts = advancedVectorization(loop_data, ped, vecDepGraph, 1);

	AST_INDEX oldLoop = loop_data[0].node;
	for (AST_INDEX stmt = list_first(vecStmts);
		 stmt != AST_NIL;
		 stmt = list_next(stmt))
		list_insert_before(oldLoop,tree_copy_with_type(stmt));
	list_remove_node(oldLoop);
}
