#include <mh.h>
#include <gi.h>
#include "stats.h"
#include <analyze.h>
#include <shape.h>
#include <mem_util.h>
#include <mark.h>

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
   
     sub_list = gen_SUBSCRIPT_get_rvalue_LIST(tree_out(node));
     sub = list_first(sub_list);
     var = FindInductionVar(loop_data,node,level);
     if (pt_find_var(sub,var) && NotInOtherPositions(sub_list,var))
       {
	pt_get_coeff(sub,var,&lin,&coeff);
	return(coeff < words && lin);
       }
     return(false);
  }

static Boolean CanMoveToInnermost(DG_Edge *edge)

  {
   int i;
   
     if (edge->level == LOOP_INDEPENDENT)
       return(true);
     for (i = edge->level; i < gen_get_dt_LVL(edge);i++)
       if (gen_get_dt_DIS(edge,i) != 0)
         return(false);
     return(true);
  }


static void DoPartition(AST_INDEX name,
			RefGroupType *RefGroup,
			DG_Edge   *dg,
			PedInfo   ped,
			int       level,
			model_loop *loop_data)
/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
 

  {
   subscript_info_type *sptr;
   int              refl;
   EDGE_INDEX       edge;

     sptr = get_subscript_ptr(name);
     sptr->visited = true;
     sptr->lnode = util_node_alloc(name,NULL);
     util_append(RefGroup->RefList,sptr->lnode);
     RefGroup->number++;
     refl = get_info(ped,name,type_levelv);
     for (edge = dg_first_src_ref( PED_DG(ped),refl);
	  edge != END_OF_LIST;
	  edge = dg_next_src_ref( PED_DG(ped),edge))
       if (dg[edge].consistent != inconsistent &&
	   (CanMoveToInnermost(&dg[edge]) && 
	    (dg[edge].level == level || 
	     OnlyInInnermostPosition(loop_data,dg[edge].src,dg[edge].level))))
	 {
	  sptr = get_subscript_ptr(dg[edge].sink);
	  if(!sptr->visited)
	    DoPartition(dg[edge].sink,RefGroup,dg,ped,level,loop_data);
	 }
     for (edge = dg_first_sink_ref( PED_DG(ped),refl);
	  edge != END_OF_LIST;
	  edge = dg_next_sink_ref( PED_DG(ped),edge))
       if (dg[edge].consistent != inconsistent &&
	   (CanMoveToInnermost(&dg[edge]) && 
	    (dg[edge].level == level || 
	     OnlyInInnermostPosition(loop_data,dg[edge].src,dg[edge].level))))
	 {
	  sptr = get_subscript_ptr(dg[edge].src);
	  if(!sptr->visited)
	    DoPartition(dg[edge].src,RefGroup,dg,ped,level,loop_data);
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

     if (is_subscript(node))
       {
	name = gen_SUBSCRIPT_get_name(node);
	sptr = get_subscript_ptr(name);
	if (!sptr->visited)
	  {
	   sptr->visited = true;
	   RefGroup = (RefGroupType *)RefInfo->
	                  ar->arena_alloc_mem_clear(LOOP_ARENA,
						    sizeof(RefGroupType));
	   RefGroup->number = 0;
	   util_append(RefInfo->GroupList,util_node_alloc((Generic)RefGroup,
							"reference"));
	   RefGroup->RefList = util_list_alloc((Generic)NULL,"ref-list");
	   DoPartition(name,RefGroup,RefInfo->dg,RefInfo->ped,
		       RefInfo->level,RefInfo->loop_data);
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
	      (!pt_expr_equal(tree_out(dg[edge].src),tree_out(dg[edge].sink)) 
	       || dg[edge].level == LOOP_INDEPENDENT) &&
	      get_subscript_ptr(dg[edge].src)->surrounding_do ==
	      get_subscript_ptr(dg[edge].sink)->surrounding_do)
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
	  if (NOT(RefGroup->Spatial))
	    loop_data[loop].TemporalCost += 1.0;
	 }
  }

static void CheckOtherSpatial(AST_INDEX  node,
			      PedInfo    ped,
			      model_loop *loop_data,
			      int        loop,
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
	     if (NOT(RefGroup->Temporal))
	       loop_data[loop].OtherSpatialCost += 1.0;
	    }
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
	      if (coeff < words && lin)
		{
		 RefGroup->Spatial = true;
		 loop_data[loop].SpatialCost +=
		    1.0/((float)(words)/(float)coeff);
		}
	     }
	   if (UTIL_HEAD(RefGroup->RefList) != UTIL_TAIL(RefGroup->RefList))
	     {
	      node = FindOldestValue(RefGroup,RefInfo->ped);
	      CheckTemporal(node,RefInfo->ped,loop_data,loop,RefGroup);
	      CheckOtherSpatial(node,RefInfo->ped,loop_data,loop,RefGroup,
				words);
	     }
	  }
	else
	  {
	   RefGroup->Invariant = true;
	   loop_data[loop].InvariantCost += 1.0;
	  }
	if (NOT(RefGroup->Invariant) && NOT(RefGroup->Spatial) &&
	    NOT(RefGroup->OtherSpatial) && NOT(RefGroup->Temporal))
	   loop_data[loop].NoneCost += 1.0;
       }
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
	   if (heap[i1].stride < heap[i2].stride)
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
	heap[i].stride = (int)((loop_data[j].TemporalCost * (num_loops * 10.0)
			       + loop_data[j].SpatialCost * (num_loops * 10.0)
			       + loop_data[j].OtherSpatialCost * 
				                            (num_loops * 10.0)
			       + loop_data[j].InvariantCost * ((num_loops-1)
							      * 10.0)
			       + loop_data[j].NoneCost * (num_loops * 10.0))
			       * 10.0);
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

	        case DDATA_ANY: if (dg[edge].consistent != consistent_SIV || 
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
	Edge = UTIL_NEXT(Edge);
	edge = UTIL_NODE_ATOM(Edge);
	dist = gen_get_dt_DIS(&dg[edge],old_level);
	switch(dist) {
	  case DDATA_GE:
	  case DDATA_GT: util_pluck(Edge);
			 break;
	  case DDATA_LT:
	  case DDATA_LE:
	  case DDATA_NE:  return(false);
	                  break;

	  case DDATA_ANY: if (dg[edge].consistent != consistent_SIV || 
			      dg[edge].symbolic)
	                        return(false);
			  else
			    util_pluck(Edge);
			  break;

	  case DDATA_ERROR: return(false);
			    break;

	  default:
	          if (dist < 0)
		    return(false);
		  else if (dist > 0)
		    util_pluck(Edge);
	       }
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
	       if (loop_data[new_heap[j].index].level = i+1)
		 {
		  heap[i] =  new_heap[j];
		  loop_not_chosen = false;
		  ut_add_number(LoopChosen,j);
		 }
	       else;
	     else if (LegalPosition(EdgeList,ped,i+1,
				    loop_data[new_heap[j].index].level))
	       {
		heap[i] =  new_heap[j];
		loop_not_chosen = false;
		ut_add_number(LoopChosen,j);
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
   int         i;

     RefInfo.loop_data = loop_data;
     RefInfo.ped = ped;
     RefInfo.dg = dg_get_edge_structure(PED_DG(ped));
     RefInfo.ar = ar;
     RefInfo.num_loops = num_loops;
     for (lnode = UTIL_HEAD(loop_list);
	  lnode != NULLNODE;
	  lnode = UTIL_NEXT(lnode))
       {
	RefInfo.GroupList = util_list_alloc((Generic)NULL,"group-list");
	loop = UTIL_NODE_ATOM(lnode);
	RefInfo.level = loop_data[loop].level;
	walk_expression(gen_DO_get_stmt_LIST(loop_data[loop].node),
			PartitionNames, NOFUNC,(Generic)&RefInfo);
	CheckRefGroups(loop_data,loop,&RefInfo);
	loop_data[loop].GroupList = RefInfo.GroupList;
	loop_data[loop].OutermostLvl = outermost_lvl;
       }
     ComputeLoopOrder(loop_data,num_loops,ped,loop_list,heap);
     EdgeInfo.ped = ped;
     EdgeInfo.EdgeList = util_list_alloc((Generic)NULL,"edge-list");
     walk_expression(gen_DO_get_stmt_LIST(loop_data[loop].node),
		     GeneratePreventingEdgeList,NOFUNC,(Generic)&EdgeInfo);
     if (outermost_lvl > 1 || NOT(util_list_empty(EdgeInfo.EdgeList)))
       ComputeLegalOrder(loop_data,loop,num_loops,ped,heap,outermost_lvl,
			 EdgeInfo.EdgeList,ar);
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
     if (!loop_data[loop].transform || !loop_data[loop].distribute || 
	 loop_data[loop].type == COMPLEX || loop_data[loop].type == TRAP)
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
			  FILE       *logfile)
  {
   int i;
   UtilNode *GroupNode;
   RefGroupType *RefGroup;

     fprintf(logfile,"Ref Group Statistics for Loop at Level %d\n\n",
	     loop_data[loop].level);
     i = 0;
     for (GroupNode = UTIL_HEAD(loop_data[loop].GroupList);
	  GroupNode != NULLNODE;
	  GroupNode = UTIL_NEXT(GroupNode))
       {
	i++;
	RefGroup = (RefGroupType *)UTIL_NODE_ATOM(GroupNode);
	fprintf(logfile,"\t Ref Group %d:\n",i);
	fprintf(logfile,"\t    Elements: %d\n",RefGroup->number);
	fprintf(logfile,"\t    Locality:\n");
	if (RefGroup->Invariant)
	  fprintf(logfile,"\t      Invariant\n");
	if (RefGroup->Spatial)
	  fprintf(logfile,"\t      Spatial\n");
	if (RefGroup->OtherSpatial)
	  fprintf(logfile,"\t      Other Spatial\n");
	if (RefGroup->Temporal)
	  fprintf(logfile,"\t      Temporal\n");
	if (NOT(RefGroup->Invariant) && NOT(RefGroup->Spatial) &&
	    NOT(RefGroup->OtherSpatial) && NOT(RefGroup->Temporal))
	  fprintf(logfile,"None\n");
       }
     fprintf(logfile,"\n\n\tNumber of Ref Groups: %d\n",i);
  }


static void DumpPermutationStats(model_loop *loop_data,
				 int        loop,
				 int        num_loops,
				 int        *inner_loops,
				 FILE       *logfile)

  {
   int i,j;
   Boolean memory_order = true,
           interchanged = false;

     fprintf(logfile,"Permutation Statistic for Perfect Loop Nest %d\n\n",
	     *inner_loops);
     fprintf(logfile,"\tFinal Order:");
     for (i = 0; i < num_loops; i++)
       {
        fprintf(logfile," %d",loop_data[loop].FinalOrder[i]);
	if (loop_data[loop].FinalOrder[i] != loop_data[loop].MemoryOrder[i])
	  memory_order = false;
	if (loop_data[loop].FinalOrder[i] != i+1)
	  interchanged = true;
       }
     fprintf(logfile,"\n\n");
     if (NOT(memory_order))
       {
	fprintf(logfile,"\tMemory Order:");
	for (i = 0; i < num_loops; i++)
          fprintf(logfile," %d",loop_data[loop].MemoryOrder[i]);
	fprintf(logfile,"\n\n");
       }
     fprintf(logfile,"\t  Nesting Depth: %d\n",loop_data[loop].level);
     fprintf(logfile,"\n\n\n");
  }


static void WalkLoopsForStats(model_loop *loop_data,
			      int        loop,
			      int        num_loops,
			      int        *inner_loops,
			      int        *total_loops,
			      Boolean    *perfect,
			      PedInfo    ped)
  {
   int next,temp;

     DumpLoopStats(loop_data,loop,
		   ((config_type *)PED_MH_CONFIG(ped))->logfile);
     if (loop_data[loop].inner_loop == -1)
       {
	*inner_loops = *inner_loops + 1;
	DumpPermutationStats(loop_data,loop,num_loops,inner_loops,
			     ((config_type *)PED_MH_CONFIG(ped))->logfile);
       }
     else
       {
	next = loop_data[loop].inner_loop;
	*perfect = (*perfect || loop_data[next].next_loop == -1);
	while (next != -1)
	  {
	   *total_loops = *total_loops + 1;
	   WalkLoopsForStats(loop_data,next,num_loops+1,inner_loops,
			     total_loops,perfect,ped);
	   next = loop_data[next].next_loop;
	  }
       }
  }


void memory_interchange_stats(PedInfo       ped,
			      AST_INDEX     root,
			      int           level,
			      Boolean       *perfect,
			      int           *total_loops,
			      SymDescriptor symtab,
			      arena_type    *ar)

  {
   pre_info_type pre_info;
   model_loop    *loop_data;
   UtilList      *loop_list;
   int           inner_loops;

     (*total_loops)++;
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
     ut_check_shape(loop_data,0);
     loop_list = util_list_alloc((Generic)NULL,"loop-list");
     loop_data[0].heap = (heap_type *)ar->arena_alloc_mem(LOOP_ARENA,
							  MAXLOOP*
							  sizeof(heap_type));
     walk_loops(loop_data,0,0,1,loop_list,loop_data[0].heap,symtab,ped,ar);
     util_list_free(loop_list);
     inner_loops = 0;
     fprintf(((config_type *)PED_MH_CONFIG(ped))->logfile,
	     "Statistics for New Loop Nest");
     WalkLoopsForStats(loop_data,0,1,&inner_loops,total_loops,perfect,ped);
  }
