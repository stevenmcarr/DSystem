#include <mh.h>
#include <mh_config.h>
#include <RefGroups.h>
#include <pt_util.h>
#include <fort/gi.h>


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

void RefGroupCore::UnsetVisitedMark(AST_INDEX node)

  {
   AST_INDEX name;

     name = gen_SUBSCRIPT_get_name(node);
     get_subscript_ptr(name)->visited = false;
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

Boolean RefGroupCore::NotInOtherPositions(AST_INDEX node,
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

char* RefGroupCore::FindInductionVar(model_loop *loop_data,
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

Boolean RefGroupCore::OnlyInInnermostPosition(model_loop *loop_data,
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

Boolean RefGroupCore::CanMoveToInnermost(DG_Edge *edge)

  {
   int i;
   
     if (edge->level == LOOP_INDEPENDENT)
       return(true);
     for (i = edge->level+1; i < gen_get_dt_LVL(edge);i++)
       if (gen_get_dt_DIS(edge,i) != 0)
         return(false);
     return(true);
  }


AST_INDEX RefGroupMember::FindOldestValue(PedInfo ped)

  {
   int      refl;
   AST_INDEX node;
   Boolean  found;
   DG_Edge  *dg;
   EDGE_INDEX edge;

     for (RefGroupMemberIter RefIter(*this);
	  node = RefIter();)
       {
        refl = get_info(ped,node,type_levelv);
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

	    if (QueryEntry(dg[edge].src))
              {
               found = false;
               break;
              }
        if (found)
          break;
       }
     return(node);
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

void RefGroupMember::CheckTemporal(AST_INDEX  node,
				   PedInfo    ped,
				   model_loop *loop_data,
				   int        loop,
				   float&     TemporalCost,
				   UniformlyGeneratedSets *UGS)

  {
   int refl;
   DG_Edge *dg;
   EDGE_INDEX edge;

     if (UGS != NULL)
       if (HasGroupTemporal() && NOT(HasSelfSpatial()) && 
	   NOT(HasSelfTemporal()))

	           /* make sure cost not counted elsewhere */

	    TemporalCost += 1.0;
       else;
     else
       {
	refl = get_info(ped,node,type_levelv);
	dg = dg_get_edge_structure( PED_DG(ped));
	for (edge = dg_first_src_ref( PED_DG(ped),refl);
	     edge != END_OF_LIST && NOT(HasGroupTemporal());
	     edge = dg_next_src_ref( PED_DG(ped),edge))

	/* look for outgoing consistent edge that would be 
	   loop independent or carried by the innermost loop
	   if "loop" were innermost */

	  if ((dg[edge].level == loop_data[loop].level  ||
	       dg[edge].level == LOOP_INDEPENDENT) && 
	      CanMoveToInnermost(&dg[edge]))
	    {
	     SetGroupTemporal();

	           /* make sure cost not counted elsewhere */
	     
	     if (NOT(HasSelfSpatial()) && NOT(HasSelfTemporal()))
	       TemporalCost += 1.0;
	    }
       }
  }

/****************************************************************/
/*                                                              */
/*   Function:   CheckGroupSpatial                              */
/*                                                              */
/*   Input:      node - oldest value in a RefGroup              */
/*               ped - dependence graph, etc.                   */
/*               loop_data - loop structure                     */
/*               loop - loop being considered as innermost      */
/*               GroupSpatialCost - lines/iteration required for*/
/*                              groups with group-spatial reuse */
/*               RefGroup - reference group                     */
/*               words - number of words in a cache line        */
/*                                                              */
/*   Description: Check if ref group with "node" as oldest      */
/*                value has group-spatial reuse.                */
/*                                                              */
/****************************************************************/

void RefGroupMember::CheckGroupSpatial(AST_INDEX  node,
				       PedInfo    ped,
				       model_loop *loop_data,
				       int        loop,
				       float&     GroupSpatialCost,
				       int        words,
				       UniformlyGeneratedSets *UGS)

  {
   int refl,coeff;
   DG_Edge *dg;
   EDGE_INDEX edge;
   AST_INDEX sub_list,sub;
   char *var;
   Boolean lin;

     if (UGS != NULL)
       if (HasGroupSpatial() && NOT(HasGroupTemporal()) && 
	   NOT(HasSelfTemporal()))
	  
	  /* make sure cost not counted elsewhere */
	  
	    GroupSpatialCost += 1.0;
       else;
     else
       {
	refl = get_info(ped,node,type_levelv);
	dg = dg_get_edge_structure( PED_DG(ped));

               /* look for an outgoing edge that is carried by the
		  loop with its induction variable in the first
		  subscript position and has no other non-zero 
		  distance vector entries */

	for (edge = dg_first_src_ref( PED_DG(ped),refl);
	     edge != END_OF_LIST && NOT(HasGroupSpatial());
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
		SetGroupSpatial();
		
		/* make sure cost not counted elsewhere */
		
		if (NOT(HasGroupTemporal()) && 
		    NOT(HasSelfTemporal()))
		  GroupSpatialCost += 1.0;
	       }
	    }
       }
  }
/****************************************************************/
/*                                                              */
/*   Function:  CheckSelfSpatial                                */
/*                                                              */
/*   Description:
/*                                                              */
/****************************************************************/

void RefGroupMember::CheckSelfSpatial(AST_INDEX node,
				      char *var,
				      int words,
				      float& SpatialCost,
				      UniformlyGeneratedSets *UGS)

  {
   AST_INDEX sub,sub_list;
   int coeff;
   Boolean lin;

     if (UGS != NULL)
       if (HasSelfSpatial())
	  SpatialCost += (1.0/((float)(words)/(float)coeff));
       else;
     else
       {
	sub_list = gen_SUBSCRIPT_get_rvalue_LIST(tree_out(node));
	sub = list_first(sub_list);
	if (pt_find_var(sub,var) && NotInOtherPositions(sub_list,var))
	  {
	   pt_get_coeff(sub,var,&lin,&coeff);	
	   if (coeff < 0)
	     coeff = -coeff;
	
	   /* make sure step size less than line size */

	   if (coeff < words && lin)
	     {
	      SetSelfSpatial();
	      SpatialCost += (1.0/((float)(words)/(float)coeff));
	     }
	  }
       }
  }
/****************************************************************/
/*                                                              */
/*   Function:  CheckSelfTemporal                               */
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

void RefGroupMember::CheckSelfTemporal(AST_INDEX  node,
				       PedInfo    ped,
				       model_loop *loop_data,
				       int        loop,
				       float&     InvariantCost,
				       UniformlyGeneratedSets *UGS)

  {
   int refl;
   DG_Edge *dg;
   EDGE_INDEX edge;

     if (UGS != NULL)
       if (HasSelfTemporal())
	  InvariantCost += 1.0;
       else;
     else
       {
	refl = get_info(ped,node,type_levelv);
	dg = dg_get_edge_structure( PED_DG(ped));
	
	/* look for consisitent dependence from a reference to
	   itself */
	
	for (edge = dg_first_src_ref( PED_DG(ped),refl);
	     edge != END_OF_LIST && NOT(HasSelfTemporal());
	     edge = dg_next_src_ref( PED_DG(ped),edge))
	  if (dg[edge].level == loop_data[loop].level &&
	      dg[edge].src == dg[edge].sink &&
	      dg[edge].consistent != inconsistent &&
	      NOT(dg[edge].symbolic))
	    {
	     SetSelfTemporal();
	     InvariantCost += 1.0;
	    }
       }
  }

void RefGroupSet::BuildRefGroupsWithUGS()

  {
   UniformlyGeneratedSetsEntry *UGSEntry;
   RefGroupMember *RefGroup;
   IntegerList UGSTemp;
   AST_INDEX node,newnode;
   Boolean DoCompare = false;
   UGSIterator UGSIter(*UGS);
   UGSEntryIterator *UGSEntryIter1, *UGSEntryIter2;

     while(UGSEntry = (UniformlyGeneratedSetsEntry *)UGSIter())
       {
	 UGSEntryIter1 = new UGSEntryIterator(*UGSEntry);
	 while(node = (AST_INDEX)(*UGSEntryIter1)())
	   if (NOT(UGSTemp.QueryEntry(node)))
	     {
	       UGSTemp += node;
	       RefGroup = new RefGroupMember;
	       (*RefGroup) += gen_SUBSCRIPT_get_name(node);
	       if (UGSEntry->SingleNodeHasSelfTemporalReuse())
		 RefGroup->SetSelfTemporal();
	       else if (UGSEntry->SingleNodeHasSelfSpatialReuse())
		 RefGroup->SetSelfSpatial();
	       UGSEntryIter2 = new UGSEntryIterator(*UGSEntry);
	       while(newnode = (AST_INDEX)(*UGSEntryIter2)())
		 if (node == newnode)
		   DoCompare = true;
		 else if (DoCompare && NOT(UGSTemp.QueryEntry(newnode)))
		   if (UGSEntry->NodesHaveGroupTemporalReuse(node,newnode))
		     {
		       (*RefGroup) += gen_SUBSCRIPT_get_name(newnode);
		       UGSTemp += newnode;
		       RefGroup->SetGroupTemporal();
		     }
		   else if (UGSEntry->NodesHaveGroupSpatialReuse(node,newnode))
		     {
		       (*RefGroup) += gen_SUBSCRIPT_get_name(newnode);
		       UGSTemp += newnode;
		       RefGroup->SetGroupSpatial();
		     }
	       DoCompare = false;
	       (*this) += (int)RefGroup;
	     }
       }
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


void RefGroupSet::DoPartition(AST_INDEX name,
			      RefGroupMember *RG,
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

              /* add reference to RefGroup */

     (*RG) += name;
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

       if (dg[edge].level >= MinLevel ||
	   dg[edge].level == LOOP_INDEPENDENT)
         if ((dg[edge].consistent == consistent_SIV ||  /* check consistency */
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
	      DoPartition(dg[edge].sink,RG,dg,ped,level,MinLevel,
			  VisitedMark,loop_data);
	   }

   /* look at all incoming edges for references fitting the
      same criteria. */

     for (edge = dg_first_sink_ref( PED_DG(ped),refl);
	  edge != END_OF_LIST;
	  edge = dg_next_sink_ref( PED_DG(ped),edge))
       if (dg[edge].level >= MinLevel ||
	   dg[edge].level == LOOP_INDEPENDENT)
         if ((dg[edge].consistent == consistent_SIV ||  /* check consistency */
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
	      DoPartition(dg[edge].src,RG,dg,ped,level,MinLevel,
			  VisitedMark,loop_data);
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

void RefGroupSet::PartitionNames(AST_INDEX   node,
				 RefInfoType& RefInfo)


  {
   subscript_info_type *sptr;
   AST_INDEX            name;
   RefGroupMember      *RG;

     if (is_subscript(node) &&
	 !is_call(ut_get_stmt(node)))
       {
	name = gen_SUBSCRIPT_get_name(node);
	sptr = get_subscript_ptr(name);
	if (sptr->visited != RefInfo.VisitedMark)
	  {
	   RG = new RefGroupMember;
	   (*this) += (int)RG;
	   DoPartition(name,RG,RefInfo.dg,RefInfo.ped,
		       RefInfo.level,RefInfo.loop_data[0].level,
		       RefInfo.VisitedMark,RefInfo.loop_data);
	  }
       }
  }



RefGroupSet::RefGroupSet(AST_INDEX loop, int NL,RefInfoType& RefInfo,
			 Boolean UseUGS,int *LIS)
  { 
   int i;
   AST_INDEX sub_list,
             sub,node;
   Boolean   lin;
   int       coeff,words;
   int       level_val,dims;
   char      *var;
   RefGroupMember *RefGroup;
   RefGroupSetIter *RefIter;

     UseUniformlyGeneratedSets = UseUGS;
     if (UseUGS && LIS == NULL)
       {
	LIS = new int[NL];
	for (i = 0; i < NL; i++)
	  if (i == RefInfo.level-1)
	    LIS[i] = 1;
	  else
	    LIS[i] = 0;
	UGS = new UniformlyGeneratedSets(loop,NL,RefInfo.IVar,LIS);
	BuildRefGroupsWithUGS();
       }
     else
       {
	for (AstIter AIter1(loop,false); (node = AIter1()) != AST_NIL;)
	  if (is_subscript(node))
	    UnsetVisitedMark(node);
	RefInfo.VisitedMark = true;
	for (AstIter AIter2(loop,false); (node = AIter2()) != AST_NIL;)
	  if (is_subscript(node))
	    PartitionNames(node,RefInfo);
	UGS = NULL;
       }
     RefIter = new RefGroupSetIter(*this);
     while(RefGroup = (RefGroupMember *)(*RefIter)())
       {
	var = RefInfo.IVar[RefInfo.level-1];
	node = RefGroup->FindOldestValue(RefInfo.ped);
	sub_list = gen_SUBSCRIPT_get_rvalue_LIST(tree_out(node));
	if (gen_get_converted_type(node) == TYPE_DOUBLE_PRECISION ||
	    gen_get_converted_type(node) == TYPE_COMPLEX)
	  words = (((config_type *)PED_MH_CONFIG(RefInfo.ped))->
		   line) >> 3; 
	else
	  words = (((config_type *)PED_MH_CONFIG(RefInfo.ped))->
		   line) >> 2; 
 
	        /* check for self-spatial reuse, induction var
		   appears in first subscript position only */

	if (pt_find_var(sub_list,var))
	  RefGroup->CheckSelfSpatial(node,var,words,RefInfo.SpatialCost,UGS);
	else 

	         /* check for invariant reuse */
	  RefGroup->CheckSelfTemporal(node,RefInfo.ped,RefInfo.loop_data,
				      RefInfo.loop,RefInfo.InvariantCost,UGS);

	        /* check for group reuse */

	if (RefGroup->Count() > 1)
	  {
	   RefGroup->CheckTemporal(node,RefInfo.ped,RefInfo.loop_data,
				   RefInfo.loop, RefInfo.TemporalCost,UGS);
	   RefGroup->CheckGroupSpatial(node,RefInfo.ped,RefInfo.loop_data,
				       RefInfo.loop,RefInfo.GroupSpatialCost,
				       words,UGS);
	  }

	       /* check if no reuse found */

	if (NOT(RefGroup->HasSelfTemporal()) && 
	    NOT(RefGroup->HasSelfSpatial()) &&
	    NOT(RefGroup->HasGroupSpatial()) && 
	    NOT(RefGroup->HasGroupTemporal()))
	   RefInfo.NoneCost += 1.0;
       }
   };
