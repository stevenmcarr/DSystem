#include <iostream.h>
#include <fstream.h>
#include <libs/support/misc/general.h>
#include <libs/Memoria/include/mh_ast.h>
#include <libs/Memoria/include/mh_config.h>
#include <libs/frontEnd/ast/forttypes.h>
#include <libs/frontEnd/ast/groups.h>
#include <libs/Memoria/ut/DDG.h>
#include <libs/frontEnd/ast/AstIterators.h>
#include <libs/Memoria/include/la.h>
#include <libs/Memoria/ut/ScalarReplaceMap.h>
#include <libs/frontEnd/fortTree/fortsym.h>
#include <libs/Memoria/include/mem_util.h>
#include <libs/frontEnd/ast/builtins.h>

extern char *mc_module;

AstIterAdvanceDirective DDG::CountNodes(AST_INDEX Node,
					int& size)
  
{
  if (is_subscript(Node))
  {
    size += 2; // one for subscript, one for name 
    return (AST_ITER_SKIP_CHILDREN);
  }
  else if (is_operator(Node) ||
	   (is_identifier(Node) && NOT(is_subscript(tree_out(Node))) &&
	    NOT(builtins_isBuiltinFunction(gen_get_text(Node)))))
    size++;
  return (AST_ITER_CONTINUE);
}

void DDG::AddArrayToMap(AST_INDEX Node)
{
  ASTMap->MapAddEntry(Node,new DDGNode(Node,this));
    
  // The array name is added to the DDG to account for address
  // calculations
    
  AST_INDEX Name = gen_SUBSCRIPT_get_name(Node);
  ASTMap->MapAddEntry(Name,new DDGNode(Name,this));
}


AstIterAdvanceDirective DDG::ProcessSubscripts(AST_INDEX Node)
{
  if (is_subscript(Node))
  {
    AddArrayToMap(Node);
    CheckForIndexArrays(Node);
  }
  return (AST_ITER_CONTINUE);
}


void DDG::CheckForIndexArrays(AST_INDEX Node)
{
  AstIterAdvanceDirective WalkAdvance;
  AST_INDEX ASTNode;

  for (AstIterator ASTIter(gen_SUBSCRIPT_get_rvalue_LIST(Node));
       ASTNode = ASTIter.Current();
       ASTIter.Advance(WalkAdvance))
    WalkAdvance = ProcessSubscripts(ASTNode);
}
    

AstIterAdvanceDirective DDG::InitializeNodeMapping(AST_INDEX Node)
{

  if (is_subscript(Node))
  {
    AddArrayToMap(Node);
    CheckForIndexArrays(Node);
    return (AST_ITER_SKIP_CHILDREN);
  }
  else if (is_operator(Node) ||
	   (is_identifier(Node) && NOT(is_subscript(tree_out(Node))) &&
	    NOT(builtins_isBuiltinFunction(gen_get_text(Node)))))
    ASTMap->MapAddEntry(Node,new DDGNode(Node,this));
	   
  return (AST_ITER_CONTINUE);
  }


int DDG::GetOperationLatency(AST_INDEX Node,
			     DataReuseModel& ReuseModel,
			     ScalarReplaceMap& SRMap,
			     PedInfo ped)
{
  int ops;

  if (gen_get_converted_type(Node) == TYPE_COMPLEX)
    ops = 2;
  else
    ops = 1;

  if (is_subscript(Node))
    if (NOT(is_assignment(tree_out(Node)) && gen_ASSIGNMENT_get_lvalue(tree_out(Node)) == Node))
    {
      switch (ReuseModel.GetNodeReuseType(Node))
      {      
      case SELF_TEMPORAL:    
      case GROUP_TEMPORAL:
	if (SRMap[Node].IsReplaced)
	  return 0; // these references will be scalar replaced
	else
	  return ((config_type *)PED_MH_CONFIG(ped))->hit_cycles;
	break;

      case SELF_SPATIAL:  
      case GROUP_SPATIAL: 
	return ((config_type *)PED_MH_CONFIG(ped))->hit_cycles;
	break;

      case NONE:           
	return (((config_type *)PED_MH_CONFIG(ped))->miss_cycles);
	break;

      default:
	return ((config_type *)PED_MH_CONFIG(ped))->hit_cycles;
	break;

      }
    }

  // 
  // We have a store. If the store is a generator, then there is no
  // delay necessary because the value will be in a register. If this
  // is not a generator then, we must wait for the store to finish
  // store misses do not need to be accounted for because of a likely
  // store buffer.
  //

    else if (SRMap[Node].IsGenerator || SRMap[Node].IsReplaced)
      return 0;
    else
      return ((config_type *)PED_MH_CONFIG(ped))->hit_cycles;
  else if (is_subscript(tree_out(Node))) // for address calculation
    return ((config_type *)PED_MH_CONFIG(ped))->add_cycles;
  else if (is_identifier(Node))
    return 0;
  else if (gen_get_converted_type(Node) == TYPE_INTEGER)
    return ((config_type *)PED_MH_CONFIG(ped))->IntegerPipeLength;
  else
  {
    int ops;

    if (gen_get_converted_type(Node) == TYPE_COMPLEX)
      ops = 2;
    else
      ops = 1;
    if (is_binary_plus(Node) || is_binary_minus(Node))
      return ((config_type *)PED_MH_CONFIG(ped))->add_cycles*ops;
    else if (is_binary_times(Node))
      return ((config_type *)PED_MH_CONFIG(ped))->mul_cycles*ops;
    else if (is_binary_divide(Node))
      return ((config_type *)PED_MH_CONFIG(ped))->div_cycles*ops;
    else
      return ((config_type *)PED_MH_CONFIG(ped))->FloatPipeLength;
  }
}

void DDG::BuildDDG(AST_INDEX LoopBody,
		   DataReuseModel& ReuseModel,
		   AddressEquivalenceClassSet& AECS,
		   ScalarReplaceMap& SRMap,
		   PedInfo ped,
		   int level)
{
  AST_INDEX ASTNode;
  AstIterAdvanceDirective WalkAdvance;
  int size = 0;

  for (AstIterator ASTIter(LoopBody);
       ASTNode = ASTIter.Current();
       ASTIter.Advance(WalkAdvance))
    WalkAdvance = CountNodes(ASTNode,size);

  ASTMap = new ASTToDDGNodeMap;
  ASTMap->MapCreate(size*2);

  for (AstIterator ASTIter(LoopBody);
       ASTNode = ASTIter.Current();
       ASTIter.Advance(WalkAdvance))
    WalkAdvance = InitializeNodeMapping(ASTNode);

  // add DDG edges corresponding to the AST and dependence graph
  // There are no statement nodes in the DDG

  AddASTAndDependenceEdgesToDDG(LoopBody,ped,ReuseModel,SRMap,level);
  AddAddressRegisterEdgesToDDG(AECS,ped);

}

void DDG::AddASTAndDependenceEdgesToDDG(AST_INDEX LoopBody,
					PedInfo ped,
					DataReuseModel& ReuseModel,
					ScalarReplaceMap& SRMap,
					int level)
{
  AST_INDEX name;
  DG_Edge   *dg;
  int       vector;
  EDGE_INDEX edge;
  int size;
  DDGNode *ddgNode;
  int InstructionLatency;
  AST_INDEX ASTNode,lhs;
  DDGEdgeType Type;

  dg = dg_get_edge_structure( PED_DG(ped));
  for (DDGNodeIterator DDGNodeIter(this);
       ddgNode = DDGNodeIter.Current();
       ++DDGNodeIter)
  {
    ASTNode = ddgNode->GetASTNode();

    InstructionLatency = GetOperationLatency(ASTNode,ReuseModel,SRMap,ped);
    if (is_subscript(ASTNode))
      {

	// add edge associated with AST

	AST_INDEX Parent = tree_out(ASTNode);

	if (NOT(is_f77_statement(Parent)))
	  if (NOT(is_subscript(Parent)))

	    // this is not an index array used
	    // as a subscript. 

	    DDGEdge *GraphEdge = new
	      DDGEdge(this,ddgNode,GetASTMap()->MapToDDGNode(Parent),
		      0,InstructionLatency,TrueDep);


	  else // deal with index arrays
	      {
		AST_INDEX Name = gen_SUBSCRIPT_get_name(Parent);
		
		// add an edge from the index array to the name node of
		// the array because the index array is part of the
		// address calculation
		
		DDGEdge *GraphEdge = new
		  DDGEdge(this,ddgNode,GetASTMap()->MapToDDGNode(Name),
			  0,InstructionLatency,TrueDep);
	      }
		
	      
	// add dependence graph edges

	AST_INDEX name = gen_SUBSCRIPT_get_name(ASTNode);
	vector = get_info(ped,name,type_levelv);
	for (edge = dg_first_src_ref(PED_DG(ped),vector);
	     edge != END_OF_LIST;
	     edge = dg_next_src_ref( PED_DG(ped),edge))
	  if (dg[edge].level == level || dg[edge].level == LOOP_INDEPENDENT)
	  {
	    if (dg[edge].type == dg_anti)
	    {
	      InstructionLatency = 0;
	      Type = AntiDep;
	    }
	    else if (dg[edge].type == dg_output)
	    {
	      InstructionLatency = 
		((config_type *)PED_MH_CONFIG(ped))->hit_cycles;
	      Type = OutputDep;
	    }
	    else
	      Type = TrueDep;
	      
	    int dist = gen_get_dt_DIS(&dg[edge],dg[edge].level);

	    if (dist < 0)   // for direction vectors make the dist 1
			    // to be conservative
	      dist = 1;

	    DDGEdge *GraphEdge = new 
	      DDGEdge(this,ddgNode,
		      GetASTMap()->MapToDDGNode(tree_out(dg[edge].sink)),
		      dist,InstructionLatency,Type);
	  }

      }


    // add scalar dependences

    else if (is_identifier(ASTNode) && NOT(is_subscript(tree_out(ASTNode))))
    {
      int Distance;

      vector = get_info(ped,ASTNode,type_levelv);
      for (edge = dg_first_src_ref(PED_DG(ped),vector);
	   edge != END_OF_LIST;
	   edge = dg_next_src_ref( PED_DG(ped),edge))
	if (dg[edge].type == dg_true)
	{
	  if (dg[edge].level == LOOP_INDEPENDENT)
	    Distance = 0;
	  else
	    Distance = 1;
	  DDGEdge *GraphEdge = new 
	    DDGEdge(this,ddgNode,
		    GetASTMap()->MapToDDGNode(dg[edge].sink),
		    Distance,0,TrueDep);
	}

      // add edge associated with AST
      
      if (NOT(is_f77_statement(tree_out(ASTNode))))
	DDGEdge *GraphEdge = new
	  DDGEdge(this,ddgNode,
		  GetASTMap()->MapToDDGNode(tree_out(ASTNode)),
		  0,0,TrueDep);
    }

    // add an AST edge, but not for the name node of a subscript
    // expression nor for statement nodes

    else if (NOT(is_f77_statement(tree_out(ASTNode))) &&
	     NOT(is_subscript(tree_out(ASTNode))))
      DDGEdge *GraphEdge = new
	DDGEdge(this,ddgNode,
		GetASTMap()->MapToDDGNode(tree_out(ASTNode)),
		0,InstructionLatency,TrueDep);
	
    // connect the rhs of an assignment with the lhs
    
    if (is_assignment(tree_out(ASTNode)))
    {
      lhs = gen_ASSIGNMENT_get_lvalue(tree_out(ASTNode));
      if (lhs != ASTNode)
	DDGEdge *GraphEdge = new
	  DDGEdge(this,ddgNode,
		  GetASTMap()->MapToDDGNode(lhs),
		  0,InstructionLatency,TrueDep);
    }
  }
}

void DDG::AddAddressRegisterEdgesToDDG(AddressEquivalenceClassSet& AECS,
				       PedInfo ped)

{
  AddressEquivalenceClass *Class;
  AST_INDEX Node;
  DDGNode *FirstAddressReference,*AddressUse;
  int Distance, Latency;
  
  for (AddressEquivSetIterator AECSIter(AECS);
       Class = AECSIter();)
  {
    FirstAddressReference =
      GetASTMap()->MapToDDGNode(gen_SUBSCRIPT_get_name(Class->GetFirstInLoop()));
    
    for (AddressClassIterator ACIter(*Class);
	 Node = ACIter();)
    {
      AddressUse = GetASTMap()->MapToDDGNode(gen_SUBSCRIPT_get_name(Node));

      // add true dependence edges for address calculations

      if (FirstAddressReference == AddressUse)
	Distance = 1;
      else
	Distance = 0;
      DDGEdge *GraphEdge = 
	new DDGEdge(this,FirstAddressReference,AddressUse, Distance,
		    ((config_type *)PED_MH_CONFIG(ped))->IntegerPipeLength,TrueDep);
      
      // Add anti-dependence edge for address register

      GraphEdge = new DDGEdge(this,AddressUse,FirstAddressReference, 1, 0,AntiDep);
      
      //
      // Add in edge from Address register to its use in the load
      // (subscript node). This will be non-zero only for the one
      // calculation of the base address
      //

      
      if (FirstAddressReference == AddressUse)
	Latency = ((config_type *)PED_MH_CONFIG(ped))->IntegerPipeLength;
      else
	Latency = 0;
      GraphEdge = new DDGEdge(this,AddressUse,GetASTMap()->MapToDDGNode(Node),0,Latency,TrueDep);
    }
  }
}

void DDG::DumpDDGInDOTForm(SymDescriptor SymbolTable)
{
  DDGNode *ddgNode;
  DDGEdge *ddgEdge;
  char *FileName = new char[strlen(mc_module)+4];
  char *NodeText = new char[80];
  char *SinkText = new char[80];

  sprintf(FileName,"%s.dot",mc_module);


  ofstream outFile(FileName,ios::out);

  outFile << "  digraph G {" << endl;
  outFile << "\tcenter=1; size= \"7.5,10\";" << endl;

  for (DDGNodeIterator ddgNodeIter(this);
       ddgNode = ddgNodeIter.Current();
       ++ddgNodeIter)
  {
    ddgNode->GetText(NodeText,SymbolTable);
    for (DDGEdgeIterator ddgEdgeIter(ddgNode,DirectedEdgeOut);
	 ddgEdge = ddgEdgeIter.Current();
	 ++ddgEdgeIter)
    {
      ddgEdge->GetSink()->GetText(SinkText,SymbolTable);
      outFile << "\t" << NodeText << " -> " << SinkText << "[label=\"<" << ddgEdge->GetOmega() 
	      << "," << ddgEdge->GetLatency() << ">\"";
      switch(ddgEdge->GetType()) 
      {
        case TrueDep:
	  outFile << "style=bold];" << endl;
	  break;
        case AntiDep:
	  outFile << ",style=dotted];" << endl;
	  break;
        case OutputDep:
	  outFile << "];" << endl;
      }
    }
  }

  outFile << "  }" << endl;

  outFile.close();

  delete FileName;
  delete NodeText;
  delete SinkText;
  
}

void DDGNode::GetText(char *Text,
		      SymDescriptor SymbolTable)
{
  AST_INDEX ASTNode = GetASTNode();
  if (is_subscript(ASTNode))
  {
    char SubscriptText[80];

    ut_GetSubscriptText(ASTNode,SubscriptText,SymbolTable);
    sprintf(Text,"\"%s:%d\"",SubscriptText,GetId());
  }
  else if (is_identifier(ASTNode))
    sprintf(Text,"\"id:%s:%d\"",gen_get_text(ASTNode),GetId());
  else if (is_binary_plus(ASTNode))
    sprintf(Text,"\"+:%d\"",GetId());
  else if (is_binary_minus(ASTNode))
    sprintf(Text,"\"-:%d\"",GetId());
  else if (is_binary_times(ASTNode))
    sprintf(Text,"\"*:%d\"",GetId());
  else if (is_binary_divide(ASTNode))
    sprintf(Text,"\"/:%d\"",GetId());
  else if (is_invocation(ASTNode))
    sprintf(Text,"\"%s:%d\"",gen_get_text(gen_INVOCATION_get_name(ASTNode)),
	    GetId());
  else 
    sprintf(Text,"\"ukn:%d\"",GetId());
}
