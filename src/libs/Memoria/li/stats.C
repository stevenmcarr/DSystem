/* $Id: stats.C,v 1.3 1992/12/07 10:14:59 carr Exp $ */
#include <mh.h>
#include <fort/gi.h>
#include <stats.h>
#include <analyze.h>
#include <shape.h>
#include <mem_util.h>
#include <mark.h>
#include <LoopStats.h>

#include	<dg.h>		/* dg_add_edge()		*/



static int RemoveEdges(AST_INDEX stmt,
			int       level,
			PedInfo   ped)

  {
   DG_Edge    *dg;
   int        vector;
   EDGE_INDEX edge,
              next_edge;
   int        i;

     dg = dg_get_edge_structure( PED_DG(ped));
     vector = get_info(ped,stmt,type_levelv);
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
     for (edge = dg_first_src_stmt( PED_DG(ped),vector,LOOP_INDEPENDENT);
	  edge != END_OF_LIST;
	  edge = next_edge)
       {
	next_edge = dg_next_src_stmt( PED_DG(ped),edge);
	if (dg[edge].type == dg_exit || dg[edge].type == dg_io ||
	    dg[edge].type == dg_call || dg[edge].type == dg_control ||
	    dg[edge].src == dg[edge].sink ||
	    (ut_get_stmt(dg[edge].src) == ut_get_stmt(dg[edge].sink) &&
	     dg[edge].type == dg_true))
	  dg_delete_free_edge( PED_DG(ped),edge);
       }
     for (edge = dg_first_sink_stmt( PED_DG(ped),vector,LOOP_INDEPENDENT);
	  edge != END_OF_LIST;
	  edge = next_edge)
       {
	next_edge = dg_next_sink_stmt( PED_DG(ped),edge);
	if (dg[edge].type == dg_exit || dg[edge].type == dg_io ||
	    dg[edge].type == dg_call || dg[edge].type == dg_control ||
	    dg[edge].src == dg[edge].sink ||
	    (ut_get_stmt(dg[edge].src) == ut_get_stmt(dg[edge].sink) &&
	     dg[edge].type == dg_true))
	   dg_delete_free_edge( PED_DG(ped),edge);
       }
     return(WALK_CONTINUE);
  }


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

static Boolean OnlyInInnermostPosition(model_loop *loop_data,
				       AST_INDEX  node,
				       int        level)
  {
   AST_INDEX sub_list,sub;
   char *var;
   int coeff,words;
   Boolean lin;
   
     if (level == LOOP_INDEPENDENT)
       return(false);
     sub_list = gen_SUBSCRIPT_get_rvalue_LIST(tree_out(node));
     sub = list_first(sub_list);
     var = FindInductionVar(loop_data,node,level);
     if (pt_find_var(sub,var) && NotInOtherPositions(sub_list,var))
       {
	pt_get_coeff(sub,var,&lin,&coeff);
	if (coeff < 0)
	  coeff = -coeff;
	return(coeff < words && lin);
       }
     return(false);
  }

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


static void DoPartition(AST_INDEX name,
			RefGroupType *RefGroup,
			DG_Edge   *dg,
			PedInfo   ped,
			int       level,
			Boolean   VisitedMark,
			model_loop *loop_data)
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
 

  {
   subscript_info_type *sptr1,*sptr2;
   int              refl;
   EDGE_INDEX       edge;

     sptr1 = get_subscript_ptr(name);
     sptr1->visited = VisitedMark;
     sptr1->lnode = util_node_alloc(name,NULL);
     util_append(RefGroup->RefList,sptr1->lnode);
     RefGroup->number++;
     refl = get_info(ped,name,type_levelv);
     for (edge = dg_first_src_ref( PED_DG(ped),refl);
	  edge != END_OF_LIST;
	  edge = dg_next_src_ref( PED_DG(ped),edge))
       if ((dg[edge].consistent == consistent_SIV ||
	    (dg[edge].consistent == consistent_MIV && 
	     (dg[edge].level == LOOP_INDEPENDENT ||
	      dg[edge].src == dg[edge].sink))) &&
	   NOT(dg[edge].symbolic) &&
	   (CanMoveToInnermost(&dg[edge]) && 
	    (dg[edge].level == level || 
	     dg[edge].level == LOOP_INDEPENDENT ||
	     OnlyInInnermostPosition(loop_data,dg[edge].src,dg[edge].level))))
	 {
	  sptr2 = get_subscript_ptr(dg[edge].sink);
	  if(sptr2->visited != VisitedMark &&
	     sptr1->surrounding_do == sptr2->surrounding_do &&
	     !is_call(ut_get_stmt(dg[edge].sink)))
	    DoPartition(dg[edge].sink,RefGroup,dg,ped,level,VisitedMark,
			loop_data);
	 }
     for (edge = dg_first_sink_ref( PED_DG(ped),refl);
	  edge != END_OF_LIST;
	  edge = dg_next_sink_ref( PED_DG(ped),edge))
       if ((dg[edge].consistent == consistent_SIV ||
	    (dg[edge].consistent == consistent_MIV && 
	     dg[edge].level == LOOP_INDEPENDENT ||
	     dg[edge].src == dg[edge].sink)) &&
	   NOT(dg[edge].symbolic) &&
	   (CanMoveToInnermost(&dg[edge]) && 
	    (dg[edge].level == level || 
	     dg[edge].level == LOOP_INDEPENDENT ||
	     OnlyInInnermostPosition(loop_data,dg[edge].src,dg[edge].level))))
	 {
	  sptr2 = get_subscript_ptr(dg[edge].src);
	  if(sptr2->visited != VisitedMark &&
	     sptr1->surrounding_do == sptr2->surrounding_do &&
	     !is_call(ut_get_stmt(dg[edge].sink)))
	    DoPartition(dg[edge].src,RefGroup,dg,ped,level,VisitedMark,
			loop_data);
	 }
  }

static int PartitionNames(AST_INDEX   node,
			   RefInfoType *RefInfo)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

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
		       RefInfo->level,RefInfo->VisitedMark,RefInfo->loop_data);
	  }
       }
    return(WALK_CONTINUE);
  }

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
        for (edge = dg_first_sink_ref( PED_DG(ped),refl);
             edge != END_OF_LIST;
             edge = dg_next_sink_ref( PED_DG(ped),edge))
          if (dg[edge].src != dg[edge].sink &&
	      ((!pt_expr_equal(tree_out(dg[edge].src),tree_out(dg[edge].sink)) 
		&& gen_get_dt_DIS(&dg[edge],dg[edge].level) != DDATA_ANY) || 
	       dg[edge].level == LOOP_INDEPENDENT) &&
	      get_subscript_ptr(dg[edge].src)->surrounding_do ==
	      get_subscript_ptr(dg[edge].sink)->surrounding_do &&
	      (dg[edge].consistent == consistent_SIV ||
	       (dg[edge].consistent == consistent_MIV && 
		dg[edge].level == LOOP_INDEPENDENT)) &&
	      NOT(dg[edge].symbolic))
            if (util_in_list(get_subscript_ptr(dg[edge].src)->lnode,
			     RefGroup->RefList))
              {
               found = false;
               break;
              }
        if (found)
          break;
       }
     return(UTIL_NODE_ATOM(RefNode));
  }

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
       if ((dg[edge].level == loop_data[loop].level  ||
	    dg[edge].level == LOOP_INDEPENDENT) && 
	   CanMoveToInnermost(&dg[edge]))
	 {
	  RefGroup->Temporal = true;
	  if (NOT(RefGroup->Spatial) && NOT(RefGroup->Invariant))
	    (*TemporalCost) += 1.0;
	 }
  }

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
     for (edge = dg_first_src_ref( PED_DG(ped),refl);
	  edge != END_OF_LIST && NOT(RefGroup->OtherSpatial);
	  edge = dg_next_src_ref( PED_DG(ped),edge))
       if (dg[edge].level != loop_data[loop].level && 
	   dg[edge].level != LOOP_INDEPENDENT &&
	   CanMoveToInnermost(&dg[edge]))
	 {
	  if (OnlyInInnermostPosition(loop_data,node,dg[edge].level))
	    {
	     RefGroup->OtherSpatial = true;
	     if (NOT(RefGroup->Temporal) && NOT(RefGroup->Invariant))
	      (*OtherSpatialCost) += 1.0;
	    }
	 }
  }

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
	if (pt_find_var(sub_list,var))
	  {
	   sub = list_first(sub_list);
	   if (pt_find_var(sub,var) && NotInOtherPositions(sub_list,var))
	     {
	      pt_get_coeff(sub,var,&lin,&coeff);	
	      if (coeff < 0)
	        coeff = -coeff;
	      if (coeff < words && lin)
		{
		 RefGroup->Spatial = true;
		 RefInfo->SpatialCost += (1.0/((float)(words)/(float)coeff));
		}
	     }
	  }
	else 
	  {
	   node = FindOldestValue(RefGroup,RefInfo->ped);
	   CheckInvariant(node,RefInfo->ped,loop_data,loop,
			  &RefInfo->InvariantCost,RefGroup);
	  }
	if (UTIL_HEAD(RefGroup->RefList) != UTIL_TAIL(RefGroup->RefList))
	  {
	   node = FindOldestValue(RefGroup,RefInfo->ped);
	   CheckTemporal(node,RefInfo->ped,loop_data,loop,
			 &RefInfo->TemporalCost,RefGroup);
	   CheckOtherSpatial(node,RefInfo->ped,loop_data,loop,
			     &RefInfo->OtherSpatialCost,RefGroup,words);
	  }
	if (NOT(RefGroup->Invariant) && NOT(RefGroup->Spatial) &&
	    NOT(RefGroup->OtherSpatial) && NOT(RefGroup->Temporal))
	   RefInfo->NoneCost += 1.0;
       }
  }


static Boolean LoopsNotInOrder(model_loop *loop_data,
			       heap_type  *heap,
			       int        num_loops)

  {
   int i;
   
     for (i = 0; i < num_loops; i++)
       if (loop_data[heap[i].index].level != i+1)
         return(true);
     return(false);
  }


static void Heapify(heap_type *heap,
		    int       i,
		    int       j,
		    int       n)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

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

static int GetLoopCost(float TemporalCost,
		       float SpatialCost,
                       float OtherSpatialCost,
		       float InvariantCost,
		       float NoneCost,
		       int   num_loops)

  {
     return((int)((TemporalCost     * 100.0 +
		   SpatialCost      * 100.0 +
		   OtherSpatialCost * 100.0 +
		   NoneCost         * 100.0 +
		   InvariantCost) * 10.0));
  }

static int GetLoopCostFirst(model_loop *loop_data,
			   int        j,
			   int        num_loops)

  {
   return
     GetLoopCost(loop_data[j].TemporalCostList.first_entry()->GetValue(),
		 loop_data[j].SpatialCostList.first_entry()->GetValue(),
		 loop_data[j].OtherSpatialCostList.first_entry()->GetValue(),
		 loop_data[j].InvariantCostList.first_entry()->GetValue(),
		 loop_data[j].NoneCostList.first_entry()->GetValue(),
		 num_loops);
  }
     

static int GetLoopCostLast(model_loop *loop_data,
			  int        j,
			  int        num_loops)

  {
   return
     GetLoopCost(loop_data[j].TemporalCostList.last_entry()->GetValue(),
		 loop_data[j].SpatialCostList.last_entry()->GetValue(),
		 loop_data[j].OtherSpatialCostList.last_entry()->GetValue(),
		 loop_data[j].InvariantCostList.last_entry()->GetValue(),
		 loop_data[j].NoneCostList.last_entry()->GetValue(),
		 num_loops);
  }

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
	heap[i].stride = GetLoopCostLast(loop_data,j,num_loops);
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

static int GeneratePreventingEdgeList(AST_INDEX node,
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
	   if (dg[edge].type == dg_input) 
	     continue;
	   less = false;
	   i = dg[edge].level + 1;
	   while(i <= gen_get_dt_LVL(&dg[edge]) && !less)
	     {
	      dist = gen_get_dt_DIS(&dg[edge],i);
	      switch(dist) {
		case DDATA_GE:
		case DDATA_GT: break;
			       
		case DDATA_LT:
	        case DDATA_LE:
	        case DDATA_NE:  less = true;
	                        break;

	        case DDATA_ANY: if (dg[edge].consistent == inconsistent || 
				    (dg[edge].consistent == consistent_MIV &&
				     dg[edge].src != dg[edge].sink) ||
				    dg[edge].symbolic)
	                          less = true;
	                        break;

	        case DDATA_ERROR: break;

	        default:
	          if (dist < 0)
	            less = true;
	       }
	      i++;
	      if (less)
	        util_append(EdgeInfo->EdgeList,util_node_alloc(edge,NULL));
	     }
	  }
       }
     return(WALK_CONTINUE);
  }


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
	switch(dist) {
	  case DDATA_GE:
	  case DDATA_GT: util_pluck(Edge);
			 break;
	  case DDATA_LT:
	  case DDATA_LE:
	  case DDATA_NE: break;

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
	switch(dist) {
	  case DDATA_GE:
	  case DDATA_GT: break;

	  case DDATA_LT:
	  case DDATA_LE:
	  case DDATA_NE:  return(false);
	                  break;

	  case DDATA_ANY: if (dg[edge].consistent == inconsistent || 
			      (dg[edge].consistent == consistent_MIV &&
			       dg[edge].src != dg[edge].sink)  ||
			      dg[edge].symbolic)
	                        return(false);
			  break;

	  case DDATA_ERROR: return(false);
			    break;

	  default:
	          if (dist < 0)
		    return(false);
	       }
	Edge = UTIL_NEXT(Edge);
       }
     return(true);
  }
	
     
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
   Boolean loop_not_chosen;
   int i,j;
   Set LoopChosen;

   
     new_heap = (heap_type *)ar->arena_alloc_mem(LOOP_ARENA,MAXLOOP*
						 sizeof(heap_type));
     LoopChosen = ut_create_set(ar,LOOP_ARENA,num_loops);
     for (i = 0; i < num_loops; i ++)
       {
	new_heap[i].index = heap[i].index;
	new_heap[i].stride = heap[i].stride;
       }
     for (i = 0; i < num_loops; i++)
       {
	j = 0;
	loop_not_chosen = true;
	while (j < num_loops && loop_not_chosen)
	  {
	   if (!ut_member_number(LoopChosen,j))
	     if (outermost_lvl > i+1)
	       if (loop_data[new_heap[j].index].level == i+1)
		 {
		  heap[i] =  new_heap[j];
		  loop_not_chosen = false;
		  ut_add_number(LoopChosen,j);
		  CleanEdgeList(EdgeList,ped,i+1);
		 }
	       else if (NOT(LegalPosition(EdgeList,ped,i+1,
					  loop_data[new_heap[j].index].level)))
		 {
		  loop_data[loop].interchange = false;
		  ut_add_number(loop_data[loop].PreventLvl[i],
				loop_data[new_heap[j].index].level);
		 }
	       else;
	     else if (LegalPosition(EdgeList,ped,i+1,
				    loop_data[new_heap[j].index].level))
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
	   j++;
	  }
       }
     for (i = 0; i < num_loops; i++)
       {
	loop_data[loop].MemoryOrder[i] = loop_data[new_heap[i].index].level; 
	loop_data[loop].FinalOrder[i] = loop_data[heap[i].index].level; 
       }
  }


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
     for (lnode = UTIL_HEAD(loop_list);
	  lnode != NULLNODE;
	  lnode = UTIL_NEXT(lnode))
       {
	RefInfo.GroupList = util_list_alloc((Generic)NULL,"group-list");
	loop1 = UTIL_NODE_ATOM(lnode);
	RefInfo.level = loop_data[loop1].level;
	walk_expression(gen_DO_get_stmt_LIST(loop_data[loop].node),
			PartitionNames, NOFUNC,(Generic)&RefInfo);
	RefInfo.InvariantCost= 0.0;
	RefInfo.TemporalCost= 0.0;
	RefInfo.SpatialCost= 0.0;
	RefInfo.OtherSpatialCost= 0.0;
	RefInfo.NoneCost= 0.0;
	CheckRefGroups(loop_data,loop1,&RefInfo);
	util_append(loop_data[loop1].GroupList,
		    util_node_alloc((Generic)RefInfo.GroupList,NULL));
	loop_data[loop1].InvariantCostList.append_entry(RefInfo.InvariantCost);
	loop_data[loop1].TemporalCostList.append_entry(RefInfo.TemporalCost);
	loop_data[loop1].SpatialCostList.append_entry(RefInfo.SpatialCost);
	loop_data[loop1].OtherSpatialCostList.append_entry(RefInfo.
							   OtherSpatialCost);
	loop_data[loop1].NoneCostList.append_entry(RefInfo.NoneCost);
	RefInfo.VisitedMark = NOT(RefInfo.VisitedMark);
       }
     ComputeLoopOrder(loop_data,num_loops,ped,loop_list,heap);
     if (LoopsNotInOrder(loop_data,heap,num_loops))
       {
	EdgeInfo.ped = ped;
	EdgeInfo.EdgeList = util_list_alloc((Generic)NULL,"edge-list");
	walk_expression(gen_DO_get_stmt_LIST(loop_data[loop].node),
			GeneratePreventingEdgeList,NOFUNC,(Generic)&EdgeInfo);
	if (outermost_lvl > 1 || NOT(util_list_empty(EdgeInfo.EdgeList)))
	  {
	   for (i = 0; i < num_loops; i++)
	     loop_data[loop].PreventLvl[i] = ut_create_set(ar,LOOP_ARENA,
							   num_loops);
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
     else
       for (i = 0; i < num_loops; i++)
	 {
	  loop_data[loop].FinalOrder[i] = loop_data[heap[i].index].level; 
	  loop_data[loop].MemoryOrder[i] = loop_data[heap[i].index].level; 
	 }
  }
   

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
       outermost_lvl = loop_data[loop].level+1;
     if (loop_data[loop].inner_loop == -1)
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


static void DumpLoopStats(model_loop *loop_data,
			  int        loop,
			  int        num_loops,
			  UtilList   *GroupList,
			  LoopStatsType *LoopStats,
			  FILE       *logfile)
  {
   int i;
   UtilNode *GroupNode;
   RefGroupType *RefGroup;

     fprintf(logfile,"Statistics for Loop at Level %d\n",
	     loop_data[loop].level);
     fprintf(logfile,"Induction variable %s\n\n",
	     gen_get_text(gen_INDUCTIVE_get_name(gen_DO_get_control(
			 loop_data[loop].node))));
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
     fprintf(logfile,"\tLoop Cost %d\n\n\n",GetLoopCostFirst(loop_data,loop,
							     num_loops));
  }


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

static void DumpPermutationStats(model_loop *loop_data,
				 int        loop,
				 int        num_loops,
				 int        *inner_loops,
				 LoopStatsType *LoopStats,
				 FILE       *logfile)

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
         MemoryCost;
   LocalityMatrixType  
      OriginalLocalityMatrix = {{0,0,0},{0,0,0},{0,0,0}},
      FinalLocalityMatrix = {{0,0,0},{0,0,0},{0,0,0}},
      MemoryLocalityMatrix = {{0,0,0},{0,0,0},{0,0,0}};
   Boolean memory_order = true,
           interchanged = false,
           TimeStep     = false,
           Complex      = false,
           Distribution = false,
           Nearby       = false,
           InOrder      = true;
   UtilNode *GroupNode;
   RefGroupType *RefGroup;

     fprintf(logfile,"Permutation Statistic for Perfect Loop Nest %d\n\n",
	     *inner_loops);
     for (i = loop;
	  i != -1;
	  i = loop_data[i].parent)
       DumpLoopStats(loop_data,i,num_loops,
		     (UtilList *)UTIL_NODE_ATOM(UTIL_HEAD(loop_data[i].
							  GroupList)),
		     LoopStats,logfile);
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
     fprintf(logfile,"Other Spatial %d\n\n\n",
		OriginalOtherSpatialGroups);
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
     fprintf(logfile,"Other Spatial %d\n\n\n",
		FinalOtherSpatialGroups);
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
     fprintf(logfile,"Other Spatial %d\n\n\n",
		MemoryOtherSpatialGroups);
     depth = loop_data[loop].level;
     FinalCost = GetLoopCostFirst(loop_data,FinalLoop,depth);
     MemoryCost = GetLoopCostFirst(loop_data,MemoryLoop,depth);
     OriginalCost = GetLoopCostFirst(loop_data,loop,depth);
     if (FinalCost != 0)
       {
	LoopStats->FinalRatio[depth-1] += 
	    (((float)OriginalCost)/((float)FinalCost));
	fprintf(logfile,"\tFinal Ratio = %.2f\n",
		((float)OriginalCost)/((float)FinalCost));
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


static void WalkLoopsForStats(model_loop *loop_data,
			      int        loop,
			      int        num_loops,
			      int        *inner_loops,
			      PedInfo    ped,
			      LoopStatsType *LoopStats)
  {
   int next,temp;

     LoopStats->TotalLoops++;
     if (loop_data[loop].inner_loop == -1)
       {
	*inner_loops = *inner_loops + 1;
	DumpPermutationStats(loop_data,loop,num_loops,inner_loops,LoopStats,
			     ((config_type *)PED_MH_CONFIG(ped))->logfile);
       }
     else
       {
	next = loop_data[loop].inner_loop;
	LoopStats->Perfect = (LoopStats->Perfect && 
			      loop_data[next].next_loop == -1);
	while (next != -1)
	  {
	   WalkLoopsForStats(loop_data,next,num_loops+1,inner_loops,ped,
			     LoopStats);
	   next = loop_data[next].next_loop;
	  }
       }
  }


void memory_interchange_stats(PedInfo       ped,
			      AST_INDEX     root,
			      int           level,
			      LoopStatsType *LoopStats,
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
     walk_statements(root,level,ut_mark_do_pre,ut_mark_do_post,
		     (Generic)&pre_info);
     if (pre_info.abort)
       return;
     walk_statements(root,level,RemoveEdges,NOFUNC,(Generic)ped);
     loop_data = (model_loop *)ar->arena_alloc_mem_clear(LOOP_ARENA,
					 pre_info.loop_num*sizeof(model_loop));
     ut_analyze_loop(root,loop_data,level,ped,symtab);
     if (loop_data[0].inner_loop == -1)
       return;
     ut_check_shape(loop_data,0);
     loop_list = util_list_alloc((Generic)NULL,"loop-list");
     loop_data[0].heap = (heap_type *)ar->arena_alloc_mem(LOOP_ARENA,
							  MAXLOOP*
							  sizeof(heap_type));
     walk_loops(loop_data,0,0,1,loop_list,loop_data[0].heap,symtab,ped,ar);
     util_list_free(loop_list);
     inner_loops = 0;
     fprintf(((config_type *)PED_MH_CONFIG(ped))->logfile,
	     "*********************************************************\n\n");
     fprintf(((config_type *)PED_MH_CONFIG(ped))->logfile,
	     "Statistics for New Loop Nest\n\n");
     WalkLoopsForStats(loop_data,0,1,&inner_loops,ped,LoopStats);
  }
