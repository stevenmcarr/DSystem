/* $Id: mh_walk.C,v 1.29 1995/01/19 13:36:36 carr Exp $ */
/****************************************************************************/
/*                                                                          */
/*    File:  mh_walk.C                                                      */
/*                                                                          */
/*    Description:  Walk the AST to find all outermost loops.  At each      */
/*                  loop nest, perform the requested transformations        */
/*                                                                          */
/****************************************************************************/

#include <general.h>

#include <stdlib.h>
#include <memory.h>

#include <mh.h>
#include <mh_ast.h>
#include <fort/walk.h>
#include <fort/FortTextTree.h>
#include <pt_util.h>
#include <mem_util.h>
#include <assert.h>

#ifndef memory_menu_h
#include <memory_menu.h>
#endif

#ifndef mh_walk_h
#include <mh_walk.h>
#endif

#ifndef gi_h
#include <fort/gi.h>
#endif

#ifndef header_h
#include <header.h>
#endif

#ifndef mh_config_h
#include <mh_config.h>
#endif

#ifndef message_h
#include <dialogs/message.h>
#endif

#include <fort/FortTextTree.h>
#include <FDgraph.h>
#include <PedExtern.h>

extern char *mc_program;
extern char *mc_module_list;

static LoopStatsType *LoopStats = NULL;

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
		       int       dummy)

  {
   set_scratch_to_NULL(node);
   return(WALK_CONTINUE);
  }

static void remove_bogus_dependences(AST_INDEX stmt,
				    PedInfo  ped)

  {
   int vector;
   EDGE_INDEX edge,next_edge;
   DG_Edge *dg;
	
     dg = dg_get_edge_structure( PED_DG(ped));
     vector = get_info(ped,stmt,type_levelv);
     for (edge = dg_first_src_stmt( PED_DG(ped),vector,LOOP_INDEPENDENT);
	  edge != END_OF_LIST;
	  edge = next_edge)
       {
	next_edge = dg_next_src_stmt( PED_DG(ped),edge);
	if (((((dg[edge].type == dg_true || dg[edge].type == dg_output) &&
	       (ut_get_stmt(dg[edge].src) == ut_get_stmt(dg[edge].sink))) ||
	      (dg[edge].src == dg[edge].sink)) &&
	     dg[edge].level == LOOP_INDEPENDENT) ||
	    (dg[edge].src == AST_NIL || dg[edge].sink == AST_NIL))
          dg_delete_free_edge(PED_DG(ped),edge);
       }
     for (edge = dg_first_sink_stmt( PED_DG(ped),vector,LOOP_INDEPENDENT);
	  edge != END_OF_LIST;
	  edge = next_edge)
       {
	next_edge = dg_next_sink_stmt( PED_DG(ped),edge);
	if (((((dg[edge].type == dg_true || dg[edge].type == dg_output) &&
	       (ut_get_stmt(dg[edge].src) == ut_get_stmt(dg[edge].sink))) ||
	      (dg[edge].src == dg[edge].sink)) &&
	     dg[edge].level == LOOP_INDEPENDENT) ||
	    (dg[edge].src == AST_NIL || dg[edge].sink == AST_NIL))
          dg_delete_free_edge(PED_DG(ped),edge);
       }
  }


/****************************************************************************/
/*                                                                          */
/*   Function:   remove_do_labels                                           */
/*                                                                          */
/*   Input:      stmt - a statement in the AST                              */
/*               level - the nesting level of stmt                          */
/*               dummy - anything                                           */
/*                                                                          */
/*   Description: This function removes references to labels to labels in   */
/*                DO statements so that all are in a DO - ENDDO format.     */
/*                This function is called by walk_statements.               */
/*                                                                          */
/****************************************************************************/

static int remove_do_labels(AST_INDEX stmt,
			    int       level,
			    int       dummy)

  {
   if (is_do(stmt))
     gen_DO_put_lbl_ref(stmt,AST_NIL);
   return(WALK_CONTINUE);
  }


/****************************************************************************/
/*                                                                          */
/*   Function:  pre_walk                                                    */
/*                                                                          */
/*   Input:     stmt - statement in the AST                                 */
/*              level - nesting level of stmt                               */
/*              walk_info - structure to hold information such as the       */
/*                          symbol table.                                   */
/*                                                                          */
/*   Description:  This function gets the symbol table for the function     */
/*                 being processed.  It is called by walk_statements in     */
/*                 preorder.                                                */
/*                                                                          */
/****************************************************************************/

static int pre_walk(AST_INDEX      stmt,
		    int            level,
		    walk_info_type *walk_info)

  {
   AST_INDEX node1;
   char      TextConstant[80];

     if (is_program(stmt))
       {
	walk_expression(stmt,set_scratch,NOFUNC,(Generic)NULL);
	walk_info->routine = gen_get_text(gen_PROGRAM_get_name(stmt));
	walk_info->symtab = ft_SymGetTable(walk_info->ft,walk_info->routine);
	walk_info->MainProgram = true;
	fst_InitField(walk_info->symtab,NEW_VAR,false,0);
       }
     else if (is_subroutine(stmt))
       {
	walk_expression(stmt,set_scratch,NOFUNC,(Generic)NULL);
	walk_info->routine = gen_get_text(gen_SUBROUTINE_get_name(stmt));
	walk_info->symtab = ft_SymGetTable(walk_info->ft,walk_info->routine);
	walk_info->MainProgram = false;
	fst_InitField(walk_info->symtab,NEW_VAR,false,0);
       }
     else if (is_function(stmt))
       {
	walk_expression(stmt,set_scratch,NOFUNC,(Generic)NULL);
	walk_info->routine = gen_get_text(gen_FUNCTION_get_name(stmt));
	walk_info->symtab = ft_SymGetTable(walk_info->ft,walk_info->routine);
	walk_info->MainProgram = false;
	fst_InitField(walk_info->symtab,NEW_VAR,false,0);
       }
     else if ((is_stop(stmt) || is_return(stmt)) && 
	      walk_info->selection == ANNOTATE)
       if (mc_program != NULL || mc_module_list != NULL)
         if (walk_info->MainProgram)
	   list_insert_before(stmt,pt_gen_call("cache_report",AST_NIL));
	 else;  
       else
	 list_insert_before(stmt,pt_gen_call("cache_report",AST_NIL));
     if (walk_info->selection != ANNOTATE)
       remove_bogus_dependences(stmt,walk_info->ped);
     return(WALK_CONTINUE);
  }

/****************************************************************************/
/*                                                                          */
/*   Function:   InterchangeStats                                           */
/*                                                                          */
/*   Input:      stmt - a DO-loop stmt                                      */
/*               level - nesting level of stmt                              */
/*               walk_info - structure to hold passed information           */
/*                                                                          */
/*   Description:  Calls the function to gather statistics about loop       */
/*                 interchange on a given loop.                             */
/*                                                                          */
/****************************************************************************/

static void InterchangeStats(AST_INDEX      stmt,
			     int            level,
			     walk_info_type *walk_info)
  {
      /* gather statistics */
   memory_interchange_stats(walk_info->ped,stmt,
			    LEVEL1,
			    walk_info->LoopStats,
			    walk_info->routine,
			    walk_info->program,
			    walk_info->symtab,
			    walk_info->ar);

     /* re-initialize scratch field (no dangling pointers) */
   walk_expression(stmt,set_scratch,NOFUNC,
		   (Generic)NULL);

     /* free up space used */
   walk_info->ar->arena_deallocate(LOOP_ARENA);
   if (NOT(walk_info->LoopStats->Perfect))
     walk_info->LoopStats->Imperfect++;
  }


/****************************************************************************/
/*                                                                          */
/*   Function:   Interchange                                                */
/*                                                                          */
/*   Input:      stmt - a DO-loop stmt                                      */
/*               level - nesting level of stmt                              */
/*               walk_info - structure to hold passed information           */
/*                                                                          */
/*   Description: Call function to perform loop interchange for memory      */
/*                performance.                                              */
/*                                                                          */
/****************************************************************************/

static void Interchange(AST_INDEX      stmt,
			int            level,
			walk_info_type *walk_info,
			Boolean        Fusion)
  {
      /* perform loop interchange */
   memory_loop_interchange(walk_info->ped,stmt,
			   LEVEL1,walk_info->symtab,
			   walk_info->ar,Fusion);

     /* re-initialize scratch field (no dangling pointers) */
   walk_expression(stmt,set_scratch,NOFUNC,
		   (Generic)NULL);

     /* free up space used */
   walk_info->ar->arena_deallocate(LOOP_ARENA);
  }


/****************************************************************************/
/*                                                                          */
/*   Function:   ScalarReplacement                                          */
/*                                                                          */
/*   Input:      stmt - a DO-loop stmt                                      */
/*               level - nesting level of stmt                              */
/*               walk_info - structure to hold passed information           */
/*                                                                          */
/*   Description: Call function to perform scalar replacement on a loop     */
/*                nest.                                                     */
/*                                                                          */
/****************************************************************************/

static void ScalarReplacement(AST_INDEX      stmt,
			      int            level,
			      walk_info_type *walk_info)
  {
     /* perform scalar replacement */
   memory_scalar_replacement(walk_info->ped,stmt,level,walk_info->symtab,
			     walk_info->ar,walk_info->LoopStats);

     /* re-initialize scratch field (no dangling pointers) */
   walk_expression(stmt,set_scratch,NOFUNC,

		   (Generic)NULL);
     /* free up space used */
   walk_info->ar->arena_deallocate(LOOP_ARENA);
  }


static void ScalarStats(AST_INDEX      stmt,
			int            level,
			walk_info_type *walk_info)
  {
     /* perform scalar replacement */
   memory_scalar_replacement(walk_info->ped,stmt,level,walk_info->symtab,
			     walk_info->ar,walk_info->LoopStats);


     /* re-initialize scratch field (no dangling pointers) */
   walk_expression(stmt,set_scratch,NOFUNC,

		   (Generic)NULL);
     /* free up space used */
   walk_info->ar->arena_deallocate(LOOP_ARENA);

  }

/****************************************************************************/
/*                                                                          */
/*   Function:   UnrollAndJam                                               */
/*                                                                          */
/*   Input:      stmt - a DO-loop stmt                                      */
/*               level - nesting level of stmt                              */
/*               walk_info - structure to hold passed information           */
/*                                                                          */
/*   Output:     the AST_INDEX of the outermost loop after unroll-and-jam.  */
/*               It may be an index to a pre-loop.                          */
/*                                                                          */
/*   Description: Call function to perform unroll-and-jam on a loop         */
/*                nest.                                                     */
/*                                                                          */
/****************************************************************************/

static AST_INDEX UnrollAndJam(AST_INDEX      stmt,
			      int            level,
			      walk_info_type *walk_info)
  {
   AST_INDEX new_stmt;

           /* symbol table field for scalar expansion */
     fst_InitField(walk_info->symtab,EXPAND_LVL,0,0);

           /* perform unroll-and-jam */
     new_stmt = memory_unroll_and_jam(walk_info->ped,stmt,level,2,
				      walk_info->symtab,walk_info->ar,
				      walk_info->LoopStats);

           /* cleanup space */
     walk_info->ar->arena_deallocate(LOOP_ARENA);
     fst_KillField(walk_info->symtab,EXPAND_LVL);
     return(new_stmt);
  }



static void UnrollStats(AST_INDEX      stmt,
			int            level,
			walk_info_type *walk_info)
  {
   AST_INDEX new_stmt;

           /* symbol table field for scalar expansion */
     fst_InitField(walk_info->symtab,EXPAND_LVL,0,0);

           /* get unroll-and-jam statistics */
     ((config_type *)PED_MH_CONFIG(walk_info->ped))->logging = 1;
     new_stmt = memory_unroll_and_jam(walk_info->ped,stmt,level,2,
				      walk_info->symtab,walk_info->ar,
				      walk_info->LoopStats);

           /* cleanup space */
     walk_info->ar->arena_deallocate(LOOP_ARENA);
     fst_KillField(walk_info->symtab,EXPAND_LVL);
  }



/****************************************************************************/
/*                                                                          */
/*   Function:     SoftwarePrefetch                                         */
/*                                                                          */
/*   Input:      stmt - a DO-loop stmt                                      */
/*               level - nesting level of stmt                              */
/*               walk_info - structure to hold passed information           */
/*                                                                          */
/*   Description:  Call the function to perform software prefetching.       */
/*                                                                          */
/****************************************************************************/


static void SoftwarePrefetch(AST_INDEX      stmt,
			     int            level,
			     walk_info_type *walk_info)
  {
      /* perform loop interchange */
   memory_software_prefetch(walk_info->ped,stmt,
			    LEVEL1,walk_info->symtab,
			    walk_info->ar);

     /* re-initialize scratch field (no dangling pointers) */
   walk_expression(stmt,set_scratch,NOFUNC,
		   (Generic)NULL);

     /* free up space used */
   walk_info->ar->arena_deallocate(LOOP_ARENA);
  }



/****************************************************************************/
/*                                                                          */
/*   Function:     AnnotateCodeForCache                                     */
/*                                                                          */
/*   Input:      stmt - a DO-loop stmt                                      */
/*               level - nesting level of stmt                              */
/*               walk_info - structure to hold passed information           */
/*                                                                          */
/*   Description:  Call function to annotate the fortran code with calls to */
/*                 the cache simulator                                      */
/*                                                                          */
/****************************************************************************/


static void AnnotateCodeForCache(AST_INDEX      stmt,
				 int            level,
				 walk_info_type *walk_info)
  {
   AST_INDEX node1,node2,ExecutableStmt;
   char TextConstant[80];

     if (walk_info->LoopStats->Nests == 1 && mc_program == NULL &&
	 mc_module_list == NULL)
       {

	  /* initialize the simulator before the first loop nest */

	ExecutableStmt = first_f77_executable_stmt(ut_GetSubprogramStmtList(stmt));
	list_insert_before(ExecutableStmt,pt_gen_call("cache_init",AST_NIL));
       }
     else if ((mc_program != NULL || mc_module_list != NULL) &&
	      walk_info->MainProgram)
       {
	ExecutableStmt = first_f77_executable_stmt(gen_PROGRAM_get_stmt_LIST(stmt));
	list_insert_before(ExecutableStmt,pt_gen_call("cache_init",AST_NIL));
       }	   
     memory_AnnotateWithCacheCalls(stmt,level, walk_info->routine,
				   walk_info->ftt);
  }


static void PerformCacheAnalysis(AST_INDEX stmt,
				 int level,
				 walk_info_type *walk_info)

  {
   memory_PerformCacheAnalysis(walk_info->ped,walk_info->symtab,walk_info->ar,
			       stmt,level);
  }


/****************************************************************************/
/*                                                                          */
/*   Function:   post_walk                                                  */
/*                                                                          */
/*   Input:      stmt - a statement in the AST                              */
/*               level - nesting level of stmt                              */
/*               walk_info - structure to hold passed information           */
/*                                                                          */
/*   Description: On each do loop found at the outermost nesting level      */
/*                perform the memory optimizations requested.  This         */
/*                function is called by walk_statements                     */
/*                                                                          */
/****************************************************************************/

static int post_walk(AST_INDEX      stmt,
		     int            level,
		     walk_info_type *walk_info)

  {
   AST_INDEX new_stmt;

   if (is_do(stmt) && level == LEVEL1)
     {
      walk_info->LoopStats->Nests++;
      walk_info->LoopStats->Perfect = true;
      switch(walk_info->selection) {
	case LI_STATS:       InterchangeStats(stmt,level,walk_info);
	                     break;
	case INTERCHANGE:    Interchange(stmt,level,walk_info,false);
	                     break;
	case SCALAR_REP:     ScalarReplacement(stmt,level,walk_info);
	                     break;
	case SR_STATS:       ScalarStats(stmt,level,walk_info);
	                     break;
	case UJ_STATS:       UnrollStats(stmt,level,walk_info);
	                     break;
	case UNROLL_AND_JAM: (void)UnrollAndJam(stmt,level,walk_info);
	                     break;
	case UNROLL_SCALAR:  new_stmt = UnrollAndJam(stmt,level,walk_info);

			        /* perform scalar replacement on each of the
				   loop bodies created by unroll-and-jam */

                             while (new_stmt != stmt)
			       {
				if (is_do(new_stmt))
				  {
				   ScalarReplacement(new_stmt,level,walk_info);
				   walk_info->ar->arena_deallocate(LOOP_ARENA);
				  }
				new_stmt = list_next(new_stmt);
			       }
			     ScalarReplacement(stmt,level,walk_info);
	                     break;
	case LI_SCALAR:      Interchange(stmt,level,walk_info,false);
			     ScalarReplacement(stmt,level,walk_info);
	                     break;
	case LI_UNROLL:      Interchange(stmt,level,walk_info,false);
			     (void)UnrollAndJam(stmt,level,walk_info);
	                     break;
	case LI_FUSION:      Interchange(stmt,level,walk_info,true);
	                     break;
	case MEM_ALL:        Interchange(stmt,level,walk_info,true);
			     new_stmt = UnrollAndJam(stmt,level,walk_info);

			        /* perform scalar replacement on each of the
				   loop bodies created by unroll-and-jam */

                             while (new_stmt != stmt)
			       {
				if (is_do(new_stmt))
				  {
				   ScalarReplacement(new_stmt,level,walk_info);
				   walk_info->ar->arena_deallocate(LOOP_ARENA);
				  }
				new_stmt = list_next(new_stmt);
			       }
			     ScalarReplacement(stmt,level,walk_info);
	                     break;
	case PREFETCH:       SoftwarePrefetch(stmt,level,walk_info);
	                     break;
	case ANNOTATE:       if (mc_program == NULL && mc_module_list == NULL)
	                       AnnotateCodeForCache(stmt,level,walk_info);
	                     break;
	case CACHE_ANALYSIS: PerformCacheAnalysis(stmt,level,walk_info);
			     break;
	case FUSION:         break;
       }
      return(WALK_FROM_OLD_NEXT);
     } 
   else if ((is_program(stmt) || is_function(stmt) || is_subroutine(stmt)) && 
	    (mc_program != NULL || mc_module_list != NULL) && 
	    walk_info->selection == ANNOTATE)
     AnnotateCodeForCache(stmt,level,walk_info);
   return(WALK_CONTINUE);
  }


static int TryFusionToCleanup(AST_INDEX stmt,
			      int       level,
			      PedInfo   ped)

  {
   FDGraph *problem;
   Boolean   all, any, free;
   int       depth;
   AST_INDEX scope;

     if (is_do(stmt) && level == LEVEL1)
       {
    
 
	problem = fdBuildFusion(ped, stmt, false, FD_ALL);
	free    = false;
	if (problem == NULL)
	  return(WALK_CONTINUE);

	fdGreedyFusion(ped, problem, false, &all, &any);
   
	for (depth = problem->depth; depth >= 1 ;  )
	  {     
	   /* Do the fusion */ 
	   if (any)
	     {
	      scope = find_scope(stmt);

	      fdDoFusion(ped, problem);
	      
	      pedReinitialize(ped);

	      fprintf(stderr, "Fusion Performed to Improve Cache Performance\n");
	      fprintf(stderr, "Fusion Candidates = %d\n", problem->size);
	      fprintf(stderr, "Fusion Applied = %d\n", problem->types);
	      if (all)
		depth--;
	     }
	   else
	     depth--;

	   fdDestroyProblem(problem);
	   free = true;

	   /* try again at this depth (if all = false) because fusion may 
	    * change reuse between loops, if no fusions, go shallower,
	    * 
	    */
	   for ( ; depth >= 1; depth--)
	     {
	      problem = fdBuildFusion(ped, stmt, false, depth);
	      free    = false;
	      if (problem != NULL)
		break;
	     }
	   if ((problem == NULL) || (depth < 1))
	     break;

	   else 
	     fdGreedyFusion(ped, problem, false, &all, &any);
	  }
	if ((!free) && problem != NULL)
	  fdDestroyProblem(problem);
	return(WALK_CONTINUE);
       }
     return(WALK_CONTINUE);
  }

 
/****************************************************************************/
/*                                                                          */
/*   Function:   build_label_symtab                                         */
/*                                                                          */
/*   Input:      stmt - a statement in the AST                              */
/*               level - nesting level of stmt                              */
/*               walk_info - structure to hold passed information           */
/*                                                                          */
/*   Description: Mark any label that is referenced in the symbol table     */
/*                Any label that is not referenced will be removed.         */
/*                                                                          */
/****************************************************************************/

static int build_label_symtab(AST_INDEX   stmt,
                              int         level,
                              walk_info_type *walk_info)

  {
   AST_INDEX label_ref;

     if (is_program(stmt))
       {
        walk_info->symtab = ft_SymGetTable(walk_info->ft,
				     gen_get_text(gen_PROGRAM_get_name(stmt)));
	fst_InitField(walk_info->symtab,REFD,false,0);
       }  
     else if (is_subroutine(stmt))
       {
	walk_info->symtab = ft_SymGetTable(walk_info->ft,
				  gen_get_text(gen_SUBROUTINE_get_name(stmt)));
	fst_InitField(walk_info->symtab,REFD,false,0);
       }
     else if (is_function(stmt))
       {
	walk_info->symtab = ft_SymGetTable(walk_info->ft,
				  gen_get_text(gen_FUNCTION_get_name(stmt)));
	fst_InitField(walk_info->symtab,REFD,false,0);
       }
     else if (is_goto(stmt))
       fst_PutField(walk_info->symtab,(int)gen_get_text(gen_GOTO_get_lbl_ref(stmt)),
		    REFD,(int)true);
     else if (is_arithmetic_if(stmt))
       {
	fst_PutField(walk_info->symtab,
		     (Generic)gen_get_text(gen_ARITHMETIC_IF_get_lbl_ref1(stmt)),REFD,
		     (int)true);
	fst_PutField(walk_info->symtab,
		     (Generic)gen_get_text(gen_ARITHMETIC_IF_get_lbl_ref2(stmt)),REFD,
		     (int)true);
	fst_PutField(walk_info->symtab,
		     (Generic)gen_get_text(gen_ARITHMETIC_IF_get_lbl_ref3(stmt)),REFD,
		     (int)true);
       }
     else if (is_computed_goto(stmt))
       for (label_ref = list_first(gen_COMPUTED_GOTO_get_lbl_ref_LIST(stmt));
	    label_ref != AST_NIL;
	    label_ref = list_next(label_ref))
         fst_PutField(walk_info->symtab,(Generic)gen_get_text(label_ref),REFD,(int)true);
     return(WALK_CONTINUE);
  }


/****************************************************************************/
/*                                                                          */
/*   Function:   remove_bogus_labels                                        */
/*                                                                          */
/*                                                                          */
/*   Input:      stmt - a statement in the AST                              */
/*               level - nesting level of stmt                              */
/*               symtab - symbol table                                      */
/*                                                                          */
/*   Description: Remove any label that is not referenced.  If the label    */
/*                is for a continue statement, remove the statement also.   */
/*                                                                          */
/****************************************************************************/

static int remove_bogus_labels(AST_INDEX     stmt,
                               int           level,
                               SymDescriptor symtab)

  {
   AST_INDEX label;

     if (labelled_stmt(stmt) && !is_format(stmt))
       if ((label = gen_get_label(stmt)) != AST_NIL)
         if (!fst_GetField(symtab,gen_get_text(label),REFD))
	   if (is_continue(stmt))
	     {
	      (void)list_remove_node(stmt);
	      return(WALK_FROM_OLD_NEXT);
	     }
	   else
            pt_tree_replace(label,AST_NIL);
     return(WALK_CONTINUE);
  }


/****************************************************************************/
/*                                                                          */
/*   Function:   check_labels                                               */
/*                                                                          */
/*   Input:      stmt - a statement in the AST                              */
/*               level - nesting level of stmt                              */
/*               walk_info - structure to hold passed information           */
/*                                                                          */
/*                                                                          */
/*   Description: Call label removing function for each routine.            */
/*                                                                          */
/****************************************************************************/

static int check_labels(AST_INDEX   stmt,
                        int         level,
                        walk_info_type *walk_info)

  {
   if (is_program(stmt))
     {
      walk_statements(gen_PROGRAM_get_stmt_LIST(stmt),level,
		      (WK_STMT_CLBACK)remove_bogus_labels,NOFUNC,
		      (Generic)walk_info->symtab);
      fst_KillField(walk_info->symtab,REFD);
     }
   else if (is_subroutine(stmt))
     {
      walk_statements(gen_SUBROUTINE_get_stmt_LIST(stmt),level,
		      (WK_STMT_CLBACK)remove_bogus_labels,NOFUNC,
		      (Generic)walk_info->symtab);
      fst_KillField(walk_info->symtab,REFD);
     }
   else if (is_function(stmt))
     {
      walk_statements(gen_FUNCTION_get_stmt_LIST(stmt),level,
		      (WK_STMT_CLBACK)remove_bogus_labels,NOFUNC,
		      (Generic)walk_info->symtab);
      fst_KillField(walk_info->symtab,REFD);
     }
   return(WALK_CONTINUE);
  }


/****************************************************************************/
/*                                                                          */
/*   Function:   check_decls                                                */
/*                                                                          */
/*   Input:      symtab - symbol table                                      */
/*               index  - symbol table index                                */
/*               decl_lists - lists of new variables to be declared         */
/*                                                                          */
/*   Description:  Searches ast for any varibles created by memory          */
/*                 optimizations and creates a declaration for those        */
/*                 variables                                                */
/*                                                                          */
/****************************************************************************/

static void check_decl(SymDescriptor  symtab,
                       fst_index_t    index,
                       decl_list_type *decl_lists)
  
  {
   if (fst_GetFieldByIndex(symtab,index,NEW_VAR))
     {
      switch(fst_GetFieldByIndex(symtab,index,SYMTAB_TYPE)) {
	case TYPE_DOUBLE_PRECISION:
	  list_insert_last(decl_lists->dbl_prec_list,gen_ARRAY_DECL_LEN(
                           pt_gen_ident((char *)fst_GetFieldByIndex(symtab,
								    index,
							    SYMTAB_NAME)),
		           AST_NIL,AST_NIL,AST_NIL));
	  break;
	case TYPE_REAL:
	  list_insert_last(decl_lists->real_list,gen_ARRAY_DECL_LEN(
                           pt_gen_ident((char*)fst_GetFieldByIndex(symtab,
								   index,
							    SYMTAB_NAME)),
		           AST_NIL,AST_NIL,AST_NIL));
	  break;
	case TYPE_COMPLEX:
	  list_insert_last(decl_lists->cmplx_list,gen_ARRAY_DECL_LEN(
                           pt_gen_ident((char*)fst_GetFieldByIndex(symtab,
								   index,
							    SYMTAB_NAME)),
			   AST_NIL,AST_NIL,AST_NIL));
	  break;
	default:;
       }
     }
  }


/****************************************************************************/
/*                                                                          */
/*   Function:  make_decls                                                  */
/*                                                                          */
/*   Input:     symtab - symbol table                                       */
/*              stmt_list - list of statements in a routine                 */
/*                                                                          */
/*   Description:  Create declaration statements for generated variables    */
/*                 and insert them at the top of the routine.               */
/*                                                                          */
/****************************************************************************/

static void make_decls(SymDescriptor symtab,
                       AST_INDEX     stmt_list)

  {
   decl_list_type decl_lists;
   AST_INDEX      type_stmt;
   AST_INDEX      stmt;
   
     decl_lists.dbl_prec_list = list_create(AST_NIL);
     decl_lists.real_list = list_create(AST_NIL);
     decl_lists.cmplx_list = list_create(AST_NIL);
     fst_ForAll(symtab,(fst_ForAllCallback)check_decl,(Generic)&decl_lists);
     for (stmt = list_first(stmt_list);
          !is_executable_stmt(stmt);
	  stmt = list_next(stmt));
     if (!list_empty(decl_lists.dbl_prec_list))
       {
	type_stmt = gen_TYPE_STATEMENT(AST_NIL,gen_TYPE_LEN(gen_REAL(),
                                       pt_gen_int(SIZE_PER_DB_PREC)),
				       decl_lists.dbl_prec_list);
	ft_SetComma(type_stmt,false);
	list_insert_before(stmt,type_stmt);
       }
     else
       tree_free(decl_lists.dbl_prec_list);
     if (!list_empty(decl_lists.real_list))
       {
	type_stmt = gen_TYPE_STATEMENT(AST_NIL,gen_TYPE_LEN(gen_REAL(),
                                       pt_gen_int(SIZE_PER_REAL)),
				       decl_lists.real_list);
	ft_SetComma(type_stmt,false);
	list_insert_before(stmt,type_stmt);
       }
     else
       tree_free(decl_lists.real_list);
     if (!list_empty(decl_lists.cmplx_list))
       {
	type_stmt = gen_TYPE_STATEMENT(AST_NIL,gen_TYPE_LEN(gen_COMPLEX(),
					       pt_gen_int(SIZE_PER_COMPLEX)),
				       decl_lists.cmplx_list);
	ft_SetComma(type_stmt,false);
	list_insert_before(stmt,type_stmt);
       }
     else
       tree_free(decl_lists.cmplx_list);
				       
  }


/****************************************************************************/
/*                                                                          */
/*   Function:   get_symtab_for_decls                                       */
/*                                                                          */
/*   Input:      stmt - statement in the AST                                */
/*               level - nesting level of stmt                              */
/*               ft - fort tree for getting symbol table                    */
/*                                                                          */
/*   Description: Search ast for routine declaration to get symbol table.   */
/*                Call declaration making routine.                          */
/*                                                                          */
/****************************************************************************/

static int get_symtab_for_decls(AST_INDEX     stmt,
                                int           level,
                                FortTree      ft)

  {
   SymDescriptor symtab = NULL;

     if (is_program(stmt))
       {
	symtab = ft_SymGetTable(ft,gen_get_text(gen_PROGRAM_get_name(stmt)));
	make_decls(symtab,gen_PROGRAM_get_stmt_LIST(stmt));
	fst_KillField(symtab,NEW_VAR);
	return(WALK_FROM_OLD_NEXT);
       }
     else if (is_subroutine(stmt))
       {
	symtab=ft_SymGetTable(ft,gen_get_text(gen_SUBROUTINE_get_name(stmt)));
	make_decls(symtab,gen_SUBROUTINE_get_stmt_LIST(stmt));
	fst_KillField(symtab,NEW_VAR);
	return(WALK_FROM_OLD_NEXT);
       }
     else if (is_function(stmt))
       {
	symtab = ft_SymGetTable(ft,gen_get_text(gen_FUNCTION_get_name(stmt)));
	make_decls(symtab,gen_FUNCTION_get_stmt_LIST(stmt));
	fst_KillField(symtab,NEW_VAR);
	return(WALK_FROM_OLD_NEXT);
       }
     else
       return(WALK_CONTINUE);
  }


/****************************************************************************/
/*                                                                          */
/*   Function:   print_RefGroups                                            */
/*                                                                          */
/*   Input:      logfile - file to dump statistics                          */
/*               title - title for statistics                               */
/*                                                                          */
/*   Description: print statistics for RefGroups.                           */
/*                                                                          */
/****************************************************************************/

static void print_RefGroups(FILE *logfile, char *title,
	int s1, int s2, int s3, int m1, int m2, int m3,
	int g1, int g2, int g3, int os)
{
	int r1, r2, r3, r4;
	double a1, a2, a3, a4;

	r1 = s1+m1;
	r2 = s2+m2;
	r3 = s3+m3;
    r4 = r1+r2+r3;

	a1 = (r1 != 0) ? double(s1+g1)/r1 : double(0);
	a2 = (r2 != 0) ? double(s2+g2)/r2 : double(0);
	a3 = (r3 != 0) ? double(s3+g3)/r3 : double(0);
    a4 = (r4 != 0) ? double(s1+g1+s2+g2+s3+g3)/r4 : double(0);

	fprintf(logfile,title);
	fprintf(logfile,"=======================\n\n");
	fprintf(logfile,"\t\t\tInvariant   Spatial    None    Total\n");
	fprintf(logfile,"\t\t\t=========   =======    ====    =====\n");
	fprintf(logfile,"\tTotal RG   \t%6d    %6d    %6d    %6d\n",
		 r1,r2,r3,r4);
	fprintf(logfile,"\tAvg Size   \t%6.2f    %6.2f    %6.2f    %6.2f\n\n",
		 a1,a2,a3,a4);
	fprintf(logfile,"\tSingle RG  \t%6d    %6d    %6d    %6d\n",
		 s1,s2,s3,s1+s2+s3);
	fprintf(logfile,"\tMulti RG   \t%6d    %6d    %6d    %6d\n",
		 m1,m2,m3,m1+m2+m3);
	fprintf(logfile,"\tRefs in RG \t%6d    %6d    %6d    %6d\n",
		 g1,g2,g3,g1+g2+g3);
	fprintf(logfile,"\tOther Spatial:   %d\n\n\n",os);
}


/****************************************************************************/
/*                                                                          */
/*   Function:  memory_stats_dump                                           */
/*                                                                          */
/*   Input:     logfile - file for statistics output                        */
/*              w - interchange statistics                                  */
/*                                                                          */
/*   Description: print out the interchange statistics to a file            */
/*                                                                          */
/****************************************************************************/

static void memory_stats_dump(FILE *logfile, LoopStatsType *LoopStats)
{
 int i;
 float TotalFinalRatio = 0.0,
       TotalMemoryRatio = 0.0;
 float WeightedTotalFinalRatio = 0.0,
       WeightedTotalMemoryRatio = 0.0;
 int NumberLoops = 0;
 int WeightedNumberLoops = 0;

	fprintf(logfile,
		"Loop Nests With Exits:             %d\n",
		LoopStats->ContainsExit);
	fprintf(logfile,
		"Loop Nests With  IO:               %d\n",
		LoopStats->ContainsIO);
	fprintf(logfile,
		"Loop Nests With Calls:             %d\n",
		LoopStats->ContainsCall);
	fprintf(logfile,
		"Loop Nests With Conditional DOs:   %d\n\n\n",
		LoopStats->ContainsConditionalDO);
	fprintf(logfile,
		"\n\nTotal Loops: %d\n",LoopStats->TotalLoops);
	fprintf(logfile,
		"Loop Nests: %d\n",LoopStats->Nests);
	fprintf(logfile,
		"Imperfect Nests: %d\n",LoopStats->Imperfect);

	fprintf(logfile,
		"Permutable Sets of Loop Nests: %d\n",
		LoopStats->InMemoryOrder +
		LoopStats->InterchangedIntoMemoryOrder +
		LoopStats->NotInMemoryOrder);

	fprintf(logfile,
		"\n\nStats for Permutable Sets of Loops\n\n");
	fprintf(logfile,
		"\tIn Memory Order:                                %d\n",
		LoopStats->InMemoryOrder);
	fprintf(logfile,
		"\tInterchanged Into Memory Order:                 %d\n",
		LoopStats->InterchangedIntoMemoryOrder);
	fprintf(logfile,
		"\tUnable to Achieve Memory Order:                 %d\n",
		LoopStats->NotInMemoryOrder);

	fprintf(logfile,
		"\tTime Step Loop Prevented Memory Order: %d\n",
		LoopStats->TimeStepPreventedMemoryOrder);
	fprintf(logfile,
		"\tNearby Permutation Attained: %d\n",
		LoopStats->NearbyPermutationAttained);

	fprintf(logfile,
		"\tInner Loop Correct:                             %d\n",
		LoopStats->InnerLoopAlreadyCorrect);
	fprintf(logfile,
		"\tInterchanged to Obtain Correct Inner Loop:      %d\n",
		LoopStats->ObtainedInnerLoop);
	fprintf(logfile,
		"\tUnable to Achieve Correct Inner Loop:           %d\n",
		LoopStats->WrongInnerLoop);

	fprintf(logfile,
		"\tDesired Inner Loop was a Time Step Loop : %d\n",
		LoopStats->DesiredInnerTimeStep);
	fprintf(logfile,
		"\tNext Best Inner Loop Attained: %d\n",
		LoopStats->NextInnerLoop);
	fprintf(logfile,
		"\tInterchange Unsafe:                             %d\n",
		LoopStats->UnsafeInterchange);
	fprintf(logfile,
		"\tDistribution Unsafe:                            %d\n",
		LoopStats->DistributionUnsafe);
	fprintf(logfile,
		"\tNeeds Scalar Expansion:                         %d\n",
		LoopStats->NeedsScalarExpansion);
	fprintf(logfile,
		"\tLoops Reversed:                                 %d\n",
		LoopStats->Reversed);
	fprintf(logfile,
		"\tToo Complex:                                    %d\n\n\n",
		LoopStats->TooComplex);

		print_RefGroups(logfile,"Original Loop RefGroups\n",
		LoopStats->OriginalLocalityMatrix.SingleGroups.Invariant,
		LoopStats->OriginalLocalityMatrix.SingleGroups.Spatial,
		LoopStats->OriginalLocalityMatrix.SingleGroups.None,
		LoopStats->OriginalLocalityMatrix.MultiGroups.Invariant,
		LoopStats->OriginalLocalityMatrix.MultiGroups.Spatial,
		LoopStats->OriginalLocalityMatrix.MultiGroups.None,
		LoopStats->OriginalLocalityMatrix.MultiRefs.Invariant,
		LoopStats->OriginalLocalityMatrix.MultiRefs.Spatial,
		LoopStats->OriginalLocalityMatrix.MultiRefs.None,
		LoopStats->OriginalOtherSpatialGroups);

		print_RefGroups(logfile,"Final Loop RefGroups\n",
		LoopStats->FinalLocalityMatrix.SingleGroups.Invariant,
		LoopStats->FinalLocalityMatrix.SingleGroups.Spatial,
		LoopStats->FinalLocalityMatrix.SingleGroups.None,
		LoopStats->FinalLocalityMatrix.MultiGroups.Invariant,
		LoopStats->FinalLocalityMatrix.MultiGroups.Spatial,
		LoopStats->FinalLocalityMatrix.MultiGroups.None,
		LoopStats->FinalLocalityMatrix.MultiRefs.Invariant,
		LoopStats->FinalLocalityMatrix.MultiRefs.Spatial,
		LoopStats->FinalLocalityMatrix.MultiRefs.None,
		LoopStats->FinalOtherSpatialGroups);

		print_RefGroups(logfile,"Memory Order RefGroups\n",
		LoopStats->MemoryLocalityMatrix.SingleGroups.Invariant,
		LoopStats->MemoryLocalityMatrix.SingleGroups.Spatial,
		LoopStats->MemoryLocalityMatrix.SingleGroups.None,
		LoopStats->MemoryLocalityMatrix.MultiGroups.Invariant,
		LoopStats->MemoryLocalityMatrix.MultiGroups.Spatial,
		LoopStats->MemoryLocalityMatrix.MultiGroups.None,
		LoopStats->MemoryLocalityMatrix.MultiRefs.Invariant,
		LoopStats->MemoryLocalityMatrix.MultiRefs.Spatial,
		LoopStats->MemoryLocalityMatrix.MultiRefs.None,
		LoopStats->MemoryOtherSpatialGroups);

	fprintf(logfile,"Depth     1      2      3      4      5 ");
	fprintf(logfile,"     6      7      8      9      10\n");
	fprintf(logfile,"         ===    ===    ===    ===    ===");
	fprintf(logfile,"    ===    ===    ===    ===    ===\n");
        fprintf(logfile,"Loops ");
	for (i = 0; i < NESTING_DEPTH; i++)
	  {
	   fprintf(logfile,"%6d ",LoopStats->NestingDepth[i]);
	   NumberLoops += LoopStats->NestingDepth[i];
	   WeightedNumberLoops += LoopStats->NestingDepth[i] * (i+1);
	  }
        fprintf(logfile,"\nFinal ");
	for (i = 0; i < NESTING_DEPTH; i++)
	  if (LoopStats->NestingDepth[i] > 0)
	    if (LoopStats->FinalRatio[i] > 0.0)
	      {
	       fprintf(logfile,"%6.2f ",LoopStats->FinalRatio[i] /
		       ((float) LoopStats->NestingDepth[i]));
	       TotalFinalRatio += LoopStats->FinalRatio[i];
	       WeightedTotalFinalRatio += 
			(i+1) * LoopStats->FinalRatio[i];
	      }
	    else
	      {
	       fprintf(logfile,"%6.2f ",1.00);
	       TotalFinalRatio += ((float) LoopStats->NestingDepth[i]);
	       WeightedTotalFinalRatio += 
			(i+1) * ((float) LoopStats->NestingDepth[i]);
	      }
	  else
	    fprintf(logfile,"%6.2f ",1.00);
        fprintf(logfile,"\nMem   ");
	for (i = 0; i < NESTING_DEPTH; i++)
	  if (LoopStats->NestingDepth[i] > 0)
	    if (LoopStats->MemoryRatio[i] > 0.0)
	      {
	       fprintf(logfile,"%6.2f ",LoopStats->MemoryRatio[i] /
		       ((float) LoopStats->NestingDepth[i]));
	       TotalMemoryRatio += LoopStats->MemoryRatio[i];
	       WeightedTotalMemoryRatio += 
			(i+1) * LoopStats->MemoryRatio[i];
	      }
	    else
	      {
	       fprintf(logfile,"%6.2f ",1.00);
	       TotalMemoryRatio += ((float) LoopStats->NestingDepth[i]);
	       WeightedTotalMemoryRatio += 
			(i+1) * ((float) LoopStats->NestingDepth[i]);
	      }
	  else
	    fprintf(logfile,"%6.2f ",1.00);
         if (NumberLoops > 0)
	   {
	    fprintf(logfile,"\n\nAverage Final Improvement = %.2f\n",
		    TotalFinalRatio/((float) NumberLoops));
	    fprintf(logfile,"Average Memory Improvement = %.2f\n",
		    TotalMemoryRatio/((float) NumberLoops));
	    fprintf(logfile,"\n\nAverage Weighted Final Improvement = %.2f\n",
		    WeightedTotalFinalRatio/((float) WeightedNumberLoops));
	    fprintf(logfile,"Average Weighted Memory Improvement = %.2f\n",
		    WeightedTotalMemoryRatio/((float) WeightedNumberLoops));
	   }
	 else
	   {
	    fprintf(logfile,"\n\nAverage Final Improvement = %.2f\n",1.00);
	    fprintf(logfile,"Average Memory Improvement = %.2f\n",1.00);
	   }
         fprintf(logfile,"\n\n\n");
	
}


/****************************************************************************/
/*                                                                          */
/*   Function:   mh_walk_ast                                                */
/*                                                                          */
/*   Input:      selection - transformation to be performed on ast          */
/*               ped - structure containing dependence graph                */
/*               root - AST root of program                                 */
/*               ft - fort tree of program                                  */
/*               mod_context - context of module being tranformed           */
/*               ar - arena object for arena-based memory allocation        */
/*                                                                          */
/*   Description: Walk the AST to perform selected transformations on each  */
/*                loop nest.  Maintain interchange statistics.              */
/*                                                                          */
/****************************************************************************/

void mh_walk_ast(int          selection,
                 PedInfo      ped,
                 AST_INDEX    root,
		 FortTree     ft,
		 Context      mod_context,
		 arena_type   *ar)

  {
   walk_info_type walk_info;
   static int file = 0,i;
   char fn[DB_PATH_LENGTH];
   
     show_message2("The Memory Optimizer is in progress");

     if (((config_type *)PED_MH_CONFIG(ped))->logging > 0 ||
	 selection == LI_STATS || selection == UJ_STATS || 
	 selection == SR_STATS)
       {
	sprintf(fn,"%s.STATSLOG", ctxLocation(mod_context));
	((config_type *)PED_MH_CONFIG(ped))->logfile = fopen(fn,"w");
       }

     walk_info.selection = selection;
     walk_info.ped = ped;
     walk_info.ft = ft;
     walk_info.ftt = PED_FTT(ped);
     walk_info.ar = ar;
     walk_info.program = ctxLocation(mod_context); 
     walk_info.LoopStats = (LoopStatsType *)calloc(1,sizeof(LoopStatsType));
     
     walk_info.MainProgram = false;
	
     walk_statements(root,LEVEL1,(WK_STMT_CLBACK)remove_do_labels,NOFUNC,
		     (Generic)NULL);
     walk_statements(root,LEVEL1,(WK_STMT_CLBACK)build_label_symtab,
		     (WK_STMT_CLBACK)check_labels,(Generic)&walk_info);
   /*   if (selection != CACHE_ANALYSIS)

        a2i barfs if I modify program structure.  It needs to map statements
	  back into there original parsed text.  I should fix this sometime */

       walk_statements(root,LEVEL1,NOFUNC,
		       (WK_STMT_CLBACK)ut_change_logical_to_block_if,
		       (Generic)NULL);
     walk_statements(root,LEVEL1,(WK_STMT_CLBACK)pre_walk,(WK_STMT_CLBACK)post_walk,
		     (Generic)&walk_info);
     if (selection == FUSION || selection == LI_FUSION)
       walk_statements(root,LEVEL1,(WK_STMT_CLBACK)TryFusionToCleanup,NOFUNC,
		       (Generic)ped);
     walk_statements(root,LEVEL1,(WK_STMT_CLBACK)build_label_symtab,
		     (WK_STMT_CLBACK)check_labels,(Generic)&walk_info);
     walk_statements(root,LEVEL1,(WK_STMT_CLBACK)get_symtab_for_decls,NOFUNC,
		     (Generic)ft);

     switch (selection)
       {
	case UJ_STATS:
	  LoopStats->PredictedFinalBalance  +=
	      walk_info.LoopStats->PredictedFinalBalance; 
	  LoopStats->PredictedInitialBalance +=
	      walk_info.LoopStats->PredictedInitialBalance;
	  LoopStats->InitialBalanceWithInterlock +=
	      walk_info.LoopStats->InitialBalanceWithInterlock;
	  LoopStats->ActualFinalBalance +=
	      walk_info.LoopStats->ActualFinalBalance;
	  LoopStats->FinalBalanceWithInterlock +=
	      walk_info.LoopStats->FinalBalanceWithInterlock;
	  LoopStats->InitialInterlock +=
	      walk_info.LoopStats->InitialInterlock; 
	  LoopStats->FinalInterlock +=
	      walk_info.LoopStats->FinalInterlock; 
	  LoopStats->PredictedFPRegisterPressure +=
	      walk_info.LoopStats->PredictedFPRegisterPressure;
	  LoopStats->ActualFPRegisterPressure +=
	      walk_info.LoopStats->ActualFPRegisterPressure;
	  LoopStats->UnrolledLoops +=
	      walk_info.LoopStats->UnrolledLoops;
	  LoopStats->NotUnrolled +=
	      walk_info.LoopStats->NotUnrolled;
	  LoopStats->SingleDepth +=
	      walk_info.LoopStats->SingleDepth;
          LoopStats->NotUnrolledBalance +=
              walk_info.LoopStats->NotUnrolledBalance;
	  LoopStats->NotUnrolledBalanceWithInterlock +=
	      walk_info.LoopStats->NotUnrolledBalanceWithInterlock;
          LoopStats->NotUnrolledInterlock +=
              walk_info.LoopStats->NotUnrolledInterlock;
	  LoopStats->NotUnrolledFPRegisterPressure +=
	      walk_info.LoopStats->NotUnrolledFPRegisterPressure;
	  LoopStats->SingleDepthBalance +=
	      walk_info.LoopStats->SingleDepthBalance;
	  LoopStats->SingleDepthBalanceWithInterlock +=
	      walk_info.LoopStats->SingleDepthBalanceWithInterlock;
	  LoopStats->SingleDepthInterlock +=
	      walk_info.LoopStats->SingleDepthInterlock;
	  LoopStats->SingleDepthFPRegisterPressure +=
	      walk_info.LoopStats->SingleDepthFPRegisterPressure;
	  LoopStats->Distribute +=
	      walk_info.LoopStats->Distribute;
	  LoopStats->Interchange +=
	      walk_info.LoopStats->Interchange;
	  LoopStats->NoImprovement +=
	      walk_info.LoopStats->NoImprovement;
	  LoopStats->Normalized +=
	      walk_info.LoopStats->Normalized;
	  LoopStats->AlreadyBalanced +=
	      walk_info.LoopStats->AlreadyBalanced;
	  LoopStats->InterlockCausedUnroll +=
	      walk_info.LoopStats->InterlockCausedUnroll;
	  break;
       case LI_STATS:
         LoopStats->TotalLoops 
             += walk_info.LoopStats->TotalLoops;
         LoopStats->NotInMemoryOrder 
             += walk_info.LoopStats->NotInMemoryOrder;
         LoopStats->InMemoryOrder 
             += walk_info.LoopStats->InMemoryOrder;
         LoopStats->NearbyPermutationAttained
             += walk_info.LoopStats->NearbyPermutationAttained;
         LoopStats->NearbyPermutationAttained 
             += walk_info.LoopStats->NearbyPermutationAttained;
         LoopStats->InterchangedIntoMemoryOrder 
             += walk_info.LoopStats->InterchangedIntoMemoryOrder;
         LoopStats->TimeStepPreventedMemoryOrder 
             += walk_info.LoopStats->TimeStepPreventedMemoryOrder;
         LoopStats->ObtainedInnerLoop 
             += walk_info.LoopStats->ObtainedInnerLoop;
         LoopStats->InnerLoopAlreadyCorrect 
             += walk_info.LoopStats->InnerLoopAlreadyCorrect;
         LoopStats->WrongInnerLoop 
             += walk_info.LoopStats->WrongInnerLoop;
         LoopStats->DesiredInnerTimeStep 
             += walk_info.LoopStats->DesiredInnerTimeStep;
         LoopStats->NextInnerLoop 
             += walk_info.LoopStats->NextInnerLoop;
         LoopStats->UnsafeInterchange 
             += walk_info.LoopStats->UnsafeInterchange;
         LoopStats->DistributionUnsafe 
             += walk_info.LoopStats->DistributionUnsafe;
         LoopStats->NeedsScalarExpansion 
             += walk_info.LoopStats->NeedsScalarExpansion;
         LoopStats->TimeStepPreventedMemoryOrder
             += walk_info.LoopStats->TimeStepPreventedMemoryOrder;
         LoopStats->ContainsExit 
             += walk_info.LoopStats->ContainsExit;
         LoopStats->ContainsIO 
             += walk_info.LoopStats->ContainsIO;
         LoopStats->ContainsCall 
             += walk_info.LoopStats->ContainsCall;
         LoopStats->ContainsConditionalDO 
             += walk_info.LoopStats->ContainsConditionalDO;
         LoopStats->TooComplex 
             += walk_info.LoopStats->TooComplex;
         LoopStats->Nests 
             += walk_info.LoopStats->Nests;
         LoopStats->Imperfect 
             += walk_info.LoopStats->Imperfect;
         LoopStats->OriginalLocalityMatrix.SingleGroups.Invariant
            += walk_info.LoopStats->OriginalLocalityMatrix.SingleGroups.Invariant;
         LoopStats->OriginalLocalityMatrix.SingleGroups.Spatial
             += walk_info.LoopStats->OriginalLocalityMatrix.SingleGroups.Spatial;
         LoopStats->OriginalLocalityMatrix.SingleGroups.None
             += walk_info.LoopStats->OriginalLocalityMatrix.SingleGroups.None;
         LoopStats->FinalLocalityMatrix.SingleGroups.Invariant
            += walk_info.LoopStats->FinalLocalityMatrix.SingleGroups.Invariant;
         LoopStats->FinalLocalityMatrix.SingleGroups.Spatial
             += walk_info.LoopStats->FinalLocalityMatrix.SingleGroups.Spatial;
         LoopStats->FinalLocalityMatrix.SingleGroups.None
             += walk_info.LoopStats->FinalLocalityMatrix.SingleGroups.None;
         LoopStats->MemoryLocalityMatrix.SingleGroups.Invariant
            += walk_info.LoopStats->MemoryLocalityMatrix.SingleGroups.Invariant;
         LoopStats->MemoryLocalityMatrix.SingleGroups.Spatial
             += walk_info.LoopStats->MemoryLocalityMatrix.SingleGroups.Spatial;
         LoopStats->MemoryLocalityMatrix.SingleGroups.None
             += walk_info.LoopStats->MemoryLocalityMatrix.SingleGroups.None;
         LoopStats->OriginalLocalityMatrix.MultiGroups.Invariant
            += walk_info.LoopStats->OriginalLocalityMatrix.MultiGroups.Invariant;
         LoopStats->OriginalLocalityMatrix.MultiGroups.Spatial
             += walk_info.LoopStats->OriginalLocalityMatrix.MultiGroups.Spatial;
         LoopStats->OriginalLocalityMatrix.MultiGroups.None
             += walk_info.LoopStats->OriginalLocalityMatrix.MultiGroups.None;
         LoopStats->OriginalLocalityMatrix.MultiRefs.Invariant 
             += walk_info.LoopStats->OriginalLocalityMatrix.MultiRefs.Invariant;
         LoopStats->OriginalLocalityMatrix.MultiRefs.Spatial
             += walk_info.LoopStats->OriginalLocalityMatrix.MultiRefs.Spatial;
         LoopStats->OriginalLocalityMatrix.MultiRefs.None 
             += walk_info.LoopStats->OriginalLocalityMatrix.MultiRefs.None;
         LoopStats->FinalLocalityMatrix.MultiGroups.Invariant
            += walk_info.LoopStats->FinalLocalityMatrix.MultiGroups.Invariant;
         LoopStats->FinalLocalityMatrix.MultiGroups.Spatial
             += walk_info.LoopStats->FinalLocalityMatrix.MultiGroups.Spatial;
         LoopStats->FinalLocalityMatrix.MultiGroups.None
             += walk_info.LoopStats->FinalLocalityMatrix.MultiGroups.None;
         LoopStats->FinalLocalityMatrix.MultiRefs.Invariant 
             += walk_info.LoopStats->FinalLocalityMatrix.MultiRefs.Invariant;
         LoopStats->FinalLocalityMatrix.MultiRefs.Spatial
             += walk_info.LoopStats->FinalLocalityMatrix.MultiRefs.Spatial;
         LoopStats->FinalLocalityMatrix.MultiRefs.None 
             += walk_info.LoopStats->FinalLocalityMatrix.MultiRefs.None;
         LoopStats->MemoryLocalityMatrix.MultiGroups.Invariant
            += walk_info.LoopStats->MemoryLocalityMatrix.MultiGroups.Invariant;
         LoopStats->MemoryLocalityMatrix.MultiGroups.Spatial
             += walk_info.LoopStats->MemoryLocalityMatrix.MultiGroups.Spatial;
         LoopStats->MemoryLocalityMatrix.MultiGroups.None
             += walk_info.LoopStats->MemoryLocalityMatrix.MultiGroups.None;
         LoopStats->MemoryLocalityMatrix.MultiRefs.Invariant 
             += walk_info.LoopStats->MemoryLocalityMatrix.MultiRefs.Invariant;
         LoopStats->MemoryLocalityMatrix.MultiRefs.Spatial
             += walk_info.LoopStats->MemoryLocalityMatrix.MultiRefs.Spatial;
         LoopStats->MemoryLocalityMatrix.MultiRefs.None 
             += walk_info.LoopStats->MemoryLocalityMatrix.MultiRefs.None;
         LoopStats->OriginalOtherSpatialGroups 
             += walk_info.LoopStats->OriginalOtherSpatialGroups;
         LoopStats->FinalOtherSpatialGroups 
             += walk_info.LoopStats->FinalOtherSpatialGroups;
         LoopStats->MemoryOtherSpatialGroups 
             += walk_info.LoopStats->MemoryOtherSpatialGroups;
         for (i = 0; i < NESTING_DEPTH; i++)
	   {
	    LoopStats->FinalRatio[i] 
	      += walk_info.LoopStats->FinalRatio[i];
	    LoopStats->MemoryRatio[i] 
	      += walk_info.LoopStats->MemoryRatio[i];
	    LoopStats->NestingDepth[i]
	      += walk_info.LoopStats->NestingDepth[i];
	   }
         LoopStats->Reversed += walk_info.LoopStats->Reversed;

	 memory_stats_dump(((config_type *)PED_MH_CONFIG(ped))->logfile,
			    walk_info.LoopStats);
	 break;
       case SR_STATS:
	 /* ACCUMULATE PROGRAM TOTALS HERE */
	 /* DUMP ROUTINE TOTALS HERE */
           LoopStats->NumLoop_badexit
                  += walk_info.LoopStats->NumLoop_badexit;
	   LoopStats->NumLoop_backjump
	          += walk_info.LoopStats->NumLoop_backjump;
           LoopStats->NumLoop_illjump
                 += walk_info.LoopStats->NumLoop_illjump;
           LoopStats->NumZeroFPLoop
		+= walk_info.LoopStats->NumZeroFPLoop;
           LoopStats->NumLoopSpilled
		+= walk_info.LoopStats->NumLoopSpilled;
	   LoopStats->FPRegisterPressure
	          += walk_info.LoopStats->FPRegisterPressure;
           LoopStats->SRRegisterPressure
		  += walk_info.LoopStats->SRRegisterPressure;
	   LoopStats->NumRefRep
	          += walk_info.LoopStats->NumRefRep;
           LoopStats->NumLoopReplaced
		  += walk_info.LoopStats->NumLoopReplaced;
	   LoopStats->NumBasicBlock
	          += walk_info.LoopStats->NumBasicBlock;
	   LoopStats->NumInnermostLoop
	          += walk_info.LoopStats->NumInnermostLoop;
	   LoopStats->LoopBal
	          += walk_info.LoopStats->LoopBal;
           /* QUNYAN 0001*/
	   /* Add in accumulation for LIAV, LCAV... */
	   LoopStats->NumLIAV
		  += walk_info.LoopStats->NumLIAV;
	   LoopStats->NumLCAV
		  += walk_info.LoopStats->NumLCAV;
	   LoopStats->NumLIPAV
    	          += walk_info.LoopStats->NumLIPAV;
           LoopStats->NumLCPAV
	          += walk_info.LoopStats->NumLCPAV;
           LoopStats->NumInv
	          += walk_info.LoopStats->NumInv;
           LoopStats->NumLC1
	          += walk_info.LoopStats->NumLC1;
	   /* QUNYAN 0001 */					    

	   LoopStats->UniformRefs
	          += walk_info.LoopStats->UniformRefs;
	   LoopStats->NonUniformRefs
	          += walk_info.LoopStats->NonUniformRefs;
	   LoopStats->NonUniformLoopsReplaced
	          += walk_info.LoopStats->NonUniformLoopsReplaced;
           SRStatsDump(((config_type *)PED_MH_CONFIG(ped))->logfile,
				       walk_info.LoopStats);
           break;
       default:
         break;
      }
     if (((config_type *)PED_MH_CONFIG(ped))->logging > 0 ||
	 selection == LI_STATS || selection == UJ_STATS ||
	 selection == SR_STATS)
       fclose(((config_type *)PED_MH_CONFIG(ped))->logfile);
     hide_message2();
  }


/****************************************************************************/
/*                                                                          */
/*   Function:   ApplyMemoryCompiler                                        */
/*                                                                          */
/*   Input:      selection - transformation to be performed on ast          */
/*               ped - structure containing dependence graph                */
/*               root - AST root of program                                 */
/*               ft - fort tree of program                                  */
/*               mod_context - context of module being tranformed           */
/*               config_file - name of configuration file                   */
/*                                                                          */
/*   Description: Entry point for stand alone memory compiler.  Calls       */
/*                mh_walk_ast to perform the requested transformations.     */
/*                                                                          */
/****************************************************************************/

void ApplyMemoryCompiler(int         selection,
			 PedInfo     ped, 
			 AST_INDEX   root, 
			 FortTree    ft, 
			 Context     mod_context,
			 char        *config_file)
  {
   arena_type   ar(1);
   
   if (!PED_MH_CONFIG(ped))
     {
      PED_MH_CONFIG(ped) = (int) new config_type;
      mh_get_config((config_type *)PED_MH_CONFIG(ped),config_file);
     }
   if (LoopStats == NULL)
     LoopStats = (LoopStatsType *)calloc(1,sizeof(LoopStatsType));
   
   fprintf(stderr,"Analyzing %s...\n", ctxLocation(mod_context));
   ftt_TreeWillChange(PED_FTT(ped),root);
   mh_walk_ast(selection,ped,root,ft, mod_context, &ar);
   ftt_TreeChanged(PED_FTT(ped),root);
  }


/****************************************************************************/
/*                                                                          */
/*   Function:   memory_stats_total                                         */
/*                                                                          */
/*   Input:      program - name of program being processed                  */
/*                                                                          */
/*   Description:  print out the total statistics for an entire program     */
/*                                                                          */
/****************************************************************************/

void memory_stats_total(char *program)

  {
   char fn[DB_PATH_LENGTH];
   FILE *logfile;

     sprintf(fn,"%s.STATSLOG",program);
     logfile = fopen(fn,"w");
     fprintf(logfile,"\n\nTotal Statistics for Program %s\n",program);
     memory_stats_dump(logfile,LoopStats);
     fclose(logfile);
  }


void UnrollStatsDump(FILE *logfile, LoopStatsType *LoopStats)

  {
     fprintf(logfile,"Loops Unrolled %d\n\n",LoopStats->UnrolledLoops);
     fprintf(logfile,"Average Predicted Unroll-and-Jam Statistics\n");
     fprintf(logfile,"===========================================\n\n");
     fprintf(logfile,"Initial Loop Balance = %.4f\n",
	     LoopStats->PredictedInitialBalance/
	     (float)LoopStats->UnrolledLoops);
     fprintf(logfile,"Initial Loop Balance With Interlock= %.4f\n",
	     LoopStats->InitialBalanceWithInterlock/
	     (float)LoopStats->UnrolledLoops);
     fprintf(logfile,"Final Loop Balance   = %.4f\n",
	     LoopStats->PredictedFinalBalance/
	     (float)LoopStats->UnrolledLoops);
     fprintf(logfile,"FP Register Pressure = %.4f\n",
	     LoopStats->PredictedFPRegisterPressure/
	     (float)LoopStats->UnrolledLoops);
     fprintf(logfile,"Initial Interlock Factor     = %.4f\n",
	     LoopStats->InitialInterlock/(float)LoopStats->UnrolledLoops);
     fprintf(logfile,"\n\nAverage Actual Unroll-and-Jam Statistics\n");
     fprintf(logfile,"========================================\n\n");
     fprintf(logfile,"FP Register Pressure = %.4f\n",
	     LoopStats->ActualFPRegisterPressure/
	     (float)LoopStats->UnrolledLoops);
     fprintf(logfile,"Final Loop Balance   = %.4f\n",
	     LoopStats->ActualFinalBalance/
	     (float)LoopStats->UnrolledLoops);
     fprintf(logfile,"Final Loop Balance With Interlock  = %.4f\n",
	     LoopStats->FinalBalanceWithInterlock/
	     (float)LoopStats->UnrolledLoops);
     fprintf(logfile,"Final Interlock Factor     = %.4f\n",
	     LoopStats->FinalInterlock/(float)LoopStats->UnrolledLoops);
     fprintf(logfile,"Interlock Caused Unrolling = %d\n",
	     LoopStats->InterlockCausedUnroll);
     fprintf(logfile,"\n\nLoops Not Unrolled %d\n\n",LoopStats->NotUnrolled);
     fprintf(logfile,"Average Not Unrolled Statistics\n");
     fprintf(logfile,"===============================\n\n");
     fprintf(logfile,"Loop Balance = %.4f\n",
	     LoopStats->NotUnrolledBalance/(float)LoopStats->NotUnrolled);
     fprintf(logfile,"Loop Balance with Interlock = %.4f\n",
	     LoopStats->NotUnrolledBalanceWithInterlock/
	     (float)LoopStats->NotUnrolled);
     fprintf(logfile,"FP Register Pressure = %.4f\n",
	     LoopStats->NotUnrolledFPRegisterPressure/(float)LoopStats->NotUnrolled);
     fprintf(logfile,"Interlock Factor     = %.4f\n",
	     LoopStats->NotUnrolledInterlock/(float)LoopStats->NotUnrolled);
     fprintf(logfile,"Failed -- Distribution %d\n",LoopStats->Distribute);
     fprintf(logfile,"Failed -- Interchange %d\n",LoopStats->Interchange);
     fprintf(logfile,"No Improvement Possible %d\n",LoopStats->NoImprovement);
     fprintf(logfile,"Already Balanced %d\n",LoopStats->AlreadyBalanced);
     fprintf(logfile,"No Improvement Due to Normalization %d\n",LoopStats->Normalized);
     fprintf(logfile,"\n\nSingle Depth Loops %d\n\n",LoopStats->SingleDepth);
     fprintf(logfile,"Average Single Depth Statistics\n");
     fprintf(logfile,"===============================\n\n");
     fprintf(logfile,"Loop Balance = %.4f\n",
	     LoopStats->SingleDepthBalance/(float)LoopStats->SingleDepth);
     fprintf(logfile,"Loop Balance with Interlock= %.4f\n",
	     LoopStats->SingleDepthBalanceWithInterlock/
	     (float)LoopStats->SingleDepth);
     fprintf(logfile,"FP Register Pressure = %.4f\n",
	     LoopStats->SingleDepthFPRegisterPressure/(float)LoopStats->SingleDepth);
     fprintf(logfile,"Interlock Factor     = %.4f\n",
	     LoopStats->SingleDepthInterlock/(float)LoopStats->SingleDepth);
  }

void SRStatsDump(FILE *logfile, LoopStatsType *LoopStats)
  {
    int total_loop_replaced;

    total_loop_replaced = LoopStats->NumLoopReplaced;

     fprintf(logfile,"\n\n");
   
     fprintf(logfile, "Total Number of Bad Flow = %d\n\n",
  		     LoopStats->Numbadflow);

   if(LoopStats->Numbadflow > 0)
    {

     fprintf(logfile, "Total number of bad exits = %d\n\n",
  		     LoopStats->NumLoop_badexit); 

     fprintf(logfile, "Total number of back jumps = %d\n\n",
  		     LoopStats->NumLoop_backjump); 

     fprintf(logfile, "Total number of illegal jumps = %d\n\n",
                     LoopStats->NumLoop_illjump);
    }
   

   fprintf(logfile, "Total Number of Innermost Loop = %d\n\n",
		     LoopStats->NumInnermostLoop);

   fprintf(logfile, "Total Number of Replaced Loop = %d\n\n",
		     LoopStats->NumLoopReplaced);

   fprintf(logfile, "Total Number of Loop Spilled = %d\n\n",
		     LoopStats->NumLoopSpilled);

   fprintf(logfile, "Total number of replaced loops wo/ FP pressure = %d\n\n",
                     LoopStats->NumZeroFPLoop);
 
       int LoopRepw_pressure; /* Loop Replaced with FP register pressure */

       LoopRepw_pressure = total_loop_replaced - LoopStats->NumZeroFPLoop;


   /* QUNYAN 0002 */
   /* Add print for LIAV, LCAV .. and average */

   fprintf(logfile, "Total Number of LIAV = %dn\n", LoopStats->NumLIAV);
   fprintf(logfile, "Total Number of LCAV = %d\n\n", LoopStats->NumLCAV);
   fprintf(logfile, "Total Number of LIPAV = %d\n\n", LoopStats->NumLIPAV);
   fprintf(logfile, "Total Number of LCPAV = %d\n\n", LoopStats->NumLCPAV);
   fprintf(logfile, "Total Number of Loop Carried Invariant = %d\n\n",
			LoopStats->NumInv);
   fprintf(logfile, "Total Number of Loop Carried Distance 1 = %d\n\n",
			LoopStats->NumLC1);
   fprintf(logfile,"Average LIAV Pressure/Loop Replaced w/ pressur = %.4f\n\n",
  	            (float)LoopStats->NumLIAV/(float)LoopRepw_pressure);
   fprintf(logfile,"Average LCAV register Pressure/Loop Replaced w/ pressur = %.4f\n\n", (float)LoopStats->NumLCAV/(float)LoopRepw_pressure);
   fprintf(logfile, "Average LIPAV register Pressure/Loop Replaced w/ pressur = %.4f\n\n",
		    (float)LoopStats->NumLIPAV/(float)LoopRepw_pressure);
   fprintf(logfile, "Average LCPAV register Pressure/Loop Replaced w/ pressur = %.4f\n\n",
		    (float)LoopStats->NumLCPAV/(float)LoopRepw_pressure);
   /* QUNYAN 0002 */

   fprintf(logfile, "Average FP register Pressure/Loop Replaced w/ pressur = %.4f\n\n",
  	            (float)LoopStats->FPRegisterPressure/(float)LoopRepw_pressure);

   fprintf(logfile, "Average SR register Pressure/Loop Replaced w/ pressur = %.4f\n\n",
		    (float)LoopStats->SRRegisterPressure/(float)LoopRepw_pressure);

   if(total_loop_replaced > 0)
     {
      /*printf("total basic block = %d\n", LoopStats->NumBasicBlock); */
      fprintf(logfile, "Average FP register Pressure/Loop Replaced = %.4f\n\n",
   		     (float)LoopStats->FPRegisterPressure/(float)total_loop_replaced);

      fprintf(logfile, "Average SR register Pressure/Loop Replaced = %.4f\n\n",
		     (float)LoopStats->SRRegisterPressure/(float)total_loop_replaced);

      fprintf(logfile, "Average Number of Reference Replaced/Loop Replaced = %.4f\n\n",
   		     (float)LoopStats->NumRefRep/(float)total_loop_replaced);

      fprintf(logfile, "Average Number of Basic Blocks/Loop Repaced = %.4f\n\n",
		     (float)LoopStats->NumBasicBlock/(float)total_loop_replaced);

     }

   fprintf(logfile, "Average Loop Balance/Innermost Loop  = %.4f\n\n",
                    LoopStats->LoopBal/(float)LoopStats->NumInnermostLoop);
   fprintf(logfile, "Total Number of Uniform References = %d\n\n",
			LoopStats->UniformRefs);
   fprintf(logfile, "Total Number of Non-Uniform References = %d\n\n",
			LoopStats->NonUniformRefs);
   fprintf(logfile, "Total Number of Loops w/ Non-Uniform References = %d\n\n",
			LoopStats->NonUniformLoopsReplaced);
   fprintf(logfile, "Total Number of Loops w/ Non-Uniform References and no FP Pressure = %d\n\n",
			LoopStats->NonUniformLoopsZeroFP);


  }

/****************************************************************************/
/*                                                                          */
/*   Function:   memory_UnrollStatsTotal                                    */
/*                                                                          */
/*   Input:      program - name of program being processed                  */
/*                                                                          */
/*   Description:  print out the unroll statistics for an entire program    */
/*                                                                          */
/****************************************************************************/

void memory_UnrollStatsTotal(char *program)

  {
   char fn[DB_PATH_LENGTH];
   FILE *logfile;

     sprintf(fn,"%s.STATSLOG",program);
     logfile = fopen(fn,"w");
     UnrollStatsDump(logfile,LoopStats);
     fclose(logfile);
  }



/****************************************************************************/
/*                                                                          */
/*   Function:   memory_SRStatsTotal                                        */
/*                                                                          */
/*   Input:      program - name of program being processed                  */
/*                                                                          */
/*   Description:  print out the scalar replacement statistics for          */ 
/*                 an entire program                                        */
/*                                                                          */
/****************************************************************************/

void memory_SRStatsTotal(char *program)

  {
   char fn[DB_PATH_LENGTH];
   FILE *logfile;

     sprintf(fn,"%s.STATSLOG",program);
     logfile = fopen(fn,"w");
     SRStatsDump(logfile,LoopStats);
     fclose(logfile);
  }


