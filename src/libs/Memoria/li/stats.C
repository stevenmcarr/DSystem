/* $Id: stats.C,v 1.9 1994/07/20 11:32:08 carr Exp $ */

/****************************************************************/
/*                                                              */
/*   File:   stats.C                                            */
/*                                                              */
/*   Description: Compute locality and permutation stats        */
/*                for a loop nest.                              */
/*                                                              */
/*                                                              */
/****************************************************************/

#ifndef general_h
#include <general.h>
#endif 

#include <mh.h>
#include <mh_ast.h>
#include <fort/walk.h>
#include <pt_util.h>
#include <mh_config.h>
#include <MemoryOrder.h>

#ifndef header_h
#include <header.h>
#endif

#ifndef dt_h
#include <dt.h>
#endif

#ifndef gi_h
#include <fort/gi.h>
#endif 

#ifndef stats_h
#include <stats.h>
#endif 

#ifndef analyze_h
#include <analyze.h>
#endif 

#ifndef shape_h
#include <shape.h>
#endif 

#ifndef mem_util_h
#include <mem_util.h>
#endif 

#ifndef mark_h
#include <mark.h>
#endif 

#ifndef LoopStats_h
#include <LoopStats.h>
#endif 

#ifndef dg_h
#include	<dg.h>		/* dg_add_edge()		*/
#endif 


/****************************************************************/
/*                                                              */
/*   Function:   GetLoopCost                                    */
/*                                                              */
/*   Input:      TemporalCost - cost associated with temporal   */
/*                              reuse.                          */
/*               SpatialCost - cost associated with spatial     */
/*                             reuse.                           */
/*               OtherSpatialCost - cost associated with        */
/*                                  group-spatial reuse.        */
/*               InvariantCost - cost associated with           */
/*                               self-temporal reuse.           */
/*               NoneCost - cost associated with no reuse       */
/*                                                              */
/*   Description:  Compute total loop cost given the costs for  */
/*                 each ref group based on its locality type.   */
/*                                                              */
/****************************************************************/

static int GetLoopCost(float TemporalCost,
		       float SpatialCost,
                       float OtherSpatialCost,
		       float InvariantCost,
		       float NoneCost)

  {
     return((int)((TemporalCost     * 100.0 +
		   SpatialCost      * 100.0 +
		   OtherSpatialCost * 100.0 +
		   NoneCost         * 100.0 +
		   InvariantCost) * 10.0));
  }


/****************************************************************/
/*                                                              */
/*   Function: GetLoopCostFirst                                 */
/*                                                              */
/*   Input:    loop_data - loop structure                       */
/*             j - index into loop structure                    */
/*                                                              */
/*   Description:  To handle non-perfectly nested loops there   */
/*                 is a list of costs associated with each      */
/*                 possible perfect nest.  This function returns*/
/*                 the cost of the first loop on the list.      */
/*                                                              */
/****************************************************************/

static int GetLoopCostFirst(model_loop *loop_data,
			   int        j)

  {
   return
     GetLoopCost(loop_data[j].TemporalCostList.first_entry()->GetValue(),
		 loop_data[j].SpatialCostList.first_entry()->GetValue(),
		 loop_data[j].OtherSpatialCostList.first_entry()->GetValue(),
		 loop_data[j].InvariantCostList.first_entry()->GetValue(),
		 loop_data[j].NoneCostList.first_entry()->GetValue());
  }


/****************************************************************/
/*                                                              */
/*   Function:   DumpLoopStats                                  */
/*                                                              */
/*   Input:      loop_data - loop structure                     */
/*               loop - loop for which stats are printed        */
/*               GroupList - list of RefGroups for loop in a    */
/*                           particular perfect nest            */
/*               LoopStats - permutation statistics             */
/*               logfile - output file                          */
/*                                                              */
/*   Description: print out RefGroup statistics for a loop      */
/*                                                              */
/****************************************************************/

static void DumpLoopStats(model_loop *loop_data,
			  int        loop,
			  int        num_loops,
			  UtilList   *GroupList,
			  LoopStatsType *LoopStats,
			  FILE       *logfile,
			  AST_INDEX  InnerLoop,
			  PedInfo    ped)
  {
   int i;
   UtilNode *GroupNode;
   RefGroupType *RefGroup;
   StatsInfoType BalanceStats;

     fprintf(logfile,"Statistics for Loop at Level %d\n",
	     loop_data[loop].level);
     fprintf(logfile,"Induction variable %s\n",
	     gen_get_text(gen_INDUCTIVE_get_name(gen_DO_get_control(
			 loop_data[loop].node))));
     if (loop_data[loop].reversed)
       {
	fprintf(logfile,"Loop was reversed\n\n");
	LoopStats->Reversed++;
       }
     i = 0;
     for (GroupNode = UTIL_HEAD(GroupList);
	  GroupNode != NULLNODE;
	  GroupNode = UTIL_NEXT(GroupNode))
       {
	i++;
	RefGroup = (RefGroupType *)UTIL_NODE_ATOM(GroupNode);
	fprintf(logfile,"\t Ref Group %d:\n",i);
	fprintf(logfile,"\t    Elements: %d\n",RefGroup->number);
	fprintf(logfile,"\t    Locality:");
	if (RefGroup->Invariant)
	  fprintf(logfile," Invariant");
	if (RefGroup->Spatial)
	  fprintf(logfile," Spatial");
	if (RefGroup->OtherSpatial)
	  fprintf(logfile," OtherSpatial");
	if (RefGroup->Temporal)
	  fprintf(logfile," Temporal");
	if (NOT(RefGroup->Invariant) && NOT(RefGroup->Spatial) &&
	    NOT(RefGroup->OtherSpatial) && NOT(RefGroup->Temporal))
	  fprintf(logfile," None");
	fprintf(logfile,"\n\n");
       }
     fprintf(logfile,"\n\n\tNumber of Ref Groups: %d\n",i);
     fprintf(logfile,"\tLoop Cost %d\n\n\n",GetLoopCostFirst(loop_data,loop));
     BalanceStats.ped = ped;
     BalanceStats.flops = 0.0;
     BalanceStats.mops = 0.0;
     BalanceStats.level = loop_data[loop].level;
     BalanceStats.UseCache = true;
     BalanceStats.loop_data = loop_data;
     BalanceStats.loop = loop;
     walk_expression(InnerLoop,(WK_EXPR_CLBACK)ut_ComputeBalance,NOFUNC,
		     (Generic)&BalanceStats);
     loop_data[loop].fbalance = ((float)BalanceStats.mops) /
                                ((float)BalanceStats.flops);
   
  }


/****************************************************************/
/*                                                              */
/*   Function:   GetLocalityStats                               */
/*                                                              */
/*   Input:      LoopStatsLocalityMatrix - locality stats for   */
/*                                         entire loop nest     */
/*               LocalityMatrix - locality stats for one perfect*/
/*                                nest                          */
/*               OtherSpatialGroups - # of group-spatial RefGrps*/
/*               GroupList - list of RefGroups for a loop in a  */
/*                           particular perfect nest            */
/*                                                              */
/*   Description: Accumulate the locality properties of the    */
/*                RefGroups for a particular loop.              */
/*                                                              */
/****************************************************************/

static void GetLocalityStats(LocalityMatrixType *LoopStatsLocalityMatrix,
			     LocalityMatrixType *LocalityMatrix,
			     int                *OtherSpatialGroups,
			     UtilList           *GroupList)

  {
   UtilNode *GroupNode;
   RefGroupType *RefGroup;

     for (GroupNode = UTIL_HEAD((UtilList *)UTIL_NODE_ATOM(
                                 UTIL_HEAD(GroupList)));
	  GroupNode != NULLNODE;
	  GroupNode = UTIL_NEXT(GroupNode))
       {
	RefGroup = (RefGroupType *)UTIL_NODE_ATOM(GroupNode);
	if (RefGroup->number == 1)
	  if (RefGroup->Invariant)
	    LocalityMatrix->SingleGroups.Invariant++;
	  else if (RefGroup->Spatial)
	    LocalityMatrix->SingleGroups.Spatial++;
	  else
	    LocalityMatrix->SingleGroups.None++;
	else
	  if (RefGroup->Invariant)
	    {
	     LocalityMatrix->MultiGroups.Invariant++;
	     LocalityMatrix->MultiRefs.Invariant
	        += RefGroup->number;
	     if (RefGroup->OtherSpatial)
	       (*OtherSpatialGroups)++;
	    }
	  else if (RefGroup->Spatial)
	    {
	     LocalityMatrix->MultiGroups.Spatial++;
	     LocalityMatrix->MultiRefs.Spatial
	        += RefGroup->number;
	    }
	  else
	    {
	     LocalityMatrix->MultiGroups.None++;
	     LocalityMatrix->MultiRefs.None
	       += RefGroup->number;
	     if (RefGroup->OtherSpatial)
	       (*OtherSpatialGroups)++;
	    }
       }
     LoopStatsLocalityMatrix->SingleGroups.Invariant
       += LocalityMatrix->SingleGroups.Invariant;
     LoopStatsLocalityMatrix->MultiGroups.Invariant
       += LocalityMatrix->MultiGroups.Invariant;
     LoopStatsLocalityMatrix->MultiRefs.Invariant
       += LocalityMatrix->MultiRefs.Invariant;
     LoopStatsLocalityMatrix->SingleGroups.Spatial
       += LocalityMatrix->SingleGroups.Spatial;
     LoopStatsLocalityMatrix->MultiGroups.Spatial
       += LocalityMatrix->MultiGroups.Spatial;
     LoopStatsLocalityMatrix->MultiRefs.Spatial
       += LocalityMatrix->MultiRefs.Spatial;
     LoopStatsLocalityMatrix->SingleGroups.None
       += LocalityMatrix->SingleGroups.None;
     LoopStatsLocalityMatrix->MultiGroups.None
       += LocalityMatrix->MultiGroups.None;
     LoopStatsLocalityMatrix->MultiRefs.None
       += LocalityMatrix->MultiRefs.None;
  }


/****************************************************************/
/*                                                              */
/*   Function:  DumpPermutationStats                            */
/*                                                              */
/*   Input:     loop_data - loop structure                      */
/*              loop - innermost loop of a perfect nest         */
/*              num_loops - number of loops in the perfect nest */
/*              inner_loops - number of innermost loops in      */
/*                            entire imperfect nest             */
/*              LoopStats - permuation statistics               */
/*              logfile - output file                           */
/*                                                              */
/*   Description:  Print out the permutation statistics for a   */
/*                 perfect nest of loops.                       */
/*                                                              */
/****************************************************************/

static void DumpPermutationStats(model_loop *loop_data,
				 int        loop,
				 int        num_loops,
				 int        *inner_loops,
				 LoopStatsType *LoopStats,
				 FILE       *logfile,
				 PedInfo    ped,
				 char       *routine,
				 char       *program)

  {
   int i,j,
       depth,
       FinalLoop,
       MemoryLoop,
       TimeStepLevel,
       OriginalOtherSpatialGroups = 0,
       FinalOtherSpatialGroups = 0,
       MemoryOtherSpatialGroups = 0;
   float OriginalCost,
         FinalCost,
         FinalRatio,
         MemoryCost;
   LocalityMatrixType  
      OriginalLocalityMatrix,
      FinalLocalityMatrix,
      MemoryLocalityMatrix;
   Boolean memory_order = true,
           interchanged = false,
           TimeStep     = false,
           Complex      = false,
           Distribution = false,
           Nearby       = false,
           InOrder      = true;
   UtilNode *GroupNode;
   RefGroupType *RefGroup;

     OriginalLocalityMatrix.SingleGroups.Invariant = 0;
     OriginalLocalityMatrix.SingleGroups.Spatial = 0;
     OriginalLocalityMatrix.SingleGroups.None = 0;
     OriginalLocalityMatrix.MultiGroups.Invariant = 0;
     OriginalLocalityMatrix.MultiGroups.Spatial = 0;
     OriginalLocalityMatrix.MultiGroups.None = 0;
     OriginalLocalityMatrix.MultiRefs.Invariant = 0;
     OriginalLocalityMatrix.MultiRefs.Spatial = 0;
     OriginalLocalityMatrix.MultiRefs.None = 0;
     FinalLocalityMatrix.SingleGroups.Invariant = 0;
     FinalLocalityMatrix.SingleGroups.Spatial = 0;
     FinalLocalityMatrix.SingleGroups.None = 0;
     FinalLocalityMatrix.MultiGroups.Invariant = 0;
     FinalLocalityMatrix.MultiGroups.Spatial = 0;
     FinalLocalityMatrix.MultiGroups.None = 0;
     FinalLocalityMatrix.MultiRefs.Invariant = 0;
     FinalLocalityMatrix.MultiRefs.Spatial = 0;
     FinalLocalityMatrix.MultiRefs.None = 0;
     MemoryLocalityMatrix.SingleGroups.Invariant = 0;
     MemoryLocalityMatrix.SingleGroups.Spatial = 0;
     MemoryLocalityMatrix.SingleGroups.None = 0;
     MemoryLocalityMatrix.MultiGroups.Invariant = 0;
     MemoryLocalityMatrix.MultiGroups.Spatial = 0;
     MemoryLocalityMatrix.MultiGroups.None = 0;
     MemoryLocalityMatrix.MultiRefs.Invariant = 0;
     MemoryLocalityMatrix.MultiRefs.Spatial = 0;
     MemoryLocalityMatrix.MultiRefs.None = 0;
   
     fprintf(logfile,"Permutation Statistic for Perfect Loop Nest %d\n\n",
	     *inner_loops);

           /* print out RefGroup stats */

     for (i = loop;
	  i != -1;
	  i = loop_data[i].parent)
       DumpLoopStats(loop_data,i,num_loops,
		     (UtilList *)UTIL_NODE_ATOM(UTIL_HEAD(loop_data[i].
							  GroupList)),
		     LoopStats,logfile,loop_data[loop].node,ped);

          /* print out permutation statistics */

     fprintf(logfile,"\t **** Permutation Results ***\n\n");
     fprintf(logfile,"\tFinal Order:");
     for (i = 0; i < num_loops; i++)
       {
        fprintf(logfile," %d",loop_data[loop].FinalOrder[i]);
	if (loop_data[loop].FinalOrder[i] != loop_data[loop].MemoryOrder[i])
	  memory_order = false;
       }
     fprintf(logfile,"\n\n");
     if (NOT(memory_order))
       {
	LoopStats->NotInMemoryOrder++;
	fprintf(logfile,"\tMemory Order:");
	for (i = 0; i < num_loops; i++)
          fprintf(logfile," %d",loop_data[loop].MemoryOrder[i]);
	fprintf(logfile,"\n\n");

	       /* print out prevention statistics */

	if (NOT(loop_data[loop].interchange))
	  {
	   fprintf(logfile,"\t******* Interchange Not Safe *******\n\n");
	   for (j = 0; j < num_loops; j++)
	     if (!ut_set_is_empty(loop_data[loop].PreventLvl[j]))
	       {
		fprintf(logfile,"\tUnsafe loops at level %d:",j+1);
		ut_forall(i,loop_data[loop].PreventLvl[j])
		  fprintf(logfile," %d",i);
		fprintf(logfile,"\n");
	       }
	   fprintf(logfile,"\n\n");
	   LoopStats->UnsafeInterchange++;
	  }
	if (loop_data[loop].OutermostLvl > 0)
	  {
	   fprintf(logfile,
		   "\t******* Loop Could Not Be Transformed *******\n\n");
            for(i = loop;
		loop_data[i].level > loop_data[loop].OutermostLvl - 1;
		i = loop_data[i].parent);
            for(i = i;
		i != -1;
		i = loop_data[i].parent)
	      {
	       if (NOT(loop_data[i].distribute && loop_data[loop].expand))
		 {
		  fprintf(logfile,"\tDistribution prevented at level %d\n",
			  loop_data[i].level);
		  Distribution = true;
		  if (loop_data[i].level == 1 && loop_data[i].level != 
		      loop_data[loop].MemoryOrder[loop_data[i].level])

		          /* check for time-step loops */

		    if (loop_data[i].inner_loop != -1)
		      if (loop_data[loop_data[i].inner_loop].next_loop != -1)
			{
			 TimeStepLevel = loop_data[i].level;
			 TimeStep = true;
			 LoopStats->TimeStepPreventedMemoryOrder++;
			 fprintf(logfile,"\t*** Time Step Loop ***\n");
			}
		  fprintf(logfile,"\n");
		 }
	       if (loop_data[i].type == COMPLEX || 
		   loop_data[i].type == TRAP || 
		   loop_data[i].type == MULT)
		 {
		  fprintf(logfile,
			"\tLoop at level %d had a complex iteration space\n\n",
			  loop_data[i].level);
		  Complex = true;
		 }
	      }
	   if (Distribution)
	     LoopStats->DistributionUnsafe++;
	   if (Complex)
	     LoopStats->TooComplex++;
	  }
	i = 0;
	while(i < num_loops && NOT(Nearby))
	  {
	   if (loop_data[loop].FinalOrder[i] != i+1)
	     {
	      Nearby = true;
	      LoopStats->NearbyPermutationAttained++;
	     }
	   i++;
	  }
       }
     else
       {
	i = 0;
	while(i < num_loops && InOrder)
	  {
	   if (loop_data[loop].FinalOrder[i] != i+1)
	    InOrder = false;
	   i++;
	  }
	if (InOrder)
	  LoopStats->InMemoryOrder++;
	else
	  LoopStats->InterchangedIntoMemoryOrder++;
       }
     for (i = 0; i < num_loops; i++)
       if (loop_data[loop].FinalOrder[i] != i+1 &&
	   loop_data[loop].OutermostLvl < i+1)
	 {
	  for (j = loop;
	       loop_data[j].level != loop_data[loop].FinalOrder[i];
	       j = loop_data[j].parent);
	  if (loop_data[j].expand)
	    {
	     LoopStats->NeedsScalarExpansion++;
	     fprintf(logfile,"*** Needs Scalar Expansion ***\n\n");
	     i = num_loops-1;
	    }
	 }
     if (loop_data[loop].FinalOrder[num_loops-1] == 
	 loop_data[loop].MemoryOrder[num_loops-1])
       if (loop_data[loop].FinalOrder[num_loops-1] == num_loops)
         LoopStats->InnerLoopAlreadyCorrect++;
       else
         LoopStats->ObtainedInnerLoop++;
     else
       {
	if (TimeStep && TimeStepLevel == 
	    loop_data[loop].MemoryOrder[num_loops-1])
	  {
	   LoopStats->DesiredInnerTimeStep++;
	   if (loop_data[loop].FinalOrder[num_loops-1] == 
	       loop_data[loop].MemoryOrder[num_loops-2])
	     if (loop_data[loop].FinalOrder[num_loops-1] == num_loops)
	       LoopStats->InnerLoopAlreadyCorrect++;
	     else
	       LoopStats->ObtainedInnerLoop++;
	   else
	     {
	      LoopStats->WrongInnerLoop++;
	      if (num_loops > 2)
	        if (loop_data[loop].FinalOrder[num_loops-1] == 
		    loop_data[loop].MemoryOrder[num_loops-3])
	          LoopStats->NextInnerLoop++;
	     }
	  }
	else
	  {
	   LoopStats->WrongInnerLoop++;
	   if (loop_data[loop].FinalOrder[num_loops-1] == 
	       loop_data[loop].MemoryOrder[num_loops-2])
	      LoopStats->NextInnerLoop++;
	  }
       }
     GetLocalityStats(&LoopStats->OriginalLocalityMatrix,
		      &OriginalLocalityMatrix,
		      &OriginalOtherSpatialGroups,
                      loop_data[loop].GroupList);
     LoopStats->OriginalOtherSpatialGroups += OriginalOtherSpatialGroups;
     for (FinalLoop = loop;
	  loop_data[FinalLoop].level !=loop_data[loop].FinalOrder[num_loops-1];
	  FinalLoop = loop_data[FinalLoop].parent);
     GetLocalityStats(&LoopStats->FinalLocalityMatrix,
		      &FinalLocalityMatrix,
		      &FinalOtherSpatialGroups,
                      loop_data[FinalLoop].GroupList);
     LoopStats->FinalOtherSpatialGroups += FinalOtherSpatialGroups;
     for (MemoryLoop = loop;
	  loop_data[MemoryLoop].level
	    != loop_data[loop].MemoryOrder[num_loops-1];
	  MemoryLoop = loop_data[MemoryLoop].parent);
     GetLocalityStats(&LoopStats->MemoryLocalityMatrix,
		      &MemoryLocalityMatrix,
		      &MemoryOtherSpatialGroups,
                      loop_data[MemoryLoop].GroupList);
     LoopStats->MemoryOtherSpatialGroups += MemoryOtherSpatialGroups;
     fprintf(logfile,"Original Loop Locality\n");
     fprintf(logfile,"======================\n\n");
     fprintf(logfile,"RefGroups\n");
     fprintf(logfile,"=========\n\n");
     fprintf(logfile,"\t\tInvariant   Spatial    None\n");
     fprintf(logfile,"\t\t=========   =======    ====\n");
     fprintf(logfile,"\tSingle\t%6d      %6d     %4d\n",
		OriginalLocalityMatrix.SingleGroups.Invariant,
		OriginalLocalityMatrix.SingleGroups.Spatial,
		OriginalLocalityMatrix.SingleGroups.None);
     fprintf(logfile,"\tMulti \t%6d      %6d     %4d\n",
		OriginalLocalityMatrix.MultiGroups.Invariant,
		OriginalLocalityMatrix.MultiGroups.Spatial,
		OriginalLocalityMatrix.MultiGroups.None);
     fprintf(logfile,"\tMRefs \t%6d      %6d     %4d\n\n",
		OriginalLocalityMatrix.MultiRefs.Invariant,
		OriginalLocalityMatrix.MultiRefs.Spatial,
		OriginalLocalityMatrix.MultiRefs.None);
     fprintf(logfile,"Other Spatial %d\n\n",
		OriginalOtherSpatialGroups);
     fprintf(logfile,"Orginal Balance   = %.4f\n\n\n",
	     loop_data[loop].fbalance);
     fprintf(logfile,"Final Loop Locality\n");
     fprintf(logfile,"======================\n\n");
     fprintf(logfile,"RefGroups\n");
     fprintf(logfile,"=========\n\n");
     fprintf(logfile,"\t\tInvariant   Spatial    None\n");
     fprintf(logfile,"\t\t=========   =======    ====\n");
     fprintf(logfile,"\tSingle\t%6d      %6d     %4d\n",
		FinalLocalityMatrix.SingleGroups.Invariant,
		FinalLocalityMatrix.SingleGroups.Spatial,
		FinalLocalityMatrix.SingleGroups.None);
     fprintf(logfile,"\tMulti \t%6d      %6d     %4d\n",
		FinalLocalityMatrix.MultiGroups.Invariant,
		FinalLocalityMatrix.MultiGroups.Spatial,
		FinalLocalityMatrix.MultiGroups.None);
     fprintf(logfile,"\tMRefs \t%6d      %6d     %4d\n\n",
		FinalLocalityMatrix.MultiRefs.Invariant,
		FinalLocalityMatrix.MultiRefs.Spatial,
		FinalLocalityMatrix.MultiRefs.None);
     fprintf(logfile,"Other Spatial %d\n\n",
		FinalOtherSpatialGroups);
     fprintf(logfile,"Final Balance   = %.4f\n\n\n",
	     loop_data[FinalLoop].fbalance);
     fprintf(logfile,"Memory Loop Locality\n");
     fprintf(logfile,"======================\n\n");
     fprintf(logfile,"RefGroups\n");
     fprintf(logfile,"=========\n\n");
     fprintf(logfile,"\t\tInvariant   Spatial    None\n");
     fprintf(logfile,"\t\t=========   =======    ====\n");
     fprintf(logfile,"\tSingle\t%6d      %6d     %4d\n",
		MemoryLocalityMatrix.SingleGroups.Invariant,
		MemoryLocalityMatrix.SingleGroups.Spatial,
		MemoryLocalityMatrix.SingleGroups.None);
     fprintf(logfile,"\tMulti \t%6d      %6d     %4d\n",
		MemoryLocalityMatrix.MultiGroups.Invariant,
		MemoryLocalityMatrix.MultiGroups.Spatial,
		MemoryLocalityMatrix.MultiGroups.None);
     fprintf(logfile,"\tMRefs \t%6d      %6d     %4d\n\n",
		MemoryLocalityMatrix.MultiRefs.Invariant,
		MemoryLocalityMatrix.MultiRefs.Spatial,
		MemoryLocalityMatrix.MultiRefs.None);
     fprintf(logfile,"Other Spatial %d\n\n",
		MemoryOtherSpatialGroups);
     fprintf(logfile,"Memory Balance   = %.4f\n\n\n",
	     loop_data[MemoryLoop].fbalance);
     depth = loop_data[loop].level;
     FinalCost = GetLoopCostFirst(loop_data,FinalLoop);
     MemoryCost = GetLoopCostFirst(loop_data,MemoryLoop);
     OriginalCost = GetLoopCostFirst(loop_data,loop);
     if (FinalCost != 0)
       {
	FinalRatio = ((float)OriginalCost)/((float)FinalCost);
	LoopStats->FinalRatio[depth-1] += FinalRatio;
	fprintf(logfile,"\tFinal Ratio = %.2f\n",FinalRatio);
#ifdef STATSDEBUG
	if (FinalRatio > 1.00)
	  {
           printf("Debug Info for Perfect Loop Nest %d\n\n",*inner_loops);
	   printf("%s: %s  CostRatio = %.2f  BalanceRatio = %.2f\n",program,
		  routine,FinalRatio,
		  loop_data[loop].fbalance/loop_data[FinalLoop].fbalance);
	  }
#endif
       }
     else
       {
	LoopStats->FinalRatio[depth-1] += 1.0;
	fprintf(logfile,"\tFinal Ratio = 1.00\n");
       }
     if (MemoryCost != 0)
       {
	LoopStats->MemoryRatio[depth-1] += 
	  (((float)OriginalCost)/((float)MemoryCost));
	fprintf(logfile,"\tMemory Ratio = %.2f\n",
		((float)OriginalCost)/((float)MemoryCost));
       }
     else
       {
	LoopStats->MemoryRatio[depth-1] += 1.0;
	fprintf(logfile,"\tMemory Ratio = 1.00\n");
       }
     fprintf(logfile,"\tNesting Depth: %d\n",depth);
     LoopStats->NestingDepth[depth-1]++;
     fprintf(logfile,"\n\n\n");
     for (i = loop;
	  i != -1;
	  i = loop_data[i].parent)
       {
	util_free_nodes((UtilList *)UTIL_NODE_ATOM(
                           UTIL_HEAD(loop_data[i].GroupList)));
	util_list_free((UtilList *)UTIL_NODE_ATOM(
                           UTIL_HEAD(loop_data[i].GroupList)));
	util_free_node(util_pop(loop_data[i].GroupList));
	loop_data[i].InvariantCostList.free_head();
	loop_data[i].SpatialCostList.free_head();
	loop_data[i].OtherSpatialCostList.free_head();
	loop_data[i].TemporalCostList.free_head();
	loop_data[i].NoneCostList.free_head();
       }
  }


/****************************************************************/
/*                                                              */
/*   Function:  WalkLoopsForStats                               */
/*                                                              */
/*   Input:     loop_data - loop structure                      */
/*              loop - loop being walked                        */
/*              num_loops - number of loop in perfect nest      */
/*              inner_loops - number of innermost loops         */
/*              ped - handle to dependence graph                */
/*              LoopStats - permutation statistics              */
/*                                                              */
/*   Description: Walk loop_data structure and print out        */
/*                permutation statistics for each perfect nest  */
/*                                                              */
/****************************************************************/

static void WalkLoopsForStats(model_loop *loop_data,
			      int        loop,
			      int        num_loops,
			      int        *inner_loops,
			      PedInfo    ped,
			      LoopStatsType *LoopStats,
			      char       *routine,
			      char       *program)
  {
   int next,temp;

     LoopStats->TotalLoops++;
     if (loop_data[loop].inner_loop == -1)
       {
	*inner_loops = *inner_loops + 1;
	DumpPermutationStats(loop_data,loop,num_loops,inner_loops,LoopStats,
			     ((config_type *)PED_MH_CONFIG(ped))->logfile,
			     ped,routine,program);
       }
     else
       {
	next = loop_data[loop].inner_loop;
	LoopStats->Perfect = (Boolean)(LoopStats->Perfect && 
				       loop_data[next].next_loop == -1);
	while (next != -1)
	  {
	   WalkLoopsForStats(loop_data,next,num_loops+1,inner_loops,ped,
			     LoopStats,routine,program);
	   next = loop_data[next].next_loop;
	  }
       }
  }


/****************************************************************/
/*                                                              */
/*   Function:  memory_interchange_stats                        */
/*                                                              */
/*   Input:     ped - dependence graph handle                   */
/*              root - ast index of outermost loop statement    */
/*              level - level of root                           */
/*              LoopStats - permutation statistics              */
/*              symtab - symbol table                           */
/*              ar - arena object for memory allocation         */
/*                                                              */
/*   Description: Driver for gathering loop permutation         */
/*                statistics on a loop nest.                    */
/*                                                              */
/****************************************************************/

void memory_interchange_stats(PedInfo       ped,
			      AST_INDEX     root,
			      int           level,
			      LoopStatsType *LoopStats,
			      char          *routine,
			      char          *program,
			      SymDescriptor symtab,
			      arena_type    *ar)

  {
   pre_info_type pre_info;
   model_loop    *loop_data;
   UtilList      *loop_list;
   int           inner_loops;

     pre_info.stmt_num = 0;
     pre_info.loop_num = 0;
     pre_info.surrounding_do = -1;
     pre_info.abort = false;
     pre_info.ped = ped;
     pre_info.symtab = symtab;
     pre_info.ar = ar;

              /* build surrounding do information and initialize 
		 scratch field in ast */

     walk_statements(root,level,ut_mark_do_pre,ut_mark_do_post,
		     (Generic)&pre_info);

     if (pre_info.abort)
       return;

     loop_data = (model_loop *)ar->arena_alloc_mem_clear(LOOP_ARENA,
					 pre_info.loop_num*sizeof(model_loop));

             /* build structure representing loop nest */

     ut_analyze_loop(root,loop_data,level,ped,symtab);

     if (loop_data[0].inner_loop == -1)
       return;

             /* examine the iteration space shape for the loop nest */

     ut_check_shape(loop_data,0);

           /* compute permutatation information */

     li_ComputeMemoryOrder(loop_data,symtab,ped,ar);

     inner_loops = 0;
     fprintf(((config_type *)PED_MH_CONFIG(ped))->logfile,
	     "*********************************************************\n\n");
     fprintf(((config_type *)PED_MH_CONFIG(ped))->logfile,
	     "Statistics for New Loop Nest\n\n");

          /* print permutation statistics */

     WalkLoopsForStats(loop_data,0,1,&inner_loops,ped,LoopStats,routine,
		       program);
  }
