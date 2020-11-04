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
					(WK_STMT_CLBACK)ut_mark_do_post, (Generic)&pre_info);
	if (pre_info->abort)
		return NULL;

	loop_data = (model_loop *)ar->arena_alloc_mem_clear(LOOP_ARENA,
														pre_info->loop_num * sizeof(model_loop));
	/* create loop structure */

	ut_analyze_loop(root, loop_data, level, ped, symtab);
	ut_check_shape(loop_data, 0);

	return loop_data;
}

void advancedVectorization(DependenceGraph *dgraph,int k) {
	dgraph->SCC();
	std::list<AST_INDEX> *sccs = dgraph->getSCCS();	
	std::list<DependenceGraph *> d_pi;
	for (int i = 0; i < sccs->size(); i++) {
		std::list<AST_INDEX> scc = sccs[i];
		if (!scc.empty()) {
			
			for (std::list<AST_INDEX>::iterator it = scc.begin();
				it != scc.end();
				it++)
		}
	}
}

int buildRegion(AST_INDEX stmt,
			   int level,
			   Generic vgraph)
{
	if (!is_loop_stmt(stmt))
		((DependenceGraph*)vgraph)->addNodeToRegion(stmt,level);
	
	return(WALK_CONTINUE);

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

	pre_info_type *pre_info = new pre_info_type();

	if ((loop_data = buildLoopInformation(ped, root, level, symtab, ar, pre_info)) == NULL)
	{
		delete pre_info;
		return;
	}

	delete pre_info;


	DependenceGraph *vecDepGraph = new DependenceGraph(pre_info->stmt_num,ped);
	walk_statements(root, level, (WK_STMT_CLBACK)buildRegion,
					(WK_STMT_CLBACK)NOFUNC, (Generic)vecDepGraph);

	vecDepGraph->buildGraph(1);
	advancedVectorization(vecDepGraph,1);
}
