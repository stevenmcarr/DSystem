/****************************************************************/
/*                                                              */
/*   File:   MemoryOrder.C                                      */
/*                                                              */
/*   Description: Compute locality information and memory order */
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
#include <Reversal.h>

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
/*   Function:   UnsetVisitedMark                               */
/*                                                              */
/*   Input:      node - node in the AST                         */
/*               dummy - anything                               */
/*                                                              */
/*   Description: set the visited mark for depth-first search   */
/*                to false for each subscript.                  */
/*                                                              */
/****************************************************************/

static int UnsetVisitedMark(AST_INDEX node,
			    int       dummy)

  {
   AST_INDEX name;

     if (is_subscript(node))
       {
	name = gen_SUBSCRIPT_get_name(node);
	get_subscript_ptr(name)->visited = false;
       }
     return(WALK_CONTINUE);
    }


/****************************************************************/
/*                                                              */
/*   Function:   NotInOtherPositions                            */
/*                                                              */
/*   Input:      node - subscript list                          */
/*               var - induction variable search for            */
/*                                                              */
/*   Description: search list of subscripts of an array         */
/*                to determine if var appears in any subscript  */
/*                position other than the first.                */
/*                                                              */
/****************************************************************/

static Boolean NotInOtherPositions(AST_INDEX node,
				   char      *var)

  {
     for (node = list_next(list_first(node));
	  node != AST_NIL;
	  node = list_next(node))
       if (pt_find_var(node,var))
         return(false);
     return(true);
  }


/****************************************************************/
/*                                                              */
/*   Function:   FindInductionVar                               */
/*                                                              */
/*   Input:      loop_data - loop structure                     */
/*               node - node in AST                             */
/*               level - nesting level of induction var         */
/*                                                              */
/*   Description:  Search loops surrounding node for the        */
/*                 induction variable at nesting level "level"  */
/*                                                              */
/****************************************************************/

static char *FindInductionVar(model_loop *loop_data,
			      AST_INDEX  node,
			      int        level)

  {
   int i;

     i = get_subscript_ptr(node)->surrounding_do;
     while(loop_data[i].level != level)
       i = loop_data[i].parent;
     return(gen_get_text(gen_INDUCTIVE_get_name(gen_DO_get_control(
			 loop_data[i].node))));
  }


/****************************************************************/
/*                                                              */
/*   Function:   OnlyInInnermostPostion                         */
/*                                                              */
/*   Input:      loop_data - loop structure                     */
/*               node - subscript node in AST                   */
/*               level - nesting level of induction var         */
/*                                                              */
/*   Description: Determines if the induction var at nesting    */
/*                level "level" only appears in the first       */
/*                subscript position of node.                   */
/*                                                              */
/****************************************************************/

static Boolean OnlyInInnermostPosition(model_loop *loop_data,
				       AST_INDEX  node,
				       int        level)
  {
   AST_INDEX sub_list,sub;
   char *var;
   int coeff;
   Boolean lin;
   
     if (level == LOOP_INDEPENDENT)
       return(false);
     sub_list = gen_SUBSCRIPT_get_rvalue_LIST(tree_out(node));
     sub = list_first(sub_list);
     var = FindInductionVar(loop_data,node,level);
     if (pt_find_var(sub,var) && NotInOtherPositions(sub_list,var))
       return(true);
     return(false);
  }


/****************************************************************/
/*                                                              */
/*   Function:   CanMoveToInnermost                             */
/*                                                              */
/*   Input:      edge - dependence edge                         */
/*                                                              */
/*   Description: Determines if edge can become carried by the  */
/*                innermost loop through loop interchange.      */
/*                                                              */
/****************************************************************/

static Boolean CanMoveToInnermost(DG_Edge *edge)

  {
   int i;
   
     if (edge->level == LOOP_INDEPENDENT)
       return(true);
     for (i = edge->level+1; i < gen_get_dt_LVL(edge);i++)
       if (gen_get_dt_DIS(edge,i) != 0)
         return(false);
     return(true);
  }


/****************************************************************/
/*                                                              */
/*   Function:   DoPartition                                    */
/*                                                              */
/*   Input:      name - array name node in ast                  */
/*               RefGroup - reference group structure           */
/*               dg - dependence graph                          */
/*               ped - graph and ast information                */
/*               level - nesting level of loop on which to      */
/*                       to compute RefGroups                   */
/*               VisitedMark - for DFS                          */
/*               loop_data - loop structure information         */
/*                                                              */
/*   Description: Partitions the references in the innermost    */
/*                loop body into RefGroups based on the         */
/*                dependences carried by the loop at nesting    */
/*                level "level".  This loop is being considered */
/*                as if it were in the innermost position.      */
/*                                                              */
/****************************************************************/

static void DoPartition(AST_INDEX name,
			RefGroupType *RefGroup,
			DG_Edge   *dg,
			PedInfo   ped,
			int       level,
			int       MinLevel,
			Boolean   VisitedMark,
			model_loop *loop_data)

  {
   subscript_info_type *sptr1,*sptr2;
   int              refl;
   EDGE_INDEX       edge;

     sptr1 = get_subscript_ptr(name);
     sptr1->visited = VisitedMark;
     sptr1->lnode = util_node_alloc(name,NULL);

              /* add reference to RefGroup */

     util_append(RefGroup->RefList,sptr1->lnode);
     RefGroup->number++;
     refl = get_info(ped,name,type_levelv);

              /* look at all outgoing edges for references that belong
		 in the same RefGroup.  Only consider loop-independent 
		 edges and edges carried by the the loop at "level" or
		 edges that give rise to group-spatial locality.  
		 Additionally, each edge must be able to move into
		 the innermost position with interchange.   */

     for (edge = dg_first_src_ref( PED_DG(ped),refl);
	  edge != END_OF_LIST;
	  edge = dg_next_src_ref( PED_DG(ped),edge))

       if (dg[edge].level >= MinLevel)
         if ((dg[edge].consistent == consistent_SIV ||   /* check consistency */
	      (dg[edge].consistent == consistent_MIV && 
	       (dg[edge].level == LOOP_INDEPENDENT ||
		dg[edge].src == dg[edge].sink))) &&
	     NOT(dg[edge].symbolic) &&
	     (CanMoveToInnermost(&dg[edge]) &&          /* innermost edge? */
	      (dg[edge].level == level || 
	       dg[edge].level == LOOP_INDEPENDENT ||
	       OnlyInInnermostPosition(loop_data,dg[edge].src,dg[edge].level))))
	   {
	    sptr2 = get_subscript_ptr(dg[edge].sink);
	    if(sptr2->visited != VisitedMark &&
	       sptr1->surrounding_do == sptr2->surrounding_do &&
	       !is_call(ut_get_stmt(dg[edge].sink)))
	      DoPartition(dg[edge].sink,RefGroup,dg,ped,level,MinLevel,VisitedMark,
			  loop_data);
	   }

   /* look at all incoming edges for references fitting the
      same criteria. */

     for (edge = dg_first_sink_ref( PED_DG(ped),refl);
	  edge != END_OF_LIST;
	  edge = dg_next_sink_ref( PED_DG(ped),edge))
       if (dg[edge].level >= MinLevel)
         if ((dg[edge].consistent == consistent_SIV ||   /* check consistency */
	      (dg[edge].consistent == consistent_MIV && 
	       dg[edge].level == LOOP_INDEPENDENT ||
	       dg[edge].src == dg[edge].sink)) &&
	     NOT(dg[edge].symbolic) &&
	     (CanMoveToInnermost(&dg[edge]) &&          /* innermost edge? */
	      (dg[edge].level == level || 
	       dg[edge].level == LOOP_INDEPENDENT ||
	       OnlyInInnermostPosition(loop_data,dg[edge].src,dg[edge].level))))
	   {
	    sptr2 = get_subscript_ptr(dg[edge].src);
	    if(sptr2->visited != VisitedMark &&
	       sptr1->surrounding_do == sptr2->surrounding_do &&
	       !is_call(ut_get_stmt(dg[edge].sink)))
	      DoPartition(dg[edge].src,RefGroup,dg,ped,level,MinLevel,VisitedMark,
			loop_data);
	   }
  }


/****************************************************************/
/*                                                              */
/*   Function:   PartitionNames                                 */
/*                                                              */
/*   Input:      node - node in the AST                         */
/*               RefInfo - various information                  */
/*                                                              */
/*   Description: Search AST for references that are not in a   */
/*                RefGroup and call DoPartition on those        */
/*                references.                                   */
/*                                                              */
/****************************************************************/

static int PartitionNames(AST_INDEX   node,
			   RefInfoType *RefInfo)


  {
   subscript_info_type *sptr;
   AST_INDEX            name;
   RefGroupType        *RefGroup;

     if (is_subscript(node) &&
	 !is_call(ut_get_stmt(node)))
       {
	name = gen_SUBSCRIPT_get_name(node);
	sptr = get_subscript_ptr(name);
	if (sptr->visited != RefInfo->VisitedMark)
	  {
	   RefGroup = (RefGroupType *)RefInfo->
	                  ar->arena_alloc_mem_clear(LOOP_ARENA,
						    sizeof(RefGroupType));
	   RefGroup->number = 0;
	   util_append(RefInfo->GroupList,util_node_alloc((Generic)RefGroup,
							"reference"));
	   RefGroup->RefList = util_list_alloc((Generic)NULL,"ref-list");
	   DoPartition(name,RefGroup,RefInfo->dg,RefInfo->ped,
		       RefInfo->level,RefInfo->loop_data[0].level,
		       RefInfo->VisitedMark,RefInfo->loop_data);
	  }
       }
    return(WALK_CONTINUE);
  }


/****************************************************************/
/*                                                              */
/*   Function:   FindOldestValue                                */
/*                                                              */
/*   Input:      RefGroup - reference group                     */
/*               ped - various loop information                 */
/*                                                              */
/*   Description: Search through RefGroup for the array         */
/*                reference that is first to reference the      */
/*                value that flows through out the group.       */
/*                That reference will have no incoming          */
/*                dependence from a another member of the       */
/*                group or will only have dependences that      */
/*                are carried by a loop that the reference is   */
/*                invariant with respect to.                    */
/*                                                              */
/****************************************************************/

static AST_INDEX FindOldestValue(RefGroupType *RefGroup,
				 PedInfo      ped)

  {
   UtilNode *RefNode;
   int      refl;
   AST_INDEX node;
   Boolean  found;
   DG_Edge  *dg;
   EDGE_INDEX edge;

     for (RefNode = UTIL_HEAD(RefGroup->RefList);
	  RefNode != NULLNODE;
	  RefNode = UTIL_NEXT(RefNode))
       {
        refl = get_info(ped,(AST_INDEX)UTIL_NODE_ATOM(RefNode),
                        type_levelv);
	dg = dg_get_edge_structure( PED_DG(ped));
        found = true;

	    /*  Assume reference is the oldest value.  Search incoming
		edges for an edge that disproves the assertion. */

        for (edge = dg_first_sink_ref( PED_DG(ped),refl);
             edge != END_OF_LIST;
             edge = dg_next_sink_ref( PED_DG(ped),edge))

          if (dg[edge].src != dg[edge].sink &&  /* edge not from self */
	      ((!pt_expr_equal(tree_out(dg[edge].src),tree_out(dg[edge].sink)) 
		&& gen_get_dt_DIS(&dg[edge],dg[edge].level) != DDATA_ANY) || 
	       dg[edge].level == LOOP_INDEPENDENT) && /* make sure edge not
							 carried by 
							 src-invariant loop */

	      get_subscript_ptr(dg[edge].src)->surrounding_do ==
	      get_subscript_ptr(dg[edge].sink)->surrounding_do && /* source
								     and sink
								     in same
								     loop */

	      (dg[edge].consistent == consistent_SIV ||
	       (dg[edge].consistent == consistent_MIV && /* consistent edge */
		dg[edge].level == LOOP_INDEPENDENT)) &&
	      NOT(dg[edge].symbolic))

            if (util_in_list(get_subscript_ptr(dg[edge].src)->lnode,
			     RefGroup->RefList))  /* in same ref group */
              {
               found = false;
               break;
              }
        if (found)
          break;
       }
     return(UTIL_NODE_ATOM(RefNode));
  }


/****************************************************************/
/*                                                              */
/*   Function:   CheckTemporal                                  */
/*                                                              */
/*   Input:      node - oldest value in a RefGroup              */
/*               ped - dependence graph, etc.                   */
/*               loop_data - loop structure                     */
/*               loop - loop being considered as innermost      */
/*               TemporalCost - lines/iteration required for    */
/*                              groups with temporal reuse      */
/*               RefGroup - reference group                     */
/*                                                              */
/*   Description: Determines if ref group with "node" as        */
/*                oldest value has temporal reuse               */
/*                                                              */
/****************************************************************/

static void CheckTemporal(AST_INDEX  node,
			  PedInfo    ped,
			  model_loop *loop_data,
			  int        loop,
			  float      *TemporalCost,
			  RefGroupType *RefGroup)

  {
   int refl;
   DG_Edge *dg;
   EDGE_INDEX edge;

     refl = get_info(ped,node,type_levelv);
     dg = dg_get_edge_structure( PED_DG(ped));
     for (edge = dg_first_src_ref( PED_DG(ped),refl);
	  edge != END_OF_LIST && NOT(RefGroup->Temporal);
	  edge = dg_next_src_ref( PED_DG(ped),edge))

           /* look for outgoing consistent edge that would be 
	      loop independent or carried by the innermost loop
              if "loop" were innermost */

       if ((dg[edge].level == loop_data[loop].level  ||
	    dg[edge].level == LOOP_INDEPENDENT) && 
	   CanMoveToInnermost(&dg[edge]))
	 {
	  RefGroup->Temporal = true;

	           /* make sure cost not counted elsewhere */

	  if (NOT(RefGroup->Spatial) && NOT(RefGroup->Invariant))
	    (*TemporalCost) += 1.0;
	 }
  }


/****************************************************************/
/*                                                              */
/*   Function:   CheckOtherSpatial                              */
/*                                                              */
/*   Input:      node - oldest value in a RefGroup              */
/*               ped - dependence graph, etc.                   */
/*               loop_data - loop structure                     */
/*               loop - loop being considered as innermost      */
/*               OtherSpatialCost - lines/iteration required for*/
/*                              groups with group-spatial reuse */
/*               RefGroup - reference group                     */
/*               words - number of words in a cache line        */
/*                                                              */
/*   Description: Check if ref group with "node" as oldest      */
/*                value has group-spatial reuse.                */
/*                                                              */
/****************************************************************/

static void CheckOtherSpatial(AST_INDEX  node,
			      PedInfo    ped,
			      model_loop *loop_data,
			      int        loop,
			      float      *OtherSpatialCost,
			      RefGroupType *RefGroup,
			      int        words)

  {
   int refl,coeff;
   DG_Edge *dg;
   EDGE_INDEX edge;
   AST_INDEX sub_list,sub;
   char *var;
   Boolean lin;

     refl = get_info(ped,node,type_levelv);
     dg = dg_get_edge_structure( PED_DG(ped));

               /* look for an outgoing edge that is carried by the
		  loop with its induction variable in the first
		  subscript position and has no other non-zero 
		  distance vector entries */

     for (edge = dg_first_src_ref( PED_DG(ped),refl);
	  edge != END_OF_LIST && NOT(RefGroup->OtherSpatial);
	  edge = dg_next_src_ref( PED_DG(ped),edge))
       if (dg[edge].level != loop_data[loop].level && 
	   dg[edge].level != LOOP_INDEPENDENT &&
	   CanMoveToInnermost(&dg[edge]))  /* check distance vector */
	 {
	  if (OnlyInInnermostPosition(loop_data,node,dg[edge].level) &&
	      gen_get_dt_DIS(&dg[edge],dg[edge].level) < words)

	           /* make sure index in first subscript position and
		      the dependence distance is less than the number
		      of words in a line */
	    {
	     RefGroup->OtherSpatial = true;

	          /* make sure cost not counted elsewhere */

	     if (NOT(RefGroup->Temporal) && NOT(RefGroup->Invariant))
	      (*OtherSpatialCost) += 1.0;
	    }
	 }
  }


/****************************************************************/
/*                                                              */
/*   Function:  CheckInvariant                                  */
/*                                                              */
/*   Input:      node - oldest value in a RefGroup              */
/*               ped - dependence graph, etc.                   */
/*               loop_data - loop structure                     */
/*               loop - loop being considered as innermost      */
/*               InvariantCost - lines/iteration required for   */
/*                              groups with self-temporal reuse */
/*               RefGroup - reference group                     */
/*                                                              */
/*   Description:
/*                                                              */
/****************************************************************/

static void CheckInvariant(AST_INDEX  node,
			   PedInfo    ped,
			   model_loop *loop_data,
			   int        loop,
			   float      *InvariantCost,
			   RefGroupType *RefGroup)

  {
   int refl;
   DG_Edge *dg;
   EDGE_INDEX edge;

     refl = get_info(ped,node,type_levelv);
     dg = dg_get_edge_structure( PED_DG(ped));

                 /* look for consisitent dependence from a reference to
		    itself */

     for (edge = dg_first_src_ref( PED_DG(ped),refl);
	  edge != END_OF_LIST && NOT(RefGroup->Invariant);
	  edge = dg_next_src_ref( PED_DG(ped),edge))
       if (dg[edge].level == loop_data[loop].level &&
	   dg[edge].src == dg[edge].sink &&
	   dg[edge].consistent != inconsistent &&
	   NOT(dg[edge].symbolic))
	 {
	  RefGroup->Invariant = true;
	  (*InvariantCost) += 1.0;
	 }
  }


/****************************************************************/
/*                                                              */
/*   Function:   CheckRefGroups                                 */
/*                                                              */
/*   Input:      loop_data - loop structure                     */
/*               loop - loop considered as innermost            */
/*               RefInfo - miscellaneous data                   */
/*                                                              */
/*   Description: Go through list of ref groups and determine   */
/*                locality properties.                          */
/*                                                              */
/****************************************************************/

static void CheckRefGroups(model_loop    *loop_data,
			   int           loop,
			   RefInfoType   *RefInfo)

  {
   AST_INDEX sub_list,
             sub,node;
   Boolean   lin;
   int       coeff,words;
   int       level_val,dims;
   char      *var;
   UtilNode  *GroupNode;
   RefGroupType  *RefGroup;

     for (GroupNode = UTIL_HEAD(RefInfo->GroupList);
	  GroupNode != NULLNODE;
	  GroupNode = UTIL_NEXT(GroupNode))
       {
	RefGroup = (RefGroupType *)UTIL_NODE_ATOM(GroupNode);
	var = gen_get_text(gen_INDUCTIVE_get_name(gen_DO_get_control(
			   loop_data[loop].node)));
	node = tree_out(UTIL_NODE_ATOM(UTIL_HEAD(RefGroup->RefList)));
	sub_list = gen_SUBSCRIPT_get_rvalue_LIST(node);
	if (gen_get_converted_type(node) == TYPE_DOUBLE_PRECISION ||
	    gen_get_converted_type(node) == TYPE_COMPLEX)
	  words = (((config_type *)PED_MH_CONFIG(RefInfo->ped))->
		   line) >> 3; 
	else
	  words = (((config_type *)PED_MH_CONFIG(RefInfo->ped))->
		   line) >> 2; 
 
	        /* check for self-spatial reuse, induction var
		   appears in first subscript position only */

	if (pt_find_var(sub_list,var))
	  {
	   sub = list_first(sub_list);
	   if (pt_find_var(sub,var) && NotInOtherPositions(sub_list,var))
	     {
	      pt_get_coeff(sub,var,&lin,&coeff);	
	      if (coeff < 0)
	        coeff = -coeff;

	                /* make sure step size less than line size */

	      if (coeff < words && lin)
		{
		 RefGroup->Spatial = true;
		 RefInfo->SpatialCost += (1.0/((float)(words)/(float)coeff));
		}
	     }
	  }
	else 

	         /* check for invariant reuse */
	  {
	   node = FindOldestValue(RefGroup,RefInfo->ped);
	   CheckInvariant(node,RefInfo->ped,loop_data,loop,
			  &RefInfo->InvariantCost,RefGroup);
	  }

	        /* check for group reuse */

	if (UTIL_HEAD(RefGroup->RefList) != UTIL_TAIL(RefGroup->RefList))
	  {
	   node = FindOldestValue(RefGroup,RefInfo->ped);
	   CheckTemporal(node,RefInfo->ped,loop_data,loop,
			 &RefInfo->TemporalCost,RefGroup);
	   CheckOtherSpatial(node,RefInfo->ped,loop_data,loop,
			     &RefInfo->OtherSpatialCost,RefGroup,words);
	  }

	       /* check if no reuse found */

	if (NOT(RefGroup->Invariant) && NOT(RefGroup->Spatial) &&
	    NOT(RefGroup->OtherSpatial) && NOT(RefGroup->Temporal))
	   RefInfo->NoneCost += 1.0;
       }
  }


/****************************************************************/
/*                                                              */
/*   Function:   LoopsNotInOrder                                */
/*                                                              */
/*   Input:      loop_data - loop structure                     */
/*               heap - sorted order of loops                   */
/*               num_loops - number of loops in permutation     */
/*                                                              */
/*   Description:  Determine if "heap" does not reperesent the  */
/*                 original order of loops.                     */
/*                                                              */
/****************************************************************/

static Boolean LoopsNotInOrder(model_loop *loop_data,
			       heap_type  *heap,
			       int        num_loops)

  {
   int i;
   
     for (i = 0; i < num_loops; i++)

          /* level that does not match index means not in order */

       if (loop_data[heap[i].index].level != i+1)
         return(true);
     return(false);
  }


/****************************************************************/
/*                                                              */
/*   Function:  Heapify                                         */
/*                                                              */
/*   Input:     heap - heap of loops ordered by memory cost     */
/*              i - beginning index of heap                     */
/*              j - ending index of heap                        */
/*              n - size of heap                                */
/*                                                              */
/*   Description: make a section of the array "heap" into a     */
/*                heap ordered by memory costs.                 */
/*                                                              */
/****************************************************************/

static void Heapify(heap_type *heap,
		    int       i,
		    int       j,
		    int       n)

  {
   int k,i1,i2,
       temp_index,
       temp_stride;

     if (i < ((j+1) >> 1))
       {
	i1 = (i << 1) + 1;
	i2 = (i << 1) + 2;
	if (i2 > n) 
	  i2 = i1;
	if ((heap[i1].stride < heap[i].stride || 
	     (heap[i1].stride == heap[i].stride &&
	      heap[i1].index > heap[i].index)) ||
	    (heap[i2].stride < heap[i].stride ||
	     (heap[i2].stride == heap[i].stride &&
	      heap[i2].index > heap[i].index)))
	  {
	   if (heap[i1].stride < heap[i2].stride ||
	       (heap[i1].stride == heap[i1].stride &&
		heap[i1].index > heap[i2].index))
	     k = i1;
	   else
	     k = i2;
	   temp_index = heap[i].index;
	   temp_stride = heap[i].stride;
	   heap[i].index = heap[k].index;
	   heap[i].stride = heap[k].stride;
	   heap[k].index = temp_index;
	   heap[k].stride = temp_stride;
	   Heapify(heap,k,j,n);
	  }
       }
  }


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
/*   Function: GetLoopCostLast                                  */
/*                                                              */
/*   Input:    loop_data - loop structure                       */
/*             j - index into loop structure                    */
/*                                                              */
/*   Description:  To handle non-perfectly nested loops there   */
/*                 is a list of costs associated with each      */
/*                 possible perfect nest.  This function returns*/
/*                 the cost of the last loop on the list.       */
/*                                                              */
/****************************************************************/

static int GetLoopCostLast(model_loop *loop_data,
			  int        j)

  {
   return
     GetLoopCost(loop_data[j].TemporalCostList.last_entry()->GetValue(),
		 loop_data[j].SpatialCostList.last_entry()->GetValue(),
		 loop_data[j].OtherSpatialCostList.last_entry()->GetValue(),
		 loop_data[j].InvariantCostList.last_entry()->GetValue(),
		 loop_data[j].NoneCostList.last_entry()->GetValue());
  }


/****************************************************************/
/*                                                              */
/*   Function:   ComputeLoopOrder                               */
/*                                                              */
/*   Input:      loop_data - loop structure                     */
/*               num_loops - number of loops in perfect nest    */
/*               ped - dependence information, etc.             */
/*               loop_list - list of loops in perfect nest      */
/*               heap - sorted list of loops in memory order    */
/*                       (output)                               */
/*                                                              */
/*   Description: Perform a heap sort of the list of loops to   */
/*                compute memory order                          */
/*                                                              */
/****************************************************************/

static void ComputeLoopOrder(model_loop *loop_data,
			     int        num_loops,
			     PedInfo    ped,
			     UtilList      *loop_list,
			     heap_type     *heap)

  {
   int           i,pos,j;
   int           temp_index,temp_stride;
   UtilNode      *node;

     for (node = UTIL_HEAD(loop_list),i = 0;
	  node != NULLNODE;
	  node = UTIL_NEXT(node),i++)
       {
	heap[i].index = UTIL_NODE_ATOM(node);
	j = heap[i].index;
	heap[i].stride = GetLoopCostLast(loop_data,j);
       }
     for (i = (num_loops-1) >> 1; i >= 0; i--)
       Heapify(heap,i,num_loops-1,num_loops-1);
     for (i = num_loops-1; i > 0; i--)
       {
	temp_index = heap[i].index;
	temp_stride = heap[i].stride;
	heap[i].index = heap[0].index;
	heap[i].stride = heap[0].stride;
	heap[0].index = temp_index;
	heap[0].stride = temp_stride;
	Heapify(heap,0,i-1,i-1);
       }
  }


/****************************************************************/
/*                                                              */
/*   Function:   GenerateEdgeList                               */
/*                                                              */
/*   Input:      node - ast node                                */
/*               EdgeInfo - dependence graph, edge list, etc.   */
/*                                                              */
/*   Description: Make a list of all carried true, anti and     */
/*   output edges                                               */
/*                                                              */
/****************************************************************/

static int GenerateEdgeList(AST_INDEX node,
			    EdgeInfoType *EdgeInfo)

  {
   AST_INDEX name;
   int       refl,i,dist;
   DG_Edge   *dg;
   Boolean   less;
   EDGE_INDEX edge;

     if (is_subscript(node))
       {
	name = gen_SUBSCRIPT_get_name(node);
	refl = get_info(EdgeInfo->ped,name,type_levelv);
	dg = dg_get_edge_structure( PED_DG(EdgeInfo->ped));
	for (edge = dg_first_src_ref( PED_DG(EdgeInfo->ped),refl);
	     edge != END_OF_LIST;
	     edge = dg_next_src_ref( PED_DG(EdgeInfo->ped),edge))
	  {
	   if (dg[edge].type == dg_input || /* inputs can change direction */
	       dg[edge].level == LOOP_INDEPENDENT)
	     continue;
	   util_append(EdgeInfo->EdgeList,util_node_alloc(edge,NULL));
	  }
       }
     return(WALK_CONTINUE);
  }


/****************************************************************/
/*                                                              */
/*   Function:   CleanEdgeList                                  */
/*                                                              */
/*   Input:      EdgeList - list of preventing edges            */
/*               ped - dependence info, etc.                    */
/*               old_level - original level of a loop for which */
/*                           a position in memory order has     */
/*                           been chosen.                       */
/*                                                              */
/*   Description: Remove edges from the preventing edge list    */
/*                whose legality has been satisfied by the loop */
/*                at "old_level".                               */
/*                                                              */
/****************************************************************/

static void CleanEdgeList(UtilList *EdgeList,
			  PedInfo  ped,
			  int      old_level)

  {
   UtilNode *Edge;
   DG_Edge  *dg;
   int      dist;
   EDGE_INDEX edge;

     dg = dg_get_edge_structure( PED_DG(ped));
     Edge = UTIL_HEAD(EdgeList);
     while(Edge != NULLNODE)
       {
	edge = UTIL_NODE_ATOM(Edge);
	dist = gen_get_dt_DIS(&dg[edge],old_level);

	      /* only edges that are not carried by a loop at this 
		 point in memory order will be left on the list.
		 Remove any edges that will be carried by the loop
		 from "old_level" since their legality has been 
		 satisfied */

	switch(dist) {
	  case DDATA_NE: 
	  case DDATA_GE:
	  case DDATA_GT: break;

	  case DDATA_LT:
	  case DDATA_LE: util_pluck(Edge);
			 break;

	  case DDATA_ANY: if ((dg[edge].consistent == consistent_SIV ||
			       (dg[edge].consistent == consistent_MIV &&
				dg[edge].src == dg[edge].sink)) && 
			      NOT(dg[edge].symbolic))
			    util_pluck(Edge);
			  break;

	  case DDATA_ERROR: break;

	  default:
		  if (dist > 0)
		    util_pluck(Edge);
	       }
	Edge = UTIL_NEXT(Edge);
       }
  }


/****************************************************************/
/*                                                              */
/*   Function:   LegalPostion                                   */
/*                                                              */
/*   Input:      EdgeList - list of preventing edges            */
/*               ped - handle to dependence info                */
/*               new_level - desired level in permutation       */
/*               old_level - original level of loop             */
/*                                                              */
/*   Description:  Determine if loop in nest that was           */
/*                 at level "old_level" can be safely           */
/*                 moved to "new_level"                         */
/*                                                              */
/****************************************************************/

static Boolean LegalPosition(UtilList *EdgeList,
			     PedInfo  ped,
			     int      new_level,
			     int      old_level)
  {
   UtilNode *Edge;
   DG_Edge  *dg;
   int      dist;
   EDGE_INDEX edge;

    
     if (old_level == new_level)
       return(true);
     dg = dg_get_edge_structure( PED_DG(ped));
     Edge = UTIL_HEAD(EdgeList);
     while(Edge != NULLNODE)
       {
	edge = UTIL_NODE_ATOM(Edge);
	dist = gen_get_dt_DIS(&dg[edge],old_level);

	    /* look for a (>) or a distance < 0 at old_level in the
	       preventing edge list.  This means an illegal interchange */

	switch(dist) {
	  case DDATA_GE:
	  case DDATA_GT: 
	  case DDATA_NE:  return(false);
	                  
	  case DDATA_LT:
	  case DDATA_LE:  break;

	  case DDATA_ANY: if (dg[edge].consistent == inconsistent || 
			      (dg[edge].consistent == consistent_MIV &&
			       dg[edge].src != dg[edge].sink)  ||
			      dg[edge].symbolic)
	                        return(false);
			  break;

	  case DDATA_ERROR: return(false);

	  default:
	          if (dist < 0)
		    return(false);
	       }
	Edge = UTIL_NEXT(Edge);
       }
     return(true);
  }


/****************************************************************/
/*                                                              */
/*   Function:  ComputeLegalOrder                               */
/*                                                              */
/*   Input:     loop_data - loop structure                      */
/*              loop - index of innermost loop                  */
/*              num_loops - number of loops in perfect nest     */
/*              ped - handle to dependence graph                */
/*              heap - list of loops in Memory Order            */
/*              outermost_lvl - outermost loop level where      */
/*                              permutation can occur           */
/*              EdgeList - list of interchange preventing edges */
/*              ar - arena object for memory allocation         */
/*                                                              */
/*   Description:  Use the algorithm "NearbyPermutation" to     */
/*                 compute the closest legal approximation to   */
/*                 Memory Order.  (alg. form McKinley's thesis) */
/*                                                              */
/****************************************************************/
     
static void ComputeLegalOrder(model_loop *loop_data,
			      int        loop,
			      int        num_loops,
			      PedInfo    ped,
			      heap_type  *heap,
			      int        outermost_lvl,
			      UtilList   *EdgeList,
			      arena_type *ar)
  {
   heap_type *new_heap;
   Boolean loop_not_chosen, Legal;
   int i,j;
   Set LoopChosen;

   
     new_heap = (heap_type *)ar->arena_alloc_mem(LOOP_ARENA,MAXLOOP*
						 sizeof(heap_type));
     LoopChosen = ut_create_set(ar,LOOP_ARENA,num_loops);

              /* store heap in new_heap so that we have the original
		 memory order available for priority in level
		 placement.  */

     for (i = 0; i < num_loops; i ++)
       {
	new_heap[i].index = heap[i].index;
	new_heap[i].stride = heap[i].stride;
       }
     for (i = 0; i < num_loops; i++)
       {
	j = 0;
	loop_not_chosen = true;

	        /* In the set of loops that have not been chosen, look for
		   the loop that has the highest cost that can legally be
		   placed at level "i". */

	while (j < num_loops && loop_not_chosen)
	  {
	   if (!ut_member_number(LoopChosen,j))
	     if (outermost_lvl > i+1)
	       {

	        /* all levels outside of "outermost_lvl" must
		   remain in original order to preserve correctness */

		if (loop_data[new_heap[j].index].level == i+1)
		  {
		   heap[i] =  new_heap[j];
		   loop_not_chosen = false;
		   ut_add_number(LoopChosen,j);
		   CleanEdgeList(EdgeList,ped,i+1);
		  }
		if (NOT(LegalPosition(EdgeList,ped,i+1,
				      loop_data[new_heap[j].index].level)) ||
		    NOT(loop_data[new_heap[j].index].DependencesHandled))
		    /*set false in analyze.C if dependence not handled*/ 
		  {
		    /* this is checked for stats only */

		   loop_data[loop].interchange = false;
		   ut_add_number(loop_data[loop].PreventLvl[i],
				 loop_data[new_heap[j].index].level);
		  }
	       }

	   /* check if placement at level "i" does not change the */
	   /* direction of a dependence */

	   else 
	     {
	      Legal = LegalPosition(EdgeList,ped,i+1,
				    loop_data[new_heap[j].index].level);
	      if (NOT(Legal))
	        Legal = li_LoopReversal(loop_data,new_heap[j].index,EdgeList,
					ped);
	      if (Legal && loop_data[new_heap[j].index].DependencesHandled)
		{
		 heap[i] =  new_heap[j];
		 loop_not_chosen = false;
		 ut_add_number(LoopChosen,j);
		 CleanEdgeList(EdgeList,ped,
				 loop_data[new_heap[j].index].level);
		}
	      else
		{
		 loop_data[loop].interchange = false;
		 ut_add_number(loop_data[loop].PreventLvl[i],
			       loop_data[new_heap[j].index].level);
		}
	     }
	   j++;
	  }
       }

       /* store the original Memory Order and the best legal order
	  for statistics reporting */

     for (i = 0; i < num_loops; i++)
       {
	loop_data[loop].MemoryOrder[i] = loop_data[new_heap[i].index].level; 
	loop_data[loop].FinalOrder[i] = loop_data[heap[i].index].level; 
       }
  }


/****************************************************************/
/*                                                              */
/*   Function:   CheckInterchange                               */
/*                                                              */
/*   Input:      loop_data - loop structure                     */
/*               loop - index of innermost loop                 */
/*               num_loop - number of loops in perfect nest     */
/*               outermost_lvl - outermost level of legal       */
/*                               interchange                    */
/*               ped - handle to dependence information         */
/*               ar - arena object for memory allocation        */
/*               loop_list - list of loops in perfect nest      */
/*               heap - array to store loop permutation         */
/*                                                              */
/*   Description: Compute the RefGroups and cost for each loop  */
/*                in loop_list as if it were in the innermost   */
/*                position.   Based on those costs, compute the */
/*                legal permutation that is closest to memory   */
/*                order.                                        */
/*                                                              */
/****************************************************************/

static void CheckInterchange(model_loop    *loop_data,
			     int           loop,
			     int           num_loops,
			     int           outermost_lvl,
			     PedInfo       ped,
			     arena_type    *ar,
			     UtilList      *loop_list,
			     heap_type     *heap)

  {
   RefInfoType RefInfo;
   EdgeInfoType EdgeInfo;
   UtilNode    *lnode;
   int         i,loop1;

     RefInfo.loop_data = loop_data;
     RefInfo.ped = ped;
     RefInfo.dg = dg_get_edge_structure(PED_DG(ped));
     RefInfo.ar = ar;
     RefInfo.num_loops = num_loops;
     walk_expression(gen_DO_get_stmt_LIST(loop_data[loop].node),
		     UnsetVisitedMark,NOFUNC,(Generic)NULL);
     RefInfo.VisitedMark = true;
     loop_data[loop].OutermostLvl = outermost_lvl;

           /* for each loop in the perfect nest, compute RefGroups and
	      RefCost */

     for (lnode = UTIL_HEAD(loop_list);
	  lnode != NULLNODE;
	  lnode = UTIL_NEXT(lnode))
       {
	RefInfo.GroupList = util_list_alloc((Generic)NULL,"group-list");
	loop1 = UTIL_NODE_ATOM(lnode);
	RefInfo.level = loop_data[loop1].level;

	      /* compute RefGroups */

	walk_expression(gen_DO_get_stmt_LIST(loop_data[loop].node),
			(WK_EXPR_CLBACK)PartitionNames, NOFUNC,(Generic)&RefInfo);

	RefInfo.InvariantCost= 0.0;
	RefInfo.TemporalCost= 0.0;
	RefInfo.SpatialCost= 0.0;
	RefInfo.OtherSpatialCost= 0.0;
	RefInfo.NoneCost= 0.0;

	   /* compute costs */

	CheckRefGroups(loop_data,loop1,&RefInfo);

	util_append(loop_data[loop1].GroupList,
		    util_node_alloc((Generic)RefInfo.GroupList,NULL));

	  /* need a list of costs because a loop may be in more than
	     one perfect nest */

	loop_data[loop1].InvariantCostList.append_entry(RefInfo.InvariantCost);
	loop_data[loop1].TemporalCostList.append_entry(RefInfo.TemporalCost);
	loop_data[loop1].SpatialCostList.append_entry(RefInfo.SpatialCost);
	loop_data[loop1].OtherSpatialCostList.append_entry(RefInfo.
							   OtherSpatialCost);
	loop_data[loop1].NoneCostList.append_entry(RefInfo.NoneCost);
	RefInfo.VisitedMark = NOT(RefInfo.VisitedMark);
       }

            /* compute memory order */

     ComputeLoopOrder(loop_data,num_loops,ped,loop_list,heap);

     if (LoopsNotInOrder(loop_data,heap,num_loops))
       {
	EdgeInfo.ped = ped;
	EdgeInfo.EdgeList = util_list_alloc((Generic)NULL,"edge-list");

	    /* build list of edges that can prevent memory order */

	walk_expression(gen_DO_get_stmt_LIST(loop_data[loop].node),
			(WK_EXPR_CLBACK)GenerateEdgeList,NOFUNC,(Generic)&EdgeInfo);
	for (i = 0; i < num_loops; i++)
	  loop_data[loop].PreventLvl[i] = ut_create_set(ar,LOOP_ARENA,
							   num_loops);

	       /* compute NearbyPermutation */

	ComputeLegalOrder(loop_data,loop,num_loops,ped,heap,outermost_lvl,
			  EdgeInfo.EdgeList,ar);
       }
     else
       for (i = 0; i < num_loops; i++)
	 {
	  loop_data[loop].FinalOrder[i] = loop_data[heap[i].index].level; 
	  loop_data[loop].MemoryOrder[i] = loop_data[heap[i].index].level; 
	 }
  }

static void CountDistribution(model_loop *loop_data,
			      int        loop)

  {
   AST_INDEX stmt;
   Boolean OtherStmts;
   
     loop_data[loop].Distribute = true;
     stmt = list_first(gen_DO_get_stmt_LIST(loop_data[loop].node));

       /* count blocks of statements around which we distribute the loop.
	  a block consists of a list of non-do statements or a just one do-statement */

     while (stmt != AST_NIL)
       {
	loop_data[loop].DistributeNumber++;
	OtherStmts = false;
	while (!is_do(stmt) && stmt != AST_NIL)
	  {
	   OtherStmts = true;
	   stmt = list_next(stmt);
	  }
	if (is_do(stmt))
	  {
	   if (OtherStmts)
	     loop_data[loop].DistributeNumber++;
	   stmt = list_next(stmt);
	  }
       }
  }



/****************************************************************/
/*                                                              */
/*   Function:   walk_loops                                     */
/*                                                              */
/*   Input:      loop_data - loop structure                     */
/*               loop - current loop being examined             */
/*               outermost_lvl  - outermost nesting level where */
/*                                interchange is safe           */
/*               num_loops - number of loops in perfect nest    */
/*               loop_list - list of loops in perfect nest      */
/*               heap - storage for loop order                  */
/*               symtab - symbol table                          */
/*               ped - handle for dependence graph              */
/*               ar - arena object for memory allocation        */
/*                                                              */
/*   Description:  Walk loop_data structure and at each         */
/*                 innermost loop compute memory order for the  */
/*                 perfect nest of loops surrounding that loop  */
/*                                                              */
/****************************************************************/

static void walk_loops(model_loop    *loop_data,
		       int           loop,
		       int           outermost_lvl,
		       int           num_loops,
		       UtilList      *loop_list,
		       heap_type     *heap,
		       SymDescriptor symtab,
		       PedInfo       ped,
		       arena_type    *ar)
		       
  {
   int next;

     util_append(loop_list,util_node_alloc(loop,"loop node"));
     if (!loop_data[loop].transform || (!loop_data[loop].distribute && 
					!loop_data[loop].expand) || 
	 loop_data[loop].type == COMPLEX || loop_data[loop].type == TRAP ||
	 loop_data[loop].type == MULT)

           /* illegal distribution and complex iteration space shapes prevent
	      interchange at this level and out.  This is a sufficient 
	      condition for safety, not necessary */

       outermost_lvl = loop_data[loop].level+1;
     if (loop_data[loop].inner_loop == -1)

               /* compute best loop order */

       CheckInterchange(loop_data,loop,num_loops,outermost_lvl,
			ped,ar,loop_list,heap);
     else
       {
	next = loop_data[loop].inner_loop;
	while (next != -1)
	  {
	   walk_loops(loop_data,next,outermost_lvl,num_loops+1,loop_list,heap,
		      symtab,ped,ar);
	   next = loop_data[next].next_loop;
	   if (next != -1)

	           /* create a heap for a new perfect loop nest */

	     {
	      loop_data[next].heap=(heap_type *)ar->arena_alloc_mem(LOOP_ARENA,
								    MAXLOOP * 
							   sizeof(heap_type));
	      heap = loop_data[next].heap;
	     }
	  }
       }
     util_pluck(UTIL_TAIL(loop_list));
  }


/****************************************************************/
/*                                                              */
/*   Function:  li_ComputeMemoryOrder                           */
/*                                                              */
/****************************************************************/

void li_ComputeMemoryOrder(model_loop    *loop_data,
			   SymDescriptor symtab,
			   PedInfo       ped,
			   arena_type    *ar)

  {
   UtilList      *loop_list;

     loop_list = util_list_alloc((Generic)NULL,"loop-list");
     loop_data[0].heap = (heap_type *)ar->arena_alloc_mem(LOOP_ARENA,
							  MAXLOOP*
							  sizeof(heap_type));

           /* compute permutatation information */

     walk_loops(loop_data,0,0,1,loop_list,loop_data[0].heap,symtab,ped,ar);

     util_list_free(loop_list);
  }
