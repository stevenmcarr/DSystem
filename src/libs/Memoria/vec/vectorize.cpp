/* $Id: vectorize.C,v 1.17 1997/06/25 15:24:09 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/****************************************************************/
/*                                                              */
/*   File:        interchange.C                                 */
/*                                                              */
/*   Description: Apply the AdancedVectorization algorithm from */
/*                Allen and Kennedy book to a loop nest         */
/*                                                              */
/****************************************************************/

#include <sstream>

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

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

static PiDependenceGraph *buildDPi(std::list<AST_INDEX> *sccs, int n, int k, PedInfo ped)
{
	PiDependenceGraph *d_pi = new PiDependenceGraph(n, ped);
	for (int i = 0; i < n; i++)
	{
		std::list<AST_INDEX> scc = sccs[i];
		RegionNode *R = new RegionNode();
		for (std::list<AST_INDEX>::iterator it = scc.begin();
			 it != scc.end();
			 it++)
		{
			AST_INDEX stmt = *it;
			R->addStmt(stmt, get_stmt_info_ptr(stmt)->level);
		}
		R->setSCCNum(i);
		d_pi->addRegionNode(R);
	}
	d_pi->buildGraph(k);

	return d_pi;
}

static DependenceGraph *buildIntraRegionGraph(RegionNode *R, PedInfo ped, int k) {

	DependenceGraph *dgraph = new DependenceGraph(R->getNumStmts(),ped);
	std::list<pair<AST_INDEX,int> > *stmts = R->getStmts();
	for (std::list<pair<AST_INDEX,int> >::iterator it = stmts->begin();
	     it != stmts->end();
		 it++)
    {
		dgraph->addNodeToRegion(it->first,it->second);
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
		for (int i = k; i < max; i++)
		{
		    stringstream ss;
			ss << i;
			s->append(ss.str()+", ");
		}

		stringstream ss;
		ss << max;
        s->append(ss.str());

		return s;
	}

}

void advancedVectorization(PedInfo ped, DependenceGraph *dgraph, int k)
{
	dgraph->SCC();
	std::list<AST_INDEX> *sccs = dgraph->getSCCS();

	PiDependenceGraph *d_pi = buildDPi(sccs, dgraph->size(), k, ped);

	std::list<RegionNode*> *regionOrder = d_pi->topSort();

	for (std::list<RegionNode*>::iterator it = regionOrder->begin();
		 it != regionOrder->end();
		 it++)
    {
		RegionNode *R = *it;
		if (dgraph->isSCCCyclic(R->getSCCNum()))
		{
			///generate a level-k DO
			advancedVectorization(ped,buildIntraRegionGraph(R,ped,k+1),k+1);
			//generate a level-k ENDDO
		} else
		{
			std::list< pair<AST_INDEX,int> > *regionStmts = R->getStmts();
			for (std::list< pair<AST_INDEX,int> >::iterator it2 = regionStmts->begin();
				 it2 != regionStmts->end();
				 it2++)
			{
				AST_INDEX stmt = it2->first;
				stringstream ss;
				ss << get_stmt_info_ptr(stmt)->stmt_num ;
				cout << "Statement " << ss.str() <<  " is vectorizable at levels: " <<  
								*getLevelsString(k,get_stmt_info_ptr(stmt)->level);
			}
		}
		
	}

}

int buildRegion(AST_INDEX stmt,
				int level,
				Generic vgraph)
{
	if (!is_loop_stmt(stmt))
		((DependenceGraph *)vgraph)->addNodeToRegion(stmt, level);

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
	advancedVectorization(ped, vecDepGraph, 1);
}
