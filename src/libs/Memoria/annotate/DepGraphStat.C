/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
#include <libs/support/misc/general.h>
#include <libs/Memoria/include/mh.h>
#include <libs/Memoria/include/mh_ast.h>
#include <libs/Memoria/include/mh_config.h>
#include <libs/Memoria/include/Memoria_label.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>
#include <libs/Memoria/include/bound.h>
#include <libs/frontEnd/include/walk.h>
#include <libs/Memoria/include/header.h>
#include <libs/Memoria/include/mark.h>
#include <libs/Memoria/include/analyze.h>
#include <libs/Memoria/include/mem_util.h>
#include <libs/Memoria/annotate/CacheAnalysis.h>
#include <libs/Memoria/annotate/DirectivesInclude.h>
 
static int RefCount = 0;

static int remove_edges(AST_INDEX      stmt,
			int            level,
			PedInfo        ped)

  {
   DG_Edge    *dg;
   int        vector,lvl;
   EDGE_INDEX edge,
              next_edge;
   stmt_info_type *sptr;

     dg = dg_get_edge_structure( PED_DG(ped));
     vector = get_info(ped,stmt,type_levelv);
     for (lvl = 1; lvl < level;lvl++)
       {
	for (edge = dg_first_src_stmt( PED_DG(ped),vector,lvl);
	     edge != END_OF_LIST;
	     edge = next_edge)
	  {
	   next_edge = dg_next_src_stmt( PED_DG(ped),edge);
	   if (dg[edge].type == dg_exit || dg[edge].type == dg_io ||
	       dg[edge].type == dg_call || dg[edge].type == dg_control)
	     dg_delete_free_edge( PED_DG(ped),edge);
	  }
	for (edge = dg_first_sink_stmt( PED_DG(ped),vector,lvl);
	     edge != END_OF_LIST;
	     edge = next_edge)
	  {
	   next_edge = dg_next_sink_stmt( PED_DG(ped),edge);
	   if (dg[edge].type == dg_exit || dg[edge].type == dg_io ||
	       dg[edge].type == dg_call || dg[edge].type == dg_control)
	     dg_delete_free_edge( PED_DG(ped),edge);
	  }
       }
     for (edge = dg_first_src_stmt( PED_DG(ped),vector,
				   LOOP_INDEPENDENT);
	  edge != END_OF_LIST;
	  edge = next_edge)
       {
	next_edge = dg_next_src_stmt( PED_DG(ped),edge);
	if (dg[edge].type == dg_exit || dg[edge].type == dg_io ||
	    dg[edge].type == dg_call || dg[edge].type == dg_control)
	  dg_delete_free_edge( PED_DG(ped),edge);
       }
     for (edge = dg_first_sink_stmt( PED_DG(ped),vector,
				    LOOP_INDEPENDENT);
	  edge != END_OF_LIST;
	  edge = next_edge)
       {
	next_edge = dg_next_sink_stmt( PED_DG(ped),edge);
	if (dg[edge].type == dg_exit || dg[edge].type == dg_io ||
	    dg[edge].type == dg_call || dg[edge].type == dg_control)
	  dg_delete_free_edge( PED_DG(ped),edge);
       }
     return(WALK_CONTINUE);
  }

static int GetDependenceStats(AST_INDEX node,DependenceInfoType *DependenceInfo)

  {
   DG_Edge    *dg;
   int        vector;
   EDGE_INDEX edge;
   AST_INDEX  name;
   int        InnerLevel;

     if (is_subscript(node))
       {
	name = gen_SUBSCRIPT_get_name(node);
	dg = dg_get_edge_structure( PED_DG(DependenceInfo->ped));
	vector = get_info(DependenceInfo->ped,name,type_levelv);
	for (edge = dg_first_src_ref(PED_DG(DependenceInfo->ped),vector);
	     edge != END_OF_LIST;
	     edge = dg_next_src_ref(PED_DG(DependenceInfo->ped),edge))

	    switch (dg[edge].type) 
	      {
	       case dg_true:
		 DependenceInfo->LoopStats->NumberOfTrueDependences++;
		 break;

	       case dg_anti:
		 DependenceInfo->LoopStats->NumberOfAntiDependences++;
		 break;

	       case dg_output:
		 DependenceInfo->LoopStats->NumberOfOutputDependences++;
		 break;

	       case dg_input:
		 DependenceInfo->LoopStats->NumberOfInputDependences++;
		 break;

	       default:
		 break;
	      }

       }
     return(WALK_CONTINUE);
  }


static void walk_loops(DependenceInfoType  *DependenceInfo,
		       int            loop)
		       

  {
   int i;

     if (DependenceInfo->loop_data[loop].inner_loop == -1)
       walk_expression(DependenceInfo->loop_data[loop].node,
		       (WK_EXPR_CLBACK)GetDependenceStats,
		       NOFUNC,(Generic)DependenceInfo);
     else
       {
	i = DependenceInfo->loop_data[loop].inner_loop;
	while(i != -1)
	  {
	   walk_loops(DependenceInfo,i);
	   i = DependenceInfo->loop_data[i].next_loop;
	  }
       }
  }


void memory_GetDependenceStats(PedInfo      ped,
			       AST_INDEX    root,
			       LoopStatsType *LoopStats,
			       SymDescriptor symtab,
			       arena_type   *ar,
			       int          level)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   pre_info_type       pre_info;
   DependenceInfoType  DependenceInfo;

     pre_info.stmt_num = 0;
     pre_info.loop_num = 0;
     pre_info.surrounding_do = -1;
     pre_info.surround_node = AST_NIL;
     pre_info.abort = false;
     pre_info.ped = ped;
     pre_info.symtab = symtab;
     pre_info.ar = ar;
     walk_statements(root,level,(WK_STMT_CLBACK)ut_mark_do_pre,
		     (WK_STMT_CLBACK)ut_mark_do_post,(Generic)&pre_info);
     walk_statements(root,level,(WK_STMT_CLBACK)remove_edges,NOFUNC,(Generic)ped);
     DependenceInfo.loop_data = (model_loop *)ar->arena_alloc_mem_clear
                  (LOOP_ARENA,pre_info.loop_num*sizeof(model_loop)*4);
     ut_analyze_loop(root,DependenceInfo.loop_data,level,ped,symtab);

     DependenceInfo.ped = ped;
     DependenceInfo.symtab = symtab;
     DependenceInfo.ar = ar;
     DependenceInfo.LoopStats = LoopStats;
     walk_loops(&DependenceInfo,0);
  }
