/* $Id: prefetch.C,v 1.24 2000/02/01 19:40:25 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

//  
//  File:  prefetch.C
//
//  Description: This code walks the loop structure model_loop looking for innermost
//               loops.  For each innermost loop, it determines the cache locality
//               of each array reference.  Then, using the algorithm of Mowry, et al.,
//               it software pipelines software prefetches.
//
//  Creation: 9/14/95
//  
//  Modifications:  Removed code to ensure that no bogus prefetches are issued.
//                  Now count on hardware to suppress such prefetches. 
//                  - smc 5/20/96
//
//                  fixed bug in unroll amount determination - smc 9/29/96

#include <libs/Memoria/include/mh.h>
#include <libs/frontEnd/include/gi.h>
#include <libs/Memoria/sp/prefetch.h>
#include <libs/Memoria/include/analyze.h>
#include <libs/Memoria/include/shape.h>
#include <libs/Memoria/include/mem_util.h>
#include <libs/Memoria/include/mark.h>
#include <libs/Memoria/include/mh_ast.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>
#include <libs/frontEnd/include/walk.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dt.h>
#include <libs/Memoria/include/mh_config.h>
#include <libs/support/lists/SinglyLinkedList.h>
#include <libs/Memoria/include/label.h>
#include <strings.h>
#include <assert.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dg.h>
#include <libs/Memoria/include/UniformlyGeneratedSets.h>
#include <libs/Memoria/ut/Recurrence.h>
#undef is_open
#include <iostream.h>

extern Boolean Memoria_LetRocketSchedulePrefetches;
extern Boolean Memoria_IssueDead;
extern Boolean CheckRecurrencesForPrefetching;



//
//  Function: remove_edges
//
//  Input: stmt - AST of a statement in a loop
//         level - nesting level of statement
//         ped - dependence graph
//
//  Output: modified dependence graph
//
//  Description: Remove all io, exit, control and call dependences
//

static int remove_edges(AST_INDEX stmt,
			int       level,
			PedInfo   ped)

  {
    DG_Edge    *dg;       // dependence graph
    int        vector;    // level vector for dependence graph
    EDGE_INDEX edge,      // dependence edge
      next_edge;          // next dependence edge in list
    int        i;         // counter for nesting levels

     dg = dg_get_edge_structure( PED_DG(ped));
     vector = get_info(ped,stmt,type_levelv);

     // set store field for lhs of an assignment

     if (is_assignment(stmt))
       if (is_subscript(gen_ASSIGNMENT_get_lvalue(stmt)))
         get_subscript_ptr(gen_SUBSCRIPT_get_name(gen_ASSIGNMENT_get_lvalue(
			   stmt)))->store = true;

     // for each nesting level remove io,exit, call and control dependences

     for (i = 1;i <= level; i++)
       {
	for (edge = dg_first_src_stmt( PED_DG(ped),vector,i);
	     edge != END_OF_LIST;
	     edge = next_edge)
	  {
	   next_edge = dg_next_src_stmt( PED_DG(ped),edge);
	   if (dg[edge].type == dg_exit || dg[edge].type == dg_io ||
	       dg[edge].type == dg_call || dg[edge].type == dg_control)
	     dg_delete_free_edge( PED_DG(ped),edge);
	  }
	for (edge = dg_first_sink_stmt( PED_DG(ped),vector,i);
	     edge != END_OF_LIST;
	     edge = next_edge)
	  {
	   next_edge = dg_next_sink_stmt( PED_DG(ped),edge);
	   if (dg[edge].type == dg_exit || dg[edge].type == dg_io ||
	       dg[edge].type == dg_call || dg[edge].type == dg_control)
	     dg_delete_free_edge( PED_DG(ped),edge);
	  }
       }

     // remove loop independent exit, io, call and control dependences

     for (edge = dg_first_src_stmt( PED_DG(ped),vector,LOOP_INDEPENDENT);
	  edge != END_OF_LIST;
	  edge = next_edge)
       {
	next_edge = dg_next_src_stmt( PED_DG(ped),edge);
	if (dg[edge].type == dg_exit || dg[edge].type == dg_io ||
	    dg[edge].type == dg_call || dg[edge].type == dg_control)
	  dg_delete_free_edge( PED_DG(ped),edge);
       }

     // remove loop independent exit, io, call and control dependences
     for (edge = dg_first_sink_stmt( PED_DG(ped),vector,LOOP_INDEPENDENT);
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

//
//  Function: CheckLocality
//
//  Input: node - AST node
//         locality_info - structure containing various needed info
//
//  Output: updated Locality field for array references
//
//  Description: For each array reference check the type of locality it has and
//               set the file in subscript info to indicate.
//

static int CheckLocality(AST_INDEX          node,
			 locality_info_type *locality_info)

  {
    AST_INDEX name;             // identifier for array ref
    subscript_info_type *sptr;  // pointer to subscript info (store locality there)


   // do we have an array reference 

     if (is_subscript(node))
       {
	name = gen_SUBSCRIPT_get_name(node);
	sptr = get_subscript_ptr(name);

	// store locality information

	sptr->Locality = ut_GetReferenceType(node,locality_info->loop_data,
					     locality_info->loop,
					     locality_info->ped,
					     locality_info->UGS);
	return(WALK_SKIP_CHILDREN);
       }
     return(WALK_CONTINUE);
  }

//
//  Function: BuildPrefetchList
//
//  Input: node - AST node 
//         locality_info - structure containing prefetchlists
//
//  Output: possibly update prefetch list
//
//  Description: If we have an array reference, check locality and
//               add to appropriate list.
//

static int BuildPrefetchList(AST_INDEX          node,
			     locality_info_type *locality_info)

  {
    AST_INDEX name;           // identifier for array reference
    subscript_info_type *sptr;// pointer to subscript info (contains localilty for array)


    // do we have an array reference?

     if (is_subscript(node))
       {

	 // get subscript info (contains locality for reference)

	 name = gen_SUBSCRIPT_get_name(node);
	 sptr = get_subscript_ptr(name);

	 // We only prefetch stores if they are leaders of Group Spatial Set

	 // We prefetch any load with self-spatial or no reuse

	 if (sptr->Locality == NONE &&
	     (NOT(sptr->store) || sptr->GroupSpatialDistance > 0))
	   locality_info->WordPrefetches->append_entry(node); 
	 else if (sptr->Locality == SELF_SPATIAL &&
		  (NOT(sptr->store) || sptr->GroupSpatialDistance > 0))
	   locality_info->DPLinePrefetches->append_entry(node); 

	return(WALK_SKIP_CHILDREN);
       }
     return(WALK_CONTINUE);
  }

static int BuildDeadList(AST_INDEX          node,
			 locality_info_type *locality_info)

  {
   AST_INDEX name;
   subscript_info_type *sptr;

     if (is_subscript(node)){
        name = gen_SUBSCRIPT_get_name(node);
        sptr = get_subscript_ptr(name);
        if (sptr->Locality == NONE){
           locality_info->NoLocality->append_entry(node);
        }
        else if (sptr->Locality == SELF_TEMPORAL)
          locality_info->TempLocality->append_entry(node);
        else if (sptr->Locality == SELF_SPATIAL){
           locality_info->SpatLocality->append_entry(node);
        }
        return(WALK_SKIP_CHILDREN);
     }
     return(WALK_CONTINUE);
  }

//
//  Function: CheckRefsForPrefetch
//
//  Input: loop_data - tree structure for loop nest
//         loop - index into loop_data for current loop
//         LinePrefetches - list of array references to be prefetched by line
//         WordPrefetches - list of array references to be prefetched by word
//         ped - dependence graph and configuration info
//         IVar - induction variables
//
//  Output: Lists of array references for prefetching
//
//  Description: Walk statements and determine the locality of each array reference.
//               Then, walk them again and build up lists for prefetching.
//

static void CheckRefsForPrefetch(model_loop    *loop_data,
				 int           loop,
				 PrefetchList  *LinePrefetches,
				 PrefetchList  *WordPrefetches,
				 PrefetchList  *NoLocality,
				 PrefetchList  *TempLocality,
				 PrefetchList  *SpatLocality,
				 PedInfo       ped,
				 char          **IVar)

  {
   locality_info_type locality_info;

     locality_info.ped = ped;
     locality_info.loop = loop;
     locality_info.loop_data = loop_data;
     locality_info.DPLinePrefetches = LinePrefetches;
     locality_info.WordPrefetches = WordPrefetches;
     locality_info.NoLocality = NoLocality;
     locality_info.TempLocality = TempLocality;
     locality_info.SpatLocality = SpatLocality;

     // Use uniformly generated sets to compute locality information
     // this is not likely to work right now.

     if (mc_extended_cache)
       locality_info.UGS = new UniformlyGeneratedSets(loop_data[loop].node,
						      loop_data[loop].level,
						      IVar);
     else
       locality_info.UGS = NULL;

     // determine locality for each array reference in loop

     walk_expression(gen_DO_get_stmt_LIST(loop_data[loop].node),
		     (WK_EXPR_CLBACK)CheckLocality,NOFUNC,(Generic)&locality_info);

     // build up word and line prefetch lists

     walk_expression(gen_DO_get_stmt_LIST(loop_data[loop].node),
		     (WK_EXPR_CLBACK)BuildPrefetchList,NOFUNC,
		     (Generic)&locality_info);
     walk_expression(gen_DO_get_stmt_LIST(loop_data[loop].node),
		     (WK_EXPR_CLBACK)BuildDeadList,NOFUNC,
		     (Generic)&locality_info);
     delete locality_info.UGS;
  }
   
//
//  Function: LoadCycles
//
//  Input: Node - AST index of a subscript
//         ped - configuration info
//
//  Output: number of cycles for a load of a value
//
//  Description: based on type return how long an operation takes
//

static int LoadCycles(AST_INDEX Node,
		      PedInfo   ped)

  {
   int LoadPenalty;

    LoadPenalty = ((config_type *)PED_MH_CONFIG(ped))->hit_cycles;
    if (gen_get_converted_type(Node) == TYPE_REAL)
      return(LoadPenalty);

    // on some architectures a 64-bit value may take to load instructions
    // not likely anymore though

    else if (gen_get_converted_type(Node) == TYPE_DOUBLE_PRECISION ||
            gen_get_converted_type(Node) == TYPE_COMPLEX)
      return(((config_type *)PED_MH_CONFIG(ped))->double_fetches*LoadPenalty);
    else	
      return(0);
  }

//
//  Function: OperationCycles
//
//  Input: Node - binary operator AST
//         ped - configuration info
//
//  Output: number of cycles for an operator
//
//  Description: look up in configuration info how long a particular operation takes
//

static int OperationCycles(AST_INDEX Node,
			   PedInfo   ped)

  {
    int ops;  // complex operations take more than 1 instruction
     
     if (!is_binary_times(Node) || 
	 (!is_binary_plus(tree_out(Node)) && 
	  !is_binary_minus(tree_out(Node))) ||
	 !((config_type *)PED_MH_CONFIG(ped))->mult_accum)
       if (gen_get_converted_type(Node) == TYPE_DOUBLE_PRECISION ||
	   gen_get_converted_type(Node) == TYPE_COMPLEX ||
	   gen_get_converted_type(Node) == TYPE_REAL)
	 {
	  if (gen_get_converted_type(Node) == TYPE_COMPLEX)
	    ops = 2;
	  else
	    ops = 1;
	  if (is_binary_times(Node))
	    return(((config_type *)PED_MH_CONFIG(ped))->mul_cycles * ops);
	  else if (is_binary_plus(Node) || is_binary_minus(Node))
	    return(((config_type *)PED_MH_CONFIG(ped))->add_cycles * ops);
	  else if (is_binary_divide(Node))
	    return(((config_type *)PED_MH_CONFIG(ped))->div_cycles * ops);
	  else
	    return(ops); 
	 }
     return(0);
  }

//
//  Function: CountCycles
//
//  Input: Node - AST node of operation to check cycle count on
//         CycleInfo - record of cycles in loop so far
//
//  Output: Increase in number of cycles in loop depending on operation
//
//  Description: Add to memory cycles for array references and flops for operators
//

static int CountCycles(AST_INDEX     Node,
		       SPCycleInfoType *CycleInfo)

  {
    subscript_info_type *sptr; // point to subscript info

     if (is_subscript(Node))
       {
	sptr = get_subscript_ptr(gen_SUBSCRIPT_get_name(Node));

	// determine whether this is not scalar replaced to get cycle time

	if (sptr->Locality == NONE || sptr->Locality == SELF_SPATIAL ||
	    sptr->Locality == GROUP_SPATIAL)
	  CycleInfo->MemCycles += LoadCycles(Node,CycleInfo->ped);
       }
     else if (is_binary_op(Node))
	CycleInfo->FlopCycles += OperationCycles(Node,CycleInfo->ped);
     return(WALK_CONTINUE);
  }

//
//  Function: CyclesPerIteration
//
//  Input: Node -  AST index of an innermost loop
//         ped - dependence graph and configuration info
//
//  Output: The number of floating-point and memory cycles in a loop
//
//  Description: walk the statements in the AST and compute how much each
//               node requires in machine cycles
//

static int CyclesPerIteration(AST_INDEX Node,
			      PedInfo   ped)

  {
    SPCycleInfoType CycleInfo; // Cycle information
  
    // Initilization 

     CycleInfo.MemCycles = 0;
     CycleInfo.FlopCycles = 0;
     CycleInfo.ped = ped;

     // walk statements and compute cycles

     walk_expression(gen_DO_get_stmt_LIST(Node),(WK_EXPR_CLBACK)CountCycles,NOFUNC,
		     (Generic)&CycleInfo);
     
     // conservatively assume that memory and flops are parallel.  This gives a
     // lower bound on cycle time 

     if (CycleInfo.MemCycles >= CycleInfo.FlopCycles)
       return(CycleInfo.MemCycles);
     else
       return(CycleInfo.FlopCycles);
  }


static void AllocatePrefetches(PrefetchListIterator WordIterator,
			       PrefetchListIterator LineIterator,
			       PrefetchList *WordPrefetches,
			       PrefetchList *LinePrefetches,
			       float& ScheduleBandwidth,
			       float PrefetchBandwidth,
			       float LineValue,
			       float Cycles,
			       Recurrence *RData,
			       Boolean& NothingFetched,
			       Boolean RecurrenceOnly)
{

  Boolean OnRecurrence;

  // Add word prefetches until bandwidth required is too high
  
  while (WordIterator.current() != NULL)
    {
      if (RecurrenceOnly)
	OnRecurrence = 
	  RData->IsReferenceOnRecurrence(WordIterator.current()->GetValue());
      else
	OnRecurrence = true;
      
      if ((ScheduleBandwidth <= (PrefetchBandwidth - 1.0/Cycles) || NothingFetched)
	  && OnRecurrence)
	{
	  ScheduleBandwidth += (1.0/Cycles);
	  (void)++WordIterator;
	  NothingFetched = false;
	}
      else if (NOT(RecurrenceOnly))
	{
	  PrefetchListEntry *temp = WordIterator.current();
	  (void)++WordIterator;
	  WordPrefetches->Delete(temp);
	}
      else
	(void)++WordIterator;
    }
  
  // Add line prefetches until bandwidth too high
  
  while (LineIterator.current() != NULL)
    {
      if (RecurrenceOnly)
	{
	  OnRecurrence = 
	    RData->IsReferenceOnRecurrence(LineIterator.current()->GetValue());
	}
      else
	OnRecurrence = true;
      
      if ((ScheduleBandwidth <= (PrefetchBandwidth - LineValue/Cycles) || NothingFetched)
	  && OnRecurrence)
	{
	  ScheduleBandwidth += (LineValue/Cycles);
	  (void)++LineIterator;
	  NothingFetched = false;
	}
      else if (NOT(RecurrenceOnly))
	{
	  PrefetchListEntry *temp = LineIterator.current();
	  (void)++LineIterator;
	  LinePrefetches->Delete(temp);
	}
      else
	(void)++LineIterator;
    }
}
      
//
//  Function: ModeratePrefetchRequirements
//
//  Input: loop_data - tree structure of loop nest
//         loop - index into loop_data of current loop
//         LinePrefetches - list of array references to prefetch by line
//         WordPrefetches - list of array references to prefetch by word
//         ped - dependence graph and configuration info
//
//  Output: smaller list of prefetches that fit into the bandwidth provided
//          by target machine
//
//  Description: Remove prefetches if not enough bandwidth.  Give preference to
//               word prefetches as they eliminate more misses. 
//

static void ModeratePrefetchRequirements(model_loop   *loop_data,
					 int          loop,
					 PrefetchList *LinePrefetches,
					 PrefetchList *WordPrefetches,
					 PedInfo      ped)

  {
    float PrefetchBandwidth;   // bandwidth provided by machine
    float BandwidthNeeded;     // bandwidth required by loop
    float ScheduleBandwidth;   // bandwidth required by modified prefetch lists
    float LineValue;           // bandwidth need by line prefetches
    float Cycles;
    PrefetchListIterator LineIterator(LinePrefetches),// Iterator for line prefetches
      WordIterator(WordPrefetches);                   // Iterator for word prefetches
    PrefetchListEntry    *temp;
    Boolean NothingFetched = true;



   // Get machine prefetch bandwidth 

     if (((config_type *)PED_MH_CONFIG(ped))->prefetch_latency == 0)
       PrefetchBandwidth = 0.0;
     else
       PrefetchBandwidth = 
        ((float) ((config_type *)PED_MH_CONFIG(ped))->prefetch_buffer) /
        ((float) ((config_type *)PED_MH_CONFIG(ped))->prefetch_latency);

     // compute total bandwidth required

     BandwidthNeeded = WordPrefetches->Count();
     LineValue = ((float)LinePrefetches->Count()) *
       (8.0/(float)((config_type *)PED_MH_CONFIG(ped))->line);
     BandwidthNeeded += LineValue;
     Cycles = CyclesPerIteration(loop_data[loop].node,ped);
     BandwidthNeeded /= Cycles;


     // If machine does not have enough bandwidth, remove some prefetches
     // We would like to issue at least one prefetch in a loop even if
     // bandwidth is not there (Is this too aggressive? I don't know yet.)

     if (BandwidthNeeded > PrefetchBandwidth)
       {
	 Recurrence *RData = NULL;
	 ScheduleBandwidth = 0.0;

	 // First Check for References on Recurrences

	 if (CheckRecurrencesForPrefetching)
	   {
	     RData = new Recurrence(loop_data[loop].node,ped,loop_data[loop].level);
	     AllocatePrefetches(WordIterator,LineIterator,WordPrefetches,LinePrefetches,
				ScheduleBandwidth,PrefetchBandwidth,LineValue,Cycles,
				RData,NothingFetched,true);
	     delete RData;
	     RData = NULL;
	   }

	 // Now Check for any References

	 WordIterator.Reset();
	 LineIterator.Reset();

	 AllocatePrefetches(WordIterator,LineIterator,WordPrefetches,LinePrefetches,
			    ScheduleBandwidth,PrefetchBandwidth,LineValue,Cycles,
			    RData,NothingFetched,false);
       }
  }

//
//  Function: MakePrefetchStatement
//
//  Input: Node - AST of array reference to prefetch
//         Var - innermost loop induction variable AST
//         distance - iterations ahead to prefetch
//         LowerBound - loop lower bound AST
//
//  Output: a statement that is a call to $$Prefetch
//
//  Description: If LowerBound is AST_NIL, make a prefetch statement that
//               includes the induction variable (the statement is inside the
//               loop) and add the distance as an offset.  If LowerBound is not
//               AST_NIL, then the statement is an offset off of the LowerBound
//               (this will go outside a loop for priming).
//

static AST_INDEX MakePrefetchStmt(AST_INDEX    Node,
				  AST_INDEX    Var,
				  AST_INDEX    Step,
				  int          distance,
				  Boolean      FetchSelfSpatial,
				  Boolean      InsertDependences,
				  AST_INDEX    LowerBound = AST_NIL)

  {
   AST_INDEX PrefetchNode;
   AST_INDEX ArgList;
   char      Text[80],
             Instruction[100];

           /* generate a function call so that pt_var_add will update references
	      to the induction variable.  Later the call is changed to a 
              directive */

     PrefetchNode = tree_copy_with_type(Node);
     if (LowerBound == AST_NIL)
       pt_var_add(PrefetchNode,gen_get_text(Var),distance);
     else{
       pt_var_replace(PrefetchNode,gen_get_text(Var),
		      pt_simplify_expr(pt_gen_add(tree_copy_with_type(LowerBound),
						  pt_gen_int(distance))));
     }
     ArgList = list_create(pt_simplify_expr(PrefetchNode));
     if (InsertDependences)
       {
	 list_insert_last(ArgList,tree_copy_with_type(Node));
	 list_insert_last(ArgList,tree_copy_with_type(Var));
	 if (Step == AST_NIL)
	   Step = pt_gen_int(1);
	 list_insert_last(ArgList,tree_copy_with_type(Step));
       }
     if (FetchSelfSpatial)
       return pt_gen_call("$$PrefetchSS",ArgList);
     else
       return pt_gen_call("$$PrefetchN",ArgList);
  }

//
//  Function: InsertPrefetchesBeforeStmt
//
//  Input: Stmt - statement before which to insert prefetches
//         Prefetches - list of array references to prefetch
//         Var - AST index of innermost loop induction variable
//         distance - number of iterations ahead to prefetch
//         LowerBound - Loop lower bound if prefetches go before loop
//
//  Output: modified AST with calls to $$Prefetch
//
//  Description: If we pass in an actual statement we insert prefetches
//               before that statement.  If we pass in AST_NIL, we insert
//               the prefetches before the statement containing the reference.
//               If LowerBound is not null, then we are inserting before the
//               loop and we base our distance off of the lower bound of the
//               loop.  This is done for "priming" prefetches
//

static void InsertPrefetchesBeforeStmt(AST_INDEX    Stmt,
				       PrefetchList *Prefetches,
				       AST_INDEX    Var,
				       AST_INDEX    Step,
				       int          distance,
				       Boolean      FetchSelfSpatial,
				       Boolean      InsertDependences,
				       AST_INDEX    LowerBound = AST_NIL)

  {
   PrefetchListIterator Iterator(Prefetches);
   PrefetchListEntry    *Node;

     for (Node = Iterator.current();
	  Node != NULL;
	  Node = Iterator.current())
       {
	if (Stmt != AST_NIL)
          list_insert_before(Stmt,MakePrefetchStmt(Node->GetValue(),Var,Step,distance,
						   FetchSelfSpatial,InsertDependences,
						   LowerBound));
	else
         list_insert_before(ut_get_stmt(Node->GetValue()),
			    MakePrefetchStmt(Node->GetValue(),Var,Step,distance,
					     FetchSelfSpatial,InsertDependences));
	++Iterator;
       }
  }

//
//  Function: PipelineIterations
//
//  Input: minimum - minimum number of iterations ahead to prefetch data
//                   based on whether we are prefetching on a word or a line
//                   regardless of latency. word = 1, line = cache-line size
//         iteration - how many iterations it takes to hide a prefetch
//
//  Output:  the number of iterations ahead to prefetch
//
//  Description: If we are prefetching enough iterations ahead for machine
//               to hide latency, then return minimum. Otherwise, we need to
//               prefetch enough multiples of minimum to hide the entire prefetch
//

static AST_INDEX MakeDeadStmt(AST_INDEX    Node,
                                  AST_INDEX    Var,
                                  int          distance,
				  AST_INDEX    LowerBound = AST_NIL)

  {
   AST_INDEX DeadNode;
   char      Text[80],
             Instruction[100];

           /* generate a function call so that pt_var_add will update reference
s
              to the induction variable.  Later the call is changed to a
              directive */

     DeadNode = tree_copy_with_type(Node);
     pt_var_add(DeadNode,gen_get_text(Var),distance);
     return(pt_gen_call("$$Dead",list_create(pt_simplify_expr(DeadNode))));
  }

static void InsertNoReuseDead(AST_INDEX    Stmt,
                                       PrefetchList *NoLocality,
                                       AST_INDEX    Var,
                                       int          distance,
                                       AST_INDEX    LowerBound=AST_NIL)

  {
   PrefetchListIterator Iterator(NoLocality);
   PrefetchListEntry    *Node;

   AST_INDEX name;
   subscript_info_type *sptr;
   int add_distance;

     for (Node = Iterator.current();
          Node != NULL;
          Node = Iterator.current())
       {
        name = gen_SUBSCRIPT_get_name(Node->GetValue());
        sptr = get_subscript_ptr(name);

        if (sptr->GroupSpatialDistance){
           if (Stmt != AST_NIL){
             list_insert_after(Stmt,MakeDeadStmt(Node->GetValue(),Var,
             distance,LowerBound));
           }
           else{
              list_insert_after(ut_get_stmt(Node->GetValue()),
              MakeDeadStmt(Node->GetValue(),Var,0));
           }
        }
        else if (sptr->GroupTemporalDistance){
           add_distance = 0 - sptr->GroupTemporalDistance;
           if (Stmt != AST_NIL){
             list_insert_after(Stmt,MakeDeadStmt(Node->GetValue(),Var,
             add_distance,LowerBound));
           }
           else{
              list_insert_after(ut_get_stmt(Node->GetValue()),
              MakeDeadStmt(Node->GetValue(),Var,add_distance));
           }
        }
        else{
           if (Stmt != AST_NIL){
             list_insert_after(Stmt,MakeDeadStmt(Node->GetValue(),Var,
             distance,LowerBound));
           }
           else{
              list_insert_after(ut_get_stmt(Node->GetValue()),
              MakeDeadStmt(Node->GetValue(),Var,0));
           }
        }
        ++Iterator;
       }
  }

static void InsertSSDead(AST_INDEX    Stmt,
                                       PrefetchList *NoLocality,
                                       AST_INDEX    Var,
                                       int          distance,
                                       AST_INDEX    LowerBound=AST_NIL)

{
   PrefetchListIterator Iterator(NoLocality);
   PrefetchListEntry    *Node;

   AST_INDEX name;
   subscript_info_type *sptr;
   int add_distance;

     for (Node = Iterator.current();
          Node != NULL;
          Node = Iterator.current()){

        name = gen_SUBSCRIPT_get_name(Node->GetValue());
        sptr = get_subscript_ptr(name);

        if (!sptr->GroupTemporalDistance){
           if (Stmt != AST_NIL){
             list_insert_after(Stmt,MakeDeadStmt(Node->GetValue(),Var,
             distance,LowerBound));
           }
           else{
              list_insert_after(ut_get_stmt(Node->GetValue()),
              MakeDeadStmt(Node->GetValue(),Var,distance));
           }
        }
        else{
           add_distance = distance - sptr->GroupTemporalDistance;
           if (Stmt != AST_NIL){
             list_insert_after(Stmt,MakeDeadStmt(Node->GetValue(),Var,
             add_distance,LowerBound));
           }
           else{
              list_insert_after(ut_get_stmt(Node->GetValue()),
              MakeDeadStmt(Node->GetValue(),Var,add_distance));
           }
       }
       ++Iterator;
    }
}

static void InsertTempLocalDead(AST_INDEX    Stmt,
                                       PrefetchList *TempLocality,
                                       AST_INDEX    Var,
                                       int          distance)

  {
   PrefetchListIterator Iterator(TempLocality);
   PrefetchListEntry    *Node;

   AST_INDEX name;
   subscript_info_type *sptr;
     for (Node = Iterator.current();
          Node != NULL;
          Node = Iterator.current())
       {
        if (Stmt != AST_NIL){
          list_insert_after(Stmt,MakeDeadStmt(Node->GetValue(),Var,
          0));
        }
        else{
          name = gen_SUBSCRIPT_get_name((Node->GetValue()));
          sptr = get_subscript_ptr(name);
          list_insert_after(sptr->surround_node,
          MakeDeadStmt(Node->GetValue(),Var,0));
        }
        ++Iterator;
       }
  }

static int PipelineIterations(int minimum,
			      int iteration)

  {
    return minimum >= iteration ? minimum : minimum * ceil_ab(iteration,minimum);
  }


static int set_type(AST_INDEX node,
		    Generic   dummy)
 {   
   gen_put_real_type(node,TYPE_INTEGER);   
   gen_put_converted_type(node,TYPE_INTEGER);   
   return(WALK_CONTINUE);
 }

static AST_INDEX make_integer_type(AST_INDEX expr)

 {   
   walk_expression(expr,(WK_EXPR_CLBACK)set_type,(WK_EXPR_CLBACK)NOFUNC,(Generic)NULL);
   return expr;
 }

//  Function: CreatePreLoop
//
//  Input: loop_data - tree structure of loop nest
//         loop - index into loop_data of loop being pre-conditioned
//         UnrollVal - amount by which loop is unrolled
//         Symtab - symbol table
//
//  Output: pre-conditioned loop
//
//  Description: create a pre-loop to make sure loop executes the right number
//               of times when unrolled.  Pre-loop will execute enough iteration
//               to make make loop execute a multiple of the unroll amount.
//

static AST_INDEX CreatePreLoop(model_loop    *loop_data,
			       int           loop,
			       int           UnrollVal,
			       SymDescriptor Symtab)


  {
    AST_INDEX control,         // control AST for do loop
      new_loop = AST_NIL,      // AST for pre loop
      lwb,                     // lower bound AST of loop
      upb,                     // upper bound AST of loop
      step;                    // step size AST of loop
    Boolean need_pre_loop = false;
    int lwb_v,                 // lower bound of loop
      upb_v,                   // upper bound of loop
      step_v = 0;              // step size of loop

     control = gen_DO_get_control(loop_data[loop].node);
     lwb = gen_INDUCTIVE_get_rvalue1(control);

     step = gen_INDUCTIVE_get_rvalue3(control);
     if (step == AST_NIL)
       step_v = 1;

     // if step symbolic we need a pre loop

     else if (pt_eval(step,&step_v))
        need_pre_loop = true;

     // if lower bound symbolic we need a pre loop

     if (pt_eval(lwb,&lwb_v))
       need_pre_loop = true;
     upb = gen_INDUCTIVE_get_rvalue2(control);

	// if upper bound symbolic we need a pre loop

     if (pt_eval(upb,&upb_v))
       need_pre_loop = true;
     else if (mod((upb_v - lwb_v + 1)/step_v, UnrollVal + 1)
	           != 0)

       // if loop not executed a multiple of unroll value then need pre loop
       
       need_pre_loop = true;
     if (need_pre_loop)
       {

	 if (UnrollVal < 3 && step_v == 1)
	   {
	     AST_INDEX PeeledIteration;
	     AST_INDEX IndexVal;
	     AST_INDEX TestVal;
	     AST_INDEX TestExpr;
	     AST_INDEX OffsetVar = pt_gen_ident("offset$$");
	     char      *Index = gen_get_text(gen_INDUCTIVE_get_name(control));
	     AST_INDEX Mod = 
	       make_integer_type(pt_gen_mod(pt_simplify_expr(pt_gen_add(
						pt_gen_sub(tree_copy_with_type(upb),
							   tree_copy_with_type(lwb)),
							 pt_gen_int(1))),
					    pt_simplify_expr(pt_gen_int(UnrollVal+1))));

	     list_insert_before(loop_data[loop].node,
				gen_ASSIGNMENT(AST_NIL,OffsetVar,Mod));
	     for (int i = 0; i < UnrollVal; i++)
	       {
		 IndexVal = pt_simplify_expr(pt_gen_add(tree_copy_with_type(lwb),
			 				pt_gen_int(i)));
		 PeeledIteration = 
		   tree_copy_with_type(gen_DO_get_stmt_LIST(loop_data[loop].node));
		 ut_update_labels(PeeledIteration,Symtab);
		 pt_var_replace(PeeledIteration,Index,IndexVal);
		 TestVal = pt_gen_int(i+1);
		 TestExpr = gen_BINARY_GE(tree_copy_with_type(OffsetVar),
					  tree_copy_with_type(TestVal));
		 list_insert_before(loop_data[loop].node,
				    gen_IF(AST_NIL,AST_NIL,
					   list_create(gen_GUARD(AST_NIL,TestExpr,
								 PeeledIteration))));
	       }

	     gen_INDUCTIVE_put_rvalue1(control,
				       pt_gen_add(tree_copy_with_type(lwb),
						  tree_copy_with_type(OffsetVar)));
	     gen_INDUCTIVE_put_rvalue3(control,pt_gen_int(UnrollVal+1));
	   }
	 else
	   {
	     // create pre loop
	     
	     new_loop = tree_copy_with_type(loop_data[loop].node);
	     
	     // change duplicate labels
	     
	     ut_update_labels(new_loop,Symtab);
	     
	     // update loop bounds for pre-loop and loop
	     
	     ut_update_bounds(loop_data[loop].node,new_loop,UnrollVal);
	     
	     // put pre loop before current loop
	     
	     list_insert_before(loop_data[loop].node,new_loop);
	   }
       }
     else 

       // update step size of loop

       ut_update_bounds(loop_data[loop].node,AST_NIL,UnrollVal);
     return(new_loop);
  }
  
//
//  Function: ReplicateBody
//
//  Input: DoNode - Do statement AST of loop being unrolled
//         UnrollVal - how many copies of loop to make
//         Ivar - loop induction variable AST
//         Symtab - symbol table
//         ar - Arena for memory allocation
//
//  Output: A loop AST with a replicated loop body
//
//  Description: Replicate the AST of a loop body by copying the AST and
//               updating the statement labels.
//

static void ReplicateBody(AST_INDEX DoNode,
			  int       UnrollVal,
			  AST_INDEX Ivar,
			  SymDescriptor Symtab,
			  arena_type    *ar)
  
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
    AST_INDEX *NewCode, // replicated loop body
      Step,             // AST of loop Step size
      StmtList;         // statement list for loop
    int        i,       // counter for loop replication
      StepVal;          // value of loop step size
   
     StmtList = gen_DO_get_stmt_LIST(DoNode);

     // Get loop step size

     Step = gen_INDUCTIVE_get_rvalue3(gen_DO_get_control(DoNode));
     if (Step == AST_NIL)
       StepVal = 1;
     else 
       {
	(void)pt_eval(Step,&StepVal);
	StepVal /= (UnrollVal+1);
       }
     
     // create structure for each replicated loop body

     NewCode = (AST_INDEX *)ar->arena_alloc_mem(LOOP_ARENA,
						UnrollVal * sizeof(AST_INDEX));

     // Replicate loop body

     for (i = 0; i < UnrollVal-1; i++)
       {
	NewCode[i] = tree_copy_with_type(StmtList);

	// update loop induction variable in replicated body

	pt_var_add(NewCode[i],gen_get_text(Ivar),StepVal*(i+1));

	// modify any labels so there are not duplicates

	ut_update_labels(NewCode[i],Symtab);
       }
     NewCode[UnrollVal-1] = tree_copy_with_type(StmtList);
     
     // update loop induction variable in replicated body

     pt_var_add(NewCode[UnrollVal-1],gen_get_text(Ivar),StepVal*UnrollVal);

	// modify any labels so there are not duplicates

     ut_update_labels(StmtList,Symtab);

     // append new loop bodies onto existing loop bodies

     for (i = 0; i < UnrollVal; i++)
       StmtList = list_append(StmtList,NewCode[i]);
  }
	
//
//  Function: UnrollLoop
//
//  Input: loop_data - tree structure of loop nest
//         loop - index into loop_data for loop being stripped
//         WordPretches - list of array references to be prefetched every iteration
//         WordPrefetchDistance - number of iteration ahead to prefetch word prefetches
//         LinePrefetches - list of array references to be prefetched once
//                          every cache-line iterations
//         LineDistance - number of iterations ahead to prefetch line prefetches
//         UnrollVal - amount by which to unroll loop
//         Var - induction variable of loop
//         symtab - symbol table
//         ar - Arena for memory allocation
//
//
//  Output: unrolled loop with prefetches inserted
//
//  Description: Unroll a loop to accomodate line prefetches.  Create a pre loop
//               to get right number of iterations.  
//

static void UnrollLoop(model_loop   *loop_data,
		       int           loop,
		       PrefetchList *WordPrefetches,
		       int           WordPrefetchDistance,
		       PrefetchList *LinePrefetches,
		       PrefetchList *NoLocality,
		       PrefetchList *SpatLocality,
		       int           LineDistance,
		       int           UnrollVal,
		       AST_INDEX     Var,
		       SymDescriptor Symtab,
		       arena_type    *ar)
  {
    AST_INDEX PreLoop;  // AST of pre-conditioning loop
    int       i;        // counter for inserting prefetches that need to be done
                        // before loop begins

    AST_INDEX Step = gen_INDUCTIVE_get_rvalue3(gen_DO_get_control(loop_data[loop].node));

    // copy loop to get pre-conditioning loop

     PreLoop = CreatePreLoop(loop_data,loop,UnrollVal-1,Symtab);

     // unroll the original loop

     ReplicateBody(loop_data[loop].node,UnrollVal-1,Var,Symtab,ar);

     // insert line prefetches at beginning of unrolled loop

     InsertPrefetchesBeforeStmt(list_first(gen_DO_get_stmt_LIST(loop_data[loop].node)),
				LinePrefetches,Var,Step,LineDistance*2,true,true);

     if (Memoria_IssueDead)
       InsertSSDead(list_first(gen_DO_get_stmt_LIST(loop_data[loop].node)),
		    SpatLocality,Var,(0-UnrollVal));

     // insert "priming" prefetches before pre-loop begins.  These are the things that
     // need to be prefetched but aren't in loop do to distance offset

//      if (NOT(Memoria_LetRocketSchedulePrefetches))
//        { 
//         if (PreLoop == AST_NIL)
//           PreLoop = loop_data[loop].node;
//         for (i = 0; i < WordPrefetchDistance; i++)
//           InsertPrefetchesBeforeStmt(PreLoop,WordPrefetches,Var,
// 				     gen_INDUCTIVE_get_rvalue3(gen_DO_get_control(PreLoop)),
// 				     i,false,false,
// 				     gen_INDUCTIVE_get_rvalue1(gen_DO_get_control(PreLoop)));
       
//         // insert "priming" prefetches for line prefetches.  Prefetch all lines before
//         // the first prefetch in the pre-conditioning loop.

//         for (i = 0; i < 2*LineDistance; i+= UnrollVal)
//           InsertPrefetchesBeforeStmt(PreLoop,LinePrefetches,Var,
// 				     gen_INDUCTIVE_get_rvalue3(gen_DO_get_control(PreLoop)),
// 				     i,true,false,
// 				     gen_INDUCTIVE_get_rvalue1(gen_DO_get_control(PreLoop)));
//       }
      
  }


//
//  Function: StripLoop
//
//  Input: loop_data - tree structure of loop nest
//         loop - index into loop_data for loop being stripped
//         LinePrefetches - list of array references to be prefetched once
//                          every cache-line iterations
//         LineDistance - number of iterations ahead to prefetch
//         StripVal - amount by which to strip loop
//         Var - induction variable of loop
//         symtab - symbol table
//         ar - Arena for memory allocation
//
//  Output: stripped loop with line prefetches inserted
//
//  Description: Strip mine a loop and insert line prefetches before the new
//               loop.  This will ensure that line prefetches are only executed
//               once every cache-line iterations.
//

static void StripLoop(model_loop   *loop_data,
		      int           loop,
		      PrefetchList *LinePrefetches,
		      int           LineDistance,
		      int           StripVal,
		      AST_INDEX     Var,
		      SymDescriptor Symtab,
		      arena_type    *ar)
  {
    AST_INDEX NewLoop,          // new loop that iterates through strip
      Ctrl,                     // control AST of loop
      Ivar,                     // induction variable AST of loop
      StmtList,                 // list of statements in loop
      ArgList,                  // argument list to MIN function
      Min,                      // AST for MIN function
      Step;                     // Step value AST for loop
    int       StepVal;          // Step value
    char      NewIvar[80];      // Text for new induction variable

     StmtList = gen_DO_get_stmt_LIST(loop_data[loop].node);
     gen_DO_put_stmt_LIST(loop_data[loop].node,AST_NIL);
     Ctrl = gen_DO_get_control(loop_data[loop].node);
     Ivar = gen_INDUCTIVE_get_name(Ctrl);

     // generate new induction variable for through-strip loop

     sprintf(NewIvar,"%s$%d",gen_get_text(Ivar),loop_data[loop].level);

     // get step value for old loop

     Step = gen_INDUCTIVE_get_rvalue3(Ctrl);
     if (Step == AST_NIL)
       StepVal = 1;
     else 
       (void)pt_eval(Step,&StepVal);

     // Create Min Function for upper bound of through-strip loop

     ArgList = list_create(pt_gen_add(tree_copy_with_type(Ivar),
				      pt_gen_int((StripVal-1)*StepVal)));
     ArgList = list_insert_last(ArgList,
				tree_copy_with_type(gen_INDUCTIVE_get_rvalue2(Ctrl)));
     Min = gen_INVOCATION(pt_gen_ident("min"),ArgList);

     // generate AST for new loop

     NewLoop = gen_DO(AST_NIL,AST_NIL,AST_NIL,
		      gen_INDUCTIVE(pt_gen_ident(NewIvar),tree_copy_with_type(Ivar),
				    Min,tree_copy_with_type(Step)),
		      StmtList);

     // update step value for by-strip loop

     if (Step == AST_NIL)
       gen_INDUCTIVE_put_rvalue3(Ctrl,pt_gen_int(StripVal*StepVal));
     else
       pt_tree_replace(gen_INDUCTIVE_get_rvalue3(Ctrl),
		       pt_gen_int(StripVal*StepVal));


     // make the through-strip loop the loop body of by-strip loop

     gen_DO_put_stmt_LIST(loop_data[loop].node,list_create(NewLoop));

     // insert line prefetches before through-strip loop
     InsertPrefetchesBeforeStmt(NewLoop,LinePrefetches,Var,Step,LineDistance*2,true,
				true);

     // update induction variable for through-strip loop
     pt_var_replace(StmtList,gen_get_text(Ivar),pt_gen_ident(NewIvar));
  }

//
//  Function: SchedulePrefetches
//
//  Input: loop_data - tree structure of loop nest
//         loop - index into loop_data for loop being scheduled
//         LinePrefetches - list of array references to be prefetched once
//                          every cache-line iterations
//         WordPrefetches - list of array references to be prefetched each iteration
//         ped - dependence graph and configuration info
//         symtab - symbol table
//         ar - Arena for memory allocation
//
//  Output: AST for loop with software prefetches inserted
//
//  Description: Software pipeline the software prefetches. Determine how far in
//               advance prefetches must be scheduled to ensure they have finished.
//               Then, schedule them the appropriate number of loop iteration in
//               advance.
//

static void SchedulePrefetches(model_loop *loop_data,
			       int        loop,
			       PrefetchList *LinePrefetches,
			       PrefetchList *WordPrefetches,
			       PrefetchList *NoLocality,
			       PrefetchList *TempLocality,
			       PrefetchList *SpatLocality,
			       PedInfo      ped,
			       SymDescriptor symtab,
			       arena_type    *ar)
  {
    AST_INDEX var;                  // AST for loop induction variable
    AST_INDEX Step;
    int      LinePrefetchDistance,  // Number of iterations ahead to prefetch lines
      WordPrefetchDistance,         // Number of iterations ahead to prefetch words 
      MaxDistance,                  // Maximum number of iterations ahead to prefetch
      IterationDist,                // Iterations ahead to prefetch based upon latency
                                    // and loop cycles
      Cycles,                       // Number of cycles for loop to finish
      UnrollVal;                    // amount to unroll loop due to line prefetches


    if (Memoria_LetRocketSchedulePrefetches)
      {

	// Rocket can schedule better than we can, so just tell it what to prefetch
	// and that will be enough

	UnrollVal = (LinePrefetches->NullList()) ? 0 :
	  (((config_type *)PED_MH_CONFIG(ped))->line >> 3);
	LinePrefetchDistance = UnrollVal >> 1;
	WordPrefetchDistance = 0;
	MaxDistance = (LinePrefetches->NullList() &&
		       WordPrefetches->NullList()) ? 0 : 1;
      }
    else
      {
	// determine how long for loop to finish

	Cycles = CyclesPerIteration(loop_data[loop].node,ped);

	// determine number of iterations ahead we must prefetch to cover latency of
	// prefetch with loop cycles

	if (!Cycles) Cycles = 1;
	IterationDist = ceil_ab(((config_type *)PED_MH_CONFIG(ped))->
				prefetch_latency,Cycles);

	// determine how far in advance word prefetches must be issued

	WordPrefetchDistance = (NOT(WordPrefetches->NullList())) ?
	  PipelineIterations(1,IterationDist) : 0;
     
	// determine how far in advance line prefetches must be issued
	// The line size is in bytes and we are prefetching 8-byte words (dp).

	LinePrefetchDistance = (NOT(LinePrefetches->NullList()))?
	  PipelineIterations(((config_type *)PED_MH_CONFIG(ped))->line>>3,
			     IterationDist):0;
	
	MaxDistance = MAX(LinePrefetchDistance,WordPrefetchDistance);
	
	// Assumes Double Precision (which is all Rocket supports anyway)
	// Comput the unroll value as 0 if no line prefetching is done
	// otherwise make it the number of double precision words that fit in a cache line
	
	UnrollVal = (LinePrefetchDistance == 0) ? 0 : 
	  (((config_type *)PED_MH_CONFIG(ped))->line >> 3);
      }
	
     var = gen_INDUCTIVE_get_name(gen_DO_get_control(loop_data[loop].node));
     Step = gen_INDUCTIVE_get_rvalue3(gen_DO_get_control(loop_data[loop].node));

     if (MaxDistance != 0){

	// Insert word prefetches before appropriate statements now
	// They will be copied during unrolling.

	InsertPrefetchesBeforeStmt(AST_NIL,WordPrefetches,var,Step,WordPrefetchDistance,
				   false,true);

	if (NOT(LinePrefetches->NullList())){
	  
	    if (Memoria_IssueDead)
              InsertNoReuseDead(AST_NIL,NoLocality,var,0);
	    assert(UnrollVal != 0);

	    // Unroll loop to accomodate prefetching once per cache-line iterations
	    // If unroll value is too high, then perform strip mining instead
 
	    if (UnrollVal <= 32)
	      UnrollLoop(loop_data,loop,WordPrefetches,WordPrefetchDistance,
			 LinePrefetches,NoLocality,SpatLocality,
                         LinePrefetchDistance,UnrollVal,var,symtab,ar);
	    else
	      StripLoop(loop_data,loop,LinePrefetches,LinePrefetchDistance,
			UnrollVal,var,symtab,ar);
        }
        else
	  if (Memoria_IssueDead)
	    InsertNoReuseDead(AST_NIL,NoLocality,var,0);
     }
     if (Memoria_IssueDead)
       InsertTempLocalDead(AST_NIL,TempLocality,var,0);
  }


static char *BuildDependenceDirective(AST_INDEX ArrayRef,
				      char *Ivar,
				      int StepVal,
				      int Offset)

{
  char *SubText;

  SubText = new char[72];
  AST_INDEX node = tree_copy_with_type(ArrayRef);
  pt_var_add(node,Ivar,StepVal*Offset);
  node = pt_simplify_expr(node);
  ut_GetSubscriptText(node,SubText);
  return SubText;
}
  
static void AddDependenceDirectives(AST_INDEX OriginalRef,
				    int       PrefetchNumber,
				    char      *IVar,
				    int       StepVal,
				    int       Offset,
				    Boolean   IsSelfSpatialFetch,
				    int       LineLength,
				    AST_INDEX Directive)
{
  char Instruction[72];

  sprintf(Instruction,"$directive dep %d, %s",PrefetchNumber,
	  BuildDependenceDirective(OriginalRef,IVar,StepVal,0));

  AST_INDEX NewDirective = pt_gen_comment(Instruction);
  list_insert_after(Directive,NewDirective);
  Directive = NewDirective;

  if (IsSelfSpatialFetch)
    for (int i = 1; i < LineLength; i++)
      {
	sprintf(Instruction,"$directive dep %d, %s",PrefetchNumber,
		BuildDependenceDirective(OriginalRef,IVar,StepVal,i));
	NewDirective = pt_gen_comment(Instruction);
	list_insert_after(Directive,NewDirective);
	Directive = NewDirective;
      }
}
			      
//
//  Function: ConvertPrefetchCallsToDirectives
//
//  Input: stmt - a statement in a loop
//         level - the nesting level of the statement
//         dummy -  a bogus parameter need by walk_statements
//
//  Output: potentially modified AST
//
//  Description: For each call to $$Prefetch function, convert statement to
//               to a comment with a directive
//

static int ConvertPrefetchCallsToDirectives(AST_INDEX stmt,
					    int       level,
					    PrefetchInfoType *PrefetchInfo)

  {
    char Text[80],            // text for array subscript
      Instruction[100];       // text for prefetch directive
    AST_INDEX Inv,            // AST for invocation
             Comment;         // AST for prefetch directive
    static int PrefetchNumber = 0; // used to uniquely identify each prefetch 
                                   // instruction 


     if (is_call(stmt))
       {
	Inv = gen_CALL_get_invocation(stmt);
	char *FunctionName = gen_get_text(gen_INVOCATION_get_name(Inv)); 
        int SelfSpatialFetch;

	// Determine if this is a call to $$Prefetch

	if ((SelfSpatialFetch = strcmp("$$PrefetchSS",FunctionName)) == 0 ||
	    strcmp("$$PrefetchN",FunctionName) == 0)
	  {

	    int LineLength = ((config_type*)PED_MH_CONFIG(PrefetchInfo->ped))->line >> 3;
	    // Get Text representation of subscript AST
	    
	    AST_INDEX ArrayRef = list_first(gen_INVOCATION_get_actual_arg_LIST(Inv));
	    AST_INDEX IVar;
	    AST_INDEX Step;
	    AST_INDEX OriginalRef = list_next(ArrayRef);

	    if (OriginalRef != AST_NIL)
	      {
		IVar = list_next(OriginalRef);
		Step = list_next(IVar);
	      }

	    ut_GetSubscriptText(ArrayRef,Text,PrefetchInfo->symtab);
	   
	    // Create comment containing the directive
 
	    sprintf(Instruction,"$directive prefetch %d, %s",PrefetchNumber,Text);
	    
	    // Generate comment and put in AST where stmt used to be

	    Comment = pt_gen_comment(Instruction);
	    pt_tree_replace(stmt,Comment);
	    
	    // Now add dependence directives

	    if (OriginalRef != AST_NIL)
	      {
		int StepVal;

		pt_eval(Step,&StepVal);
		AddDependenceDirectives(OriginalRef,PrefetchNumber,gen_get_text(IVar),
					StepVal,0,BOOL(SelfSpatialFetch == 0),
					LineLength,Comment);
	      }

	    PrefetchNumber++;

	    return(WALK_FROM_OLD_NEXT);
	  }
        if (strcmp("$$Dead",gen_get_text(gen_INVOCATION_get_name(Inv))) == 0)
          {
            ut_GetSubscriptText(list_first(gen_INVOCATION_get_actual_arg_LIST(Inv)),
                                Text);
            sprintf(Instruction,"$directive dead (%s)",Text);
            Comment = pt_gen_comment(Instruction);
            pt_tree_replace(stmt,Comment);
            return(WALK_FROM_OLD_NEXT);
          }
       }
     return(WALK_CONTINUE);
  }

//
//  Function: walk_loops
//
//  Input: loop_data - tree structure of loop nest
//         loop - index into loop_data for current loop
//         ped - dependendence graph and configuration info 
//         symtab - symbol table
//         ar - Arena for memory allocation
//         IVar - induction variable for current loop
//
//  Output: modified loops after software prefetching
//
//  Description: walk the tree structure for a loop nest.  For each innermost loop
//               apply software prefetching.
//

static void walk_loops(model_loop    *loop_data,
		       int           loop,
		       PedInfo       ped,
		       SymDescriptor symtab,
		       arena_type    *ar,
		       char          **IVar)
		       
  {
    int next;  // the next loop at to examine in loop_data
    PrefetchList LinePrefetches; // List of array references that must be prefetched
                                // every cache-line iterations.
    PrefetchList WordPrefetches; // List of array references that must be prefetched
                                // every iteration.
    PrefetchList NoLocality;
    PrefetchList TempLocality;
    PrefetchList SpatLocality;


   // Get the induction variable of the current loop

     IVar[loop_data[loop].level-1] = gen_get_text(gen_INDUCTIVE_get_name(
					  gen_DO_get_control(loop_data[loop].node)));

     // If this is an innermost loop the perform software prefetching

     if (loop_data[loop].inner_loop == -1)
       {

	 // Examine array references to determine which ones need to be prefetched 
	 // ever iteration and which need to be prefetched every cache-line iterations

	CheckRefsForPrefetch(loop_data,loop,&LinePrefetches,&WordPrefetches,
        &NoLocality,&TempLocality,&SpatLocality,ped,IVar);

	// If the architecture stalls on prefetch buffer overflow, we want to 
	// limit the prefetching done to keep that from happening

	if (!((config_type *)PED_MH_CONFIG(ped))->aggressive)
	  ModeratePrefetchRequirements(loop_data,loop,&LinePrefetches,
          &WordPrefetches,ped);

	// Software pipeline the prefetches
	SchedulePrefetches(loop_data,loop,&LinePrefetches,&WordPrefetches,
        &NoLocality,&TempLocality,&SpatLocality,ped,symtab,ar);

	// Make prefetches into comments with directives

	PrefetchInfoType PrefetchInfo;
	
	PrefetchInfo.ped = ped;
	PrefetchInfo.symtab = symtab; 

	walk_statements(tree_out(loop_data[loop].node),loop_data[loop].level,NOFUNC,
			(WK_STMT_CLBACK)ConvertPrefetchCallsToDirectives,
			(Generic)&PrefetchInfo);
       }

     // If this is not an innermost loop, look at all loops at the next level

     else
       {
	for (next = loop_data[loop].inner_loop;
	     next != -1;
	     next = loop_data[next].next_loop)
	  walk_loops(loop_data,next,ped,symtab,ar,IVar);
       }
  }

//
//  Function:  memory_software_prefetch
//
//  Input:   ped - Structure holding dependence graph and configuration info
//           root - AST_INDEX of an outermost loop
//           level - the level of the outermost loop
//           symtab - Symbol Table
//           ar - Arena for memory allocation
//
//  Output:  A loop structure with prefetches inserted in each innermost loop
//           via the algorithm of Mowry, et al.
//
//  Description: Driver for the algorithm
//


void memory_software_prefetch(PedInfo       ped,
			      AST_INDEX     root,
			      int           level,
			      SymDescriptor symtab,
			      arena_type    *ar)

  {
    pre_info_type pre_info;      // structure for initializing subscript_ptr
    model_loop    *loop_data;    // tree structure for loop nest
    char          **IVar;        // Induction Variable of loop


   //  Initial structure need to record surrounding do information

     pre_info.stmt_num = 0;
     pre_info.loop_num = 0;
     pre_info.surrounding_do = -1;
     pre_info.abort = false;
     pre_info.ped = ped;
     pre_info.symtab = symtab;
     pre_info.ar = ar;

     // Walk loop and set the surrounding do field in the subscript_ptr
     // field of the "name" of subscript nodes.  This also initializes the
     // subscript_ptr

     walk_statements(root,level,(WK_STMT_CLBACK)ut_mark_do_pre,
		     (WK_STMT_CLBACK)ut_mark_do_post,(Generic)&pre_info);

     //  If loop contains control flow or funciton calls, abort algorithm

     if (pre_info.abort)
       return;

     //  Remove dependence edges that are not needed

     walk_statements(root,level,(WK_STMT_CLBACK)remove_edges,NOFUNC,(Generic)ped);

     // allocate structure for loop information

     loop_data = (model_loop *)ar->arena_alloc_mem_clear(LOOP_ARENA,
					 pre_info.loop_num*sizeof(model_loop));

     // set up model_loop structure (tree) for this loop nest

     ut_analyze_loop(root,loop_data,level,ped,symtab);

     // determine if loop nest is rectangular, triangular, etc.

     ut_check_shape(loop_data,0);

     // walk the loop structure, performing software prefetching on each innermost
     // loop.

     IVar = new char*[pre_info.loop_num];
     walk_loops(loop_data,0,ped,symtab,ar,IVar);

     delete IVar;
  }
