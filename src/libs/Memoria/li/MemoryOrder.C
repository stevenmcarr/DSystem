/* $Id: MemoryOrder.C,v 1.8 1998/09/29 20:44:02 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

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
#include <libs/support/misc/general.h>
#endif 

#include <libs/Memoria/include/mh.h>
#include <libs/Memoria/include/mh_ast.h>
#include <libs/frontEnd/include/walk.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>
#include <libs/Memoria/include/mh_config.h>
#include <libs/Memoria/li/MemoryOrder.h>
#include <libs/Memoria/li/Reversal.h>

#ifndef header_h
#include <libs/Memoria/include/header.h>
#endif

#ifndef dt_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dt.h>
#endif

#ifndef gi_h
#include <libs/frontEnd/include/gi.h>
#endif 

#ifndef stats_h
#include <libs/Memoria/li/stats.h>
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

#ifndef LoopStats_h
#include <libs/Memoria/include/LoopStats.h>
#endif 

#ifndef dg_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dg.h>
#endif 
	      
#include <libs/Memoria/include/RefGroups.h>


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
   int         i,n,loop1;
   char        *temp;
   RefGroupSet *RefGroups;

     RefInfo.loop_data = loop_data;
     RefInfo.ped = ped;
     RefInfo.dg = dg_get_edge_structure(PED_DG(ped));
     RefInfo.ar = ar;
     RefInfo.num_loops = num_loops;
     RefInfo.IVar = new char*[loop_data[loop].level];
     loop_data[loop].OutermostLvl = outermost_lvl;

           /* for each loop in the perfect nest, compute RefGroups and
	      RefCost */
     for (lnode = UTIL_HEAD(loop_list),n = 0;
	  lnode != NULLNODE;
	  lnode = UTIL_NEXT(lnode), n++)
       {
	loop1 = UTIL_NODE_ATOM(lnode);
	RefInfo.IVar[n] = gen_get_text(
			    gen_INDUCTIVE_get_name(
			      gen_DO_get_control(loop_data[loop1].node)));
       }
     n--;
     for (lnode = UTIL_HEAD(loop_list);
	  lnode != NULLNODE;
	  lnode = UTIL_NEXT(lnode))
       {
	loop1 = UTIL_NODE_ATOM(lnode);
	RefInfo.loop = loop1;
	RefInfo.level = loop_data[loop1].level;
	RefInfo.InvariantCost= 0.0;
	RefInfo.TemporalCost= 0.0;
	RefInfo.SpatialCost= 0.0;
	RefInfo.GroupSpatialCost= 0.0;
	RefInfo.NoneCost= 0.0;

	      /* compute RefGroups */
	RefGroups = new RefGroupSet(loop_data[loop].node,loop_data[loop].level,
				    RefInfo);


	util_append(loop_data[loop1].GroupList,
		    util_node_alloc((Generic)RefGroups,NULL));

	  /* need a list of costs because a loop may be in more than
	     one perfect nest */

	loop_data[loop1].InvariantCostList.append_entry(RefInfo.InvariantCost);
	loop_data[loop1].TemporalCostList.append_entry(RefInfo.TemporalCost);
	loop_data[loop1].SpatialCostList.append_entry(RefInfo.SpatialCost);
	loop_data[loop1].OtherSpatialCostList.append_entry(RefInfo.
							   GroupSpatialCost);

	loop_data[loop1].NoneCostList.append_entry(RefInfo.NoneCost);

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
     delete RefInfo.IVar;
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
